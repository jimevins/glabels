/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prop_bc.c:  barcode properties widget module
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

#include "prop_bc.h"

#include "debug.h"

#define RED(x)   ( ((x)>>24) & 0xff )
#define GREEN(x) ( ((x)>>16) & 0xff )
#define BLUE(x)  ( ((x)>>8)  & 0xff )
#define ALPHA(x) (  (x)      & 0xff )

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};

typedef void (*glPropBCSignal) (GtkObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkContainerClass *parent_class;

static gint prop_bc_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_prop_bc_class_init (glPropBCClass * class);
static void gl_prop_bc_init (glPropBC * prop);
static void gl_prop_bc_destroy (GtkObject * object);
static void gl_prop_bc_construct (glPropBC * prop, gchar * label);
static void changed_cb (glPropBC * prop);

/*================================================================*/
/* Boilerplate Object stuff.                                      */
/*================================================================*/
guint
gl_prop_bc_get_type (void)
{
	static guint prop_bc_type = 0;

	if (!prop_bc_type) {
		GtkTypeInfo prop_bc_info = {
			"glPropBC",
			sizeof (glPropBC),
			sizeof (glPropBCClass),
			(GtkClassInitFunc) gl_prop_bc_class_init,
			(GtkObjectInitFunc) gl_prop_bc_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
		};

		prop_bc_type = gtk_type_unique (gtk_vbox_get_type (),
						&prop_bc_info);
	}

	return prop_bc_type;
}

static void
gl_prop_bc_class_init (glPropBCClass * class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	object_class->destroy = gl_prop_bc_destroy;

	prop_bc_signals[CHANGED] =
	    gtk_signal_new ("changed", GTK_RUN_LAST, object_class->type,
			    GTK_SIGNAL_OFFSET (glPropBCClass, changed),
			    gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals (object_class, prop_bc_signals,
				      LAST_SIGNAL);

	class->changed = NULL;
}

static void
gl_prop_bc_init (glPropBC * prop)
{
	prop->scale_spin = NULL;
	prop->color_picker = NULL;
}

static void
gl_prop_bc_destroy (GtkObject * object)
{
	glPropBC *prop;
	glPropBCClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_PROP_BC (object));

	prop = GL_PROP_BC (object);
	class = GL_PROP_BC_CLASS (GTK_OBJECT (prop)->klass);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

GtkWidget *
gl_prop_bc_new (gchar * label)
{
	glPropBC *prop;

	prop = gtk_type_new (gl_prop_bc_get_type ());

	gl_prop_bc_construct (prop, label);

	return GTK_WIDGET (prop);
}

/*============================================================*/
/* Construct composite widget.                                */
/*============================================================*/
static void
gl_prop_bc_construct (glPropBC * prop,
		      gchar * label)
{
	GtkWidget *wvbox, *wframe, *wtable, *wlabel;
	GtkObject *adjust;

	wvbox = GTK_WIDGET (prop);

	wframe = gtk_frame_new (label);
	gtk_box_pack_start (GTK_BOX (wvbox), wframe, FALSE, FALSE, 0);

	wtable = gtk_table_new (2, 3, TRUE);
	gtk_container_set_border_width (GTK_CONTAINER (wtable), 10);
	gtk_table_set_row_spacings (GTK_TABLE (wtable), 5);
	gtk_table_set_col_spacings (GTK_TABLE (wtable), 5);
	gtk_container_add (GTK_CONTAINER (wframe), wtable);

	/* Scale Label */
	wlabel = gtk_label_new (_("Scale:"));
	gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
	gtk_label_set_justify (GTK_LABEL (wlabel), GTK_JUSTIFY_RIGHT);
	gtk_table_attach_defaults (GTK_TABLE (wtable), wlabel, 0, 1, 0, 1);
	/* Scale widget */
	adjust = gtk_adjustment_new (100.0, 50.0, 200.0, 10.0, 10.0, 10.0);
	prop->scale_spin =
	    gtk_spin_button_new (GTK_ADJUSTMENT (adjust), 10.0, 0);
	gtk_signal_connect_object (GTK_OBJECT (prop->scale_spin), "changed",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (prop));
	gtk_table_attach_defaults (GTK_TABLE (wtable), prop->scale_spin, 1, 2,
				   0, 1);
	/* % Label */
	wlabel = gtk_label_new (_("%"));
	gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
	gtk_label_set_justify (GTK_LABEL (wlabel), GTK_JUSTIFY_RIGHT);
	gtk_table_attach_defaults (GTK_TABLE (wtable), wlabel, 2, 3, 0, 1);

	/* Line Color Label */
	wlabel = gtk_label_new (_("Color:"));
	gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
	gtk_label_set_justify (GTK_LABEL (wlabel), GTK_JUSTIFY_RIGHT);
	gtk_table_attach_defaults (GTK_TABLE (wtable), wlabel, 0, 1, 1, 2);
	/* Line Color picker widget */
	prop->color_picker = gnome_color_picker_new ();
	gtk_signal_connect_object (GTK_OBJECT (prop->color_picker), "color_set",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (prop));
	gtk_table_attach_defaults (GTK_TABLE (wtable), prop->color_picker, 1, 3,
				   1, 2);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the widget has changed.       */
/*--------------------------------------------------------------------------*/
static void
changed_cb (glPropBC * prop)
{
	/* Emit our "changed" signal */
	gtk_signal_emit (GTK_OBJECT (prop), prop_bc_signals[CHANGED]);
}

/*====================================================================*/
/* query values from controls.                                        */
/*====================================================================*/
void
gl_prop_bc_get_params (glPropBC * prop,
		       gdouble * scale,
		       guint * color)
{
	guint8 r, g, b, a;

	/* ------- Get updated scale ------ */
	*scale =
	    gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON
						(prop->scale_spin));
	*scale /= 100.0;

	/* ------- Get updated line color ------ */
	gnome_color_picker_get_i8 (GNOME_COLOR_PICKER (prop->color_picker),
				   &r, &g, &b, &a);
	*color = GNOME_CANVAS_COLOR_A (r, g, b, a);

}

/*====================================================================*/
/* fill in values and ranges for controls.                            */
/*====================================================================*/
void
gl_prop_bc_set_params (glPropBC * prop,
		       gdouble scale,
		       guint color)
{
	scale *= 100.0;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (prop->scale_spin), scale);

	gnome_color_picker_set_i8 (GNOME_COLOR_PICKER (prop->color_picker),
				   RED (color), GREEN (color), BLUE (color),
				   ALPHA (color));

}
