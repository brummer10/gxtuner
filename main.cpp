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
 *        file: main.cpp      guitar tuner for jack
 *
 * ----------------------------------------------------------------------------
 */

#include "./cmdparser.h"
#include "./pitchtracker.h"
#include "./gxtuner.h"
#include "./jacktuner.h"
#include "./tuner.h"
#include "./deskpager.h"



static void wrap_window_area(int* x, int* y, int* w, int* l) {
    tw.window_area(x, y, w, l);
}

static void wrap_get_reference_pitch(double* x) {
   *x = gx_tuner_get_reference_pitch(GX_TUNER(tw.get_tuner()));
}

static void wrap_get_threshold(double* x) {
    *x = pitch_tracker.get_threshold();
}

static void wrap_session_quit() {
    tw.session_quit();
}

static void wrap_pitch_tracker_add(int x, float* input) {
    pitch_tracker.add(x, input);
}

static void wrap_main_quit() {
    gtk_main_quit ();
}

static std::string wrap_get_optvar(int x) {
   return cmd.get_optvar(x);
}

static jack_port_t* wrap_input_port() {
    return jt.input_port;
}

static jack_client_t* wrap_client() {
    return jt.client;
}

static float wrap_estimated_freq() {
    return pitch_tracker.estimated_freq;
}

static void wrap_set_threshold(float x) {
    pitch_tracker.set_threshold(x);
}

static void wrap_get_desk(int *x) {
    *x = dp.get_active_desktop_for_window(GTK_WIDGET(tw.get_window()));
}

static void set_pointers_to_f() {
    // set function pointers to wrap functions
    fptr->widi      = &wrap_window_area;
    fptr->rp        = &wrap_get_reference_pitch;
    fptr->gt        = &wrap_get_threshold;
    fptr->ex        = &wrap_session_quit;
    fptr->pt        = &wrap_pitch_tracker_add;
    fptr->qt        = &wrap_main_quit;
    fptr->desk      = &wrap_get_desk;
    
    cptr->cv        = &wrap_get_optvar;
    cptr->gp        = &wrap_input_port;
    cptr->gc        = &wrap_client;
    cptr->ef        = &wrap_estimated_freq;
    cptr->sf        = &wrap_set_threshold;
}

int main(int argc, char *argv[]) {

    // trap signals to quit clean
    signal(SIGTERM, tw.signal_handler);
    signal(SIGHUP,  tw.signal_handler);
    signal(SIGINT,  tw.signal_handler);
    signal(SIGQUIT, tw.signal_handler);
    // init thread system
    tw.g_threads    = 0;
    
    // process comandline options
    cmd.process_cmdline_options(argc, argv);
    // set pointers to function pointer classes
    fptr            = new FuncPtr;
    cptr            = new CmdPtr;
    set_pointers_to_f();
    // init jack
    jt.gx_jack_init(cptr->cv(JACK_UUID));
    // init gtk
    gtk_init (&argc, &argv);
    // activate jack
    jt.gx_jack_activate(cptr->cv(JACK_UUID), cptr->cv(JACK_INP));
    // start pitchtracker
    pitch_tracker.init(static_cast<int>(jt.jack_sr),
                                jack_client_thread_id(cptr->gc()));
    // create window
    tw.create_window();
    tw.parse_cmd();
    tw.show();
    // start thread to update the frequency
    tw.g_threads    = g_timeout_add(
        100, tw.gx_update_frequency, 0);
    // run main programm
    gtk_main ();
    // stop pitch tracker thread
    pitch_tracker.stop_thread();
    // delete function pointer class pointer
    delete fptr;
    delete cptr;
    //fprintf (stderr,"gxtuner, return 0 ...\n");
    return 0;
}

