/*
 * Copyright (C) 2011 Hermann Meyer, Andreas Degert
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * ---------------------------------------------------------------------------
 *
 *        file: deskpager.h   set and get the virtual desktop for a gtkwindow
 *
 * ----------------------------------------------------------------------------
 */

#pragma once

#ifndef _DESKPAGER_H_
#define _DESKPAGER_H_

#include <gtk/gtk.h>
#include <gdk/gdkx.h>


class DeskPager {
 private:
    GdkAtom         actual_property_type;
    int             actual_format;
    int             actual_length;
    gint            desktops_count;
    gint            num_desktop;
    Display         *display;
    Window          root_win;
    GdkDisplay      *displays;
    XEvent          xevent;
    long            *data;
    guint           get_all_desktops ();
 public:
    explicit DeskPager();
    ~DeskPager();
    void            move_window_to_desktop(guint desktop_num,
                                           GtkWidget * window);
    gint            get_active_desktop_for_window (GtkWidget * window);
};
extern DeskPager dp;


#endif // _DESKPAGER_H_
