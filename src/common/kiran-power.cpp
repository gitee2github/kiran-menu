/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#include "kiran-power.h"
#include <iostream>
#include "lib/base.h"

#define SESSION_MANAGER_DBUS "org.gnome.SessionManager"
#define SESSION_MANAGER_PATH "/org/gnome/SessionManager"
#define SESSION_MANAGER_INTERFACE "org.gnome.SessionManager"

#define LOGIN_MANAGER_DBUS "org.freedesktop.login1"
#define LOGIN_MANAGER_PATH "/org/freedesktop/login1"
#define LOGIN_MANAGER_INTERFACE "org.freedesktop.login1.Manager"

#define DISPLAY_MANAGER_DBUS "org.freedesktop.DisplayManager"
#define DISPLAY_MANAGER_SEAT_PATH "/org/freedesktop/DisplayManager/Seat0"
#define DISPLAY_MANAGER_INTERFACE "org.freedesktop.DisplayManager.Seat"

static bool read_boolean_key_from_gsettings(const Glib::ustring &key, bool default_value)
{
    try
    {
        auto settings = Gio::Settings::create("org.mate.lockdown");
        std::vector<Glib::ustring> keys = settings->list_keys();

        if (std::find(keys.begin(), keys.end(), key.c_str()) == keys.end())
        {
            KLOG_WARNING("key '%s' not found in schema\n", key.c_str());
            return default_value;
        }
        return settings->get_boolean(key);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Failed to read settings from schema: %s", e.what().c_str());
        return default_value;
    }
}

bool KiranPower::suspend()
{
    try
    {
        Glib::Variant<bool> variant = Glib::Variant<bool>::create(false);
        Glib::VariantContainerBase container = Glib::VariantContainerBase::create_tuple(variant);
        Glib::RefPtr<Gio::DBus::Proxy> proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                                     LOGIN_MANAGER_DBUS,
                                                                                     LOGIN_MANAGER_PATH,
                                                                                     LOGIN_MANAGER_INTERFACE);

        proxy->call_sync("Suspend", container, 300);
        return true;
    }
    catch (const Gio::DBus::Error &e)
    {
        KLOG_WARNING("Failed to request suspend method: %s", e.what().c_str());
        return false;
    }
}

bool KiranPower::hibernate()
{
    try
    {
        Glib::Variant<bool> variant = Glib::Variant<bool>::create(false);
        Glib::VariantContainerBase container = Glib::VariantContainerBase::create_tuple(variant);
        Glib::RefPtr<Gio::DBus::Proxy> proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                                     LOGIN_MANAGER_DBUS,
                                                                                     LOGIN_MANAGER_PATH,
                                                                                     LOGIN_MANAGER_INTERFACE);

        proxy->call_sync("Hibernate", container, 300);
        return true;
    }
    catch (const Gio::DBus::Error &e)
    {
        KLOG_WARNING("Failed to request hibernate method: %s", e.what().c_str());
        return false;
    }
}
bool KiranPower::shutdown()
{
    try
    {
        Glib::RefPtr<Gio::DBus::Proxy> proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SESSION,
                                                                                     SESSION_MANAGER_DBUS,
                                                                                     SESSION_MANAGER_PATH,
                                                                                     SESSION_MANAGER_INTERFACE);

        proxy->call_sync("RequestShutdown");
        return true;
    }
    catch (const Gio::DBus::Error &e)
    {
        KLOG_WARNING("Failed to request shutdown method: %s", e.what().c_str());
        return false;
    }
}
bool KiranPower::reboot()
{
    try
    {
        Glib::RefPtr<Gio::DBus::Proxy> proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SESSION,
                                                                                     SESSION_MANAGER_DBUS,
                                                                                     SESSION_MANAGER_PATH,
                                                                                     SESSION_MANAGER_INTERFACE);

        proxy->call_sync("RequestReboot");
        return true;
    }
    catch (const Gio::DBus::Error &e)
    {
        KLOG_WARNING("Failed to request reboot method: %s", e.what().c_str());
        return false;
    }
}

bool KiranPower::logout(int mode)
{
    try
    {
        Glib::Variant<uint> variant = Glib::Variant<uint>::create(mode);
        Glib::VariantContainerBase container = Glib::VariantContainerBase::create_tuple(variant);
        Glib::RefPtr<Gio::DBus::Proxy> proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SESSION,
                                                                                     SESSION_MANAGER_DBUS,
                                                                                     SESSION_MANAGER_PATH,
                                                                                     SESSION_MANAGER_INTERFACE);

        proxy->call_sync("Logout", container);
        return true;
    }
    catch (const Gio::DBus::Error &e)
    {
        KLOG_WARNING("Failed to request logout method: %s", e.what().c_str());
        return false;
    }
}

bool KiranPower::can_suspend()
{
    if (read_boolean_key_from_gsettings("disable-suspend", false))
        return false;

    try
    {
        Glib::VariantBase result;
        Glib::ustring data;
        Glib::RefPtr<Gio::DBus::Proxy> proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                                     LOGIN_MANAGER_DBUS,
                                                                                     LOGIN_MANAGER_PATH,
                                                                                     LOGIN_MANAGER_INTERFACE);

        result = proxy->call_sync("CanSuspend").get_child();
        data = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(result).get();

        return (data == "yes");
    }
    catch (const Gio::DBus::Error &e)
    {
        //如果获取失败，就假设其可以挂起，由挂起操作调用时做检查
        KLOG_WARNING("Failed to query CanSuspend: %s", e.what().c_str());
        return true;
    }
}

bool KiranPower::can_hibernate()
{
    if (read_boolean_key_from_gsettings("disable-suspend", false))
        return false;

    try
    {
        Glib::VariantBase result;
        Glib::ustring data;
        Glib::RefPtr<Gio::DBus::Proxy> proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                                     LOGIN_MANAGER_DBUS,
                                                                                     LOGIN_MANAGER_PATH,
                                                                                     LOGIN_MANAGER_INTERFACE);

        result = proxy->call_sync("CanHibernate").get_child();
        data = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(result).get();

        return (data == "yes");
    }
    catch (const Gio::DBus::Error &e)
    {
        //如果获取失败，就假设其可以挂起，由挂起操作调用时做检查
        KLOG_WARNING("Failed to query CanHibernate: %s", e.what().c_str());
        return true;
    }
}

bool KiranPower::switch_user()
{
    try
    {
        Glib::RefPtr<Gio::DBus::Proxy> proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                                     DISPLAY_MANAGER_DBUS,
                                                                                     DISPLAY_MANAGER_SEAT_PATH,
                                                                                     DISPLAY_MANAGER_INTERFACE);

        proxy->call_sync("SwitchToGreeter", Glib::VariantContainerBase(), 300);
        return true;
    }
    catch (const Gio::DBus::Error &e)
    {
        KLOG_WARNING("Failed to request SwitchToGreeter method: %s", e.what().c_str());
        return false;
    }
}

bool KiranPower::can_switchuser()
{
    bool result = false;

    try
    {
        auto settings = Gio::Settings::create("org.mate.session");

        result = settings->get_boolean("logout-prompt");
    }
    catch (const Glib::Exception &e)
    {
        result = false;
    }
    return result;
}

bool KiranPower::can_shutdown()
{
    try
    {
        Glib::RefPtr<Gio::DBus::Proxy> proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                                     LOGIN_MANAGER_DBUS,
                                                                                     LOGIN_MANAGER_PATH,
                                                                                     LOGIN_MANAGER_INTERFACE);

        auto result = proxy->call_sync("CanPowerOff").get_child();
        auto data = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(result).get();

        return (data == "yes");
    }
    catch (const Gio::DBus::Error &e)
    {
        //如果获取失败，就假设其可以关机，由关机操作调用时做检查
        KLOG_WARNING("Failed to query CanPowerOff: %s", e.what().c_str());
        return true;
    }
}

bool KiranPower::can_reboot()
{
    if (read_boolean_key_from_gsettings("disable-reboot", false))
    {
        return false;
    }

    try
    {
        Glib::VariantBase result;
        Glib::ustring data;
        Glib::RefPtr<Gio::DBus::Proxy> proxy = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                                     LOGIN_MANAGER_DBUS,
                                                                                     LOGIN_MANAGER_PATH,
                                                                                     LOGIN_MANAGER_INTERFACE);

        result = proxy->call_sync("CanReboot").get_child();
        data = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(result).get();

        return (data == "yes");
    }
    catch (const Gio::DBus::Error &e)
    {
        //如果获取失败，就假设其可以关机，由关机操作调用时做检查
        KLOG_WARNING("Failed to query Reboot: %s", e.what().c_str());
        return true;
    }
}

bool KiranPower::lock_screen()
{
    std::vector<std::string> argv;

    argv.push_back("mate-screensaver-command");
    argv.push_back("-l");

    try
    {
        Glib::spawn_async("", argv, Glib::SPAWN_SEARCH_PATH | Glib::SPAWN_STDOUT_TO_DEV_NULL);
        return true;
    }
    catch (const Glib::SpawnError &e)
    {
        return false;
    }
}