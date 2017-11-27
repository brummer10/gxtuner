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
 *        file: tuner.cpp   guitar tuner for jack
 *
 * ----------------------------------------------------------------------------
 */

#include "./config.h"
#include "./tuner.h"
#include "./paintbox.h"
#include "./gtkknob.h"
#include "./gxtuner.h"
#include "./deskpager.h"
#include "./resources.h"


TunerWidget::TunerWidget() {}
TunerWidget::~TunerWidget() {}

void TunerWidget::session_quit() {
    jack_port_unregister(cptr->gc(), cptr->gp());
    jack_deactivate(cptr->gc());
    jack_client_close(cptr->gc());
    if (tw.g_threads > 0) {
        g_source_remove(tw.g_threads);
    }
    gtk_main_quit ();
}

void TunerWidget::destroy( GtkWidget *widget, gpointer data) {
    jack_port_unregister(cptr->gc(), cptr->gp());
    jack_deactivate(cptr->gc());
    jack_client_close(cptr->gc());
    if (tw.g_threads > 0) {
        g_source_remove(tw.g_threads);
    }
    gtk_main_quit ();
}

gboolean TunerWidget::delete_event(GtkWidget *widget, GdkEvent *event,
                             gpointer data) {
    return FALSE;
}

gboolean TunerWidget::gx_update_frequency(gpointer arg) {
    gx_tuner_set_freq(GX_TUNER(tw.get_tuner()),
        cptr->ef());
    return true;
}

gboolean TunerWidget::ref_freq_changed(gpointer arg) {
    gx_tuner_set_reference_pitch(GX_TUNER(tw.get_tuner()),
        gtk_adjustment_get_value(GTK_ADJUSTMENT(arg)));
    return true;
}

gboolean TunerWidget::threshold_changed(gpointer arg) {
    cptr->sf(gtk_adjustment_get_value(GTK_ADJUSTMENT(arg)));
    return true;
}

gboolean TunerWidget::mode_changed(gpointer arg) {
    int m = gtk_combo_box_get_active(GTK_COMBO_BOX(arg));
    GtkWidget *top = gtk_widget_get_toplevel(GTK_WIDGET(arg));
    std::string title ="gxtuner - ";
    title +=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(arg));
    gtk_window_set_title(GTK_WINDOW(top),title.c_str());
    gx_tuner_set_mode(GX_TUNER(tw.get_tuner()),m);
    return true;
}

void TunerWidget::signal_handler(int sig) {
    // print out a warning
    g_print ("signal: %i received, exiting ...\n", sig);
    destroy(NULL,NULL);
}

void TunerWidget::create_window() {

    // create main window and set icon to use
    err = NULL;
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    GdkPixbuf  *icon = gdk_pixbuf_new_from_resource("/gxtuner/gxtuner.png", NULL);
    gtk_window_set_icon(GTK_WINDOW(window),icon);
    if (err != NULL) g_error_free(err);
    g_object_unref(icon);

    // create all used widgets
    tuner = gx_tuner_new();
    box = gx_paint_box_new(GTK_ORIENTATION_VERTICAL,false, 0);
    box1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(box1),false);
    box2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(box2),false);
    set_expose_func(GX_PAINT_BOX(box),"rahmen_expose");
    gtk_container_set_border_width(GTK_CONTAINER(box1),15);
    abox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(abox),false);
    bbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(bbox),false);
    cbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(cbox),false);
    dbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(dbox),false);
    ebox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(ebox),false);
    fbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(fbox),false);
    gbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(gbox),false);
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(hbox),false);
    ibox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(ibox),false);
    jbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(jbox),false);
    kbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(kbox),false);
    lbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(lbox),false);
    mbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(mbox),false);
    nbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(nbox),false);
    obox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(obox),false);
        
    adj = gtk_adjustment_new(440, 200, 600, 0.1, 1.0, 0);
    spinner = gtk_knob_new_with_value_label(GTK_ADJUSTMENT(adj), 0);
    gtk_widget_set_valign(spinner, GTK_ALIGN_START);
    gtk_widget_set_margin_end(spinner, 4);
    gtk_widget_set_margin_bottom(spinner, 2);
    
    adjt = gtk_adjustment_new(0.001, 0.001, 0.2, 0.001, 1.0, 0);
    spinnert = gtk_knob_new_with_value_label(GTK_ADJUSTMENT(adjt), 1);
    gtk_widget_set_valign(spinnert, GTK_ALIGN_START);
    gtk_widget_set_margin_start(spinnert, 4);
    gtk_widget_set_margin_bottom(spinnert, 2);
    // scale
    selectord = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectord), NULL, "chromatic");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectord), NULL, "shruti");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectord), NULL, "diatonic");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectord), NULL, "johnston5limit");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectord), NULL, "johnston7limit");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectord), NULL, "johnston7limitno5");
    gtk_combo_box_set_active(GTK_COMBO_BOX(selectord), 2);
    gtk_widget_set_opacity(GTK_WIDGET(selectord), 0.1);
    // Reference note
    selectore = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectore), NULL, "C");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectore), NULL, "D");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectore), NULL, "E");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectore), NULL, "F");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectore), NULL, "G");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectore), NULL, "A");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectore), NULL, "B");
    gtk_combo_box_set_active(GTK_COMBO_BOX(selectore), 3);
    gtk_widget_set_opacity(GTK_WIDGET(selectore), 0.1);
    // Flat or Sharps
    selectorf = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorf), NULL, "♭♭♭");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorf), NULL, "♭♭");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorf), NULL, "♭");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorf), NULL, "");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorf), NULL, "♯");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorf), NULL, "♯♯");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorf), NULL, "♯♯♯");
    gtk_combo_box_set_active(GTK_COMBO_BOX(selectorf), 3);
    gtk_widget_set_opacity(GTK_WIDGET(selectorf), 0.1);
    // 5 comma
    selectorg = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorg), NULL, "---");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorg), NULL, "--");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorg), NULL, "-");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorg), NULL, "");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorg), NULL, "+");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorg), NULL, "++");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorg), NULL, "+++");
    gtk_combo_box_set_active(GTK_COMBO_BOX(selectorg), 3);
    gtk_widget_set_opacity(GTK_WIDGET(selectorg), 0.1);
    // 7 comma
    selectorh = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorh), NULL, "LLL");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorh), NULL, "LL");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorh), NULL, "L");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorh), NULL, "");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorh), NULL, "7");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorh), NULL, "77");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorh), NULL, "777");
    gtk_combo_box_set_active(GTK_COMBO_BOX(selectorh), 3);
    gtk_widget_set_opacity(GTK_WIDGET(selectorh), 0.1);
    // 11 comma
    selectori = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectori), NULL, "↓↓↓");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectori), NULL, "↓↓");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectori), NULL, "↓");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectori), NULL, "");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectori), NULL, "↑");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectori), NULL, "↑↑");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectori), NULL, "↑↑↑");
    gtk_combo_box_set_active(GTK_COMBO_BOX(selectori), 3);
    gtk_widget_set_opacity(GTK_WIDGET(selectori), 0.1);
    // 13 comma
    selectorj = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorj), NULL, "-3");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorj), NULL, "-2");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorj), NULL, "-1");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorj), NULL, "0");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorj), NULL, "1");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorj), NULL, "2");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorj), NULL, "3");
    gtk_combo_box_set_active(GTK_COMBO_BOX(selectorj), 3);
    gtk_widget_set_opacity(GTK_WIDGET(selectorj), 0.1);
    // 17 comma
    selectork = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectork), NULL, "-3");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectork), NULL, "-2");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectork), NULL, "-1");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectork), NULL, "0");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectork), NULL, "1");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectork), NULL, "2");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectork), NULL, "3");
    gtk_combo_box_set_active(GTK_COMBO_BOX(selectork), 3);
    gtk_widget_set_opacity(GTK_WIDGET(selectork), 0.1);
    // 19 comma
    selectorl = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorl), NULL, "-3");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorl), NULL, "-2");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorl), NULL, "-1");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorl), NULL, "0");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorl), NULL, "1");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorl), NULL, "2");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorl), NULL, "3");
    gtk_combo_box_set_active(GTK_COMBO_BOX(selectorl), 3);
    gtk_widget_set_opacity(GTK_WIDGET(selectorl), 0.1);
    // 23 comma
    selectorm = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorm), NULL, "-3");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorm), NULL, "-2");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorm), NULL, "-1");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorm), NULL, "0");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorm), NULL, "1");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorm), NULL, "2");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorm), NULL, "3");
    gtk_combo_box_set_active(GTK_COMBO_BOX(selectorm), 3);
    gtk_widget_set_opacity(GTK_WIDGET(selectorm), 0.1);
    // 29 comma
    selectorn = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorn), NULL, "-3");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorn), NULL, "-2");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorn), NULL, "-1");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorn), NULL, "0");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorn), NULL, "1");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorn), NULL, "2");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectorn), NULL, "3");
    gtk_combo_box_set_active(GTK_COMBO_BOX(selectorn), 3);
    gtk_widget_set_opacity(GTK_WIDGET(selectorn), 0.1);
    // 31 comma
    selectoro = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectoro), NULL, "-3");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectoro), NULL, "-2");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectoro), NULL, "-1");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectoro), NULL, "0");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectoro), NULL, "1");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectoro), NULL, "2");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(selectoro), NULL, "3");
    gtk_combo_box_set_active(GTK_COMBO_BOX(selectoro), 3);
    gtk_widget_set_opacity(GTK_WIDGET(selectoro), 0.1);
    
    // set some options to widgets
    gtk_widget_set_app_paintable(window, TRUE);
    gtk_widget_set_redraw_on_allocate(GTK_WIDGET(window), TRUE); 
    gtk_widget_set_can_focus(GTK_WIDGET(spinner), true);
    gtk_widget_set_can_focus(GTK_WIDGET(spinnert), true);
    gtk_widget_set_can_default(GTK_WIDGET(spinner), true);
    gtk_widget_set_can_default(GTK_WIDGET(spinnert), true);
    gtk_widget_set_tooltip_text(GTK_WIDGET(spinner),"reference pitch");
    gtk_widget_set_tooltip_text(GTK_WIDGET(spinnert),"threshold");
    gtk_widget_set_tooltip_text(GTK_WIDGET(selectord),"scale");
    gtk_widget_set_tooltip_text(GTK_WIDGET(selectore),"Reference note");
    gtk_widget_set_tooltip_text(GTK_WIDGET(selectorf),"Flats or Sharps");
    gtk_widget_set_tooltip_text(GTK_WIDGET(selectorg),"Syncomma");
    gtk_widget_set_tooltip_text(GTK_WIDGET(selectorh),"7 comma");
    gtk_widget_set_tooltip_text(GTK_WIDGET(selectori),"11 comma");
    gtk_widget_set_tooltip_text(GTK_WIDGET(selectorj),"13 comma");
    gtk_widget_set_tooltip_text(GTK_WIDGET(selectork),"17 comma");
    gtk_widget_set_tooltip_text(GTK_WIDGET(selectorl),"19 comma");
    gtk_widget_set_tooltip_text(GTK_WIDGET(selectorm),"23 comma");
    gtk_widget_set_tooltip_text(GTK_WIDGET(selectorn),"29 comma");
    gtk_widget_set_tooltip_text(GTK_WIDGET(selectoro),"31 comma");
    
    // stack all together
    // the main window box
    gtk_container_add (GTK_CONTAINER(window), box2);
    // the paintbox to create the frame
    gtk_box_pack_start(GTK_BOX(box2), box,true,true,0);
    // put the tuner inside the frame
    gtk_box_pack_start(GTK_BOX(box), box1,true,true,0);
    gtk_box_pack_start(GTK_BOX(box1), tuner,true,true,0);
    // add a box for the controls to the bottom
    gtk_box_pack_end(GTK_BOX(box2), hbox, false,false,0);
    // put all the selectors and spinners in a box
    gtk_container_add (GTK_CONTAINER (abox), spinner);
    gtk_container_add (GTK_CONTAINER (bbox), spinnert);
    gtk_container_add (GTK_CONTAINER (dbox), selectord);
    gtk_container_add (GTK_CONTAINER (ebox), selectore);
    gtk_container_add (GTK_CONTAINER (fbox), selectorf);
    gtk_container_add (GTK_CONTAINER (gbox), selectorg);
    gtk_container_add (GTK_CONTAINER (hhbox), selectorh);
    gtk_container_add (GTK_CONTAINER (ibox), selectori);
    gtk_container_add (GTK_CONTAINER (jbox), selectorj);
    gtk_container_add (GTK_CONTAINER (kbox), selectork);
    gtk_container_add (GTK_CONTAINER (lbox), selectorl);
    gtk_container_add (GTK_CONTAINER (mbox), selectorm);
    gtk_container_add (GTK_CONTAINER (nbox), selectorn);
    gtk_container_add (GTK_CONTAINER (obox), selectoro);
        
    //put all the filled boxes in hbox
    gtk_box_pack_start(GTK_BOX(hbox),abox,false,false,0);
    gtk_box_pack_start(GTK_BOX(hbox),bbox,false,false,0);
    gtk_box_pack_start(GTK_BOX(hbox),cbox,true,false,0);
    gtk_box_pack_start(GTK_BOX(hbox),dbox,false,false,5);
    gtk_box_pack_start(GTK_BOX(hbox),ebox,false,false,5);
    gtk_box_pack_start(GTK_BOX(hbox),fbox,false,false,5);
    gtk_box_pack_start(GTK_BOX(hbox),gbox,false,false,5);
    gtk_box_pack_start(GTK_BOX(hbox),hhbox,false,false,5);
    gtk_box_pack_start(GTK_BOX(hbox),ibox,false,false,5);
    gtk_box_pack_start(GTK_BOX(hbox),jbox,false,false,5);
    gtk_box_pack_start(GTK_BOX(hbox),kbox,false,false,5);
    gtk_box_pack_start(GTK_BOX(hbox),lbox,false,false,5);
    gtk_box_pack_start(GTK_BOX(hbox),mbox,false,false,5);
    gtk_box_pack_start(GTK_BOX(hbox),nbox,false,false,5);
    gtk_box_pack_start(GTK_BOX(hbox),obox,false,false,5);
        
    // connect the signal handlers 
    // connect the controls with a function to do what they should do
    // when they changed there value.
    g_signal_connect(G_OBJECT(adj), "value-changed",
        G_CALLBACK(ref_freq_changed),(gpointer)adj);
    g_signal_connect(G_OBJECT(adjt), "value-changed",
        G_CALLBACK(threshold_changed),(gpointer)adjt);
    g_signal_connect(GTK_COMBO_BOX(selectord), "changed",
        G_CALLBACK(mode_changed),(gpointer)selectord);
    g_signal_connect (window, "delete-event",
            G_CALLBACK (delete_event), NULL);
    g_signal_connect (window, "destroy",
            G_CALLBACK (destroy), NULL);
    
    parse_cmd();
    show();
}

void TunerWidget::parse_cmd() {
    GtkCssProvider* provider = gtk_css_provider_new();
    GdkDisplay * display = gdk_display_get_default ();
    GdkScreen * sc = gdk_display_get_default_screen (display);
    GdkWindow * wd = gdk_screen_get_root_window (sc);

    gtk_style_context_add_provider_for_screen(sc, GTK_STYLE_PROVIDER(provider),
                                      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(provider),
                "  * {  \n"
                " color:  #fff; \n"
                " background-color: black;  \n"
                "} \n", -1, NULL);

    g_object_unref(provider);
   
    // *** commandline parsing ***
    // set default window position optional by command line options
    int x = 120;
    int y = 90;
    if (!cptr->cv(3).empty()) {
        x = atoi(cptr->cv(3).c_str());
    } else {
        x = gdk_window_get_width(wd)/2 -x;
    }
    if (!cptr->cv(4).empty()) {
        y = atoi(cptr->cv(4).c_str());
    } else {
        y = gdk_window_get_height(wd)/2 -y;
    }
    gtk_window_move(GTK_WINDOW(window),x,y);
    // set default window size optional by command line options
    x = 1000;
    y = 500;
    if (!cptr->cv(1).empty()) {
        x = atoi(cptr->cv(1).c_str());
    }
    if (!cptr->cv(2).empty()) {
        y = atoi(cptr->cv(2).c_str());
    }
    gtk_window_resize(GTK_WINDOW(window), x,y);
    // set reference pitch and threshold by command line options
    double p,t;
    if (!cptr->cv(5).empty()) {
        p = atof(cptr->cv(5).c_str());
    } else {
        p = 440.0;
    }
    if (!cptr->cv(6).empty()) {
        t = atof(cptr->cv(6).c_str());
    } else {
        t = 0.001;
    }
    gtk_window_set_title(GTK_WINDOW(window),"gxtuner-chromatic");
    // here we check if a special mode is given on commandline, 
    // add your mode here as well.
    if (!cptr->cv(9).empty()) {
        std::string m = cptr->cv(9).c_str();
        if(m == "shruti") {
            gtk_combo_box_set_active(GTK_COMBO_BOX(selectord), 1);
        } else if(m == "diatonic") {
            gtk_combo_box_set_active(GTK_COMBO_BOX(selectord), 2);
        } else if(m == "johnston5limit") {
            gtk_combo_box_set_active(GTK_COMBO_BOX(selectord), 3);
        } else if(m == "johnston7limit") {
            gtk_combo_box_set_active(GTK_COMBO_BOX(selectord), 4);
        } else if(m == "johnston7limitno5") {
           gtk_combo_box_set_active(GTK_COMBO_BOX(selectord), 5);
         }
    }
    gtk_adjustment_set_value(GTK_ADJUSTMENT(adj),p);
    gtk_adjustment_set_value(GTK_ADJUSTMENT(adjt),t);
    // set virtual desktop to use
    desk = 0;
    if (!cptr->cv(8).empty()) {
        desk = atoi(cptr->cv(8).c_str());
    } 
}

void TunerWidget::show() {
    // finaly show the window with all widgets
    gtk_widget_show_all(window);
    if (desk) dp.move_window_to_desktop(desk, window);
}

TunerWidget tw;
CmdPtr *cptr = 0;

