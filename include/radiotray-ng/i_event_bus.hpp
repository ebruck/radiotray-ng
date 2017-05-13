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

#include <functional>
#include <stdint.h>
#include <string>
#include <map>


class IEventBus
{
public:
	enum class event: uint8_t
	{
		state_changed,
		station_error,
		tags_changed,
		volume_changed,
		message
	};

	enum class event_pos : uint8_t
	{
		any,
		first,
		last
	};

	using event_data_t = std::map<std::string, std::string>;

	using event_callback_t = std::function<void (const IEventBus::event&, IEventBus::event_data_t& data)>;

	virtual ~IEventBus() = default;

	virtual bool subscribe(const IEventBus::event ev, IEventBus::event_callback_t cb, const IEventBus::event_pos req_pos = IEventBus::event_pos::any) = 0;

	virtual bool publish(const IEventBus::event ev, IEventBus::event_data_t& data) = 0;

	virtual bool publish_only(const IEventBus::event ev, const std::string& key, const std::string& value) = 0;
};
