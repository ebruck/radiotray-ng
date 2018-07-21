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
#include <radiotray-ng/event_bus/event_bus.hpp>


bool EventBus::subscribe(const IEventBus::event ev, IEventBus::event_callback_t cb,
	const IEventBus::event_pos req_pos)
{
	switch(req_pos)
	{
		case IEventBus::event_pos::first:
		{
			if (!this->first_pos_map[ev])
			{
				this->first_pos_map[ev] = cb;
				return true;
			}
			return false;
		}
		case IEventBus::event_pos::last:
		{
			if (!this->last_pos_map[ev])
			{
				this->last_pos_map[ev] = cb;
				return true;
			}
			return false;
		}
		default:
			this->default_pos_map[ev].push_back(cb);
			return true;
	}
}


bool EventBus::publish(const IEventBus::event ev, IEventBus::event_data_t& data)
{
	if (this->first_pos_map.count(ev))
	{
		this->first_pos_map[ev](ev, data);
	}

	for(auto& cb : this->default_pos_map[ev])
	{
		cb(ev, data);
	}

	if (this->last_pos_map.count(ev))
	{
		this->last_pos_map[ev](ev, data);
	}

	return true;
}


bool EventBus::publish_only(const IEventBus::event ev, const std::string& key, const std::string& value)
{
	IEventBus::event_data_t data;

	data[key] = value;

	return this->publish(ev, data);
}
