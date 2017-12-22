/*
 * Copyright (C) 2017 Hermann Meyer, Andreas Degert, Hans Bezemer
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

// the tuner widget class, add all functions and widget pointers 
// used in the tuner class here.

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
    GtkWidget*          abox;  
    GtkWidget*          bbox; 
    GtkWidget*          cbox; 
    GtkWidget*          dbox; 
    GtkWidget*          ebox;
    GtkWidget*          fbox;
    GtkWidget*          gbox;
    GtkWidget*          hbox;
    GtkWidget*          habox;
    GtkWidget*          hbbox;
    GtkWidget*          hcbox;
    GtkWidget*          hhbox;
    GtkWidget*          ibox;
    GtkWidget*          jbox;
    GtkWidget*          kbox;
    GtkWidget*          lbox;
    GtkWidget*          mbox;
    GtkWidget*          nbox;
    GtkWidget*          obox;
    GtkWidget*          pbox;
    GtkWidget*          pabox;
    GtkWidget*          pbbox;
    GtkWidget*          pcbox;
    GtkWidget*          qbox;
    GtkWidget*          spinner;
    GtkWidget*          spinnert;
    GtkWidget*          selectord; //changes mode
    GtkWidget*          selectore; // changes reference note
    GtkWidget*          selectorf; // 03comma
    GtkWidget*          selectorg; // 05comma
    GtkWidget*          selectorh; // 07comma 
    GtkWidget*          selectori; // 11comma
    GtkWidget*          selectorj; // 13comma
    GtkWidget*          selectork; // 17comma
    GtkWidget*          selectorl; // 19comma
    GtkWidget*          selectorm; // 23comma
    GtkWidget*          selectorn; // 29comma
    GtkWidget*          selectoro; // 31comma
    GtkWidget*          selectorq; // doremi box, skipped p because this was already taken
    
    static gboolean     delete_event(GtkWidget *widget, GdkEvent *event,
                             gpointer data);
    static gboolean     ref_freq_changed(gpointer arg);
    static gboolean     threshold_changed(gpointer arg);
    static gboolean     mode_changed(gpointer arg);
    static gboolean     doremi_changed(gpointer arg); //#1
    static gboolean     reference_note_changed(gpointer arg);
    static gboolean     reference_03comma_changed(gpointer arg);
    static gboolean     reference_05comma_changed(gpointer arg);
    static gboolean     reference_07comma_changed(gpointer arg);
    static gboolean     reference_11comma_changed(gpointer arg);
    static gboolean     reference_13comma_changed(gpointer arg);
    static gboolean     reference_17comma_changed(gpointer arg);
    static gboolean     reference_19comma_changed(gpointer arg);
    static gboolean     reference_23comma_changed(gpointer arg);
    static gboolean     reference_29comma_changed(gpointer arg);
    static gboolean     reference_31comma_changed(gpointer arg);
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