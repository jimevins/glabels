/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_fill.c:  fill properties widget module
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

#include "wdgt-fill.h"
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

typedef void (*glWdgtFillSignal) (GObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkContainerClass *parent_class;

static gint wdgt_fill_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_wdgt_fill_class_init (glWdgtFillClass * class);
static void gl_wdgt_fill_instance_init (glWdgtFill * fill);
static void gl_wdgt_fill_finalize (GObject * object);
static void gl_wdgt_fill_construct (glWdgtFill * fill, gchar * label);
static void changed_cb (glWdgtFill * fill);

/*================================================================*/
/* Boilerplate Object stuff.                                      */
/*================================================================*/
guint
gl_wdgt_fill_get_type (void)
{
	static guint wdgt_fill_type = 0;

	if (!wdgt_fill_type) {
		GTypeInfo wdgt_fill_info = {
			sizeof (glWdgtFillClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_wdgt_fill_class_init,
			NULL,
			NULL,
			sizeof (glWdgtFill),
			0,
			(GInstanceInitFunc) gl_wdgt_fill_instance_init,
		};

		wdgt_fill_type =
		    g_type_register_static (gtk_vbox_get_type (),
					    "glWdgtFill",
					    &wdgt_fill_info, 0);
	}

	return wdgt_fill_type;
}

static void
gl_wdgt_fill_class_init (glWdgtFillClass * class)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	object_class->finalize = gl_wdgt_fill_finalize;

	wdgt_fill_signals[CHANGED] =
	    g_signal_new ("changed",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glWdgtFillClass, changed),
			  NULL, NULL,
			  gl_marshal_VOID__VOID,
			  G_TYPE_NONE, 0);

}

static void
gl_wdgt_fill_instance_init (glWdgtFill * fill)
{
	fill->color_picker = NULL;
}

static void
gl_wdgt_fill_finalize (GObject * object)
{
	glWdgtFill *fill;
	glWdgtFillClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WDGT_FILL (object));

	fill = GL_WDGT_FILL (object);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

GtkWidget *
gl_wdgt_fill_new (gchar * label)
{
	glWdgtFill *fill;

	fill = g_object_new (gl_wdgt_fill_get_type (), NULL);

	gl_wdgt_fill_construct (fill, label);

	return GTK_WIDGET (fill);
}

/*============================================================*/
/* Construct composite widget.                                */
/*============================================================*/
static void
gl_wdgt_fill_construct (glWdgtFill * fill,
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
	g_signal_connect_swapped (G_OBJECT (fill->color_picker), "color_set",
				  G_CALLBACK (changed_cb),
				  G_OBJECT (fill));
	gtk_table_attach_defaults (GTK_TABLE (wtable), fill->color_picker, 1, 3,
				   0, 1);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the widget has changed.       */
/*--------------------------------------------------------------------------*/
static void
changed_cb (glWdgtFill * fill)
{
	/* Emit our "changed" signal */
	g_signal_emit (G_OBJECT (fill), wdgt_fill_signals[CHANGED], 0);
}

/*====================================================================*/
/* query values from controls.                                        */
/*====================================================================*/
void
gl_wdgt_fill_get_params (glWdgtFill * fill,
			 guint * color)
{
	guint8 r, g, b, a;

	gnome_color_picker_get_i8 (GNOME_COLOR_PICKER (fill->color_picker),
				   &r, &g, &b, &a);
	*color = GL_COLOR_A (r, g, b, a);
}

/*====================================================================*/
/* fill in values and ranges for controls.                            */
/*====================================================================*/
void
gl_wdgt_fill_set_params (glWdgtFill * fill,
			 guint color)
{
	gnome_color_picker_set_i8 (GNOME_COLOR_PICKER (fill->color_picker),
				   GL_COLOR_I_RED (color),
				   GL_COLOR_I_GREEN (color),
				   GL_COLOR_I_BLUE (color),
				   GL_COLOR_I_ALPHA (color));
}
