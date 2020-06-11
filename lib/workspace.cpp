/*
 * @Author       : tangjie02
 * @Date         : 2020-06-09 15:56:04
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-11 17:07:25
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/workspace.cpp
 */

#include "lib/workspace.h"

#include "window-manager.h"

namespace Kiran
{
Workspace::Workspace(WnckWorkspace *workspace) : workspace_(workspace)
{
}

Workspace::~Workspace()
{
}

int Workspace::get_number()
{
    return wnck_workspace_get_number(this->workspace_);
}

std::string Workspace::get_name()
{
    return wnck_workspace_get_name(this->workspace_);
}

void Workspace::change_name(const std::string &name)
{
    return wnck_workspace_change_name(this->workspace_, name.c_str());
}

WindowVec Workspace::get_windows()
{
    WindowVec windows;

    flush_windows();

    for (auto iter = this->windows_.begin(); iter != this->windows_.end(); ++iter)
    {
        auto window = WindowManager::get_instance()->get_window(*iter);
        if (window)
        {
            windows.push_back(window);
        }
    }
    return windows;
}

void Workspace::flush_windows()
{
    for (auto iter = this->windows_.begin(); iter != this->windows_.end(); ++iter)
    {
        auto window = WindowManager::get_instance()->get_window(*iter);

        bool is_erase = false;

        if (!window)
        {
            is_erase = true;
        }

        auto workspace = window->get_workspace();

        if (workspace && workspace->get_number() != this->get_number())
        {
            is_erase = true;
        }
        else if (!workspace && !window->is_pinned())
        {
            is_erase = true;
        }

        if (is_erase)
        {
            this->windows_.erase(iter);
        }
    }
}

void Workspace::add_window(std::shared_ptr<Window> window)
{
    this->windows_.insert(window->get_xid());
}

void Workspace::remove_window(std::shared_ptr<Window> window)
{
    this->windows_.erase(window->get_xid());
}

}  // namespace Kiran