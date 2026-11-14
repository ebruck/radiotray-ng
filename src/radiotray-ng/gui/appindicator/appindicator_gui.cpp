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

#include "appindicator_gui.hpp"
#include <algorithm>
#include <radiotray-ng/i_config.hpp>
#include <radiotray-ng/i_bookmarks.hpp>
#include <radiotray-ng/i_radiotray_ng.hpp>
#include <rtng_user_agent.hpp>
#include <gdk-pixbuf/gdk-pixbuf.h>


namespace
{
	GtkWidget* create_station_menu_item(const IBookmarks::station_data_t& station)
	{
		if (!station.image.empty())
		{
			const std::string image_path{radiotray_ng::word_expand(station.image)};
			GError* error = nullptr;
			GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file_at_scale(image_path.c_str(), 36, 36, TRUE, &error);

			if (pixbuf != nullptr)
			{
				GtkWidget* menu_item = gtk_image_menu_item_new_with_label(station.name.c_str());
				GtkWidget* image = gtk_image_new_from_pixbuf(pixbuf);

				gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item), image);
				gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menu_item), TRUE);
				gtk_widget_show(image);
				g_object_unref(pixbuf);

				if (error != nullptr)
				{
					g_error_free(error);
				}

				return menu_item;
			}

			if (error != nullptr)
			{
				g_error_free(error);
			}
		}

		return gtk_menu_item_new_with_label(station.name.c_str());
	}
}


AppindicatorGui::AppindicatorGui(std::shared_ptr<IConfig> config, std::shared_ptr<IRadioTrayNG> radiotray_ng,
	                             std::shared_ptr<IBookmarks> bookmarks, std::shared_ptr<IEventBus> event_bus)
	: radiotray_ng(std::move(radiotray_ng))
	, bookmarks(std::move(bookmarks))
	, config(std::move(config))
	, event_bus(std::move(event_bus))
	, direct_sni(nullptr)
	, menu(nullptr)
	, action_menu_item(nullptr)
	, volume_menu_item(nullptr)
	, status_menu_item(nullptr)
	, sleep_timer_menu_item(nullptr)
	, sleep_timer_id(0)
	, file_monitor_timer_id(0)
{
	// monitor bookmark changes?
	if (this->config->get_bool(FILE_MONITOR_KEY, DEFAULT_FILE_MONITOR_VALUE))
	{
		this->bookmarks_monitor = std::make_unique<radiotray_ng::FileMonitor>(this->config->get_string(BOOKMARKS_KEY, RTNG_DEFAULT_BOOKMARK_FILE));
		this->file_monitor_timer_id = g_timeout_add(this->config->get_uint32(FILE_MONITOR_INTERVAL_KEY, DEFAULT_FILE_MONITOR_INTERVAL_VALUE) * 1000,
			on_file_monitor_timer_event, this);
	}

	this->event_bus->subscribe(IEventBus::event::tags_changed, std::bind(&AppindicatorGui::on_tags_event, this, std::placeholders::_1,
		std::placeholders::_2), IEventBus::event_pos::any);

	this->event_bus->subscribe(IEventBus::event::state_changed, std::bind(&AppindicatorGui::on_state_event, this, std::placeholders::_1,
		std::placeholders::_2), IEventBus::event_pos::any);

	this->event_bus->subscribe(IEventBus::event::volume_changed, std::bind(&AppindicatorGui::on_volume_event, this, std::placeholders::_1,
		std::placeholders::_2), IEventBus::event_pos::any);

	this->event_bus->subscribe(IEventBus::event::station_error, std::bind(&AppindicatorGui::on_station_error_event, this, std::placeholders::_1,
		std::placeholders::_2), IEventBus::event_pos::any);
}


void AppindicatorGui::on_state_event(const IEventBus::event& /*ev*/, IEventBus::event_data_t& data)
{
	const std::string state{data[STATE_KEY]};

	this->update_status_menu_item(state);
	this->update_action_menu_item(state);

	if (state == STATE_PLAYING || state == STATE_BUFFERING || state == STATE_CONNECTING)
	{
		if (this->direct_sni != nullptr)
		{
			this->direct_sni->set_icon(radiotray_ng::word_expand(this->config->get_string(
				RADIOTRAY_NG_ICON_ON_KEY, DEFAULT_RADIOTRAY_NG_ICON_ON_VALUE)));
		}
		return;
	}

	if (state == STATE_STOPPED)
	{
		if (this->direct_sni != nullptr)
		{
			this->direct_sni->set_icon(radiotray_ng::word_expand(this->config->get_string(
				RADIOTRAY_NG_ICON_OFF_KEY, DEFAULT_RADIOTRAY_NG_ICON_OFF_VALUE)));
		}
		return;
	}
}


void AppindicatorGui::on_volume_event(const IEventBus::event& /*ev*/, IEventBus::event_data_t& data)
{
	if (data.count(VOLUME_LEVEL_KEY))
	{
		this->radiotray_ng->set_volume(data[VOLUME_LEVEL_KEY]);
	}

	this->update_volume_menu_item();
}


void AppindicatorGui::on_station_error_event(const IEventBus::event& /*ev*/, IEventBus::event_data_t& /*data*/)
{
	this->update_action_menu_item(this->radiotray_ng->get_state());
}


void AppindicatorGui::on_tags_event(const IEventBus::event& /*ev*/, IEventBus::event_data_t& data)
{
	this->update_status_menu_item(this->radiotray_ng->get_state());

	if (data.count(TAG_BITRATE) || data.count(TAG_CODEC))
	{
		this->update_volume_menu_item();
	}
}


void AppindicatorGui::on_station_menu_item(GtkWidget* /*widget*/, gpointer data)
{
	auto mid = static_cast<menu_item_data*>(data);

	mid->gui->radiotray_ng->play(mid->group, mid->station);
}


void AppindicatorGui::on_indicator_scrolled(GtkWidget* /*widget*/, gint /*delta*/, GdkScrollDirection direction, gpointer data)
{
	auto app = static_cast<AppindicatorGui*>(data);

	switch(direction)
	{
		case GDK_SCROLL_UP:
		case GDK_SCROLL_RIGHT:
			app->radiotray_ng->volume_up();
			break;

		case GDK_SCROLL_DOWN:
		case GDK_SCROLL_LEFT:
			app->radiotray_ng->volume_down();
			break;

		default:
			break;
	}
}


void AppindicatorGui::position_native_menu(
    GtkMenu* menu,
    gint* x,
    gint* y,
    gboolean* push_in,
    gpointer data)
{
    auto app = static_cast<AppindicatorGui*>(data);

    GtkRequisition minimum_size{};
    GtkRequisition natural_size{};

    gtk_widget_get_preferred_size(
        GTK_WIDGET(menu),
        &minimum_size,
        &natural_size);

    GdkDisplay* display = gdk_display_get_default();

    if (display == nullptr)
    {
        *x = app->native_menu_x;
        *y = app->native_menu_y;
        *push_in = TRUE;
        return;
    }

    // ALWAYS use the monitor containing the tray click/pointer.
    // Do not move the menu to a monitor above or below it.
    GdkMonitor* monitor =
        gdk_display_get_monitor_at_point(
            display,
            app->native_menu_x,
            app->native_menu_y);

    GdkRectangle geometry{};
    GdkRectangle workarea{};

    if (monitor != nullptr)
    {
        gdk_monitor_get_geometry(monitor, &geometry);
        gdk_monitor_get_workarea(monitor, &workarea);
        gtk_menu_place_on_monitor(menu, monitor);
    }
    else
    {
        geometry.x = 0;
        geometry.y = 0;
        geometry.width = gdk_screen_width();
        geometry.height = gdk_screen_height();
        workarea = geometry;
    }

    constexpr gint margin = 2;
    constexpr gint icon_half_width = 10;

    // GNOME tray menus are normally right-aligned to the indicator.
    gint menu_x =
        app->native_menu_x -
        natural_size.width +
        icon_half_width;

    const gint work_right =
        workarea.x + workarea.width;

    if (menu_x + natural_size.width > work_right - margin)
        menu_x = work_right - natural_size.width - margin;

    if (menu_x < workarea.x + margin)
        menu_x = workarea.x + margin;

    const gint geometry_bottom =
        geometry.y + geometry.height;

    const gint work_bottom =
        workarea.y + workarea.height;

    // Detect whether GNOME has reserved panel space at the top or bottom.
    const gboolean top_panel =
        workarea.y > geometry.y &&
        app->native_menu_y <= workarea.y + 32;

    const gboolean bottom_panel =
        work_bottom < geometry_bottom &&
        app->native_menu_y >= work_bottom - 32;

    gint menu_y = 0;

    if (top_panel)
    {
        // Directly BELOW the GNOME top panel.
        menu_y = workarea.y + margin;
    }
    else if (bottom_panel)
    {
        // Directly ABOVE a bottom panel.
        menu_y =
            work_bottom -
            natural_size.height -
            margin;
    }
    else if (app->native_menu_y <
             geometry.y + (geometry.height / 2))
    {
        // Fallback for panels that do not reserve a work area.
        menu_y =
            app->native_menu_y + 18;
    }
    else
    {
        menu_y =
            app->native_menu_y -
            natural_size.height -
            18;
    }

    if (menu_y < workarea.y + margin)
        menu_y = workarea.y + margin;

    if (menu_y + natural_size.height > work_bottom - margin)
        menu_y = work_bottom - natural_size.height - margin;

    *x = menu_x;
    *y = menu_y;
    *push_in = TRUE;
}


void AppindicatorGui::show_native_menu(gint x, gint y)
{
    // SecondaryActivate/ContextMenu supply genuine GNOME Shell coordinates.
    // KEEP them.  Only the DBusMenu single-click bridge uses -1/-1, because
    // DBusMenu's root "opened" event contains no pointer coordinates.
    if (x < 0 || y < 0)
    {
        GdkDisplay* display = gdk_display_get_default();

        if (display != nullptr)
        {
            GdkSeat* seat =
                gdk_display_get_default_seat(display);

            if (seat != nullptr)
            {
                GdkDevice* pointer =
                    gdk_seat_get_pointer(seat);

                if (pointer != nullptr)
                {
                    gint pointer_x = 0;
                    gint pointer_y = 0;

                    gdk_device_get_position(
                        pointer,
                        nullptr,
                        &pointer_x,
                        &pointer_y);

                    x = pointer_x;
                    y = pointer_y;
                }
            }
        }
    }

    if (x < 0 || y < 0)
    {
        x = 0;
        y = 0;
    }

    this->native_menu_x = x;
    this->native_menu_y = y;

    const std::string state{
        this->radiotray_ng->get_state()
    };

    this->update_action_menu_item(state);
    this->update_status_menu_item(state);
    this->update_volume_menu_item();

    gtk_menu_popdown(GTK_MENU(this->menu));
    gtk_widget_show_all(this->menu);

    gtk_menu_popup(
        GTK_MENU(this->menu),
        nullptr,
        nullptr,
        &AppindicatorGui::position_native_menu,
        this,
        0,
        GDK_CURRENT_TIME);
}

void AppindicatorGui::on_status_menu_item(GtkWidget* /*widget*/, gpointer data)
{
	auto app = static_cast<AppindicatorGui*>(data);

	if (app->clipboard)
	{
		std::string clipboard_text = app->radiotray_ng->get_title() + "\n" + app->radiotray_ng->get_artist();

		gtk_clipboard_set_text(app->clipboard, clipboard_text.c_str(), -1);
	}
	else
	{
		LOG(error) << "clipboard unavailable!";
	}
}


void AppindicatorGui::on_action_menu_item(GtkWidget* /*widget*/, gpointer data)
{
	auto app = static_cast<AppindicatorGui*>(data);

	std::string state{app->radiotray_ng->get_state()};

	if (state == STATE_STOPPED)
	{
		app->update_action_menu_item(STATE_PLAYING);

		app->radiotray_ng->play();

		return;
	}

	if (state == STATE_PLAYING || state == STATE_BUFFERING)
	{
		app->radiotray_ng->stop();

		app->update_action_menu_item(STATE_STOPPED);

		return;
	}
}


void AppindicatorGui::build_root_bookmarks_menu_item()
{
	// append root items
	std::vector<IBookmarks::station_data_t> root_stations;

	if (this->bookmarks->get_group_stations(ROOT_BOOKMARK_GROUP, root_stations))
	{
		for (const IBookmarks::station_data_t& s : root_stations)
		{
			GtkWidget* menu_item = create_station_menu_item(s);
			gtk_menu_shell_append(GTK_MENU_SHELL(this->menu), menu_item);

			menu_item_data* cb_data = new menu_item_data{this->shared_from_this(), ROOT_BOOKMARK_GROUP, s.name};

			g_signal_connect_data(menu_item, "activate",
				GCallback(on_station_menu_item), cb_data, GClosureNotify(&menu_item_data::free_cb_data), GConnectFlags(0));

			gtk_widget_show(menu_item);
		}
	}
}


void AppindicatorGui::build_bookmarks_menu_item()
{
	this->add_separator(this->menu);

	const bool root_bottom = this->config->get_bool(ROOT_GROUP_BOTTOM_POS_KEY, DEFAULT_ROOT_GROUP_BOTTOM_POS_VALUE);

	if (!root_bottom)
	{
		this->build_root_bookmarks_menu_item();
	}

	for (size_t i = 0; i < this->bookmarks->size(); ++i)
	{
		const std::string& group = (*this->bookmarks)[i].group;

		// skip root as it's not a submenu
		if (group != ROOT_BOOKMARK_GROUP)
		{
			GtkWidget* menu_items = gtk_menu_item_new_with_label(group.c_str());
			gtk_menu_shell_append(GTK_MENU_SHELL(this->menu), menu_items);

			GtkWidget* sub_menu_items = gtk_menu_new();
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_items), sub_menu_items);

			for (const auto& s : (*this->bookmarks)[i].stations)
			{
				GtkWidget* sub_menu_item = create_station_menu_item(s);
				gtk_menu_shell_append(GTK_MENU_SHELL(sub_menu_items), sub_menu_item);

				auto cb_data = new menu_item_data{this->shared_from_this(), group, s.name};

				g_signal_connect_data(sub_menu_item, "activate",
					GCallback(on_station_menu_item), cb_data, GClosureNotify(&menu_item_data::free_cb_data), GConnectFlags(0));

				gtk_widget_show(sub_menu_item);
			}

			gtk_widget_show(menu_items);
		}
	}

	if (root_bottom)
	{
		this->build_root_bookmarks_menu_item();
	}
}


void AppindicatorGui::add_separator(GtkWidget* menu)
{
	if (!this->config->get_bool(COMPACT_MENU_KEY, DEFAULT_COMPACT_MENU_VALUE))
	{
		GtkWidget* menu_item = gtk_separator_menu_item_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
		gtk_widget_show(menu_item);
	}
}


void AppindicatorGui::update_volume_menu_item()
{
	std::string volume_info{std::string("Volume: ") + this->radiotray_ng->get_volume() + std::string("%")};

	if (this->config->get_bool(TAG_INFO_VERBOSE_KEY, DEFAULT_TAG_INFO_VERBOSE_VALUE))
	{
		std::string bitrate = this->radiotray_ng->get_bitrate();
		if (!bitrate.empty())
		{
			std::string bitrate_info{std::string(", Bitrate: ") + bitrate};

			volume_info += bitrate_info;
		}

		std::string codec = this->radiotray_ng->get_codec();
		if (!codec.empty())
		{
			std::string codec_info{std::string("\nCodec: ") + codec};

			volume_info += codec_info;
		}
	}

	gtk_menu_item_set_label(GTK_MENU_ITEM(this->volume_menu_item), volume_info.c_str());
	gtk_widget_set_sensitive(this->volume_menu_item, FALSE);
}


void AppindicatorGui::update_action_menu_item(const std::string& state)
{
	std::string action_text;

	if (state == STATE_STOPPED)
	{
		action_text = "Turn On";

		if (!this->radiotray_ng->get_station().empty())
		{
			action_text += " \"" + this->radiotray_ng->get_station() + "\"";
		}

		gtk_menu_item_set_label(GTK_MENU_ITEM(this->action_menu_item), action_text.c_str());

		// Is the station still in our bookmarks?
		gtk_widget_set_sensitive(this->action_menu_item,
			this->bookmarks->station_exists(this->radiotray_ng->get_group(), this->radiotray_ng->get_station()));

		return;
	}

	if (state == STATE_PLAYING || state == STATE_BUFFERING)
	{
		action_text = "Turn Off";

		if (!this->radiotray_ng->get_station().empty())
		{
			action_text += " \"" + this->radiotray_ng->get_station() + "\"";
		}

		gtk_menu_item_set_label(GTK_MENU_ITEM(this->action_menu_item), action_text.c_str());
		gtk_widget_set_sensitive(this->action_menu_item, TRUE);
		return;
	}
}


void AppindicatorGui::update_status_menu_item(const std::string& state)
{
	const bool copy_enabled = this->config->get_bool(TRACK_INFO_COPY_KEY, DEFAULT_TRACK_INFO_COPY_VALUE);

	if (state == STATE_PLAYING)
	{
		std::string title{this->radiotray_ng->get_title()};
		std::string artist{this->radiotray_ng->get_artist()};
		std::string status_text;

		// at least a title will be there...
		if (title.empty())
		{
			status_text = std::string("Playing");

			if (copy_enabled)
			{
				gtk_widget_set_sensitive(this->status_menu_item, FALSE);
			}
		}
		else
		{
			const bool wrap_enabled = this->config->get_bool(WRAP_TRACK_INFO_KEY, DEFAULT_WRAP_TRACK_INFO_VALUE);

			const uint32_t wrap_len = this->config->get_uint32(WRAP_TRACK_INFO_LEN_KEY, DEFAULT_WRAP_TRACK_INFO_LEN_VALUE);

			status_text = ((wrap_enabled) ? radiotray_ng::word_wrap(title, wrap_len) : title);

			if (!artist.empty())
			{
				status_text += "\n" + ((wrap_enabled) ? radiotray_ng::word_wrap(artist, wrap_len) : artist);
			}

			if (copy_enabled)
			{
				gtk_widget_set_sensitive(this->status_menu_item, TRUE);
			}

			// if wrap is disabled then we truncate at the wrap_len to prevent wide menus...
			if (!wrap_enabled)
			{
				if (status_text.length() > wrap_len)
				{
					status_text.resize(wrap_len);
					status_text += "...";
				}
			}
		}

		gtk_menu_item_set_label(GTK_MENU_ITEM(this->status_menu_item), status_text.c_str());
		return;
	}

	if (state == STATE_STOPPED)
	{
		gtk_menu_item_set_label(GTK_MENU_ITEM(this->status_menu_item), std::string("Stopped").c_str());

		if (copy_enabled)
		{
			gtk_widget_set_sensitive(this->status_menu_item, FALSE);
		}
		return;
	}
}


void AppindicatorGui::build_status_menu_item()
{
	this->add_separator(this->menu);

	this->status_menu_item = gtk_menu_item_new_with_label("status");
	gtk_menu_shell_append(GTK_MENU_SHELL(this->menu), this->status_menu_item);
	gtk_widget_set_sensitive(this->status_menu_item, FALSE);

	if (this->config->get_bool(TRACK_INFO_COPY_KEY, DEFAULT_TRACK_INFO_COPY_VALUE))
	{
		if (this->clipboard == nullptr)
		{
			this->clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
		}

		g_signal_connect(G_OBJECT(this->status_menu_item), "activate", G_CALLBACK(on_status_menu_item), gpointer(this));
	}

	gtk_widget_show(this->status_menu_item);

	this->update_status_menu_item(STATE_STOPPED);
}


void AppindicatorGui::build_action_menu_item()
{
	this->action_menu_item = gtk_menu_item_new_with_label("action");
	gtk_menu_shell_append(GTK_MENU_SHELL(this->menu), this->action_menu_item);
	gtk_widget_show(this->action_menu_item);

	g_signal_connect(G_OBJECT(this->action_menu_item), "activate", G_CALLBACK(on_action_menu_item), gpointer(this));

	this->update_action_menu_item(STATE_STOPPED);
}


void AppindicatorGui::build_volume_menu_item()
{
	this->add_separator(this->menu);

	this->volume_menu_item = gtk_menu_item_new_with_label("volume");
	gtk_menu_shell_append(GTK_MENU_SHELL(this->menu), this->volume_menu_item);
	gtk_widget_show(this->volume_menu_item);

	this->update_volume_menu_item();
}


void AppindicatorGui::build_preferences_menu()
{
	this->add_separator(this->menu);

	GtkWidget* menu_items = gtk_menu_item_new_with_label("Preferences");
	gtk_menu_shell_append(GTK_MENU_SHELL(this->menu), menu_items);
	gtk_widget_show(menu_items);

	GtkWidget* sub_menu_items = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_items), sub_menu_items);

	GtkWidget* sub_menu_item = gtk_menu_item_new_with_label("Bookmark Editor...");
	gtk_menu_shell_append(GTK_MENU_SHELL(sub_menu_items), sub_menu_item);
	g_signal_connect(G_OBJECT(sub_menu_item), "activate", G_CALLBACK(on_bookmark_editor_menu_item), gpointer(this));
	gtk_widget_show(sub_menu_item);

	sub_menu_item = gtk_menu_item_new_with_label("Reload Bookmarks");
	gtk_menu_shell_append(GTK_MENU_SHELL(sub_menu_items), sub_menu_item);
	g_signal_connect(G_OBJECT(sub_menu_item), "activate", G_CALLBACK(on_reload_bookmarks_menu_item), gpointer(this));
	gtk_widget_show(sub_menu_item);

	sub_menu_item = gtk_menu_item_new_with_label("Buffering...");
	gtk_menu_shell_append(GTK_MENU_SHELL(sub_menu_items), sub_menu_item);
	g_signal_connect(G_OBJECT(sub_menu_item), "activate", G_CALLBACK(on_buffering_menu_item), gpointer(this));
	gtk_widget_show(sub_menu_item);
}


void AppindicatorGui::on_buffering_menu_item(GtkWidget* /*widget*/, gpointer data)
{
	auto app = static_cast<AppindicatorGui*>(data);

	auto dialog = gtk_dialog_new_with_buttons("Buffering Settings",
		nullptr, GTK_DIALOG_MODAL,
		"_Cancel", GTK_RESPONSE_CANCEL,
		"_Save", GTK_RESPONSE_ACCEPT,
		nullptr);

	auto content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_container_set_border_width(GTK_CONTAINER(content), 12);
	gtk_box_set_spacing(GTK_BOX(content), 10);

	auto row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	auto label = gtk_label_new("Buffer duration (seconds):");
	auto spin = gtk_spin_button_new_with_range(1.0, 120.0, 1.0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spin), TRUE);
	const uint32_t saved = app->config->get_uint32(BUFFER_DURATION_KEY, DEFAULT_BUFFER_DURATION_VALUE);
	const uint32_t current = std::min(std::max(saved, uint32_t(1)), uint32_t(120));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), current);
	gtk_box_pack_start(GTK_BOX(row), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(row), spin, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(content), row, FALSE, FALSE, 0);

	auto help = gtk_label_new(
		"Default: 2 seconds. Try 10-15 seconds if stations stutter.\n"
		"A larger buffer may improve stability but increases startup delay\n"
		"and puts playback further behind the live broadcast.\n"
		"Saving reconnects the current station to apply the change.");
	gtk_label_set_xalign(GTK_LABEL(help), 0.0f);
	gtk_box_pack_start(GTK_BOX(content), help, FALSE, FALSE, 0);

	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_widget_show_all(dialog);

	const gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	const uint32_t seconds = static_cast<uint32_t>(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin)));
	gtk_widget_destroy(dialog);

	if (response != GTK_RESPONSE_ACCEPT || seconds == saved)
	{
		return;
	}

	app->config->set_uint32(BUFFER_DURATION_KEY, seconds);
	if (!app->config->save())
	{
		app->event_bus->publish_only(IEventBus::event::message, MESSAGE_KEY, "Unable to save buffering setting");
		return;
	}

	const std::string state{app->radiotray_ng->get_state()};
	if (state == STATE_PLAYING || state == STATE_BUFFERING || state == STATE_CONNECTING)
	{
		app->radiotray_ng->play();
	}
	else
	{
		app->event_bus->publish_only(IEventBus::event::message, MESSAGE_KEY,
			"Buffer set to " + std::to_string(seconds) + " seconds");
	}
}


void AppindicatorGui::build_sleep_timer_menu_item()
{
	this->sleep_timer_menu_item = (GtkCheckMenuItem*)gtk_check_menu_item_new_with_label("Sleep Timer");

	// toggle before we hook up callback as a reload loses this state...
	gtk_check_menu_item_set_active(this->sleep_timer_menu_item, this->sleep_timer_id);

	gtk_menu_shell_append(GTK_MENU_SHELL(this->menu), (GtkWidget *) this->sleep_timer_menu_item);
	g_signal_connect(G_OBJECT((GtkWidget *) this->sleep_timer_menu_item), "activate", G_CALLBACK(on_sleep_timer_menu_item), gpointer(this));
	gtk_widget_show((GtkWidget *) this->sleep_timer_menu_item);
}


void AppindicatorGui::build_about_menu_item()
{
	auto menu_items = gtk_menu_item_new_with_label("About");
	gtk_menu_shell_append(GTK_MENU_SHELL (this->menu), menu_items);
	g_signal_connect(menu_items, "activate", G_CALLBACK(&AppindicatorGui::on_about_menu_item), this);
	gtk_widget_show(menu_items);
}


void AppindicatorGui::build_quit_menu_item()
{
	auto menu_items = gtk_menu_item_new_with_label("Quit");
	gtk_menu_shell_append(GTK_MENU_SHELL (this->menu), menu_items);
	g_signal_connect(menu_items, "activate", GCallback(gtk_main_quit), nullptr);
	gtk_widget_show(menu_items);
}


void AppindicatorGui::build_menu()
{
	this->menu = gtk_menu_new();

	if (!this->config->get_bool(INVERT_MENU_KEY, DEFAULT_INVERT_MENU_VALUE))
	{
		// action (play/stop)
		this->build_action_menu_item();

		// status (tags)
		this->build_status_menu_item();

		// volume etc.
		this->build_volume_menu_item();

		// bookmarks
		this->build_bookmarks_menu_item();

		// preferences
		this->build_preferences_menu();

		// sleep timer
		this->build_sleep_timer_menu_item();

		// about etc.
		this->add_separator(this->menu);

		// about
		this->build_about_menu_item();

		// quit
		this->build_quit_menu_item();
	}
	else
	{
		// sleep timer
		this->build_sleep_timer_menu_item();

		// preferences
		this->build_preferences_menu();

		// bookmarks
		this->build_bookmarks_menu_item();

		// volume etc.
		this->build_volume_menu_item();

		// status (tags)
		this->build_status_menu_item();

		// action (play/stop)
		this->build_action_menu_item();

		// about etc.
		this->add_separator(this->menu);

		// about
		this->build_about_menu_item();

		// quit
		this->build_quit_menu_item();
	}

}


gboolean AppindicatorGui::on_timer_event(gpointer data)
{
	auto app = static_cast<AppindicatorGui*>(data);

	if (app->radiotray_ng->get_state() == STATE_PLAYING)
	{
		app->radiotray_ng->stop();

		app->update_action_menu_item(STATE_STOPPED);
	}

	gtk_check_menu_item_set_active(app->sleep_timer_menu_item, FALSE);

	app->event_bus->publish_only(IEventBus::event::message, MESSAGE_KEY, "Sleep timer expired");

	return FALSE;
}


gboolean AppindicatorGui::on_file_monitor_timer_event(gpointer data)
{
	auto app = static_cast<AppindicatorGui*>(data);

	if (app->bookmarks_monitor->changed())
	{
		app->event_bus->publish_only(IEventBus::event::message, MESSAGE_KEY, "Bookmarks changed on disk");
	}

	return TRUE;
}


bool AppindicatorGui::sleep_timer_dialog()
{
	auto dialog = gtk_dialog_new_with_buttons("Sleep Timer",
		nullptr,
		GTK_DIALOG_DESTROY_WITH_PARENT,
		"Cancel",
		GTK_RESPONSE_REJECT,
		"OK",
		GTK_RESPONSE_ACCEPT,
		nullptr);

	auto entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry), 4);
	auto label = gtk_label_new("Minutes:");
	auto hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

	gtk_entry_set_text(GTK_ENTRY(entry), this->config->get_string(SLEEP_TIMER_KEY, std::to_string(DEFAULT_SLEEP_TIMER_VALUE)).c_str());

	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(label), false, true, 5);
	gtk_box_pack_end(GTK_BOX(hbox), GTK_WIDGET(entry), true, true, 5);
	gtk_box_pack_end(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), GTK_WIDGET(hbox), true, true, 20);

	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_widget_show_all(GTK_WIDGET(dialog));
	gint ret = gtk_dialog_run(GTK_DIALOG(dialog));

	std::string timeout = gtk_entry_get_text(GTK_ENTRY(entry));
	gtk_widget_destroy(dialog);

	if (ret == GTK_RESPONSE_ACCEPT)
	{
		if (!timeout.empty())
		{
			try
			{
				this->config->set_uint32(SLEEP_TIMER_KEY, std::stoul(timeout));
			}
			catch(std::invalid_argument& ex)
			{
				this->event_bus->publish_only(IEventBus::event::message, MESSAGE_KEY, "Invalid sleep timer");

				return false;
			}

			return true;
		}
	}

	return false;
}


void AppindicatorGui::on_sleep_timer_menu_item(GtkWidget* /*widget*/, gpointer data)
{
	auto app = static_cast<AppindicatorGui*>(data);

	// Must be a way to toggle without triggering another event?
	if (app->ignore_sleep_timer_toggle)
	{
		app->ignore_sleep_timer_toggle = false;
		return;
	}

	if (app->sleep_timer_id)
	{
		g_source_remove(app->sleep_timer_id);

		app->sleep_timer_id = 0;

		app->event_bus->publish_only(IEventBus::event::message, MESSAGE_KEY, "Sleep timer stopped");
	}
	else
	{
		if (app->sleep_timer_dialog())
		{
			app->event_bus->publish_only(IEventBus::event::message, MESSAGE_KEY,
				std::to_string(app->config->get_uint32(SLEEP_TIMER_KEY, DEFAULT_SLEEP_TIMER_VALUE)) + " minute sleep timer started");

			app->sleep_timer_id = g_timeout_add(
				app->config->get_uint32(SLEEP_TIMER_KEY, DEFAULT_SLEEP_TIMER_VALUE) * 60000, on_timer_event, data);
		}
		else
		{
			app->ignore_sleep_timer_toggle = true;
			gtk_check_menu_item_set_active(app->sleep_timer_menu_item, FALSE);
		}
	}
}


void AppindicatorGui::on_bookmark_editor_menu_item(GtkWidget* /*widget*/, gpointer data)
{
	auto app = static_cast<AppindicatorGui*>(data);

	std::string cmd(radiotray_ng::word_expand(app->config->get_string(BOOKMARK_EDITOR_KEY, DEFAULT_BOOKMARK_EDITOR)));

	cmd += " \"" + radiotray_ng::word_expand(app->config->get_string(BOOKMARKS_KEY, RTNG_DEFAULT_BOOKMARK_FILE)) + "\"";

	LOG(debug) << "launching: " << cmd;

	g_autoptr(GError) error(nullptr);
	if (!g_spawn_command_line_async(cmd.c_str(), &error))
	{
		LOG(error) << error->message;
	}
}


void AppindicatorGui::on_reload_bookmarks_menu_item(GtkWidget* /*widget*/, gpointer data)
{
	auto app = static_cast<AppindicatorGui*>(data);

	if (app->radiotray_ng->reload_bookmarks())
	{
		// reset file monitor...
		if (app->config->get_bool(FILE_MONITOR_KEY, DEFAULT_FILE_MONITOR_VALUE))
		{
			app->bookmarks_monitor = std::make_unique<radiotray_ng::FileMonitor>(app->config->get_string(BOOKMARKS_KEY, RTNG_DEFAULT_BOOKMARK_FILE));
		}

		gtk_widget_destroy(app->menu);

		app->build_menu();

		const std::string state{app->radiotray_ng->get_state()};

		app->update_action_menu_item(state);
		app->update_status_menu_item(state);
	}
}


void AppindicatorGui::on_about_menu_item(GtkWidget* /*widget*/, gpointer /*data*/)
{
	const gchar* authors[sizeof(APP_AUTHOR)] =
	{
		APP_AUTHOR,
		nullptr,
	};

	std::string version{"v" RTNG_VERSION};

	// if git version differs, then append hash...
	if (version != RTNG_GIT_VERSION)
	{
		version += "\n(" RTNG_GIT_VERSION ")";
	}

	auto dialog = g_object_new(GTK_TYPE_ABOUT_DIALOG
		, "program-name", APP_NAME_DISPLAY
		, "license-type", GTK_LICENSE_GPL_3_0
		, "version", version.c_str()
		, "copyright", APP_COPYRIGHT
		, "website", APP_WEBSITE
		, "authors", authors
		, nullptr);

	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);

	g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), nullptr);

	gtk_window_present(GTK_WINDOW(dialog));
}


void AppindicatorGui::stop()
{
	gtk_main_quit();
}


void AppindicatorGui::reload_bookmarks()
{
	AppindicatorGui::on_reload_bookmarks_menu_item(nullptr, gpointer(this));
}


void AppindicatorGui::run(int argc, char* argv[])
{
	// Panel icon: direct modern D-Bus StatusNotifierItem.
	// Popup menu: native GTK through XWayland, positioned using GNOME's X/Y.
	gdk_set_allowed_backends("x11");
	gtk_init(&argc, &argv);

	// get default icon (todo: figure this out at runtime)
	this->resource_path = RTNG_DEFAULT_INSTALL_DIR;
	gtk_window_set_default_icon_from_file((this->resource_path + RADIOTRAY_NG_LOGO_ICON).c_str(), nullptr);

	// Keep the real menu inside RadioTray-NG so GtkImageMenuItem preserves the
	// existing 36x36 station artwork on the LEFT.
	this->build_menu();

	this->direct_sni = std::make_unique<DirectSni>(
		[this](gint x, gint y)
		{
			this->show_native_menu(x, y);
		},
		[this](gint delta, const std::string& orientation)
		{
			// GNOME forwards smooth-scroll deltas through SNI Scroll().
			// Match RadioTray's original AppIndicator behaviour:
			//   vertical:   UP (negative dy)   -> volume up
			//               DOWN (positive dy) -> volume down
			//   horizontal: RIGHT (positive dx)-> volume up
			//               LEFT (negative dx) -> volume down
			if (orientation == "horizontal")
			{
				if (delta > 0)
				{
					this->radiotray_ng->volume_up();
				}
				else if (delta < 0)
				{
					this->radiotray_ng->volume_down();
				}
			}
			else
			{
				if (delta < 0)
				{
					this->radiotray_ng->volume_up();
				}
				else if (delta > 0)
				{
					this->radiotray_ng->volume_down();
				}
			}
		});

	const std::string icon_off{radiotray_ng::word_expand(this->config->get_string(
		RADIOTRAY_NG_ICON_OFF_KEY, DEFAULT_RADIOTRAY_NG_ICON_OFF_VALUE))};

	this->direct_sni->set_icon(icon_off);

	if (!this->direct_sni->start())
	{
		LOG(error) << "Unable to register direct StatusNotifierItem";
	}

	if (argc > 1)
	{
		if (std::string(argv[1]) == "--play")
		{
			radiotray_ng->play();
		}
	}

	gtk_main();

	this->direct_sni.reset();
	gtk_widget_destroy(this->menu);
}
