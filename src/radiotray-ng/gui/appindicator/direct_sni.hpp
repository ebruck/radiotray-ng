#pragma once

#include <gio/gio.h>

#include <functional>
#include <string>

class DirectSni
{
public:
    using ActivateCallback = std::function<void(gint, gint)>;
    using ScrollCallback = std::function<void(gint, const std::string&)>;

    DirectSni(ActivateCallback activate_cb, ScrollCallback scroll_cb);
    ~DirectSni();

    DirectSni(const DirectSni&) = delete;
    DirectSni& operator=(const DirectSni&) = delete;

    bool start();
    void set_icon(const std::string& icon);

private:
    static void on_sni_method_call(
        GDBusConnection* connection,
        const gchar* sender,
        const gchar* object_path,
        const gchar* interface_name,
        const gchar* method_name,
        GVariant* parameters,
        GDBusMethodInvocation* invocation,
        gpointer user_data);

    static GVariant* on_sni_get_property(
        GDBusConnection* connection,
        const gchar* sender,
        const gchar* object_path,
        const gchar* interface_name,
        const gchar* property_name,
        GError** error,
        gpointer user_data);

    static void on_menu_method_call(
        GDBusConnection* connection,
        const gchar* sender,
        const gchar* object_path,
        const gchar* interface_name,
        const gchar* method_name,
        GVariant* parameters,
        GDBusMethodInvocation* invocation,
        gpointer user_data);

    static GVariant* on_menu_get_property(
        GDBusConnection* connection,
        const gchar* sender,
        const gchar* object_path,
        const gchar* interface_name,
        const gchar* property_name,
        GError** error,
        gpointer user_data);

    void emit_icon_changed();

    ActivateCallback activate_cb;
    ScrollCallback scroll_cb;

    GDBusConnection* connection = nullptr;
    GDBusNodeInfo* sni_node = nullptr;
    GDBusNodeInfo* menu_node = nullptr;
    guint sni_registration = 0;
    guint menu_registration = 0;

    std::string icon_name{"radiotray-ng-off"};
    std::string icon_theme_path;
};
