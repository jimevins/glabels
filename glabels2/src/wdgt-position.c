/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_position.c:  position properties widget module
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

#include "wdgt-position.h"
#include "prefs.h"
#include "marshal.h"

#include "debug.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};

typedef void (*glWdgtPositionSignal) (GObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GObjectClass *parent_class;

static gint wdgt_position_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_wdgt_position_class_init (glWdgtPositionClass * class);
static void gl_wdgt_position_instance_init (glWdgtPosition * position);
static void gl_wdgt_position_finalize (GObject * object);
static void gl_wdgt_position_construct (glWdgtPosition * position,
					gchar * label);
static void changed_cb (glWdgtPosition * position);

/*================================================================*/
/* Boilerplate Object stuff.                                      */
/*================================================================*/
guint
gl_wdgt_position_get_type (void)
{
	static guint wdgt_position_type = 0;

	if (!wdgt_position_type) {
		GTypeInfo wdgt_position_info = {
			sizeof (glWdgtPositionClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_wdgt_position_class_init,
			NULL,
			NULL,
			sizeof (glWdgtPosition),
			0,
			(GInstanceInitFunc) gl_wdgt_position_instance_init,
		};

		wdgt_position_type = g_type_register_static (gtk_vbox_get_type (),
							     "glWdgtPosition",
							     &wdgt_position_info,
							     0);
	}

	return wdgt_position_type;
}

static void
gl_wdgt_position_class_init (glWdgtPositionClass * class)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	object_class->finalize = gl_wdgt_position_finalize;

	wdgt_position_signals[CHANGED] =
	    g_signal_new ("changed",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glWdgtPositionClass, changed),
			  NULL, NULL,
			  gl_marshal_VOID__VOID,
			  G_TYPE_NONE, 0);

}

static void
gl_wdgt_position_instance_init (glWdgtPosition * position)
{
	position->x_spin = NULL;
	position->y_spin = NULL;
}

static void
gl_wdgt_position_finalize (GObject * object)
{
	glWdgtPosition *position;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WDGT_POSITION (object));

	position = GL_WDGT_POSITION (object);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

GtkWidget *
gl_wdgt_position_new (gchar * label)
{
	glWdgtPosition *position;

	position = g_object_new (gl_wdgt_position_get_type (), NULL);

	gl_wdgt_position_construct (position, label);

	return GTK_WIDGET (position);
}

/*============================================================*/
/* Construct composite widget.                                */
/*============================================================*/
static void
gl_wdgt_position_construct (glWdgtPosition * position,
			    gchar * label)
{
	GtkWidget *wvbox, *wframe, *wtable, *wlabel;
	GtkObject *x_adjust, *y_adjust;
	const gchar *units_string;
	gdouble units_per_point, climb_rate;
	gint digits;

	gl_debug (DEBUG_WDGT, "START");

	units_string = gl_prefs_get_units_string ();
	units_per_point = gl_prefs_get_units_per_point ();
	climb_rate = gl_prefs_get_units_step_size ();
	digits = gl_prefs_get_units_precision ();

	gl_debug (DEBUG_WDGT, "units = %s", units_string);
	gl_debug (DEBUG_WDGT, "units/point = %f", units_per_point);
	gl_debug (DEBUG_WDGT, "climb_rate = %f", climb_rate);
	gl_debug (DEBUG_WDGT, "digits = %d", digits);

	wvbox = GTK_WIDGET (position);

	wframe = gtk_frame_new (label);
	gtk_box_pack_start (GTK_BOX (wvbox), wframe, FALSE, FALSE, 0);

	wtable = gtk_table_new (2, 3, TRUE);
	gtk_container_set_border_width (GTK_CONTAINER (wtable), 10);
	gtk_table_set_row_spacings (GTK_TABLE (wtable), 5);
	gtk_table_set_col_spacings (GTK_TABLE (wtable), 5);
	gtk_container_add (GTK_CONTAINER (wframe), wtable);

	/* X label */
	wlabel = gtk_label_new (_("X:"));
	gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
	gtk_label_set_justify (GTK_LABEL (wlabel), GTK_JUSTIFY_RIGHT);
	gtk_table_attach_defaults (GTK_TABLE (wtable), wlabel, 0, 1, 0, 1);
	/* X spin */
	x_adjust = gtk_adjustment_new (0.0, 0.0, 100.0, climb_rate, 10.0, 10.0);
	position->x_spin = gtk_spin_button_new (GTK_ADJUSTMENT (x_adjust),
						climb_rate, digits);
	gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (position->x_spin),
					   TRUE);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (position->x_spin), TRUE);
	g_signal_connect_swapped (G_OBJECT (position->x_spin), "changed",
				  G_CALLBACK (changed_cb),
				  G_OBJECT (position));
	gtk_table_attach_defaults (GTK_TABLE (wtable), position->x_spin,
				   1, 2, 0, 1);

	/* Y label */
	wlabel = gtk_label_new (_("Y:"));
	gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
	gtk_label_set_justify (GTK_LABEL (wlabel), GTK_JUSTIFY_RIGHT);
	gtk_table_attach_defaults (GTK_TABLE (wtable), wlabel, 0, 1, 1, 2);
	/* Y spin */
	y_adjust = gtk_adjustment_new (0.0, 0.0, 100.0, climb_rate, 10.0, 10.0);
	position->y_spin = gtk_spin_button_new (GTK_ADJUSTMENT (y_adjust),
						climb_rate, digits);
	gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (position->y_spin),
					   TRUE);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (position->y_spin), TRUE);
	g_signal_connect_swapped (G_OBJECT (position->y_spin), "changed",
				  G_CALLBACK (changed_cb),
				  G_OBJECT (position));
	gtk_table_attach_defaults (GTK_TABLE (wtable), position->y_spin,
				   1, 2, 1, 2);

	/* Units */
	position->units_label = gtk_label_new (units_string);
	gtk_misc_set_alignment (GTK_MISC (position->units_label), 0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (wtable),
				   position->units_label, 2, 3, 1, 2);

	gl_debug (DEBUG_WDGT, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the widget has changed.       */
/*--------------------------------------------------------------------------*/
static void
changed_cb (glWdgtPosition * position)
{
	/* Emit our "changed" signal */
	g_signal_emit (G_OBJECT (position), wdgt_position_signals[CHANGED], 0);
}

/*====================================================================*/
/* query values from controls.                                        */
/*====================================================================*/
void
gl_wdgt_position_get_position (glWdgtPosition * position,
			       gdouble * x,
			       gdouble * y)
{
	gdouble units_per_point;

	units_per_point = gl_prefs_get_units_per_point ();

	*x = gtk_spin_button_get_value (GTK_SPIN_BUTTON(position->x_spin));
	*y = gtk_spin_button_get_value (GTK_SPIN_BUTTON(position->y_spin));

	/* convert everything back to our internal units (points) */
	*x /= units_per_point;
	*y /= units_per_point;
}

/*====================================================================*/
/* fill in values and ranges for controls.                            */
/*====================================================================*/
void
gl_wdgt_position_set_params (glWdgtPosition * position,
			     gdouble x,
			     gdouble y,
			     gdouble x_max,
			     gdouble y_max)
{
	GtkObject *x_adjust, *y_adjust;
	const gchar *units_string;
	gdouble units_per_point, climb_rate;
	gint digits;

	units_string = gl_prefs_get_units_string ();
	units_per_point = gl_prefs_get_units_per_point ();
	climb_rate = gl_prefs_get_units_step_size ();
	digits = gl_prefs_get_units_precision ();

	/* Put everything into our display units */
	x *= units_per_point;
	y *= units_per_point;
	x_max *= units_per_point;
	y_max *= units_per_point;

	/* update X/Y spin controls */
	x_adjust = gtk_adjustment_new (x, 0.0, x_max, climb_rate, 10.0, 10.0);
	gtk_spin_button_configure (GTK_SPIN_BUTTON (position->x_spin),
				   GTK_ADJUSTMENT (x_adjust), climb_rate,
				   digits);
	y_adjust = gtk_adjustment_new (y, 0.0, y_max, climb_rate, 10.0, 10.0);
	gtk_spin_button_configure (GTK_SPIN_BUTTON (position->y_spin),
				   GTK_ADJUSTMENT (y_adjust), climb_rate,
				   digits);

	/* Units */
	gtk_label_set_text (GTK_LABEL (position->units_label), units_string);

}

/*====================================================================*/
/* fill in position info only.                                        */
/*====================================================================*/
void
gl_wdgt_position_set_position (glWdgtPosition * position,
			       gdouble x,
			       gdouble y)
{
	gdouble units_per_point;

	gl_debug (DEBUG_WDGT, "START");

	units_per_point = gl_prefs_get_units_per_point ();

	gl_debug (DEBUG_WDGT, "units/point = %f", units_per_point);

	/* Put everything in our display units */
	x *= units_per_point;
	y *= units_per_point;

	/* update X/Y spin controls */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (position->x_spin), x);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (position->y_spin), y);

	gl_debug (DEBUG_WDGT, "END");
}
