/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_bc_props.c:  barcode properties widget module
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

#include "wdgt-bc-props.h"
#include "marshal.h"

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

typedef void (*glWdgtBCPropsSignal) (GObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkContainerClass *parent_class;

static gint wdgt_bc_props_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_wdgt_bc_props_class_init (glWdgtBCPropsClass * class);
static void gl_wdgt_bc_props_instance_init (glWdgtBCProps * prop);
static void gl_wdgt_bc_props_finalize (GObject * object);
static void gl_wdgt_bc_props_construct (glWdgtBCProps * prop, gchar * label);
static void changed_cb (glWdgtBCProps * prop);

/*================================================================*/
/* Boilerplate Object stuff.                                      */
/*================================================================*/
guint
gl_wdgt_bc_props_get_type (void)
{
	static guint wdgt_bc_props_type = 0;

	if (!wdgt_bc_props_type) {
		GTypeInfo wdgt_bc_props_info = {
			sizeof (glWdgtBCPropsClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_wdgt_bc_props_class_init,
			NULL,
			NULL,
			sizeof (glWdgtBCProps),
			0,
			(GInstanceInitFunc) gl_wdgt_bc_props_instance_init,
		};

		wdgt_bc_props_type =
			g_type_register_static (gtk_vbox_get_type (),
						"glWdgtBCProps", &wdgt_bc_props_info, 0);
	}

	return wdgt_bc_props_type;
}

static void
gl_wdgt_bc_props_class_init (glWdgtBCPropsClass * class)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	object_class->finalize = gl_wdgt_bc_props_finalize;

	wdgt_bc_props_signals[CHANGED] =
	    g_signal_new ("changed",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glWdgtBCPropsClass, changed),
			  NULL, NULL,
			  gl_marshal_VOID__VOID,
			  G_TYPE_NONE, 0);

}

static void
gl_wdgt_bc_props_instance_init (glWdgtBCProps * prop)
{
	prop->scale_spin = NULL;
	prop->color_picker = NULL;
}

static void
gl_wdgt_bc_props_finalize (GObject * object)
{
	glWdgtBCProps *prop;
	glWdgtBCPropsClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WDGT_BC_PROPS (object));

	prop = GL_WDGT_BC_PROPS (object);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

GtkWidget *
gl_wdgt_bc_props_new (gchar * label)
{
	glWdgtBCProps *prop;

	prop = g_object_new (gl_wdgt_bc_props_get_type (), NULL);

	gl_wdgt_bc_props_construct (prop, label);

	return GTK_WIDGET (prop);
}

/*============================================================*/
/* Construct composite widget.                                */
/*============================================================*/
static void
gl_wdgt_bc_props_construct (glWdgtBCProps * prop,
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
	g_signal_connect_swapped (G_OBJECT (prop->scale_spin), "changed",
				  G_CALLBACK (changed_cb),
				  G_OBJECT (prop));
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
	g_signal_connect_swapped (G_OBJECT (prop->color_picker), "color_set",
				  G_CALLBACK (changed_cb),
				  G_OBJECT (prop));
	gtk_table_attach_defaults (GTK_TABLE (wtable), prop->color_picker, 1, 3,
				   1, 2);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the widget has changed.       */
/*--------------------------------------------------------------------------*/
static void
changed_cb (glWdgtBCProps * prop)
{
	/* Emit our "changed" signal */
	g_signal_emit (G_OBJECT (prop), wdgt_bc_props_signals[CHANGED], 0);
}

/*====================================================================*/
/* query values from controls.                                        */
/*====================================================================*/
void
gl_wdgt_bc_props_get_params (glWdgtBCProps * prop,
			     gdouble * scale,
			     guint * color)
{
	guint8 r, g, b, a;

	/* ------- Get updated scale ------ */
	*scale =
	    gtk_spin_button_get_value (GTK_SPIN_BUTTON(prop->scale_spin));
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
gl_wdgt_bc_props_set_params (glWdgtBCProps * prop,
			     gdouble scale,
			     guint color)
{
	scale *= 100.0;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (prop->scale_spin), scale);

	gnome_color_picker_set_i8 (GNOME_COLOR_PICKER (prop->color_picker),
				   RED (color), GREEN (color), BLUE (color),
				   ALPHA (color));

}
