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
 *        file: comparse.cpp      guitar tuner for jack
 *
 * ----------------------------------------------------------------------------
 */


#include "./cmdparser.h"
#include "./config.h"

CmdParse::CmdParse() {}
CmdParse::~CmdParse() {}

void CmdParse::init() {
    error           = NULL;
    infostring      = "\n        version ";
    infostring      += VERSION;
    infostring      += "\n    A tuner for jack, with an option for Just Intonation\n"; 
    infostring      += "    with full jack session management support";
    opt_context     = g_option_context_new(infostring.c_str());
    jack_uuid       = NULL;
    jack_input      = NULL;
    size_x          = NULL;
    size_y          = NULL;
    pos_x           = NULL;
    pos_y           = NULL;
    desktop         = NULL;
    pitch           = NULL;
    threshold       = NULL;
    mode            = NULL;
    doremi          = NULL; //#1
    reference_note  = NULL; 
    reference_03comma = NULL;
    reference_05comma = NULL;
    reference_07comma = NULL;
    reference_11comma = NULL;
    reference_13comma = NULL;
    reference_17comma = NULL;
    reference_19comma = NULL;
    reference_23comma = NULL;
    reference_29comma = NULL;
    reference_31comma = NULL;
}

void CmdParse::write_optvar() {
    // *** process UUID (hidden option)
    if (jack_uuid != NULL) {
        optvar[JACK_UUID] = jack_uuid; // leads to no automatic connection
        g_free(jack_uuid);
    } else if (!optvar[JACK_UUID].empty()) {
        optvar[JACK_UUID] = ""; 
    }
    // *** process ENGINE options
    if (pitch != NULL) {
        optvar[PITCH] = pitch;
        g_free(pitch);
    } else if (!optvar[PITCH].empty()) {
        optvar[PITCH] = ""; 
    }

    if (threshold != NULL) {
        optvar[THRESHOLD] = threshold;
        g_free(threshold);
    } else if (!optvar[THRESHOLD].empty()) {
        optvar[THRESHOLD] = ""; 
    }

    if (mode != NULL) {
        optvar[MODE] = mode;
        g_free(mode);
    } else if (!optvar[MODE].empty()) {
        optvar[MODE] = ""; 
    }
    if (doremi != NULL) { //#2
        optvar[DOREMI] = doremi;
        g_free(doremi);
    } else if (!optvar[DOREMI].empty()) {
        optvar[DOREMI] = ""; 
    }
    if (reference_note != NULL) { 
        optvar[REFERENCE_NOTE] = reference_note;
        g_free(reference_note);
    } else if (!optvar[REFERENCE_NOTE].empty()) {
        optvar[REFERENCE_NOTE] = ""; 
    }
if (reference_03comma != NULL) { 
        optvar[REFERENCE_03COMMA] = reference_03comma;
        g_free(reference_03comma);
    } else if (!optvar[REFERENCE_03COMMA].empty()) {
        optvar[REFERENCE_03COMMA] = ""; 
    }
if (reference_05comma != NULL) { 
        optvar[REFERENCE_05COMMA] = reference_05comma;
        g_free(reference_05comma);
    } else if (!optvar[REFERENCE_05COMMA].empty()) {
        optvar[REFERENCE_05COMMA] = ""; 
    }
if (reference_07comma != NULL) { 
        optvar[REFERENCE_07COMMA] = reference_07comma;
        g_free(reference_07comma);
    } else if (!optvar[REFERENCE_07COMMA].empty()) {
        optvar[REFERENCE_07COMMA] = ""; 
    }
if (reference_11comma != NULL) { 
        optvar[REFERENCE_11COMMA] = reference_11comma;
        g_free(reference_11comma);
    } else if (!optvar[REFERENCE_11COMMA].empty()) {
        optvar[REFERENCE_11COMMA] = ""; 
    }
if (reference_13comma != NULL) { 
        optvar[REFERENCE_13COMMA] = reference_13comma;
        g_free(reference_13comma);
    } else if (!optvar[REFERENCE_13COMMA].empty()) {
        optvar[REFERENCE_13COMMA] = ""; 
    }
if (reference_17comma != NULL) { 
        optvar[REFERENCE_17COMMA] = reference_17comma;
        g_free(reference_17comma);
    } else if (!optvar[REFERENCE_17COMMA].empty()) {
        optvar[REFERENCE_17COMMA] = ""; 
    }
if (reference_19comma != NULL) { 
        optvar[REFERENCE_19COMMA] = reference_19comma;
        g_free(reference_19comma);
    } else if (!optvar[REFERENCE_19COMMA].empty()) {
        optvar[REFERENCE_19COMMA] = ""; 
    }
if (reference_23comma != NULL) { 
        optvar[REFERENCE_23COMMA] = reference_23comma;
        g_free(reference_23comma);
    } else if (!optvar[REFERENCE_23COMMA].empty()) {
        optvar[REFERENCE_23COMMA] = ""; 
    }
if (reference_29comma != NULL) { 
        optvar[REFERENCE_29COMMA] = reference_29comma;
        g_free(reference_29comma);
    } else if (!optvar[REFERENCE_29COMMA].empty()) {
        optvar[REFERENCE_29COMMA] = ""; 
    }
if (reference_31comma != NULL) { 
        optvar[REFERENCE_31COMMA] = reference_31comma;
        g_free(reference_31comma);
    } else if (!optvar[REFERENCE_31COMMA].empty()) {
        optvar[REFERENCE_31COMMA] = ""; 
    }
    
    // *** process GTK options
    if (size_y != NULL) {
        optvar[SIZE_Y] = size_y;
        g_free(size_y);
    } else if (!optvar[SIZE_Y].empty()) {
        optvar[SIZE_Y] = ""; 
    }

    if (size_x != NULL) {
        optvar[SIZE_X] = size_x;
        g_free(size_x);
    } else if (!optvar[SIZE_X].empty()) {
        optvar[SIZE_X] = ""; 
    }

    if (pos_y != NULL) {
        optvar[POS_Y] = pos_y;
        g_free(pos_y);
    } else if (!optvar[POS_Y].empty()) {
        optvar[POS_Y] = ""; 
    }

    if (pos_x != NULL) {
        optvar[POS_X] = pos_x;
        g_free(pos_x);
    } else if (!optvar[POS_X].empty()) {
        optvar[POS_X] = ""; 
    }

    if (desktop != NULL) {
        optvar[DESK] = desktop;
        g_free(desktop);
    } else if (!optvar[DESK].empty()) {
        optvar[DESK] = ""; 
    }
    
    // *** process JACK options
    if (jack_input != NULL) {
        optvar[JACK_INP] = jack_input;
        g_free(jack_input);
    } else if (!optvar[JACK_INP].empty()) {
        optvar[JACK_INP] = ""; // leads to no automatic connection
    }
}

void CmdParse::parse(int& argc, char**& argv) {
    // parsing command options
    if (!g_option_context_parse(opt_context, &argc, &argv, &error)) {
        g_print ("option parsing failed: %s\n", error->message);
        error = NULL;
    }
    g_option_context_free(opt_context);
}

void CmdParse::setup_groups() {
    optgroup_gtk = g_option_group_new("gtk",
          "\033[1;32mGTK configuration options\033[0m",
          "\033[1;32mGTK configuration options\033[0m",
          NULL, NULL);
    GOptionEntry opt_entries_gtk[] =
    {
        { "posx", 'x', 0, G_OPTION_ARG_STRING, &pos_x,
            "window position x-axis ( -x 1 -> . .)", "POSITION_X"},
        { "posy", 'y', 0, G_OPTION_ARG_STRING, &pos_y,
            "window position y-axis ( -y 1 -> . . )", "POSITION_Y" },
        { "wigth", 'w', 0, G_OPTION_ARG_STRING, &size_x,
            "'default' width ( -w 120 -> . .)", "WIDTH"},
        { "height", 'l', 0, G_OPTION_ARG_STRING, &size_y,
            "'default' height ( -l 100 -> . .)", "HEIGHT" },
        { "desktop", 'd', 0, G_OPTION_ARG_STRING, &desktop,
            "set to virtual desktop num ( -d 0 -> . .)", "NUM" },
        { NULL }
    };
    g_option_group_add_entries(optgroup_gtk, opt_entries_gtk);

    optgroup_jack = g_option_group_new("jack",
          "\033[1;32mJACK configuration options\033[0m",
          "\033[1;32mJACK configuration options\033[0m",
          NULL, NULL);
    GOptionEntry opt_entries_jack[] =
    {
        { "jack-input", 'i', 0, G_OPTION_ARG_STRING, &jack_input,
            "connect to JACK port name (-i system:capture_1)", "PORT" },
        { NULL }
    };
    g_option_group_add_entries(optgroup_jack, opt_entries_jack);

    GOptionEntry opt_entries_uuid[] =
    {
        { "jack-uuid", 'U', G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_STRING, &jack_uuid,
            "JACK session UUID (set by session manager)", "UUID" },
        { NULL }
    };
    g_option_group_add_entries(optgroup_jack, opt_entries_uuid);

    optgroup_engine = g_option_group_new("engine",
          "\033[1;32mENGINE configuration options\033[0m",
          "\033[1;32mENGINE configuration options\033[0m",
          NULL, NULL);
    GOptionEntry opt_entries_engine[] =
    {
        { "pitch", 'p', 0, G_OPTION_ARG_STRING, &pitch,
            "set reference pitch ( -p 200,0 <-> 600,0)", "PITCH"},
        { "threshold", 't', 0, G_OPTION_ARG_STRING, &threshold,
            "set threshold level (-t 0,001 <-> 0,5)", "THRESHOLD" },
        { "mode", 'm', 0, G_OPTION_ARG_STRING, &mode,
            "set tuner mode (-m chromatic / scale3diatonic / scale35chromatic / scale357chromatic / scale37chromatic / scaleovertones / scale16limit / scalegreekdorian )", "MODE" },
        { "doremi", 'N', 0, G_OPTION_ARG_STRING, &doremi,
            "set base note type (-N cde / doremi )", "DOREMI" },
        { "reference_note", 'R', 0, G_OPTION_ARG_STRING, &reference_note,
            "set reference note (-R C / D / E / F / G / A / B )", "REFERENCE_NOTE" },
        { "reference_03comma", 'A', 0, G_OPTION_ARG_STRING, &reference_03comma,
            "set reference 3 limit comma (-A min3 / min2 / min1 / 0 / 1 / 2 / 3 )", "REFERENCE_03COMMA" },
        { "reference_05comma", 'B', 0, G_OPTION_ARG_STRING, &reference_05comma,
            "set reference 5 limit comma (-B min3 / min2 / min1 / 0 / 1 / 2 / 3 )", "REFERENCE_05COMMA" },
        { "reference_07comma", 'C', 0, G_OPTION_ARG_STRING, &reference_07comma,
            "set reference 7 limit comma (-C min3 / min2 / min1 / 0 / 1 / 2 / 3 )", "REFERENCE_07COMMA" },
        { "reference_11comma", 'D', 0, G_OPTION_ARG_STRING, &reference_11comma,
            "set reference 11 limit comma (-D min3 / min2 / min1 / 0 / 1 / 2 / 3 )", "REFERENCE_11COMMA" },
        { "reference_13comma", 'E', 0, G_OPTION_ARG_STRING, &reference_13comma,
            "set reference 13 limit comma (-E min3 / min2 / min1 / 0 / 1 / 2 / 3 )", "REFERENCE_13COMMA" },
        { "reference_17comma", 'F', 0, G_OPTION_ARG_STRING, &reference_17comma,
            "set reference 17 limit comma (-F min3 / min2 / min1 / 0 / 1 / 2 / 3 )", "REFERENCE_17COMMA" },
        { "reference_19comma", 'G', 0, G_OPTION_ARG_STRING, &reference_19comma,
            "set reference 19 limit comma (-G min3 / min2 / min1 / 0 / 1 / 2 / 3 )", "REFERENCE_19COMMA" },
        { "reference_23comma", 'H', 0, G_OPTION_ARG_STRING, &reference_23comma,
            "set reference 23 limit comma (-H min3 / min2 / min1 / 0 / 1 / 2 / 3 )", "REFERENCE_23COMMA" },
        { "reference_29comma", 'I', 0, G_OPTION_ARG_STRING, &reference_29comma,
            "set reference 29 limit comma (-I min3 / min2 / min1 / 0 / 1 / 2 / 3 )", "REFERENCE_29COMMA" },
        { "reference_31comma", 'J', 0, G_OPTION_ARG_STRING, &reference_31comma,
            "set reference 31 limit comma (-J min3 / min2 / min1 / 0 / 1 / 2 / 3 )", "REFERENCE_31COMMA" },        
        { NULL }
    };
    g_option_group_add_entries(optgroup_engine, opt_entries_engine);
    g_option_context_add_group(opt_context, optgroup_gtk);
    g_option_context_add_group(opt_context, optgroup_jack);
    g_option_context_add_group(opt_context, optgroup_engine);
    g_option_context_set_ignore_unknown_options(opt_context, true);
}

// ---- parse command line options
void CmdParse::process_cmdline_options(int& argc, char**& argv)
{
    init();
    setup_groups();
    parse(argc, argv);
    write_optvar();
}

CmdParse cmd;