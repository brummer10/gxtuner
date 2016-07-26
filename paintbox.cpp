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


#include "paintbox.h"
#include "frame.h"

#include <gtk/gtkprivate.h>
#include <gtk/gtk.h>
#include <cmath>
#include <cstring>

#define P_(s) (s)   // FIXME -> gettext


typedef struct {
	const char *icon_name;
	const guint8 *icon_data;
} image_entry;

enum {
	PROP_PAINT_FUNC = 1,
	PROP_ICON_SET = 2,
};

static void gx_paint_box_destroy(GtkObject *object);
static void gx_paint_box_set_property(
	GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void gx_paint_box_get_property(
	GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static gboolean gx_paint_box_expose(GtkWidget *widget, GdkEventExpose *event);
static void gx_paint_box_style_set (GtkWidget *widget, GtkStyle  *previous_style);

G_DEFINE_TYPE(GxPaintBox, gx_paint_box, GTK_TYPE_BOX)

#define get_stock_id(widget) (GX_PAINT_BOX_CLASS(GTK_OBJECT_GET_CLASS(widget))->stock_id)

static void gx_paint_box_class_init (GxPaintBoxClass *klass)
{
	GObjectClass   *gobject_class = G_OBJECT_CLASS(klass);
	GtkObjectClass *object_class = (GtkObjectClass*) klass;
	GtkWidgetClass *widget_class = (GtkWidgetClass*)klass;
	gobject_class->set_property = gx_paint_box_set_property;
	gobject_class->get_property = gx_paint_box_get_property;
	object_class->destroy = gx_paint_box_destroy;
	widget_class->style_set = gx_paint_box_style_set;
	widget_class->expose_event = gx_paint_box_expose;
	klass->stock_id = "rahmen";

	g_object_class_install_property(
		gobject_class, PROP_PAINT_FUNC,
		g_param_spec_string("paint-func",
		                    P_("Paint Type"),
		                    P_("Type of paint function for background"),
		                    "",
		                    GParamFlags(GTK_PARAM_READWRITE)));

	gtk_widget_class_install_style_property(
		GTK_WIDGET_CLASS(klass),
		g_param_spec_string("paint-func",
		                    P_("Paint Type"),
		                    P_("Type of paint function for background"),
		                    NULL,
		                    GParamFlags(GTK_PARAM_READABLE)));
	g_object_class_install_property(
		gobject_class,PROP_ICON_SET,
	    g_param_spec_int ("icon-set",
						 P_("Icon Set"),
						 P_("Type of Icon function for background"),
						 0,
						 G_MAXINT,
						 0,
						 GParamFlags(GTK_PARAM_READWRITE)));
	gtk_widget_class_install_style_property(
		GTK_WIDGET_CLASS(klass),
		g_param_spec_int("icon-set",
		                 P_("Icon Set"),
		                 P_("Type of Icon function for background"),
		                 0,
						 G_MAXINT,
						 0,
		                 GParamFlags(GTK_PARAM_READABLE)));
	gtk_widget_class_install_style_property(
		GTK_WIDGET_CLASS(klass),
		g_param_spec_int("width",
		                 P_("Width"),
		                 P_("size.width request for paintbox"),
		                 0,
						 G_MAXINT,
						 0,
		                 GParamFlags(GTK_PARAM_READABLE)));
	gtk_widget_class_install_style_property(
		GTK_WIDGET_CLASS(klass),
		g_param_spec_int("height",
		                 P_("Height"),
		                 P_("size.height request for paintbox"),
		                 0,
						 G_MAXINT,
						 0,
		                 GParamFlags(GTK_PARAM_READABLE)));

}



static void set_paint_func(GxPaintBox *paint_box, const gchar *paint_func)
{
	gchar *spf;
	gtk_widget_style_get(GTK_WIDGET(paint_box), "paint-func", &spf, NULL);
	if (spf) {
		if (paint_box->paint_func && strcmp(paint_box->paint_func, spf) == 0) {
			return;
		}
	} else {
		if (!paint_func) {
			paint_func = "";
		}
		if (paint_box->paint_func && strcmp(paint_box->paint_func, paint_func) == 0) {
			return;
		}
		spf = g_strdup(paint_func);
	}
	g_free(paint_box->paint_func);
	paint_box->paint_func = spf;
	set_expose_func(paint_box, spf);
	g_object_notify(G_OBJECT(paint_box), "paint-func");
}

static void gx_paint_box_style_set(GtkWidget *widget, GtkStyle  *previous_style)
{
	GxPaintBox *paint_box = GX_PAINT_BOX(widget);
	set_paint_func(paint_box, paint_box->paint_func);
}

static image_entry image_data[] = {
    { "rahmen", rahmen },
	{ NULL, NULL },
};


static void gx_paint_box_init (GxPaintBox *paint_box)
{
    GtkIconFactory *factory = gtk_icon_factory_new();
	for (image_entry *p = image_data; p->icon_name; p++) {
		gtk_icon_factory_add(
			factory, p->icon_name,
			gtk_icon_set_new_from_pixbuf(
				gdk_pixbuf_new_from_inline(
					-1, p->icon_data, FALSE, NULL)));
	}
	gtk_icon_factory_add_default(factory);
	gtk_widget_set_redraw_on_allocate(GTK_WIDGET(paint_box), TRUE);
	paint_box->paint_func = g_strdup("");
	set_paint_func(paint_box, NULL);
	paint_box->gxr_image = NULL;
	paint_box->icon_set = 0;
    paint_box->stock_image = gtk_widget_render_icon(GTK_WIDGET(paint_box),
        get_stock_id(GTK_WIDGET(paint_box)),(GtkIconSize)-1,NULL);
}

static void gx_paint_box_destroy(GtkObject *object)
{
	GxPaintBox *paint_box = GX_PAINT_BOX(object);
	if (paint_box->paint_func) {
		g_free(paint_box->paint_func);
		paint_box->paint_func = NULL;
	}
	if (G_IS_OBJECT(paint_box->gxr_image)) {
		g_object_unref(paint_box->gxr_image);
	}
	paint_box->gxr_image = NULL;
    if (G_IS_OBJECT(paint_box->stock_image)) {
		g_object_unref(paint_box->stock_image);
	}
	paint_box->stock_image = NULL;
	GTK_OBJECT_CLASS(gx_paint_box_parent_class)->destroy(object);
}

static gboolean gx_paint_box_expose(GtkWidget *widget, GdkEventExpose *event)
{
	GxPaintBox *paint_box = GX_PAINT_BOX(widget);
	if (paint_box->expose_func) {
		paint_box->expose_func(widget, event);
	}
	GTK_WIDGET_CLASS(GTK_OBJECT_CLASS(gx_paint_box_parent_class))->expose_event(widget, event);
	return FALSE;
}
static void set_icon(GxPaintBox *paint_box, int value)
{
	int spf;
	gtk_widget_style_get(GTK_WIDGET(paint_box), "icon-set", &spf, NULL);
	 paint_box->icon_set = spf;
}
static void gx_paint_box_set_property(
	GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	GxPaintBox *paint_box = GX_PAINT_BOX(object);
	switch (prop_id) {
	case PROP_PAINT_FUNC:
		set_paint_func(paint_box, g_value_get_string(value));
		break;
	case PROP_ICON_SET:
		set_icon(paint_box, g_value_get_int(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void gx_paint_box_get_property(
	GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	switch (prop_id) {
	case PROP_PAINT_FUNC:
		g_value_set_string(value, GX_PAINT_BOX(object)->paint_func);
		break;
	case PROP_ICON_SET:
		g_value_set_int (value, GX_PAINT_BOX(object)->icon_set);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

GtkWidget *gx_paint_box_new (GtkOrientation orientation, gboolean homogeneous, gint spacing)
{
	return (GtkWidget*)g_object_new(
		GX_TYPE_PAINT_BOX,
		"orientation", orientation,
		"spacing",     spacing,
		"homogeneous", homogeneous ? TRUE : FALSE,
		NULL);
}

/****************************************************************
 ** Paint functions
 */

static void rahmen_expose(GtkWidget *wi, GdkEventExpose *ev)
{
	gint rect_width  = wi->allocation.width-2;
	gint rect_height = wi->allocation.height-3;
	if (rect_width <= 0 || rect_height <= 0) {
	    return;
	}
	cairo_t *cr;
	GxPaintBox *paintbox = GX_PAINT_BOX(wi);
	/* create a cairo context */
	cr = gdk_cairo_create(wi->window);
	GdkRegion *region;
	region = gdk_region_rectangle (&wi->allocation);
	gdk_region_intersect (region, ev->region);
	gdk_cairo_region (cr, region);
	cairo_clip (cr);
	
	gint x0      = wi->allocation.x+1;
	gint y0      = wi->allocation.y+1;

	static double ne_w = 0.;
	if (ne_w != rect_width*rect_height || !(GDK_IS_PIXBUF (paintbox-> gxr_image))) {
		ne_w = rect_width*rect_height;
		if (G_IS_OBJECT(paintbox-> gxr_image)) {
			g_object_unref(paintbox->gxr_image);
		}
		GdkPixbuf  *frame;
		double scalew = rect_width/double(gdk_pixbuf_get_width(paintbox->stock_image)-48);
		double scaleh = rect_height/double(gdk_pixbuf_get_height(paintbox->stock_image)-48);
		
		paintbox->gxr_image = gdk_pixbuf_scale_simple(
			paintbox->stock_image, rect_width, rect_height, GDK_INTERP_NEAREST);
		// upper border
		frame = gdk_pixbuf_new_subpixbuf(
			paintbox->stock_image,24,0,gdk_pixbuf_get_width(paintbox->stock_image)-48,12);
		gdk_pixbuf_scale (
			frame, paintbox->gxr_image,0,0,rect_width,12,0,0,scalew,1,GDK_INTERP_BILINEAR);
		// under border
		frame = gdk_pixbuf_new_subpixbuf(
			paintbox->stock_image,24,gdk_pixbuf_get_height(paintbox->stock_image)-12,
			gdk_pixbuf_get_width(paintbox->stock_image)-48,12);
		gdk_pixbuf_scale (
			frame,paintbox->gxr_image,0,gdk_pixbuf_get_height(paintbox->gxr_image)-12,
			rect_width,12,0,gdk_pixbuf_get_height(paintbox->gxr_image)-12,
			scalew,1,GDK_INTERP_BILINEAR);
		// left border
		frame = gdk_pixbuf_new_subpixbuf(
			paintbox->stock_image,0,24,12,gdk_pixbuf_get_height(paintbox->stock_image)-48);
		gdk_pixbuf_scale(
			frame, paintbox->gxr_image,0,12,12,rect_height-24,0,0,1,scaleh,GDK_INTERP_BILINEAR);
		// right border	
		frame = gdk_pixbuf_new_subpixbuf(
			paintbox->stock_image,gdk_pixbuf_get_width(paintbox->stock_image)-12,
			24,12,gdk_pixbuf_get_height(paintbox->stock_image)-48);
		gdk_pixbuf_scale(
			frame,paintbox->gxr_image,gdk_pixbuf_get_width(paintbox->gxr_image)-12,
			12,12,rect_height-24,gdk_pixbuf_get_width(paintbox->gxr_image)-12,
			0,1,scaleh,GDK_INTERP_BILINEAR);
		//left upper corner
		frame = gdk_pixbuf_new_subpixbuf(
			paintbox->stock_image,0,0,20,20);
		gdk_pixbuf_scale (
			frame, paintbox->gxr_image,0,0,20,20,0,0,1,1,GDK_INTERP_BILINEAR);
		//right upper corner
		frame = gdk_pixbuf_new_subpixbuf(
			paintbox->stock_image,gdk_pixbuf_get_width(paintbox->stock_image)-20,0,20,20);
		gdk_pixbuf_scale (
			frame, paintbox->gxr_image,gdk_pixbuf_get_width(paintbox->gxr_image)-20,
			0,20,20,gdk_pixbuf_get_width(paintbox->gxr_image)-20,0,1,1,
			GDK_INTERP_BILINEAR);
		//left under corner
		frame = gdk_pixbuf_new_subpixbuf(
			paintbox->stock_image,0,gdk_pixbuf_get_height(paintbox->stock_image)-20,20,20);
		gdk_pixbuf_scale (
			frame, paintbox->gxr_image,0,gdk_pixbuf_get_height(paintbox->gxr_image)-20,
			20,20,0,gdk_pixbuf_get_height(paintbox->gxr_image)-20,1,1,
			GDK_INTERP_BILINEAR);
		//right under corner
		frame = gdk_pixbuf_new_subpixbuf(
			paintbox->stock_image,gdk_pixbuf_get_width(paintbox->stock_image)-20,
			gdk_pixbuf_get_height(paintbox->stock_image)-20,20,20);
		gdk_pixbuf_scale (
			frame, paintbox->gxr_image,gdk_pixbuf_get_width(paintbox->gxr_image)-20,
			gdk_pixbuf_get_height(paintbox->gxr_image)-20,
			20,20,gdk_pixbuf_get_width(paintbox->gxr_image)-20,
			gdk_pixbuf_get_height(paintbox->gxr_image)-20,1,1,
			GDK_INTERP_BILINEAR);
		g_object_unref(paintbox->stock_image);
		g_object_unref(frame);
	}
	
	// draw to display
	gdk_draw_pixbuf(GDK_DRAWABLE(wi->window), gdk_gc_new(GDK_DRAWABLE(wi->window)),
	                paintbox->gxr_image, 0, 0,
	                x0, y0, rect_width,rect_height,
	                GDK_RGB_DITHER_NORMAL, 0, 0);

	// base 
	
	cairo_pattern_t*pat =
	cairo_pattern_create_linear (x0, y0, rect_width, y0);
    
    cairo_pattern_add_color_stop_rgb (pat, 1, 0.2, 0.2 , 0.2);
    cairo_pattern_add_color_stop_rgb (pat, 0.5, 0.1, 0.1 , 0.1);
    cairo_pattern_add_color_stop_rgb (pat, 0,0.05, 0.05 , 0.05);
    x0      += 11;
	y0      += 11;
	rect_width  -= 22;
	rect_height -= 22;
	cairo_set_source (cr, pat);
    cairo_rectangle (cr, x0,y0,rect_width,rect_height);
	cairo_fill (cr);

	cairo_pattern_destroy (pat);
	cairo_destroy(cr);
	gdk_region_destroy (region);                
}

void set_expose_func(GxPaintBox *paint_box, const gchar *paint_func)
{
	if (strcmp(paint_func, "rahmen_expose") == 0) {
		paint_box->expose_func = rahmen_expose;
	} else {
		paint_box->expose_func = 0;
	}
}
