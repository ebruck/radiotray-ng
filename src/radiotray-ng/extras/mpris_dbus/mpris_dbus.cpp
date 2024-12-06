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
		"<interface name='org.mpris.MediaPlayer2'>"
		"	<method name='Raise' />"
		"	<method name='Quit'>"
		"		<annotation name='org.freedesktop.DBus.Method.NoReply' value='true' />"
		"	</method>"
		"	<property name='CanQuit' type='b' access='read'>"
		"		<annotation name='org.freedesktop.DBus.Property.EmitsChangedSignal' value='const' />"
		"	</property>"
		"	<property name='Fullscreen' type='b' access='readwrite' />"
		"	<property name='CanSetFullscreen' type='b' access='read'>"
		"		<annotation name='org.freedesktop.DBus.Property.EmitsChangedSignal' value='const' />"
		"	</property>"
		"	<property name='CanRaise' type='b' access='read'>"
		"		<annotation name='org.freedesktop.DBus.Property.EmitsChangedSignal' value='const' />"
		"	</property>"
		"	<property name='HasTrackList' type='b' access='read'>"
		"		<annotation name='org.freedesktop.DBus.Property.EmitsChangedSignal' value='const' />"
		"	</property>"
		"	<property name='Identity' type='s' access='read'>"
		"		<annotation name='org.freedesktop.DBus.Property.EmitsChangedSignal' value='const' />"
		"	</property>"
		"	<property name='DesktopEntry' type='s' access='read'>"
		"		<annotation name='org.freedesktop.DBus.Property.EmitsChangedSignal' value='const' />"
		"	</property>"
		"	<property name='SupportedUriSchemes' type='as' access='read'>"
		"		<annotation name='org.freedesktop.DBus.Property.EmitsChangedSignal' value='const' />"
		"	</property>"
		"	<property name='SupportedMimeTypes' type='as' access='read'>"
		"		<annotation name='org.freedesktop.DBus.Property.EmitsChangedSignal' value='const' />"
		"	</property>"
		"</interface>"
		"</node>";
	const char PLAYER_INTROSPECTION_XML[] =
		"<node>"
		"<interface name='org.mpris.MediaPlayer2.Player'>"
		"	<method name='Next' />"
		"	<method name='Previous' />"
		"	<method name='Pause' />"
		"	<method name='PlayPause' />"
		"	<method name='Stop' />"
		"	<method name='Play' />"
		"	<method name='Seek'>"
		"		<arg name='Offset' type='x' direction='in' />"
		"	</method>"
		"	<method name='SetPosition'>"
		"		<arg name='TrackId' type='o' direction='in' />"
		"		<arg name='Position' type='x' direction='in' />"
		"	</method>"
		"	<method name='OpenUri'>"
		"		<arg name='Uri' type='s' direction='in' />"
		"	</method>"
		"	<signal name='Seeked'>"
		"		<arg name='Position' type='x' />"
		"	</signal>"
		"	<property name='PlaybackStatus' type='s' access='read' />"
		"	<property name='LoopStatus' type='s' access='readwrite' />"
		"	<property name='Rate' type='d' access='readwrite'>"
		"		<annotation name='org.freedesktop.DBus.Property.EmitsChangedSignal' value='const' />"
		"	</property>"
		"	<property name='Shuffle' type='b' access='readwrite' />"
		"	<property name='Metadata' type='a{sv}' access='read' />"
		"	<property name='Volume' type='d' access='readwrite' />"
		"	<property name='Position' type='x' access='read'>"
		"		<annotation name='org.freedesktop.DBus.Property.EmitsChangedSignal' value='false' />"
		"	</property>"
		"	<property name='MinimumRate' type='d' access='read'>"
		"		<annotation name='org.freedesktop.DBus.Property.EmitsChangedSignal' value='const' />"
		"	</property>"
		"	<property name='MaximumRate' type='d' access='read'>"
		"		<annotation name='org.freedesktop.DBus.Property.EmitsChangedSignal' value='const' />"
		"	</property>"
		"	<property name='CanGoNext' type='b' access='read' />"
		"	<property name='CanGoPrevious' type='b' access='read' />"
		"	<property name='CanPlay' type='b' access='read' />"
		"	<property name='CanPause' type='b' access='read' />"
		"	<property name='CanSeek' type='b' access='read' />"
		"	<property name='CanControl' type='b' access='read'>"
		"		<annotation name='org.freedesktop.DBus.Property.EmitsChangedSignal' value='const' />"
		"	</property>"
		"</interface>"
		"</node>";
	/* const char TRACKLIST_INTROSPECTION_XML[] =
		"<node>"
		"  <interface name='org.mpris.MediaPlayer2.TrackList'>"
		"    <method name='GetTracksMetadata'>"
		"      <arg direction='in' name='TrackIds' type='ao'/>"
		"      <arg direction='out' name='Metadata' type='aa{sv}'/>"
		"    </method>"
		"    <method name='AddTrack'>"
		"      <arg direction='in' name='Uri' type='s'/>"
		"      <arg direction='in' name='AfterTrack' type='o'/>"
		"      <arg direction='in' name='SetAsCurrent' type='b'/>"
		"    </method>"
		"    <method name='RemoveTrack'>"
		"      <arg direction='in' name='TrackId' type='o'/>"
		"    </method>"
		"    <method name='GoTo'>"
		"      <arg direction='in' name='TrackId' type='o'/>"
		"    </method>"
		"    <signal name='TrackListReplaced'>"
		"      <arg name='Tracks' type='ao'/>"
		"      <arg name='CurrentTrack' type='o'/>"
		"    </signal>"
		"    <signal name='TrackAdded'>"
		"      <arg name='Metadata' type='a{sv}'/>"
		"      <arg name='AfterTrack' type='o'/>"
		"    </signal>"
		"    <signal name='TrackRemoved'>"
		"      <arg name='TrackId' type='o'/>"
		"    </signal>"
		"    <signal name='TrackMetadataChanged'>"
		"      <arg name='TrackId' type='o'/>"
		"      <arg name='Metadata' type='a{sv}'/>"
		"    </signal>"
		"    <property name='Tracks' type='ao' access='read'/>"
		"    <property name='CanEditTracks' type='b' access='read'/>"
		"  </interface>"
		"</node>"; */
	const char MPRIS_DBUS_NAME[] = "org.mpris.MediaPlayer2.radiotray-ng";
	const char MPRIS_DBUS_OBJECT_PATH[] = "/org/mpris/MediaPlayer2";
}


MprisDbus::MprisDbus(std::shared_ptr<IGui> gui, std::shared_ptr<IRadioTrayNG> radiotray_ng)
	: interface_vtable(
		sigc::mem_fun(*this, &MprisDbus::on_method_call),
		sigc::mem_fun(*this, &MprisDbus::on_interface_get_property),
		sigc::mem_fun(*this, &MprisDbus::on_interface_set_property))
	, radiotray_ng(std::move(radiotray_ng))
	, gui(std::move(gui))
{
	this->dbus_setup();
}


MprisDbus::~MprisDbus()
{
	Gio::DBus::unown_name(this->own_name_id);
}


void MprisDbus::on_method_call(
	const Glib::RefPtr<Gio::DBus::Connection>& /*connection*/, 
	const Glib::ustring& /*sender*/, 
	const Glib::ustring& /*object_path*/,
	const Glib::ustring& /*interface_name*/, 
	const Glib::ustring& method_name, 
	const Glib::VariantContainerBase& parameters, 
	const Glib::RefPtr<Gio::DBus::MethodInvocation>& invocation)
{
	LOG(debug) << "method called: " << method_name;
	
	if (method_name == "PlayPause")
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

	if (method_name == "Play")
	{
		if (this->radiotray_ng->get_state() != STATE_PLAYING)
		{
			this->radiotray_ng->play();
		}
		invocation->return_value(Glib::VariantContainerBase());
		return;
	}
    if (method_name == "Mute")
    {
        this->radiotray_ng->mute();
        invocation->return_value(Glib::VariantContainerBase());
        return;
    }

	if (method_name == "Stop" || method_name == "Pause" )
	{
		if (this->radiotray_ng->get_state() != STATE_STOPPED)
		{
			this->radiotray_ng->stop();
		}
		invocation->return_value(Glib::VariantContainerBase());
		return;
	}

	if (method_name == "Quit")
	{
		this->gui->stop();

		invocation->return_value(Glib::VariantContainerBase());
		return;
	}

	if (method_name == "reload_bookmarks")
	{
		this->gui->reload_bookmarks();
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
	//TODO Implement OpenURI Properly
	if (method_name == "play_url")
	{
		Glib::Variant<Glib::ustring> param;

		parameters.get_child(param, 0);
		const Glib::ustring url = param.get();

		this->radiotray_ng->play_url(url);
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

void MprisDbus::on_interface_get_property(
	Glib::VariantBase& property,
  	const Glib::RefPtr<Gio::DBus::Connection>& /* connection */,
  	const Glib::ustring& /* sender */, 
	const Glib::ustring& /* object_path */,
  	const Glib::ustring& /* interface_name */, 
	const Glib::ustring& property_name){
		LOG(debug) << "get property: " << property_name;
		if (property_name == "CanQuit") {
			property = Glib::Variant<bool>::create(true);
		} else if (property_name == "CanRaise") {
			property = Glib::Variant<bool>::create(false);
		} else if (property_name == "CanSetFullscreen") {
			property = Glib::Variant<bool>::create(false);
		} else if (property_name == "DesktopEntry") {
			property = Glib::Variant<Glib::ustring>::create("/usr/share/applications/vlc"); 
		} else if (property_name == "Fullscreen") {
			property = Glib::Variant<bool>::create(false);
		} else if (property_name == "HasTrackList") {
			property = Glib::Variant<bool>::create(false);
		} else if (property_name == "Identity") {
			property = Glib::Variant<Glib::ustring>::create("radiotray-ng");
		} else if (property_name == "SupportedMimeTypes") {
			property = Glib::Variant<std::vector<Glib::ustring>>::create({});
		} else if (property_name == "SupportedUriSchemes") {
			property = Glib::Variant<std::vector<Glib::ustring>>::create({});
		} else if (property_name == "CanControl") {
			property = Glib::Variant<bool>::create(true);
		} else if (property_name == "CanGoNext") {
			property = Glib::Variant<bool>::create(true);
		} else if (property_name == "CanGoPrevious") {
			property = Glib::Variant<bool>::create(true);
		} else if (property_name == "CanPause") {
			property = Glib::Variant<bool>::create(true);
		} else if (property_name == "CanPlay") {
			property = Glib::Variant<bool>::create(true);
		} else if (property_name == "CanSeek") {
			property = Glib::Variant<bool>::create(false);
		} else if (property_name == "MaximumRate") {
			property = Glib::Variant<double>::create(1.0);
		} else if (property_name == "Metadata") {
			std::map<Glib::ustring, Glib::VariantBase> metadata;

			metadata["MprisDbus:trackid"] = Glib::Variant<Glib::DBusObjectPathString>::create("/Track1");
			metadata["xesam:title"] = Glib::Variant<Glib::ustring>::create(radiotray_ng->get_title());
			metadata["xesam:album"] = Glib::Variant<Glib::ustring>::create(radiotray_ng->get_station());
			metadata["xesam:artist"] = Glib::Variant<std::vector<Glib::ustring>>::create({radiotray_ng->get_artist()});
			
			property = Glib::Variant<std::map<Glib::ustring, Glib::VariantBase>>::create(metadata);

		} else if (property_name == "MinimumRate") {
			property = Glib::Variant<double>::create(1.0);
		} else if (property_name == "PlaybackStatus") {
			if(radiotray_ng->get_state()==STATE_PLAYING){
				property = Glib::Variant<Glib::ustring>::create("Playing");
			}
			else{
				property = Glib::Variant<Glib::ustring>::create("Paused");
			}
		} else if (property_name == "Position") {
			property = Glib::Variant<long>::create(0);
		} else if (property_name == "Rate") {
			property = Glib::Variant<double>::create(1.0);
		} else if (property_name == "Volume") {
			property = Glib::Variant<double>::create(stoi(radiotray_ng->get_volume())/100);
		} else if (property_name == "Shuffle") {
			property = Glib::Variant<bool>::create(true); //Set to true since not processing through a playlist
		} else if (property_name == "LoopStatus"){
			property = Glib::Variant<Glib::ustring>::create("None");//Set to none, since radiostations don't loop
		}
		};
  
bool MprisDbus::on_interface_set_property(
  const Glib::RefPtr<Gio::DBus::Connection>& /* connection */,
  const Glib::ustring& /* sender */, 
  const Glib::ustring& /* object_path */,
  const Glib::ustring& /* interface_name */,
  const Glib::ustring& property_name, 
  const Glib::VariantBase& value ){
		LOG(debug) << "set property: " << property_name;
		try {
		if (property_name == "FullScreen") {
			return false;
		} else if (property_name == "Rate") {
		} else if (property_name == "Volume") {
			double derived = (Glib::VariantBase::cast_dynamic<Glib::Variant<double>>(value)).get();
			radiotray_ng->set_volume(std::to_string(int(derived*100)));
		} else {
			return false;
		}

		return true;
	} catch (...) {
	}

	return false;
	}


void MprisDbus::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connection, const Glib::ustring& /*name*/)
{
	try
	{
		this->registered_id = connection->register_object(MPRIS_DBUS_OBJECT_PATH, this->introspection_data->lookup_interface(),
			this->interface_vtable);
		this->registered_id = connection->register_object(MPRIS_DBUS_OBJECT_PATH, this->player_introspection_data->lookup_interface(),
			this->interface_vtable);
		/* this->registered_id = connection->register_object(MPRIS_DBUS_OBJECT_PATH, this->tracklist_introspection_data->lookup_interface(),
			this->interface_vtable); */
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
		this->player_introspection_data = Gio::DBus::NodeInfo::create_for_xml(PLAYER_INTROSPECTION_XML);
		// this->tracklist_introspection_data = Gio::DBus::NodeInfo::create_for_xml(TRACKLIST_INTROSPECTION_XML);
	 
	 
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
			if (connection)
			{
				connection->unregister_object(this->registered_id);
			}
		});

}
