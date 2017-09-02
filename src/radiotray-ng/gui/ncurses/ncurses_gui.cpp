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
#include <radiotray-ng/gui/ncurses/ncurses_gui.hpp>

#include <ncurses.h>


NCursesGui::NCursesGui(std::shared_ptr<IRadioTrayNG> radiotray_ng, std::shared_ptr<IEventBus> event_bus)
	: radiotray_ng(std::move(radiotray_ng))
{
	event_bus->subscribe(IEventBus::event::tags_changed, std::bind(&NCursesGui::on_tags_event_data, this, std::placeholders::_1,
		std::placeholders::_2));

	event_bus->subscribe(IEventBus::event::state_changed, std::bind(&NCursesGui::on_state_event_data, this, std::placeholders::_1,
		std::placeholders::_2));

	event_bus->subscribe(IEventBus::event::volume_changed, std::bind(&NCursesGui::on_volume_event_data, this, std::placeholders::_1,
		std::placeholders::_2));

	initscr();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);
}


NCursesGui::~NCursesGui()
{
	endwin();
}


void NCursesGui::update_display()
{
	clear();

	mvprintw(0, 0, "Station: %s ", this->radiotray_ng->get_station().c_str());
	mvprintw(1, 0, "  Title: %s ", this->radiotray_ng->get_title().c_str());
	mvprintw(2, 0, " Artist: %s ", this->radiotray_ng->get_artist().c_str());
	mvprintw(3, 0, "  Codec: %s ", this->radiotray_ng->get_codec().c_str());
	mvprintw(4, 0, "Bitrate: %s ", this->radiotray_ng->get_bitrate().c_str());
	mvprintw(5, 0, "  State: %s ", this->radiotray_ng->get_state().c_str());
	mvprintw(6, 0, " Volume: %s ", this->radiotray_ng->get_volume().c_str());

	refresh();
}


void NCursesGui::on_state_event_data(const IEventBus::event& /*ev*/, IEventBus::event_data_t& /*data*/)
{
	this->update_display();
}


void NCursesGui::on_volume_event_data(const IEventBus::event& /*ev*/, IEventBus::event_data_t& /*data*/)
{
	this->update_display();
}


void NCursesGui::on_tags_event_data(const IEventBus::event& /*ev*/, IEventBus::event_data_t& /*data*/)
{
	this->update_display();
}


void NCursesGui::run(int /*argc*/, char* /*argv*/[])
{
	this->update_display();

	while(1)
	{
		int c;

		switch(c = getch())
		{
			case KEY_UP:
				this->radiotray_ng->volume_up();
				break;

			case KEY_DOWN:
				this->radiotray_ng->volume_down();
				break;

			case ' ':
				if (this->radiotray_ng->get_state() == STATE_PLAYING)
				{
					this->radiotray_ng->stop();
				}
				else
				{
					this->radiotray_ng->play();
				}
				break;

			case 'q':
				return;

			default:
				break;
		}
	}
}

