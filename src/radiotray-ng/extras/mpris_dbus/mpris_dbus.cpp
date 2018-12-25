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
#include "mpris_dbus.hpp"


namespace
{
	const char INTROSPECTION_XML[] =
		"<node>"
		"  <interface name='org.mpris.MediaPlayer2.Player'>"
		"    <method name='Play'>"
		"    </method>"
		"    <method name='PlayPause'>"
		"    </method>"
		"    <method name='Stop'>"
		"    </method>"
		"    <method name='Previous'>"
		"    </method>"
		"    <method name='Next'>"
		"    </method>"
		"    <method name='OpenUri'>"
		"      <arg type='s' direction='in'/>"
		"    </method>"
		"  </interface>"
		"</node>";

	const char MPRIS_DBUS_NAME[] = "org.mpris.MediaPlayer2.radiotray_ng";
	const char MPRIS_DBUS_OBJECT_PATH[] = "/org/mpris/MediaPlayer2";
}


MprisDbus::MprisDbus(std::shared_ptr<IGui> gui, std::shared_ptr<IRadioTrayNG> radiotray_ng)
	: interface_vtable(sigc::mem_fun(*this, &MprisDbus::on_method_call))
	, radiotray_ng(std::move(radiotray_ng))
	, gui(std::move(gui))
{
	this->dbus_setup();
}


MprisDbus::~MprisDbus()
{
	Gio::DBus::unown_name(this->own_name_id);
}


void MprisDbus::on_method_call(const Glib::RefPtr<Gio::DBus::Connection>& /*connection*/, const Glib::ustring& /*sender*/, const Glib::ustring& /*object_path*/,
	const Glib::ustring& /*interface_name*/, const Glib::ustring& method_name, const Glib::VariantContainerBase& parameters, const Glib::RefPtr<Gio::DBus::MethodInvocation>& invocation)
{
	LOG(debug) << "method called: " << method_name;

	if (method_name == "Play" || method_name == "PlayPause")
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

	if (method_name == "Stop")
	{
		if (this->radiotray_ng->get_state() != STATE_STOPPED)
		{
			this->radiotray_ng->stop();
		}
		invocation->return_value(Glib::VariantContainerBase());
		return;
	}

	if (method_name == "Previous")
	{
		this->radiotray_ng->previous_station_msg();
		invocation->return_value(Glib::VariantContainerBase());
		return;
	}

	if (method_name == "Next")
	{
		this->radiotray_ng->next_station_msg();
		invocation->return_value(Glib::VariantContainerBase());
		return;
	}

	if (method_name == "OpenUri")
	{
		Glib::Variant<Glib::ustring> param;

		parameters.get_child(param, 0);
		const Glib::ustring url = param.get();

		this->radiotray_ng->play_url(url);
		invocation->return_value(Glib::VariantContainerBase());
		return;
	}

	// non-existent method on the interface...
	invocation->return_error(Gio::DBus::Error(Gio::DBus::Error::UNKNOWN_METHOD, "method does not exist"));
}


void MprisDbus::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connection, const Glib::ustring& /*name*/)
{
	try
	{
		this->registered_id = connection->register_object(MPRIS_DBUS_OBJECT_PATH, this->introspection_data->lookup_interface(),
			this->interface_vtable);
	}
	catch (const Glib::Error& ex)
	{
		LOG(error) << "register_object of object failed: " << MPRIS_DBUS_OBJECT_PATH;
	}
}


void MprisDbus::dbus_setup()
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

	this->own_name_id = Gio::DBus::own_name(Gio::DBus::BusType::BUS_TYPE_SESSION, MPRIS_DBUS_NAME,
		sigc::mem_fun(*this, &MprisDbus::on_bus_acquired),
		Gio::DBus::SlotNameAcquired(),
		[this](const Glib::RefPtr<Gio::DBus::Connection>& connection, const Glib::ustring&)
		{
			connection->unregister_object(this->registered_id);
		});

}
