/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 17:21:54
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-11 13:50:57
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/kiran-menu-system.h
 */
#pragma once

#include <gio/gio.h>
#include <libwnck/libwnck.h>

#include "lib/kiran-menu-app.h"
#include "lib/kiran-menu-unit.h"

#define KIRAN_TYPE_MENU_SYSTEM (kiran_menu_system_get_type())

G_DECLARE_FINAL_TYPE(KiranMenuSystem, kiran_menu_system, KIRAN, MENU_SYSTEM, KiranMenuUnit)

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
GList *kiran_menu_system_get_apps(KiranMenuSystem *self);

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
KiranMenuApp *kiran_menu_system_lookup_app(KiranMenuSystem *self,
                                           const gchar *app_id);

/**
 * @description: 
 * @param {type} 
 * @return: 
 * @author: tangjie02
 */
KiranMenuApp *kiran_menu_system_lookup_apps_with_window(KiranMenuSystem *self,
                                                        WnckWindow *window);

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
GList *kiran_menu_system_get_nnew_apps(KiranMenuSystem *self, gint top_n);

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
GList *kiran_menu_system_get_all_sorted_apps(KiranMenuSystem *self);

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
KiranMenuSystem *kiran_menu_system_get_new(void);
