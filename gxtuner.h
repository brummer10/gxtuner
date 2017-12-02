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
 *        file: gxtuner.h   guitar tuner for jack
 *
 * ----------------------------------------------------------------------------
 */

#pragma once

#ifndef _GX_TUNER_H_
#define _GX_TUNER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <glib-object.h>
#include <gtk/gtk.h>
#include <string> 

G_BEGIN_DECLS

#define GX_TYPE_TUNER          (gx_tuner_get_type())
#define GX_TUNER(obj)          (G_TYPE_CHECK_INSTANCE_CAST ((obj), GX_TYPE_TUNER, GxTuner))
#define GX_IS_TUNER(obj)       (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GX_TYPE_TUNER))
#define GX_TUNER_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST ((klass),  GX_TYPE_TUNER, GxTunerClass))
#define GX_IS_TUNER_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GX_TYPE_TUNER))
# define NRPRIMES 12

typedef struct _GxTuner GxTuner;
typedef struct _GxTunerClass GxTunerClass;

// the internal struct of the tuner widget,
// add variables for new propertys in the struct

struct _GxTuner
{
    GtkDrawingArea parent;
    std::string tempscalenames;
    std::string namecomma();
    double freq;
    double reference_pitch;
    double scale_w;
    double scale_h;
    double tempscaleratios;
    int tempscale;
    int numberofnotes;
    int tempreference_note;
    int tempscaletranslated;
    int tempscaletranslatedpowprimes;
    int mode;
    int reference_note; //#1
    int reference_03comma;
    int reference_05comma;
    int reference_07comma;
    int reference_11comma;
    int reference_13comma;
    int reference_17comma;
    int reference_19comma;
    int reference_23comma;
    int reference_29comma;
    int reference_31comma;  
};

struct _GxTunerClass
{
    GtkDrawingAreaClass parent_class;
    /*< private >*/
    cairo_surface_t *surface_tuner;
};

GType gx_tuner_get_type();

// this are the calles which could be used from outside the widget
// if you add a new property, add a call to set it here

void gx_tuner_set_freq(GxTuner *tuner, double freq);
void gx_tuner_set_reference_pitch(GxTuner *tuner, double reference_pitch);
double gx_tuner_get_reference_pitch(GxTuner *tuner);
void gx_tuner_set_mode(GxTuner *tuner, int mode);
void gx_tuner_set_reference_note(GxTuner *tuner, int reference_note); //#2
void gx_tuner_set_reference_03comma(GxTuner *tuner, int reference_03comma);
void gx_tuner_set_reference_05comma(GxTuner *tuner, int reference_05comma);
void gx_tuner_set_reference_07comma(GxTuner *tuner, int reference_07comma);
void gx_tuner_set_reference_11comma(GxTuner *tuner, int reference_11comma);
void gx_tuner_set_reference_13comma(GxTuner *tuner, int reference_13comma);
void gx_tuner_set_reference_17comma(GxTuner *tuner, int reference_17comma);
void gx_tuner_set_reference_19comma(GxTuner *tuner, int reference_19comma);
void gx_tuner_set_reference_23comma(GxTuner *tuner, int reference_23comma);
void gx_tuner_set_reference_29comma(GxTuner *tuner, int reference_29comma);
void gx_tuner_set_reference_31comma(GxTuner *tuner, int reference_31comma);
GtkWidget *gx_tuner_new(void);

G_END_DECLS

#ifdef  __cplusplus
}
#endif

#endif // _GX_TUNER_H_