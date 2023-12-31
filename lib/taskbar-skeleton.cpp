/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
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

#include "lib/taskbar-skeleton.h"

#include "lib/base.h"
#include "lib/common.h"

namespace Kiran
{
TaskBarSkeleton::TaskBarSkeleton(AppManager *app_manager) : app_manager_(app_manager)
{
    this->settings_ = Gio::Settings::create(KIRAN_TASKBAR_SCHEMA);
    this->fixed_apps_ = read_as_to_list_quark(this->settings_, TASKBAR_KEY_FIXED_APPS);

    settings_->signal_changed().connect_notify(
        [this](const Glib::ustring &key) -> void {
            if (key == TASKBAR_KEY_SHOW_ACTIVE_WORKSPACE)
                signal_app_show_policy_changed().emit();
        });
}

TaskBarSkeleton::~TaskBarSkeleton()
{
}

TaskBarSkeleton *TaskBarSkeleton::instance_ = nullptr;
void TaskBarSkeleton::global_init(AppManager *app_manager)
{
    instance_ = new TaskBarSkeleton(app_manager);
    instance_->init();
}

void TaskBarSkeleton::init()
{
    this->desktop_app_changed();

    this->app_manager_->signal_desktop_app_changed().connect(sigc::mem_fun(this, &TaskBarSkeleton::desktop_app_changed));

    this->settings_->signal_changed(TASKBAR_KEY_FIXED_APPS).connect(sigc::mem_fun(this, &TaskBarSkeleton::app_changed));
}

bool TaskBarSkeleton::add_fixed_app(const std::string &desktop_id)
{
    KLOG_PROFILE("id: %s.", desktop_id.c_str());

    Glib::Quark quark(desktop_id);

    auto iter = std::find(this->fixed_apps_.begin(), this->fixed_apps_.end(), quark.id());
    if (iter == this->fixed_apps_.end())
    {
        auto app = this->app_manager_->lookup_app(desktop_id);
        if (app && (app->get_kind() == AppKind::USER_TASKBAR || app->get_kind() == AppKind::NORMAL))
        {
            this->fixed_apps_.push_back(quark.id());

            AppVec add_apps = {app};
            this->fixed_app_added_.emit(add_apps);
            return write_list_quark_to_as(this->settings_, TASKBAR_KEY_FIXED_APPS, this->fixed_apps_);
        }
        else
        {
            KLOG_DEBUG("desktop id '%s' not found or invalid.", desktop_id.c_str());
        }
    }
    return false;
}

bool TaskBarSkeleton::del_fixed_app(const std::string &desktop_id)
{
    KLOG_PROFILE("id: %s.", desktop_id.c_str());

    Glib::Quark quark(desktop_id);

    auto iter = std::find(this->fixed_apps_.begin(), this->fixed_apps_.end(), quark.id());

    if (iter != this->fixed_apps_.end())
    {
        auto app = this->app_manager_->lookup_app(desktop_id);
        if (app && (app->get_kind() == AppKind::NORMAL || app->get_kind() == AppKind::USER_TASKBAR))
        {
            this->fixed_apps_.erase(iter);

            AppVec delete_apps = {app};
            this->fixed_app_deleted_.emit(delete_apps);
            return write_list_quark_to_as(this->settings_, TASKBAR_KEY_FIXED_APPS, this->fixed_apps_);
        }
        else
        {
            KLOG_DEBUG("desktop id '%s' not found or invalid.", desktop_id.c_str());
        }
    }
    return false;
}

std::shared_ptr<App> TaskBarSkeleton::lookup_fixed_app(const std::string &desktop_id)
{
    Glib::Quark quark(desktop_id);

    auto iter = std::find(this->fixed_apps_.begin(), this->fixed_apps_.end(), quark.id());

    if (iter != this->fixed_apps_.end())
    {
        return app_manager_->lookup_app(desktop_id);
    }
    return nullptr;
}

AppVec TaskBarSkeleton::get_fixed_apps()
{
    AppVec apps;
    for (auto iter = this->fixed_apps_.begin(); iter != this->fixed_apps_.end(); ++iter)
    {
        Glib::QueryQuark query_quark((GQuark)*iter);
        Glib::ustring desktop_id = query_quark;
        auto app = this->app_manager_->lookup_app(desktop_id.raw());
        if (app)
        {
            apps.push_back(app);
        }
    }
    return apps;
}

TaskBarSkeleton::AppShowPolicy TaskBarSkeleton::get_app_show_policy()
{
    bool value = settings_->get_boolean(TASKBAR_KEY_SHOW_ACTIVE_WORKSPACE);

    if (value)
        return TaskBarSkeleton::POLICY_SHOW_ACTIVE_WORKSPACE;
    return TaskBarSkeleton::POLICY_SHOW_ALL;
}

void TaskBarSkeleton::desktop_app_changed()
{
    KLOG_PROFILE("");
    auto apps = this->app_manager_->get_apps_by_kind((AppKind)(AppKind::USER_TASKBAR | AppKind::NORMAL));

    std::set<int32_t> app_set;

    for (int i = 0; i < (int)apps.size(); ++i)
    {
        auto &app = apps[i];
        auto &desktop_id = app->get_desktop_id();
        Glib::Quark quark(desktop_id);
        app_set.insert(quark.id());
    }

    AppVec delete_apps;

    auto iter = std::remove_if(this->fixed_apps_.begin(), this->fixed_apps_.end(), [this, &delete_apps, &app_set](int32_t elem) -> bool {
        if (app_set.find(elem) == app_set.end())
        {
            Glib::QueryQuark query_quark((GQuark)elem);
            Glib::ustring desktop_id = query_quark;
            auto app = this->app_manager_->lookup_app(desktop_id.raw());
            if (app)
            {
                delete_apps.push_back(app);
            }
            return true;
        }
        return false;
    });

    if (iter != this->fixed_apps_.end())
    {
        this->fixed_apps_.erase(iter, this->fixed_apps_.end());
        write_list_quark_to_as(this->settings_, TASKBAR_KEY_FIXED_APPS, this->fixed_apps_);
        this->fixed_app_deleted_.emit(delete_apps);
    }
}

void TaskBarSkeleton::app_changed(const Glib::ustring &key)
{
    KLOG_PROFILE("key: %s.", key.c_str());

    auto new_fixed_apps = read_as_to_list_quark(this->settings_, TASKBAR_KEY_FIXED_APPS);

    AppVec add_apps;
    AppVec delete_apps;

    for (auto iter = this->fixed_apps_.begin(); iter != this->fixed_apps_.end(); ++iter)
    {
        auto value = *iter;
        if (std::find(new_fixed_apps.begin(), new_fixed_apps.end(), value) == new_fixed_apps.end())
        {
            Glib::QueryQuark query_quark((GQuark)value);
            Glib::ustring desktop_id = query_quark;
            auto app = this->app_manager_->lookup_app(desktop_id.raw());
            if (app)
            {
                delete_apps.push_back(app);
            }
        }
    }

    for (auto iter = new_fixed_apps.begin(); iter != new_fixed_apps.end(); ++iter)
    {
        auto value = *iter;
        if (std::find(this->fixed_apps_.begin(), this->fixed_apps_.end(), value) == this->fixed_apps_.end())
        {
            Glib::QueryQuark query_quark((GQuark)value);
            Glib::ustring desktop_id = query_quark;
            auto app = this->app_manager_->lookup_app(desktop_id.raw());
            if (app)
            {
                add_apps.push_back(app);
            }
        }
    }

    this->fixed_apps_ = new_fixed_apps;

    if (delete_apps.size() > 0)
    {
        this->fixed_app_deleted_.emit(delete_apps);
    }

    if (add_apps.size() > 0)
    {
        this->fixed_app_added_.emit(add_apps);
    }
}
}  // namespace Kiran
