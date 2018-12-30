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

#include <radiotray-ng/common.hpp>
#include <radiotray-ng/event_bus/event_bus.hpp>
#include <gtest/gtest.h>
#include <iostream>


class EventBusTest : public ::testing::Test
{
public:
	void on_song_changed(const IEventBus::event& ev, IEventBus::event_data_t& data)
	{
		EXPECT_EQ(ev, IEventBus::event::tags_changed);
		data[SONG_KEY] = "on_song_changed";
	}

	void on_state_changed_first(const IEventBus::event& ev, IEventBus::event_data_t& data)
	{
		EXPECT_EQ(ev, IEventBus::event::state_changed);
		data[STATE_KEY] += "1";
	}

	void on_state_changed_default(const IEventBus::event& ev, IEventBus::event_data_t& data)
	{
		EXPECT_EQ(ev, IEventBus::event::state_changed);
		data[STATE_KEY] += "2";
	}

	void on_state_changed_last(const IEventBus::event& ev, IEventBus::event_data_t& data)
	{
		EXPECT_EQ(ev, IEventBus::event::state_changed);
		data[STATE_KEY] += "3";
	}

	EventBus evm;
};


TEST_F(EventBusTest, test_that_subscriber_is_called_for_its_event_and_it_can_modify_the_message_data)
{
	ASSERT_TRUE(this->evm.subscribe(IEventBus::event::tags_changed,
		std::bind(&EventBusTest::on_song_changed, this, std::placeholders::_1, std::placeholders::_2), IEventBus::event_pos::any));

	IEventBus::event_data_t data;

	data[SONG_KEY] = "song title";

	this->evm.publish(IEventBus::event::tags_changed, data);

	EXPECT_EQ(data[SONG_KEY], "on_song_changed");

	// should not modify value... (todo: test properly)
	this->evm.publish_only(IEventBus::event::tags_changed, SONG_KEY, "");

	EXPECT_NE(data[SONG_KEY], "");
}


TEST_F(EventBusTest, test_that_subscription_fails_for_first_last_if_already_occupied)
{
	ASSERT_TRUE(this->evm.subscribe(IEventBus::event::tags_changed,
			std::bind(&EventBusTest::on_song_changed, this, std::placeholders::_1, std::placeholders::_2), IEventBus::event_pos::first));
	ASSERT_TRUE(this->evm.subscribe(IEventBus::event::tags_changed,
			std::bind(&EventBusTest::on_song_changed, this, std::placeholders::_1, std::placeholders::_2), IEventBus::event_pos::last));

	ASSERT_FALSE(this->evm.subscribe(IEventBus::event::tags_changed,
		std::bind(&EventBusTest::on_song_changed, this, std::placeholders::_1, std::placeholders::_2), IEventBus::event_pos::first));
	ASSERT_FALSE(this->evm.subscribe(IEventBus::event::tags_changed,
		std::bind(&EventBusTest::on_song_changed, this, std::placeholders::_1, std::placeholders::_2), IEventBus::event_pos::last));
}


TEST_F(EventBusTest, test_that_subscribers_are_called_in_order)
{
	ASSERT_TRUE(this->evm.subscribe(IEventBus::event::state_changed,
			std::bind(&EventBusTest::on_state_changed_first, this, std::placeholders::_1, std::placeholders::_2), IEventBus::event_pos::first));
	ASSERT_TRUE(this->evm.subscribe(IEventBus::event::state_changed,
			std::bind(&EventBusTest::on_state_changed_last, this, std::placeholders::_1, std::placeholders::_2), IEventBus::event_pos::last));
	ASSERT_TRUE(this->evm.subscribe(IEventBus::event::state_changed,
			std::bind(&EventBusTest::on_state_changed_default, this, std::placeholders::_1, std::placeholders::_2), IEventBus::event_pos::any));
	ASSERT_TRUE(this->evm.subscribe(IEventBus::event::state_changed,
			std::bind(&EventBusTest::on_state_changed_default, this, std::placeholders::_1, std::placeholders::_2), IEventBus::event_pos::any));

	IEventBus::event_data_t data;

	data[STATE_KEY] = "0";

	this->evm.publish(IEventBus::event::state_changed, data);

	EXPECT_EQ(data[STATE_KEY], "01223");
}
