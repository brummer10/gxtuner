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
 *        file: gxtuner.cpp      guitar tuner for jack
 *
 * ----------------------------------------------------------------------------
 */

#include "./gxtuner.h"


#include <string.h> 
#include <math.h>
#include <stdlib.h>
#define P_(s) (s)   // FIXME -> gettext

// Use propertys to set variables for the widget from outside,
// add new propertys here in the enum 

enum {
    PROP_FREQ = 1,
    PROP_REFERENCE_PITCH = 2,
    PROP_MODE = 3,
};

static gboolean gtk_tuner_expose (GtkWidget *widget, cairo_t *cr);
static void draw_background(cairo_surface_t *surface_tuner);
static void gx_tuner_class_init (GxTunerClass *klass);
static void gx_tuner_base_class_finalize(GxTunerClass *klass);
static void gx_tuner_init(GxTuner *tuner);
static void gx_tuner_set_property(
    GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void gx_tuner_get_property(
    GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

static const int tuner_width = 100;
static const int tuner_height = 60;
static const double rect_width = 100;
static const double rect_height = 60;

static int cents = 0;
static float mini_cents = 0.0;

static const double dashline[] = {
    3.0                
};

static const double dashes[] = {
    0.0,                      /* ink  */
    rect_height,              /* skip */
    10.0,                     /* ink  */
    10.0                      /* skip */
};

static const double dash_ind[] = {
    0,                         /* ink  */
    14,                        /* skip */
    rect_height-18,            /* ink  */
    100.0                      /* skip */
};

static const double no_dash[] = {
    1000,                      /* ink  */
    0,                         /* skip */
    1000,                      /* ink  */
    0                          /* skip */
};


static void gx_tuner_get_preferred_size (GtkWidget *widget,
	GtkOrientation orientation, gint *minimal_size, gint *natural_size)
{
	if (orientation == GTK_ORIENTATION_HORIZONTAL)
	{
		*minimal_size = *natural_size = 100;
	}
	else
	{
		*minimal_size = *natural_size = 60;
	}
}

static void gx_tuner_get_preferred_width (
	GtkWidget *widget, gint *minimal_width, gint *natural_width)
{
	gx_tuner_get_preferred_size (widget,
		GTK_ORIENTATION_HORIZONTAL, minimal_width, natural_width);
}

static void gx_tuner_get_preferred_height (
	GtkWidget *widget, gint *minimal_height, gint *natural_height)
{
  gx_tuner_get_preferred_size (widget,
	GTK_ORIENTATION_VERTICAL, minimal_height, natural_height);
}


GType gx_tuner_get_type(void) {
    static GType tuner_type = 0;

    if (!tuner_type) {
        const GTypeInfo tuner_info = {
            sizeof (GxTunerClass),
            NULL,                /* base_class_init */
            (GBaseFinalizeFunc) gx_tuner_base_class_finalize,
            (GClassInitFunc) gx_tuner_class_init,
            NULL,                /* class_finalize */
            NULL,                /* class_data */
            sizeof (GxTuner),
            0,                    /* n_preallocs */
            (GInstanceInitFunc) gx_tuner_init,
            NULL,                /* value_table */
        };
        tuner_type = g_type_register_static(
            GTK_TYPE_DRAWING_AREA, "GxTuner", &tuner_info, (GTypeFlags)0);
    }
    return tuner_type;
}

static void gx_tuner_class_init(GxTunerClass *klass) {
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->draw = gtk_tuner_expose;
	widget_class->get_preferred_width = gx_tuner_get_preferred_width;
	widget_class->get_preferred_height = gx_tuner_get_preferred_height;
	
	// here we setup get and set methodes for the propertys
    gobject_class->set_property = gx_tuner_set_property;
    gobject_class->get_property = gx_tuner_get_property;
    // here we install the propertys for the widget, add new 
    // preopertys here
    g_object_class_install_property(
        gobject_class, PROP_FREQ, g_param_spec_double (
            "freq", P_("Frequency"),
            P_("The frequency for which tuning is displayed"),
            0.0, 1000.0, 0.0, G_PARAM_READWRITE));
    g_object_class_install_property(
        gobject_class, PROP_REFERENCE_PITCH, g_param_spec_double (
            "reference-pitch", P_("Reference Pitch"),
            P_("The frequency for which tuning is displayed"),
            400.0, 500.0, 440.0, G_PARAM_READWRITE));
    g_object_class_install_property(
        gobject_class, PROP_MODE, g_param_spec_int (
            "mode", P_("Tuning Mode"),
            P_("The Mode for which tuning is displayed"),
            0, 1, 0, G_PARAM_READWRITE));
    klass->surface_tuner = cairo_image_surface_create(
        CAIRO_FORMAT_ARGB32, tuner_width*3., tuner_height*3.);
    g_assert(klass->surface_tuner != NULL);
    draw_background(klass->surface_tuner);
}

static void gx_tuner_base_class_finalize(GxTunerClass *klass) {
    if (klass->surface_tuner) {
        g_object_unref(klass->surface_tuner);
    }
}

static void gx_tuner_init (GxTuner *tuner) {
    // here we set all propertys to a default value
    g_assert(GX_IS_TUNER(tuner));
    tuner->freq = 0;
    tuner->reference_pitch = 440.0;
    tuner->mode = 0;
    tuner->scale_w = 1.;
    tuner->scale_h = 1.;
    //GtkWidget *widget = GTK_WIDGET(tuner);
}

// this are the function calls to set the propertys called by
//  gx_tuner_set_property, here we set the internal var to the 
// value of the property

void gx_tuner_set_freq(GxTuner *tuner, double freq) {
    g_assert(GX_IS_TUNER(tuner));
    if (tuner->freq != freq) {
        tuner->freq = freq;
        gtk_widget_queue_draw(GTK_WIDGET(tuner));
        g_object_notify(G_OBJECT(tuner), "freq");
    }
}

void gx_tuner_set_reference_pitch(GxTuner *tuner, double reference_pitch) {
    g_assert(GX_IS_TUNER(tuner));
    tuner->reference_pitch = reference_pitch;
    gtk_widget_queue_draw(GTK_WIDGET(tuner));
    g_object_notify(G_OBJECT(tuner), "reference-pitch");
}

void gx_tuner_set_mode(GxTuner *tuner, int mode) {
    g_assert(GX_IS_TUNER(tuner));
    tuner->mode = mode;
    gtk_widget_queue_draw(GTK_WIDGET(tuner));
    g_object_notify(G_OBJECT(tuner), "mode");
}

double gx_tuner_get_reference_pitch(GxTuner *tuner) {
    g_assert(GX_IS_TUNER(tuner));
    return tuner->reference_pitch;
}

GtkWidget *gx_tuner_new(void) {
    return (GtkWidget*)g_object_new(GX_TYPE_TUNER, NULL);
}

// here we set propertys, add new ones here

static void gx_tuner_set_property(GObject *object, guint prop_id,
                                      const GValue *value, GParamSpec *pspec) {
    GxTuner *tuner = GX_TUNER(object);

    switch(prop_id) {
    case PROP_FREQ:
        gx_tuner_set_freq(tuner, g_value_get_double(value));
        break;
    case PROP_REFERENCE_PITCH:
        gx_tuner_set_reference_pitch(tuner, g_value_get_double(value));
        break;
    case PROP_MODE:
        gx_tuner_set_mode(tuner, g_value_get_int(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

// the property get methode, return the current value

static void gx_tuner_get_property(GObject *object, guint prop_id,
                                      GValue *value, GParamSpec *pspec) {
    GxTuner *tuner = GX_TUNER(object);

    switch(prop_id) {
    case PROP_FREQ:
        g_value_set_double(value, tuner->freq);
        break;
    case PROP_REFERENCE_PITCH:
        g_value_set_double(value, tuner->reference_pitch);
        break;
    case PROP_MODE:
        g_value_set_int(value, tuner->mode);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static double log_scale(int cent, double set) {
    if (cent == 0) {
        return set;
    } else if (cent < 2 && cent > -2) { // 1 cent
        if (set<0.0) return set - 0.0155;
        else return set + 0.0155;
    } else if (cent < 3 && cent > -3) { // 2 cent
        if (set<0.0) return set - 0.0265;
        else return set + 0.0265;
    }else if (cent < 4 && cent > -4) { // 3 cent
        if (set<0.0) return set - 0.0355;
        else return set + 0.0355;
    } else if (cent < 5 && cent > -5) { // 4 cent
        if (set<0.0) return set - 0.0455;
        else return set + 0.0455;
    } else if (cent < 6 && cent > -6) { // 5 cent
        if (set<0.0) return set - 0.0435;
        else return set + 0.0435;
    } else if (cent < 7 && cent > -7) { // 6 cent
        if (set<0.0) return set - 0.0425;
        else return set + 0.0425;
    } else if (cent < 8 && cent > -8) { // 7 cent
        if (set<0.0) return set - 0.0415;
        else return set + 0.0415;
    } else if (cent < 9 && cent > -9) { // 8 cent
        if (set<0.0) return set - 0.0405;
        else return set + 0.0405;
    } else if (cent < 10 && cent > -10) { // 9 cent
        if (set<0.0) return set - 0.0385;
        else return set + 0.0385;
    } else if (cent < 11 && cent > -11) { // 10 cent
        if (set<0.0) return set - 0.0365;
        else return set + 0.0365;
    } else if (cent < 51 && cent > -51) { // < 50 cent
        return set + (0.4/cent);
    } else return set;
}

static void gx_tuner_triangle(cairo_t *cr, double posx, double posy, double width, double height)
{
	double h2 = height/2.0;
    cairo_move_to(cr, posx, posy-h2);
    if (width > 0) {
        cairo_curve_to(cr,posx, posy-h2, posx+10, posy, posx, posy+h2);
    } else {
        cairo_curve_to(cr,posx, posy-h2, posx-10, posy, posx, posy+h2);
    }
    cairo_curve_to(cr,posx, posy+h2, posx+width/2, posy+h2, posx+width, posy);
    cairo_curve_to(cr, posx+width, posy, posx+width/2, posy-h2, posx, posy-h2);
	cairo_fill(cr);
}

static void gx_tuner_strobe(cairo_t *cr, double x0, double y0, double cents) {
    static double move = 0;
    static double hold_l = 0;
    cairo_pattern_t *pat = cairo_pattern_create_linear (x0+50, y0,x0, y0);
    cairo_pattern_set_extend(pat, CAIRO_EXTEND_REFLECT);
    cairo_pattern_add_color_stop_rgb (pat, 0, 0.1, 0.8, 0.1);
    cairo_pattern_add_color_stop_rgb (pat, 0.1, 0.1, 0.6, 0.1);
    cairo_pattern_add_color_stop_rgb (pat, 0.2, 0.3, 0.6, 0.1);
    cairo_pattern_add_color_stop_rgb (pat, 0.3, 0.4, 0.4, 0.1);
    cairo_pattern_add_color_stop_rgb (pat, 1, 0.8, 0.1, 0.1);
    cairo_set_source (cr, pat);

    if (abs(cents)>0) {
        if(hold_l>0 )
            hold_l -= 10.0 ;
        if (cents>0)
            move += pow(abs(cents),0.25);
        else if (cents<0)
            move -= pow(abs(cents),0.25);
    } else if (fabs(cents)>0.015){
        move += cents;
        if(hold_l>0 )
            hold_l -= 10.0 ;
    } else {
        move = 0;
        if(hold_l<rect_width/2 )
            hold_l += 10.0 ;
        else if(hold_l<rect_width/2 +1.5)
            hold_l = rect_width/2+1.5 ;
    }
    if(move<0)
        move = rect_width;
    else if (move>rect_width)
        move = 0;

    cairo_set_dash (cr, dashline, sizeof(dashline)/sizeof(dashline[0]), move);
    cairo_set_line_width(cr, 2.0);
    cairo_move_to(cr,x0+hold_l, y0+1);
    cairo_line_to(cr, x0+rect_width-hold_l , y0+1);
    cairo_stroke(cr);   
    cairo_pattern_destroy(pat);

}

// copy the following block and edit it to your needs to add a new tuning mode
static gboolean gtk_tuner_expose_diatonic(GtkWidget *widget, cairo_t *cr) {
    // Note names for display
    static const char* diatonic_note[] = {"Do","Re","Mi","Fa","Sol","La ","Ti"};
    // ratios of notes + 2/1
    float noteratio[] = {1/1.0, 9/8.0, 5/4.0, 4/3.0, 3/2.0, 5/3.0, 15/8.0, 2/1.0};
    // Set which ratio is the reference pitch (note that the first ratio is 0 and the second ratio 1 and so on and so forth)
    float refratio = noteratio[5];
    // calculating the number of notes of the preset
    int numberofnotes = (sizeof(noteratio) / sizeof(noteratio[0]) - 1) ;
    // Frequency Octave divider 
    float multiply = 1.0;
    // ratio 
    float percent = 0.0;
    // Note indicator
    int display_note = 0;
    // Octave names for display
    static const char* octave[] = {"0","1","2","3","4","5","6","7"," "};
    // Octave indicator
    static int indicate_oc = 0;
    
    GxTuner *tuner = GX_TUNER(widget);
    // fetch widget size and location
    GtkAllocation *allocation = g_new0 (GtkAllocation, 1);
    gtk_widget_get_allocation(GTK_WIDGET(widget), allocation); 

    double x0      = (allocation->width - 100) * 0.5;
    double y0      = (allocation->height - 60) * 0.5;

    static double grow   = 0.;

    if(allocation->width > allocation->height +(10.*grow*3)) {
        grow = (allocation->height/60.)/10.;
    } else {
        grow =  (allocation->width/100.)/10.;
    }
    
    tuner->scale_h = (allocation->height/60.)/3.;
    tuner->scale_w =  (allocation->width/100.)/3.;
    // translate widget size to standard size
    cairo_translate(cr, -x0*tuner->scale_w, -y0*tuner->scale_h);
    cairo_scale(cr, tuner->scale_w, tuner->scale_h);
    cairo_set_source_surface(cr, GX_TUNER_CLASS(GTK_WIDGET_GET_CLASS(widget))->surface_tuner, x0, y0);
    cairo_paint (cr);
    cairo_restore(cr);

    cairo_save(cr);
    cairo_translate(cr, -x0*tuner->scale_w*3., -y0*tuner->scale_h*3.);
    cairo_scale(cr, tuner->scale_w*3., tuner->scale_h*3.);
    
    // fetch Octave we are in 
    float scale = -0.4;
    if (tuner->freq) {
        // this is the frequency we get from the pitch tracker
        float freq_is = tuner->freq;
        // Set reference frequency of the first note of the preset
        float ref_c = tuner->reference_pitch / refratio;
        // now check in which octave we are with the tracked frequency
        // and set the frequency octave divider
        // ref_c is now the frequency of the first note in octave, 
        // but we want to check if the frequency is below the last note in octave
        // so, for example if freq_is is below ref_c we are in octave 3
        // if freq_is is below ref_c/2 we are in octave 2, etc.
    for (int n=0 ; n <= 8 ; ++n )
         { float ratiodiffhighnoteandoctave = exp((log(noteratio[numberofnotes-1])+log(noteratio[numberofnotes]))/2) ;  
            if (freq_is < (ref_c*pow(2,n-3))-(2-ratiodiffhighnoteandoctave)*(ref_c*pow(2,n-3)) && freq_is >0.0) {
                 indicate_oc = n; 
                 multiply = pow(2, 4-n);
                 break;
                }
         }
    percent = (freq_is/(ref_c/multiply)) ;
    // now we chould check which ratio we have
    // we split the range using log-average
     for (int n=0 ; n <= numberofnotes ; ++n )
         { float ratiodiff = exp((log(noteratio[n])+log(noteratio[n+1]))/2) ;  
                 if (percent < ratiodiff) {
                     display_note = n;
                     scale = ((percent-noteratio[n]))/2.0;
                     break;
                 }
         }
   // fprintf(stderr, " percent == %f freq = %f ref_c = %f indicate_oc = %i \n value of numberofnotes is %i ", percent, freq_is, ref_c, indicate_oc, numberofnotes );   
        // display note
        cairo_set_source_rgba(cr, fabsf(scale)*3.0, 1-fabsf(scale)*3.0, 0.2,1-fabsf(scale)*2);
        cairo_set_font_size(cr, 18.0);
        cairo_move_to(cr,x0+50 -9 , y0+30 +9 );
        cairo_show_text(cr, diatonic_note[display_note]);
        cairo_set_font_size(cr, 8.0);
        cairo_move_to(cr,x0+54  , y0+30 +16 );
        cairo_show_text(cr, octave[indicate_oc]);
    }

    // display frequency
    char s[10];
    snprintf(s, sizeof(s), "%.1f Hz", tuner->freq);
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.1);
    cairo_set_font_size (cr, 7.5);
    cairo_text_extents_t ex;
    cairo_text_extents(cr, s, &ex);
    cairo_move_to (cr, x0+98-ex.width, y0+58);
    cairo_show_text(cr, s);
    // display cent
    if(scale>-0.4) {
        if(scale>0.004) {
            // here we translate the scale factor to cents and display them
            cents = static_cast<int>((floorf(scale * 10000) / 50));
            snprintf(s, sizeof(s), "+%i", cents);
            cairo_set_source_rgb (cr, 0.05, 0.5+0.022* abs(cents), 0.1);
            gx_tuner_triangle(cr, x0+80, y0+40, -15, 10);
            cairo_set_source_rgb (cr, 0.5+ 0.022* abs(cents), 0.35, 0.1);
            gx_tuner_triangle(cr, x0+20, y0+40, 15, 10);
            gx_tuner_strobe(cr, x0, y0, static_cast<double>(cents));
        } else if(scale<-0.004) {
            cents = static_cast<int>((ceil(scale * 10000) / 50));
            snprintf(s, sizeof(s), "%i", cents);
            cairo_set_source_rgb (cr, 0.05, 0.5+0.022* abs(cents), 0.1);
            gx_tuner_triangle(cr, x0+20, y0+40, 15, 10);
            cairo_set_source_rgb (cr, 0.5+ 0.022* abs(cents), 0.35, 0.1);
            gx_tuner_triangle(cr, x0+80, y0+40, -15, 10);
            gx_tuner_strobe(cr, x0, y0, static_cast<double>(cents));
        } else {
            cents = static_cast<int>((ceil(scale * 10000) / 50));
            mini_cents = (scale * 10000) / 50;
            if (mini_cents<0)
                snprintf(s, sizeof(s), "%.2f", mini_cents);
            else
                snprintf(s, sizeof(s), "+%.2f", mini_cents);
            cairo_set_source_rgb (cr, 0.05* abs(cents), 0.5, 0.1);
            gx_tuner_triangle(cr, x0+80, y0+40, -15, 10);
            gx_tuner_triangle(cr, x0+20, y0+40, 15, 10);
            gx_tuner_strobe(cr, x0, y0, mini_cents);
        }
    } else {
        cents = 100;
        snprintf(s, sizeof(s), "+ - cent");
    }    
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.1);
    cairo_set_font_size (cr, 6.0);
    cairo_text_extents(cr, s, &ex);
    cairo_move_to (cr, x0+28-ex.width, y0+58);
    cairo_show_text(cr, s);

    double ux=2., uy=2.;
    cairo_device_to_user_distance (cr, &ux, &uy);
    if (ux < uy)
        ux = uy;
    cairo_set_line_width (cr, ux + grow);

    // indicator (line)
    cairo_move_to(cr,x0+50, y0+rect_height+5);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_dash (cr, dash_ind, sizeof(dash_ind)/sizeof(dash_ind[0]), 1);
    cairo_line_to(cr, (log_scale(cents, scale)*2*rect_width)+x0+50, y0+(scale*scale*30)+2);
    cairo_set_source_rgb(cr,  0.5, 0.1, 0.1);
    cairo_stroke(cr);   

    g_free (allocation); 
    return FALSE;
}

static gboolean gtk_tuner_expose_shruti(GtkWidget *widget, cairo_t *cr) {
    static const char* shruti_note[22] = {"S ","r1","r2","R1","R2","g1","g2","G1","G2","M1","M2","m1","m2","P ","d1","d2","D1","D2","n1","n2","N1","N2"};
    float noteratio[] = {1/1.0, 256/243.0, 16/15.0, 10/9.0, 9/8.0, 32/27.0, 6/5.0, 81/64.0, 4/3.0, 27/20.0, 45/32.0, 729/512.0, 3/2.0, 128/81.0, 8/5.0, 5/3.0, 27/16.0, 16/9.0, 9/5.0, 15/8.0, 243/128.0, 2/1.0};
    float refratio = noteratio[0];
    int numberofnotes = (sizeof(noteratio) / sizeof(noteratio[0]) - 1) ;
    float multiply = 1.0;
    float percent = 0.0;
    int display_note = 0;
    static const char* octave[] = {"0","1","2","3","4","5","6","7"," "};
    static int indicate_oc = 0;
    
    GxTuner *tuner = GX_TUNER(widget);
    // fetch widget size and location
    GtkAllocation *allocation = g_new0 (GtkAllocation, 1);
    gtk_widget_get_allocation(GTK_WIDGET(widget), allocation); 

    double x0      = (allocation->width - 100) * 0.5;
    double y0      = (allocation->height - 60) * 0.5;

    static double grow   = 0.;

    if(allocation->width > allocation->height +(10.*grow*3)) {
        grow = (allocation->height/60.)/10.;
    } else {
        grow =  (allocation->width/100.)/10.;
    }
    
    tuner->scale_h = (allocation->height/60.)/3.;
    tuner->scale_w =  (allocation->width/100.)/3.;
    // translate widget size to standard size
    cairo_translate(cr, -x0*tuner->scale_w, -y0*tuner->scale_h);
    cairo_scale(cr, tuner->scale_w, tuner->scale_h);
    cairo_set_source_surface(cr, GX_TUNER_CLASS(GTK_WIDGET_GET_CLASS(widget))->surface_tuner, x0, y0);
    cairo_paint (cr);
    cairo_restore(cr);

    cairo_save(cr);
    cairo_translate(cr, -x0*tuner->scale_w*3., -y0*tuner->scale_h*3.);
    cairo_scale(cr, tuner->scale_w*3., tuner->scale_h*3.);
        
    
    float scale = -0.4;
    if (tuner->freq) {
    
        float freq_is = tuner->freq;
        float ref_c = tuner->reference_pitch / refratio;
        for (int n=0 ; n <= 8 ; ++n )
         { float ratiodiffhighnoteandoctave = exp((log(noteratio[numberofnotes-1])+log(noteratio[numberofnotes]))/2) ;  
            if (freq_is < (ref_c*pow(2,n-3))-(2-ratiodiffhighnoteandoctave)*(ref_c*pow(2,n-3)) && freq_is >0.0) {
                 indicate_oc = n; 
                 multiply = pow(2, 4-n);
                 break;
                }
         }
    percent = (freq_is/(ref_c/multiply)) ;
     for (int n=0 ; n <= numberofnotes ; ++n )
         { float ratiodiff = exp((log(noteratio[n])+log(noteratio[n+1]))/2) ;  
                 if (percent < ratiodiff) {
                     display_note = n;
                     scale = ((percent-noteratio[n]))/2.0;
                     break;
                 }
         }
        cairo_set_source_rgba(cr, fabsf(scale)*3.0, 1-fabsf(scale)*3.0, 0.2,1-fabsf(scale)*2);
        cairo_set_font_size(cr, 18.0);
        cairo_move_to(cr,x0+50 -9 , y0+30 +9 );
        cairo_show_text(cr, shruti_note[display_note]);
        cairo_set_font_size(cr, 8.0);
        cairo_move_to(cr,x0+54  , y0+30 +16 );
        cairo_show_text(cr, octave[indicate_oc]);
    }
    char s[10];
    snprintf(s, sizeof(s), "%.1f Hz", tuner->freq);
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.1);
    cairo_set_font_size (cr, 7.5);
    cairo_text_extents_t ex;
    cairo_text_extents(cr, s, &ex);
    cairo_move_to (cr, x0+98-ex.width, y0+58);
    cairo_show_text(cr, s);
    if(scale>-0.4) {
        if(scale>0.004) {
            cents = static_cast<int>((floorf(scale * 10000) / 50));
            snprintf(s, sizeof(s), "+%i", cents);
            cairo_set_source_rgb (cr, 0.05, 0.5+0.022* abs(cents), 0.1);
            gx_tuner_triangle(cr, x0+80, y0+40, -15, 10);
            cairo_set_source_rgb (cr, 0.5+ 0.022* abs(cents), 0.35, 0.1);
            gx_tuner_triangle(cr, x0+20, y0+40, 15, 10);
            gx_tuner_strobe(cr, x0, y0, static_cast<double>(cents));
        } else if(scale<-0.004) {
            cents = static_cast<int>((ceil(scale * 10000) / 50));
            snprintf(s, sizeof(s), "%i", cents);
            cairo_set_source_rgb (cr, 0.05, 0.5+0.022* abs(cents), 0.1);
            gx_tuner_triangle(cr, x0+20, y0+40, 15, 10);
            cairo_set_source_rgb (cr, 0.5+ 0.022* abs(cents), 0.35, 0.1);
            gx_tuner_triangle(cr, x0+80, y0+40, -15, 10);
            gx_tuner_strobe(cr, x0, y0, static_cast<double>(cents));
        } else {
            cents = static_cast<int>((ceil(scale * 10000) / 50));
            mini_cents = (scale * 10000) / 50;
            if (mini_cents<0)
                snprintf(s, sizeof(s), "%.2f", mini_cents);
            else
                snprintf(s, sizeof(s), "+%.2f", mini_cents);
            cairo_set_source_rgb (cr, 0.05* abs(cents), 0.5, 0.1);
            gx_tuner_triangle(cr, x0+80, y0+40, -15, 10);
            gx_tuner_triangle(cr, x0+20, y0+40, 15, 10);
            gx_tuner_strobe(cr, x0, y0, mini_cents);
        }
    } else {
        cents = 100;
        snprintf(s, sizeof(s), "+ - cent");
    }    
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.1);
    cairo_set_font_size (cr, 6.0);
    cairo_text_extents(cr, s, &ex);
    cairo_move_to (cr, x0+28-ex.width, y0+58);
    cairo_show_text(cr, s);

    double ux=2., uy=2.;
    cairo_device_to_user_distance (cr, &ux, &uy);
    if (ux < uy)
        ux = uy;
    cairo_set_line_width (cr, ux + grow);

    // indicator (line)
    cairo_move_to(cr,x0+50, y0+rect_height+5);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_dash (cr, dash_ind, sizeof(dash_ind)/sizeof(dash_ind[0]), 1);
    cairo_line_to(cr, (log_scale(cents, scale)*2*rect_width)+x0+50, y0+(scale*scale*30)+2);
    cairo_set_source_rgb(cr,  0.5, 0.1, 0.1);
    cairo_stroke(cr);   

    g_free (allocation); 
    return FALSE;
}

static gboolean gtk_tuner_expose_johnston5limit(GtkWidget *widget, cairo_t *cr) {
    static const char* johnston5limit_note[] = {"C","C+","D♭♭-","B♯♯+","C♯","C♯+","D♭-","D♭","E♭♭♭-","C♯♯+","D-","D","E♭♭-","E♭♭","F♭♭♭-","D♯","E♭-","E♭","E♭+","F♭♭","D♯♯+","E","E+","F♭","D♯♯♯+","E♯","E♯+","F","F+","G♭♭-","E♯♯+","F♯","F♯+","G♭-","G♭","A♭♭♭-","F♯♯+","G-","G","G+","A♭♭","G♯","A♭-","A♭","F♯♯♯♯++","G♯♯","G♯♯+","F♯♯♯++","A","A+","B♭♭-","G♯♯♯+","A♯","A♯+","B♭-","B♭","C♭♭-","C♭♭","A♯♯++","B","C♭-","C♭","D♭♭♭-","B♯","C-"};
    float noteratio[] = {1/1.0, 81/80.0, 128/125.0, 16875/16364.0, 25/24.0, 135/128.0, 16/15.0, 27/25.0, 2048/1875.0, 1125/1024.0, 10/9.0, 9/8.0, 256/225.0, 144/125.0, 32768/28125.0, 75/64.0, 32/27.0, 6/5.0, 243/200.0, 768/625.0, 10125/8192.0, 5/4.0, 81/64.0, 32/25.0, 84375/65536.0, 125/96.0, 675/512.0, 4/3.0, 27/20.0, 512/375.0, 5625/4096.0, 25/18.0, 45/32.0, 64/45.0, 36/25.0, 8192/5625.0, 375/256.0, 40/27.0, 3/2.0, 243/160.0, 192/125.0, 25/16.0, 128/81.0, 8/5.0, 421875/262144.0, 625/384.0, 3375/2048.0, 54375/32768.0, 5/3.0, 27/16.0, 128/75.0, 28125/16385.0, 125/72.0, 225/128.0, 16/9.0, 9/5.0, 2048/1125.0, 1152/625.0, 30375/16384.0, 15/8.0, 258/135.0, 48/25.0, 32768/16875.0, 125/64.0, 160/81.0, 2/1.0};
    float refratio = noteratio[48];
    int numberofnotes = (sizeof(noteratio) / sizeof(noteratio[0]) - 1) ;
    float multiply = 1.0;
    float percent = 0.0;
    int display_note = 0;
    static const char* octave[] = {"0","1","2","3","4","5","6","7"," "};
    static int indicate_oc = 0;
    
    GxTuner *tuner = GX_TUNER(widget);
    // fetch widget size and location
    GtkAllocation *allocation = g_new0 (GtkAllocation, 1);
    gtk_widget_get_allocation(GTK_WIDGET(widget), allocation); 

    double x0      = (allocation->width - 100) * 0.5;
    double y0      = (allocation->height - 60) * 0.5;

    static double grow   = 0.;

    if(allocation->width > allocation->height +(10.*grow*3)) {
        grow = (allocation->height/60.)/10.;
    } else {
        grow =  (allocation->width/100.)/10.;
    }
    
    tuner->scale_h = (allocation->height/60.)/3.;
    tuner->scale_w =  (allocation->width/100.)/3.;
    // translate widget size to standard size
    cairo_translate(cr, -x0*tuner->scale_w, -y0*tuner->scale_h);
    cairo_scale(cr, tuner->scale_w, tuner->scale_h);
    cairo_set_source_surface(cr, GX_TUNER_CLASS(GTK_WIDGET_GET_CLASS(widget))->surface_tuner, x0, y0);
    cairo_paint (cr);
    cairo_restore(cr);

    cairo_save(cr);
    cairo_translate(cr, -x0*tuner->scale_w*3., -y0*tuner->scale_h*3.);
    cairo_scale(cr, tuner->scale_w*3., tuner->scale_h*3.);
        
    float scale = -0.4;
    if (tuner->freq) {
        float freq_is = tuner->freq;
        float ref_c = tuner->reference_pitch / refratio;
        for (int n=0 ; n <= 8 ; ++n )
         { float ratiodiffhighnoteandoctave = exp((log(noteratio[numberofnotes-1])+log(noteratio[numberofnotes]))/2) ;  
            if (freq_is < (ref_c*pow(2,n-3))-(2-ratiodiffhighnoteandoctave)*(ref_c*pow(2,n-3)) && freq_is >0.0) {
                 indicate_oc = n; 
                 multiply = pow(2, 4-n);
                 break;
                }
         }
    percent = (freq_is/(ref_c/multiply)) ;
     for (int n=0 ; n <= numberofnotes ; ++n )
         { float ratiodiff = exp((log(noteratio[n])+log(noteratio[n+1]))/2) ;  
                 if (percent < ratiodiff) {
                     display_note = n;
                     scale = ((percent-noteratio[n]))/2.0;
                     break;
                 }
         }
        cairo_set_source_rgba(cr, fabsf(scale)*3.0, 1-fabsf(scale)*3.0, 0.2,1-fabsf(scale)*2);
        cairo_set_font_size(cr, 18.0);
        cairo_move_to(cr,x0+50 -9 , y0+30 +9 );
        cairo_show_text(cr, johnston5limit_note[display_note]);
        cairo_set_font_size(cr, 8.0);
        cairo_move_to(cr,x0+54  , y0+30 +16 );
        cairo_show_text(cr, octave[indicate_oc]);
    }

    // display frequency
    char s[10];
    snprintf(s, sizeof(s), "%.1f Hz", tuner->freq);
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.1);
    cairo_set_font_size (cr, 7.5);
    cairo_text_extents_t ex;
    cairo_text_extents(cr, s, &ex);
    cairo_move_to (cr, x0+98-ex.width, y0+58);
    cairo_show_text(cr, s);
    // display cent
    if(scale>-0.4) {
        if(scale>0.004) {
            // here we translate the scale factor to cents and display them
            cents = static_cast<int>((floorf(scale * 10000) / 50));
            snprintf(s, sizeof(s), "+%i", cents);
            cairo_set_source_rgb (cr, 0.05, 0.5+0.022* abs(cents), 0.1);
            gx_tuner_triangle(cr, x0+80, y0+40, -15, 10);
            cairo_set_source_rgb (cr, 0.5+ 0.022* abs(cents), 0.35, 0.1);
            gx_tuner_triangle(cr, x0+20, y0+40, 15, 10);
            gx_tuner_strobe(cr, x0, y0, static_cast<double>(cents));
        } else if(scale<-0.004) {
            cents = static_cast<int>((ceil(scale * 10000) / 50));
            snprintf(s, sizeof(s), "%i", cents);
            cairo_set_source_rgb (cr, 0.05, 0.5+0.022* abs(cents), 0.1);
            gx_tuner_triangle(cr, x0+20, y0+40, 15, 10);
            cairo_set_source_rgb (cr, 0.5+ 0.022* abs(cents), 0.35, 0.1);
            gx_tuner_triangle(cr, x0+80, y0+40, -15, 10);
            gx_tuner_strobe(cr, x0, y0, static_cast<double>(cents));
        } else {
            cents = static_cast<int>((ceil(scale * 10000) / 50));
            mini_cents = (scale * 10000) / 50;
            if (mini_cents<0)
                snprintf(s, sizeof(s), "%.2f", mini_cents);
            else
                snprintf(s, sizeof(s), "+%.2f", mini_cents);
            cairo_set_source_rgb (cr, 0.05* abs(cents), 0.5, 0.1);
            gx_tuner_triangle(cr, x0+80, y0+40, -15, 10);
            gx_tuner_triangle(cr, x0+20, y0+40, 15, 10);
            gx_tuner_strobe(cr, x0, y0, mini_cents);
        }
    } else {
        cents = 100;
        snprintf(s, sizeof(s), "+ - cent");
    }    
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.1);
    cairo_set_font_size (cr, 6.0);
    cairo_text_extents(cr, s, &ex);
    cairo_move_to (cr, x0+28-ex.width, y0+58);
    cairo_show_text(cr, s);

    double ux=2., uy=2.;
    cairo_device_to_user_distance (cr, &ux, &uy);
    if (ux < uy)
        ux = uy;
    cairo_set_line_width (cr, ux + grow);

    // indicator (line)
    cairo_move_to(cr,x0+50, y0+rect_height+5);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_dash (cr, dash_ind, sizeof(dash_ind)/sizeof(dash_ind[0]), 1);
    cairo_line_to(cr, (log_scale(cents, scale)*2*rect_width)+x0+50, y0+(scale*scale*30)+2);
    cairo_set_source_rgb(cr,  0.5, 0.1, 0.1);
    cairo_stroke(cr);   

    g_free (allocation); 
    return FALSE;
}

static gboolean gtk_tuner_expose_johnston7limit(GtkWidget *widget, cairo_t *cr) {
    // ♯ ♭
    static const char* johnston7limit_note[] = {"C","D7♭-","D♭-","D-","DL-","E7♭","E♭", "E","EL","F7+","F+","F♯+","F♯L+","G","A7♭","A♭","A","AL","B7♭","B♭","B","BL"};
    float noteratio[] = {1/1.0, 28/27.0, 16/15.0, 10/9.0, 8/7.0, 7/6.0, 6/5.0, 5/4.0, 9/7.0, 21/16.0, 27/20.0, 45/32.0, 81/56.0, 3/2.0, 14/9.0, 8/5.0, 5/3.0, 12/7.0, 7/4.0, 9/5.0, 15/8.0, 27/14.0, 2/1.0};
    // Set which ratio is the reference pitch (note that the first ratio is 0 and the second ratio 1 and so on and so forth)
    float refratio = noteratio[16];
    // calculating the number of notes of the preset
    int numberofnotes = (sizeof(noteratio) / sizeof(noteratio[0]) - 1) ;
    // Frequency Octave divider 
    float multiply = 1.0;
    // ratio 
    float percent = 0.0;
    // Note indicator
    int display_note = 0;
    // Octave names for display
    static const char* octave[] = {"0","1","2","3","4","5","6","7"," "};
    // Octave indicator
    static int indicate_oc = 0;
    
    GxTuner *tuner = GX_TUNER(widget);
    // fetch widget size and location
    GtkAllocation *allocation = g_new0 (GtkAllocation, 1);
    gtk_widget_get_allocation(GTK_WIDGET(widget), allocation); 

    double x0      = (allocation->width - 100) * 0.5;
    double y0      = (allocation->height - 60) * 0.5;

    static double grow   = 0.;

    if(allocation->width > allocation->height +(10.*grow*3)) {
        grow = (allocation->height/60.)/10.;
    } else {
        grow =  (allocation->width/100.)/10.;
    }
    
    tuner->scale_h = (allocation->height/60.)/3.;
    tuner->scale_w =  (allocation->width/100.)/3.;
    // translate widget size to standard size
    cairo_translate(cr, -x0*tuner->scale_w, -y0*tuner->scale_h);
    cairo_scale(cr, tuner->scale_w, tuner->scale_h);
    cairo_set_source_surface(cr, GX_TUNER_CLASS(GTK_WIDGET_GET_CLASS(widget))->surface_tuner, x0, y0);
    cairo_paint (cr);
    cairo_restore(cr);

    cairo_save(cr);
    cairo_translate(cr, -x0*tuner->scale_w*3., -y0*tuner->scale_h*3.);
    cairo_scale(cr, tuner->scale_w*3., tuner->scale_h*3.);
    
    // fetch Octave we are in 
    float scale = -0.4;
    if (tuner->freq) {
        // this is the frequency we get from the pitch tracker
        float freq_is = tuner->freq;
        // Set reference frequency of the first note of the preset
        float ref_c = tuner->reference_pitch / refratio;
        // now check in which octave we are with the tracked frequency
        // and set the frequency octave divider
        // ref_c is now the frequency of the first note in octave, 
        // but we want to check if the frequency is below the last note in octave
        // so, for example if freq_is is below ref_c we are in octave 3
        // if freq_is is below ref_c/2 we are in octave 2, etc.
    for (int n=0 ; n <= 8 ; ++n )
         { float ratiodiffhighnoteandoctave = exp((log(noteratio[numberofnotes-1])+log(noteratio[numberofnotes]))/2) ;  
            if (freq_is < (ref_c*pow(2,n-3))-(2-ratiodiffhighnoteandoctave)*(ref_c*pow(2,n-3)) && freq_is >0.0) {
                 indicate_oc = n; 
                 multiply = pow(2, 4-n);
                 break;
                }
         }
    percent = (freq_is/(ref_c/multiply)) ;
    // now we chould check which ratio we have
    // we split the range using log-average
     for (int n=0 ; n <= numberofnotes ; ++n )
         { float ratiodiff = exp((log(noteratio[n])+log(noteratio[n+1]))/2) ;  
                 if (percent < ratiodiff) {
                     display_note = n;
                     scale = ((percent-noteratio[n]))/2.0;
                     break;
                 }
         }
    //fprintf(stderr, " percent == %f freq = %f ref_c = %f indicate_oc = %i \n value of numberofnotes is %i ", percent, freq_is, ref_c, indicate_oc, numberofnotes );   
        // display note
        cairo_set_source_rgba(cr, fabsf(scale)*3.0, 1-fabsf(scale)*3.0, 0.2,1-fabsf(scale)*2);
        cairo_set_font_size(cr, 18.0);
        cairo_move_to(cr,x0+50 -9 , y0+30 +9 );
        cairo_show_text(cr, johnston7limit_note[display_note]);
        cairo_set_font_size(cr, 8.0);
        cairo_move_to(cr,x0+54  , y0+30 +16 );
        cairo_show_text(cr, octave[indicate_oc]);
    }

    // display frequency
    char s[10];
    snprintf(s, sizeof(s), "%.1f Hz", tuner->freq);
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.1);
    cairo_set_font_size (cr, 7.5);
    cairo_text_extents_t ex;
    cairo_text_extents(cr, s, &ex);
    cairo_move_to (cr, x0+98-ex.width, y0+58);
    cairo_show_text(cr, s);
    // display cent
    if(scale>-0.4) {
        if(scale>0.004) {
            // here we translate the scale factor to cents and display them
            cents = static_cast<int>((floorf(scale * 10000) / 50));
            snprintf(s, sizeof(s), "+%i", cents);
            cairo_set_source_rgb (cr, 0.05, 0.5+0.022* abs(cents), 0.1);
            gx_tuner_triangle(cr, x0+80, y0+40, -15, 10);
            cairo_set_source_rgb (cr, 0.5+ 0.022* abs(cents), 0.35, 0.1);
            gx_tuner_triangle(cr, x0+20, y0+40, 15, 10);
            gx_tuner_strobe(cr, x0, y0, static_cast<double>(cents));
        } else if(scale<-0.004) {
            cents = static_cast<int>((ceil(scale * 10000) / 50));
            snprintf(s, sizeof(s), "%i", cents);
            cairo_set_source_rgb (cr, 0.05, 0.5+0.022* abs(cents), 0.1);
            gx_tuner_triangle(cr, x0+20, y0+40, 15, 10);
            cairo_set_source_rgb (cr, 0.5+ 0.022* abs(cents), 0.35, 0.1);
            gx_tuner_triangle(cr, x0+80, y0+40, -15, 10);
            gx_tuner_strobe(cr, x0, y0, static_cast<double>(cents));
        } else {
            cents = static_cast<int>((ceil(scale * 10000) / 50));
            mini_cents = (scale * 10000) / 50;
            if (mini_cents<0)
                snprintf(s, sizeof(s), "%.2f", mini_cents);
            else
                snprintf(s, sizeof(s), "+%.2f", mini_cents);
            cairo_set_source_rgb (cr, 0.05* abs(cents), 0.5, 0.1);
            gx_tuner_triangle(cr, x0+80, y0+40, -15, 10);
            gx_tuner_triangle(cr, x0+20, y0+40, 15, 10);
            gx_tuner_strobe(cr, x0, y0, mini_cents);
        }
    } else {
        cents = 100;
        snprintf(s, sizeof(s), "+ - cent");
    }    
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.1);
    cairo_set_font_size (cr, 6.0);
    cairo_text_extents(cr, s, &ex);
    cairo_move_to (cr, x0+28-ex.width, y0+58);
    cairo_show_text(cr, s);

    double ux=2., uy=2.;
    cairo_device_to_user_distance (cr, &ux, &uy);
    if (ux < uy)
        ux = uy;
    cairo_set_line_width (cr, ux + grow);

    // indicator (line)
    cairo_move_to(cr,x0+50, y0+rect_height+5);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_dash (cr, dash_ind, sizeof(dash_ind)/sizeof(dash_ind[0]), 1);
    cairo_line_to(cr, (log_scale(cents, scale)*2*rect_width)+x0+50, y0+(scale*scale*30)+2);
    cairo_set_source_rgb(cr,  0.5, 0.1, 0.1);
    cairo_stroke(cr);   

    g_free (allocation); 
    return FALSE;
}


static gboolean gtk_tuner_expose_johnston7limitno5(GtkWidget *widget, cairo_t *cr) {
    // ♯ ♭
    static const char* johnston7limitn05_note[] = {"C","D7♭","D","E7♭","EL","F","F♯LL","G","A7♭","AL","B7♭","BL"};
    float noteratio[] = {1/1.0, 49/48.0, 9/8.0, 7/6.0, 9/7.0, 4/3.0, 72/49.0, 3/2.0, 14/9.0, 12/7.0, 7/4.0, 27/14.0, 2/1.0};
    // Set which ratio is the reference pitch (note that the first ratio is 0 and the second ratio 1 and so on and so forth)
    float refratio = 5/3.0;
    // calculating the number of notes of the preset
    int numberofnotes = (sizeof(noteratio) / sizeof(noteratio[0]) - 1) ;
    // Frequency Octave divider 
    float multiply = 1.0;
    // ratio 
    float percent = 0.0;
    // Note indicator
    int display_note = 0;
    // Octave names for display
    static const char* octave[] = {"0","1","2","3","4","5","6","7"," "};
    // Octave indicator
    static int indicate_oc = 0;
    
    GxTuner *tuner = GX_TUNER(widget);
    // fetch widget size and location
    GtkAllocation *allocation = g_new0 (GtkAllocation, 1);
    gtk_widget_get_allocation(GTK_WIDGET(widget), allocation); 

    double x0      = (allocation->width - 100) * 0.5;
    double y0      = (allocation->height - 60) * 0.5;

    static double grow   = 0.;

    if(allocation->width > allocation->height +(10.*grow*3)) {
        grow = (allocation->height/60.)/10.;
    } else {
        grow =  (allocation->width/100.)/10.;
    }
    
    tuner->scale_h = (allocation->height/60.)/3.;
    tuner->scale_w =  (allocation->width/100.)/3.;
    // translate widget size to standard size
    cairo_translate(cr, -x0*tuner->scale_w, -y0*tuner->scale_h);
    cairo_scale(cr, tuner->scale_w, tuner->scale_h);
    cairo_set_source_surface(cr, GX_TUNER_CLASS(GTK_WIDGET_GET_CLASS(widget))->surface_tuner, x0, y0);
    cairo_paint (cr);
    cairo_restore(cr);

    cairo_save(cr);
    cairo_translate(cr, -x0*tuner->scale_w*3., -y0*tuner->scale_h*3.);
    cairo_scale(cr, tuner->scale_w*3., tuner->scale_h*3.);
    
    // fetch Octave we are in 
    float scale = -0.4;
    if (tuner->freq) {
        // this is the frequency we get from the pitch tracker
        float freq_is = tuner->freq;
        // Set reference frequency of the first note of the preset
        float ref_c = tuner->reference_pitch / refratio;
        // now check in which octave we are with the tracked frequency
        // and set the frequency octave divider
        // ref_c is now the frequency of the first note in octave, 
        // but we want to check if the frequency is below the last note in octave
        // so, for example if freq_is is below ref_c we are in octave 3
        // if freq_is is below ref_c/2 we are in octave 2, etc.
    for (int n=0 ; n <= 8 ; ++n )
         { float ratiodiffhighnoteandoctave = exp((log(noteratio[numberofnotes-1])+log(noteratio[numberofnotes]))/2) ;  
            if (freq_is < (ref_c*pow(2,n-3))-(2-ratiodiffhighnoteandoctave)*(ref_c*pow(2,n-3)) && freq_is >0.0) {
                 indicate_oc = n; 
                 multiply = pow(2, 4-n);
                 break;
                }
         }
    percent = (freq_is/(ref_c/multiply)) ;
    // now we chould check which ratio we have
    // we split the range using log-average
     for (int n=0 ; n <= numberofnotes ; ++n )
         { float ratiodiff = exp((log(noteratio[n])+log(noteratio[n+1]))/2) ;  
                 if (percent < ratiodiff) {
                     display_note = n;
                     scale = ((percent-noteratio[n]))/2.0;
                     break;
                 }
         }
    //fprintf(stderr, " percent == %f freq = %f ref_c = %f indicate_oc = %i \n value of numberofnotes is %i ", percent, freq_is, ref_c, indicate_oc, numberofnotes );   
        // display note
        cairo_set_source_rgba(cr, fabsf(scale)*3.0, 1-fabsf(scale)*3.0, 0.2,1-fabsf(scale)*2);
        cairo_set_font_size(cr, 18.0);
        cairo_move_to(cr,x0+50 -9 , y0+30 +9 );
        cairo_show_text(cr, johnston7limitn05_note[display_note]);
        cairo_set_font_size(cr, 8.0);
        cairo_move_to(cr,x0+54  , y0+30 +16 );
        cairo_show_text(cr, octave[indicate_oc]);
    }

    // display frequency
    char s[10];
    snprintf(s, sizeof(s), "%.1f Hz", tuner->freq);
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.1);
    cairo_set_font_size (cr, 7.5);
    cairo_text_extents_t ex;
    cairo_text_extents(cr, s, &ex);
    cairo_move_to (cr, x0+98-ex.width, y0+58);
    cairo_show_text(cr, s);
    // display cent
    if(scale>-0.4) {
        if(scale>0.004) {
            // here we translate the scale factor to cents and display them
            cents = static_cast<int>((floorf(scale * 10000) / 50));
            snprintf(s, sizeof(s), "+%i", cents);
            cairo_set_source_rgb (cr, 0.05, 0.5+0.022* abs(cents), 0.1);
            gx_tuner_triangle(cr, x0+80, y0+40, -15, 10);
            cairo_set_source_rgb (cr, 0.5+ 0.022* abs(cents), 0.35, 0.1);
            gx_tuner_triangle(cr, x0+20, y0+40, 15, 10);
            gx_tuner_strobe(cr, x0, y0, static_cast<double>(cents));
        } else if(scale<-0.004) {
            cents = static_cast<int>((ceil(scale * 10000) / 50));
            snprintf(s, sizeof(s), "%i", cents);
            cairo_set_source_rgb (cr, 0.05, 0.5+0.022* abs(cents), 0.1);
            gx_tuner_triangle(cr, x0+20, y0+40, 15, 10);
            cairo_set_source_rgb (cr, 0.5+ 0.022* abs(cents), 0.35, 0.1);
            gx_tuner_triangle(cr, x0+80, y0+40, -15, 10);
            gx_tuner_strobe(cr, x0, y0, static_cast<double>(cents));
        } else {
            cents = static_cast<int>((ceil(scale * 10000) / 50));
            mini_cents = (scale * 10000) / 50;
            if (mini_cents<0)
                snprintf(s, sizeof(s), "%.2f", mini_cents);
            else
                snprintf(s, sizeof(s), "+%.2f", mini_cents);
            cairo_set_source_rgb (cr, 0.05* abs(cents), 0.5, 0.1);
            gx_tuner_triangle(cr, x0+80, y0+40, -15, 10);
            gx_tuner_triangle(cr, x0+20, y0+40, 15, 10);
            gx_tuner_strobe(cr, x0, y0, mini_cents);
        }
    } else {
        cents = 100;
        snprintf(s, sizeof(s), "+ - cent");
    }    
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.1);
    cairo_set_font_size (cr, 6.0);
    cairo_text_extents(cr, s, &ex);
    cairo_move_to (cr, x0+28-ex.width, y0+58);
    cairo_show_text(cr, s);

    double ux=2., uy=2.;
    cairo_device_to_user_distance (cr, &ux, &uy);
    if (ux < uy)
        ux = uy;
    cairo_set_line_width (cr, ux + grow);

    // indicator (line)
    cairo_move_to(cr,x0+50, y0+rect_height+5);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_dash (cr, dash_ind, sizeof(dash_ind)/sizeof(dash_ind[0]), 1);
    cairo_line_to(cr, (log_scale(cents, scale)*2*rect_width)+x0+50, y0+(scale*scale*30)+2);
    cairo_set_source_rgb(cr,  0.5, 0.1, 0.1);
    cairo_stroke(cr);   

    g_free (allocation); 
    return FALSE;
}


static gboolean gtk_tuner_expose (GtkWidget *widget, cairo_t *cr) {
    GxTuner *tuner = GX_TUNER(widget);
    // here we check in which mode we are add your mode here.
    if (tuner->mode == 1) {
        if (!gtk_tuner_expose_shruti (widget, cr)) return FALSE;
    } else if (tuner->mode == 2) {
        if (!gtk_tuner_expose_diatonic (widget, cr)) return FALSE;
    } else if (tuner->mode == 3) {
        if (!gtk_tuner_expose_johnston5limit (widget, cr)) return FALSE;
    } else if (tuner->mode == 4) {
        if (!gtk_tuner_expose_johnston7limit (widget, cr)) return FALSE;
    } else if (tuner->mode == 5) {
        if (!gtk_tuner_expose_johnston7limitno5 (widget, cr)) return FALSE;
    }
    static const char* note[12] = {"A ","A#","B ","C ","C#","D ","D#","E ","F ","F#","G ","G#"};
    static const char* octave[9] = {"0","1","2","3","4","5","6","7"," "};
    static int indicate_oc = 0;
    
    GtkAllocation *allocation = g_new0 (GtkAllocation, 1);
    gtk_widget_get_allocation(GTK_WIDGET(widget), allocation); 

    double x0      = (allocation->width - 100) * 0.5;
    double y0      = (allocation->height - 60) * 0.5;

    static double grow   = 0.;

    if(allocation->width > allocation->height +(10.*grow*3)) {
        grow = (allocation->height/60.)/10.;
    } else {
        grow =  (allocation->width/100.)/10.;
    }
    
    tuner->scale_h = (allocation->height/60.)/3.;
    tuner->scale_w =  (allocation->width/100.)/3.;
    
    cairo_translate(cr, -x0*tuner->scale_w, -y0*tuner->scale_h);
    cairo_scale(cr, tuner->scale_w, tuner->scale_h);
    cairo_set_source_surface(cr, GX_TUNER_CLASS(GTK_WIDGET_GET_CLASS(widget))->surface_tuner, x0, y0);
    cairo_paint (cr);
    cairo_restore(cr);

    cairo_save(cr);
    cairo_translate(cr, -x0*tuner->scale_w*3., -y0*tuner->scale_h*3.);
    cairo_scale(cr, tuner->scale_w*3., tuner->scale_h*3.);
    
    float scale = -0.4;
    if (tuner->freq) {
        float freq_is = tuner->freq;
        float fvis = 12 * log2f(freq_is/tuner->reference_pitch);
        int vis = int(round(fvis));
        scale = (fvis-vis) / 2;
        vis = vis % 12;
        if (vis < 0) {
            vis += 12;
        }
        if (fabsf(scale) < 0.1) {
            if (freq_is < 31.78 && freq_is >0.0) {
                indicate_oc = 0;
            } else if (freq_is < 63.57) {
                indicate_oc = 1;
            } else if (freq_is < 127.14) {
                indicate_oc = 2;
            } else if (freq_is < 254.28) {
                indicate_oc = 3;
            } else if (freq_is < 509.44) {
                indicate_oc = 4;
            } else if (freq_is < 1017.35) {
                indicate_oc = 5;
            } else if (freq_is < 2034.26) {
                indicate_oc = 6;
            } else if (freq_is < 4068.54) {
                indicate_oc = 7;
            } else {
                indicate_oc = 8;
            }
        }else {
            indicate_oc = 8;
        }

        // display note
        cairo_set_source_rgba(cr, fabsf(scale)*3.0, 1-fabsf(scale)*3.0, 0.2,1-fabsf(scale)*2);
        cairo_set_font_size(cr, 18.0);
        cairo_move_to(cr,x0+50 -9 , y0+30 +9 );
        cairo_show_text(cr, note[vis]);
        cairo_set_font_size(cr, 8.0);
        cairo_move_to(cr,x0+54  , y0+30 +16 );
        cairo_show_text(cr, octave[indicate_oc]);
    }

    // display frequency
    char s[10];
    snprintf(s, sizeof(s), "%.1f Hz", tuner->freq);
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.1);
    cairo_set_font_size (cr, 7.5);
    cairo_text_extents_t ex;
    cairo_text_extents(cr, s, &ex);
    cairo_move_to (cr, x0+98-ex.width, y0+58);
    cairo_show_text(cr, s);
    // display cent
    if(scale>-0.4) {
        if(scale>0.004) {
            cents = static_cast<int>((floorf(scale * 10000) / 50));
            snprintf(s, sizeof(s), "+%i", cents);
            cairo_set_source_rgb (cr, 0.05, 0.5+0.022* abs(cents), 0.1);
            gx_tuner_triangle(cr, x0+80, y0+40, -15, 10);
            cairo_set_source_rgb (cr, 0.5+ 0.022* abs(cents), 0.35, 0.1);
            gx_tuner_triangle(cr, x0+20, y0+40, 15, 10);
            gx_tuner_strobe(cr, x0, y0, static_cast<double>(cents));
        } else if(scale<-0.004) {
            cents = static_cast<int>((ceil(scale * 10000) / 50));
            snprintf(s, sizeof(s), "%i", cents);
            cairo_set_source_rgb (cr, 0.05, 0.5+0.022* abs(cents), 0.1);
            gx_tuner_triangle(cr, x0+20, y0+40, 15, 10);
            cairo_set_source_rgb (cr, 0.5+ 0.022* abs(cents), 0.35, 0.1);
            gx_tuner_triangle(cr, x0+80, y0+40, -15, 10);
            gx_tuner_strobe(cr, x0, y0, static_cast<double>(cents));
        } else {
            cents = static_cast<int>((ceil(scale * 10000) / 50));
            mini_cents = (scale * 10000) / 50;
            if (mini_cents<0)
                snprintf(s, sizeof(s), "%.2f", mini_cents);
            else
                snprintf(s, sizeof(s), "+%.2f", mini_cents);
            cairo_set_source_rgb (cr, 0.05* abs(cents), 0.5, 0.1);
            gx_tuner_triangle(cr, x0+80, y0+40, -15, 10);
            gx_tuner_triangle(cr, x0+20, y0+40, 15, 10);
            gx_tuner_strobe(cr, x0, y0, mini_cents);
        }
    } else {
        cents = 100;
        snprintf(s, sizeof(s), "+ - cent");
    }    
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.1);
    cairo_set_font_size (cr, 6.0);
    cairo_text_extents(cr, s, &ex);
    cairo_move_to (cr, x0+28-ex.width, y0+58);
    cairo_show_text(cr, s);

    double ux=2., uy=2.;
    cairo_device_to_user_distance (cr, &ux, &uy);
    if (ux < uy)
        ux = uy;
    cairo_set_line_width (cr, ux + grow);

    // indicator (line)
    cairo_move_to(cr,x0+50, y0+rect_height+5);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_dash (cr, dash_ind, sizeof(dash_ind)/sizeof(dash_ind[0]), 1);
    cairo_line_to(cr, (log_scale(cents, scale)*2*rect_width)+x0+50, y0+(scale*scale*30)+2);
    cairo_set_source_rgb(cr,  0.5, 0.1, 0.1);
    cairo_stroke(cr);   

    g_free (allocation); 
    return FALSE;
}

/*
** paint tuner background picture (the non-changing parts)
*/
static void draw_background(cairo_surface_t *surface) {
    cairo_t *cr;

    double x0      = 0;
    double y0      = 0;

    cr = cairo_create(surface);
    cairo_scale(cr, 3, 3);
    // background
    cairo_rectangle (cr, x0-1,y0-1,rect_width+2,rect_height+2);
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_fill_preserve(cr);
    // light
    cairo_pattern_t*pat =
        cairo_pattern_create_radial (-50, y0, 5,rect_width-10,  rect_height, 20.0);
    cairo_pattern_add_color_stop_rgba (pat, 0, 1, 1, 1, 0.8);
    cairo_pattern_add_color_stop_rgba (pat, 0.3, 0.4, 0.4, 0.4, 0.8);
    cairo_pattern_add_color_stop_rgba (pat, 0.6, 0.05, 0.05, 0.05, 0.8);
    cairo_pattern_add_color_stop_rgba (pat, 1, 0.0, 0.0, 0.0, 0.8);
    cairo_set_source (cr, pat);
    //cairo_rectangle (cr, x0+2,y0+2,rect_width-3,rect_height-3);
    cairo_fill(cr);
     // division scale
    pat = cairo_pattern_create_linear (x0+50, y0,x0, y0);
    cairo_pattern_set_extend(pat, CAIRO_EXTEND_REFLECT);
    cairo_pattern_add_color_stop_rgb (pat, 0, 0.1, 0.8, 0.1);
    cairo_pattern_add_color_stop_rgb (pat, 0.1, 0.1, 0.6, 0.1);
    cairo_pattern_add_color_stop_rgb (pat, 0.2, 0.3, 0.6, 0.1);
    cairo_pattern_add_color_stop_rgb (pat, 0.3, 0.4, 0.4, 0.1);
    cairo_pattern_add_color_stop_rgb (pat, 1, 0.8, 0.1, 0.1);
    cairo_set_source (cr, pat);
    cairo_set_dash (cr, dashes, sizeof (dashes)/sizeof(dashes[0]), 100.0);
    cairo_set_line_width(cr, 3.0);
    for (int i = -5; i < -1; i++) {
        cairo_move_to(cr,x0+50, y0+rect_height-5);
        cairo_line_to(cr, (((i*0.08))*rect_width)+x0+50, y0+(((i*0.1*i*0.1))*30)+2);
    }
    for (int i = 2; i < 6; i++) {
        cairo_move_to(cr,x0+50, y0+rect_height-5);
        cairo_line_to(cr, (((i*0.08))*rect_width)+x0+50, y0+(((i*0.1*i*0.1))*30)+2);
    }
    cairo_move_to(cr,x0+50, y0+rect_height-5);
    cairo_line_to(cr, x0+50, y0+2);
    cairo_stroke(cr);
    cairo_set_line_width(cr, 1);
    cairo_set_dash (cr, dashes, sizeof (dashes)/sizeof(dashes[0]), 100.0);
    cairo_move_to(cr,x0+50, y0+rect_height-5);
    cairo_line_to(cr, (((-3*0.04))*rect_width)+x0+50, y0+(((-3*0.1*-3*0.1))*30)+2);
    cairo_move_to(cr,x0+50, y0+rect_height-5);
    cairo_line_to(cr, (((-2*0.048))*rect_width)+x0+50, y0+(((-2*0.1*-2*0.1))*30)+2);
    cairo_move_to(cr,x0+50, y0+rect_height-5);
    cairo_line_to(cr, (((3*0.04))*rect_width)+x0+50, y0+(((3*0.1*3*0.1))*30)+2);
    cairo_move_to(cr,x0+50, y0+rect_height-5);
    cairo_line_to(cr, (((2*0.048))*rect_width)+x0+50, y0+(((2*0.1*2*0.1))*30)+2);

    for (int i = -2; i < 3; i++) {
        cairo_move_to(cr,x0+50, y0+rect_height-5);
        cairo_line_to(cr, (((i*0.035))*rect_width)+x0+50, y0+(((i*0.1*i*0.1))*30)+2);
    }
    cairo_stroke(cr);


    pat =
	cairo_pattern_create_linear (x0+30, y0, x0+70, y0);
    
    cairo_pattern_add_color_stop_rgb (pat, 1, 0.2, 0.2 , 0.2);
    cairo_pattern_add_color_stop_rgb (pat, 0.5, 0.1, 0.1 , 0.1);
    cairo_pattern_add_color_stop_rgb (pat, 0,0.05, 0.05 , 0.05);
    cairo_set_source (cr, pat);
    cairo_arc(cr, x0+50, y0+rect_height+5, 12.0, 0, 2*M_PI);
    cairo_fill_preserve(cr);
    cairo_set_dash (cr, no_dash, sizeof(no_dash)/sizeof(no_dash[0]), 0);
    cairo_pattern_add_color_stop_rgb (pat, 0, 0.1, 0.1 , 0.1);
    cairo_pattern_add_color_stop_rgb (pat, 0.8, 0.05, 0.05 , 0.05);
    cairo_pattern_add_color_stop_rgb (pat, 1,0.01, 0.01 , 0.01);
    cairo_set_source (cr, pat);
    cairo_set_line_width(cr, 1.0);
    cairo_stroke(cr);
    
    cairo_set_source_rgb(cr,0.1,0.1,0.1);
    gx_tuner_triangle(cr, x0+20, y0+40, 15, 10);
    gx_tuner_triangle(cr, x0+80, y0+40, -15, 10);
    cairo_stroke(cr);

    
    // indicator shaft (circle)
    /*cairo_set_dash (cr, dash_ind, sizeof(dash_ind)/sizeof(dash_ind[0]), 0);
    cairo_move_to(cr, x0+50, y0+rect_height-5);
    cairo_arc(cr, x0+50, y0+rect_height-5, 2.0, 0, 2*M_PI);
    cairo_set_source_rgb(cr,  0.5, 0.1, 0.1);
    cairo_set_line_width(cr, 2.0);
    cairo_stroke(cr);*/
    cairo_pattern_destroy(pat);
    cairo_destroy(cr);
}


