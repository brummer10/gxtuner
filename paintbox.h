/*
 * Copyright (C) 2009, 2010 Hermann Meyer, James Warden, Andreas Degert
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
 */

#pragma once

#ifndef __PAINT_BOX_H__
#define __PAINT_BOX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GX_TYPE_PAINT_BOX            (gx_paint_box_get_type ())
#define GX_PAINT_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GX_TYPE_PAINT_BOX, GxPaintBox))
#define GX_PAINT_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GX_TYPE_PAINT_BOX, GxPaintBoxClass))
#define GX_IS_PAINT_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GX_TYPE_PAINT_BOX))
#define GX_IS_PAINT_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GX_TYPE_PAINT_BOX))
#define GX_PAINT_BOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GX_TYPE_PAINT_BOX, GxPaintBoxClass))

typedef struct _GxPaintBox GxPaintBox;
typedef struct _GxPaintBoxClass GxPaintBoxClass;



struct _GxPaintBox {
	GtkBox box;
	gchar *paint_func;
	void (*expose_func)(GtkWidget*, cairo_t *);
	GdkPixbuf *gxr_image;
    GdkPixbuf *stock_image;
	guint icon_set;
};

struct _GxPaintBoxClass {
	GtkBoxClass parent_class;
	const gchar *stock_id;

};

GType gx_paint_box_get_type(void) G_GNUC_CONST;
GtkWidget *gx_paint_box_new(GtkOrientation orientation, gboolean homogeneous, gint spacing);
void set_expose_func(GxPaintBox *paint_box, const gchar *paint_func);

G_END_DECLS

#ifdef  __cplusplus
}
#endif

#endif /* __PAINT_BOX_H__ */
