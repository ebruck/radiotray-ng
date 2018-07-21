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

#include <string>


class IRadioTrayNG
{
public:
	virtual ~IRadioTrayNG() = default;

	virtual void play(const std::string& group, const std::string& station) = 0;

	virtual void play() = 0;

	virtual void volume_up() = 0;

	virtual void volume_up_msg() = 0;

	virtual void volume_down() = 0;

	virtual void volume_down_msg() = 0;

	virtual void stop() = 0;

	virtual std::string get_title() = 0;

	virtual std::string get_artist() = 0;

	virtual std::string get_group() = 0;

	virtual std::string get_station() = 0;

	virtual std::string get_state() = 0;

	virtual std::string get_bitrate() = 0;

	virtual std::string get_codec() = 0;

	virtual std::string get_volume() = 0;

	virtual void set_title(const std::string& title) = 0;

	virtual void set_artist(const std::string& artist) = 0;

	virtual void set_station(const std::string& group, const std::string& station, bool notifications) = 0;

	virtual void set_state(const std::string& state) = 0 ;

	virtual void set_bitrate(const std::string& bitrate) = 0;

	virtual void set_codec(const std::string& codec) = 0;

	virtual void set_volume(const std::string& volume) = 0;

	virtual void set_volume_msg(uint32_t volume) = 0;

	virtual void previous_station_msg() = 0;

	virtual void next_station_msg() = 0;

	virtual bool reload_bookmarks() = 0;

	virtual std::string get_player_state() = 0;

	virtual std::string get_bookmarks() = 0;

	virtual std::string get_config() = 0;
};
