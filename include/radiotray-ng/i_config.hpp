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
#include <string>

// easier interface to work with than jsoncpp

class IConfig
{
public:
	virtual ~IConfig() = default;

	virtual bool save() = 0;

	virtual bool load() = 0;

	virtual void set_string(const std::string& key, const std::string& value) = 0;

	virtual void set_uint32(const std::string& key, const uint32_t value) = 0;

	virtual void set_bool(const std::string& key, const bool value) = 0;

	virtual std::string get_string(const std::string& key, const std::string& default_value) = 0;

	virtual uint32_t get_uint32(const std::string& key, const uint32_t default_value) = 0;

	virtual bool get_bool(const std::string& key, const bool default_value) = 0;

	virtual bool exists(const std::string& key) = 0;
};
