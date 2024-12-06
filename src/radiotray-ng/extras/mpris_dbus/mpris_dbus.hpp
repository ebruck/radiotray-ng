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

#include <radiotray-ng/i_event_bus.hpp>

#include <giomm.h>
#include <glibmm.h>

#include <memory>

class IRadioTrayNG;
class IGui;


class MprisDbus
{
public:
	MprisDbus(std::shared_ptr<IGui> gui, std::shared_ptr<IRadioTrayNG> radiotray_ng, std::shared_ptr<IEventBus> event_bus);

	~MprisDbus();

private:
	void on_tags_event(const IEventBus::event& ev, IEventBus::event_data_t& data);
	void on_state_event(const IEventBus::event& ev, IEventBus::event_data_t& data);

	void dbus_setup();

	void on_method_call(
		const Glib::RefPtr<Gio::DBus::Connection>& connection,
		const Glib::ustring& sender,
		const Glib::ustring& object_path,
		const Glib::ustring& interface_name,
		const Glib::ustring& method_name,
		const Glib::VariantContainerBase& parameters,
		const Glib::RefPtr<Gio::DBus::MethodInvocation>& invocation);

	void on_interface_get_property(
		Glib::VariantBase& property,
		const Glib::RefPtr<Gio::DBus::Connection>& connection,
		const Glib::ustring& sender, 
		const Glib::ustring& object_path,
		const Glib::ustring& interface_name, 
		const Glib::ustring& property_name);
  
	bool on_interface_set_property(
		const Glib::RefPtr<Gio::DBus::Connection>& connection,
		const Glib::ustring& sender, 
		const Glib::ustring& object_path,
		const Glib::ustring& interface_name,
		const Glib::ustring& property_name, 
		const Glib::VariantBase& value);
	
	Glib::Variant<Glib::ustring> create_playbackstatus();
	Glib::Variant<std::map<Glib::ustring, Glib::VariantBase>> create_metadata();
	
	void PlayerPropertyChanged(
		const Glib::ustring &name,
		const Glib::VariantBase &value);

	void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connection, const Glib::ustring& name);

	guint registered_id;
	guint own_name_id;
	Glib::RefPtr<Gio::DBus::NodeInfo> introspection_data;
	Glib::RefPtr<Gio::DBus::NodeInfo> player_introspection_data;
	// Glib::RefPtr<Gio::DBus::NodeInfo> tracklist_introspection_data;
	const Gio::DBus::InterfaceVTable  interface_vtable;

	std::shared_ptr<IRadioTrayNG> radiotray_ng;
	std::shared_ptr<IGui> gui;
	std::shared_ptr<IEventBus> event_bus;
};
