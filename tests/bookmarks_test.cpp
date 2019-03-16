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
#include <radiotray-ng/i_bookmarks.hpp>
#include <gtest/gtest.h>
#include <unistd.h>

namespace
{
	const std::string TEST_FILE{"bookmarks-utest.json"};

	const std::string GROUP_A{"group a"};
	const std::string GROUP_IMAGE_A{"image a"};
	const std::string STATION_NAME_A{"station a"};
	const std::string STATION_URL_A{"url a"};
	const std::string STATION_IMAGE_A{"url a"};

	const std::string GROUP_B{"group b"};
	const std::string GROUP_IMAGE_B{"image b"};
	const std::string STATION_NAME_B{"station b"};
	const std::string STATION_URL_B{"url b"};
	const std::string STATION_IMAGE_B{"url b"};

	const std::string GROUP_C{"group c"};
	const std::string GROUP_IMAGE_C{"image c"};
}


TEST(Bookmarks, test_that_a_group_is_added_and_removed)
{
	Bookmarks bm(RTNG_BOOKMARK_FILE);

	EXPECT_EQ(bm.get_file_name(), RTNG_BOOKMARK_FILE);

	ASSERT_TRUE(bm.add_group(GROUP_A, GROUP_IMAGE_A));
	ASSERT_FALSE(bm.add_group(GROUP_A, GROUP_IMAGE_A));
	EXPECT_EQ(bm.size(), size_t(1));

	{
		ASSERT_FALSE(bm.station_exists(GROUP_A, "name"));
		bm.add_station(GROUP_A, "name", "url", "image", true);
		ASSERT_FALSE(bm.station_exists(GROUP_A, "bad_name"));
		ASSERT_TRUE(bm.station_exists(GROUP_A, "name"));
		std::vector<IBookmarks::station_data_t> stations;
		ASSERT_TRUE(bm.get_group_stations(GROUP_A, stations));
		EXPECT_EQ(stations.size(), size_t(1));
		ASSERT_TRUE(stations[0].name == "name");
		ASSERT_TRUE(stations[0].url == "url");
		ASSERT_TRUE(stations[0].image == "image");
		ASSERT_TRUE(stations[0].notifications);

		ASSERT_TRUE(bm.add_group(GROUP_C, GROUP_IMAGE_C));
		ASSERT_TRUE(bm.get_group_stations(GROUP_C, stations));
		EXPECT_EQ(stations.size(), size_t(0));
		bm.add_station(GROUP_C, "name", "url", "image", false);
		ASSERT_TRUE(bm.get_group_stations(GROUP_C, stations));
		EXPECT_EQ(stations.size(), size_t(1));
		ASSERT_TRUE(stations[0].name == "name");
		ASSERT_TRUE(stations[0].url == "url");
		ASSERT_TRUE(stations[0].image == "image");
		ASSERT_FALSE(stations[0].notifications);

		// test json
		std::string json_str;
		{
			EXPECT_TRUE(bm.get_group_as_json(GROUP_A, json_str));
			Json::Value json;
			Json::Reader reader;
			reader.parse(json_str, json);

			EXPECT_EQ(json["group"].asString(), GROUP_A);
			EXPECT_EQ(json["image"].asString(), GROUP_IMAGE_A);
			EXPECT_EQ(json["stations"].size(), size_t(1));
			EXPECT_EQ(json["stations"][0]["image"].asString(), "image");
			EXPECT_EQ(json["stations"][0]["name"].asString(), "name");
			EXPECT_EQ(json["stations"][0]["url"].asString(), "url");
			EXPECT_EQ(json["stations"][0]["notifications"].asBool(), true);
		}
		json_str.clear();

		// json station
		{
			EXPECT_FALSE(bm.get_station_as_json(GROUP_A, "Name", json_str));

			json_str =
			"{"
				"\"name\" : \"name_json\","
				"\"image\" : \"image_json\","
				"\"url\" : \"url_json\""
			"}";

			std::string station_name;
			EXPECT_TRUE(bm.add_station_from_json(GROUP_A, json_str, station_name));
			EXPECT_TRUE(bm.get_station_as_json(GROUP_A, station_name, json_str));
			Json::Value json;
			Json::Reader reader;
			reader.parse(json_str, json);

			EXPECT_EQ(json["image"].asString(), "image_json");
			EXPECT_EQ(json["name"].asString(), "name_json");
			EXPECT_EQ(json["url"].asString(), "url_json");
			EXPECT_EQ(json["notifications"].asBool(), true);
		}
	}

	EXPECT_EQ(bm[0].group, GROUP_A);
	EXPECT_EQ(bm[0].image, GROUP_IMAGE_A);
	ASSERT_TRUE(bm.rename_group(GROUP_A, GROUP_B));
	ASSERT_FALSE(bm.remove_group(GROUP_A));
	ASSERT_TRUE(bm.update_group(GROUP_B, GROUP_IMAGE_B));
	EXPECT_EQ(bm[0].group, GROUP_B);
	EXPECT_EQ(bm[0].image, GROUP_IMAGE_B);
	ASSERT_TRUE(bm.remove_group(GROUP_B));
	ASSERT_TRUE(bm.remove_group(GROUP_C));
	std::string tmp_json;
	ASSERT_FALSE(bm.get_group_as_json(GROUP_C, tmp_json));
	ASSERT_TRUE(tmp_json.empty());
	EXPECT_EQ(bm.size(), size_t(0));
	EXPECT_THROW(bm[100], std::out_of_range);
}


TEST(Bookmarks, test_that_group_can_be_move_to_a_new_position)
{
	Bookmarks bm(RTNG_BOOKMARK_FILE);

	ASSERT_TRUE(bm.add_group(GROUP_B, GROUP_IMAGE_B));
	ASSERT_TRUE(bm.add_group(GROUP_A,GROUP_IMAGE_A));

	EXPECT_EQ(bm.size(), size_t(2));
	EXPECT_EQ(bm[0].group, GROUP_B);

	ASSERT_FALSE(bm.move_group_to_pos(GROUP_A, 10));
	ASSERT_TRUE (bm.move_group_to_pos(GROUP_A, 0));

	EXPECT_EQ(bm[0].group, GROUP_A);
	EXPECT_EQ(bm[1].group, GROUP_B);
}


TEST(Bookmarks, test_that_stations_are_added_and_removed_from_a_group_and_moved)
{
	Bookmarks bm(RTNG_BOOKMARK_FILE);

	// station B
	ASSERT_FALSE(bm.add_station(GROUP_A, STATION_NAME_B, STATION_URL_B, STATION_IMAGE_B, false));
	ASSERT_TRUE (bm.add_group(GROUP_A, GROUP_IMAGE_A));
	ASSERT_TRUE (bm.add_station(GROUP_A, STATION_NAME_B, STATION_URL_B, STATION_IMAGE_B, false));
	EXPECT_EQ   (bm[0].stations.size(), size_t(1));

	IBookmarks::station_data_t std;
	ASSERT_TRUE(bm.get_station(GROUP_A, STATION_NAME_B, std));
	EXPECT_EQ(std.name, STATION_NAME_B);
	EXPECT_EQ(std.url, STATION_URL_B);
	EXPECT_EQ(std.image, STATION_IMAGE_B);

	// station A
	ASSERT_FALSE(bm.add_station(GROUP_A, STATION_NAME_B, STATION_URL_B, STATION_IMAGE_B, true));
	ASSERT_TRUE (bm.add_station(GROUP_A, STATION_NAME_A, STATION_URL_A, STATION_IMAGE_A, true));
	EXPECT_EQ   (bm[0].stations.size(), size_t(2));

	// check values
	EXPECT_EQ(bm[0].stations[0].name,  STATION_NAME_B);
	EXPECT_EQ(bm[0].stations[0].url,   STATION_URL_B);
	EXPECT_EQ(bm[0].stations[0].image, STATION_IMAGE_B);
	ASSERT_FALSE(bm[0].stations[0].notifications);

	EXPECT_EQ(bm[0].stations[1].name,  STATION_NAME_A);
	EXPECT_EQ(bm[0].stations[1].url,   STATION_URL_A);
	EXPECT_EQ(bm[0].stations[1].image, STATION_IMAGE_A);
	ASSERT_TRUE(bm[0].stations[1].notifications);

	// Move station
	ASSERT_FALSE(bm.move_station_to_pos(GROUP_A, STATION_NAME_A, 10));
	ASSERT_TRUE (bm.move_station_to_pos(GROUP_A, STATION_NAME_A, 0));

	// check values
	EXPECT_EQ(bm[0].stations[0].name,  STATION_NAME_A);
	EXPECT_EQ(bm[0].stations[0].url,   STATION_URL_A);
	EXPECT_EQ(bm[0].stations[0].image, STATION_IMAGE_A);
	ASSERT_TRUE(bm[0].stations[0].notifications);

	EXPECT_EQ(bm[0].stations[1].name,  STATION_NAME_B);
	EXPECT_EQ(bm[0].stations[1].url,   STATION_URL_B);
	EXPECT_EQ(bm[0].stations[1].image, STATION_IMAGE_B);
	ASSERT_FALSE(bm[0].stations[1].notifications);

	// remove station
	ASSERT_FALSE(bm.remove_station(GROUP_A, "lsdkfalsdfk"));
	ASSERT_TRUE(bm.remove_station(GROUP_A, STATION_NAME_A));
	EXPECT_EQ(bm[0].stations.size(), size_t(1));

	EXPECT_EQ(bm[0].stations[0].name,  STATION_NAME_B);
	EXPECT_EQ(bm[0].stations[0].url,   STATION_URL_B);
	EXPECT_EQ(bm[0].stations[0].image, STATION_IMAGE_B);
	ASSERT_FALSE(bm[0].stations[0].notifications);

	// vector only throws when using at()
	EXPECT_THROW(bm[0].stations.at(100), std::out_of_range);
	EXPECT_THROW(bm[1], std::out_of_range);
}


TEST(Bookmarks, test_that_bookmarks_can_be_loaded_and_saved)
{
	// create
	{
		Bookmarks bm(TEST_FILE);

		ASSERT_TRUE(bm.add_group(GROUP_A, GROUP_IMAGE_A));
		ASSERT_TRUE(bm.add_station(GROUP_A, STATION_NAME_A, STATION_URL_A, STATION_IMAGE_A, true));

		ASSERT_TRUE(bm.save());
	}

	// load
	{
		Bookmarks bm(TEST_FILE);

		ASSERT_TRUE(bm.load());

		EXPECT_EQ(bm[0].group, GROUP_A);
		EXPECT_EQ(bm[0].stations[0].name,  STATION_NAME_A);
		EXPECT_EQ(bm[0].stations[0].url,   STATION_URL_A);
		EXPECT_EQ(bm[0].stations[0].image, STATION_IMAGE_A);
		ASSERT_TRUE(bm[0].stations[0].notifications);

		EXPECT_EQ(bm[0].stations.size(), size_t(1));
		EXPECT_EQ(bm.size(), size_t(1));
	}

	EXPECT_EQ(unlink(TEST_FILE.c_str()), 0);
}
