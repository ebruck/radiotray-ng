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

#include <radiotray-ng/i_radiotray_ng.hpp>
#include <radiotray-ng/i_event_bus.hpp>
#include <radiotray-ng/i_gui.hpp>
#include <memory>


class NCursesGui final : public IGui
{
public:
	NCursesGui(std::shared_ptr<IRadioTrayNG> radiotray_ng, std::shared_ptr<IEventBus> event_bus);

	NCursesGui() = delete;

	virtual ~NCursesGui();

	void run(int argc, char* argv[]);

	void reload_bookmarks() { /* TODO */ }

	void stop(){};

private:
	// handlers for updating tag/status
	void   on_tags_event_data(const IEventBus::event& ev, IEventBus::event_data_t& data);
	void  on_state_event_data(const IEventBus::event& ev, IEventBus::event_data_t& data);
	void on_volume_event_data(const IEventBus::event& ev, IEventBus::event_data_t& data);

	void update_display();

	std::shared_ptr<IRadioTrayNG> radiotray_ng;
};
