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
 *        file: jacktuner.cpp      guitar tuner for jack
 *
 * ----------------------------------------------------------------------------
 */

#include "./jacktuner.h"

JackTuner::JackTuner() {}
JackTuner::~JackTuner() {}

bool JackTuner::gx_jack_init(std::string jack_uuid) {
    client_name =   "gxtuner";
    input_port  =   0;
#ifdef HAVE_JACK_SESSION
    if (!jack_uuid.empty()) {
        client = jack_client_open (client_name.c_str(), jack_options_t(
                 JackNoStartServer | JackSessionID), &jackstat,
                 jack_uuid.c_str());
    } else {
        client = jack_client_open(client_name.c_str(), JackNoStartServer, &jackstat);
    }
#else
    client = jack_client_open(client_name.c_str(), JackNoStartServer, &jackstat);
#endif
    if (client) {
        jack_sr = jack_get_sample_rate(client); // jack sample rate
        jack_bs = jack_get_buffer_size(client); // jack buffer size
        jack_set_process_callback(client, gx_jack_process, 0); // compute
        jack_on_shutdown (client, jack_shutdown, 0);  // shutdown clean up
#ifdef HAVE_JACK_SESSION
        if (jack_set_session_callback) {
            jack_set_session_callback(client, gx_jack_session_callback, 0);
        }
#endif
        input_port = jack_port_register(client, "in_0", JACK_DEFAULT_AUDIO_TYPE,
                     JackPortIsInput|JackPortIsTerminal, 0);
    } else {
        fprintf (stderr, "connection to jack failed, . . exit\n");
        exit(1);
    }

    return true;
}

void JackTuner::gx_jack_activate(std::string jack_uuid, std::string jack_in) {
    if (jack_activate (client)) {
        fprintf (stderr, "cannot activate client\n");
        exit (1);
    } else {
#ifdef HAVE_JACK_SESSION
        if (!jack_in.empty() && jack_uuid.empty()) {
            jack_connect(client, jack_in.c_str(),
                         jack_port_name(input_port));
        }
#else
        if (!jack_in.empty()) {
            jack_connect(client, jack_in.c_str(),
                         jack_port_name(input_port));
        }
#endif
    }
}

void JackTuner::jack_shutdown (void *arg) {fptr->qt();}

int JackTuner::gx_jack_process(jack_nframes_t nframes, void *arg) {
    float *input = static_cast<float *>
                       (jack_port_get_buffer(jt.input_port, nframes));
    fptr->pt(nframes, input);
    return 0;
}

#ifdef HAVE_JACK_SESSION
int JackTuner::gx_jack_session_callback_helper(void* arg) {
    jack_session_event_t *event = static_cast<jack_session_event_t *>(arg);
    int x = 0,y = 0,w = 0,l = 0;
    fptr->widi(&x, &y,&w,&l);
    double p = 0.0,t = 0.0;
    int d = 0;
    fptr->rp(&p);
    fptr->gt(&t);
    fptr->desk(&d);
    std::string cmd("gxtuner -U ");
    cmd += event->client_uuid;
    char buffer [100];
    sprintf (buffer, " -x %i -y %i -w %i -l %i -p %f -t %f -d %i",x, y, w, l, p, t, d);
    cmd += buffer;
    event->command_line = strdup(cmd.c_str());

    jack_session_reply(jt.client, event);

    if (event->type == JackSessionSaveAndQuit) {
        fptr->ex();
    }
    jack_session_event_free(event);
    return 0;
}

void JackTuner::gx_jack_session_callback(jack_session_event_t *event, void *arg) {
    gtk_idle_add(jt.gx_jack_session_callback_helper, static_cast<void *>(event));
}
#endif

FuncPtr *fptr = 0;
JackTuner jt;

