/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prop_position.c:  position properties widget module
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

#include "prop_position.h"
#include "prefs.h"

#include "debug.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};

typedef void (*glPropPositionSignal) (GtkObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkContainerClass *parent_class;

static gint prop_position_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_prop_position_class_init (glPropPositionClass * class);
static void gl_prop_position_init (glPropPosition * position);
static void gl_prop_position_destroy (GtkObject * object);
static void gl_prop_position_construct (glPropPosition * position,
					gchar * label);
static void changed_cb (glPropPosition * position);

/*================================================================*/
/* Boilerplate Object stuff.                                      */
/*================================================================*/
guint
gl_prop_position_get_type (void)
{
	static guint prop_position_type = 0;

	if (!prop_position_type) {
		GtkTypeInfo prop_position_info = {
			"glPropPosition",
			sizeof (glPropPosition),
			sizeof (glPropPositionClass),
			(GtkClassInitFunc) gl_prop_position_class_init,
			(GtkObjectInitFunc) gl_prop_position_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
		};

		prop_position_type = gtk_type_unique (gtk_vbox_get_type (),
						      &prop_position_info);
	}

	return prop_position_type;
}

static void
gl_prop_position_class_init (glPropPositionClass * class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	object_class->destroy = gl_prop_position_destroy;

	prop_position_signals[CHANGED] =
	    gtk_signal_new ("changed", GTK_RUN_LAST, object_class->type,
			    GTK_SIGNAL_OFFSET (glPropPositionClass, changed),
			    gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals (object_class, prop_position_signals,
				      LAST_SIGNAL);

	class->changed = NULL;
}

static void
gl_prop_position_init (glPropPosition * position)
{
	position->x_spin = NULL;
	position->y_spin = NULL;
}

static void
gl_prop_position_destroy (GtkObject * object)
{
	glPropPosition *position;
	glPropPositionClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_PROP_POSITION (object));

	position = GL_PROP_POSITION (object);
	class = GL_PROP_POSITION_CLASS (GTK_OBJECT (position)->klass);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

GtkWidget *
gl_prop_position_new (gchar * label)
{
	glPropPosition *position;

	position = gtk_type_new (gl_prop_position_get_type ());

	gl_prop_position_construct (position, label);

	return GTK_WIDGET (position);
}

/*============================================================*/
/* Construct composite widget.                                */
/*============================================================*/
static void
gl_prop_position_construct (glPropPosition * position,
			    gchar * label)
{
	GtkWidget *wvbox, *wframe, *wtable, *wlabel;
	GtkObject *x_adjust, *y_adjust;
	const gchar *units_string;
	gdouble units_per_point, climb_rate;
	gint digits;

	units_string = gl_prefs_get_units_string ();
	units_per_point = gl_prefs_get_units_per_point ();
	climb_rate = gl_prefs_get_units_step_size ();
	digits = gl_prefs_get_units_precision ();

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
	gtk_signal_connect_object (GTK_OBJECT (position->x_spin), "changed",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (position));
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
	gtk_signal_connect_object (GTK_OBJECT (position->y_spin), "changed",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (position));
	gtk_table_attach_defaults (GTK_TABLE (wtable), position->y_spin,
				   1, 2, 1, 2);

	/* Units */
	position->units_label = gtk_label_new (units_string);
	gtk_misc_set_alignment (GTK_MISC (position->units_label), 0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (wtable),
				   position->units_label, 2, 3, 1, 2);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the widget has changed.       */
/*--------------------------------------------------------------------------*/
static void
changed_cb (glPropPosition * position)
{
	/* Emit our "changed" signal */
	gtk_signal_emit (GTK_OBJECT (position), prop_position_signals[CHANGED]);
}

/*====================================================================*/
/* query values from controls.                                        */
/*====================================================================*/
void
gl_prop_position_get_position (glPropPosition * position,
			       gdouble * x,
			       gdouble * y)
{
	gdouble units_per_point;

	units_per_point = gl_prefs_get_units_per_point ();

	*x = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON
						 (position->x_spin));
	*y = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON
						 (position->y_spin));

	/* convert everything back to our internal units (points) */
	*x /= units_per_point;
	*y /= units_per_point;
}

/*====================================================================*/
/* fill in values and ranges for controls.                            */
/*====================================================================*/
void
gl_prop_position_set_params (glPropPosition * position,
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
