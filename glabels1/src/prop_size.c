/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prop_size.c:  size properties widget module
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

#include "prop_size.h"
#include "prefs.h"

#include "debug.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};

typedef void (*glPropSizeSignal) (GtkObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkContainerClass *parent_class;

static gint prop_size_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_prop_size_class_init (glPropSizeClass * class);
static void gl_prop_size_init (glPropSize * size);
static void gl_prop_size_destroy (GtkObject * object);
static void gl_prop_size_construct (glPropSize * size, gchar * label);

static void aspect_toggle_cb (GtkToggleButton * togglebutton,
			      gpointer user_data);
static void w_spin_cb (GtkSpinButton * spinbutton, gpointer user_data);
static void h_spin_cb (GtkSpinButton * spinbutton, gpointer user_data);

/*================================================================*/
/* Boilerplate Object stuff.                                      */
/*================================================================*/
guint
gl_prop_size_get_type (void)
{
	static guint prop_size_type = 0;

	if (!prop_size_type) {
		GtkTypeInfo prop_size_info = {
			"glPropSize",
			sizeof (glPropSize),
			sizeof (glPropSizeClass),
			(GtkClassInitFunc) gl_prop_size_class_init,
			(GtkObjectInitFunc) gl_prop_size_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
		};

		prop_size_type =
		    gtk_type_unique (gtk_vbox_get_type (), &prop_size_info);
	}

	return prop_size_type;
}

static void
gl_prop_size_class_init (glPropSizeClass * class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	object_class->destroy = gl_prop_size_destroy;

	prop_size_signals[CHANGED] =
	    gtk_signal_new ("changed", GTK_RUN_LAST, object_class->type,
			    GTK_SIGNAL_OFFSET (glPropSizeClass, changed),
			    gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals (object_class, prop_size_signals,
				      LAST_SIGNAL);

	class->changed = NULL;
}

static void
gl_prop_size_init (glPropSize * size)
{
	size->aspect_ratio = 1.0;

	size->w_spin = NULL;
	size->h_spin = NULL;

	size->units_label = NULL;

	size->aspect_checkbox = NULL;
}

static void
gl_prop_size_destroy (GtkObject * object)
{
	glPropSize *size;
	glPropSizeClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_PROP_SIZE (object));

	size = GL_PROP_SIZE (object);
	class = GL_PROP_SIZE_CLASS (GTK_OBJECT (size)->klass);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

GtkWidget *
gl_prop_size_new (gchar * label)
{
	glPropSize *size;

	size = gtk_type_new (gl_prop_size_get_type ());

	gl_prop_size_construct (size, label);

	return GTK_WIDGET (size);
}

/*============================================================*/
/* Construct composite widget.                                */
/*============================================================*/
static void
gl_prop_size_construct (glPropSize * size,
			gchar * label)
{
	GtkWidget *wvbox, *wframe, *wtable, *wlabel;
	GtkObject *w_adjust, *h_adjust;
	const gchar *units_string;
	gdouble units_per_point, climb_rate;
	gint digits;

	units_string = gl_prefs_get_units_string ();
	units_per_point = gl_prefs_get_units_per_point ();
	climb_rate = gl_prefs_get_units_step_size ();
	digits = gl_prefs_get_units_precision ();

	wvbox = GTK_WIDGET (size);

	wframe = gtk_frame_new (label);
	gtk_box_pack_start (GTK_BOX (wvbox), wframe, FALSE, FALSE, 0);

	wtable = gtk_table_new (3, 3, TRUE);
	gtk_container_set_border_width (GTK_CONTAINER (wtable), 10);
	gtk_table_set_row_spacings (GTK_TABLE (wtable), 5);
	gtk_table_set_col_spacings (GTK_TABLE (wtable), 5);
	gtk_container_add (GTK_CONTAINER (wframe), wtable);

	/* W Label */
	wlabel = gtk_label_new (_("Width:"));
	gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
	gtk_label_set_justify (GTK_LABEL (wlabel), GTK_JUSTIFY_RIGHT);
	gtk_table_attach_defaults (GTK_TABLE (wtable), wlabel, 0, 1, 0, 1);
	/* W spin */
	w_adjust = gtk_adjustment_new (climb_rate, climb_rate, 100.0,
				       climb_rate, 10.0, 10.0);
	size->w_spin = gtk_spin_button_new (GTK_ADJUSTMENT (w_adjust),
					    climb_rate, digits);
	gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (size->w_spin),
					   TRUE);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (size->w_spin), TRUE);
	gtk_table_attach_defaults (GTK_TABLE (wtable), size->w_spin,
				   1, 2, 0, 1);

	/* H label */
	wlabel = gtk_label_new (_("Height:"));
	gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
	gtk_label_set_justify (GTK_LABEL (wlabel), GTK_JUSTIFY_RIGHT);
	gtk_table_attach_defaults (GTK_TABLE (wtable), wlabel, 0, 1, 1, 2);
	/* H spin */
	h_adjust = gtk_adjustment_new (climb_rate, climb_rate,
				       100.0, climb_rate, 10.0, 10.0);
	size->h_spin = gtk_spin_button_new (GTK_ADJUSTMENT (h_adjust),
					    climb_rate, digits);
	gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (size->h_spin),
					   TRUE);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (size->h_spin), TRUE);
	gtk_table_attach_defaults (GTK_TABLE (wtable), size->h_spin,
				   1, 2, 1, 2);

	/* Units */
	size->units_label = gtk_label_new (units_string);
	gtk_misc_set_alignment (GTK_MISC (size->units_label), 0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (wtable),
				   size->units_label, 2, 3, 1, 2);

	/* Maintain aspect ratio checkbox */
	size->aspect_checkbox =
	    gtk_check_button_new_with_label (_
					     ("Maintain current aspect ratio"));
	gtk_table_attach_defaults (GTK_TABLE (wtable), size->aspect_checkbox, 0,
				   3, 2, 3);

	/* Connect signals to controls */
	gtk_signal_connect (GTK_OBJECT (size->aspect_checkbox), "toggled",
			    GTK_SIGNAL_FUNC (aspect_toggle_cb), size);
	gtk_signal_connect (GTK_OBJECT (size->w_spin), "changed",
			    GTK_SIGNAL_FUNC (w_spin_cb), size);
	gtk_signal_connect (GTK_OBJECT (size->h_spin), "changed",
			    GTK_SIGNAL_FUNC (h_spin_cb), size);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Maintain aspect ratio checkbox callback.                       */
/*--------------------------------------------------------------------------*/
static void
aspect_toggle_cb (GtkToggleButton * togglebutton,
		  gpointer user_data)
{
	glPropSize *size = GL_PROP_SIZE (user_data);
	GtkAdjustment *w_adjust, *h_adjust;

	if (gtk_toggle_button_get_active (togglebutton)) {

		size->w =
		    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON
							(size->w_spin));
		size->h =
		    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON
							(size->h_spin));

		size->aspect_ratio = size->h / size->w;

		/* We have a new aspect ratio, adjust one of the maxes accordingly */
		if ((size->h_max_orig / size->w_max_orig) < size->aspect_ratio) {
			size->w_max = size->h_max_orig / size->aspect_ratio;
			size->h_max = size->h_max_orig;
		} else {
			size->w_max = size->w_max_orig;
			size->h_max = size->w_max_orig * size->aspect_ratio;
		}

	} else {

		/* Reset maximums */
		size->w_max = size->w_max_orig;
		size->h_max = size->h_max_orig;

	}

	gtk_signal_handler_block_by_func (GTK_OBJECT (size->w_spin),
					  GTK_SIGNAL_FUNC (w_spin_cb),
					  user_data);
	gtk_signal_handler_block_by_func (GTK_OBJECT (size->h_spin),
					  GTK_SIGNAL_FUNC (h_spin_cb),
					  user_data);
	w_adjust =
	    gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (size->w_spin));
	w_adjust->upper = size->w_max;
	gtk_spin_button_update (GTK_SPIN_BUTTON (size->w_spin));
	h_adjust =
	    gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (size->h_spin));
	h_adjust->upper = size->h_max;
	gtk_spin_button_update (GTK_SPIN_BUTTON (size->h_spin));
	gtk_signal_handler_unblock_by_func (GTK_OBJECT (size->w_spin),
					    GTK_SIGNAL_FUNC (w_spin_cb),
					    user_data);
	gtk_signal_handler_unblock_by_func (GTK_OBJECT (size->h_spin),
					    GTK_SIGNAL_FUNC (h_spin_cb),
					    user_data);

	/* Emit our "changed" signal */
	gtk_signal_emit (GTK_OBJECT (size), prop_size_signals[CHANGED]);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  W spin button changed callback.                                */
/*--------------------------------------------------------------------------*/
static void
w_spin_cb (GtkSpinButton * spinbutton,
	   gpointer user_data)
{
	glPropSize *size = GL_PROP_SIZE (user_data);
	GtkToggleButton *toggle = GTK_TOGGLE_BUTTON (size->aspect_checkbox);

	size->w =
	    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (size->w_spin));

	if (gtk_toggle_button_get_active (toggle)) {

		size->h = size->w * size->aspect_ratio;

		/* Update our sibling control, blocking recursion. */
		gtk_signal_handler_block_by_func (GTK_OBJECT (size->h_spin),
						  GTK_SIGNAL_FUNC (h_spin_cb),
						  user_data);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (size->h_spin),
					   size->h);
		gtk_signal_handler_unblock_by_func (GTK_OBJECT (size->h_spin),
						    GTK_SIGNAL_FUNC (h_spin_cb),
						    user_data);
	}

	/* Emit our "changed" signal */
	gtk_signal_emit (GTK_OBJECT (size), prop_size_signals[CHANGED]);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  H spin button changed callback.                                */
/*--------------------------------------------------------------------------*/
static void
h_spin_cb (GtkSpinButton * spinbutton,
	   gpointer user_data)
{
	glPropSize *size = GL_PROP_SIZE (user_data);
	GtkToggleButton *toggle = GTK_TOGGLE_BUTTON (size->aspect_checkbox);

	size->h =
	    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (size->h_spin));

	if (gtk_toggle_button_get_active (toggle)) {

		size->w = size->h / size->aspect_ratio;

		/* Update our sibling control, blocking recursion. */
		gtk_signal_handler_block_by_func (GTK_OBJECT (size->w_spin),
						  GTK_SIGNAL_FUNC (w_spin_cb),
						  user_data);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (size->w_spin),
					   size->w);
		gtk_signal_handler_unblock_by_func (GTK_OBJECT (size->w_spin),
						    GTK_SIGNAL_FUNC (w_spin_cb),
						    user_data);
	}

	/* Emit our "changed" signal */
	gtk_signal_emit (GTK_OBJECT (size), prop_size_signals[CHANGED]);

}

/*====================================================================*/
/* query values from controls.                                        */
/*====================================================================*/
void
gl_prop_size_get_size (glPropSize * size,
		       gdouble * w,
		       gdouble * h,
		       gboolean * keep_aspect_ratio_flag)
{
	gdouble units_per_point;

	units_per_point = gl_prefs_get_units_per_point ();

	*w = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON
						 (size->w_spin));
	*h = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON
						 (size->h_spin));

	*keep_aspect_ratio_flag =
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
					  (size->aspect_checkbox));

	/* convert everything back to our internal units (points) */
	*w /= units_per_point;
	*h /= units_per_point;
}

/*====================================================================*/
/* set values and ranges for controls.                                */
/*====================================================================*/
void
gl_prop_size_set_params (glPropSize * size,
			 gdouble w,
			 gdouble h,
			 gboolean keep_aspect_ratio_flag,
			 gdouble w_max,
			 gdouble h_max)
{
	GtkObject *w_adjust, *h_adjust;
	const gchar *units_string;
	gdouble units_per_point, climb_rate;
	gint digits;

	units_string = gl_prefs_get_units_string ();
	units_per_point = gl_prefs_get_units_per_point ();
	climb_rate = gl_prefs_get_units_step_size ();
	digits = gl_prefs_get_units_precision ();

	/* Put everything into our display units */
	size->w = w * units_per_point;
	size->h = h * units_per_point;
	size->w_max = w_max * units_per_point;
	size->h_max = h_max * units_per_point;

	/* Squirrel away copies of our original maximums */
	size->w_max_orig = size->w_max;
	size->h_max_orig = size->h_max;

	size->aspect_ratio = size->h / size->w;
	if (keep_aspect_ratio_flag) {

		/* When tracking aspect ratio, adjust one of the maxes */
		if ((size->h_max / size->w_max) < size->aspect_ratio) {
			size->w_max = size->h_max / size->aspect_ratio;
		} else {
			size->h_max = size->w_max * size->aspect_ratio;
		}

		/* before adjusting w & h, limit to max values */
		if (size->w > size->w_max)
			size->w = size->w_max;
		if (size->h > size->h_max)
			size->h = size->h_max;

	}

	/* update W/H spin controls */
	w_adjust = gtk_adjustment_new (size->w, climb_rate, size->w_max,
				       climb_rate, 10.0, 10.0);
	gtk_spin_button_configure (GTK_SPIN_BUTTON (size->w_spin),
				   GTK_ADJUSTMENT (w_adjust), climb_rate,
				   digits);
	h_adjust =
	    gtk_adjustment_new (size->h, climb_rate, size->h_max, climb_rate,
				10.0, 10.0);
	gtk_spin_button_configure (GTK_SPIN_BUTTON (size->h_spin),
				   GTK_ADJUSTMENT (h_adjust), climb_rate,
				   digits);

	gtk_label_set_text (GTK_LABEL (size->units_label), units_string);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (size->aspect_checkbox),
				      keep_aspect_ratio_flag);

}
