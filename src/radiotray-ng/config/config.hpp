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
#include <radiotray-ng/i_config.hpp>
#include <json/json.h>


class Config final : public IConfig
{
public:
	Config(const std::string& config_file);

	bool save();

	bool load();

	void set_string(const std::string& key, const std::string& value);

	void set_uint32(const std::string& key, uint32_t value);

	void set_bool(const std::string& key, bool value);

	std::string get_string(const std::string& key, const std::string& default_value);

	uint32_t get_uint32(const std::string& key, uint32_t default_value);

	bool get_bool(const std::string& key, bool default_value);

	bool exists(const std::string& key);

	std::string dump();

private:
	const std::string config_file;
	Json::Value config;
};
