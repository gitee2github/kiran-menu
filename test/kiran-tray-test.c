/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     wangxiaoqing <wangxiaoqing@kylinos.com.cn>
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

#include "kiran-tray.h"
#include <gtk/gtk.h>

#define CALENDA_WIDTH 1
#define CALENDA_HEIGHT 40

static void tran_setup(GtkWidget *window)
{
    GdkScreen *screen;
    GdkVisual *visual;

    gtk_widget_set_app_paintable(window, TRUE);
    screen = gdk_screen_get_default();
    visual = gdk_screen_get_rgba_visual(screen);

    if (visual != NULL && gdk_screen_is_composited(screen))
    {
        gtk_widget_set_visual(window, visual);
    }
}

int main(int argc,
         char **argv)
{
    GtkWidget *window;
    GtkWidget *tray;
    int x, y;
    GdkDisplay *display;
    GdkRectangle monitor;
    GError *error = NULL;

    if (!gtk_init_with_args(&argc, &argv, NULL, NULL, NULL, &error))
    {
        fprintf(stderr, "%s", error->message);
        g_error_free(error);
        exit(1);
    }

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(GTK_WINDOW(window), "Test Calendar");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    /*
    gtk_widget_set_size_request (window, CALENDA_WIDTH, CALENDA_HEIGHT);
    gtk_window_set_resizable(GTK_WINDOW (window), FALSE);
    gtk_window_set_deletable (GTK_WINDOW (window), FALSE);
    gtk_window_set_decorated (GTK_WINDOW (window), FALSE);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (window), TRUE);
    gtk_window_stick (GTK_WINDOW (window));
    gtk_container_set_border_width (GTK_CONTAINER (window), 0);
    */
    gtk_widget_set_name(GTK_WIDGET(window), "trayWindow");

    //tran_setup (window);

    tray = kiran_tray_new();

    gtk_container_add(GTK_CONTAINER(window), tray);
    gtk_widget_show(tray);

    display = gdk_display_get_default();
    gdk_monitor_get_geometry(gdk_display_get_monitor(display, 0), &monitor);
    x = monitor.width - CALENDA_WIDTH;
    y = monitor.height - CALENDA_HEIGHT;
    //gtk_window_move (GTK_WINDOW (window), x, y);

    gtk_widget_show(window);

    gtk_main();

    return EXIT_SUCCESS;
}
