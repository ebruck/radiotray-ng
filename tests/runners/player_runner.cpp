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
#include <radiotray-ng/config/config.hpp>
#include <radiotray-ng/event_bus/event_bus.hpp>
#include <radiotray-ng/notification/notification.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <signal.h>


Notification* g_notify=nullptr;
std::string organization{"Radiotray NG"};
std::mutex m;
std::condition_variable cv;


void signal_handler(int sig)
{
	if (sig == SIGINT || sig == SIGQUIT)
	{
		cv.notify_all();
	}
}

std::vector<std::string> split(const std::string& s, const std::string& seperator)
{
   std::vector<std::string> output;

    std::string::size_type prev_pos = 0, pos = 0;

    while((pos = s.find(seperator, pos)) != std::string::npos)
    {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );

        output.push_back(substring);

        prev_pos = pos+=seperator.size();
    }

    output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word

    return output;
}


void on_event_data(const IEventBus::event& ev, IEventBus::event_data_t& data)
{
	switch(ev)
	{
		case IEventBus::event::state_changed:
		{
			if (data["state"] == "playing")
			{
				g_notify->notify("Radiotray NG", organization);
			}

			if (data["state"] == "stopped")
			{
				g_notify->notify(organization, "Stopped");
			}

			if (data["state"] == "paused")
			{
				g_notify->notify(organization, "Paused");
			}
		}
		break;

		case IEventBus::event::station_error:
		{
			g_notify->notify("Error", data["error"]);
		}
		break;

		default:
			g_notify->notify("Unknown Event", std::to_string(int(ev)));
			break;
	}
}


void on_tags_event_data(const IEventBus::event& /*ev*/, IEventBus::event_data_t& data)
{
	std::stringstream ss;
	ss << std::endl << std::endl;
	for(auto& entry : data)
	{
		ss << std::setw(15) << std::setfill(' ') << entry.first << ": " << entry.second << std::endl;
	}
	std::cout << ss.str();

	if (data.count("organization") && !data["organization"].empty())
	{
		organization = data["organization"];
	}

	if (data.count("title") && !data["title"].empty())
	{
		auto v = split(data["title"], " - ");

		if (!v.empty())
		{
			std::string s;
			for(size_t i=1; i < v.size(); ++i)
			{
				// test filtering...
				if (v[i].find("Album") == std::string::npos)
				{
					s += v[i] + "\n";
				}
			}
			s += "by " + v[0];

			g_notify->notify(organization, s);

		}
		else
		{
			g_notify->notify("Radiotray NG", data["title"]);
		}
	}
}


int main(int argc, char** argv)
{
	if (argc > 1)
	{
		signal(SIGINT, signal_handler);
		signal(SIGQUIT, signal_handler);

		Notification notify;
		g_notify = &notify;

		std::shared_ptr<IEventBus> evm(new EventBus());

		evm->subscribe(IEventBus::event::tags_changed,  on_tags_event_data);
		evm->subscribe(IEventBus::event::state_changed, on_event_data);
		evm->subscribe(IEventBus::event::station_error, on_event_data);

		std::shared_ptr<IConfig> cfg(new Config("~/.config/radiotray-ng/radiotray-ng.json"));
		cfg->load();

		Player player(cfg, evm);

		playlist_t pls;
		pls.push_back(argv[1]);

		player.play(pls);

		std::unique_lock<std::mutex> lock(m);
		cv.wait(lock);
	}

	return 0;
}
