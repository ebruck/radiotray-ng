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

#pragma once

#include <radiotray-ng/notification/notification.hpp>
#include <radiotray-ng/i_radiotray_ng.hpp>
#include <radiotray-ng/i_event_bus.hpp>
#include <radiotray-ng/i_bookmarks.hpp>

#include <memory>
#include <string>
#include <vector>

class IConfig;
class IPlayer;
class IBookmarks;


class RadiotrayNG final : public IRadioTrayNG
{
public:
	RadiotrayNG(std::shared_ptr<IConfig> config, std::shared_ptr<IBookmarks> bookmarks,
		std::shared_ptr<IPlayer> player, std::shared_ptr<IEventBus> event_bus);

	RadiotrayNG(RadiotrayNG&) = delete;
	RadiotrayNG() = delete;

	virtual ~RadiotrayNG();

	void play(const std::string& group, const std::string& station);

	void play();

	void volume_up();

	void volume_up_msg();

	void volume_down();

	void volume_down_msg();

	void stop();

	std::string get_title();

	std::string get_artist();

	std::string get_group();

	std::string get_station();

	std::string get_state();

	std::string get_bitrate();

	std::string get_codec();

	std::string get_volume();

	void set_title(const std::string& title);

	void set_artist(const std::string& artist);

	void set_station(const std::string& group, const std::string& station, bool notifications);

	void set_state(const std::string& state);

	void set_bitrate(const std::string& bitrate);

	void set_codec(const std::string& codec);

	void set_volume(const std::string& volume);

	void set_volume_msg(uint32_t volume);

	bool reload_bookmarks();

	void next_station_msg();

	void previous_station_msg();

	std::string get_bookmarks();

	std::string get_player_state();

	std::string get_config();

private:
	std::shared_ptr<IConfig>    config;
	std::shared_ptr<IBookmarks> bookmarks;
	std::shared_ptr<IPlayer>    player;
	std::shared_ptr<IEventBus>  event_bus;

	// current tag data
	std::string title;
	std::string artist;
	std::string group;
	std::string station;
	std::string state;
	std::string bitrate;
	std::string codec;
	std::string volume;
	bool station_notifications;

	// event handlers
	void on_state_changed_event(const IEventBus::event& ev, IEventBus::event_data_t& data);
	void on_station_error_event(const IEventBus::event &ev, IEventBus::event_data_t& data);
	void on_message_event(const IEventBus::event& ev, IEventBus::event_data_t& data);
	void on_tags_changed_event_notification(const IEventBus::event& ev, IEventBus::event_data_t& data);
	void on_tags_changed_event_processing(const IEventBus::event& ev, IEventBus::event_data_t& data);

	void display_volume_level();
	void register_handlers();
	void set_and_save_volume(uint32_t new_volume);
	void clear_tags();

	Notification notification;

	std::string notification_image;
	std::pair<std::string, std::string> last_notification;

	std::vector<IBookmarks::station_data_t> current_group_stations;
	int current_station_index;
};
