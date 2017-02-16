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

#include <radiotray-ng/i_event_bus.hpp>
#include <map>
#include <mutex>
#include <vector>


class EventBus final : public IEventBus
{
public:
	virtual ~EventBus() = default;

	bool subscribe(const IEventBus::event, IEventBus::event_callback_t cb,
		const IEventBus::event_pos req_pos = IEventBus::event_pos::any);

	bool publish(const IEventBus::event ev, event_data_t& data);

	bool publish_only(const IEventBus::event ev, const std::string& key, const std::string& value);

private:

	using event_map_t = std::map<IEventBus::event, event_callback_t>;
	using event_map_list_t = std::map<IEventBus::event, std::vector<event_callback_t>>;

	event_map_list_t default_pos_map;
	event_map_t      first_pos_map;
	event_map_t      last_pos_map;
	std::mutex       event_map_lock;
};
