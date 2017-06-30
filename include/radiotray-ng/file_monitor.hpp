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

#include <radiotray-ng/common.hpp>
#include <radiotray-ng/helpers.hpp>

#include <sys/inotify.h>
#include <fcntl.h>
#include <string>

namespace radiotray_ng
{
	class file_monitor
	{
	public:
		file_monitor(const std::string& file)
		{
			this->inotify_fd = inotify_init();

			if (this->inotify_fd < 0)
			{
				LOG(error) << "could not initialize inotify, monitoring disabled: " << errno;
				return;
			}

			if (fcntl(this->inotify_fd, F_SETFL, fcntl(this->inotify_fd, F_GETFL) | O_NONBLOCK) < 0)
			{
				LOG(error) << "could not set notify descriptor to non-blocking, monitoring disabled: " << errno;
				close(this->inotify_fd);
				return;
			}

			this->watch_fd = inotify_add_watch(this->inotify_fd, radiotray_ng::word_expand(file).c_str(), IN_MODIFY);

			if (this->watch_fd == -1)
			{
				LOG(error) << "could not add inotify watch, monitoring disabled: " << errno;
				close(this->inotify_fd);
				return;
			}

			LOG(info) << "monitoring for changes: " << file;

			this->setup = true;
		}

		~file_monitor()
		{
			if (this->setup)
			{
				inotify_rm_watch(this->inotify_fd, this->watch_fd);
				close(this->inotify_fd);
			}
		}

		bool changed()
		{
			if (this->setup)
			{
				inotify_event event;

				if (read(inotify_fd, &event, sizeof(event)) < 0)
				{
					// for now assume nothing changed...
					return false;
				}

				return (event.mask & IN_MODIFY);
			}

			return false;
		}

	private:
		int inotify_fd;
		int watch_fd;
		bool setup{false};
	};
}
