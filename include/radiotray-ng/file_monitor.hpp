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
	class FileMonitor
	{
	public:
		FileMonitor(const std::string& file)
			: file_to_watch(radiotray_ng::word_expand(file))
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

			if (!this->add_watch())
			{
				LOG(error) << "could not add inotify watch, monitoring disabled: " << errno;
				close(this->inotify_fd);
				return;
			}

			LOG(debug) << "monitoring: " << file;

			this->setup = true;
		}

		~FileMonitor()
		{
			if (this->setup)
			{
				this->cleanup();
			}
		}

		bool changed()
		{
			bool modified{false};

			if (this->setup)
			{
				std::vector<inotify_event> events;

				inotify_event tmp;
				while (read(inotify_fd, &tmp, sizeof(tmp)) > 0)
				{
					events.push_back(tmp);
				}

				for(auto event : events)
				{
					if (event.mask & IN_MODIFY || event.mask & IN_ATTRIB)
					{
						modified = true;
					}

					if (event.mask & IN_DELETE_SELF || event.mask & IN_MOVE_SELF || event.mask & IN_IGNORED)
					{
						inotify_rm_watch(this->inotify_fd, this->watch_fd);

						if (!this->add_watch())
						{
							LOG(error) << "could not add inotify watch, monitoring disabled: " << errno;

							close(this->inotify_fd);

							this->setup = false;
						}

						modified = true;

						break;
					}
				}
			}

			return modified;
		}

	private:

		bool add_watch()
		{
			this->watch_fd = inotify_add_watch(this->inotify_fd, file_to_watch.c_str(),
				IN_MODIFY | IN_ATTRIB | IN_DELETE_SELF | IN_MOVE_SELF | IN_IGNORED);

			return (this->watch_fd != -1);
		}

		void cleanup()
		{
			inotify_rm_watch(this->inotify_fd, this->watch_fd);
			close(this->inotify_fd);
		}

		int inotify_fd;
		int watch_fd;
		const std::string file_to_watch;
		bool setup{false};
	};
}
