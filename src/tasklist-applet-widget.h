#ifndef TASKLISTAPPLETWIDGET_H
#define TASKLISTAPPLETWIDGET_H

#include <gtkmm.h>
#include "tasklist-buttons-container.h"

class TasklistAppletWidget : public Gtk::Box
{
public:
    TasklistAppletWidget(MatePanelApplet *applet);
    void update_action_buttons_state();

    void on_app_buttons_page_changed();
    void on_applet_orient_changed();

protected:
    void init_ui();
    Gtk::Button *create_action_button(std::string icon_resource, std::string tooltip_text);

private:
    Gtk::Box button_box;
    Gtk::Button *prev_btn, *next_btn;
    TasklistButtonsContainer container;

    MatePanelApplet *applet;
};

#endif // TASKLISTAPPLETWIDGET_H
