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
 *        file: comparse.h      guitar tuner for jack
 *
 * ----------------------------------------------------------------------------
 */
 
#pragma once

#ifndef CMD_PARSER_H_
#define CMD_PARSER_H_

#include <glib.h>
#include <string> 
#include <cstdlib>


#define JACK_INP            (0)
#define SIZE_X              (1)
#define SIZE_Y              (2)
#define POS_X               (3)
#define POS_Y               (4)
#define PITCH               (5)
#define THRESHOLD           (6)
#define JACK_UUID           (7)
#define DESK                (8)
#define MODE                (9)
#define REFERENCE_NOTE      (10) 
#define REFERENCE_03COMMA   (11)
#define REFERENCE_05COMMA   (12)
#define REFERENCE_07COMMA   (13)
#define REFERENCE_11COMMA   (14)
#define REFERENCE_13COMMA   (15)
#define REFERENCE_17COMMA   (16)
#define REFERENCE_19COMMA   (17)
#define REFERENCE_23COMMA   (18)
#define REFERENCE_29COMMA   (19)
#define REFERENCE_31COMMA   (20)

class CmdParse {
 private:
    GError*             error;
    GOptionContext*     opt_context;
    GOptionGroup*       optgroup_gtk;
    GOptionGroup*       optgroup_jack;
    GOptionGroup*       optgroup_engine;
    gchar*              jack_uuid;
    gchar*              jack_input;
    gchar*              size_x;
    gchar*              size_y;
    gchar*              pos_x;
    gchar*              pos_y;
    gchar*              desktop;
    gchar*              pitch;
    gchar*              threshold;
    gchar*              mode;
    gchar*              reference_note; 
    gchar*              reference_03comma;
    gchar*              reference_05comma;
    gchar*              reference_07comma;
    gchar*              reference_11comma;
    gchar*              reference_13comma;
    gchar*              reference_17comma;
    gchar*              reference_19comma;
    gchar*              reference_23comma;
    gchar*              reference_29comma;
    gchar*              reference_31comma;
    std::string         infostring;
    void                init();
    void                setup_groups();
    void                parse(int& argc, char**& argv);
    void                write_optvar();
 protected:
    std::string         optvar[21]; //#3

 public:
    explicit CmdParse();
    ~CmdParse();
    std::string         get_optvar(int a) {return optvar[a];}
    void                process_cmdline_options(int& argc, char**& argv);
};
extern CmdParse         cmd;

#endif // CMD_PARSER_H_