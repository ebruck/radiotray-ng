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

#include <radiotray-ng/i_gui.hpp>
#include <radiotray-ng/i_event_bus.hpp>
#include <radiotray-ng/file_monitor.hpp>

#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>
#include <memory>

class IConfig;
class IBookmarks;
class IPlayer;
class IRadioTrayNG;


class AppindicatorGui final : public IGui, public std::enable_shared_from_this<AppindicatorGui>
{
public:
	AppindicatorGui(std::shared_ptr<IConfig> config, std::shared_ptr<IRadioTrayNG> radiotray_ng, std::shared_ptr<IBookmarks> bookmarks,
		std::shared_ptr<IEventBus> event_bus);

	AppindicatorGui() = delete;

	void run(int argc, char* argv[]);

	void reload_bookmarks();

	void stop();

private:
	// event bus handlers
	void on_tags_event(const IEventBus::event& ev, IEventBus::event_data_t& data);
	void on_state_event(const IEventBus::event& ev, IEventBus::event_data_t& data);
	void on_volume_event(const IEventBus::event& ev, IEventBus::event_data_t& data);
	void on_station_error_event(const IEventBus::event& ev, IEventBus::event_data_t& data);

	// menu event handlers
	static void on_about_menu_item(GtkWidget* widget, gpointer data);
	static void on_station_menu_item(GtkWidget* widget, gpointer data);
	static void on_action_menu_item(GtkWidget* widget, gpointer data);
	static void on_status_menu_item(GtkWidget* widget, gpointer data);
	static void on_reload_bookmarks_menu_item(GtkWidget* widget, gpointer data);
	static void on_bookmark_editor_menu_item(GtkWidget* widget, gpointer data);
	static void on_sleep_timer_menu_item(GtkWidget* widget, gpointer data);
	static gboolean on_timer_event(gpointer data);
	static gboolean on_file_monitor_timer_event(gpointer data);
	static void on_indicator_scrolled(GtkWidget* w, gint delta ,GdkScrollDirection direction, gpointer data);

	void build_menu();
	void build_action_menu_item();
	void build_status_menu_item();
	void build_volume_menu_item();
	void build_bookmarks_menu_item();
	void build_preferences_menu();
	void build_sleep_timer_menu_item();
	void build_about_menu_item();
	void build_quit_menu_item();
	void build_root_bookmarks_menu_item();

	void add_separator(GtkWidget* menu);

	void update_status_menu_item(const std::string& state);
	void update_action_menu_item(const std::string& state);
	void update_volume_menu_item();

	bool sleep_timer_dialog();

	std::shared_ptr<IRadioTrayNG> radiotray_ng;
	std::shared_ptr<IBookmarks>   bookmarks;
	std::unique_ptr<radiotray_ng::FileMonitor> bookmarks_monitor;
	std::shared_ptr<IConfig>      config;
	std::shared_ptr<IEventBus>    event_bus;

	struct menu_item_data
	{
		std::shared_ptr<AppindicatorGui> gui;
		std::string group;
		std::string station;

		static void free_cb_data(gpointer* p,  GClosure* /*closure*/)
		{
			delete reinterpret_cast<menu_item_data*>(p);
		}
	};

	AppIndicator*     appindicator;
	GtkWidget*        menu;
	GtkWidget*        action_menu_item;
	GtkWidget*        volume_menu_item;
	GtkWidget*        status_menu_item;
	GtkCheckMenuItem* sleep_timer_menu_item;
	guint             sleep_timer_id;
	guint             file_monitor_timer_id;
	std::string       resource_path;
	bool              ignore_sleep_timer_toggle = false;
	GtkClipboard*     clipboard = nullptr;
};
