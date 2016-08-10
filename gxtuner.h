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


G_BEGIN_DECLS

#define GX_TYPE_TUNER          (gx_tuner_get_type())
#define GX_TUNER(obj)          (G_TYPE_CHECK_INSTANCE_CAST ((obj), GX_TYPE_TUNER, GxTuner))
#define GX_IS_TUNER(obj)       (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GX_TYPE_TUNER))
#define GX_TUNER_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST ((klass),  GX_TYPE_TUNER, GxTunerClass))
#define GX_IS_TUNER_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GX_TYPE_TUNER))

typedef struct _GxTuner GxTuner;
typedef struct _GxTunerClass GxTunerClass;

struct _GxTuner
{
    GtkDrawingArea parent;
    double freq;
    double reference_pitch;
    double scale_w;
    double scale_h;
};

struct _GxTunerClass
{
    GtkDrawingAreaClass parent_class;
    /*< private >*/
    cairo_surface_t *surface_tuner;
};

GType gx_tuner_get_type();

void gx_tuner_set_freq(GxTuner *tuner, double freq);
void gx_tuner_set_reference_pitch(GxTuner *tuner, double reference_pitch);
double gx_tuner_get_reference_pitch(GxTuner *tuner);
GtkWidget *gx_tuner_new(void);

G_END_DECLS

#ifdef  __cplusplus
}
#endif

#endif // _GX_TUNER_H_

