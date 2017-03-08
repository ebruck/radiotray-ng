// Copyright 2017 Edward G. Bruck <ed.bruck1@gmail.com>
//
// This file is part of Radiotray-NG.
//
// Radiotray-NG is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Radiotray-NG is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Radiotray-NG.  If not, see <http://www.gnu.org/licenses/>.

#include <radiotray-ng/common.hpp>
#include <radiotray-ng/media_keys/media_keys.hpp>
#include <radiotray-ng/i_radiotray_ng.hpp>
#include <radiotray-ng/i_config.hpp>

#include <condition_variable>
#include <gio/gio.h>
#include <radiotray-ng/g_threading_helper.hpp>
#include <mutex>
#include <thread>
#include <map>

#include <unistd.h>

class media_keys_t
{
public:
	media_keys_t(std::shared_ptr<IRadioTrayNG> radiotray_ng, std::shared_ptr<IConfig> config)
		: radiotray_ng(std::move(radiotray_ng))
		, config(std::move(config))
		, main_loop(nullptr)
		, app_name(std::string(APP_NAME) + "-" + std::to_string(::getpid()))
	{
		// install volume up/down?
		if (this->config->get_bool(MEDIA_KEY_MAPPING, DEFAULT_MEDIA_KEY_MAPPING))
		{
			this->volume_key_map[radiotray_ng::to_lower(this->config->get_string(VOLUME_UP_MEDIA_KEY, DEFAULT_VOLUME_UP_MEDIA_KEY))] = true;
			this->volume_key_map[radiotray_ng::to_lower(this->config->get_string(VOLUME_DOWN_MEDIA_KEY, DEFAULT_VOLUME_DOWN_MEDIA_KEY))] = false;

			this->station_key_map[radiotray_ng::to_lower(this->config->get_string(NEXT_STAITON_MEDIA_KEY, DEFAULT_NEXT_STATION_MEDIA_KEY))] = true;
			this->station_key_map[radiotray_ng::to_lower(this->config->get_string(PREVIOUS_STAITON_MEDIA_KEY, DEFAULT_PREVIOUS_STATION_MEDIA_KEY))] = false;

			LOG(info) << "mapping volume up/down to: "<< this->config->get_string(VOLUME_UP_MEDIA_KEY, DEFAULT_VOLUME_UP_MEDIA_KEY) << ", "
				<< this->config->get_string(VOLUME_DOWN_MEDIA_KEY, DEFAULT_VOLUME_DOWN_MEDIA_KEY);
		}

		LOG(info) << "starting gio thread for: " << this->app_name;

		std::unique_lock<std::mutex> lock(main_loop_mutex);

		this->gio_player_thread = std::thread(&media_keys_t::gio_thread, this);

		// wait for main_loop to be ready...
		main_loop_ready_cv.wait(lock);
	}

	~media_keys_t()
	{
		LOG(info) << "stopping gio thread";

		if (g_main_loop_is_running(this->main_loop))
		{
			g_main_loop_quit(this->main_loop);
		}

		if (this->gio_player_thread.joinable())
		{
			this->gio_player_thread.join();
		}
	}

private:
	void gio_thread();

	static void on_gio_signal(GDBusProxy* proxy, gchar* sender_name, gchar* signal_name, GVariant* parameters, gpointer user_data);

	std::shared_ptr<IRadioTrayNG> radiotray_ng;
	std::shared_ptr<IConfig> config;
	GMainLoop* main_loop;
	const std::string app_name;

	std::thread gio_player_thread;
	std::mutex  main_loop_mutex;
	std::condition_variable main_loop_ready_cv;

	std::map<std::string, bool> volume_key_map;
	std::map<std::string, bool> station_key_map;
};


void media_keys_t::on_gio_signal(GDBusProxy* /*proxy*/, gchar* /*sender_name*/, gchar* /*signal_name*/, GVariant* parameters, gpointer user_data)
{
	media_keys_t* media_keys{static_cast<media_keys_t*>(user_data)};

	if (parameters == nullptr || g_variant_n_children(parameters) != 2)
	{
		LOG(error) << "media key signal invalid, ignoring event";
		return;
	}

	// grab the key info
	std::string key_pressed;

	GVariant* tmp = g_variant_get_child_value(parameters, 1);
	const gchar* key = g_variant_get_string(tmp, NULL);

	if (key != nullptr)
	{
		key_pressed = key;

		LOG(debug) << "media key pressed: " << key_pressed;
	}
	else
	{
		LOG(error) << "failed to extract media key information, ignoring event";
		g_variant_unref(tmp);
		return;
	}
	g_variant_unref(tmp);

	// finally...
	if (key_pressed == "Stop")
	{
		media_keys->radiotray_ng->stop();
		return;
	}

	if (key_pressed == "Play")
	{
		if (media_keys->radiotray_ng->get_state() == STATE_STOPPED)
		{
			media_keys->radiotray_ng->play();
		}
		else
		{
			media_keys->radiotray_ng->stop();
		}
		return;
	}

	// use media key mapping?
	if (media_keys->config->get_bool(MEDIA_KEY_MAPPING, DEFAULT_MEDIA_KEY_MAPPING))
	{
		// volume switching...
		{
			auto it = media_keys->volume_key_map.find(radiotray_ng::to_lower(key_pressed));

			if (it != media_keys->volume_key_map.end())
			{
				if (it->second == true)
				{
					media_keys->radiotray_ng->volume_up_msg();
					return;
				}
				else
				{
					media_keys->radiotray_ng->volume_down_msg();
					return;
				}
			}
		}

		// station switching...
		{
			auto it = media_keys->station_key_map.find(radiotray_ng::to_lower(key_pressed));

			if (it != media_keys->station_key_map.end())
			{
				if (it->second == true)
				{
					media_keys->radiotray_ng->next_station_msg();
					return;
				}
				else
				{
					media_keys->radiotray_ng->previous_station_msg();
					return;
				}
			}
		}
	}

	LOG(debug) << "ignoring " << key_pressed;
}


void media_keys_t::gio_thread()
{
	GError*     error{nullptr};
	GDBusProxy* proxy{nullptr};

	radiotray_ng::g_threading_helper gth;

	// scope our locking...
	{
		std::unique_lock<std::mutex> lock(this->main_loop_mutex);

		this->main_loop = g_main_loop_new(nullptr, FALSE);

		proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
			GDBusProxyFlags{G_DBUS_PROXY_FLAGS_NONE},
			nullptr,
			"org.gnome.SettingsDaemon",
			"/org/gnome/SettingsDaemon/MediaKeys",
			"org.gnome.SettingsDaemon.MediaKeys",
			nullptr,
			&error);

		if (proxy == nullptr)
		{
			LOG(error) << "could not connect to dbus, media keys disabled";

			this->main_loop_ready_cv.notify_one();
			g_main_loop_unref(this->main_loop);
			return;
		}

		g_signal_connect(proxy, "g-signal", G_CALLBACK(on_gio_signal), this);

		g_dbus_proxy_call(proxy,
			"GrabMediaPlayerKeys",
			g_variant_new("(su)", this->app_name.c_str(), 0),
			G_DBUS_CALL_FLAGS_NO_AUTO_START,
			-1,
			nullptr,
			nullptr,
			nullptr);
	}

	// signal that we are starting our loop
	this->main_loop_ready_cv.notify_one();

	g_main_loop_run(this->main_loop);

	// cleanup
	g_dbus_proxy_call(proxy,
		"ReleaseMediaPlayerKeys",
		g_variant_new("(s)", this->app_name.c_str()),
		G_DBUS_CALL_FLAGS_NO_AUTO_START,
		-1,
		nullptr,
		nullptr,
		nullptr);

	g_object_unref(proxy);
	g_main_loop_unref(this->main_loop);
}


MediaKeys::MediaKeys(std::shared_ptr<IRadioTrayNG> radiotray_ng, std::shared_ptr<IConfig> config)
	: media_keys_impl(std::make_shared<media_keys_t>(radiotray_ng, config))
{
}
