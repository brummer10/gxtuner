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
 *        file: gtkkob.cc      guitar tuner for jack
 *
 * ----------------------------------------------------------------------------
 */

#include "gtkknob.h"

#include <gdk/gdkkeysyms.h>
#include <math.h>


#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif
#ifndef max
#define max(x, y) ((x) < (y) ? (y) : (x))
#endif


#define GTK_KNOB_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), GTK_TYPE_KNOB, GtkKnobPrivate))

G_DEFINE_TYPE(GtkKnob, gtk_knob, GTK_TYPE_RANGE);

typedef struct _GtkKnobPrivate GtkKnobPrivate;

struct _GtkKnobPrivate
{
	int knob_x;
	int knob_y;
	int knob_step;
	int button_is;
	double start_x, start_y, max_value;
	int last_quadrant;
    int order;
    
};

const double scale_zero = 20 * (M_PI/180); // defines "dead zone" for knobs

static void print_value(GObject *widget, char* s)
{
	
	//GtkWidget * label = (GTK_WIDGET(obj));
	float v = gtk_adjustment_get_value(GTK_ADJUSTMENT(widget));
	//char s[64];
	if (gtk_adjustment_get_step_increment(GTK_ADJUSTMENT(widget)) < 0.009999)
	{
		const char* format[] = {"%.1f", "%.2f", "%.3f"};
		snprintf(s, 63, format[3-1], v);
	}
	else if (gtk_adjustment_get_step_increment(GTK_ADJUSTMENT(widget)) < 0.09999)
	{
		const char* format[] = {"%.1f", "%.2f", "%.3f"};
		snprintf(s, 63, format[2-1], v);
	}
	else if (gtk_adjustment_get_step_increment(GTK_ADJUSTMENT(widget)) < 0.9999)
	{
		const char* format[] = {"%.1f", "%.2f", "%.3f"};
		snprintf(s, 63, format[1-1], v);
	}
	else if (gtk_adjustment_get_step_increment(GTK_ADJUSTMENT(widget)) < 9.9999)
	{
		snprintf(s, 63, "%d", (int)v);
	}
	else
		snprintf(s, 63, "%d", (int)v);
	//return s;
}

static void knob_expose(GtkWidget *widget, cairo_t *cr, int knob_x, int knob_y, int arc_offset)
{
	/** check resize **/
	int grow;
    static const double vala = 37.;
    GtkAllocation *allocation = g_new0 (GtkAllocation, 1);
    gtk_widget_get_allocation(GTK_WIDGET(widget), allocation); 
	if(allocation->width-vala > allocation->height) {
		grow = allocation->height;
	} else {
		grow =  allocation->width-vala;
	}
	knob_x = grow-4;
	knob_y = grow-4;
	/** get values for the knob **/
	GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(widget));
    int knobx,knobx1;
    GtkKnobPrivate *priv = GTK_KNOB_GET_PRIVATE(widget);
    int order = priv->order;
    if (order) {
        knobx = (2 + (allocation->width-vala-4 - knob_x) * 0.5);
        knobx1 = (2 + (allocation->width-vala-4)* 0.5);
    } else {
        knobx = (2 + (allocation->width+vala-4 - knob_x) * 0.5);
        knobx1 = (2 + (allocation->width+vala-4)* 0.5);
    }
	int knoby = (2 + (allocation->height-4 - knob_y) * 0.5);
	
	int knoby1 = (2 + (allocation->height-4) * 0.5);
	double knobstate = (gtk_adjustment_get_value(adj) - gtk_adjustment_get_lower(adj)) / (gtk_adjustment_get_upper(adj) - gtk_adjustment_get_lower(adj));
	double angle = scale_zero + knobstate * 2 * (M_PI - scale_zero);
	
	double knobstate1 = (0. - gtk_adjustment_get_lower(adj)) / (gtk_adjustment_get_upper(adj) - gtk_adjustment_get_lower(adj));
	double pointer_off = knob_x/6;
	double radius = min(knob_x-pointer_off, knob_y-pointer_off) / 2;
	double lengh_x = (knobx+radius+pointer_off/2) - radius * sin(angle);
	double lengh_y = (knoby+radius+pointer_off/2) + radius * cos(angle);
	double radius1 = min(knob_x, knob_y) / 2 ;

	/** get widget forground color convert to cairo **/
	//GtkStyle *style = gtk_widget_get_style (widget);
	//double r = min(0.6,style->fg[gtk_widget_get_state_flags(widget)].red/65535.0),
	//	   g = min(0.6,style->fg[gtk_widget_get_state_flags(widget)].green/65535.0),
	//	   b = min(0.6,style->fg[gtk_widget_get_state_flags(widget)].blue/65535.0);
	double r = 0.6;
	double b = 0.6;
	double g = 0.6;

	/** paint focus **/
	//if (gtk_widget_has_focus(widget)== TRUE) {
	//	gtk_paint_focus(style, cr, GTK_STATE_NORMAL, widget, NULL,
	//	                knobx-2, knoby-2, knob_x+4, knob_y+4);
	//}
	/** create clowing knobs with cairo **/
	
	cairo_arc(cr,knobx1+arc_offset, knoby1+arc_offset, knob_x/2.1, 0, 2 * M_PI );
	cairo_pattern_t*pat =
		cairo_pattern_create_radial (knobx1+arc_offset-knob_x/6,knoby1+arc_offset-knob_x/6, 1,knobx1+arc_offset,knoby1+arc_offset,knob_x/2.1 );
	if(gtk_adjustment_get_lower(adj)<428 && gtk_adjustment_get_value(adj)>440.) {
		cairo_pattern_add_color_stop_rgb (pat, 0, 0.4 +( knobstate-0.5), 0.4, 0.4);
		cairo_pattern_add_color_stop_rgb (pat, 0.7, 0.15+( knobstate-0.3), 0.15 , 0.15);
		cairo_pattern_add_color_stop_rgb (pat, 1, 0.15,0.15,0.15);
	} else if(gtk_adjustment_get_lower(adj)>425 && gtk_adjustment_get_value(adj)<440.) {
		cairo_pattern_add_color_stop_rgb (pat, 0, 0.4 , 0.4, 0.4 + (0.5 - knobstate));
		cairo_pattern_add_color_stop_rgb (pat, 0.7, 0.15 , 0.15, 0.15 + (0.7 - knobstate));
		cairo_pattern_add_color_stop_rgb (pat, 1, 0.15,0.15,0.15);
	} else if(gtk_adjustment_get_lower(adj)<5 && gtk_adjustment_get_value(adj)<4.) {
		cairo_pattern_add_color_stop_rgb (pat, 0, 0.4+knobstate*0.5 , 0.4-(knobstate*0.2), 0.4);
		cairo_pattern_add_color_stop_rgb (pat, 0.7, 0.15+knobstate*0.5 , 0.4-(knobstate*0.2), 0.15);
		cairo_pattern_add_color_stop_rgb (pat, 1,0.15,0.15,0.15);
	} else {
		cairo_pattern_add_color_stop_rgb (pat, 0, 0.4, 0.4, 0.4);
		cairo_pattern_add_color_stop_rgb (pat, 0.7, 0.15, 0.4, 0.15);
		cairo_pattern_add_color_stop_rgb (pat, 1, 0.15, 0.15,0.15);
	}
	GdkRGBA fg_color =   {0.3, 0.3, 0.3, 0.7};
	cairo_set_source (cr, pat);
	cairo_fill_preserve (cr);
	gdk_cairo_set_source_rgba(cr, &fg_color);
	cairo_set_line_width(cr, 2.0);
	cairo_stroke(cr);

	/** create a rotating pointer on the kob**/
	gdk_cairo_set_source_rgba(cr, &fg_color);
	cairo_set_line_width(cr,max(3, min(5, knob_x/15)));
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND); 
	cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);
	cairo_move_to(cr, knobx+radius1, knoby+radius1);
	cairo_line_to(cr,lengh_x,lengh_y);
	cairo_stroke_preserve(cr);
	cairo_set_line_width(cr,min(5, max(1,knob_x/30)));
	pat =
		cairo_pattern_create_linear (knobx+radius1, knoby+radius1,lengh_x,lengh_y);
	if(gtk_adjustment_get_lower(adj)<0 && gtk_adjustment_get_value(adj)>0.) {
		cairo_pattern_add_color_stop_rgb (pat, 0.9, r+0.4, g+0.4 + knobstate-knobstate1, b+0.4);
		cairo_pattern_add_color_stop_rgb (pat, 0.3, r+0.15, g+0.15 + (knobstate-knobstate1)*0.5, b+0.15);
		cairo_pattern_add_color_stop_rgb (pat, 0.1, r, g, b);
	} else if(gtk_adjustment_get_lower(adj)<0 && gtk_adjustment_get_value(adj)<=0.) {
		cairo_pattern_add_color_stop_rgb (pat, 0.9, r+0.4 +knobstate1- knobstate, g+0.4, b+0.4);
		cairo_pattern_add_color_stop_rgb (pat, 0.3, r+0.15 +(knobstate1- knobstate)*0.5, g+0.15, b+0.15);
		cairo_pattern_add_color_stop_rgb (pat, 0.1, r, g, b);
	} else {
		cairo_pattern_add_color_stop_rgb (pat, 0.9, r+0.4, g+0.4 +knobstate, b+0.4);
		cairo_pattern_add_color_stop_rgb (pat, 0.3, r+0.15, g+0.15 + knobstate*0.5, b+0.15);
		cairo_pattern_add_color_stop_rgb (pat, 0.1, r, g, b);
	}
	cairo_set_source (cr, pat);
	cairo_stroke_preserve(cr);
	gdk_cairo_set_source_rgba(cr, &fg_color);
	cairo_set_line_width(cr, 0.5);
	cairo_stroke(cr);
	cairo_pattern_destroy (pat);
    
    char s[64];
    print_value(G_OBJECT(adj),s);
    cairo_set_source_rgba (cr, 0.8, 0.8, 0.2,0.6);
    cairo_set_font_size (cr, 11.0);
    cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_BOLD);
    if (order) {
        cairo_move_to (cr, knobx1+15., knoby1+5.);
    } else {
        cairo_move_to (cr, knobx1-50., knoby1+5.);
    }
    cairo_show_text(cr, s);
    g_free (allocation); 
}

/****************************************************************
 ** general expose events for all "knob" controllers
 */

//----------- draw the Knob when moved
static gboolean gtk_knob_expose (GtkWidget *widget, cairo_t *cr)
{
	g_assert(GTK_IS_KNOB(widget));
	GtkKnob *knob = GTK_KNOB(widget);
	GtkKnobPrivate *priv = GTK_KNOB_GET_PRIVATE(knob);
	knob_expose(widget, cr, priv->knob_x, priv->knob_y, 0);
	return FALSE;
}

/****************************************************************
 ** set value from key bindings
 */

static void gtk_knob_set_value (GtkWidget *widget, int dir_down)
{
	g_assert(GTK_IS_KNOB(widget));

	GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(widget));

	int oldstep = (int)(0.5f + (gtk_adjustment_get_value(adj) - gtk_adjustment_get_lower(adj)) / gtk_adjustment_get_step_increment(adj));
	int step;
	int nsteps = (int)(0.5f + (gtk_adjustment_get_upper(adj) - gtk_adjustment_get_lower(adj)) / gtk_adjustment_get_step_increment(adj));
	if (dir_down)
		step = oldstep - 1;
	else
		step = oldstep + 1;
	float value = gtk_adjustment_get_lower(adj) + step * (double)(gtk_adjustment_get_upper(adj) - gtk_adjustment_get_lower(adj)) / nsteps;
	gtk_widget_grab_focus(widget);
	gtk_range_set_value(GTK_RANGE(widget), value);
}

/****************************************************************
 ** keyboard bindings
 */

static gboolean gtk_knob_key_press (GtkWidget *widget, GdkEventKey *event)
{
	g_assert(GTK_IS_KNOB(widget));

	GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(widget));
	switch (event->keyval) {
	case GDK_KEY_Home:
		gtk_range_set_value(GTK_RANGE(widget), gtk_adjustment_get_lower(adj));
		return TRUE;
	case GDK_KEY_End:
		gtk_range_set_value(GTK_RANGE(widget), gtk_adjustment_get_upper(adj));
		return TRUE;
	case GDK_KEY_Up:
		gtk_knob_set_value(widget, 0);
		return TRUE;
	case GDK_KEY_Right:
		gtk_knob_set_value(widget, 0);
		return TRUE;
	case GDK_KEY_Down:
		gtk_knob_set_value(widget, 1);
		return TRUE;
	case GDK_KEY_Left:
		gtk_knob_set_value(widget, 1);
		return TRUE;
	}

	return FALSE;
}


/****************************************************************
 ** alternative (radial) knob motion mode (ctrl + mouse pressed)
 */

static void knob_pointer_event(GtkWidget *widget, gdouble x, gdouble y, int knob_x, int knob_y,
                               gboolean drag, int state)
{
	static double last_y = 2e20;
	GtkKnob *knob = GTK_KNOB(widget);
	GtkKnobPrivate *priv = GTK_KNOB_GET_PRIVATE(knob);
	GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(widget));
    GtkAllocation *allocation = g_new0 (GtkAllocation, 1);
    gtk_widget_get_allocation(GTK_WIDGET(widget), allocation); 
	double radius =  min(knob_x, knob_y) / 2;
	int  knobx = (allocation->width - knob_x) / 2;
	int  knoby = (allocation->height - knob_y) / 2;
	double posx = (knobx + radius) - x; // x axis right -> left
	double posy = (knoby + radius) - y; // y axis top -> bottom
	double value;
	if (!drag) {
		if (state & GDK_CONTROL_MASK) {
			last_y = 2e20;
			return;
		} else {
			last_y = posy;
		}
	}
	if (last_y < 1e20) { // in drag started with Control Key
		const double scaling = 0.005;
		double scal = (state & GDK_SHIFT_MASK ? scaling*0.1 : scaling);
		value = (last_y - posy) * scal;
		last_y = posy;
		gtk_range_set_value(GTK_RANGE(widget), gtk_adjustment_get_value(adj) - value * (gtk_adjustment_get_upper(adj) - gtk_adjustment_get_lower(adj)));
		return;
	}

	double angle = atan2(-posx, posy) + M_PI; // clockwise, zero at 6 o'clock, 0 .. 2*M_PI
	if (drag) {
		// block "forbidden zone" and direct moves between quadrant 1 and 4
		int quadrant = 1 + (int)(angle/M_PI_2);
		if (priv->last_quadrant == 1 && (quadrant == 3 || quadrant == 4)) {
			angle = scale_zero;
		} else if (priv->last_quadrant == 4 && (quadrant == 1 || quadrant == 2)) {
			angle = 2*M_PI - scale_zero;
		} else {
			if (angle < scale_zero) {
				angle = scale_zero;
			} else if (angle > 2*M_PI - scale_zero) {
				angle = 2*M_PI - scale_zero;
			}
			priv->last_quadrant = quadrant;
		}
	} else {
		if (angle < scale_zero) {
			angle = scale_zero;
		} else if (angle > 2*M_PI - scale_zero) {
			angle = 2*M_PI - scale_zero;
		}
		priv->last_quadrant = 0;
	}
	angle = (angle - scale_zero) / (2 * (M_PI-scale_zero)); // normalize to 0..1
	gtk_range_set_value(GTK_RANGE(widget), gtk_adjustment_get_lower(adj) + angle * (gtk_adjustment_get_upper(adj) - gtk_adjustment_get_lower(adj)));
    g_free (allocation); 
}

/****************************************************************
 ** mouse button pressed set value
 */

static gboolean gtk_knob_button_press (GtkWidget *widget, GdkEventButton *event)
{
	g_assert(GTK_IS_KNOB(widget));
	GtkKnob *knob = GTK_KNOB(widget);
	GtkKnobPrivate *priv = GTK_KNOB_GET_PRIVATE(knob);
	
	switch (event->button) {
	case 1:  // left button
		gtk_widget_grab_focus(widget);
		gtk_widget_grab_default (widget);
		gtk_grab_add(widget);
		priv->button_is = 1;
		knob_pointer_event(widget, event->x, event->y, priv->knob_x, priv->knob_y,
						   FALSE, event->state);
		break;
	case 2: //wheel
		priv->button_is = 2;
		break;
	case 3:  // right button 
		priv->button_is = 3;
		break;
	default: // do nothing
		break;
	}
	return TRUE;
}

/****************************************************************
 ** mouse button release
 */

static gboolean gtk_knob_button_release (GtkWidget *widget, GdkEventButton *event)
{
	g_assert(GTK_IS_KNOB(widget));
	GtkKnob *knob = GTK_KNOB(widget);
	GtkKnobPrivate *priv = GTK_KNOB_GET_PRIVATE(knob);
	priv->button_is = 0;
	if (gtk_widget_has_grab(widget))
		gtk_grab_remove(widget);
	return FALSE;
}

/****************************************************************
 ** set the value from mouse movement
 */

static gboolean gtk_knob_pointer_motion (GtkWidget *widget, GdkEventMotion *event)
{
	g_assert(GTK_IS_KNOB(widget));
	GtkKnob *knob = GTK_KNOB(widget);
	GtkKnobPrivate *priv = GTK_KNOB_GET_PRIVATE(knob);
	gdk_event_request_motions (event);
	
	if (gtk_widget_has_grab(widget)) {
		knob_pointer_event(widget, event->x, event->y, priv->knob_x, priv->knob_y,
						   TRUE, event->state);
	}
	return FALSE;
}

/****************************************************************
 ** set value from mouseweel
 */

static gboolean gtk_knob_scroll (GtkWidget *widget, GdkEventScroll *event)
{
	g_assert(GTK_IS_KNOB(widget));
	if(event->delta_y < 0)
		gtk_knob_set_value(widget, 0);
	else if (event->delta_y > 0)
		gtk_knob_set_value(widget, 1);
	return FALSE;
}

static void
gtk_knob_map (GtkWidget *widget)
{
	GTK_WIDGET_CLASS (gtk_knob_parent_class)->map (widget);
}

static void
gtk_knob_unmap (GtkWidget *widget)
{
	GTK_WIDGET_CLASS (gtk_knob_parent_class)->unmap (widget);
}

static void gtk_knob_get_preferred_size (GtkWidget *widget,
	GtkOrientation orientation, gint *minimal_size, gint *natural_size)
{
	if (orientation == GTK_ORIENTATION_HORIZONTAL)
	{
		*minimal_size = *natural_size = 62;
	}
	else
	{
		*minimal_size = *natural_size = 25;
	}
}

static void gtk_knob_get_preferred_width (
	GtkWidget *widget, gint *minimal_width, gint *natural_width)
{
	gtk_knob_get_preferred_size (widget,
		GTK_ORIENTATION_HORIZONTAL, minimal_width, natural_width);
}

static void gtk_knob_get_preferred_height (
	GtkWidget *widget, gint *minimal_height, gint *natural_height)
{
  gtk_knob_get_preferred_size (widget,
	GTK_ORIENTATION_VERTICAL, minimal_height, natural_height);
}


/****************************************************************
 ** init the GtkKnobClass
 */

static void gtk_knob_class_init (GtkKnobClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS (klass);

	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

//--------- connect the events with funktions
	widget_class->map = gtk_knob_map;
	widget_class->unmap = gtk_knob_unmap;
	widget_class->draw = gtk_knob_expose;
	widget_class->get_preferred_width = gtk_knob_get_preferred_width;
	widget_class->get_preferred_height = gtk_knob_get_preferred_height;
	widget_class->button_press_event = gtk_knob_button_press;
	widget_class->button_release_event = gtk_knob_button_release;
	widget_class->motion_notify_event = gtk_knob_pointer_motion;
	widget_class->key_press_event = gtk_knob_key_press;
	widget_class->scroll_event = gtk_knob_scroll;
	
	g_type_class_add_private(obj_class, sizeof (GtkKnobPrivate));
}

/****************************************************************
 ** init the Knob type/size
 */

static void gtk_knob_init (GtkKnob *knob)
{
	g_assert(GTK_IS_KNOB(knob));
	GtkWidget *widget = GTK_WIDGET(knob);
	GtkKnobPrivate *priv = GTK_KNOB_GET_PRIVATE(knob);
	priv->knob_x = 62;
	priv->knob_y = 25;
	priv->knob_step = 86;
	priv->button_is = 0;
    priv->order = 2;
	
	gtk_widget_set_can_focus (widget, true);
	gtk_widget_set_can_default(widget, true);
}

/****************************************************************
 ** redraw when value changed
 */

static gboolean gtk_knob_value_changed(gpointer obj)
{
	GtkWidget *widget = (GtkWidget *)obj;
	g_assert(GTK_IS_KNOB(widget));
	gtk_widget_queue_draw(widget);
	return FALSE;
}

/****************************************************************
 ** create small knob
 */

GtkWidget *gtk_knob_new_with_value_label(GtkAdjustment *_adjustment, int order)
{
	GtkWidget *widget = GTK_WIDGET( g_object_new (GTK_TYPE_KNOB, NULL ));
	GtkKnobPrivate *priv = GTK_KNOB_GET_PRIVATE(widget);
	priv->order = order;
 
	if (widget) {
		gtk_range_set_adjustment(GTK_RANGE(widget), _adjustment);
		g_signal_connect(G_OBJECT(widget), "value-changed",
		                 G_CALLBACK(gtk_knob_value_changed), widget);
	}
    return widget;
}
