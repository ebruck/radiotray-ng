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

#include <radiotray-ng/bookmarks/bookmarks.hpp>
#include <iostream>


Bookmarks::Bookmarks(const std::string& bookmarks_file)
	: bookmarks(Json::Value(Json::arrayValue))
	, bookmarks_file(radiotray_ng::word_expand(bookmarks_file))
{
}


bool Bookmarks::load()
{
	LOG(debug) << "loading: " << this->bookmarks_file;

	try
	{
		std::ifstream ifile(this->bookmarks_file);
		ifile.exceptions(std::ios::failbit);

		Json::Reader reader;
		Json::Value new_bookmarks;

		if (!reader.parse(ifile, new_bookmarks))
		{
			LOG(error) << "Failed to parse: " << this->bookmarks_file << " : " << reader.getFormattedErrorMessages();
			return false;
		}

		this->bookmarks = new_bookmarks;
	}
	catch(std::exception& /*e*/)
	{
		LOG(error) << "Failed to load: " << this->bookmarks_file << " : "<< strerror(errno);
		return false;
	}

	return true;
}


bool Bookmarks::save()
{
	return this->save_as("");
}


bool Bookmarks::save_as(const std::string& new_filename)
{
	std::string filename = this->bookmarks_file;

	if (!new_filename.empty())
	{
		filename = radiotray_ng::word_expand(new_filename);
	}

	LOG(info) << "saving: " << filename;

	try
	{
		std::ofstream ofile(filename);
		ofile.exceptions(std::ios::failbit);

		ofile << Json::StyledWriter().write(this->bookmarks);
	}
	catch(std::exception& /*e*/)
	{
		LOG(error) << "Failed to save: " << filename << " : "<< strerror(errno);
		return false;
	}

	this->bookmarks_file = filename;

	return true;
}


bool Bookmarks::array_search(Json::Value& array, const std::string& key, const std::string& value, Json::ArrayIndex& index)
{
	auto it = std::find_if(array.begin(), array.end(), [key,value](const Json::Value& v){return v[key] == value;});

	if (it != array.end())
	{
		index = std::distance(array.begin(), it);
		return true;
	}

	return false;
}


bool Bookmarks::find_group(const std::string& group_name, Json::ArrayIndex& group_index)
{
	return this->array_search(this->bookmarks, GROUP_KEY, group_name, group_index);
}


bool Bookmarks::find_station(const Json::ArrayIndex group_index, const std::string& station_name, Json::ArrayIndex& station_index)
{
	return this->array_search(this->bookmarks[group_index][STATIONS_KEY], STATION_NAME_KEY, station_name, station_index);
}


bool Bookmarks::add_group(const std::string& group_name, const std::string& image)
{
	Json::ArrayIndex group_index;

	if (this->find_group(group_name, group_index))
	{
		return false;
	}

	Json::Value& value = this->bookmarks.append(Json::Value(Json::objectValue));

	value[GROUP_KEY]       = group_name;
	value[GROUP_IMAGE_KEY] = image;
	value[STATIONS_KEY]    = Json::Value(Json::arrayValue);

	return true;
}


bool Bookmarks::remove_group(const std::string& group_name)
{
	Json::ArrayIndex group_index;

	if (this->find_group(group_name, group_index))
	{
		Json::Value value;

		return this->bookmarks.removeIndex(group_index, &value);
	}

	return false;
}


bool Bookmarks::update_group(const std::string& group_name, const std::string& new_group_image)
{
	Json::ArrayIndex group_index;

	if (this->find_group(group_name, group_index))
	{
		this->bookmarks[group_index][GROUP_IMAGE_KEY] = new_group_image;
		return true;
	}

	return false;
}


bool Bookmarks::rename_group(const std::string& group_name, const std::string& new_group_name)
{
	Json::ArrayIndex group_index;

	if (!this->find_group(new_group_name, group_index))
	{
		if (this->find_group(group_name, group_index))
		{
			this->bookmarks[group_index][GROUP_KEY] = new_group_name;
			return true;
		}
	}

	return false;
}


bool Bookmarks::add_station(const std::string& group_name, const std::string& station_name, const std::string& station_url, const std::string& station_image, bool notifications)
{
	Json::ArrayIndex group_index;

	if (this->find_group(group_name, group_index))
	{
		Json::ArrayIndex station_index;

		if (!this->find_station(group_index, station_name, station_index))
		{
			Json::Value value;

			value[STATION_NAME_KEY]  = station_name;
			value[STATION_URL_KEY]   = station_url;
			value[STATION_IMAGE_KEY] = station_image;
			value[STATION_NOTIFICATIONS_KEY] = notifications;

			this->bookmarks[group_index][STATIONS_KEY].append(value);

			return true;
		}
	}

	return false;
}


bool Bookmarks::remove_station(const std::string& group_name, const std::string& station_name)
{
	Json::ArrayIndex group_index;

	if (this->find_group(group_name, group_index))
	{
		Json::ArrayIndex station_index;

		if (this->find_station(group_index, station_name, station_index))
		{
			Json::Value value;

			return this->bookmarks[group_index][STATIONS_KEY].removeIndex(station_index, &value);
		}
	}

	return false;
}


bool Bookmarks::rename_station(const std::string& group_name, const std::string& station_name, const std::string& new_station_name)
{
	Json::ArrayIndex group_index;

	if (this->find_group(group_name, group_index))
	{
		Json::ArrayIndex station_index;

		if (!this->find_station(group_index, new_station_name, station_index))
		{
			if (this->find_station(group_index, station_name, station_index))
			{
				this->bookmarks[group_index][STATIONS_KEY][station_index][STATION_NAME_KEY] = new_station_name;

				return true;
			}
		}
	}

	return false;
}


bool Bookmarks::update_station(const std::string& group_name, const std::string& station_name, const std::string& new_station_url, const std::string& new_station_image, bool new_notifications)
{
	Json::ArrayIndex group_index;

	if (this->find_group(group_name, group_index))
	{
		Json::ArrayIndex station_index;

		if (this->find_station(group_index, station_name, station_index))
		{
			this->bookmarks[group_index][STATIONS_KEY][station_index][STATION_URL_KEY]   = new_station_url;
			this->bookmarks[group_index][STATIONS_KEY][station_index][STATION_IMAGE_KEY] = new_station_image;
			this->bookmarks[group_index][STATIONS_KEY][station_index][STATION_NOTIFICATIONS_KEY] = new_notifications;

			return true;
		}
	}

	return false;
}


bool Bookmarks::station_exists(const std::string& group_name, const std::string& station_name)
{
	Json::ArrayIndex group_index;

	if (this->find_group(group_name, group_index))
	{
		Json::ArrayIndex station_index;

		if (this->find_station(group_index, station_name, station_index))
		{
			return true;
		}
	}

	return false;
}


void Bookmarks::move_to_pos(Json::Value& array, const Json::ArrayIndex old_index, const Json::ArrayIndex new_index)
{
	Json::Value value;
	array.removeIndex(old_index, &value);

	Json::Value new_array(Json::arrayValue);

	if (new_index < array.size())
	{
		for(Json::ArrayIndex i=0; i < array.size(); ++i)
		{
			if (i == new_index)
			{
				new_array.append(value);
			}

			new_array.append(array[i]);
		}

		array = new_array;
	}
	else
	{
		array.append(value);
	}
}


bool Bookmarks::move_group_to_pos(const std::string& group_name, const size_t new_group_index)
{
	if (new_group_index >= this->bookmarks.size())
	{
		return false;
	}

	Json::ArrayIndex group_index;

	if (this->find_group(group_name, group_index))
	{
		if (group_index == new_group_index)
		{
			return true;
		}

		this->move_to_pos(this->bookmarks, group_index, new_group_index);

		return true;
	}

	return false;
}


bool Bookmarks::move_station_to_pos(const std::string& group_name, const std::string& station_name, const size_t new_station_index)
{
	Json::ArrayIndex group_index;

	if (this->find_group(group_name, group_index))
	{
		if (new_station_index >= this->bookmarks[group_index][STATIONS_KEY].size())
		{
			return false;
		}

		Json::ArrayIndex station_index;

		if (this->find_station(group_index, station_name, station_index))
		{
			if (station_index == new_station_index)
			{
				return true;
			}

			this->move_to_pos(this->bookmarks[group_index][STATIONS_KEY], station_index, new_station_index);

			return true;
		}
	}

	return false;
}


IBookmarks::group_data_t Bookmarks::operator[](const size_t group_index)
{
	if (group_index >= this->bookmarks.size())
	{
		throw std::out_of_range("group index out of range");
	}

	IBookmarks::group_data_t stations;

	stations.group = this->bookmarks[Json::ArrayIndex(group_index)][GROUP_KEY].asString();
	stations.image = this->bookmarks[Json::ArrayIndex(group_index)][GROUP_IMAGE_KEY].asString();

	for(auto& station : this->bookmarks[Json::ArrayIndex(group_index)][STATIONS_KEY])
	{
		stations.stations.push_back({station[STATION_NAME_KEY].asString(), station[STATION_URL_KEY].asString(), station[STATION_IMAGE_KEY].asString(),
			this->get_station_notifications(station)});
	}

	return stations;
}


bool Bookmarks::get_group_stations(const std::string& group_name, std::vector<IBookmarks::station_data_t>& stations)
{
	Json::ArrayIndex group_index;

	if (this->find_group(group_name, group_index))
	{
		stations.clear();

		for(auto& station : this->bookmarks[Json::ArrayIndex(group_index)][STATIONS_KEY])
		{
			stations.push_back({station[STATION_NAME_KEY].asString(), station[STATION_URL_KEY].asString(), station[STATION_IMAGE_KEY].asString(),
				this->get_station_notifications(station)});
		}

		return true;
	}

	return false;
}


bool Bookmarks::get_station(const std::string& group_name, const std::string& station_name, station_data_t& station_data)
{
	Json::ArrayIndex group_index;

	if (this->find_group(group_name, group_index))
	{
		Json::ArrayIndex station_index;
		if (this->find_station(group_index, station_name, station_index))
		{
			station_data.name  = this->bookmarks[group_index][STATIONS_KEY][station_index][STATION_NAME_KEY].asString();
			station_data.url   = this->bookmarks[group_index][STATIONS_KEY][station_index][STATION_URL_KEY].asString();
			station_data.image = this->bookmarks[group_index][STATIONS_KEY][station_index][STATION_IMAGE_KEY].asString();
			station_data.notifications = this->get_station_notifications(this->bookmarks[group_index][STATIONS_KEY][station_index]);

			// use group image if not overridden
			if (station_data.image.empty())
			{
				station_data.image = this->bookmarks[Json::ArrayIndex(group_index)][GROUP_IMAGE_KEY].asString();
			}

			// clean up...
			radiotray_ng::trim(station_data.url);

			return true;
		}
	}

	return false;
}


const std::string& Bookmarks::get_file_name() const
{
	return this->bookmarks_file;
}


size_t Bookmarks::size()
{
	return size_t(this->bookmarks.size());
}


std::string Bookmarks::dump()
{
	return this->bookmarks.toStyledString();
}


bool Bookmarks::get_group_as_json(const std::string& group_name, std::string& json)
{
	Json::ArrayIndex group_index;

	if (this->find_group(group_name, group_index))
	{
		json = Json::StyledWriter().write(this->bookmarks[Json::ArrayIndex(group_index)]);
		return true;
	}

	return false;
}


bool Bookmarks::get_station_as_json(const std::string& group_name, const std::string& station_name, std::string& json)
{
	Json::ArrayIndex group_index;

	if (this->find_group(group_name, group_index))
	{
		Json::ArrayIndex station_index;
		if (this->find_station(group_index, station_name, station_index))
		{
			auto station = this->bookmarks[group_index][STATIONS_KEY][station_index];

			// may not be there...
			station[NOTIFICATION_KEY] = this->get_station_notifications(station);

			json = Json::StyledWriter().write(station);
			return true;
		}
	}

	return false;
}


bool Bookmarks::add_station_from_json(const std::string& group_name, const std::string& json, std::string& station_name)
{
	// first, validate the station
	Json::Reader reader;
	Json::Value station;

	if (!reader.parse(json, station))
	{
		LOG(error) << "Failed to parse:\n<<" << json << ">>\n" << reader.getFormattedErrorMessages();
		return false;
	}

	if (!station.isMember(STATION_NAME_KEY)|| !station.isMember(STATION_URL_KEY) ||	!station.isMember(STATION_IMAGE_KEY))
	{
		LOG(warning) << "Insufficient station data ...\n<<" << json << "<<";
		return false;
	}

	Json::ArrayIndex group_index;
	if (!this->find_group(group_name, group_index))
	{
		LOG(error) << "Failed to find group: " << group_name;
		return false;
	}

	Json::ArrayIndex station_index;
	if (this->find_station(group_index, station[STATION_NAME_KEY].asString(), station_index))
	{
		LOG(warning) << "Station <<" << station[STATION_NAME_KEY].asString() << "already exists!";
		return false;
	}

	this->bookmarks[group_index][STATIONS_KEY].append(station);
	station_name = station[STATION_NAME_KEY].asString();

	return true;
}


bool Bookmarks::get_station_notifications(const Json::Value& station)
{
	// asBool() returns false for a missing entry...
	if (station.isMember(STATION_NOTIFICATIONS_KEY))
	{
		return station[STATION_NOTIFICATIONS_KEY].asBool();
	}

	return DEFAULT_STATION_NOTIFICATION_VALUE;
}
