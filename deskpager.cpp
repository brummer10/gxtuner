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
 *        file: deskpager.cpp   set and get the virtual desktop for a gtkwindow
 *
 * ----------------------------------------------------------------------------
 */

#include "./deskpager.h"

DeskPager::DeskPager() {}
DeskPager::~DeskPager() {}

guint DeskPager::get_all_desktops () {
    data = NULL;

    if (!gdk_property_get ( gdk_screen_get_root_window (gdk_screen_get_default ()),
            gdk_atom_intern ("_NET_NUMBER_OF_DESKTOPS", FALSE),
            gdk_atom_intern ("CARDINAL", FALSE), 0, G_MAXLONG, FALSE,
            &actual_property_type, &actual_format, &actual_length, reinterpret_cast<guchar **>(&data))) {
        gchar *actual_property_type_name;
        g_critical ("Unable to get _NET_NUMBER_OF_DESKTOPS");
        actual_property_type_name = gdk_atom_name (actual_property_type);
        if (actual_property_type_name) {
            g_message ("actual_property_type: %s", actual_property_type_name);
            g_free (actual_property_type_name);
        }
        return -1;
    }
    desktops_count = static_cast<guint>(data[0]);
    g_free (data);
    return desktops_count;
}

void DeskPager::move_window_to_desktop(guint desktop_num, GtkWidget * window) {

    if (desktop_num+1 > get_all_desktops()) return;

    display = gdk_x11_get_default_xdisplay();
    root_win = GDK_WINDOW_XID(gdk_get_default_root_window());
    displays = gdk_window_get_display (gtk_widget_get_window(window));

    xevent.type              = ClientMessage;
    xevent.xclient.type      = ClientMessage; 
    xevent.xclient.serial = 0;
    xevent.xclient.send_event = True;
    xevent.xclient.display   = display;
    xevent.xclient.window    = GDK_WINDOW_XID (gtk_widget_get_window(window)); 
    xevent.xclient.message_type = gdk_x11_get_xatom_by_name_for_display (displays, "_NET_WM_DESKTOP"); //atom_net_current_desktop;
    xevent.xclient.format    = 32;
    xevent.xclient.data.l[0] = desktop_num;
    xevent.xclient.data.l[1] = CurrentTime;
    xevent.xclient.data.l[2] = 0;
    xevent.xclient.data.l[3] = 0;
    xevent.xclient.data.l[4] = 0;

    XSendEvent(display, root_win, False, SubstructureNotifyMask | SubstructureRedirectMask, &xevent);

    XFlush(display);
}

gint DeskPager::get_active_desktop_for_window (GtkWidget * window) {
    data = NULL;

    if (!gdk_property_get (gtk_widget_get_window(window), gdk_atom_intern ("_NET_WM_DESKTOP", FALSE),
            gdk_atom_intern ("CARDINAL", FALSE), 0, G_MAXLONG, FALSE, &actual_property_type,
            &actual_format, &actual_length, reinterpret_cast<guchar **>(&data))) {
        gchar *actual_property_type_name;
        g_critical ("Unable to get _NET_WM_DESKTOP");
        actual_property_type_name = gdk_atom_name (actual_property_type);
        if (actual_property_type_name) {
            g_message ("actual_property_type: %s", actual_property_type_name);
            g_free (actual_property_type_name);
        }
        return -1;
    }
    num_desktop = static_cast<guint>(data[0]);
    g_free (data);
    return num_desktop;
}

DeskPager dp;
