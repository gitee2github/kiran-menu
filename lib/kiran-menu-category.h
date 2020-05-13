/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 17:30:32
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-11 13:55:41
 * @Description  : 管理菜单中的APP的分类
 * @FilePath     : /kiran-menu-2.0/lib/kiran-menu-category.h
 */
#pragma once

#include <gio/gio.h>

#include "lib/kiran-menu-system.h"
#include "lib/kiran-menu-unit.h"

G_BEGIN_DECLS

#define KIRAN_TYPE_MENU_CATEGORY (kiran_menu_category_get_type())
G_DECLARE_FINAL_TYPE(KiranMenuCategory, kiran_menu_category, KIRAN, MENU_CATEGORY, KiranMenuUnit)

/**
 * @description: 创建KiranMenuCategory对象
 * @return:
 * @author: tangjie02
 */
KiranMenuCategory *kiran_menu_category_get_new();

/**
 * @description: 
 * @param {type} 
 * @return: 
 * @author: tangjie02
 */
void kiran_menu_category_flush_app(KiranMenuCategory *self, KiranApp *app);

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
gboolean kiran_menu_category_add_app(KiranMenuCategory *self,
                                     const char *category_name,
                                     KiranMenuApp *menu_app);

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
gboolean kiran_menu_category_del_app(KiranMenuCategory *self,
                                     const char *category_name,
                                     KiranMenuApp *menu_app);

/**
 * @description:
 * @param {type}
 * @return:
 * @author: tangjie02
 */
GList *kiran_menu_category_get_apps(KiranMenuCategory *self,
                                    const char *category_name);

/**
 * @description: 获取所有分类
 * @param {type}
 * @return:
 * @author: tangjie02
 */
GList *kiran_menu_category_get_names(KiranMenuCategory *self);

/**
 * @description: 获取所有分类和APP
 * @param {type}
 * @return:
 * @author: tangjie02
 */
GHashTable *kiran_menu_category_get_all(KiranMenuCategory *self);

G_END_DECLS
