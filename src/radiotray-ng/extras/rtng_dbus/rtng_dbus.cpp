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
#include <radiotray-ng/i_radiotray_ng.hpp>
#include <radiotray-ng/i_gui.hpp>
#include "rtng_dbus.hpp"


namespace
{
	const char INTROSPECTION_XML[] =
		"<node>"
		"  <interface name='com.github.radiotray_ng'>"
		"    <method name='volume_up'>"
		"    </method>"
		"    <method name='volume_down'>"
		"    </method>"
		"    <method name='play'>"
		"    </method>"
		"    <method name='stop'>"
		"    </method>"
		"    <method name='quit'>"
		"    </method>"
		"    <method name='reload_bookmarks'>"
		"    </method>"
		"    <method name='previous_station'>"
		"    </method>"
		"    <method name='next_station'>"
		"    </method>"
		"    <method name='get_bookmarks'>"
		"      <arg type='s' direction='out'/>"
		"    </method>"
		"    <method name='get_config'>"
		"      <arg type='s' direction='out'/>"
		"    </method>"
		"    <method name='get_player_state'>"
		"      <arg type='s' direction='out'/>"
		"    </method>"
		"    <method name='play_station'>"
		"      <arg type='s' direction='in'/>"
		"      <arg type='s' direction='in'/>"
		"    </method>"
		"    <method name='set_volume'>"
		"      <arg type='s' direction='in'/>"
  		"    </method>"
		"  </interface>"
		"</node>";

	const char RTNG_DBUS_NAME[] = "com.github.radiotray_ng";
	const char RTNG_DBUS_OBJECT_PATH[] = "/com/github/radiotray_ng";
}


RtngDbus::RtngDbus(std::shared_ptr<IGui> gui, std::shared_ptr<IRadioTrayNG> radiotray_ng)
	: interface_vtable(sigc::mem_fun(*this, &RtngDbus::on_method_call))
	, radiotray_ng(std::move(radiotray_ng))
	, gui(std::move(gui))
{
	this->dbus_setup();
}


RtngDbus::~RtngDbus()
{
	Gio::DBus::unown_name(this->own_name_id);
}


void RtngDbus::on_method_call(const Glib::RefPtr<Gio::DBus::Connection>& /*connection*/, const Glib::ustring& /*sender*/, const Glib::ustring& /*object_path*/,
	const Glib::ustring& /*interface_name*/, const Glib::ustring& method_name, const Glib::VariantContainerBase& parameters, const Glib::RefPtr<Gio::DBus::MethodInvocation>& invocation)
{
	LOG(debug) << "method called: " << method_name;

	if (method_name == "play")
	{
		if (this->radiotray_ng->get_state() == STATE_STOPPED)
		{
			this->radiotray_ng->play();
		}
		else
		{
			this->radiotray_ng->stop();
		}

		invocation->return_value(Glib::VariantContainerBase());
		return;
	}

	if (method_name == "stop")
	{
		if (this->radiotray_ng->get_state() != STATE_STOPPED)
		{
			this->radiotray_ng->stop();
		}
		invocation->return_value(Glib::VariantContainerBase());
		return;
	}

	if (method_name == "quit")
	{
		this->gui->stop();

		invocation->return_value(Glib::VariantContainerBase());
		return;
	}

	if (method_name == "reload_bookmarks")
	{
		this->radiotray_ng->reload_bookmarks();
		invocation->return_value(Glib::VariantContainerBase());
		return;
	}

	if (method_name == "volume_up")
	{
		this->radiotray_ng->volume_up_msg();
		invocation->return_value(Glib::VariantContainerBase());
		return;
	}

	if (method_name == "volume_down")
	{
		this->radiotray_ng->volume_down_msg();
		invocation->return_value(Glib::VariantContainerBase());
		return;
	}

	if (method_name == "previous_station")
	{
		this->radiotray_ng->previous_station_msg();
		invocation->return_value(Glib::VariantContainerBase());
		return;
	}

	if (method_name == "next_station")
	{
		this->radiotray_ng->next_station_msg();
		invocation->return_value(Glib::VariantContainerBase());
		return;
	}

	if (method_name == "get_player_state")
	{
		auto var = Glib::Variant<Glib::ustring>::create(this->radiotray_ng->get_player_state());
		invocation->return_value(Glib::VariantContainerBase::create_tuple(var));
		return;
	}

	if (method_name == "play_station")
	{
		Glib::Variant<Glib::ustring> param;

		parameters.get_child(param, 0);
		const Glib::ustring group = param.get();

		parameters.get_child(param, 1);
		const Glib::ustring station = param.get();

		this->radiotray_ng->play(group, station);
		invocation->return_value(Glib::VariantContainerBase());
		return;
	}

	if (method_name == "get_bookmarks")
	{
		auto var = Glib::Variant<Glib::ustring>::create(this->radiotray_ng->get_bookmarks());
		invocation->return_value(Glib::VariantContainerBase::create_tuple(var));
		return;
	}

	if (method_name == "get_config")
	{
		auto var = Glib::Variant<Glib::ustring>::create(this->radiotray_ng->get_config());
		invocation->return_value(Glib::VariantContainerBase::create_tuple(var));
		return;
	}

	if (method_name == "set_volume")
	{
		Glib::Variant<Glib::ustring> param;

		parameters.get_child(param, 0);
		const Glib::ustring volume = param.get();

		try
		{
			this->radiotray_ng->set_volume_msg(std::stoul(volume));
			invocation->return_value(Glib::VariantContainerBase());
		}
		catch(std::invalid_argument& ex)
		{
			invocation->return_error(Gio::DBus::Error(Gio::DBus::Error::INVALID_ARGS, "volume not an integer"));
		}
		return;
	}

	// non-existent method on the interface...
	invocation->return_error(Gio::DBus::Error(Gio::DBus::Error::UNKNOWN_METHOD, "method does not exist"));
}


void RtngDbus::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connection, const Glib::ustring& /*name*/)
{
	try
	{
		this->registered_id = connection->register_object(RTNG_DBUS_OBJECT_PATH, this->introspection_data->lookup_interface(),
			this->interface_vtable);
	}
	catch (const Glib::Error& ex)
	{
		LOG(error) << "register_object of object failed: " << RTNG_DBUS_OBJECT_PATH;
	}
}


void RtngDbus::dbus_setup()
{
	std::locale::global(std::locale(""));

	Gio::init();

	try
	{
		this->introspection_data = Gio::DBus::NodeInfo::create_for_xml(INTROSPECTION_XML);
	}
	catch (const Glib::Error& ex)
	{
		LOG(error) << "could not create introspection data: " << ex.what();

		return;
	}

	this->own_name_id = Gio::DBus::own_name(Gio::DBus::BusType::BUS_TYPE_SESSION, RTNG_DBUS_NAME,
		sigc::mem_fun(*this, &RtngDbus::on_bus_acquired),
		Gio::DBus::SlotNameAcquired(),
		[this](const Glib::RefPtr<Gio::DBus::Connection>& connection, const Glib::ustring&)
		{
			connection->unregister_object(this->registered_id);
		});

}
