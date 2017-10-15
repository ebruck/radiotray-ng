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
#include <vector>


class IBookmarks
{
public:

	using station_data_t = struct
	{
		std::string name;
		std::string url;
		std::string image;
	};

	using group_data_t = struct
	{
		std::string group;
		std::string image;

		std::vector<IBookmarks::station_data_t> stations;
	};

	virtual ~IBookmarks() = default;

	virtual bool load() = 0;

	virtual bool save(const std::string& new_filename) = 0;

	virtual bool add_group(const std::string& group_name, const std::string& image) = 0;

	virtual bool update_group(const std::string& group_name, const std::string& new_group_image) = 0;

	virtual bool remove_group(const std::string& group_name) = 0;

	virtual bool rename_group(const std::string& group_name, const std::string& new_group_name) = 0;

	virtual bool add_station(const std::string& group_name, const std::string& station_name, const std::string& url, const std::string& image) = 0;

	virtual bool remove_station(const std::string& group_name, const std::string& station_name) = 0;

	virtual bool rename_station(const std::string& group_name, const std::string& station_name, const std::string& new_station_name) = 0;

	virtual bool update_station(const std::string& group_name, const std::string& station_name, const std::string& new_station_url, const std::string& new_station_image) = 0;

	virtual bool station_exists(const std::string& group_name, const std::string& station_name) = 0;

	virtual bool move_group_to_pos(const std::string& group_name, const size_t new_position) = 0;

	virtual bool move_station_to_pos(const std::string& group_name, const std::string& station_name, const size_t new_position) = 0;

	virtual bool get_station(const std::string& group_name, const std::string& station_name, station_data_t& station_data) = 0;

	virtual bool get_group_stations(const std::string& group_name, std::vector<IBookmarks::station_data_t>& station_data) = 0;

	bool get_group_as_json(const std::string& group_name, std::string& json);

	bool get_station_as_json(const std::string& group_name, const std::string& station_name, std::string& json);

	bool add_station_from_json(const std::string& group_name, const std::string& json, std::string& station_name);

	virtual group_data_t operator[](const size_t index) = 0;

	virtual size_t size() = 0;

	virtual std::string dump() = 0;
};
