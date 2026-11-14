#include "direct_sni.hpp"

#include <iostream>
#include <utility>

namespace
{
const char* SNI_XML = R"XML(
<node>
  <interface name="org.kde.StatusNotifierItem">
    <property name="Category" type="s" access="read"/>
    <property name="Id" type="s" access="read"/>
    <property name="Title" type="s" access="read"/>
    <property name="Status" type="s" access="read"/>
    <property name="WindowId" type="i" access="read"/>
    <property name="IconThemePath" type="s" access="read"/>
    <property name="Menu" type="o" access="read"/>
    <property name="ItemIsMenu" type="b" access="read"/>
    <property name="IconName" type="s" access="read"/>
    <property name="IconPixmap" type="a(iiay)" access="read"/>
    <property name="OverlayIconName" type="s" access="read"/>
    <property name="OverlayIconPixmap" type="a(iiay)" access="read"/>
    <property name="AttentionIconName" type="s" access="read"/>
    <property name="AttentionIconPixmap" type="a(iiay)" access="read"/>
    <property name="AttentionMovieName" type="s" access="read"/>

    <method name="ContextMenu">
      <arg name="x" type="i" direction="in"/>
      <arg name="y" type="i" direction="in"/>
    </method>

    <method name="Activate">
      <arg name="x" type="i" direction="in"/>
      <arg name="y" type="i" direction="in"/>
    </method>

    <method name="SecondaryActivate">
      <arg name="x" type="i" direction="in"/>
      <arg name="y" type="i" direction="in"/>
    </method>

    <method name="Scroll">
      <arg name="delta" type="i" direction="in"/>
      <arg name="orientation" type="s" direction="in"/>
    </method>
  </interface>
</node>
)XML";

const char* MENU_XML = R"XML(
<node>
  <interface name="com.canonical.dbusmenu">
    <property name="Version" type="u" access="read"/>
    <property name="TextDirection" type="s" access="read"/>
    <property name="Status" type="s" access="read"/>
    <property name="IconThemePath" type="as" access="read"/>

    <method name="GetLayout">
      <arg type="i" name="parentId" direction="in"/>
      <arg type="i" name="recursionDepth" direction="in"/>
      <arg type="as" name="propertyNames" direction="in"/>
      <arg type="u" name="revision" direction="out"/>
      <arg type="(ia{sv}av)" name="layout" direction="out"/>
    </method>

    <method name="GetGroupProperties">
      <arg type="ai" name="ids" direction="in"/>
      <arg type="as" name="propertyNames" direction="in"/>
      <arg type="a(ia{sv})" name="properties" direction="out"/>
    </method>

    <method name="GetProperty">
      <arg type="i" name="id" direction="in"/>
      <arg type="s" name="name" direction="in"/>
      <arg type="v" name="value" direction="out"/>
    </method>

    <method name="Event">
      <arg type="i" name="id" direction="in"/>
      <arg type="s" name="eventId" direction="in"/>
      <arg type="v" name="data" direction="in"/>
      <arg type="u" name="timestamp" direction="in"/>
    </method>

    <method name="EventGroup">
      <arg type="a(isvu)" name="events" direction="in"/>
      <arg type="ai" name="idErrors" direction="out"/>
    </method>

    <method name="AboutToShow">
      <arg type="i" name="id" direction="in"/>
      <arg type="b" name="needUpdate" direction="out"/>
    </method>

    <method name="AboutToShowGroup">
      <arg type="ai" name="ids" direction="in"/>
      <arg type="ai" name="updatesNeeded" direction="out"/>
      <arg type="ai" name="idErrors" direction="out"/>
    </method>
  </interface>
</node>
)XML";

GVariant* empty_pixmaps()
{
    return g_variant_new_array(G_VARIANT_TYPE("(iiay)"), nullptr, 0);
}

GVariant* empty_int_array()
{
    return g_variant_new_array(G_VARIANT_TYPE_INT32, nullptr, 0);
}
}

DirectSni::DirectSni(ActivateCallback activate_cb, ScrollCallback scroll_cb)
    : activate_cb(std::move(activate_cb))
    , scroll_cb(std::move(scroll_cb))
{
}

DirectSni::~DirectSni()
{
    if (connection != nullptr)
    {
        if (sni_registration != 0)
            g_dbus_connection_unregister_object(connection, sni_registration);

        if (menu_registration != 0)
            g_dbus_connection_unregister_object(connection, menu_registration);
    }

    if (sni_node != nullptr)
        g_dbus_node_info_unref(sni_node);

    if (menu_node != nullptr)
        g_dbus_node_info_unref(menu_node);

    if (connection != nullptr)
        g_object_unref(connection);
}

bool DirectSni::start()
{
    GError* error = nullptr;

    connection = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);

    if (connection == nullptr)
    {
        std::cerr << "DirectSni: session bus unavailable: "
                  << (error != nullptr ? error->message : "unknown error")
                  << std::endl;
        g_clear_error(&error);
        return false;
    }

    sni_node = g_dbus_node_info_new_for_xml(SNI_XML, &error);

    if (sni_node == nullptr)
    {
        std::cerr << "DirectSni: invalid SNI XML: "
                  << (error != nullptr ? error->message : "unknown error")
                  << std::endl;
        g_clear_error(&error);
        return false;
    }

    menu_node = g_dbus_node_info_new_for_xml(MENU_XML, &error);

    if (menu_node == nullptr)
    {
        std::cerr << "DirectSni: invalid DBusMenu XML: "
                  << (error != nullptr ? error->message : "unknown error")
                  << std::endl;
        g_clear_error(&error);
        return false;
    }

    static const GDBusInterfaceVTable sni_vtable = {
        &DirectSni::on_sni_method_call,
        &DirectSni::on_sni_get_property,
        nullptr,
        {0}
    };

    static const GDBusInterfaceVTable menu_vtable = {
        &DirectSni::on_menu_method_call,
        &DirectSni::on_menu_get_property,
        nullptr,
        {0}
    };

    sni_registration = g_dbus_connection_register_object(
        connection,
        "/StatusNotifierItem",
        sni_node->interfaces[0],
        &sni_vtable,
        this,
        nullptr,
        &error);

    if (sni_registration == 0)
    {
        std::cerr << "DirectSni: failed to export StatusNotifierItem: "
                  << (error != nullptr ? error->message : "unknown error")
                  << std::endl;
        g_clear_error(&error);
        return false;
    }

    menu_registration = g_dbus_connection_register_object(
        connection,
        "/MenuBar",
        menu_node->interfaces[0],
        &menu_vtable,
        this,
        nullptr,
        &error);

    if (menu_registration == 0)
    {
        std::cerr << "DirectSni: failed to export single-click bridge menu: "
                  << (error != nullptr ? error->message : "unknown error")
                  << std::endl;
        g_clear_error(&error);
        return false;
    }

    GVariant* result = g_dbus_connection_call_sync(
        connection,
        "org.kde.StatusNotifierWatcher",
        "/StatusNotifierWatcher",
        "org.kde.StatusNotifierWatcher",
        "RegisterStatusNotifierItem",
        g_variant_new("(s)", "/StatusNotifierItem"),
        nullptr,
        G_DBUS_CALL_FLAGS_NONE,
        3000,
        nullptr,
        &error);

    if (result == nullptr)
    {
        std::cerr << "DirectSni: watcher registration failed: "
                  << (error != nullptr ? error->message : "unknown error")
                  << std::endl;
        g_clear_error(&error);
        return false;
    }

    g_variant_unref(result);
    return true;
}

void DirectSni::set_icon(const std::string& icon)
{
    if (icon.empty())
        return;

    if (icon.find('/') == std::string::npos)
    {
        icon_name = icon;
        icon_theme_path.clear();
    }
    else
    {
        gchar* dirname = g_path_get_dirname(icon.c_str());
        gchar* basename = g_path_get_basename(icon.c_str());

        icon_theme_path = dirname != nullptr ? dirname : "";
        icon_name = basename != nullptr ? basename : "";

        const auto dot = icon_name.rfind('.');

        if (dot != std::string::npos)
            icon_name.resize(dot);

        g_free(dirname);
        g_free(basename);
    }

    emit_icon_changed();
}

void DirectSni::emit_icon_changed()
{
    if (connection == nullptr)
        return;

    GVariantBuilder changed;
    g_variant_builder_init(&changed, G_VARIANT_TYPE("a{sv}"));

    g_variant_builder_add(
        &changed,
        "{sv}",
        "IconName",
        g_variant_new_string(icon_name.c_str()));

    g_variant_builder_add(
        &changed,
        "{sv}",
        "IconThemePath",
        g_variant_new_string(icon_theme_path.c_str()));

    GVariantBuilder invalidated;
    g_variant_builder_init(&invalidated, G_VARIANT_TYPE("as"));

    g_dbus_connection_emit_signal(
        connection,
        nullptr,
        "/StatusNotifierItem",
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged",
        g_variant_new(
            "(s@a{sv}@as)",
            "org.kde.StatusNotifierItem",
            g_variant_builder_end(&changed),
            g_variant_builder_end(&invalidated)),
        nullptr);
}

void DirectSni::on_sni_method_call(
    GDBusConnection*,
    const gchar*,
    const gchar*,
    const gchar*,
    const gchar* method_name,
    GVariant* parameters,
    GDBusMethodInvocation* invocation,
    gpointer user_data)
{
    auto self = static_cast<DirectSni*>(user_data);

    if (g_strcmp0(method_name, "ContextMenu") == 0 ||
        g_strcmp0(method_name, "Activate") == 0)
    {
        gint x = 0;
        gint y = 0;

        g_variant_get(parameters, "(ii)", &x, &y);

        if (self->activate_cb)
            self->activate_cb(x, y);

        g_dbus_method_invocation_return_value(invocation, nullptr);
        return;
    }

    if (g_strcmp0(method_name, "SecondaryActivate") == 0)
    {
        // Middle click is deliberately ignored. The GNOME bridge routes only
        // primary and secondary mouse clicks through Activate().
        g_dbus_method_invocation_return_value(invocation, nullptr);
        return;
    }

    if (g_strcmp0(method_name, "Scroll") == 0)
    {
        gint delta = 0;
        const gchar* orientation = "";

        g_variant_get(parameters, "(i&s)", &delta, &orientation);

        if (self->scroll_cb)
            self->scroll_cb(delta, orientation != nullptr ? orientation : "");

        g_dbus_method_invocation_return_value(invocation, nullptr);
        return;
    }

    g_dbus_method_invocation_return_value(invocation, nullptr);
}

GVariant* DirectSni::on_sni_get_property(
    GDBusConnection*,
    const gchar*,
    const gchar*,
    const gchar*,
    const gchar* property_name,
    GError**,
    gpointer user_data)
{
    auto self = static_cast<DirectSni*>(user_data);

    if (g_strcmp0(property_name, "Category") == 0)
        return g_variant_new_string("ApplicationStatus");

    if (g_strcmp0(property_name, "Id") == 0)
        return g_variant_new_string("radiotray-ng");

    if (g_strcmp0(property_name, "Title") == 0)
        return g_variant_new_string("RadioTray-NG");

    if (g_strcmp0(property_name, "Status") == 0)
        return g_variant_new_string("Active");

    if (g_strcmp0(property_name, "WindowId") == 0)
        return g_variant_new_int32(0);

    if (g_strcmp0(property_name, "IconThemePath") == 0)
        return g_variant_new_string(self->icon_theme_path.c_str());

    if (g_strcmp0(property_name, "Menu") == 0)
        return g_variant_new_object_path("/MenuBar");

    if (g_strcmp0(property_name, "ItemIsMenu") == 0)
        return g_variant_new_boolean(FALSE);

    if (g_strcmp0(property_name, "IconName") == 0)
        return g_variant_new_string(self->icon_name.c_str());

    if (g_strcmp0(property_name, "IconPixmap") == 0)
        return empty_pixmaps();

    if (g_strcmp0(property_name, "OverlayIconName") == 0)
        return g_variant_new_string("");

    if (g_strcmp0(property_name, "OverlayIconPixmap") == 0)
        return empty_pixmaps();

    if (g_strcmp0(property_name, "AttentionIconName") == 0)
        return g_variant_new_string("");

    if (g_strcmp0(property_name, "AttentionIconPixmap") == 0)
        return empty_pixmaps();

    if (g_strcmp0(property_name, "AttentionMovieName") == 0)
        return g_variant_new_string("");

    return nullptr;
}

void DirectSni::on_menu_method_call(
    GDBusConnection*,
    const gchar*,
    const gchar*,
    const gchar*,
    const gchar* method_name,
    GVariant* parameters,
    GDBusMethodInvocation* invocation,
    gpointer user_data)
{
    auto self = static_cast<DirectSni*>(user_data);

    if (g_strcmp0(method_name, "GetLayout") == 0)
    {
        GVariantBuilder root_props;
        g_variant_builder_init(&root_props, G_VARIANT_TYPE("a{sv}"));

        GVariantBuilder child_props;
        g_variant_builder_init(&child_props, G_VARIANT_TYPE("a{sv}"));
        // GNOME Shell must see one menu item so PopupMenu.numMenuItems > 0.
        // Keep the bridge item hidden so Shell has no visible row/pill to draw.
        // The companion GNOME bridge handles PRIMARY/SECONDARY click directly
        // via Activate(), while MIDDLE is intentionally suppressed.
        g_variant_builder_add(
            &child_props, "{sv}", "label", g_variant_new_string("\xE2\x80\x8B"));
        g_variant_builder_add(
            &child_props, "{sv}", "visible", g_variant_new_boolean(FALSE));
        g_variant_builder_add(
            &child_props, "{sv}", "enabled", g_variant_new_boolean(FALSE));

        GVariantBuilder child_children;
        g_variant_builder_init(&child_children, G_VARIANT_TYPE("av"));

        GVariant* child = g_variant_new(
            "(i@a{sv}@av)",
            1,
            g_variant_builder_end(&child_props),
            g_variant_builder_end(&child_children));

        GVariantBuilder root_children;
        g_variant_builder_init(&root_children, G_VARIANT_TYPE("av"));
        g_variant_builder_add_value(
            &root_children,
            g_variant_new_variant(child));

        GVariant* layout = g_variant_new(
            "(i@a{sv}@av)",
            0,
            g_variant_builder_end(&root_props),
            g_variant_builder_end(&root_children));

        g_dbus_method_invocation_return_value(
            invocation,
            g_variant_new("(u@(ia{sv}av))", 1u, layout));
        return;
    }

    if (g_strcmp0(method_name, "GetGroupProperties") == 0)
    {
        GVariantBuilder props;
        g_variant_builder_init(&props, G_VARIANT_TYPE("a(ia{sv})"));

        g_dbus_method_invocation_return_value(
            invocation,
            g_variant_new(
                "(@a(ia{sv}))",
                g_variant_builder_end(&props)));
        return;
    }

    if (g_strcmp0(method_name, "GetProperty") == 0)
    {
        gint id = 0;
        const gchar* property_name = "";
        g_variant_get(parameters, "(i&s)", &id, &property_name);

        GVariant* value = nullptr;

        if (id == 1 && g_strcmp0(property_name, "visible") == 0)
            value = g_variant_new_boolean(FALSE);
        else if (id == 1 && g_strcmp0(property_name, "enabled") == 0)
            value = g_variant_new_boolean(FALSE);
        else if (id == 1 && g_strcmp0(property_name, "label") == 0)
            value = g_variant_new_string("\xE2\x80\x8B");
        else
            value = g_variant_new_string("");

        g_dbus_method_invocation_return_value(
            invocation,
            g_variant_new("(v)", value));
        return;
    }

    if (g_strcmp0(method_name, "Event") == 0)
    {
        gint id = 0;
        const gchar* event_id = "";
        GVariant* data = nullptr;
        guint timestamp = 0;

        g_variant_get(
            parameters,
            "(i&svu)",
            &id,
            &event_id,
            &data,
            &timestamp);

        (void)timestamp;

        if (id == 0 &&
            g_strcmp0(event_id, "opened") == 0 &&
            self->activate_cb)
        {
            self->activate_cb(-1, -1);
        }

        if (data != nullptr)
            g_variant_unref(data);

        g_dbus_method_invocation_return_value(invocation, nullptr);
        return;
    }

    if (g_strcmp0(method_name, "EventGroup") == 0)
    {
        g_dbus_method_invocation_return_value(
            invocation,
            g_variant_new(
                "(@ai)",
                empty_int_array()));
        return;
    }

    if (g_strcmp0(method_name, "AboutToShow") == 0)
    {
        g_dbus_method_invocation_return_value(
            invocation,
            g_variant_new("(b)", FALSE));
        return;
    }

    if (g_strcmp0(method_name, "AboutToShowGroup") == 0)
    {
        g_dbus_method_invocation_return_value(
            invocation,
            g_variant_new(
                "(@ai@ai)",
                empty_int_array(),
                empty_int_array()));
        return;
    }

    g_dbus_method_invocation_return_value(invocation, nullptr);
}

GVariant* DirectSni::on_menu_get_property(
    GDBusConnection*,
    const gchar*,
    const gchar*,
    const gchar*,
    const gchar* property_name,
    GError**,
    gpointer)
{
    if (g_strcmp0(property_name, "Version") == 0)
        return g_variant_new_uint32(4);

    if (g_strcmp0(property_name, "TextDirection") == 0)
        return g_variant_new_string("ltr");

    if (g_strcmp0(property_name, "Status") == 0)
        return g_variant_new_string("normal");

    if (g_strcmp0(property_name, "IconThemePath") == 0)
        return g_variant_new_strv(nullptr, 0);

    return nullptr;
}
