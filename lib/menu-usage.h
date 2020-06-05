/*
 * @Author       : tangjie02
 * @Date         : 2020-04-09 19:44:16
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-04 20:20:39
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/menu-usage.h
 */
#pragma once

#include "lib/menu-unit.h"

typedef struct _WnckWindow WnckWindow;
typedef struct _WnckScreen WnckScreen;

namespace Kiran
{
class MenuUsage : public MenuUnit
{
    struct UsageData
    {
        UsageData(double s = 0, int32_t l = 0)
        {
            score = s;
            last_seen = l;
        }
        double score;
        int32_t last_seen;
    };

   public:
    MenuUsage();
    virtual ~MenuUsage();

    virtual void flush(const AppVec &apps);

    void active_window_changed(WnckScreen *screen, WnckWindow *previously_active_window);

    std::vector<std::string> get_nfrequent_apps(gint top_n);

    void reset();

    //signal accessor:
    sigc::signal<void()> &signal_app_changed() { return this->app_changed_; }

   private:
    void on_session_status_changed(uint32_t status);
    void session_proxy_signal(const Glib::ustring &sender_name, const Glib::ustring &signal_name, const Glib::VariantContainerBase &parameters);
    long get_system_time(void);
    MenuUsage::UsageData &get_usage_for_app(const std::string &desktop_id);
    bool write_usages_to_settings();
    bool read_usages_from_settings();
    void ensure_queued_save();
    void increment_usage_for_app_at_time(const std::string &desktop_id, int32_t time);

   protected:
    sigc::signal<void()> app_changed_;

   private:
    Glib::RefPtr<Gio::Settings> settings;

    std::map<uint32_t, UsageData> app_usages;

    int32_t watch_start_time;
    std::string focus_desktop_id;

    sigc::connection save_id;

    Glib::RefPtr<Gio::DBus::Proxy> session_proxy;
    bool screen_idle;
};

}  // namespace Kiran
