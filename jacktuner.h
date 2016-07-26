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
 *        file: jacktuner.h      guitar tuner for jack
 *
 * ----------------------------------------------------------------------------
 */

#pragma once

#ifndef JACK_TUNER_H_
#define JACK_TUNER_H_

#include "./config.h"
#include <jack/jack.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <string.h> 

#ifdef HAVE_JACK_SESSION
#include <jack/session.h>
#endif

#include <string> 
#include <cstdlib>
    
typedef void (*funcpointer)
             (int* x, int* y, int* w, int* l);
typedef void (*getpointer)
             (double* x);
typedef void (*getintpointer)
             (int* x);
typedef void (*npointer)
             ();
typedef void (*gettracker)
             (int x, float *input);

class JackTuner {
 private:
    jack_status_t       jackstat;
    std::string         client_name;
    static void         jack_shutdown (void *arg);
    static int          gx_jack_process(jack_nframes_t nframes, void *arg);
#ifdef HAVE_JACK_SESSION
    static void         gx_jack_session_callback(jack_session_event_t *event, void *arg);
    static int          gx_jack_session_callback_helper(void* arg);
#endif
 public:
    explicit JackTuner();
    ~JackTuner();
    jack_port_t*        input_port;
    jack_client_t*      client;
    jack_nframes_t      jack_sr;   // jack sample rate
    jack_nframes_t      jack_bs;   // jack buffer size
    void                gx_jack_activate(std::string jack_uuid, std::string jack_in);
    bool                gx_jack_init(std::string jack_uuid);
    

};
extern JackTuner jt;

class FuncPtr {
 public:
    funcpointer         widi;
    getpointer          rp;
    getpointer          gt;
    getintpointer       desk;
    npointer            ex;
    npointer            qt;
    gettracker          pt;
};
extern FuncPtr *fptr;


#endif // JACK_TUNER_H_
