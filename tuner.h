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
 *        file: tuner.h   guitar tuner for jack
 *
 * ----------------------------------------------------------------------------
 */

#pragma once

#ifndef _TUNER_H_
#define _TUNER_H_

#include <gtk/gtk.h>
//#include <gtk/gtkprivate.h>

#include <jack/jack.h>

#include <string> 
#include <cmath>
#include <cstdlib>



typedef std::string (*getcmdvar)
             (int x);
typedef float (*getptvar)
             ();
typedef jack_port_t* (*getport)
             ();
typedef jack_client_t* (*getclient)
             ();
typedef void (*setptvar)
             (float x);

class TunerWidget {
 private:
    int                 desk;
    GtkAdjustment*      adjt;
    GtkAdjustment*      adj;
    GtkWidget*          window;
    GtkWidget*          tuner;
    GError*             err;
    GtkWidget*          box;
    GtkWidget*          box1;
    GtkWidget*          box2;
    GtkWidget*          hbox;
    GtkWidget*          abox;
    GtkWidget*          bbox;
    GtkWidget*          fbox;
    GtkWidget*          cbox;
    GtkWidget*          dbox;
    GtkWidget*          spinner;
    GtkWidget*          spinnert;
    GtkWidget*          selector;

    static gboolean     delete_event(GtkWidget *widget, GdkEvent *event,
                             gpointer data);
    static gboolean     ref_freq_changed(gpointer arg);
    static gboolean     threshold_changed(gpointer arg);
    static gboolean     mode_changed(gpointer arg);
    static void         destroy( GtkWidget *widget, gpointer data);
 public:
    explicit TunerWidget();
    ~TunerWidget();
    int                 g_threads;
    void*               get_tuner() { return tuner;}
    void*               get_window() { return window;}
    void                session_quit();
    void                create_window();
    void                parse_cmd();
    void                show();
    void                window_area(int* x, int* y, int* w, int* l) {
        gtk_window_get_position(GTK_WINDOW(window), x, y);
        gtk_window_get_size(GTK_WINDOW(window), w, l);
    }
    static void         signal_handler(int sig);
    static gboolean     gx_update_frequency(gpointer arg);
};
extern TunerWidget tw;

class CmdPtr {
 public:
    getcmdvar           cv;
    getport             gp;
    getclient           gc;
    getptvar            ef;
    setptvar            sf;
};
extern CmdPtr *cptr;

#endif // _TUNER_H_

