/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prop_fill.c:  fill properties widget module
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

#include "prop_fill.h"

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

typedef void (*glPropFillSignal) (GtkObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkContainerClass *parent_class;

static gint prop_fill_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_prop_fill_class_init (glPropFillClass * class);
static void gl_prop_fill_init (glPropFill * fill);
static void gl_prop_fill_destroy (GtkObject * object);
static void gl_prop_fill_construct (glPropFill * fill, gchar * label);
static void changed_cb (glPropFill * fill);

/*================================================================*/
/* Boilerplate Object stuff.                                      */
/*================================================================*/
guint
gl_prop_fill_get_type (void)
{
	static guint prop_fill_type = 0;

	if (!prop_fill_type) {
		GtkTypeInfo prop_fill_info = {
			"glPropFill",
			sizeof (glPropFill),
			sizeof (glPropFillClass),
			(GtkClassInitFunc) gl_prop_fill_class_init,
			(GtkObjectInitFunc) gl_prop_fill_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
		};

		prop_fill_type =
		    gtk_type_unique (gtk_vbox_get_type (), &prop_fill_info);
	}

	return prop_fill_type;
}

static void
gl_prop_fill_class_init (glPropFillClass * class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	object_class->destroy = gl_prop_fill_destroy;

	prop_fill_signals[CHANGED] =
	    gtk_signal_new ("changed", GTK_RUN_LAST, object_class->type,
			    GTK_SIGNAL_OFFSET (glPropFillClass, changed),
			    gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals (object_class, prop_fill_signals,
				      LAST_SIGNAL);

	class->changed = NULL;
}

static void
gl_prop_fill_init (glPropFill * fill)
{
	fill->color_picker = NULL;
}

static void
gl_prop_fill_destroy (GtkObject * object)
{
	glPropFill *fill;
	glPropFillClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_PROP_FILL (object));

	fill = GL_PROP_FILL (object);
	class = GL_PROP_FILL_CLASS (GTK_OBJECT (fill)->klass);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

GtkWidget *
gl_prop_fill_new (gchar * label)
{
	glPropFill *fill;

	fill = gtk_type_new (gl_prop_fill_get_type ());

	gl_prop_fill_construct (fill, label);

	return GTK_WIDGET (fill);
}

/*============================================================*/
/* Construct composite widget.                                */
/*============================================================*/
static void
gl_prop_fill_construct (glPropFill * fill,
			gchar * label)
{
	GtkWidget *wvbox, *wframe, *wtable, *wlabel;

	wvbox = GTK_WIDGET (fill);

	wframe = gtk_frame_new (label);
	gtk_box_pack_start (GTK_BOX (wvbox), wframe, FALSE, FALSE, 0);

	wtable = gtk_table_new (1, 3, TRUE);
	gtk_container_set_border_width (GTK_CONTAINER (wtable), 10);
	gtk_table_set_row_spacings (GTK_TABLE (wtable), 5);
	gtk_table_set_col_spacings (GTK_TABLE (wtable), 5);
	gtk_container_add (GTK_CONTAINER (wframe), wtable);

	/* Fill Color Label */
	wlabel = gtk_label_new (_("Color:"));
	gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
	gtk_label_set_justify (GTK_LABEL (wlabel), GTK_JUSTIFY_RIGHT);
	gtk_table_attach_defaults (GTK_TABLE (wtable), wlabel, 0, 1, 0, 1);

	/* Fill Color picker widget */
	fill->color_picker = gnome_color_picker_new ();
	gtk_signal_connect_object (GTK_OBJECT (fill->color_picker), "color_set",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (fill));
	gtk_table_attach_defaults (GTK_TABLE (wtable), fill->color_picker, 1, 3,
				   0, 1);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the widget has changed.       */
/*--------------------------------------------------------------------------*/
static void
changed_cb (glPropFill * fill)
{
	/* Emit our "changed" signal */
	gtk_signal_emit (GTK_OBJECT (fill), prop_fill_signals[CHANGED]);
}

/*====================================================================*/
/* query values from controls.                                        */
/*====================================================================*/
void
gl_prop_fill_get_params (glPropFill * fill,
			 guint * color)
{
	guint8 r, g, b, a;

	gnome_color_picker_get_i8 (GNOME_COLOR_PICKER (fill->color_picker),
				   &r, &g, &b, &a);
	*color = GNOME_CANVAS_COLOR_A (r, g, b, a);
}

/*====================================================================*/
/* fill in values and ranges for controls.                            */
/*====================================================================*/
void
gl_prop_fill_set_params (glPropFill * fill,
			 guint color)
{
	gnome_color_picker_set_i8 (GNOME_COLOR_PICKER (fill->color_picker),
				   RED (color), GREEN (color), BLUE (color),
				   ALPHA (color));
}
