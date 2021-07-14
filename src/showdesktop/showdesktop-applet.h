/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
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
#ifndef __SHOWDESKTOP_APPLET_H__
#define __SHOWDESKTOP_APPLET_H__

#include <mate-panel-applet.h>
#include <gtkmm.h>

G_BEGIN_DECLS

gboolean showdesktop_applet_fill(MatePanelApplet *applet);

G_END_DECLS


#endif // __SHOWDESKTOP_APPLET_H__