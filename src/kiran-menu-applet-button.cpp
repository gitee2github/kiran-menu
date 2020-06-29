#include "kiran-menu-applet-button.h"
#include <glibmm/i18n.h>

#define BUTTON_MARGIN 6

KiranMenuAppletButton::KiranMenuAppletButton(MatePanelApplet *panel_applet)
{
    applet = MATE_PANEL_APPLET(g_object_ref(panel_applet));
    icon_size = 16;
    set_tooltip_text(_("Kiran Start Menu"));

    update_icon();
    /**
     * 当窗口隐藏时更新插件按钮状态
    */
    connection1 = window.signal_hide().connect(sigc::bind<bool>(sigc::mem_fun(*this, &Gtk::ToggleButton::set_active), false));
}

KiranMenuAppletButton::~KiranMenuAppletButton()
{
    if (connection2.connected())
            connection2.disconnect();
    connection1.disconnect();
    g_object_unref(applet);
}

void KiranMenuAppletButton::get_preferred_width_vfunc(int &minimum_width, int &natural_width) const
{
    int size = mate_panel_applet_get_size(applet);
    MatePanelAppletOrient orient = mate_panel_applet_get_orient(applet);

    if (orient == MATE_PANEL_APPLET_ORIENT_DOWN || orient == MATE_PANEL_APPLET_ORIENT_UP)
        minimum_width = natural_width = size + 2 * BUTTON_MARGIN;
    else
        minimum_width = natural_width = size;
}

void KiranMenuAppletButton::get_preferred_height_vfunc(int &minimum_height, int &natural_height) const
{
    int size = mate_panel_applet_get_size(applet);
    MatePanelAppletOrient orient = mate_panel_applet_get_orient(applet);

    if (orient == MATE_PANEL_APPLET_ORIENT_DOWN || orient == MATE_PANEL_APPLET_ORIENT_UP)
        minimum_height = natural_height = size;
    else
        minimum_height = natural_height = size + 2 * BUTTON_MARGIN;
}

void KiranMenuAppletButton::on_size_allocate(Gtk::Allocation &allocation)
{
    MatePanelAppletOrient orient;

    orient = mate_panel_applet_get_orient(applet);
    if (orient == MATE_PANEL_APPLET_ORIENT_UP ||
        orient == MATE_PANEL_APPLET_ORIENT_DOWN)
    {
        icon_size = allocation.get_height() - 2 * BUTTON_MARGIN;
    }
    else
        icon_size = allocation.get_width() - 2 * BUTTON_MARGIN;

    Gtk::ToggleButton::on_size_allocate(allocation);
}

bool KiranMenuAppletButton::on_draw(const::Cairo::RefPtr<Cairo::Context> &cr)
{
    Gtk::Allocation allocation;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;
    auto context = get_style_context();

    pixbuf = icon_pixbuf->scale_simple(icon_size, icon_size, Gdk::INTERP_BILINEAR);
    allocation = get_allocation();

    context->set_state(get_state_flags());
    context->render_background(cr, 0, 0, allocation.get_width(), allocation.get_height());

    //将图标绘制在按钮的正中心位置
    Gdk::Cairo::set_source_pixbuf(cr, pixbuf,
                                  (allocation.get_width() - pixbuf->get_width())/2.0,
                                  (allocation.get_height() - pixbuf->get_height())/2.0);
    cr->paint();
    return false;
}

void KiranMenuAppletButton::on_toggled()
{
    if (get_active()) {
        MatePanelAppletOrient orient;
        int root_x, root_y;
        Gtk::Allocation button_allocation, window_allocation;

        window.show();

        button_allocation = get_allocation();
        window_allocation = window.get_allocation();

        //获取按钮的位置坐标
        get_window()->get_origin(root_x, root_y);
        orient = mate_panel_applet_get_orient(applet);
        switch (orient)
        {
        case MATE_PANEL_APPLET_ORIENT_UP:
            root_y -= window_allocation.get_height();
            break;
        case MATE_PANEL_APPLET_ORIENT_DOWN:
            root_y += button_allocation.get_height();
            break;
        case MATE_PANEL_APPLET_ORIENT_LEFT:
            root_x -= window_allocation.get_width();
            break;
        case MATE_PANEL_APPLET_ORIENT_RIGHT:
            root_x += button_allocation.get_width();
            break;
        default:
            g_error("invalid applet orientation: %d\n", orient);
            break;
        }

        window.move(root_x, root_y);
    } else
        window.hide();
}

void KiranMenuAppletButton::update_icon()
{
    auto icon_theme = Gtk::IconTheme::get_default();

    if (icon_pixbuf)
        icon_pixbuf.reset();

    icon_pixbuf = icon_theme->load_icon("start-here", 96, get_scale_factor(), Gtk::ICON_LOOKUP_FORCE_SIZE);
    if (!connection2.connected())
        connection2 = icon_theme->signal_changed().connect(sigc::mem_fun(*this, &KiranMenuAppletButton::update_icon));

    if (get_realized())
        queue_resize();        //resize之后会redraw的
}
