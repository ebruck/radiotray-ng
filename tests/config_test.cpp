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

#include <radiotray-ng/config/config.hpp>
#include <gtest/gtest.h>

namespace
{
	const std::string TEST_FILE{"config-utest.json"};
}

///@todo probably could use typed tests
TEST(Config, test_that_key_value_are_saved_and_retrieved)
{
	Config cfg{RTNG_CONFIG_FILE};

	// strings
	{
		std::string value = "asdf";

		cfg.set_string("s_key", "string");
		EXPECT_EQ(cfg.get_string("s_key", value), "string");

		cfg.set_string("s_key", std::string("new_value"));
		EXPECT_EQ(cfg.get_string("s_key", ""), "new_value");
		ASSERT_TRUE(cfg.exists("s_key"));
	}

	// uint32_t
	{
		cfg.set_uint32("u_key", uint32_t(123));
		EXPECT_EQ(cfg.get_uint32("u_key", 0), uint32_t(123));

		cfg.set_uint32("u_key", 456);
		EXPECT_EQ(cfg.get_uint32("u_key", 0), uint32_t(456));
		ASSERT_TRUE(cfg.exists("u_key"));
	}

	// bool
	{
		cfg.set_bool("b_key", true);
		EXPECT_EQ(cfg.get_bool("b_key", false), true);

		cfg.set_bool("b_key", false);
		EXPECT_EQ(cfg.get_bool("b_key", true), false);
		ASSERT_TRUE(cfg.exists("b_key"));
	}

	ASSERT_FALSE(cfg.exists("exists_test"));
}


TEST(Config, test_config_is_loaded_and_saved)
{
	// save
	{
		Config cfg{TEST_FILE};

		cfg.set_string("key", "value");
		ASSERT_TRUE(cfg.save());
	}

	// load
	{
		Config cfg{TEST_FILE};

		ASSERT_TRUE(cfg.load());

		EXPECT_EQ(cfg.get_string("key", ""), "value");
	}

	EXPECT_EQ(unlink(TEST_FILE.c_str()), 0);
}


TEST(Config, test_that_default_key_values_are_retrieved)
{
	Config cfg{RTNG_CONFIG_FILE};

	EXPECT_EQ(cfg.get_bool("key", true), true);

	EXPECT_EQ(cfg.get_string("key", "asdf"), "asdf");

	EXPECT_EQ(cfg.get_uint32("key",123), uint32_t(123));
}
