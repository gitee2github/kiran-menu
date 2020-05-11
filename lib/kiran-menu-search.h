/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 20:58:19
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-11 11:38:36
 * @Description  : 用于菜单搜索功能
 * @FilePath     : /kiran-menu-2.0/lib/kiran-menu-search.h
 */
#pragma once

#include <gio/gio.h>

#include "lib/kiran-menu-unit.h"

G_BEGIN_DECLS

#define KIRAN_TYPE_MENU_SEARCH (kiran_menu_search_get_type())

G_DECLARE_FINAL_TYPE(KiranMenuSearch, kiran_menu_search, KIRAN, MENU_SEARCH, KiranMenuUnit)

KiranMenuSearch *kiran_menu_search_get_new();

/**
 * @description: 通过关键字搜索app
 * @param {type}
 * @return:
 * @author: tangjie02
 */
GList *kiran_menu_search_by_keyword(KiranMenuSearch *self,
                                    const char *keyword,
                                    gboolean ignore_case,
                                    GList *apps);

G_END_DECLS
