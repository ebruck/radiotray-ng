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

#include <radiotray-ng/helpers.hpp>
#include <iostream>

#ifndef __APPLE__
	#include <bsd/libutil.h>
#endif


namespace radiotray_ng
{
#ifndef __APPLE__
	class Pidfile
	{
	public:
		Pidfile(const std::string& app_name)
			: pfh(nullptr)
			, already_running(false)
		{
			std::string base_dir{radiotray_ng::get_runtime_dir()};

			// fallback to config dir?
			if (base_dir.empty())
			{
				base_dir = radiotray_ng::get_data_dir(app_name);
			}

			std::string pid_file{base_dir + "/" + app_name + ".pid"};

			pid_t other_pid = 0;
			errno = 0;
			this->pfh = pidfile_open(pid_file.c_str(), 0600, &other_pid);

			if (this->pfh == nullptr)
			{
				if (errno == EEXIST || errno == EAGAIN || errno == EINVAL)
				{
					std::cerr << "An instance of " << app_name << " is already running";
					if (other_pid > 0)
					{
						std::cerr << ", pid: " << other_pid;
					}
					std::cerr << std::endl;
				}
				else
				{
					std::cerr << "Could not acquire single-instance lock: " << pid_file << std::endl;
				}

				// Never run without the lock: doing so can create duplicate tray indicators.
				this->already_running = true;
				return;
			}

			if (pidfile_write(this->pfh) != 0)
			{
				std::cerr << "Could not write single-instance lock: " << pid_file << std::endl;
				pidfile_remove(this->pfh);
				this->pfh = nullptr;
				this->already_running = true;
			}
		}

		bool is_running()
		{
			return this->already_running;
		}

		~Pidfile()
		{
			if (this->pfh != nullptr)
			{
				pidfile_remove(this->pfh);
			}
		}

	private:
		pidfh* pfh;
		bool already_running;
	};
#else
	// until I figure this out...
	class Pidfile
	{
	public:
		Pidfile(const std::string& ){}
		bool is_running(){ return false; }
	};
#endif
}

