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
#include <radiotray-ng/i_bookmarks.hpp>
#include <json/json.h>


class Bookmarks final : public IBookmarks
{
public:
	Bookmarks(const std::string& bookmarks_file);

	virtual ~Bookmarks() = default;

	bool load();

	bool save();

	bool add_group(const std::string& group_name, const std::string& image);

	bool remove_group(const std::string& group_name);

	bool rename_group(const std::string& group_name, const std::string& new_group_name);

	bool update_group(const std::string& group_name, const std::string& new_group_image);

	bool add_station(const std::string& group_name, const std::string& station_name, const std::string& url, const std::string& image);

	bool remove_station(const std::string& group_name, const std::string& station_name);

	bool rename_station(const std::string& group_name, const std::string& station_name, const std::string& new_station_name);

	bool update_station(const std::string& group_name, const std::string& station_name, const std::string& new_station_url,
		const std::string& new_station_image);

	bool station_exists(const std::string& group_name, const std::string& station_name);

	bool move_group_to_pos(const std::string& group_name, const size_t new_position);

	bool move_station_to_pos(const std::string& group_name, const std::string& station_name, const size_t new_position);

	bool get_station(const std::string& group_name, const std::string& station_name, station_data_t& station_data);

	bool get_group_stations(const std::string& group_name, std::vector<IBookmarks::station_data_t>& station_data);

	IBookmarks::group_data_t operator[](const size_t index);

	size_t size();

private:

	const std::string GROUP_KEY{"group"};
	const std::string GROUP_IMAGE_KEY{"image"};
	const std::string STATIONS_KEY{"stations"};
	const std::string STATION_NAME_KEY{"name"};
	const std::string STATION_URL_KEY{"url"};
	const std::string STATION_IMAGE_KEY{"image"};

	bool find_group(const std::string& group_name, Json::ArrayIndex& group_index);

	bool find_station(const Json::ArrayIndex group_index, const std::string& station_name, Json::ArrayIndex& station_index);

	void move_to_pos(Json::Value& array, const Json::ArrayIndex old_index, const Json::ArrayIndex new_index);

	bool array_search(Json::Value& array, const std::string& key, const std::string& value, Json::ArrayIndex& index);

	Json::Value bookmarks;
	const std::string bookmarks_file;
};
