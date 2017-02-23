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
#include <radiotray-ng/helpers.hpp>
#include <radiotray-ng/i_bookmarks.hpp>
#include <radiotray-ng/i_config.hpp>
#include <radiotray-ng/i_player.hpp>
#include <radiotray-ng/notification/notification.hpp>
#include <radiotray-ng/playlist/playlist_downloader.hpp>
#include <cmath>


RadiotrayNG::RadiotrayNG(std::shared_ptr<IConfig> config, std::shared_ptr<IBookmarks> bookmarks,
	                     std::shared_ptr<IPlayer> player, std::shared_ptr<IEventBus> event_bus)
	: config(std::move(config))
	, bookmarks(std::move(bookmarks))
	, player(std::move(player))
	, event_bus(std::move(event_bus))
	, state(STATE_STOPPED)
	, notification(new Notification())
{
	this->notification_image = this->config->get_string(NOTIFICATION_IMAGE_KEY, DEFAULT_NOTIFICATION_IMAGE_VALUE);

	this->set_volume(this->config->get_string(VOLUME_LEVEL_KEY, std::to_string(DEFAULT_VOLUME_LEVEL_VALUE)));

	this->set_station(this->config->get_string(LAST_STATION_GROUP_KEY, ""), this->config->get_string(LAST_STATION_KEY, ""));

	this->register_handlers();
}


RadiotrayNG::~RadiotrayNG()
{
	this->config->save();
}


void RadiotrayNG::stop()
{
	LOG(info) << "stopping player";

	if (this->config->get_bool(NOTIFICATION_KEY, DEFAULT_NOTIFICATION_VALUE) &&
		this->config->get_bool(NOTIFICATION_VERBOSE_KEY, DEFAULT_NOTIFICATION_VERBOSE_VALUE))
	{
		this->notification->notify("Stopped", this->get_station(), this->notification_image);
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
	std::lock_guard<std::mutex> lock(this->tag_update_mutex);

	return this->title;
}


std::string RadiotrayNG::get_artist()
{
	std::lock_guard<std::mutex> lock(this->tag_update_mutex);

	return this->artist;
}


std::string RadiotrayNG::get_group()
{
	std::lock_guard<std::mutex> lock(this->tag_update_mutex);

	return this->group;
}


std::string RadiotrayNG::get_station()
{
	std::lock_guard<std::mutex> lock(this->tag_update_mutex);

	return this->station;
}


std::string RadiotrayNG::get_state()
{
	std::lock_guard<std::mutex> lock(this->tag_update_mutex);

	return this->state;
}


std::string RadiotrayNG::get_bitrate()
{
	std::lock_guard<std::mutex> lock(this->tag_update_mutex);

	return this->bitrate;
}


std::string RadiotrayNG::get_codec()
{
	std::lock_guard<std::mutex> lock(this->tag_update_mutex);

	return this->codec;
}


std::string RadiotrayNG::get_volume()
{
	std::lock_guard<std::mutex> lock(this->tag_update_mutex);

	return this->volume;
}


void RadiotrayNG::set_title(const std::string& title)
{
	std::lock_guard<std::mutex> lock(this->tag_update_mutex);

	this->title = title;
}


void RadiotrayNG::set_artist(const std::string& artist)
{
	std::lock_guard<std::mutex> lock(this->tag_update_mutex);

	this->artist = artist;
}


void RadiotrayNG::set_station(const std::string& group, const std::string& station)
{
	std::lock_guard<std::mutex> lock(this->tag_update_mutex);

	this->group = group;
	this->station = station;
}


void RadiotrayNG::set_state(const std::string& state)
{
	std::lock_guard<std::mutex> lock(this->tag_update_mutex);

	this->state = state;
}


void RadiotrayNG::set_bitrate(const std::string& bitrate)
{
	std::lock_guard<std::mutex> lock(this->tag_update_mutex);

	this->bitrate = bitrate;
}


void RadiotrayNG::set_codec(const std::string& codec)
{
	std::lock_guard<std::mutex> lock(this->tag_update_mutex);

	this->codec = codec;
}


void RadiotrayNG::set_volume(const std::string& volume)
{
	std::lock_guard<std::mutex> lock(this->tag_update_mutex);

	this->volume = volume;
}


void RadiotrayNG::on_tags_changed_event_processing(const IEventBus::event& /*ev*/, IEventBus::event_data_t& data)
{
	std::lock_guard<std::mutex> lock(this->tag_update_mutex);

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
			this->notification->notify(this->get_station(), APP_NAME_DISPLAY, this->notification_image);
			return;
		}

		if (this->config->get_bool(NOTIFICATION_VERBOSE_KEY, DEFAULT_NOTIFICATION_VERBOSE_VALUE))
		{
			if (data[STATE_KEY] == STATE_CONNECTING)
			{
				this->notification->notify("Connecting", APP_NAME_DISPLAY, this->notification_image);
				return;
			}

			if (data[STATE_KEY] == STATE_BUFFERING)
			{
				this->notification->notify("Buffering", this->get_station(), this->notification_image);
				return;
			}
		}
	}
}


void RadiotrayNG::on_station_error_event(const IEventBus::event& /*ev*/, IEventBus::event_data_t& data)
{
	// always show errors
	this->notification->notify("Error", data[ERROR_KEY], this->notification_image);
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
						this->notification->notify(this->last_notification.first, this->last_notification.second, this->notification_image);
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
				this->last_notification.second = APP_NAME_DISPLAY;

				if (this->config->get_bool(NOTIFICATION_KEY, DEFAULT_NOTIFICATION_VALUE))
				{
					this->notification->notify(this->last_notification.first, this->last_notification.second, this->notification_image);
				}
			}
		}
	}
}


void RadiotrayNG::play(const std::string& group, const std::string& station)
{
	if (this->state == STATE_PLAYING)
	{
		this->player->stop();
	}

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
			this->notification_image = radiotray_ng::word_expand(this->config->get_string(NOTIFICATION_IMAGE_KEY, DEFAULT_NOTIFICATION_IMAGE_VALUE));
		}

		this->event_bus->publish_only(IEventBus::event::state_changed, STATE_KEY, STATE_CONNECTING);

		if (PlaylistDownloader(this->config).download_playlist(std.url, pls))
		{
			this->config->set_string(LAST_STATION_GROUP_KEY, group);
			this->config->set_string(LAST_STATION_KEY, std.name);

			this->set_station(group, std.name);

			if (this->player->play(pls))
			{
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
	uint32_t volume{this->config->get_uint32(VOLUME_LEVEL_KEY, DEFAULT_VOLUME_LEVEL_VALUE)};

	if (++volume <= this->config->get_uint32(VOLUME_MAX_LEVEL_KEY, DEFAULT_VOLUME_LEVEL_MAX_VALUE))
	{
		this->volume = std::to_string(volume);
		this->player->volume(volume);

		LOG(debug) << "volume: " << this->volume;

		this->config->save();
	}
}


void RadiotrayNG::volume_down()
{
	uint32_t volume{this->config->get_uint32(VOLUME_LEVEL_KEY, DEFAULT_VOLUME_LEVEL_VALUE)};

	if (volume > 0)
	{
		this->volume = std::to_string(--volume);
		this->player->volume(volume);

		LOG(debug) << "volume: " << this->volume;

		this->config->save();
	}
}


bool RadiotrayNG::reload_bookmarks()
{
	bool result = this->bookmarks->load();

	if (result)
	{
		// always show...
		this->notification->notify("Bookmarks Reloaded", APP_NAME_DISPLAY,
				radiotray_ng::word_expand(this->config->get_string(NOTIFICATION_IMAGE_KEY, DEFAULT_NOTIFICATION_IMAGE_VALUE)));
	}
	else
	{
		// always show...
		this->notification->notify("Bookmarks Reload Failed", APP_NAME_DISPLAY,
				radiotray_ng::word_expand(this->config->get_string(NOTIFICATION_IMAGE_KEY, DEFAULT_NOTIFICATION_IMAGE_VALUE)));
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
}
