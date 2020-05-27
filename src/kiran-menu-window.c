#include "kiran-menu-window.h"
#include "kiran-search-entry.h"
#include "kiran-app-button.h"
#include "kiran-power-button.h"
#include "kiran-category-item.h"
#include "kiran-app-item.h"
#include "kiran-expand-button.h"
#include <kiran-menu-based.h>
#include <kiran-menu-skeleton.h>
#include <glib/gi18n.h>
#include "config.h"

#define FREQUENT_APPS_SHOW_MAX  4           //开始菜单中显示的最常使用应用数量
#define NEW_APPS_SHOW_MAX       4           //开始菜单中显示的新安装应用数量

struct _KiranMenuWindow {
    GObject obj;

    GtkWidget *window, *parent;
    GtkWidget *all_apps_box, *new_apps_box, *frequent_apps_box, *favorite_apps_box;
    GtkWidget *search_results_box;
    GtkWidget *apps_view_stack, *overview_stack;
    GtkWidget *apps_overview_page, *category_overview_box;
    GtkWidget *all_apps_viewport, *default_apps_viewport;
    GtkWidget *back_button, *all_apps_button;
    GtkWidget *search_entry;
    GtkWidget *sidebar_box;
    GtkBuilder *builder;
    GResource *resource;
    GDBusProxy *proxy;

    const char *last_app_view;

    KiranMenuBased *backend;
    GList *category_list;
    GList *favorite_apps;

    GHashTable *category_items;
    GAppInfoMonitor *monitor;
};

G_DEFINE_TYPE(KiranMenuWindow, kiran_menu_window, G_TYPE_OBJECT)

/**
 * 删除container的所有子控件
 */
static void gtk_container_clear(GtkContainer *container)
{
    GList *children;

    children = gtk_container_get_children(container);
    g_list_foreach(children, (GFunc)gtk_widget_destroy, NULL);

    g_list_free(children);
}

static gboolean kiran_menu_window_load_styles(KiranMenuWindow *self)
{
    GtkCssProvider *provider = gtk_css_provider_get_default();

    gtk_css_provider_load_from_resource(provider, "/kiran-menu/menu.css");
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);

    return TRUE;
}

static void show_default_apps_page(KiranMenuWindow *self)
{
    GtkAdjustment *adjustment;

    //更改动画切换方向，看起来更自然
    gtk_stack_set_transition_type(GTK_STACK(self->apps_view_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_RIGHT);
    gtk_stack_set_visible_child_name(GTK_STACK(self->apps_view_stack), "default-apps-page");

    adjustment = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(self->default_apps_viewport));
    gtk_adjustment_set_value(adjustment, 0);
    gtk_widget_grab_focus(self->search_entry);
}

static void show_all_apps_page(KiranMenuWindow *self)
{
    GtkAdjustment *adjustment;

    //更改动画切换方向，看起来更自然
    gtk_stack_set_transition_type(GTK_STACK(self->apps_view_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT);
    gtk_stack_set_visible_child_name(GTK_STACK(self->apps_view_stack), "all-apps-page");

    //滚动到页面最上方
    adjustment = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(self->all_apps_viewport));
    gtk_adjustment_set_value(adjustment, 0);
    gtk_widget_grab_focus(self->search_entry);
}


static void show_apps_overview(KiranMenuWindow *self)
{
    gtk_stack_set_transition_type(GTK_STACK(self->overview_stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_visible_child_name(GTK_STACK(self->overview_stack), "apps-overview-page");
    gtk_widget_grab_focus(self->search_entry);
}


/**
 * 跳转到指定的应用分类
 *
 */
void kiran_menu_window_jump_to_category(KiranMenuWindow *self, const char *category_name)
{
    GtkAllocation item_allocation;
    KiranCategoryItem *item;
    GtkAdjustment *adjustment;

    //切换到所有应用列表
    show_apps_overview(self);
    show_all_apps_page(self);

    item = g_hash_table_lookup(self->category_items, category_name);
    if (!item) {
        g_warning("%s: Item for category '%s' not found\n", __func__, category_name);
        return;
    }

    adjustment = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(self->all_apps_viewport));
    gtk_widget_get_allocation(GTK_WIDGET(item), &item_allocation);
    gtk_adjustment_set_value(adjustment, item_allocation.y);

    //获取焦点
    gtk_widget_grab_focus(GTK_WIDGET(item));
}

void grab_pointer(KiranMenuWindow *self)
{
    GdkEventMask mask = GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK;

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gdk_pointer_grab(gtk_widget_get_window(self->window), TRUE, mask, NULL, NULL, GDK_CURRENT_TIME);
    gdk_keyboard_grab(gtk_widget_get_window(self->window), TRUE, GDK_CURRENT_TIME);
    G_GNUC_END_IGNORE_DEPRECATIONS

}

void ungrab_pointer(KiranMenuWindow *self)
{

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gdk_pointer_ungrab(GDK_CURRENT_TIME);
    gdk_keyboard_ungrab(GDK_CURRENT_TIME);
    G_GNUC_END_IGNORE_DEPRECATIONS
}
/**
 * 为给定的app创建应用程序列表项目对象
 */
KiranAppItem *kiran_menu_window_create_app_item(KiranMenuWindow *self, KiranApp *app)
{
    KiranAppItem *app_item = kiran_app_item_new(app);

    g_signal_connect_swapped(app_item, "app-launched", G_CALLBACK(gtk_widget_hide), self->window);
    return app_item;
}

/**
 * 为开始菜单侧边栏创建应用快捷按钮
 */
void kiran_menu_window_add_app_button(KiranMenuWindow *self,
        const char *icon_resource,
        const char *tooltip_text,
        const char *app_cmdline)
{
    KiranAppButton *app_btn = kiran_app_button_new(icon_resource, tooltip_text, app_cmdline);

    g_signal_connect_swapped(app_btn, "app-launched", G_CALLBACK(gtk_widget_hide), self->window);
    gtk_container_add(GTK_CONTAINER(self->sidebar_box), GTK_WIDGET(app_btn));
    gtk_widget_show(GTK_WIDGET(app_btn));
}



/**
 * 在分类选择视图中点击分类时的回调函数 
 */
static void category_selected_callback(KiranMenuWindow *self, KiranCategoryItem *item)
{
    const char *category_name;

    category_name = kiran_category_item_get_category_name(item);

    g_message("%s: jump to category '%s'\n", __func__, category_name);
    kiran_menu_window_jump_to_category(self, category_name);
}


/**
 * 显示分类选择视图
 *
 */
static void show_category_overview(KiranMenuWindow *self, GtkButton *button)
{
    GList *ptr;
    const char *current_category;

    current_category = kiran_category_item_get_category_name(KIRAN_CATEGORY_ITEM(button));

    //清空分类选择列表
    gtk_container_foreach(GTK_CONTAINER(self->category_overview_box), (GtkCallback)gtk_widget_destroy, NULL);

    for (ptr = self->category_list; ptr != NULL; ptr = ptr->next) {
        GtkStyleContext *context;
        GtkWidget *category_item;


        category_item = GTK_WIDGET(kiran_category_item_new((char*)ptr->data, TRUE));
        gtk_widget_set_can_focus(category_item, TRUE);
        gtk_container_add(GTK_CONTAINER(self->category_overview_box), category_item);
        gtk_widget_show_all(category_item);

        if (!strcmp((char*)ptr->data, current_category)) {
            //将当前分类设置为默认激活状态

            gtk_widget_grab_focus(category_item);
            g_message("%s: found same category\n", __func__);
        }
        g_signal_connect_swapped(category_item, "clicked", G_CALLBACK(category_selected_callback), self);
    }
    gtk_stack_set_transition_type(GTK_STACK(self->overview_stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_visible_child_name(GTK_STACK(self->overview_stack), "category-overview-page");
}


/**
 * 应用搜索框停止搜索时回调函数
 *
 */
static void search_stop_callback(KiranMenuWindow *self)
{
    //返回到搜索之前的页面
    gtk_stack_set_transition_type(GTK_STACK(self->apps_view_stack), GTK_STACK_TRANSITION_TYPE_NONE);
    gtk_stack_set_visible_child_name(GTK_STACK(self->apps_view_stack), self->last_app_view?self->last_app_view:"default-apps-page");
}

/**
 * 应用搜索框内容变化时回调函数
 *
 */
static void search_change_callback(KiranMenuWindow *self)
{
    GList *result_apps, *ptr;
    const gchar *keyword, *visible_view;
    KiranCategoryItem *category_item;

    if (gtk_entry_get_text_length(GTK_ENTRY(self->search_entry)) == 0) {
        //搜索内容为空，停止搜索，并返回上一个页面
        search_stop_callback(self);
        return;
    }

    //记录搜索前的页面，在搜索返回时使用
    visible_view = gtk_stack_get_visible_child_name(GTK_STACK(self->apps_view_stack));
    if (strcmp(visible_view, "search-results-page"))
        self->last_app_view = visible_view;

    //切换到搜索页面
    gtk_stack_set_transition_type(GTK_STACK(self->apps_view_stack), GTK_STACK_TRANSITION_TYPE_NONE);
    gtk_stack_set_visible_child_name(GTK_STACK(self->apps_view_stack), "search-results-page");

    //清空之前的搜索结果
    gtk_container_foreach(GTK_CONTAINER(self->search_results_box), (GtkCallback)gtk_widget_destroy, NULL);

    keyword = gtk_entry_get_text(GTK_ENTRY(self->search_entry));
    result_apps = kiran_menu_based_search_app_ignore_case(self->backend, keyword);

    if (!g_list_length(result_apps)) {

        //搜索结果为空
        GtkStyleContext *context;
        GtkWidget *label = gtk_label_new(_("No Apps match the results!"));

        context = gtk_widget_get_style_context(label);
        gtk_style_context_add_class(context, "search-empty-prompt");

        gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
        gtk_widget_set_valign(label, GTK_ALIGN_CENTER);
        gtk_widget_set_hexpand(label, TRUE);
        gtk_widget_set_vexpand(label, TRUE);

        gtk_container_add(GTK_CONTAINER(self->search_results_box), label);
        gtk_widget_show(label);
        return;
    }

    category_item = kiran_category_item_new(_("Search Results"), FALSE);
    gtk_container_add(GTK_CONTAINER(self->search_results_box), GTK_WIDGET(category_item));

    for (ptr = result_apps; ptr != NULL; ptr = ptr->next)
    {
        KiranAppItem *app_item;
        KiranApp *app = ptr->data;

        app_item = kiran_menu_window_create_app_item(self, app);
        g_message("Found result app '%s'\n", kiran_app_get_name(app));
        gtk_container_add(GTK_CONTAINER(self->search_results_box), GTK_WIDGET(app_item));
    }
    gtk_widget_show_all(GTK_WIDGET(self->search_results_box));
    g_list_free_full(result_apps, g_object_unref);
}

/**
 * 当列表项获取到焦点时，检查列表项在所在的滚动框中是否全部可见，如果没有全部可见，通过移动滚动条
 * 来保证列表项可见
 */
gboolean item_focus_in_callback(GtkWidget *widget, GdkEventFocus *ev, gpointer userdata)
{
    GtkAllocation allocation;
    GtkAdjustment *adjustment;
    GdkWindow *bin_win, *view_win;
    int bin_height, view_height;
    int adjust_value, delta;

    GtkViewport *viewport = userdata;

    gtk_widget_get_allocation(widget, &allocation);

    adjustment = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(viewport));
    bin_win = gtk_viewport_get_bin_window(GTK_VIEWPORT(viewport));
    view_win = gtk_viewport_get_view_window(GTK_VIEWPORT(viewport));

    adjust_value = (int)gtk_adjustment_get_value(adjustment);
    bin_height = gdk_window_get_height(bin_win);
    view_height = gdk_window_get_height(view_win);

    delta = (allocation.y + allocation.height) - (adjust_value + view_height);
    if (delta > 0) {
        //控件绘制区域在滚动区域内并非全部可见，需要滚动
        gtk_adjustment_set_value(adjustment, adjust_value + delta);
    }

    return FALSE;
}

/**
 *
 * 加载应用程序和分类列表
 *
 */
void kiran_menu_window_load_applications(KiranMenuWindow *self)
{
    GHashTableIter iter;
    gpointer key, value;
    GList *node;

    /**
     * 清空原来的应用程序数据
     */
    gtk_container_clear(GTK_CONTAINER(self->all_apps_box));
    g_list_free_full(self->category_list, g_free);
    g_hash_table_remove_all(self->category_items);

    self->category_list = kiran_menu_based_get_category_names(self->backend);

    for (node = self->category_list; node != NULL; node = node->next)
    {
        GList *apps, *ptr;
        KiranCategoryItem *category_item;
        KiranAppItem *app_item;
        GtkWidget *list_box;
        gchar *category_name = node->data;

        apps = kiran_menu_based_get_category_apps(self->backend, category_name);

        //添加应用分类标签
        category_item = kiran_category_item_new(category_name, TRUE);
        g_hash_table_insert(self->category_items, g_strdup(category_name), g_object_ref(category_item));
        gtk_container_add(GTK_CONTAINER(self->all_apps_box), GTK_WIDGET(category_item));
        g_signal_connect(category_item, "focus-in-event",
                G_CALLBACK(item_focus_in_callback), self->all_apps_viewport);

        //添加应用程序标签
        list_box = gtk_list_box_new();
        for (ptr = apps; ptr != NULL; ptr = ptr->next) {
            KiranApp *app = ptr->data;

            app_item = kiran_menu_window_create_app_item(self, app);
            gtk_container_add(GTK_CONTAINER(self->all_apps_box), GTK_WIDGET(app_item));

            g_signal_connect(app_item, "focus-in-event",
                    G_CALLBACK(item_focus_in_callback), self->all_apps_viewport);
        }
        gtk_widget_show_all(self->all_apps_box);
        g_signal_connect_swapped(category_item, "clicked", G_CALLBACK(show_category_overview), self);
    }
}

/**
 *
 * 加载收藏夹列表
 */
void kiran_menu_window_load_favorites(KiranMenuWindow *self)
{
    GList *fav_list, *ptr;
    KiranCategoryItem *category_item;

    gtk_container_clear(GTK_CONTAINER(self->favorite_apps_box));

    category_item = kiran_category_item_new(_("Favorites"), FALSE);
    fav_list = kiran_menu_based_get_favorite_apps(self->backend);
    gtk_container_add(GTK_CONTAINER(self->favorite_apps_box), GTK_WIDGET(category_item));
    g_message("%d favorite apps found\n", g_list_length(fav_list));

    if (!g_list_length(fav_list)) {
        GtkWidget *label = gtk_label_new(_("No apps available"));

        gtk_widget_set_name(label, "app-empty-prompt");
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_container_add(GTK_CONTAINER(self->favorite_apps_box), label);
        gtk_widget_show_all(self->favorite_apps_box);
        return;
    }

    for (ptr = fav_list; ptr != NULL; ptr = ptr->next)
    {
        KiranAppItem *app_item;
        KiranApp *app = ptr->data;

        app_item = kiran_menu_window_create_app_item(self, app);
        gtk_container_add(GTK_CONTAINER(self->favorite_apps_box), GTK_WIDGET(app_item));
        g_signal_connect(app_item, "focus-in-event",
                G_CALLBACK(item_focus_in_callback), self->default_apps_viewport);
    }
    //g_list_free_full(fav_list, g_object_unref);
    gtk_widget_show_all(self->favorite_apps_box);
    self->favorite_apps = fav_list;
}

/**
 *
 * 加载常用应用列表
 */
void kiran_menu_window_load_frequent_apps(KiranMenuWindow *self)
{
    GList *recently_apps, *ptr;
    KiranCategoryItem *category_item;

    gtk_container_clear(GTK_CONTAINER(self->frequent_apps_box));

    recently_apps = kiran_menu_based_get_nfrequent_apps(self->backend, FREQUENT_APPS_SHOW_MAX);
    g_message("%d frequent apps found\n", g_list_length(recently_apps));

    if (!g_list_length(recently_apps)) {
        //最近使用列表为空
        return;
    }

    category_item = kiran_category_item_new(_("Frequently Used"), FALSE);
    gtk_container_add(GTK_CONTAINER(self->frequent_apps_box), GTK_WIDGET(category_item));
    for (ptr = recently_apps; ptr != NULL; ptr = ptr->next)
    {
        KiranAppItem *app_item;
        KiranApp *app = ptr->data;

        app_item = kiran_menu_window_create_app_item(self, app);
        g_message("Found frequent app '%s'\n", kiran_app_get_name(app));
        gtk_container_add(GTK_CONTAINER(self->frequent_apps_box), GTK_WIDGET(app_item));
        g_signal_connect(app_item, "focus-in-event",
                G_CALLBACK(item_focus_in_callback), self->default_apps_viewport);
    }
    gtk_widget_show_all(self->frequent_apps_box);
    g_list_free_full(recently_apps, g_object_unref);
}

static void toggle_more_new_apps(GtkToggleButton *button, gpointer userdata)
{
    GtkWidget *more_box = userdata;
    gboolean active = gtk_toggle_button_get_active(button);

    gtk_widget_set_visible(more_box, active);
}

/**
 *
 * 加载新安装应用列表
 */
void kiran_menu_window_load_new_apps(KiranMenuWindow *self)
{
    GList *new_apps, *ptr;
    KiranCategoryItem *category_item;
    GtkWidget *more_box = NULL, *expand_button;
    int index;

    gtk_container_clear(GTK_CONTAINER(self->new_apps_box));

    new_apps = kiran_menu_based_get_nnew_apps(self->backend, -1);
    g_message("%d new apps found\n", g_list_length(new_apps));

    if (!g_list_length(new_apps)) {
        //最近使用列表为空，不再显示“新安装”分类标签
        return;
    }

    category_item = kiran_category_item_new(_("New Installed"), FALSE);
    gtk_container_add(GTK_CONTAINER(self->new_apps_box), GTK_WIDGET(category_item));
    gtk_widget_show_all(GTK_WIDGET(category_item));

    for (ptr = new_apps, index = 0; ptr != NULL; ptr = ptr->next, index++)
    {
        KiranAppItem *app_item;
        KiranApp *app = ptr->data;

        app_item = kiran_menu_window_create_app_item(self, app);
        g_message("Found new app '%s'\n", kiran_app_get_name(app));

        g_signal_connect(app_item, "focus-in-event",
                G_CALLBACK(item_focus_in_callback), self->all_apps_viewport);
        if (index < NEW_APPS_SHOW_MAX)
            gtk_container_add(GTK_CONTAINER(self->new_apps_box), GTK_WIDGET(app_item));
        else {
            if (!more_box)
                more_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

            gtk_container_add(GTK_CONTAINER(more_box), GTK_WIDGET(app_item));
        }
    }
    gtk_widget_show_all(self->new_apps_box);
    if (more_box) {
        expand_button = kiran_expand_button_new(FALSE);

        g_signal_connect(expand_button, "toggled", G_CALLBACK(toggle_more_new_apps), more_box);

        gtk_container_add(GTK_CONTAINER(self->new_apps_box), more_box);
        gtk_container_add(GTK_CONTAINER(self->new_apps_box), expand_button);
        gtk_widget_show_all(expand_button);
        gtk_widget_show_all(more_box);
        gtk_widget_set_visible(more_box, FALSE);
    }

    g_list_free_full(new_apps, g_object_unref);
}

gboolean button_press_event_callback(GtkWidget *widget, GdkEventButton *event, gpointer userdata)
{
    int root_x, root_y;
    int width, height;
    GdkWindow *window;

    g_message("got button press event\n");

    window = gtk_widget_get_window(widget);
    gdk_window_get_root_origin(window, &root_x, &root_y);
    width = gdk_window_get_width(window);
    height = gdk_window_get_height(window);

    g_message("window (%d, %d) -> (%d, %d), event (%d, %d)\n", root_x, root_y, root_x + width, root_y + height, (int)event->x_root, (int)event->y_root);

    if (((int)event->x_root < root_x || (int)event->x_root > root_x + width) ||
        ((int)event->y_root < root_y || (int)event->y_root > root_y + height))
        gtk_widget_hide(widget);

    return FALSE;
}

/**
 * 加载应用程序数据，包括常用应用、收藏应用、新安装应用和所有应用列表
 */
static void kiran_menu_window_reload_app_data(KiranMenuWindow *self)
{
    kiran_menu_window_load_frequent_apps(self);
    kiran_menu_window_load_favorites(self);
    kiran_menu_window_load_new_apps(self);
    kiran_menu_window_load_applications(self);
}

static void window_map_handler(GtkWidget *widget, GdkEvent *event, gpointer userdata)
{
    KiranMenuWindow *self = userdata;

    //将开始菜单窗口总保持在上层
    gtk_window_set_keep_above(GTK_WINDOW(widget), TRUE);
    grab_pointer(userdata);

    gtk_widget_grab_focus(self->search_entry);
}

static void window_unmap_handler(GtkWidget *widget, GdkEvent *event, gpointer userdata)
{
    gtk_window_set_keep_above(GTK_WINDOW(widget), FALSE);
    ungrab_pointer(userdata);
}

static void window_active_change_callback(KiranMenuWindow *self)
{
    if (gtk_window_is_active(GTK_WINDOW(self->window))) {
        //开始菜单窗口位于最前方时，抓取焦点
        g_message("menu window is active\n");
        grab_pointer(self);
    } else
        g_message("menu window is inactive\n");
}

static gboolean key_press_event_callback(GtkWidget *widget, GdkEventKey *event, gpointer userdata)
{
    KiranMenuWindow *self = userdata;

    if (event->keyval == GDK_KEY_Escape) {

        const char *page_name = gtk_stack_get_visible_child_name(GTK_STACK(self->overview_stack));
        if (!strcmp(page_name, "category-overview-page")) {
            //从分类跳转视图返回到所有应用视图
            show_apps_overview(self);
            show_all_apps_page(self);
            return TRUE;
        }

        page_name = gtk_stack_get_visible_child_name(GTK_STACK(self->apps_view_stack));
        if (!strcmp(page_name, "search-results-page"))
            //当前处于搜索页面，按下ESC键应当取消搜索
            return FALSE;
        else {
            //按下ESC时隐藏开始菜单窗口
            gtk_widget_hide(widget);
        }
        return TRUE;
    }

    return FALSE;
}

gboolean leave_notify_callback(GtkWidget *widget, GdkEventCrossing *ev, gpointer userdata)
{
    GtkAllocation allocation;
    GdkWindow *window;
    int root_x, root_y;
    KiranMenuWindow *self = userdata;

    window = gtk_widget_get_window(widget);
    gtk_widget_get_allocation(widget, &allocation);
    gdk_window_get_origin(window, &root_x, &root_y);

    g_message("window (%d, %d)->(%d, %d), event (%d, %d)\n",
            root_x, root_y, root_x + allocation.width, root_y + allocation.height,
            (int)ev->x_root, (int)ev->y_root);

    if (((int)ev->x_root > root_x && (int)ev->x_root < root_x + allocation.width) &&
        ((int)ev->y_root > root_y && (int)ev->y_root < root_y + allocation.height))
        return FALSE;

    gtk_widget_grab_focus(self->search_entry);
    return FALSE;
}

/**
 * 重新调整开始菜单窗口大小，保证其最小大小不超过可显示的屏幕区域大小
 *
 */
static void auto_resize_window(GtkWidget *window)
{
    GdkMonitor *monitor;
    GdkDisplay *display;
    GdkRectangle rect;
    int requested_width, requested_height;

    display = gdk_display_get_default();
    if (gtk_widget_get_realized(window)) {
        monitor = gdk_display_get_monitor_at_window(display, gtk_widget_get_window(window));
    } else {
        monitor = gdk_display_get_primary_monitor(display);
    }
    
    gtk_widget_get_size_request(window, &requested_width, &requested_height);

    //获取实际可用的显示区域大小，避免覆盖panel
    gdk_monitor_get_workarea(monitor, &rect);

    if (requested_width > rect.width)
        requested_width = rect.width;

    if (requested_height > rect.height)
        requested_height = rect.height;

    gtk_widget_set_size_request(window, requested_width, requested_height);
}

void kiran_menu_window_init(KiranMenuWindow *self)
{
    GError *error = NULL;
    GtkWidget *search_box, *separator, *power_btn;
    GtkWidget *top_box, *bottom_box;
    KiranMenuSkeleton *skeleton;
    GdkScreen *screen;

    self->monitor = g_app_info_monitor_get();
    self->backend = kiran_menu_based_skeleton_get();
    self->last_app_view = NULL;
    self->category_list = NULL;
    self->category_items = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)g_free, (GDestroyNotify)g_object_unref);

    self->resource = g_resource_load(RESOURCE_PATH, &error);
    if (!self->resource) {
        /**
         * 如果资源加载失败，所有界面元素都将无法获取，所以要直接退出程序
         */
        g_error("Failed to load resource '%s': %s\n", RESOURCE_PATH, error->message);
        exit(1);
    }
    g_resources_register(self->resource);

    kiran_menu_window_load_styles(self);
    self->builder = gtk_builder_new_from_resource("/kiran-menu/ui/menu");
    self->window = GTK_WIDGET(gtk_builder_get_object(self->builder, "menu-window"));
    self->window = g_object_ref(self->window);
    gtk_window_set_decorated(GTK_WINDOW(self->window), FALSE);

    self->all_apps_box= GTK_WIDGET(gtk_builder_get_object(self->builder, "all-apps-box"));
    self->frequent_apps_box = GTK_WIDGET(gtk_builder_get_object(self->builder, "frequent-apps-box"));
    self->favorite_apps_box = GTK_WIDGET(gtk_builder_get_object(self->builder, "favorite-apps-box"));
    self->new_apps_box = GTK_WIDGET(gtk_builder_get_object(self->builder, "new-apps-box"));
    self->search_results_box = GTK_WIDGET(gtk_builder_get_object(self->builder, "search-results-box"));
    self->sidebar_box = GTK_WIDGET(gtk_builder_get_object(self->builder, "sidebar-box"));

    self->category_overview_box = GTK_WIDGET(gtk_builder_get_object(self->builder, "category-overview-box"));
    self->all_apps_viewport = GTK_WIDGET(gtk_builder_get_object(self->builder, "all-apps-viewport"));
    self->default_apps_viewport = GTK_WIDGET(gtk_builder_get_object(self->builder, "default-apps-viewport"));

    self->apps_view_stack = GTK_WIDGET(gtk_builder_get_object(self->builder, "apps-view-stack"));
    self->overview_stack = GTK_WIDGET(gtk_builder_get_object(self->builder, "overview-stack"));

    self->search_entry = GTK_WIDGET(kiran_search_entry_new());
    search_box = GTK_WIDGET(gtk_builder_get_object(self->builder, "search-box"));
    gtk_container_add(GTK_CONTAINER(search_box), self->search_entry);
    gtk_widget_show(self->search_entry);

    g_signal_connect_swapped(self->search_entry, "search-changed", G_CALLBACK(search_change_callback), self);
    g_signal_connect_swapped(self->search_entry, "stop-search", G_CALLBACK(search_stop_callback), self);

    self->back_button = GTK_WIDGET(gtk_builder_get_object(self->builder, "back-button"));
    self->all_apps_button = GTK_WIDGET(gtk_builder_get_object(self->builder, "all-apps-button"));

    separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_name(separator, "sidebar-separator");

    power_btn = GTK_WIDGET(kiran_power_button_new());
    g_signal_connect_swapped(power_btn, "action-triggered", G_CALLBACK(gtk_widget_hide), self->window);

    gtk_orientable_set_orientation(GTK_ORIENTABLE(self->sidebar_box), GTK_ORIENTATION_VERTICAL);
    kiran_menu_window_add_app_button(self, "/kiran-menu/sidebar/home-dir", _("Home Directory"), "caja");
    kiran_menu_window_add_app_button(self, "/kiran-menu/sidebar/monitor", _("System monitor"), "mate-system-monitor");
    kiran_menu_window_add_app_button(self, "/kiran-menu/sidebar/help", _("Open help"), "yelp");
    gtk_container_add(GTK_CONTAINER(self->sidebar_box), separator);
    kiran_menu_window_add_app_button(self, "/kiran-menu/sidebar/avatar", _("About me"), "mate-about-me");
    kiran_menu_window_add_app_button(self, "/kiran-menu/sidebar/settings", _("Control center"), "mate-control-center");
    gtk_container_add(GTK_CONTAINER(self->sidebar_box), power_btn);
    gtk_widget_show_all(self->sidebar_box);

    g_signal_connect_swapped(self->back_button, "clicked", G_CALLBACK(show_default_apps_page), self);
    g_signal_connect_swapped(self->all_apps_button, "clicked", G_CALLBACK(show_all_apps_page), self);

    gtk_widget_add_events(self->window, GDK_KEY_RELEASE_MASK);
    g_signal_connect(self->window, "map-event", G_CALLBACK(window_map_handler), self);
    g_signal_connect(self->window, "unmap-event", G_CALLBACK(window_unmap_handler), self);
    g_signal_connect(self->window, "button-press-event", G_CALLBACK(button_press_event_callback), self);
    g_signal_connect(self->window, "key-press-event", G_CALLBACK(key_press_event_callback), self);
    g_signal_connect(self->window, "leave-notify-event", G_CALLBACK(leave_notify_callback), self);

    g_signal_connect_swapped(self->window, "notify::is-active", G_CALLBACK(window_active_change_callback), self);

    /* 加载应用程序数据 */
    kiran_menu_window_reload_app_data(self);

    skeleton = KIRAN_MENU_SKELETON(self->backend);

    g_signal_connect_swapped(skeleton, "new-app-changed", G_CALLBACK(kiran_menu_window_load_new_apps), self);
    g_signal_connect_swapped(skeleton, "favorite-app-deleted", G_CALLBACK(kiran_menu_window_load_favorites), self);
    g_signal_connect_swapped(skeleton, "favorite-app-added", G_CALLBACK(kiran_menu_window_load_favorites), self);
    g_signal_connect_swapped(skeleton, "frequent-usage-app-changed", G_CALLBACK(kiran_menu_window_load_frequent_apps), self);
    g_signal_connect_swapped(skeleton, "app-changed", G_CALLBACK(kiran_menu_window_reload_app_data), self);

    //屏幕大小或分辨率变化时自动调整开始菜单窗口大小
    screen = gdk_screen_get_default();
    auto_resize_window(self->window);
    g_signal_connect_swapped(screen, "size-changed", G_CALLBACK(auto_resize_window), self->window);
    g_signal_connect_swapped(screen, "monitors-changed", G_CALLBACK(auto_resize_window), self->window);

}

void kiran_menu_window_finalize(GObject *obj)
{
    KiranMenuWindow *self = KIRAN_MENU_WINDOW(obj);

    g_object_unref(self->window);
    g_object_unref(self->builder);
    g_resources_unregister(self->resource);
    g_list_free_full(self->category_list, g_free);
    g_hash_table_destroy(self->category_items);
    g_list_free_full(self->favorite_apps, g_object_unref);
    g_object_unref(self->monitor);
}

void kiran_menu_window_class_init(KiranMenuWindowClass *kclass)
{
    G_OBJECT_CLASS(kclass)->finalize = kiran_menu_window_finalize;
}

KiranMenuWindow *kiran_menu_window_new(GtkWidget *parent)
{
    KiranMenuWindow *window;

    window = g_object_new(KIRAN_TYPE_MENU_WINDOW, NULL);
    window->parent = parent;
    return window;
}

void kiran_menu_window_reset_layout(KiranMenuWindow *self)
{
    show_apps_overview(self);
    show_default_apps_page(self);

    gtk_entry_set_text(GTK_ENTRY(self->search_entry), "");
    gtk_widget_grab_focus(self->search_entry);
}

GtkWidget *kiran_menu_window_get_window(KiranMenuWindow *window)
{
    return window->window;
}
