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

#include "radiotray_ng.hpp"
#include <radiotray-ng/common.hpp>
#include <radiotray-ng/i_config.hpp>
#include <radiotray-ng/i_player.hpp>
#include <radiotray-ng/playlist/playlist_downloader.hpp>
#include <json/json.h>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <cmath>


RadiotrayNG::RadiotrayNG(std::shared_ptr<IConfig> config, std::shared_ptr<IBookmarks> bookmarks,
	std::shared_ptr<IPlayer> player, std::shared_ptr<IEventBus> event_bus)
	: config(std::move(config))
	, bookmarks(std::move(bookmarks))
	, player(std::move(player))
	, event_bus(std::move(event_bus))
	, state(STATE_STOPPED)
	, current_station_index(-1)
	, play_url_group("PLAY-URL:" + boost::uuids::to_string(boost::uuids::random_generator()()))
{
	this->notification_image = radiotray_ng::word_expand(this->config->get_string(RADIOTRAY_NG_NOTIFICATION_KEY, DEFAULT_RADIOTRAY_NG_NOTIFICATION_VALUE));

	this->set_volume(this->config->get_string(VOLUME_LEVEL_KEY, std::to_string(DEFAULT_VOLUME_LEVEL_VALUE)));

	this->set_station(this->config->get_string(LAST_STATION_GROUP_KEY, ""), this->config->get_string(LAST_STATION_KEY, ""), this->config->get_bool(LAST_STATION_NOTIFICATION_KEY, true));

	this->register_handlers();
}


RadiotrayNG::~RadiotrayNG()
{
	this->config->save();
}


void RadiotrayNG::stop()
{
	LOG(info) << "stopping player";

	this->playing_notification_sent = false;

	if (this->config->get_bool(NOTIFICATION_KEY, DEFAULT_NOTIFICATION_VALUE) &&
		this->config->get_bool(NOTIFICATION_VERBOSE_KEY, DEFAULT_NOTIFICATION_VERBOSE_VALUE))
	{
		this->notification.notify("Stopped", this->get_station(), this->notification_image);
	}

	this->player->stop();

	this->clear_tags();
}


void RadiotrayNG::clear_tags()
{
	IEventBus::event_data_t data;

	data[TAG_ARTIST];
	data[TAG_TITLE];
	data[TAG_BITRATE];
	data[TAG_CODEC];

	this->event_bus->publish(IEventBus::event::tags_changed, data);
}


std::string RadiotrayNG::get_title()
{
	return this->title;
}


std::string RadiotrayNG::get_artist()
{
	return this->artist;
}


std::string RadiotrayNG::get_group()
{
	return this->group;
}


std::string RadiotrayNG::get_station()
{
	return this->station;
}


std::string RadiotrayNG::get_state()
{
	return this->state;
}


std::string RadiotrayNG::get_bitrate()
{
	return this->bitrate;
}


std::string RadiotrayNG::get_codec()
{
	return this->codec;
}


std::string RadiotrayNG::get_volume()
{
	return this->volume;
}


std::string RadiotrayNG::get_bookmarks()
{
	// hack... filter out play_url group...
	std::vector<IBookmarks::station_data_t> play_url_group_data;

	if (this->bookmarks->get_group_stations(this->play_url_group, play_url_group_data))
	{
		this->bookmarks->remove_group(this->play_url_group);
		const std::string dump{this->bookmarks->dump()};

		// *sigh* now add it back...
		if (!play_url_group_data.empty())
		{
			this->bookmarks->add_group(this->play_url_group, "");
			this->bookmarks->add_station(this->play_url_group, "", play_url_group_data[0].url, "", true);
		}
		return dump;
	}

	return this->bookmarks->dump();
}


std::string RadiotrayNG::get_player_state()
{
	Json::Value value;

	value[DBUS_MSG_STATE_KEY]   = this->state;
	value[DBUS_MSG_VOLUME_KEY]  = this->volume;
	value[DBUS_MSG_TITLE_KEY]   = this->title;
	value[DBUS_MSG_ARTIST_KEY]  = this->artist;
	value[DBUS_MSG_STATION_KEY] = this->station;
	value[DBUS_MSG_URL_KEY]     = this->url;
	value[DBUS_MSG_GROUP_KEY]   = (this->group != this->play_url_group) ? this->group : "";
	value[DBUS_MSG_CODEC_KEY]   = this->codec;
	value[DBUS_MSG_BITRATE_KEY] = this->bitrate;
	value[DBUS_MSG_MUTE_KEY]    = this->player->is_muted();
	value[DBUS_MSG_IMAGE_KEY]   = radiotray_ng::word_expand(this->notification_image);

	return value.toStyledString();
}


std::string RadiotrayNG::get_config()
{
	return this->config->dump();
}


void RadiotrayNG::set_title(const std::string& title)
{
	this->title = title;
}


void RadiotrayNG::set_artist(const std::string& artist)
{
	this->artist = artist;
}


void RadiotrayNG::set_station(const std::string& group, const std::string& station, bool notifications)
{
	this->group = group;
	this->station = station;
	this->station_notifications = notifications;

	// This allows us to start playing the first station in a group if we've reloaded
	// the bookmarks file and it's no longer within it.
	this->current_station_index = -1;

	if (this->bookmarks->get_group_stations(group, this->current_group_stations))
	{
		for(size_t i=0; i < this->current_group_stations.size(); ++i)
		{
			if (this->current_group_stations[i].name == station)
			{
				this->station_notifications = this->current_group_stations[i].notifications;
				this->current_station_index = i;
				break;
			}
		}
	}
}


void RadiotrayNG::set_state(const std::string& state)
{
	this->state = state;
}


void RadiotrayNG::set_bitrate(const std::string& bitrate)
{
	this->bitrate = bitrate;
}


void RadiotrayNG::set_codec(const std::string& codec)
{
	this->codec = codec;
}


void RadiotrayNG::set_volume(const std::string& volume)
{
	this->volume = volume;
}


void RadiotrayNG::set_volume_msg(const uint32_t volume)
{
	this->set_and_save_volume(std::min(volume,
		this->config->get_uint32(VOLUME_MAX_LEVEL_KEY, DEFAULT_VOLUME_LEVEL_MAX_VALUE)));

	this->display_volume_level();
}


void RadiotrayNG::next_station_msg()
{
	if (!this->current_group_stations.empty())
	{
		if (this->current_station_index < int(this->current_group_stations.size()-1))
		{
			this->play(this->group, this->current_group_stations[++this->current_station_index].name);
		}
	}
}


void RadiotrayNG::previous_station_msg()
{
	if (!this->current_group_stations.empty())
	{
		if (this->current_station_index > 0)
		{
			this->play(this->group, this->current_group_stations[--this->current_station_index].name);
		}
	}
}


void RadiotrayNG::on_tags_changed_event_processing(const IEventBus::event& /*ev*/, IEventBus::event_data_t& data)
{
	if (this->config->get_bool(SPLIT_TITLE_KEY, DEFAULT_SPLIT_TITLE_VALUE))
	{
		if (data.count(TAG_TITLE) && !data[TAG_TITLE].empty())
		{
			// very simplistic, but works "most" of the time...
			std::string::size_type pos = data[TAG_TITLE].find(" - ");

			if (pos != std::string::npos)
			{
				radiotray_ng::trim(data[TAG_ARTIST] = data[TAG_TITLE].substr(0, pos));
				radiotray_ng::trim(data[TAG_TITLE]  = data[TAG_TITLE].substr(pos+3));

				// protect against bad tagging...
				if (data[TAG_ARTIST].empty())
				{
					data.erase(TAG_ARTIST);
				}

				if (data[TAG_TITLE].empty())
				{
					data.erase(TAG_TITLE);
				}
				else
				{
					// extra parsing for text="title"...
					if (this->config->get_bool(IHR_TITLE_KEY, DEFAULT_IHR_TITLE_KEY_VALUE))
					{
						pos = data[TAG_TITLE].find(R"(text=")");

						if (pos != std::string::npos)
						{
							auto end_pos = data[TAG_TITLE].find('"', pos+6);

							if (end_pos != std::string::npos)
							{
								data[TAG_TITLE] = data[TAG_TITLE].substr(pos+6, end_pos - (pos+6));
							}
						}
					}
				}
			}
			else
			{
				// sometimes tags are empty, especially with IHR...
				if (radiotray_ng::trim(data[TAG_TITLE]) == "-")
				{
					data.erase(TAG_TITLE);
				}
			}
		}
	}

	// we "always" get a title...
	if (data.count(TAG_TITLE))
	{
		this->title = data[TAG_TITLE];

		// make sure last entry doesn't stick around if tags change and artist is not available
		this->artist.clear();
	}

	if (data.count(TAG_ARTIST))
	{
		this->artist = data[TAG_ARTIST];
	}

	if (data.count(TAG_BITRATE))
	{
		this->bitrate.clear();

		try
		{
			if (!data[TAG_BITRATE].empty())
			{
				this->bitrate = std::to_string(lround(std::stoul(data[TAG_BITRATE])/1000.0)) + " kb/s";
			}
		}
		catch(std::invalid_argument& ex)
		{
			LOG(warning) << "invalid bitrate format, ignoring tag (" << ex.what() << ")";
		}
	}

	if (data.count(TAG_CODEC))
	{
		this->codec = data[TAG_CODEC];
	}
}


void RadiotrayNG::on_state_changed_event(const IEventBus::event& /*ev*/, IEventBus::event_data_t& data)
{
	this->last_notification = {"",""};

	this->state = data[STATE_KEY];

	if (this->config->get_bool(NOTIFICATION_KEY, DEFAULT_NOTIFICATION_VALUE))
	{
		if (data[STATE_KEY] == STATE_PLAYING)
		{
			if (!this->playing_notification_sent)
			{
				if (this->config->get_bool(NOTIFICATION_KEY, DEFAULT_NOTIFICATION_VALUE))
				{
					this->notification.notify(this->get_station(), APP_NAME_DISPLAY, this->notification_image);
				}

				this->playing_notification_sent = true;
			}

			return;
		}

		if (this->config->get_bool(NOTIFICATION_VERBOSE_KEY, DEFAULT_NOTIFICATION_VERBOSE_VALUE))
		{
			if (data[STATE_KEY] == STATE_CONNECTING)
			{
				if (this->config->get_bool(NOTIFICATION_KEY, DEFAULT_NOTIFICATION_VALUE))
				{
					this->notification.notify("Connecting", APP_NAME_DISPLAY, this->notification_image);
				}
				return;
			}

			if (data[STATE_KEY] == STATE_BUFFERING)
			{
				const auto station = this->get_station();
				if (this->config->get_bool(NOTIFICATION_KEY, DEFAULT_NOTIFICATION_VALUE))
				{
					this->notification.notify("Buffering", (station.empty()) ? APP_NAME_DISPLAY : station, this->notification_image);
				}
				return;
			}
		}
	}
}


void RadiotrayNG::on_station_error_event(const IEventBus::event& /*ev*/, IEventBus::event_data_t& data)
{
	// always show errors
	this->notification.notify("Error", data[ERROR_KEY], this->notification_image);
}


void RadiotrayNG::on_message_event(const IEventBus::event& /*ev*/, IEventBus::event_data_t &data)
{
	if (this->config->get_bool(NOTIFICATION_VERBOSE_KEY, DEFAULT_NOTIFICATION_VERBOSE_VALUE))
	{
		this->notification.notify(data[MESSAGE_KEY], APP_NAME_DISPLAY,
			radiotray_ng::word_expand(this->config->get_string(RADIOTRAY_NG_NOTIFICATION_KEY, DEFAULT_RADIOTRAY_NG_NOTIFICATION_VALUE)));
	}
}


void RadiotrayNG::on_tags_changed_event_notification(const IEventBus::event& /*ev*/, IEventBus::event_data_t& data)
{
	if (data.count(TAG_TITLE))
	{
		if (data.count(TAG_ARTIST))
		{
			// prevent flooding...
			if (this->last_notification.first != data[TAG_TITLE ] || this->last_notification.second != data[TAG_ARTIST])
			{
				this->last_notification.first = data[TAG_TITLE];
				this->last_notification.second = data[TAG_ARTIST];

				// guard against tag clearing...
				if (!this->last_notification.first.empty() || !this->last_notification.second.empty())
				{
					if (this->config->get_bool(NOTIFICATION_KEY, DEFAULT_NOTIFICATION_VALUE))
					{
						if (this->station_notifications)
						{
							this->notification.notify(this->last_notification.first, this->last_notification.second,
								this->notification_image);
						}
					}
				}
			}
		}
		else
		{
			// prevent flooding...
			if (this->last_notification.first != data[TAG_TITLE])
			{
				this->last_notification.first = data[TAG_TITLE];
				this->last_notification.second = this->station;

				if (this->config->get_bool(NOTIFICATION_KEY, DEFAULT_NOTIFICATION_VALUE))
				{
					if (this->station_notifications)
					{
						this->notification.notify(this->last_notification.first, this->last_notification.second,
							this->notification_image);
					}
				}
			}
		}
	}
}


void RadiotrayNG::play_url(const std::string& url)
{
	this->bookmarks->add_group(this->play_url_group, "");
	this->bookmarks->add_station(this->play_url_group, "", "", "", true);
	this->bookmarks->update_station(this->play_url_group, "", url, "", true);
	this->play(this->play_url_group, "");
}


void RadiotrayNG::play(const std::string& group, const std::string& station)
{
	if (this->state == STATE_PLAYING)
	{
		this->player->stop();
	}

	this->playing_notification_sent = false;

	playlist_t pls;
	IBookmarks::station_data_t std;

	this->clear_tags();

	if (bookmarks->get_station(group, station, std))
	{
		LOG(info) << "downloading: " << group << ", " << station << ", " << std.url;

		// replace image path & expand if necessary...
		if (!std.image.empty())
		{
			this->notification_image = std.image;
		}
		else
		{
			this->notification_image = radiotray_ng::word_expand(this->config->get_string(RADIOTRAY_NG_NOTIFICATION_KEY, DEFAULT_RADIOTRAY_NG_NOTIFICATION_VALUE));
		}

		this->event_bus->publish_only(IEventBus::event::state_changed, STATE_KEY, STATE_CONNECTING);

		if (PlaylistDownloader(this->config).download_playlist(std, pls))
		{
			if (group != this->play_url_group)
			{
				this->config->set_string(LAST_STATION_GROUP_KEY, group);
				this->config->set_string(LAST_STATION_KEY, std.name);
				this->config->set_bool(LAST_STATION_NOTIFICATION_KEY, std.notifications);
			}

			this->set_station(group, std.name, std.notifications);

			if (this->player->play(pls))
			{
				this->url = std.url;

				this->config->save();
				return;
			}
		}

		LOG(error) << "failed to download playlist: " << std.url;

		this->event_bus->publish_only(IEventBus::event::state_changed, STATE_KEY, STATE_STOPPED);
		this->event_bus->publish_only(IEventBus::event::station_error, ERROR_KEY, "Failed to download playlist");
	}
	else
	{
		LOG(error) << "failed to read bookmark: " << group << " : " << station;

		this->event_bus->publish_only(IEventBus::event::station_error, ERROR_KEY, "Station Error");
	}
}


void RadiotrayNG::play()
{
	this->play(this->get_group(), this->get_station());
}


void RadiotrayNG::volume_up()
{
	const uint32_t max_volume{this->config->get_uint32(VOLUME_MAX_LEVEL_KEY, DEFAULT_VOLUME_LEVEL_MAX_VALUE)};
	const uint32_t volume{this->config->get_uint32(VOLUME_LEVEL_KEY, DEFAULT_VOLUME_LEVEL_VALUE)};

	this->set_and_save_volume(std::min(volume + this->config->get_uint32(VOLUME_STEP_KEY, DEFAULT_VOLUME_STEP_VALUE), max_volume));
}


void RadiotrayNG::volume_up_msg()
{
	this->volume_up();

	this->display_volume_level();
}


void RadiotrayNG::volume_down()
{
	const uint32_t volume_step(this->config->get_uint32(VOLUME_STEP_KEY, DEFAULT_VOLUME_STEP_VALUE));
	const uint32_t volume(this->config->get_uint32(VOLUME_LEVEL_KEY, DEFAULT_VOLUME_LEVEL_VALUE));

	this->set_and_save_volume((volume > volume_step) ? (volume - volume_step) : 0);
}


void RadiotrayNG::volume_down_msg()
{
	this->volume_down();

	this->display_volume_level();
}


void RadiotrayNG::mute()
{
    std::string msg{"Volume: " + this->volume + "% "};

    if (!this->player->is_muted())
    {
        this->player->mute();

        msg += "(Mute)";
    }
    else
    {
        this->player->unmute();
    }

    if (this->config->get_bool(NOTIFICATION_KEY, DEFAULT_NOTIFICATION_VALUE))
    {
        this->notification.notify(msg, APP_NAME_DISPLAY,
            radiotray_ng::word_expand(this->config->get_string(RADIOTRAY_NG_NOTIFICATION_KEY, DEFAULT_RADIOTRAY_NG_NOTIFICATION_VALUE)));
    }
}


void RadiotrayNG::set_and_save_volume(uint32_t new_volume)
{
	const uint32_t volume(this->config->get_uint32(VOLUME_LEVEL_KEY, DEFAULT_VOLUME_LEVEL_VALUE));

	if (new_volume != volume)
	{
		this->volume = std::to_string(new_volume);
		this->player->volume(new_volume);
	}
}


void RadiotrayNG::display_volume_level()
{
    if (this->config->get_bool(NOTIFICATION_KEY, DEFAULT_NOTIFICATION_VALUE))
    {
        std::string volume_str =
            "Volume: " + std::to_string(this->config->get_uint32(VOLUME_LEVEL_KEY, DEFAULT_VOLUME_LEVEL_VALUE)) + "%";

        this->notification.notify(volume_str, APP_NAME_DISPLAY,
            radiotray_ng::word_expand(this->config->get_string(RADIOTRAY_NG_NOTIFICATION_KEY, DEFAULT_RADIOTRAY_NG_NOTIFICATION_VALUE)));
    }
}


bool RadiotrayNG::reload_bookmarks()
{
	bool result = this->bookmarks->load();

	if (result)
	{
		if (this->config->get_bool(NOTIFICATION_KEY, DEFAULT_NOTIFICATION_VALUE))
		{
			this->notification.notify("Bookmarks Reloaded", APP_NAME_DISPLAY,
				radiotray_ng::word_expand(this->config->get_string(RADIOTRAY_NG_NOTIFICATION_KEY, DEFAULT_RADIOTRAY_NG_NOTIFICATION_VALUE)));
		}

		// force reloading of current groups station list...
		this->set_station(this->group, this->station, this->station_notifications);
	}
	else
	{
		// always show...
		this->notification.notify("Bookmarks Reload Failed", APP_NAME_DISPLAY,
			radiotray_ng::word_expand(this->config->get_string(RADIOTRAY_NG_NOTIFICATION_KEY, DEFAULT_RADIOTRAY_NG_NOTIFICATION_VALUE)));
	}

	return result;
}


void RadiotrayNG::register_handlers()
{
	this->event_bus->subscribe(IEventBus::event::tags_changed,
		std::bind(&RadiotrayNG::on_tags_changed_event_processing, this, std::placeholders::_1, std::placeholders::_2), IEventBus::event_pos::first);

	this->event_bus->subscribe(IEventBus::event::tags_changed,
		std::bind(&RadiotrayNG::on_tags_changed_event_notification, this, std::placeholders::_1, std::placeholders::_2), IEventBus::event_pos::last);

	this->event_bus->subscribe(IEventBus::event::state_changed,
		std::bind(&RadiotrayNG::on_state_changed_event, this, std::placeholders::_1, std::placeholders::_2), IEventBus::event_pos::first);

	this->event_bus->subscribe(IEventBus::event::station_error,
		std::bind(&RadiotrayNG::on_station_error_event, this, std::placeholders::_1, std::placeholders::_2), IEventBus::event_pos::first);

	this->event_bus->subscribe(IEventBus::event::message,
		std::bind(&RadiotrayNG::on_message_event, this, std::placeholders::_1, std::placeholders::_2), IEventBus::event_pos::last);
}
