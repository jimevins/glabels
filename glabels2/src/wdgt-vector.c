/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_vector.c:  vector properties widget module
 *
 *  Copyright (C) 2001-2002  Jim Evins <evins@snaught.com>.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <config.h>

#include "wdgt-vector.h"
#include "prefs.h"
#include "marshal.h"

#include "debug.h"

#include <math.h>

#ifndef M_PI
#define M_PI		3.14159265358979323846	/* pi */
#endif

#define LENGTH(x,y) sqrt( (x)*(x) + (y)*(y) )
#define ANGLE(x,y)  ( (180.0/M_PI)*atan2( -(y), (x) ) )
#define COMP_X(l,a) ( (l) * cos( (M_PI/180.0)*(a) ) )
#define COMP_Y(l,a) ( -(l) * sin( (M_PI/180.0)*(a) ) )

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};

typedef void (*glWdgtVectorSignal) (GObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkContainerClass *parent_class;

static gint wdgt_vector_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_wdgt_vector_class_init (glWdgtVectorClass * class);
static void gl_wdgt_vector_instance_init (glWdgtVector * vector);
static void gl_wdgt_vector_finalize (GObject * object);
static void gl_wdgt_vector_construct (glWdgtVector * vector, gchar * label);
static void changed_cb (glWdgtVector * vector);

/*================================================================*/
/* Boilerplate Object stuff.                                      */
/*================================================================*/
guint
gl_wdgt_vector_get_type (void)
{
	static guint wdgt_vector_type = 0;

	if (!wdgt_vector_type) {
		GTypeInfo wdgt_vector_info = {
			sizeof (glWdgtVectorClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_wdgt_vector_class_init,
			NULL,
			NULL,
			sizeof (glWdgtVector),
			0,
			(GInstanceInitFunc) gl_wdgt_vector_instance_init,
		};

		wdgt_vector_type =
			g_type_register_static (gtk_vbox_get_type (),
						"glWdgtVector",
						&wdgt_vector_info, 0);
	}

	return wdgt_vector_type;
}

static void
gl_wdgt_vector_class_init (glWdgtVectorClass * class)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	object_class->finalize = gl_wdgt_vector_finalize;

	wdgt_vector_signals[CHANGED] =
	    g_signal_new ("changed",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glWdgtVectorClass, changed),
			  NULL, NULL,
			  gl_marshal_VOID__VOID,
			  G_TYPE_NONE, 0);

}

static void
gl_wdgt_vector_instance_init (glWdgtVector * vector)
{
	vector->len_spin = NULL;
	vector->angle_spin = NULL;
}

static void
gl_wdgt_vector_finalize (GObject * object)
{
	glWdgtVector *vector;
	glWdgtVectorClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WDGT_VECTOR (object));

	vector = GL_WDGT_VECTOR (object);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

GtkWidget *
gl_wdgt_vector_new (gchar * label)
{
	glWdgtVector *vector;

	vector = g_object_new (gl_wdgt_vector_get_type (), NULL);

	gl_wdgt_vector_construct (vector, label);

	return GTK_WIDGET (vector);
}

/*============================================================*/
/* Construct composite widget.                                */
/*============================================================*/
static void
gl_wdgt_vector_construct (glWdgtVector * vector,
			  gchar * label)
{
	GtkWidget *wvbox, *wframe, *wtable, *wlabel;
	GtkObject *adjust;
	const gchar *units_string;
	gdouble units_per_point, climb_rate;
	gint digits;

	units_string = gl_prefs_get_units_string ();
	units_per_point = gl_prefs_get_units_per_point ();
	climb_rate = gl_prefs_get_units_step_size ();
	digits = gl_prefs_get_units_precision ();

	wvbox = GTK_WIDGET (vector);

	wframe = gtk_frame_new (label);
	gtk_box_pack_start (GTK_BOX (wvbox), wframe, FALSE, FALSE, 0);

	wtable = gtk_table_new (2, 3, TRUE);
	gtk_container_set_border_width (GTK_CONTAINER (wtable), 10);
	gtk_table_set_row_spacings (GTK_TABLE (wtable), 5);
	gtk_table_set_col_spacings (GTK_TABLE (wtable), 5);
	gtk_container_add (GTK_CONTAINER (wframe), wtable);

	/* Length label */
	wlabel = gtk_label_new (_("Length:"));
	gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
	gtk_label_set_justify (GTK_LABEL (wlabel), GTK_JUSTIFY_RIGHT);
	gtk_table_attach_defaults (GTK_TABLE (wtable), wlabel, 0, 1, 0, 1);
	/* Length spin */
	adjust = gtk_adjustment_new (climb_rate, climb_rate, 100.0, climb_rate,
				     10.0, 10.0);
	vector->len_spin = gtk_spin_button_new (GTK_ADJUSTMENT (adjust),
						climb_rate, digits);
	gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (vector->len_spin),
					   TRUE);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (vector->len_spin), TRUE);
	g_signal_connect_swapped (G_OBJECT (vector->len_spin), "changed",
				  G_CALLBACK (changed_cb),
				  G_OBJECT (vector));
	gtk_table_attach_defaults (GTK_TABLE (wtable), vector->len_spin, 1, 2,
				   0, 1);
	/* Length units label */
	vector->len_units_label = gtk_label_new (units_string);
	gtk_misc_set_alignment (GTK_MISC (vector->len_units_label), 0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (wtable), vector->len_units_label,
				   2, 3, 0, 1);

	/* Angle label */
	wlabel = gtk_label_new (_("Angle:"));
	gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (wtable), wlabel, 0, 1, 1, 2);
	/* Angle spin */
	adjust = gtk_adjustment_new (0.0, -180.0, +180.0, 1.0, 10.0, 10.0);
	vector->angle_spin =
	    gtk_spin_button_new (GTK_ADJUSTMENT (adjust), 1.0, 0);
	gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (vector->angle_spin),
					   TRUE);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (vector->angle_spin),
				     TRUE);
	g_signal_connect_swapped (G_OBJECT (vector->angle_spin), "changed",
				  G_CALLBACK (changed_cb),
				  G_OBJECT (vector));
	gtk_table_attach_defaults (GTK_TABLE (wtable), vector->angle_spin, 1, 2,
				   1, 2);
	/* Angle units label */
	vector->angle_units_label = gtk_label_new (_("degrees"));
	gtk_misc_set_alignment (GTK_MISC (vector->angle_units_label), 0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (wtable),
				   vector->angle_units_label, 2, 3, 1, 2);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the widget has changed.       */
/*--------------------------------------------------------------------------*/
static void
changed_cb (glWdgtVector * vector)
{
	/* Emit our "changed" signal */
	g_signal_emit (G_OBJECT (vector), wdgt_vector_signals[CHANGED], 0);
}

/*====================================================================*/
/* query values from controls.                                        */
/*====================================================================*/
void
gl_wdgt_vector_get_params (glWdgtVector * vector,
			   gdouble * dx,
			   gdouble * dy)
{
	gdouble length, angle;
	gdouble units_per_point;

	units_per_point = gl_prefs_get_units_per_point ();

	length =
	    gtk_spin_button_get_value (GTK_SPIN_BUTTON(vector->len_spin));
	angle =
	    gtk_spin_button_get_value (GTK_SPIN_BUTTON(vector->angle_spin));

	length /= units_per_point;

	*dx = COMP_X (length, angle);
	*dy = COMP_Y (length, angle);
}

/*====================================================================*/
/* set values and ranges for controls.                                */
/*====================================================================*/
void
gl_wdgt_vector_set_params (glWdgtVector * vector,
			   gdouble dx,
			   gdouble dy,
			   gdouble x_max,
			   gdouble y_max)
{
	GtkObject *length_adjust;
	gdouble length, angle;
	const gchar *units_string;
	gdouble units_per_point, climb_rate;
	gint digits;

	units_string = gl_prefs_get_units_string ();
	units_per_point = gl_prefs_get_units_per_point ();
	climb_rate = gl_prefs_get_units_step_size ();
	digits = gl_prefs_get_units_precision ();

	/* Put everything into our display units */
	dx *= units_per_point;
	dy *= units_per_point;
	x_max *= units_per_point;
	y_max *= units_per_point;

	length = LENGTH (dx, dy);
	angle = ANGLE (dx, dy);

	length_adjust = gtk_adjustment_new (length, 0.0, LENGTH (x_max, y_max),
					    climb_rate, 10.0, 10.0);
	gtk_spin_button_configure (GTK_SPIN_BUTTON (vector->len_spin),
				   GTK_ADJUSTMENT (length_adjust),
				   climb_rate, digits);
	gtk_spin_button_update (GTK_SPIN_BUTTON (vector->len_spin));

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (vector->angle_spin), angle);
}
