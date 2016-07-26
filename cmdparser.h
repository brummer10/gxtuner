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


#define JACK_INP        (0)
#define SIZE_X          (1)
#define SIZE_Y          (2)
#define POS_X           (3)
#define POS_Y           (4)
#define PITCH           (5)
#define THRESHOLD       (6)
#define JACK_UUID       (7)
#define DESK            (8)

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
    std::string         infostring;
    void                init();
    void                setup_groups();
    void                parse(int& argc, char**& argv);
    void                write_optvar();
 protected:
    std::string         optvar[9];

 public:
    explicit CmdParse();
    ~CmdParse();
    std::string         get_optvar(int a) {return optvar[a];}
    void                process_cmdline_options(int& argc, char**& argv);
};
extern CmdParse         cmd;

#endif // CMD_PARSER_H_


