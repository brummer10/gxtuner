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

// Use properties to set variables for the widget from outside,
// add new properties here in the enum 

enum {
    PROP_FREQ = 1,
    PROP_REFERENCE_PITCH = 2,
    PROP_MODE = 3,
    PROP_DOREMI = 4,
    PROP_REFERENCE_NOTE =5, 
    PROP_REFERENCE_03COMMA =6,
    PROP_REFERENCE_05COMMA =7,
    PROP_REFERENCE_07COMMA =8,
    PROP_REFERENCE_11COMMA =9,
    PROP_REFERENCE_13COMMA =10,
    PROP_REFERENCE_17COMMA =11,
    PROP_REFERENCE_19COMMA =12,
    PROP_REFERENCE_23COMMA =13,
    PROP_REFERENCE_29COMMA =14,
    PROP_REFERENCE_31COMMA =15,
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
static void gx_tuner_destroy(GObject  *object);

static const int tuner_width = 100;
static const int tuner_height = 60;
static const double rect_width = 100;
static const double rect_height = 60;

static int cents = 0;
static float mini_cents = 0.0;

// base scale: 3limit diatonic (Pythagorean)
static int scale3base[7][NRPRIMES] = {
    //{notename,2,3,5,7,11,13,17,19,23,29,31}
    {0,2,-1,0,0,0,0,0,0,0,0,0}, //F 0
    {0,0,0,0,0,0,0,0,0,0,0,0}, //C 1
    {0,-1,1,0,0,0,0,0,0,0,0,0}, //G 2
    {0,-3,2,0,0,0,0,0,0,0,0,0}, //D 3
    {0,-4,3,0,0,0,0,0,0,0,0,0}, //A 4 
    {0,-6,4,0,0,0,0,0,0,0,0,0}, //E 5
    {0,-7,5,0,0,0,0,0,0,0,0,0} //B 6
};
const char* scale3basenames[7] = {"F","C","G","D","A","E","B"};
const char* scale3basenamesdoremi[7] = {"Fa","Do","Sol","Ré","La","Mi","Si"};

static int a02comma[NRPRIMES] = {0,1,0,0,0,0,0,0,0,0,0,0};
static int a03comma[NRPRIMES] = {0,-11,7,0,0,0,0,0,0,0,0,0};
static int a05comma[NRPRIMES] = {0,-4,4,-1,0,0,0,0,0,0,0,0};
static int a07comma[NRPRIMES] = {0,-6,2,0,1,0,0,0,0,0,0,0};
static int a11comma[NRPRIMES] = {0,-5,1,0,0,1,0,0,0,0,0,0};
static int a13comma[NRPRIMES] = {0,-10,4,0,0,0,1,0,0,0,0,0};
static int a17comma[NRPRIMES] = {0,7,-7,0,0,0,0,1,0,0,0,0};
static int a19comma[NRPRIMES] = {0,-9,3,0,0,0,0,0,1,0,0,0};
static int a23comma[NRPRIMES] = {0,5,-6,0,0,0,0,0,0,1,0,0};
static int a29comma[NRPRIMES] = {0,-8,2,0,0,0,0,0,0,0,1,0};
static int a31comma[NRPRIMES] = {0,3,-5,0,0,0,0,0,0,0,0,1};

//here we define the scales. Every row of the array has 11 digits. The first 
static int scale3diatonic[7][NRPRIMES] = {
    //notename+integers for the comma's
    {1,0,0,0,0,0,0,0,0,0,0,0}, //C
    {3,0,0,0,0,0,0,0,0,0,0,0}, //D
    {5,0,0,0,0,0,0,0,0,0,0,0}, //E
    {0,0,0,0,0,0,0,0,0,0,0,0}, //F
    {2,0,0,0,0,0,0,0,0,0,0,0}, //G
    {4,0,0,0,0,0,0,0,0,0,0,0}, //A
    {6,0,0,0,0,0,0,0,0,0,0,0}  //B
};
static int numnotesscale3diatonic = 7;

static int scale35chromatic[12][NRPRIMES] = {
    //basenote,2,3,5,7,11,13,17,19,23,29,31
    {1,0,0,0,0,0,0,0,0,0,0,0}, //C
    {1,0,1,0,0,0,0,0,0,0,0,0}, //C♯
    {3,0,0,0,0,0,0,0,0,0,0,0}, //D
    {5,0,-1,1,0,0,0,0,0,0,0,0}, //Eb+
    {5,0,0,-1,0,0,0,0,0,0,0,0}, //E-
    {0,0,0,0,0,0,0,0,0,0,0,0}, //F
    {2,0,-1,-1,0,0,0,0,0,0,0,0}, //Gb 36/25
    {2,0,0,0,0,0,0,0,0,0,0,0}, //G
    {4,0,-1,1,0,0,0,0,0,0,0,0}, //Ab+ 8/5
    {4,0,0,-1,0,0,0,0,0,0,0,0}, //A- 5/3
    {6,0,-1,1,0,0,0,0,0,0,0,0}, //Bb+ 9/5
    {6,0,0,-1,0,0,0,0,0,0,0,0} //B- 15/8
};
static int numnotesscale35chromatic = 12;

static int scale357chromatic[22][NRPRIMES] = {
    //basenote,2,3,5,7,11,13,17,19,23,29,31
    {1,0,0,0,0,0,0,0,0,0,0,0}, //C
    {3,0,-1,0,1,0,0,0,0,0,0,0}, //Db7 28/27
    {3,0,-1,1,0,0,0,0,0,0,0,0}, //Db+ 16/15
    {3,0,0,-1,0,0,0,0,0,0,0,0}, //D- 10/9
    {3,0,0,0,-1,0,0,0,0,0,0,0}, //DL 8/7
    {5,0,-1,0,1,0,0,0,0,0,0,0}, //Eb7 7/6
    {5,0,-1,1,0,0,0,0,0,0,0,0}, //Eb+ 6/5
    {5,0,0,-1,0,0,0,0,0,0,0,0}, //E- 5/4
    {5,0,0,0,-1,0,0,0,0,0,0,0}, //EL 9/7 
    {0,0,0,0,1,0,0,0,0,0,0,0}, //F7 21/16
    {0,0,0,1,0,0,0,0,0,0,0,0}, //F+ 27/20
    {0,0,1,-1,0,0,0,0,0,0,0,0}, //F#- 45/32
    {0,0,1,0,-1,0,0,0,0,0,0,0}, //F#L 81/56
    {2,0,0,0,0,0,0,0,0,0,0,0}, //G 3/2
    {4,0,-1,0,1,0,0,0,0,0,0,0}, //Ab7 14/9
    {4,0,-1,1,0,0,0,0,0,0,0,0}, //Ab+ 8/5
    {4,0,0,-1,0,0,0,0,0,0,0,0}, //A- 5/3
    {4,0,0,0,-1,0,0,0,0,0,0,0}, //AL 12/7
    {6,0,-1,0,1,0,0,0,0,0,0,0}, //Bb7 7/4
    {6,0,-1,1,0,0,0,0,0,0,0,0}, //Bb+ 9/5
    {6,0,0,-1,0,0,0,0,0,0,0,0}, //B- 15/8
    {6,0,0,0,-1,0,0,0,0,0,0,0} //BL 27/14
};
static int numnotesscale357chromatic = 22;

static int scale37chromatic[12][NRPRIMES] = {
    //basenote,2,3,5,7,11,13,17,19,23,29,31
    {1,0,0,0,0,0,0,0,0,0,0,0}, //C 1/1
    {3,0,-1,0,1,0,0,0,0,0,0,0}, //Db7 49/48
    {3,0,0,0,0,0,0,0,0,0,0,0}, //D 9/8
    {5,0,-1,0,1,0,0,0,0,0,0,0}, //Eb7 7/6
    {5,0,0,0,-1,0,0,0,0,0,0,0}, //EL 9/7
    {0,0,0,0,0,0,0,0,0,0,0,0}, //F 4/3
    {0,0,1,0,-2,0,0,0,0,0,0,0}, //F#LL 72/49
    {2,0,0,0,0,0,0,0,0,0,0,0}, //G 3/2
    {4,0,-1,0,1,0,0,0,0,0,0,0}, //Ab7 14/9
    {4,0,0,0,-1,0,0,0,0,0,0,0}, //AL 12/7
    {6,0,-1,0,1,0,0,0,0,0,0,0}, //Bb7 7/4
    {6,0,0,0,-1,0,0,0,0,0,0,0} //BL 27/14
};
static int numnotesscale37chromatic = 12;


static int scaleovertones[8][NRPRIMES] = {
    //basenote,2,3,5,7,11,13,17,19,23,29,31
    {1,0,0,0,0,0,0,0,0,0,0,0}, //C 1/1
    {3,0,0,0,0,0,0,0,0,0,0,0}, //D 9/8
    {5,0,0,-1,0,0,0,0,0,0,0,0}, //E- 5/4
    {0,0,0,0,0,1,0,0,0,0,0,0}, //F11 11/8
    {2,0,0,0,0,0,0,0,0,0,0,0}, //G 3/2
    {4,0,-1,0,0,0,1,0,0,0,0,0}, //Ab13 13/8
    {6,0,-1,0,1,0,0,0,0,0,0,0}, //Bb7 7/4
    {6,0,0,-1,0,0,0,0,0,0,0,0} //B- 15/8
};
static int numnotesscaleovertones = 8;

static int scale16limit[54][NRPRIMES] = {
    //basenote,2,3,5,7,11,13,17,19,23,29,31
    {1,0,0,0,0,0,0,0,0,0,0,0}, //C 1/1
    {3,0,-1,1,0,0,0,0,0,0,0,0}, // 16/15
    {1,0,1,-1,-1,0,0,0,0,0,0,0}, // 15/14
    {3,0,0,0,1,0,-1,0,0,0,0,0}, // 14/13
    {3,0,-1,0,0,0,1,0,0,0,0,0}, // 13/12
    {3,0,0,0,0,-1,0,0,0,0,0,0}, // 12/11
    {3,0,-1,1,0,1,0,0,0,0,0,0}, // 11/10
    {3,0,0,-1,0,0,0,0,0,0,0,0}, // 10/9
    {3,0,0,0,0,0,0,0,0,0,0,0}, // 9/8
    {3,0,0,0,-1,0,0,0,0,0,0,0}, // 8/7
    {3,0,1,-1,0,0,-1,0,0,0,0,0}, // 15/13
    {5,0,-1,0,1,0,0,0,0,0,0,0}, // 7/6
    {5,0,-1,0,0,-1,1,0,0,0,0,0}, // 13/11
    {5,0,-1,1,0,0,0,0,0,0,0,0}, // 6/5
    {5,0,-1,0,0,1,0,0,0,0,0,0}, // 11/9
    {5,0,0,0,0,0,-1,0,0,0,0,0}, // 16/13
    {5,0,0,-1,0,0,0,0,0,0,0,0}, // 5/4
    {0,0,0,0,1,-1,0,0,0,0,0,0}, // 14/11
    {5,0,0,0,-1,0,0,0,0,0,0,0}, // 9/7
    {0,0,-1,1,0,0,1,0,0,0,0,0}, // 13/10
    {0,0,0,0,0,0,0,0,0,0,0,0}, // 4/3
    {0,0,1,-1,0,-1,0,0,0,0,0,0}, // 15/11
    {0,0,0,0,0,1,0,0,0,0,0,0}, // 11/8
    {0,0,1,0,0,0,-1,0,0,0,0,0}, // 18/13
    {2,0,-1,1,1,0,0,0,0,0,0,0}, // 7/5
    {0,0,1,-1,-1,0,0,0,0,0,0,0}, // 10/7
    {2,0,-1,0,0,0,1,0,0,0,0,0}, // 13/9
    {2,0,0,0,0,-1,0,0,0,0,0,0}, // 16/11
    {2,0,-1,1,0,1,0,0,0,0,0,0}, // 22/15
    {2,0,0,0,0,0,0,0,0,0,0,0}, //3/2
    {2,0,1,-1,0,0,-1,0,0,0,0,0}, // 20/13
    {4,0,-1,0,1,0,0,0,0,0,0,0}, // 14/9
    {2,0,0,0,-1,1,0,0,0,0,0,0}, // 11/7
    {4,0,-1,1,0,0,0,0,0,0,0,0}, //8/5
    {4,0,0,0,1,0,-1,0,0,0,0,0}, // 21/13
    {4,0,-1,0,0,0,1,0,0,0,0,0}, // 13/8
    {4,0,0,0,0,-1,0,0,0,0,0,0}, // 18/11
    {4,0,0,-1,0,0,0,0,0,0,0,0}, // 5/3
    {4,0,0,0,0,1,-1,0,0,0,0,0}, // 22/13
    {4,0,0,0,-1,0,0,0,0,0,0,0}, // 14/7
    {6,0,-2,1,0,0,1,0,0,0,0,0}, // 26/15 Hier gebleven
    {6,0,-1,0,1,0,0,0,0,0,0,0}, // 7/4
    {6,0,-1,0,0,0,0,0,0,0,0,0}, // 16/9
    {4,0,1,-2,-1,0,0,0,0,0,0,0}, // 25/14
    {6,0,-1,1,0,0,0,0,0,0,0,0}, // 9/5
    {6,0,0,-1,0,-1,0,0,0,0,0,0}, // 20/11
    {6,0,-1,0,0,1,0,0,0,0,0,0}, // 11/6
    {6,0,0,0,0,0,-1,0,0,0,0,0}, // 24/13
    {6,0,-1,0,-1,0,1,0,0,0,0,0}, // 13/7
    {1,1,-1,1,1,0,0,0,0,0,0,0}, // 28/15
    {6,0,0,-1,0,0,0,0,0,0,0,0}, // 15/8
    {1,1,0,0,1,-1,0,0,0,0,0,0}, // 21/11
    {6,0,1,-2,0,0,-1,0,0,0,0,0}, // 25/13
    {6,0,0,0,-1,0,0,0,0,0,0,0}, // 27/14
};

static int numnotesscale16limit = 54;

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
	
	// here we setup get and set methodes for the properties
    gobject_class->set_property = gx_tuner_set_property;
    gobject_class->get_property = gx_tuner_get_property;
    // here we install the propertys for the widget, add new 
    // properties here
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
    g_object_class_install_property(
        gobject_class, PROP_DOREMI, g_param_spec_int ( //#2
            "doremi", P_("CDE or DoReMi"),
            P_("CDE or DoReMi"),
            0, 1, 0, G_PARAM_READWRITE));
    g_object_class_install_property(
        gobject_class, PROP_REFERENCE_NOTE, g_param_spec_int (
            "reference-note", P_("Reference note"),
            P_("The note for which tuning is displayed"),
            0, 1, 0, G_PARAM_READWRITE));
    g_object_class_install_property(
        gobject_class, PROP_REFERENCE_03COMMA, g_param_spec_int (
            "reference-03comma", P_("Reference 03 comma"),
            P_("The 03 comma for which tuning is displayed"),
            0, 1, 0, G_PARAM_READWRITE));
    g_object_class_install_property(
        gobject_class, PROP_REFERENCE_05COMMA, g_param_spec_int (
            "reference-05comma", P_("Reference 05 comma"),
            P_("The 05 comma for which tuning is displayed"),
            0, 1, 0, G_PARAM_READWRITE));
    g_object_class_install_property(
        gobject_class, PROP_REFERENCE_07COMMA, g_param_spec_int (
            "reference-07comma", P_("Reference 07 comma"),
            P_("The 07 comma for which tuning is displayed"),
            0, 1, 0, G_PARAM_READWRITE));
    g_object_class_install_property(
        gobject_class, PROP_REFERENCE_11COMMA, g_param_spec_int (
            "reference-11comma", P_("Reference 11 comma"),
            P_("The 11 comma for which tuning is displayed"),
            0, 1, 0, G_PARAM_READWRITE));
    g_object_class_install_property(
        gobject_class, PROP_REFERENCE_13COMMA, g_param_spec_int (
            "reference-13comma", P_("Reference 13 comma"),
            P_("The 13 comma for which tuning is displayed"),
            0, 1, 0, G_PARAM_READWRITE));
    g_object_class_install_property(
        gobject_class, PROP_REFERENCE_17COMMA, g_param_spec_int (
            "reference-17comma", P_("Reference 17 comma"),
            P_("The 17 comma for which tuning is displayed"),
            0, 1, 0, G_PARAM_READWRITE));
    g_object_class_install_property(
        gobject_class, PROP_REFERENCE_19COMMA, g_param_spec_int (
            "reference-19comma", P_("Reference 19 comma"),
            P_("The 19 comma for which tuning is displayed"),
            0, 1, 0, G_PARAM_READWRITE));
    g_object_class_install_property(
        gobject_class, PROP_REFERENCE_23COMMA, g_param_spec_int (
            "reference-23comma", P_("Reference 23 comma"),
            P_("The 23 comma for which tuning is displayed"),
            0, 1, 0, G_PARAM_READWRITE));
    g_object_class_install_property(
        gobject_class, PROP_REFERENCE_29COMMA, g_param_spec_int (
            "reference-29comma", P_("Reference 29 comma"),
            P_("The 29 comma for which tuning is displayed"),
            0, 1, 0, G_PARAM_READWRITE));
    g_object_class_install_property(
        gobject_class, PROP_REFERENCE_31COMMA, g_param_spec_int (
            "reference-31comma", P_("Reference 31 comma"),
            P_("The 31 comma for which tuning is displayed"),
            0, 1, 0, G_PARAM_READWRITE));
    gobject_class->finalize = gx_tuner_destroy;
    
    klass->surface_tuner = cairo_image_surface_create(
        CAIRO_FORMAT_ARGB32, tuner_width*3., tuner_height*3.);
    g_assert(klass->surface_tuner != NULL);
    draw_background(klass->surface_tuner);
}

static void gx_tuner_destroy(GObject *object) {
    g_return_if_fail (object != NULL);
    g_return_if_fail (GX_IS_TUNER (object));
    GxTuner *tuner = GX_TUNER(object);
    for(int i=0;i<MAXSCALENOTES;i++) {
           free(tuner->tempscaletranslatednames[i]);
    }
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
    tuner->mode = 1;
    tuner->doremi = 0; //#3
    tuner->reference_note = 1;
    tuner->reference_03comma = 3;
    tuner->reference_05comma = 3;
    tuner->reference_07comma = 3;
    tuner->reference_11comma = 3;
    tuner->reference_13comma = 3;
    tuner->reference_17comma = 3;
    tuner->reference_19comma = 3;
    tuner->reference_23comma = 3;
    tuner->reference_29comma = 3;
    tuner->reference_31comma = 3;
    tuner->scale_w = 1.;
    tuner->scale_h = 1.;
    for(int i=0;i<MAXSCALENOTES;i++) {
    tuner->tempscaletranslatednames[i] = (char*)malloc(sizeof(char*)); // allocate
    }
    for(int n=0;n<MAXSCALENOTES;n++){
        for(int i=0;i<NRPRIMES;i++){
            tuner->tempscaletranslated[n][i]=0;
        }
    }
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

void gx_tuner_set_doremi(GxTuner *tuner, int doremi) { //#4
    g_assert(GX_IS_TUNER(tuner));
    tuner->doremi = doremi;
    gtk_widget_queue_draw(GTK_WIDGET(tuner));
    g_object_notify(G_OBJECT(tuner), "doremi");
}
void gx_tuner_set_reference_note(GxTuner *tuner, int reference_note) {
    g_assert(GX_IS_TUNER(tuner));
    tuner->reference_note = reference_note;
    gtk_widget_queue_draw(GTK_WIDGET(tuner));
    g_object_notify(G_OBJECT(tuner), "reference-note");
}
void gx_tuner_set_reference_03comma(GxTuner *tuner, int reference_03comma) { 
    g_assert(GX_IS_TUNER(tuner));
    tuner->reference_03comma = reference_03comma;
    gtk_widget_queue_draw(GTK_WIDGET(tuner));
    g_object_notify(G_OBJECT(tuner), "reference-03comma");
}
void gx_tuner_set_reference_05comma(GxTuner *tuner, int reference_05comma) { 
    g_assert(GX_IS_TUNER(tuner));
    tuner->reference_05comma = reference_05comma;
    gtk_widget_queue_draw(GTK_WIDGET(tuner));
    g_object_notify(G_OBJECT(tuner), "reference-05comma");
}
void gx_tuner_set_reference_07comma(GxTuner *tuner, int reference_07comma) { 
    g_assert(GX_IS_TUNER(tuner));
    tuner->reference_07comma = reference_07comma;
    gtk_widget_queue_draw(GTK_WIDGET(tuner));
    g_object_notify(G_OBJECT(tuner), "reference-07comma");
}
void gx_tuner_set_reference_11comma(GxTuner *tuner, int reference_11comma) { 
    g_assert(GX_IS_TUNER(tuner));
    tuner->reference_11comma = reference_11comma;
    gtk_widget_queue_draw(GTK_WIDGET(tuner));
    g_object_notify(G_OBJECT(tuner), "reference-11comma");
}
void gx_tuner_set_reference_13comma(GxTuner *tuner, int reference_13comma) { 
    g_assert(GX_IS_TUNER(tuner));
    tuner->reference_13comma = reference_13comma;
    gtk_widget_queue_draw(GTK_WIDGET(tuner));
    g_object_notify(G_OBJECT(tuner), "reference-13comma");
}
void gx_tuner_set_reference_17comma(GxTuner *tuner, int reference_17comma) { 
    g_assert(GX_IS_TUNER(tuner));
    tuner->reference_17comma = reference_17comma;
    gtk_widget_queue_draw(GTK_WIDGET(tuner));
    g_object_notify(G_OBJECT(tuner), "reference-17comma");
}
void gx_tuner_set_reference_19comma(GxTuner *tuner, int reference_19comma) { 
    g_assert(GX_IS_TUNER(tuner));
    tuner->reference_19comma = reference_19comma;
    gtk_widget_queue_draw(GTK_WIDGET(tuner));
    g_object_notify(G_OBJECT(tuner), "reference-19comma");
}
void gx_tuner_set_reference_23comma(GxTuner *tuner, int reference_23comma) { 
    g_assert(GX_IS_TUNER(tuner));
    tuner->reference_23comma = reference_23comma;
    gtk_widget_queue_draw(GTK_WIDGET(tuner));
    g_object_notify(G_OBJECT(tuner), "reference-23comma");
}
void gx_tuner_set_reference_29comma(GxTuner *tuner, int reference_29comma) { 
    g_assert(GX_IS_TUNER(tuner));
    tuner->reference_29comma = reference_29comma;
    gtk_widget_queue_draw(GTK_WIDGET(tuner));
    g_object_notify(G_OBJECT(tuner), "reference-29comma");
}
void gx_tuner_set_reference_31comma(GxTuner *tuner, int reference_31comma) { 
    g_assert(GX_IS_TUNER(tuner));
    tuner->reference_31comma = reference_31comma;
    gtk_widget_queue_draw(GTK_WIDGET(tuner));
    g_object_notify(G_OBJECT(tuner), "reference-31comma");
}

double gx_tuner_get_reference_pitch(GxTuner *tuner) {
    g_assert(GX_IS_TUNER(tuner));
    return tuner->reference_pitch;
}

GtkWidget *gx_tuner_new(void) {
    return (GtkWidget*)g_object_new(GX_TYPE_TUNER, NULL);
}

// here we set properties, add new ones here

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
    case PROP_DOREMI: //#5
        gx_tuner_set_doremi(tuner, g_value_get_int(value));
        break;
    case PROP_REFERENCE_NOTE:
        gx_tuner_set_reference_note(tuner, g_value_get_int(value));
        break;
    case PROP_REFERENCE_03COMMA:
        gx_tuner_set_reference_03comma(tuner, g_value_get_int(value));
        break;
    case PROP_REFERENCE_07COMMA:
        gx_tuner_set_reference_07comma(tuner, g_value_get_int(value));
        break;
    case PROP_REFERENCE_11COMMA:
        gx_tuner_set_reference_11comma(tuner, g_value_get_int(value));
        break;
    case PROP_REFERENCE_13COMMA:
        gx_tuner_set_reference_13comma(tuner, g_value_get_int(value));
        break;
    case PROP_REFERENCE_17COMMA:
        gx_tuner_set_reference_17comma(tuner, g_value_get_int(value));
        break;
    case PROP_REFERENCE_19COMMA:
        gx_tuner_set_reference_19comma(tuner, g_value_get_int(value));
        break;
    case PROP_REFERENCE_23COMMA:
        gx_tuner_set_reference_23comma(tuner, g_value_get_int(value));
        break;
    case PROP_REFERENCE_29COMMA:
        gx_tuner_set_reference_29comma(tuner, g_value_get_int(value));
        break;
    case PROP_REFERENCE_31COMMA:
        gx_tuner_set_reference_31comma(tuner, g_value_get_int(value));
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
    case PROP_DOREMI: //#6
        g_value_set_int(value, tuner->doremi);
        break;
    case PROP_REFERENCE_NOTE:
        g_value_set_int(value, tuner->reference_note);
        break;
    case PROP_REFERENCE_03COMMA: 
        g_value_set_int(value, tuner->reference_03comma);
        break;
    case PROP_REFERENCE_05COMMA: 
        g_value_set_int(value, tuner->reference_05comma);
        break;
    case PROP_REFERENCE_07COMMA: 
        g_value_set_int(value, tuner->reference_07comma);
        break;
    case PROP_REFERENCE_11COMMA: 
        g_value_set_int(value, tuner->reference_11comma);
        break;
    case PROP_REFERENCE_13COMMA: 
        g_value_set_int(value, tuner->reference_13comma);
        break;
    case PROP_REFERENCE_17COMMA: 
        g_value_set_int(value, tuner->reference_17comma);
        break;
    case PROP_REFERENCE_19COMMA: 
        g_value_set_int(value, tuner->reference_19comma);
        break;
    case PROP_REFERENCE_23COMMA: 
        g_value_set_int(value, tuner->reference_23comma);
        break;
    case PROP_REFERENCE_29COMMA: 
        g_value_set_int(value, tuner->reference_29comma);
        break;
    case PROP_REFERENCE_31COMMA: 
        g_value_set_int(value, tuner->reference_31comma);
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

static gboolean gtk_tuner_expose_just(GtkWidget *widget, cairo_t *cr) {
    GxTuner *tuner = GX_TUNER(widget);
    //setting the scale
    if (tuner->mode == 1){
        tuner->tempnumofnotes = numnotesscale3diatonic;
        for (int n=0 ; n<tuner->tempnumofnotes /*notes of choosen scale */; n++){
            for (int i=0; i<NRPRIMES; i++){
                tuner->tempscale[n][i] = scale3diatonic[n][i];
            }
        }
    } else if (tuner->mode == 2 ){
        tuner->tempnumofnotes = numnotesscale35chromatic;
        for (int n=0 ; n<tuner->tempnumofnotes; n++){
            for (int i=0; i<NRPRIMES; i++){
                tuner->tempscale[n][i] = scale35chromatic[n][i];
            }
        }
    } else if (tuner->mode == 3 ){
        tuner->tempnumofnotes = numnotesscale357chromatic;
        for (int n=0 ; n<tuner->tempnumofnotes; n++){
            for (int i=0; i<NRPRIMES; i++){
                tuner->tempscale[n][i] = scale357chromatic[n][i];
            }
        }
    } else if (tuner->mode == 4 ){
    tuner->tempnumofnotes = numnotesscale37chromatic;
    for (int n=0 ; n<tuner->tempnumofnotes; n++){
        for (int i=0; i<NRPRIMES; i++){
            tuner->tempscale[n][i] = scale37chromatic[n][i];
        }
    }
    }else if (tuner->mode == 5 ){
        tuner->tempnumofnotes = numnotesscaleovertones;
        for (int n=0 ; n<tuner->tempnumofnotes; n++){
            for (int i=0; i<NRPRIMES; i++){
                tuner->tempscale[n][i] = scaleovertones[n][i];
            }
        }
    }else if (tuner->mode == 6 ){
        tuner->tempnumofnotes = numnotesscale16limit;
        for (int n=0 ; n<tuner->tempnumofnotes; n++){
            for (int i=0; i<NRPRIMES; i++){
                tuner->tempscale[n][i] = scale16limit[n][i];
            }
        }
    }
    
    
    //1. creating tempreference_note
    tuner->tempreference_note[0] = tuner->reference_note;
    tuner->tempreference_note[1] = 0;
    tuner->tempreference_note[2] = tuner->reference_03comma-3;
    tuner->tempreference_note[3] = tuner->reference_05comma-3;
    tuner->tempreference_note[4] = tuner->reference_07comma-3;
    tuner->tempreference_note[5] = tuner->reference_11comma-3;
    tuner->tempreference_note[6] = tuner->reference_13comma-3;
    tuner->tempreference_note[7] = tuner->reference_17comma-3;
    tuner->tempreference_note[8] = tuner->reference_19comma-3;
    tuner->tempreference_note[9] = tuner->reference_23comma-3;
    tuner->tempreference_note[10] = tuner->reference_29comma-3;
    tuner->tempreference_note[11] = tuner->reference_31comma-3;
    
    //2. tempscaletranslated
    //memset(tuner->tempscaletranslated[0], 0, sizeof(tempscaletranslated));
    for(int i=0;i<MAXSCALENOTES;i++) {
         memset(tuner->tempscaletranslatednames[i], 0, sizeof(char));
    }
    //int temp;
    for (int n=0; n<tuner->tempnumofnotes; n++){
        tuner->tempscaletranslated[n][2]=0;
        tuner->temp=0;
        for (int i=1; i<NRPRIMES; i++){
            tuner->tempscaletranslated[n][i]=tuner->tempreference_note[i]+tuner->tempscale[n][i];
        }
        
        tuner->temp = tuner->tempscale[n][0] + tuner->tempreference_note[0]-1;
        
        if (tuner->temp < 0){
            tuner->temp = tuner->temp + 7;
            tuner->tempscaletranslated[n][2]=tuner->tempscaletranslated[n][2]-1;
            tuner->tempscaletranslated[n][0]= tuner->temp;
        }else if (tuner->temp > 6){
            tuner->temp = tuner->temp - 7;
            tuner->tempscaletranslated[n][2]=tuner->tempscaletranslated[n][2]+1;
            tuner->tempscaletranslated[n][0]= tuner->temp;
        }
        else {tuner->tempscaletranslated[n][0]= tuner->temp;}           
    }
        
    //3. creatnotenames with tempscaletranslated
    
    int i = 0;
    for (int n=0; n<tuner->tempnumofnotes; n++){
        if(tuner->doremi == 0){
            strcat(tuner->tempscaletranslatednames[n],scale3basenames[tuner->tempscaletranslated[n][0]]);
        } else if (tuner->doremi == 1) {strcat(tuner->tempscaletranslatednames[n],scale3basenamesdoremi[tuner->tempscaletranslated[n][0]]);
            }
        //fprintf(stderr,"Stand doremi: %i \n",tuner->doremi);
        while (i < 24){ 
            i = 0;
            i++;
            // 3limit comma
            if (tuner->tempscaletranslated[n][2] < 0){
                    for (int j=0; j<abs(tuner->tempscaletranslated[n][2]); j++){
                        strcat(tuner->tempscaletranslatednames[n],"♭");
                        i++;
                    }
                } else if (tuner->tempscaletranslated[n][2] > 0){
                    for (int j=0; j<tuner->tempscaletranslated[n][2]; j++){
                        strcat(tuner->tempscaletranslatednames[n],"♯");
                        i++;
                    }              
                }
            // 5limit comma
            if (tuner->tempscaletranslated[n][3] < 0){
                    for (int j=0; j<abs(tuner->tempscaletranslated[n][3]); j++){
                        strcat(tuner->tempscaletranslatednames[n],"-");
                        i++;
                    }
                } else if (tuner->tempscaletranslated[n][3] > 0){
                    for (int j=0; j<tuner->tempscaletranslated[n][3]; j++){
                        strcat(tuner->tempscaletranslatednames[n],"+");
                        i++;
                    }              
                }
            // 7limit comma
            if (tuner->tempscaletranslated[n][4] < 0){
                    for (int j=0; j<abs(tuner->tempscaletranslated[n][4]); j++){
                        strcat(tuner->tempscaletranslatednames[n],"L");
                        i++;
                    }
                } else if (tuner->tempscaletranslated[n][4] > 0){
                    for (int j=0; j<tuner->tempscaletranslated[n][4]; j++){
                        strcat(tuner->tempscaletranslatednames[n],"7");
                        i++;
                    }              
                }
            // 11limit comma
            if (tuner->tempscaletranslated[n][5] < 0){
                    for (int j=0; j<abs(tuner->tempscaletranslated[n][5]); j++){
                        strcat(tuner->tempscaletranslatednames[n],"↓");
                        i++;
                    }
                } else if (tuner->tempscaletranslated[n][5] > 0){
                    for (int j=0; j<tuner->tempscaletranslated[n][5]; j++){
                        strcat(tuner->tempscaletranslatednames[n],"↑");
                        i++;
                    }              
                }
            // 13limit comma
            if (tuner->tempscaletranslated[n][6] < 0){
                    for (int j=0; j<abs(tuner->tempscaletranslated[n][6]); j++){
                        strcat(tuner->tempscaletranslatednames[n],"ƐƖ");
                        i++;
                    }
                } else if (tuner->tempscaletranslated[n][6] > 0){
                    for (int j=0; j<tuner->tempscaletranslated[n][6]; j++){
                        strcat(tuner->tempscaletranslatednames[n],"13");
                        i++;
                    }              
                }
            // 17limit comma
            if (tuner->tempscaletranslated[n][7] < 0){
                    for (int j=0; j<abs(tuner->tempscaletranslated[n][7]); j++){
                        strcat(tuner->tempscaletranslatednames[n],"LƖ");
                        i++;
                    }
                } else if (tuner->tempscaletranslated[n][7] > 0){
                    for (int j=0; j<tuner->tempscaletranslated[n][7]; j++){
                        strcat(tuner->tempscaletranslatednames[n],"17");
                        i++;
                    }              
                }
            // 19limit comma
            if (tuner->tempscaletranslated[n][8] < 0){
                    for (int j=0; j<abs(tuner->tempscaletranslated[n][8]); j++){
                        strcat(tuner->tempscaletranslatednames[n],"6Ɩ");
                        i++;
                    }
                } else if (tuner->tempscaletranslated[n][8] > 0){
                    for (int j=0; j<tuner->tempscaletranslated[n][8]; j++){
                        strcat(tuner->tempscaletranslatednames[n],"19");
                        i++;
                    }              
                }
            // 23limit comma
            if (tuner->tempscaletranslated[n][9] < 0){
                    for (int j=0; j<abs(tuner->tempscaletranslated[n][9]); j++){
                        strcat(tuner->tempscaletranslatednames[n],"ƐS");
                        i++;
                    }
                } else if (tuner->tempscaletranslated[n][9] > 0){
                    for (int j=0; j<tuner->tempscaletranslated[n][9]; j++){
                        strcat(tuner->tempscaletranslatednames[n],"23");
                        i++;
                    }              
                }
            // 29limit comma
            if (tuner->tempscaletranslated[n][10] < 0){
                    for (int j=0; j<abs(tuner->tempscaletranslated[n][10]); j++){
                        strcat(tuner->tempscaletranslatednames[n],"6S");
                        i++;
                    }
                } else if (tuner->tempscaletranslated[n][10] > 0){
                    for (int j=0; j<tuner->tempscaletranslated[n][10]; j++){
                        strcat(tuner->tempscaletranslatednames[n],"29");
                        i++;
                    }              
                }
            // 31limit comma
            if (tuner->tempscaletranslated[n][11] < 0){
                    for (int j=0; j<abs(tuner->tempscaletranslated[n][11]); j++){
                        strcat(tuner->tempscaletranslatednames[n],"ƖƐ");
                        i++;
                    }
                } else if (tuner->tempscaletranslated[n][11] > 0){
                    for (int j=0; j<tuner->tempscaletranslated[n][11]; j++){
                        strcat(tuner->tempscaletranslatednames[n],"31");
                        i++;
                    }              
                }
            strcat(tuner->tempscaletranslatednames[n],"\0");
            break;
        } 
        
    }
    // 4. calculating the translated scale: + comma's and chroma's to powers of primes
    for (int n=0; n<tuner->tempnumofnotes; n++){
        for (int i=1; i<NRPRIMES; i++){
                tuner->tempscaletranslatedpowprimes[n][i] = scale3base[tuner->tempscale[n][0]][i]
                                                        + tuner->tempscale[n][1]  * a02comma[i] 
                                                        + tuner->tempscale[n][2]  * a03comma[i] 
                                                        + tuner->tempscale[n][3]  * a05comma[i]
                                                        + tuner->tempscale[n][4]  * a07comma[i] 
                                                        + tuner->tempscale[n][5]  * a11comma[i]
                                                        + tuner->tempscale[n][6]  * a13comma[i]
                                                        + tuner->tempscale[n][7]  * a17comma[i]
                                                        + tuner->tempscale[n][8]  * a19comma[i]
                                                        + tuner->tempscale[n][9]  * a23comma[i]
                                                        + tuner->tempscale[n][10] * a29comma[i]
                                                        + tuner->tempscale[n][11] * a31comma[i];
        }
    }
    // 5. calculate tempscaletranslateratios
    for (int n=0; n<tuner->tempnumofnotes; n++){
            double tempratio;
            tempratio              =     (pow(2.0,tuner->tempscaletranslatedpowprimes[n][1])
                                         * pow(3.0,tuner->tempscaletranslatedpowprimes[n][2])
                                         * pow(5.0,tuner->tempscaletranslatedpowprimes[n][3])
                                         * pow(7.0,tuner->tempscaletranslatedpowprimes[n][4])
                                         * pow(11.0,tuner->tempscaletranslatedpowprimes[n][5])
                                         * pow(13.0,tuner->tempscaletranslatedpowprimes[n][6])
                                         * pow(17.0,tuner->tempscaletranslatedpowprimes[n][7])
                                         * pow(19.0,tuner->tempscaletranslatedpowprimes[n][8])
                                         * pow(23.0,tuner->tempscaletranslatedpowprimes[n][9])
                                         * pow(29.0,tuner->tempscaletranslatedpowprimes[n][10])
                                         * pow(31.0,tuner->tempscaletranslatedpowprimes[n][11]));
        
            //check if ratio is between 1/1 (unison) and 2/0 (octave) and set it between those two ratios)
            int ratiocheck = log(tempratio)/log(2.0);
            if(ratiocheck<0){
                tuner->tempscaleratios[n] = 1/tempratio/pow(2.0,ratiocheck);
            } else { tuner->tempscaleratios[n] = tempratio/pow(2.0,ratiocheck);
            }
            //fprintf(stderr,"Notenumber: %i Noteratio: %lf \n", n, tuner->tempscaleratios[n]); 
    }
    tuner->tempscaleratios[tuner->tempnumofnotes]=2.0;
    
    // Calculate ratio of reference note
    for (int i=1; i<NRPRIMES; i++){
        tuner->tempreference_notepowprimes[i] =           scale3base[tuner->tempreference_note[0]][i]
                                                        + tuner->tempreference_note[1]  * a02comma[i]
                                                        + tuner->tempreference_note[2]  * a03comma[i] 
                                                        + tuner->tempreference_note[3]  * a05comma[i]
                                                        + tuner->tempreference_note[4]  * a07comma[i] 
                                                        + tuner->tempreference_note[5]  * a11comma[i]
                                                        + tuner->tempreference_note[6]  * a13comma[i]
                                                        + tuner->tempreference_note[7]  * a17comma[i]
                                                        + tuner->tempreference_note[8]  * a19comma[i]
                                                        + tuner->tempreference_note[9]  * a23comma[i]
                                                        + tuner->tempreference_note[10] * a29comma[i]
                                                        + tuner->tempreference_note[11] * a31comma[i];
    }
    tuner->tempreference_noteratio  =     (pow(2.0,tuner->tempreference_notepowprimes[1])
                                         * pow(3.0,tuner->tempreference_notepowprimes[2])
                                         * pow(5.0,tuner->tempreference_notepowprimes[3])
                                         * pow(7.0,tuner->tempreference_notepowprimes[4])
                                         * pow(11.0,tuner->tempreference_notepowprimes[5])
                                         * pow(13.0,tuner->tempreference_notepowprimes[6])
                                         * pow(17.0,tuner->tempreference_notepowprimes[7])
                                         * pow(19.0,tuner->tempreference_notepowprimes[8])
                                         * pow(23.0,tuner->tempreference_notepowprimes[9])
                                         * pow(29.0,tuner->tempreference_notepowprimes[10])
                                         * pow(31.0,tuner->tempreference_notepowprimes[11])) ;
        
    
    
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
        // Set reference frequency to the first note of the translated scale (16/27 is the reciproce of a Pythagorean sixt, i.c. C-->A)
        float ref_c = tuner->reference_pitch * 16.0 / 27.0 * tuner->tempreference_noteratio;
        // now check in which octave we are with the tracked frequency
        // and set the frequency octave divider
        // ref_c is now the frequency of the first note in octave, 
        // but we want to check if the frequency is below the last note in octave
        // so, for example if freq_is is below ref_c we are in octave 3
        // if freq_is is below ref_c/2 we are in octave 2, etc.
    for (int n=0 ; n <= 8 ; ++n )
         { float ratiodiffhighnoteandoctave = exp((log(tuner->tempscaleratios[tuner->tempnumofnotes])+log(2.0))/2) ;  
            //fprintf(stderr, "ratio highestnote %f ratiodiffhighnoteandoctave %f \n", tuner->tempscaleratios[tuner->tempnumofnotes-1] , ratiodiffhighnoteandoctave);
             if (freq_is < (ref_c*pow(2,n-3))-(2-ratiodiffhighnoteandoctave)*(ref_c*pow(2,n-3)) && freq_is >0.0) {
                 indicate_oc = n; 
                 multiply = pow(2, 4-n);
                 break;
                }
         }
    percent = (freq_is/(ref_c/multiply));
    // now we chould check which ratio we have
    // we split the range using log-average
    for (int n=0 ; n < tuner->tempnumofnotes ; ++n ){ 
         float ratiodiff = exp((log(tuner->tempscaleratios[n])+log(tuner->tempscaleratios[n+1]))/2) ;
                 //fprintf(stderr, "ratio note: %f ratiodiff: %f \n", tuner->tempscaleratios[n] , ratiodiff);
            if (percent < ratiodiff) {
                     display_note = n;
                     scale = (percent-tuner->tempscaleratios[n])/2.0;
                     break;
                 }
         }         
    
    //fprintf(stderr, " percent == %f freq = %f ref_c = %f indicate_oc = %i value of numberofnotes is %i \n", percent, freq_is, ref_c, indicate_oc, tuner->tempnumofnotes);
        // display note
        cairo_set_source_rgba(cr, fabsf(scale)*3.0, 1-fabsf(scale)*3.0, 0.2,1-fabsf(scale)*2);
        cairo_set_font_size(cr, 10.0);
        cairo_move_to(cr,x0+40 -9 , y0+30 +9 ); //original was 50 and 54
        cairo_show_text(cr, tuner->tempscaletranslatednames[display_note]);
        cairo_set_font_size(cr, 8.0);
        cairo_move_to(cr,x0+40  , y0+30 +16 );
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
            gx_tuner_triangle(cr, x0+80, y0+45, -15, 10);
            cairo_set_source_rgb (cr, 0.5+ 0.022* abs(cents), 0.35, 0.1);
            gx_tuner_triangle(cr, x0+20, y0+45, 15, 10);
            gx_tuner_strobe(cr, x0, y0, static_cast<double>(cents));
        } else if(scale<-0.004) {
            cents = static_cast<int>((ceil(scale * 10000) / 50));
            snprintf(s, sizeof(s), "%i", cents);
            cairo_set_source_rgb (cr, 0.05, 0.5+0.022* abs(cents), 0.1);
            gx_tuner_triangle(cr, x0+20, y0+45, 15, 10);
            cairo_set_source_rgb (cr, 0.5+ 0.022* abs(cents), 0.35, 0.1);
            gx_tuner_triangle(cr, x0+80, y0+45, -15, 10);
            gx_tuner_strobe(cr, x0, y0, static_cast<double>(cents));
        } else {
            cents = static_cast<int>((ceil(scale * 10000) / 50));
            mini_cents = (scale * 10000) / 50;
            if (mini_cents<0)
                snprintf(s, sizeof(s), "%.2f", mini_cents);
            else
                snprintf(s, sizeof(s), "+%.2f", mini_cents);
            cairo_set_source_rgb (cr, 0.05* abs(cents), 0.5, 0.1);
            gx_tuner_triangle(cr, x0+80, y0+45, -15, 10);
            gx_tuner_triangle(cr, x0+20, y0+45, 15, 10);
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
    if (tuner->mode > 0) {
        if (!gtk_tuner_expose_just (widget, cr)) return FALSE;
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
            gx_tuner_triangle(cr, x0+80, y0+45, -15, 10);
            cairo_set_source_rgb (cr, 0.5+ 0.022* abs(cents), 0.35, 0.1);
            gx_tuner_triangle(cr, x0+20, y0+45, 15, 10);
            gx_tuner_strobe(cr, x0, y0, static_cast<double>(cents));
        } else if(scale<-0.004) {
            cents = static_cast<int>((ceil(scale * 10000) / 50));
            snprintf(s, sizeof(s), "%i", cents);
            cairo_set_source_rgb (cr, 0.05, 0.5+0.022* abs(cents), 0.1);
            gx_tuner_triangle(cr, x0+20, y0+45, 15, 10);
            cairo_set_source_rgb (cr, 0.5+ 0.022* abs(cents), 0.35, 0.1);
            gx_tuner_triangle(cr, x0+80, y0+45, -15, 10);
            gx_tuner_strobe(cr, x0, y0, static_cast<double>(cents));
        } else {
            cents = static_cast<int>((ceil(scale * 10000) / 50));
            mini_cents = (scale * 10000) / 50;
            if (mini_cents<0)
                snprintf(s, sizeof(s), "%.2f", mini_cents);
            else
                snprintf(s, sizeof(s), "+%.2f", mini_cents);
            cairo_set_source_rgb (cr, 0.05* abs(cents), 0.5, 0.1);
            gx_tuner_triangle(cr, x0+80, y0+45, -15, 10);
            gx_tuner_triangle(cr, x0+20, y0+45, 15, 10);
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
    gx_tuner_triangle(cr, x0+20, y0+45, 15, 10);
    gx_tuner_triangle(cr, x0+80, y0+45, -15, 10);
    cairo_stroke(cr);

    
    // indicator shaft (circle)
    cairo_pattern_destroy(pat);
    cairo_destroy(cr);
}
