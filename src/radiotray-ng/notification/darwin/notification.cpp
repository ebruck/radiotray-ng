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
#include <radiotray-ng/string_helpers.hpp>
#include <stdlib.h>

// lazy pimpl...
struct notify_t
{
	notify_t()
	{
	}

	~notify_t()
	{
	}
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
	std::string cmd = "terminal-notifier -title \"" + title + "\" -message \"" + message + "\" -appIcon \"" + radiotray_ng::word_expand(image) + "\"";
	cmd += " > /dev/null 2>&1";
	system(cmd.c_str());
}
