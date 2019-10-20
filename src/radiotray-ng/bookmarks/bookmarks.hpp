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

	bool load();

	bool save();

	bool save_as(const std::string& new_filename);

	bool add_group(const std::string& group_name, const std::string& image);

	bool remove_group(const std::string& group_name);

	bool rename_group(const std::string& group_name, const std::string& new_group_name);

	bool update_group(const std::string& group_name, const std::string& new_group_image);

	bool add_station(const std::string& group_name, const std::string& station_name, const std::string& url, const std::string& image, bool notifications);

	bool remove_station(const std::string& group_name, const std::string& station_name);

	bool rename_station(const std::string& group_name, const std::string& station_name, const std::string& new_station_name);

	bool update_station(const std::string& group_name, const std::string& station_name, const std::string& new_station_url,
		const std::string& new_station_image, bool new_notifications);

	bool station_exists(const std::string& group_name, const std::string& station_name);

	bool move_group_to_pos(const std::string& group_name, size_t new_position);

	bool move_station_to_pos(const std::string& group_name, const std::string& station_name, size_t new_position);

	bool get_station(const std::string& group_name, const std::string& station_name, station_data_t& station_data);

	bool get_group_stations(const std::string& group_name, std::vector<IBookmarks::station_data_t>& station_data);

	bool get_group_as_json(const std::string& group_name, std::string& json);

	bool get_station_as_json(const std::string& group_name, const std::string& station_name, std::string& json);

	bool add_station_from_json(const std::string& group_name, const std::string& json, std::string& station_name);

	IBookmarks::group_data_t operator[](size_t index);

	const std::string& get_file_name() const;

	size_t size();

	std::string dump();

private:

	const std::string GROUP_KEY{"group"};
	const std::string GROUP_IMAGE_KEY{"image"};
	const std::string STATIONS_KEY{"stations"};
	const std::string STATION_NAME_KEY{"name"};
	const std::string STATION_URL_KEY{"url"};
	const std::string STATION_IMAGE_KEY{"image"};
	const std::string STATION_NOTIFICATIONS_KEY{"notifications"};
	const std::string STATION_DIRECT_KEY{"direct"};

	bool find_group(const std::string& group_name, Json::ArrayIndex& group_index);

	bool find_station(Json::ArrayIndex group_index, const std::string& station_name, Json::ArrayIndex& station_index);

	void move_to_pos(Json::Value& array, Json::ArrayIndex old_index, Json::ArrayIndex new_index);

	bool array_search(Json::Value& array, const std::string& key, const std::string& value, Json::ArrayIndex& index);

	bool get_station_notifications(const Json::Value& station);

	Json::Value bookmarks;
	std::string bookmarks_file;
};
