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

#include "appindicator_gui.hpp"
#include <radiotray-ng/common.hpp>
#include <radiotray-ng/i_config.hpp>
#include <radiotray-ng/i_bookmarks.hpp>
#include <radiotray-ng/i_radiotray_ng.hpp>
#include <radiotray-ng/g_threading_helper.hpp>
#include <radiotray-ng/helpers.hpp>
#include <rtng_user_agent.hpp>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


AppindicatorGui::AppindicatorGui(std::shared_ptr<IConfig> config, std::shared_ptr<IRadioTrayNG> radiotray_ng,
	                             std::shared_ptr<IBookmarks> bookmarks, std::shared_ptr<IEventBus> event_bus)
	: radiotray_ng(std::move(radiotray_ng))
	, bookmarks(std::move(bookmarks))
	, config(std::move(config))
	, event_bus(std::move(event_bus))
	, appindicator(nullptr)
	, menu(nullptr)
	, action_menu_item(nullptr)
	, volume_menu_item(nullptr)
	, status_menu_item(nullptr)
	, sleep_timer_menu_item(nullptr)
	, sleep_timer_id(0)
{
	this->event_bus->subscribe(IEventBus::event::tags_changed, std::bind(&AppindicatorGui::on_tags_event, this, std::placeholders::_1,
		std::placeholders::_2));

	this->event_bus->subscribe(IEventBus::event::state_changed, std::bind(&AppindicatorGui::on_state_event, this, std::placeholders::_1,
		std::placeholders::_2));

	this->event_bus->subscribe(IEventBus::event::volume_changed, std::bind(&AppindicatorGui::on_volume_event, this, std::placeholders::_1,
		std::placeholders::_2));

	this->event_bus->subscribe(IEventBus::event::station_error, std::bind(&AppindicatorGui::on_station_error_event, this, std::placeholders::_1,
		std::placeholders::_2));
}


void AppindicatorGui::on_state_event(const IEventBus::event& /*ev*/, IEventBus::event_data_t& data)
{
	const std::string state{data[STATE_KEY]};

	this->update_status_menu_item(state);
	this->update_action_menu_item(state);

	if (state == STATE_PLAYING || state == STATE_BUFFERING || state == STATE_CONNECTING)
	{
		app_indicator_set_icon(this->appindicator, RADIOTRAY_NG_ICON_ON);
		return;
	}

	if (state == STATE_STOPPED)
	{
		app_indicator_set_icon(this->appindicator, RADIOTRAY_NG_ICON_OFF);
		return;
	}
}


void AppindicatorGui::on_volume_event(const IEventBus::event& /*ev*/, IEventBus::event_data_t& /*data*/)
{
	this->update_volume_menu_item();
}


void AppindicatorGui::on_station_error_event(const IEventBus::event& /*ev*/, IEventBus::event_data_t& /*data*/)
{
	this->update_action_menu_item(this->radiotray_ng->get_state());
}


void AppindicatorGui::on_tags_event(const IEventBus::event& /*ev*/, IEventBus::event_data_t& data)
{
	this->update_status_menu_item(this->radiotray_ng->get_state());

	if (data.count(TAG_BITRATE) || data.count(TAG_CODEC))
	{
		this->update_volume_menu_item();
	}
}


void AppindicatorGui::on_station_menu_item(GtkWidget* /*widget*/, gpointer data)
{
	auto mid = static_cast<menu_item_data*>(data);

	mid->gui->radiotray_ng->play(mid->group, mid->station);
}


void AppindicatorGui::on_indicator_scrolled(GtkWidget* /*widget*/, gint /*delta*/, GdkScrollDirection direction, gpointer data)
{
	AppindicatorGui* app = static_cast<AppindicatorGui*>(data);

	switch(direction)
	{
		case GDK_SCROLL_UP:
		case GDK_SCROLL_RIGHT:
			app->radiotray_ng->volume_up();
			break;

		case GDK_SCROLL_DOWN:
		case GDK_SCROLL_LEFT:
			app->radiotray_ng->volume_down();
			break;

		default:
			break;
	}
}


void AppindicatorGui::on_action_menu_item(GtkWidget* /*widget*/, gpointer data)
{
	AppindicatorGui* app = static_cast<AppindicatorGui*>(data);

	std::string state{app->radiotray_ng->get_state()};

	if (state == STATE_STOPPED)
	{
		app->radiotray_ng->play();

		app->update_action_menu_item(STATE_PLAYING);

		return;
	}

	if (state == STATE_PLAYING || state == STATE_BUFFERING)
	{
		app->radiotray_ng->stop();

		app->update_action_menu_item(STATE_STOPPED);

		return;
	}
}


void AppindicatorGui::build_bookmarks_menu_item()
{
	this->add_separator(this->menu);

	for(size_t i = 0; i < this->bookmarks->size(); ++i)
	{
		const std::string& group = (*this->bookmarks)[i].group;

		// skip root as it's not a submenu
		if (group != ROOT_BOOKMARK_GROUP)
		{
			GtkWidget* menu_items = gtk_menu_item_new_with_label(group.c_str());
			gtk_menu_append(GTK_MENU(this->menu), menu_items);

			GtkWidget* sub_menu_items = gtk_menu_new();
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_items), sub_menu_items);

			for(const IBookmarks::station_data_t& s : (*this->bookmarks)[i].stations)
			{
				GtkWidget* sub_menu_item = gtk_menu_item_new_with_label(s.name.c_str());
				gtk_menu_append(GTK_MENU(sub_menu_items), sub_menu_item);

				auto cb_data = new menu_item_data{this->shared_from_this(), group, s.name};

				g_signal_connect_data(GTK_OBJECT(sub_menu_item), "activate",
					GTK_SIGNAL_FUNC(on_station_menu_item), cb_data, GClosureNotify(&menu_item_data::free_cb_data), GConnectFlags(0));

				gtk_widget_show(sub_menu_item);
			}

			gtk_widget_show(menu_items);
		}
	}

	// append root items
	std::vector<IBookmarks::station_data_t> root_stations;

	if (this->bookmarks->get_group_stations(ROOT_BOOKMARK_GROUP, root_stations))
	{
		for(const IBookmarks::station_data_t& s : root_stations)
		{
			GtkWidget* menu_item = gtk_menu_item_new_with_label(s.name.c_str());
			gtk_menu_append(GTK_MENU(this->menu), menu_item);

			menu_item_data* cb_data = new menu_item_data{this->shared_from_this(), ROOT_BOOKMARK_GROUP, s.name};

			g_signal_connect_data(GTK_OBJECT(menu_item), "activate",
				GTK_SIGNAL_FUNC(on_station_menu_item), cb_data, GClosureNotify(&menu_item_data::free_cb_data), GConnectFlags(0));

			gtk_widget_show(menu_item);
		}
	}
}


void AppindicatorGui::add_separator(GtkWidget* menu)
{
	if (!this->config->get_bool(COMPACT_MENU_KEY, DEFAULT_COMPACT_MENU_VALUE))
	{
		GtkWidget* menu_items = gtk_menu_item_new();
		gtk_menu_append(GTK_MENU(menu), menu_items);
		gtk_widget_show(menu_items);
	}
}


void AppindicatorGui::update_volume_menu_item()
{
	std::string volume_info{std::string("Volume: ") + this->radiotray_ng->get_volume() + std::string("%")};

	if (this->config->get_bool(TAG_INFO_VERBOSE_KEY, false))
	{
		std::string bitrate = this->radiotray_ng->get_bitrate();
		if (!bitrate.empty())
		{
			std::string bitrate_info{std::string(", Bitrate: ") + bitrate};

			volume_info += bitrate_info;
		}

		std::string codec = this->radiotray_ng->get_codec();
		if (!codec.empty())
		{
			std::string codec_info{std::string("\nCodec: ") + codec};

			volume_info += codec_info;
		}
	}

	gtk_menu_item_set_label(GTK_MENU_ITEM(this->volume_menu_item), volume_info.c_str());
	gtk_widget_set_sensitive(this->volume_menu_item, FALSE);
}


void AppindicatorGui::update_action_menu_item(const std::string& state)
{
	std::string action_text;

	if (state == STATE_STOPPED)
	{
		action_text = "Turn On \"" + this->radiotray_ng->get_station() + "\"";
		gtk_menu_item_set_label(GTK_MENU_ITEM(this->action_menu_item), action_text.c_str());

		// Is the station still in our bookmarks?
		gtk_widget_set_sensitive(this->action_menu_item,
			this->bookmarks->station_exists(this->radiotray_ng->get_group(), this->radiotray_ng->get_station()));

		return;
	}

	if (state == STATE_PLAYING || state == STATE_BUFFERING)
	{
		action_text = "Turn Off \"" + this->radiotray_ng->get_station() + "\"";
		gtk_menu_item_set_label(GTK_MENU_ITEM(this->action_menu_item), action_text.c_str());
		gtk_widget_set_sensitive(this->action_menu_item, TRUE);
		return;
	}
}


void AppindicatorGui::update_status_menu_item(const std::string& state)
{
	if (state == STATE_PLAYING)
	{
		std::string title{this->radiotray_ng->get_title()};
		std::string artist{this->radiotray_ng->get_artist()};
		std::string status_text;

		// at least a title will be there...
		if (title.empty())
		{
			status_text = std::string("Playing");
		}
		else
		{
			status_text = radiotray_ng::word_wrap(title, 40);

			if (!artist.empty())
			{
				status_text += "\n" + radiotray_ng::word_wrap(artist, 40);
			}
		}

		gtk_menu_item_set_label(GTK_MENU_ITEM(this->status_menu_item), status_text.c_str());
		return;
	}

	if (state == STATE_STOPPED)
	{
		gtk_menu_item_set_label(GTK_MENU_ITEM(this->status_menu_item), std::string("Stopped").c_str());
		return;
	}
}


void AppindicatorGui::build_status_menu_item()
{
	this->add_separator(this->menu);

	this->status_menu_item = gtk_menu_item_new_with_label("status");
	gtk_menu_append(GTK_MENU(this->menu), this->status_menu_item);
	gtk_widget_set_sensitive(this->status_menu_item, FALSE);
	gtk_widget_show(this->status_menu_item);

	this->update_status_menu_item(STATE_STOPPED);
}


void AppindicatorGui::build_action_menu_item()
{
	this->action_menu_item = gtk_menu_item_new_with_label("action");
	gtk_menu_append(GTK_MENU(this->menu), this->action_menu_item);
	gtk_widget_show(this->action_menu_item);

	g_signal_connect(G_OBJECT(this->action_menu_item), "activate", G_CALLBACK(on_action_menu_item), gpointer(this));

	this->update_action_menu_item(STATE_STOPPED);
}


void AppindicatorGui::build_volume_menu_item()
{
	this->add_separator(this->menu);

	this->volume_menu_item = gtk_menu_item_new_with_label("volume");
	gtk_menu_append(GTK_MENU(this->menu), this->volume_menu_item);
	gtk_widget_show(this->volume_menu_item);

	this->update_volume_menu_item();
}


void AppindicatorGui::build_preferences_menu()
{
	this->add_separator(this->menu);

	GtkWidget* menu_items = gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES, nullptr);
	gtk_menu_append(GTK_MENU(this->menu), menu_items);
	gtk_widget_show(menu_items);

	GtkWidget* sub_menu_items = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_items), sub_menu_items);

	GtkWidget* sub_menu_item = gtk_menu_item_new_with_label("Reload Bookmarks");
	gtk_menu_append(GTK_MENU(sub_menu_items), sub_menu_item);
	g_signal_connect(GTK_OBJECT(sub_menu_item), "activate", G_CALLBACK(on_reload_bookmarks_menu_item), gpointer(this));
	gtk_widget_show(sub_menu_item);

	this->sleep_timer_menu_item = (GtkCheckMenuItem*)gtk_check_menu_item_new_with_label("Sleep Timer");

	// toggle before we hook up callback as a reload loses this state...
	gtk_check_menu_item_set_state(this->sleep_timer_menu_item, this->sleep_timer_id);

	gtk_menu_append(GTK_MENU(sub_menu_items), (GtkWidget*)this->sleep_timer_menu_item);
	g_signal_connect(GTK_OBJECT((GtkWidget*)this->sleep_timer_menu_item), "activate", G_CALLBACK(on_sleep_timer_menu_item), gpointer(this));
	gtk_widget_show((GtkWidget*)this->sleep_timer_menu_item);
}


void AppindicatorGui::build_menu()
{
	this->menu = gtk_menu_new();

	// action (play/stop)
	this->build_action_menu_item();

	// status (tags)
	this->build_status_menu_item();

	// volume etc.
	this->build_volume_menu_item();

	// bookmarks
	this->build_bookmarks_menu_item();

	// preferences
	this->build_preferences_menu();

	// about etc.
	this->add_separator(this->menu);

	auto menu_items = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, nullptr);
	gtk_menu_append(GTK_MENU (this->menu), menu_items);
	gtk_signal_connect(GTK_OBJECT(menu_items), "activate", GTK_SIGNAL_FUNC(&AppindicatorGui::on_about_menu_item), this);
	gtk_widget_show(menu_items);

	menu_items = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, nullptr);
	gtk_menu_append(GTK_MENU (this->menu), menu_items);
	gtk_signal_connect(GTK_OBJECT(menu_items), "activate", GTK_SIGNAL_FUNC(gtk_main_quit), nullptr);
	gtk_widget_show(menu_items);

	app_indicator_set_menu(appindicator, GTK_MENU(this->menu));
}


gboolean AppindicatorGui::on_timer_event(gpointer data)
{
	AppindicatorGui* app = static_cast<AppindicatorGui*>(data);

	if (app->radiotray_ng->get_state() == STATE_PLAYING)
	{
		app->radiotray_ng->stop();

		app->update_action_menu_item(STATE_STOPPED);
	}

	gtk_check_menu_item_set_state(app->sleep_timer_menu_item, FALSE);

	app->event_bus->publish_only(IEventBus::event::message, MESSAGE_KEY, "Sleep timer expired");

	return FALSE;
}


void AppindicatorGui::on_sleep_timer_menu_item(GtkWidget* /*widget*/, gpointer data)
{
	AppindicatorGui* app = static_cast<AppindicatorGui*>(data);

	// toggle...
	if (app->sleep_timer_id)
	{
		g_source_remove(app->sleep_timer_id);

		app->sleep_timer_id = 0;

		app->event_bus->publish_only(IEventBus::event::message, MESSAGE_KEY, "Sleep timer stopped");
	}
	else
	{
		app->event_bus->publish_only(IEventBus::event::message, MESSAGE_KEY,
									 std::to_string(app->config->get_uint32(SLEEP_TIMER_KEY, DEFAULT_SLEEP_TIMER_VALUE)) + " minute sleep timer started");

		app->sleep_timer_id = g_timeout_add(app->config->get_uint32(SLEEP_TIMER_KEY, DEFAULT_SLEEP_TIMER_VALUE) * 60000, on_timer_event, data);
	}
}


void AppindicatorGui::on_reload_bookmarks_menu_item(GtkWidget* /*widget*/, gpointer data)
{
	AppindicatorGui* app = static_cast<AppindicatorGui*>(data);

	if (app->radiotray_ng->reload_bookmarks())
	{
		gtk_widget_destroy(app->menu);

		app->build_menu();

		const std::string state{app->radiotray_ng->get_state()};

		app->update_action_menu_item(state);
		app->update_status_menu_item(state);
	}
}


void AppindicatorGui::on_about_menu_item(GtkWidget* /*widget*/, gpointer data)
{
	AppindicatorGui* app = static_cast<AppindicatorGui*>(data);

	std::string license{"missing"};
	std::string license_file{app->resource_path + "/COPYING"};

	if (fs::exists(license_file))
	{
		radiotray_ng::load_string_file(license_file, license);
	}

	std::string copyright{RTNG_GIT_VERSION};
	copyright += "\n\n" APP_COPYRIGHT;

	gtk_show_about_dialog(nullptr
		, "program-name", APP_NAME_DISPLAY
		, "license", license.c_str()
		, "copyright", copyright.c_str()
		, "website", APP_WEBSITE
		, nullptr);
}


void AppindicatorGui::stop()
{
	gtk_main_quit();
}


void AppindicatorGui::gtk_loop(int argc, char* argv[])
{
	radiotray_ng::g_threading_helper gth;

	gtk_init(&argc, &argv);

	this->appindicator = app_indicator_new(APP_NAME, RADIOTRAY_NG_ICON_OFF, APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

	app_indicator_set_attention_icon(this->appindicator, RADIOTRAY_NG_ICON_OFF);
	app_indicator_set_status(this->appindicator, APP_INDICATOR_STATUS_ACTIVE);

	g_signal_connect(G_OBJECT(this->appindicator), "scroll-event", G_CALLBACK(on_indicator_scrolled), gpointer(this));

	// get default icon (todo: figure this out at runtime)
	this->resource_path = RTNG_DEFAULT_INSTALL_DIR;

	gtk_window_set_default_icon_from_file((this->resource_path + RADIOTRAY_NG_LOGO_ICON).c_str(), nullptr);

	this->build_menu();

	gtk_main();

	gtk_widget_destroy(this->menu);

	g_object_unref(G_OBJECT(this->appindicator));
}


void AppindicatorGui::run(int argc, char* argv[])
{
	this->gtk_loop(argc, argv);
}
