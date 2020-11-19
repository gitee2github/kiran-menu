#ifndef MENU_APPLET_WINDOW_H
#define MENU_APPLET_WINDOW_H

#include <gtkmm.h>
#include "kiran-user-info.h"
#include "menu-app-item.h"
#include "menu-category-item.h"
#include "menu-profile.h"

#include "menu-skeleton.h"
#include "workarea-monitor.h"


enum {
    VIEW_APPS_LIST = 0,         /* 应用列表视图 */
    VIEW_CATEGORY_SELECTION,    /* 分类选择跳转视图 */
    VIEW_COMPACT_FAVORITES      /* 紧凑模式下的收藏夹视图 */
};

enum {
    PAGE_ALL_APPS_LIST = 0,     /*所有应用页面 */
    PAGE_SEARCH_RESULT          /*搜索结果页面*/
};

class MenuAppletWindow : public Gtk::Window
{
public:
    MenuAppletWindow(Gtk::WindowType window_type = Gtk::WINDOW_TOPLEVEL);
    ~MenuAppletWindow() override;

    /**
     * @brief 开始菜单窗口变化信号
     *
     * @return 返回信号，第二个和第三个int分别代表新的宽度和高度
     */
    sigc::signal<void,int,int> signal_size_changed();

    /**
     * @brief 重新加载所有的应用信息，包括收藏夹、常用应用和新安装应用
     */
    void reload_apps_data();

    /**
     * @brief 加载收藏夹应用列表
     */
    void load_favorite_apps();

    /**
     * @brief 加载常用应用列表
     */
    void load_frequent_apps();

    /**
     * @brief 加载新安装应用列表
     */
    void load_new_apps();

    /**
     * @brief 设置开始菜单当前显示模式(紧凑或扩展）
     * @param mode 新的显示模式
     */
    void set_display_mode(MenuDisplayMode mode);

    /**
     * @brief 根据设置重新设置开始菜单的显示模式
     */
    void ensure_display_mode();


    /**
     * @brief 启动搜索结果中的第一个应用，当前无搜索结果时什么都不做
     */
    void activate_search_result();

    /**
     * @brief 获取Stack中当前显示页面的索引
     * @param stack 待获取的stack
     * @return      返回当前页面的索引
     */
    int get_stack_current_index(Gtk::Stack *stack);

    /**
     * @brief 设置Stack当前页面为索引为page的页面
     * @param stack         待设置的stack
     * @param page          要设置为当前页面的页面索引
     * @param animation     是否需要动画效果
     */
    void set_stack_current_index(Gtk::Stack *stack, int page, bool animation);

protected:
    virtual bool on_map_event(GdkEventAny *any_event) override;
    virtual bool on_unmap_event(GdkEventAny *any_event) override;
    virtual bool on_leave_notify_event(GdkEventCrossing *crossing_event) override;
    virtual bool on_key_press_event(GdkEventKey *key_event) override;
    virtual bool on_button_press_event(GdkEventButton *button_event) override;
    virtual bool on_configure_event(GdkEventConfigure* configure_event) override;
    virtual void on_realize() override;
    virtual void on_unrealize() override;
    virtual void get_preferred_height_vfunc(int &min_width, int &natural_width) const override;
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;

    virtual void init_ui();


    /**
     * @brief on_active_window_changed 回调函数：系统当前活动窗口发生变化时调用
     * @param active_window   新的活动窗口
     */
    virtual void on_active_window_changed(std::shared_ptr<Kiran::Window> active_window);

    /**
     * @brief create_app_search_entry  创建并初始化应用搜索输入框
     * @return 创建的搜索框
     */
    Gtk::SearchEntry *create_app_search_entry();

    /**
     * @brief on_date_box_clicked 回调函数：点击日期时间标签时调用
     */
    virtual void on_date_box_clicked();

    /**
     * @brief on_profile_changed 回调函数：当开始菜单设置发生变化时调用
     * @param changed_key   变化的gsettings 键名
     */
    virtual void on_profile_changed(const Glib::ustring &changed_key);

    /**
     * @brief init_window_visual  设置开始菜单窗口使用的Visual
     */
    virtual void init_window_visual();

    /**
     * @brief 回调函数: 开始菜单窗口的激活状态发生变化时调用
     */
    void on_active_change();

    /**
     * @brief 回调函数: 当搜索框内容变化时调用
     */
    void on_search_change();

    /**
     * @brief 回调函数： 当搜索框中按下ESC键时调用。该函数只是返回到应用列表页面
     */
    void on_search_stop();

    /**
     * @brief 跳转到分类选择页面，并将跳转之前选择的分类标签对应的分类设置为已选择状态
     * @param button: 跳转前的分类选择标签，通常是被点击的那个
     */
    void switch_to_category_overview(const std::string &selected_category);

    /**
     * @brief 跳转到所有应用页面，如果指定了要选择的分类名称，则将所有应用页面滚动到
     *        该分类对应的分类标签处(该分类标签位于滚动窗口内部的最上方位置）
     * @param selected_category 要选择的分类名称，如果为空字符串，表示不需要滚动
     * @param animation         切换页面时是否需要动画效果
     */
    void switch_to_apps_overview(const std::string &selected_category, bool animation = true);

    /**
     * @brief 切换到应用视图，并滚动到position对应的位置
     * @param position  要滚动到的位置，该位置将传递给应用列表所在viewport的adjustment，该值小于0
     *                  时，将不进行滚动
     * @param animation 切换页面时是否需要动画效果
     */
    void switch_to_apps_overview(double position, bool animation = true);

private:
    Glib::RefPtr<Gtk::Builder> builder;

    Gtk::Box *box;
    Gtk::SearchEntry *search_entry;
    Gtk::Grid *side_box;
    Gtk::Stack *overview_stack, *appview_stack;
    Gtk::Box *all_apps_box, *new_apps_box;
    Gtk::Box *favorite_apps_box, *frequent_apps_box;
    Gtk::Grid *category_overview_box, *search_results_box;
    Gtk::Box *compact_tab_box;
    Gtk::Button *compact_apps_button;
    Gtk::Button *compact_favorites_button;

    Gdk::Rectangle geometry;                                        /*缓存的开始菜单窗口大小*/
    sigc::signal<void,int,int> m_signal_size_changed;               /*开始菜单窗口尺寸变化信号*/

    KiranUserInfo *user_info;                                       /*当前用户信息*/
    std::vector<std::string> category_names;                        /*应用分类列表*/
    std::map<std::string, MenuCategoryItem*> category_items;   /*分类名称到分类控件的映射表*/

    Gtk::StyleProperty<int> compact_min_height_property, expand_min_height_property;

    MenuProfile profile;           /*首选项*/
    MenuDisplayMode display_mode;       /*当前显示模式*/
    WorkareaMonitor *monitor;           /*屏幕变化监视器*/

    /**
     * @brief 为侧边栏添加视图切换按钮和快捷启动按钮
     */
    void add_sidebar_buttons();

    /**
     * @brief 加载系统应用列表
     */
    void load_all_apps();

    /**
     * @brief 加载当前用户信息和头像
     */
    void load_date_info();

    /**
     * @brief 加载当前用户信息
     */
    void load_user_info();


    MenuAppItem *create_app_item(std::shared_ptr<Kiran::App> app,
                                      Gtk::Orientation orient = Gtk::ORIENTATION_HORIZONTAL);
    MenuCategoryItem *create_category_item(const std::string &name,
                                                bool clickable=true);


    /**
     * @brief 创建快捷应用按钮
     * @param icon_resource: 按钮图标的资源路径
     * @param tooltip: 按钮的工具提示文本
     * @param cmdline: 按钮点击时启动应用时调用的命令行参数，如 "/bin/ls /home"
     *
     * @return 返回创建的按钮，该按钮添加到容器后会随容器一起销毁，未添加到容器时需要手动销毁
     */
    Gtk::Button* create_launcher_button(const char *icon_resource,
                                        const char *tooltip,
                                        const char *cmdline);


    /**
     * @brief 在侧边栏中添加标签按钮
     * @param icon_resource: tab标签按钮显示的图标资源路径
     * @param tooltip: 标签按钮的工具提示文本
     * @param page: 点击对应的标签按钮时，要显示的tab页索引
     *
     * @return 返回创建的按钮
     */
    Gtk::Button* create_page_button(const char *icon_resource,
                                    const char *tooltip,
                                    int page_index);

    /**
     * @brief 创建无结果的提示标签
     * @param prompt_text   提示文本
     *
     * @return 返回创建的标签
     */
    Gtk::Label* create_empty_prompt_label(const char *prompt_text);

    void init_scrollable_areas();
};

#endif // MENU_APPLET_WINDOW_H
