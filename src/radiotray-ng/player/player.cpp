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

#include <radiotray-ng/player/player.hpp>
#include <rtng_user_agent.hpp>


Player::Player(std::shared_ptr<IConfig> config, std::shared_ptr<IEventBus> event_bus)
	: pipeline(nullptr)
	, souphttpsrc(nullptr)
	, clock(nullptr)
	, clock_id(nullptr)
	, event_bus(std::move(event_bus))
	, config(std::move(config))
	, gst_bus(nullptr)
{
	LOG(info) << "starting gstreamer";

	this->gst_start();
}


Player::~Player()
{
	LOG(info) << "stopping gstreamer";

	this->gst_stop();
}


bool Player::play_next()
{
	this->stop();

	if (!this->current_playlist.empty())
	{
		LOG(debug) << "uri: " << this->current_playlist.front();

		g_object_set(this->pipeline, "uri", this->current_playlist.front().c_str(), NULL);

		this->current_playlist.erase(this->current_playlist.begin());

		const uint32_t buffer_size  = this->config->get_uint32(BUFFER_SIZE_KEY, DEFAULT_BUFFER_SIZE_VALUE);
		const uint32_t buffer_duration = this->config->get_uint32(BUFFER_DURATION_KEY, DEFAULT_BUFFER_DURATION_VALUE);

		g_object_set(G_OBJECT(this->pipeline), "buffer-size", buffer_size * buffer_duration, NULL);
		g_object_set(G_OBJECT(this->pipeline), "buffer-duration", buffer_duration * GST_SECOND, NULL);

		LOG(debug) << BUFFER_SIZE_KEY << "=" << std::to_string(buffer_size * buffer_duration) << ", " << BUFFER_DURATION_KEY << "=" << buffer_duration;

		this->volume(this->config->get_uint32(VOLUME_LEVEL_KEY, DEFAULT_VOLUME_LEVEL_VALUE));

		if (gst_element_set_state(this->pipeline, GST_STATE_PAUSED) == GST_STATE_CHANGE_FAILURE)
		{
			LOG(error) << "Failed to set pipeline to: GST_STATE_PAUSED";
			return false;
		}

		return true;
	}

	LOG(info) << "playlist is empty";

	return false;
}


bool Player::play(const playlist_t& playlist)
{
	if (this->gst_bus == nullptr)
	{
		LOG(error) << "gstreamer not ready";
		return false;
	}

	if (playlist.empty())
	{
		LOG(error) << "playlist is empty";
		return false;
	}

	this->current_playlist = playlist;

	if (!this->play_next())
	{
		this->event_bus->publish_only(IEventBus::event::station_error, ERROR_KEY,
			"Unable to set the pipeline to the playing state!");

		return false;
	}

	return true;
}


void Player::stop()
{
	GstState state;
	gst_element_get_state(GST_ELEMENT(this->pipeline), &state, nullptr, GST_CLOCK_TIME_NONE);

	if (state != GST_STATE_NULL)
	{
		gst_element_set_state(GST_ELEMENT(this->pipeline), GST_STATE_NULL);

		// Clear the flag just in case since stopping emits a pause which will register for a
		// buffering timeout.
		this->buffering = false;

		// abort outstanding callback...
		if (this->clock_id)
		{
			LOG(info) << "canceling outstanding clock request";
			gst_clock_id_unschedule(this->clock_id);
			gst_clock_id_unref(this->clock_id);
			this->clock_id = nullptr;
		}

		this->event_bus->publish_only(IEventBus::event::state_changed, STATE_KEY, STATE_STOPPED);
	}
}


void Player::volume(const uint32_t percent)
{
	gdouble volume{percent/100.0};

	g_object_set(G_OBJECT(this->pipeline), "volume", volume, NULL);

	this->event_bus->publish_only(IEventBus::event::volume_changed, VOLUME_LEVEL_KEY, std::to_string(percent));

	this->config->set_uint32(VOLUME_LEVEL_KEY, percent);
}


void Player::mute()
{
    g_object_set(G_OBJECT(this->pipeline), "mute", TRUE, NULL);
}


void Player::unmute()
{
    g_object_set(G_OBJECT(this->pipeline), "mute", FALSE, NULL);
}


bool Player::is_muted()
{
    gboolean muted{};

    g_object_get(G_OBJECT(this->pipeline), "mute", &muted, NULL);

    return !!muted;
}


void Player::notify_source_cb(GObject* obj, GParamSpec* /*param*/, gpointer /*user_data*/)
{
	// set our user-agent...
	if (g_object_class_find_property(G_OBJECT_GET_CLASS(obj), "source"))
	{
		GObject* source_element;
		g_object_get(obj, "source", &source_element, NULL);

		// todo: detect distro at runtime instead of what we were compiled on...
		if (g_object_class_find_property(G_OBJECT_GET_CLASS(source_element), "user-agent"))
		{
			g_object_set(source_element, "user-agent", RTNG_USER_AGENT, NULL);
		}

		g_object_unref(source_element);
	}
}


gboolean Player::timer_cb(GstClock* /*clock*/, GstClockTime /*time*/, GstClockID /*id*/, gpointer user_data)
{
	auto player{static_cast<Player*>(user_data)};

	gst_clock_id_unref(player->clock_id);
	player->clock_id = nullptr;

	if (player->buffering)
	{
		LOG(error) << "buffering timeout, restarting stream...";

		// kick it... (Should we play next?)
		gst_element_set_state(player->pipeline, GST_STATE_NULL);
		gst_element_set_state(player->souphttpsrc, GST_STATE_NULL);
		gst_element_set_state(player->pipeline, GST_STATE_PAUSED);
	}

	return TRUE;
}


gboolean Player::handle_messages_cb(GstBus* /*bus*/, GstMessage* message, gpointer user_data)
{
	auto player{static_cast<Player*>(user_data)};

	switch (GST_MESSAGE_TYPE (message))
	{
		case GST_MESSAGE_ERROR:
		{
			GError* err;
			gchar* debug_info;
			gst_message_parse_error(message, &err, &debug_info);

			LOG(error) << "error received from element " << GST_OBJECT_NAME(message->src) << ": " << err->message
				<< " , " << int(err->domain) << ":" << int(err->code);

			LOG(error) << "debugging information: " << ((debug_info) ? debug_info : "none");

			gst_element_set_state(player->pipeline, GST_STATE_NULL);
			gst_element_set_state(player->souphttpsrc, GST_STATE_NULL);

			if (err->domain == GST_RESOURCE_ERROR && err->code == GST_RESOURCE_ERROR_SEEK)
			{
				LOG(error) << "dropped connection, restarting stream...";

				gst_element_set_state(player->pipeline, GST_STATE_PAUSED);

				player->buffering = true;
			}
			else
			{
				if (!player->play_next())
				{
					LOG(debug) << "setting state to: " << STATE_STOPPED;

					player->event_bus->publish_only(IEventBus::event::state_changed, STATE_KEY, STATE_STOPPED);
					player->event_bus->publish_only(IEventBus::event::station_error, ERROR_KEY, err->message);
				}
			}

			g_clear_error(&err);
			g_free(debug_info);
		}
		break;

		case GST_MESSAGE_EOS:
		{
			LOG(debug) << "end-of-stream reached";

			if (!player->play_next())
			{
				LOG(debug) << "setting state to: " << STATE_STOPPED;

				player->event_bus->publish_only(IEventBus::event::state_changed, STATE_KEY, STATE_STOPPED);
			}
		}
		break;

		case GST_MESSAGE_BUFFERING:
		{
			gint percent;
			gst_message_parse_buffering(message, &percent);

			// are we done?
			if (percent == 100)
			{
				player->buffering = false;
				LOG(debug) << "stopped buffering, setting state to: GST_STATE_PLAYING";
				gst_element_set_state(GST_ELEMENT(player->pipeline), GST_STATE_PLAYING);
			}
			else
			{
				// buffering busy?
				if (!player->buffering)
				{
					// we were not buffering but PLAYING, then pause the pipeline
					LOG(debug) << "started buffering, setting state to: GST_STATE_PAUSED";
					gst_element_set_state(GST_ELEMENT(player->pipeline), GST_STATE_PAUSED);
				}

				player->buffering = true;
			}
		}
		break;

		case GST_MESSAGE_TAG:
		{
			if (player->event_bus)
			{
				IEventBus::event_data_t notify_data;

				GstTagList* tags;
				gst_message_parse_tag(message, &tags);
				gst_tag_list_foreach(tags, static_cast<GstTagForeachFunc>(&Player::for_each_tag_cb), &notify_data);
				gst_tag_list_free(tags);

				player->event_bus->publish(IEventBus::event::tags_changed, notify_data);
			}
		}
		break;

		case GST_MESSAGE_STATE_CHANGED:
		{
			GstState old_state;
			GstState new_state;
			gst_message_parse_state_changed(message, &old_state, &new_state, nullptr);

			if (GST_MESSAGE_SRC(message) == GST_OBJECT(player->pipeline))
			{
				if (new_state == GST_STATE_PLAYING)
				{
					player->event_bus->publish_only(IEventBus::event::state_changed, STATE_KEY, STATE_PLAYING);
				}
				else if (new_state == GST_STATE_PAUSED)
				{
					// abort outstanding callback...
					if (player->clock_id)
					{
						LOG(info) << "canceling outstanding clock request";

						gst_clock_id_unschedule(player->clock_id);
						gst_clock_id_unref(player->clock_id);
						player->clock_id = nullptr;
					}

					if (!player->buffering)
					{
						break;
					}

					player->event_bus->publish_only(IEventBus::event::state_changed, STATE_KEY, STATE_BUFFERING);

					// start timer to abort if buffering stalls...
					player->clock_id = gst_clock_new_single_shot_id(player->clock, gst_clock_get_time(player->clock) + (10 * GST_SECOND));
					gst_clock_id_wait_async(player->clock_id, static_cast<GstClockCallback>(&Player::timer_cb), player, nullptr);
				}
			}
		}
		break;

		default:
			break;
	}

	return TRUE;
}


void Player::for_each_tag_cb(const GstTagList* list, const gchar* tag, gpointer user_data)
{
	auto& event_data = *static_cast<IEventBus::event_data_t*>(user_data);

	const guint count = gst_tag_list_get_tag_size(list, tag);

	for (guint i = 0; i < count; i++)
	{
		gchar* str{nullptr};

		if (gst_tag_get_type(tag) == G_TYPE_STRING)
		{
			if (!gst_tag_list_get_string_index(list, tag, i, &str))
			{
				LOG(error) << "gst_tag_list_get_string_index failed for tag: " << gst_tag_get_nick(tag);
				g_free(str);
				continue;
			}
		}
		else
		{
			str = g_strdup_value_contents(gst_tag_list_get_value_index(list, tag, i));
		}

		// todo: for now ignore anything that looks encoded...
		if (std::string(str).find("<?xml") == std::string::npos)
		{
			event_data[gst_tag_get_nick(tag)] = str;
		}
		else
		{
			LOG(debug) << "ignoring encoded tag: " << gst_tag_get_nick(tag) << " : " << str;
		}

		g_free(str);
	}
}


void Player::gst_start()
{
	gst_init(nullptr, nullptr);

	if ((this->pipeline = gst_element_factory_make("playbin3", "player")) == nullptr)
	{
		LOG(error) << "could not create playbin3 element, falling back to playbin";

		if ((this->pipeline = gst_element_factory_make("playbin", "player")) == nullptr)
		{
			LOG(error) << "could not create playbin element";

			gst_deinit();
			return;
		}

		LOG(warning) << "m3u8 support will not be available using playbin";
	}

	if ((this->souphttpsrc = gst_element_factory_make("souphttpsrc", "source")) == nullptr)
	{
		LOG(error) << "could not create souphttpsrc element";

		gst_object_unref(this->pipeline);
		gst_deinit();
		return;
	}

	GstElement* audio_sink;
	if ((audio_sink = gst_element_factory_make("autoaudiosink", "audio-sink")) == nullptr)
	{
		LOG(error) << "could not create autoaudiosink element";

		gst_object_unref(this->pipeline);
		gst_object_unref(this->souphttpsrc);
		gst_deinit();
		return;
	}

	g_object_set(this->pipeline, "audio-sink", audio_sink, NULL);

	// get clock for buffering timeouts...
	this->clock = gst_pipeline_get_clock(GST_PIPELINE(this->pipeline));

	// setup callbacks...
	this->gst_bus = gst_element_get_bus(this->pipeline);
	gst_bus_add_watch(this->gst_bus, static_cast<GstBusFunc>(&Player::handle_messages_cb), this);

	g_signal_connect(G_OBJECT(this->pipeline), "notify::source",  G_CALLBACK(&Player::notify_source_cb), this);
}


void Player::gst_stop()
{
	if (this->gst_bus)
	{
		gst_object_unref(this->gst_bus);

		gst_element_set_state(this->pipeline, GST_STATE_NULL);
		gst_object_unref(this->pipeline);

		gst_element_set_state(this->souphttpsrc, GST_STATE_NULL);
		gst_object_unref(this->souphttpsrc);

		gst_object_unref(G_OBJECT(this->clock));

		gst_deinit();
	}
}
