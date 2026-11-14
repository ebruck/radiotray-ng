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

#include <radiotray-ng/notification/notification.hpp>
#include <radiotray-ng/helpers.hpp>
#include <cerrno>
#include <fcntl.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

extern char** environ;

// lazy pimpl...
struct notify_t
{
    notify_t() = default;
    ~notify_t() = default;
};


Notification::Notification()
{
}

Notification::~Notification()
{
}


void Notification::notify(const std::string& title, const std::string& message)
{
	this->notify(title, message, "");
}


void Notification::notify(const std::string& title, const std::string& message, const std::string& image)
{
    // Pass notification text as argv, not through a shell. Track metadata can
    // contain quotes or shell metacharacters and must never become executable
    // command text.
    std::string expanded_image = radiotray_ng::word_expand(image);

    std::vector<char*> argv{
        const_cast<char*>("terminal-notifier"),
        const_cast<char*>("-title"),
        const_cast<char*>(title.c_str()),
        const_cast<char*>("-message"),
        const_cast<char*>(message.c_str()),
        const_cast<char*>("-appIcon"),
        const_cast<char*>(expanded_image.c_str()),
        nullptr
    };

    posix_spawn_file_actions_t actions;
    if (posix_spawn_file_actions_init(&actions) != 0)
        return;

    // Match the old quiet behaviour without constructing shell redirections.
    posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, "/dev/null", O_WRONLY, 0);
    posix_spawn_file_actions_addopen(&actions, STDERR_FILENO, "/dev/null", O_WRONLY, 0);

    pid_t child = 0;
    const int rc = posix_spawnp(
        &child,
        argv[0],
        &actions,
        nullptr,
        argv.data(),
        environ);

    posix_spawn_file_actions_destroy(&actions);

    if (rc == 0)
    {
        int status = 0;
        while (waitpid(child, &status, 0) == -1 && errno == EINTR)
        {
        }
    }
}
