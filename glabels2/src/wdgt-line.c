/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_line.c:  line properties widget module
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

#include "wdgt-line.h"
#include "marshal.h"
#include "color.h"

#include "debug.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};

typedef void (*glWdgtLineSignal) (GObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GObjectClass *parent_class;

static gint wdgt_line_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_wdgt_line_class_init (glWdgtLineClass * class);
static void gl_wdgt_line_instance_init (glWdgtLine * line);
static void gl_wdgt_line_finalize (GObject * object);
static void gl_wdgt_line_construct (glWdgtLine * line, gchar * label);
static void changed_cb (glWdgtLine * line);

/*================================================================*/
/* Boilerplate Object stuff.                                      */
/*================================================================*/
guint
gl_wdgt_line_get_type (void)
{
	static guint wdgt_line_type = 0;

	if (!wdgt_line_type) {
		GTypeInfo wdgt_line_info = {
			sizeof (glWdgtLineClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_wdgt_line_class_init,
			NULL,
			NULL,
			sizeof (glWdgtLine),
			0,
			(GInstanceInitFunc) gl_wdgt_line_instance_init,
		};

		wdgt_line_type =
		    g_type_register_static (gtk_vbox_get_type (),
					    "glWdgtLine",
					    &wdgt_line_info, 0);
	}

	return wdgt_line_type;
}

static void
gl_wdgt_line_class_init (glWdgtLineClass * class)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	object_class->finalize = gl_wdgt_line_finalize;

	wdgt_line_signals[CHANGED] =
	    g_signal_new ("changed",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glWdgtLineClass, changed),
			  NULL, NULL,
			  gl_marshal_VOID__VOID,
			  G_TYPE_NONE, 0);

}

static void
gl_wdgt_line_instance_init (glWdgtLine * line)
{
	line->width_spin = NULL;
	line->color_picker = NULL;
	line->units_label = NULL;
}

static void
gl_wdgt_line_finalize (GObject * object)
{
	glWdgtLine *line;
	glWdgtLineClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WDGT_LINE (object));

	line = GL_WDGT_LINE (object);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

GtkWidget *
gl_wdgt_line_new (gchar * label)
{
	glWdgtLine *line;

	line = g_object_new (gl_wdgt_line_get_type (), NULL);

	gl_wdgt_line_construct (line, label);

	return GTK_WIDGET (line);
}

/*============================================================*/
/* Construct composite widget.                                */
/*============================================================*/
static void
gl_wdgt_line_construct (glWdgtLine * line,
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
	g_signal_connect_swapped (G_OBJECT (line->width_spin), "changed",
				  G_CALLBACK (changed_cb),
				  G_OBJECT (line));
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
	g_signal_connect_swapped (G_OBJECT (line->color_picker), "color_set",
				  G_CALLBACK (changed_cb),
				  G_OBJECT (line));
	gtk_table_attach_defaults (GTK_TABLE (wtable), line->color_picker, 1, 3,
				   1, 2);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the widget has changed.       */
/*--------------------------------------------------------------------------*/
static void
changed_cb (glWdgtLine * line)
{
	/* Emit our "changed" signal */
	g_signal_emit (G_OBJECT (line), wdgt_line_signals[CHANGED], 0);
}

/*====================================================================*/
/* query values from controls.                                        */
/*====================================================================*/
void
gl_wdgt_line_get_params (glWdgtLine * line,
			 gdouble * width,
			 guint * color)
{
	guint8 r, g, b, a;

	*width =
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
					      (line->width_spin));

	gnome_color_picker_get_i8 (GNOME_COLOR_PICKER (line->color_picker),
				   &r, &g, &b, &a);
	*color = GL_COLOR_A (r, g, b, a);
}

/*====================================================================*/
/* fill in values and ranges for controls.                            */
/*====================================================================*/
void
gl_wdgt_line_set_params (glWdgtLine * line,
			 gdouble width,
			 guint color)
{
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (line->width_spin), width);

	gnome_color_picker_set_i8 (GNOME_COLOR_PICKER (line->color_picker),
				   GL_COLOR_I_RED (color),
				   GL_COLOR_I_GREEN (color),
				   GL_COLOR_I_BLUE (color),
				   GL_COLOR_I_ALPHA (color));
}
