/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prop_line.c:  line properties widget module
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

#include "prop_line.h"

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

typedef void (*glPropLineSignal) (GtkObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkContainerClass *parent_class;

static gint prop_line_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_prop_line_class_init (glPropLineClass * class);
static void gl_prop_line_init (glPropLine * line);
static void gl_prop_line_destroy (GtkObject * object);
static void gl_prop_line_construct (glPropLine * line, gchar * label);
static void changed_cb (glPropLine * line);

/*================================================================*/
/* Boilerplate Object stuff.                                      */
/*================================================================*/
guint
gl_prop_line_get_type (void)
{
	static guint prop_line_type = 0;

	if (!prop_line_type) {
		GtkTypeInfo prop_line_info = {
			"glPropLine",
			sizeof (glPropLine),
			sizeof (glPropLineClass),
			(GtkClassInitFunc) gl_prop_line_class_init,
			(GtkObjectInitFunc) gl_prop_line_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
		};

		prop_line_type =
		    gtk_type_unique (gtk_vbox_get_type (), &prop_line_info);
	}

	return prop_line_type;
}

static void
gl_prop_line_class_init (glPropLineClass * class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	object_class->destroy = gl_prop_line_destroy;

	prop_line_signals[CHANGED] =
	    gtk_signal_new ("changed", GTK_RUN_LAST, object_class->type,
			    GTK_SIGNAL_OFFSET (glPropLineClass, changed),
			    gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals (object_class, prop_line_signals,
				      LAST_SIGNAL);

	class->changed = NULL;
}

static void
gl_prop_line_init (glPropLine * line)
{
	line->width_spin = NULL;
	line->color_picker = NULL;
	line->units_label = NULL;
}

static void
gl_prop_line_destroy (GtkObject * object)
{
	glPropLine *line;
	glPropLineClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_PROP_LINE (object));

	line = GL_PROP_LINE (object);
	class = GL_PROP_LINE_CLASS (GTK_OBJECT (line)->klass);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

GtkWidget *
gl_prop_line_new (gchar * label)
{
	glPropLine *line;

	line = gtk_type_new (gl_prop_line_get_type ());

	gl_prop_line_construct (line, label);

	return GTK_WIDGET (line);
}

/*============================================================*/
/* Construct composite widget.                                */
/*============================================================*/
static void
gl_prop_line_construct (glPropLine * line,
			gchar * label)
{
	GtkWidget *wvbox, *wframe, *wtable, *wlabel;
	GtkObject *adjust;

	wvbox = GTK_WIDGET (line);

	wframe = gtk_frame_new (label);
	gtk_box_pack_start (GTK_BOX (wvbox), wframe, FALSE, FALSE, 0);

	wtable = gtk_table_new (2, 3, TRUE);
	gtk_container_set_border_width (GTK_CONTAINER (wtable), 10);
	gtk_table_set_row_spacings (GTK_TABLE (wtable), 5);
	gtk_table_set_col_spacings (GTK_TABLE (wtable), 5);
	gtk_container_add (GTK_CONTAINER (wframe), wtable);

	/* Line Width Label */
	wlabel = gtk_label_new (_("Width:"));
	gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
	gtk_label_set_justify (GTK_LABEL (wlabel), GTK_JUSTIFY_RIGHT);
	gtk_table_attach_defaults (GTK_TABLE (wtable), wlabel, 0, 1, 0, 1);
	/* Line Width widget */
	adjust = gtk_adjustment_new (1.0, 0.25, 4.0, 0.25, 1.0, 1.0);
	line->width_spin =
	    gtk_spin_button_new (GTK_ADJUSTMENT (adjust), 0.25, 2);
	gtk_signal_connect_object (GTK_OBJECT (line->width_spin), "changed",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (line));
	gtk_table_attach_defaults (GTK_TABLE (wtable), line->width_spin, 1, 2,
				   0, 1);
	/* Line Width units */
	line->units_label = gtk_label_new (_("points"));
	gtk_misc_set_alignment (GTK_MISC (line->units_label), 0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (wtable), line->units_label,
				   2, 3, 0, 1);

	/* Line Color Label */
	wlabel = gtk_label_new (_("Color:"));
	gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (wtable), wlabel, 0, 1, 1, 2);
	/* Line Color picker widget */
	line->color_picker = gnome_color_picker_new ();
	gtk_signal_connect_object (GTK_OBJECT (line->color_picker), "color_set",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (line));
	gtk_table_attach_defaults (GTK_TABLE (wtable), line->color_picker, 1, 3,
				   1, 2);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the widget has changed.       */
/*--------------------------------------------------------------------------*/
static void
changed_cb (glPropLine * line)
{
	/* Emit our "changed" signal */
	gtk_signal_emit (GTK_OBJECT (line), prop_line_signals[CHANGED]);
}

/*====================================================================*/
/* query values from controls.                                        */
/*====================================================================*/
void
gl_prop_line_get_params (glPropLine * line,
			 gdouble * width,
			 guint * color)
{
	guint8 r, g, b, a;

	*width =
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
					      (line->width_spin));

	gnome_color_picker_get_i8 (GNOME_COLOR_PICKER (line->color_picker),
				   &r, &g, &b, &a);
	*color = GNOME_CANVAS_COLOR_A (r, g, b, a);
}

/*====================================================================*/
/* fill in values and ranges for controls.                            */
/*====================================================================*/
void
gl_prop_line_set_params (glPropLine * line,
			 gdouble width,
			 guint color)
{
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (line->width_spin), width);

	gnome_color_picker_set_i8 (GNOME_COLOR_PICKER (line->color_picker),
				   RED (color), GREEN (color), BLUE (color),
				   ALPHA (color));
}
