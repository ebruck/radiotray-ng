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

#include <radiotray-ng/i_config.hpp>
#include <radiotray-ng/i_event_bus.hpp>
#include <radiotray-ng/i_player.hpp>

#include <gst/gst.h>


class Player final : public IPlayer
{
public:
	Player(std::shared_ptr<IConfig> config, std::shared_ptr<IEventBus> event_bus);

	virtual ~Player();

	bool play(const playlist_t& playlist);

	void stop();

	void volume(uint32_t percent);

    void mute();

    void unmute();

    bool is_muted();

private:
	void gst_start();
	void gst_stop();

	static gboolean handle_messages_cb(GstBus* bus, GstMessage* message, gpointer user_data);

	static void notify_source_cb(GObject* obj, GParamSpec* param, gpointer user_data);

	static gboolean timer_cb(GstClock* clock, GstClockTime time, GstClockID id, gpointer user_data);

	static void for_each_tag_cb(const GstTagList* list, const gchar* tag, gpointer user_data);

	bool play_next();

	GstElement* pipeline;
	GstElement* souphttpsrc;
	GstClock*   clock;
	GstClockID  clock_id;
	bool        buffering = false;

	playlist_t current_playlist;

	std::shared_ptr<IEventBus> event_bus;
	std::shared_ptr<IConfig> config;

	GstBus* gst_bus;
};
