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

#include "mygal/widget-color-combo.h"
#include "prefs.h"
#include "wdgt-bc-props.h"
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

typedef void (*glWdgtBCPropsSignal) (GObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static glHigVBoxClass *parent_class;

static gint wdgt_bc_props_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_wdgt_bc_props_class_init    (glWdgtBCPropsClass *class);
static void gl_wdgt_bc_props_instance_init (glWdgtBCProps      *prop);
static void gl_wdgt_bc_props_finalize      (GObject            *object);
static void gl_wdgt_bc_props_construct     (glWdgtBCProps      *prop);

static void changed_cb                     (glWdgtBCProps      *prop);

/***************************************************************************/
/* Boilerplate Object stuff.                                               */
/***************************************************************************/
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
			g_type_register_static (gl_hig_vbox_get_type (),
						"glWdgtBCProps",
						&wdgt_bc_props_info, 0);
	}

	return wdgt_bc_props_type;
}

static void
gl_wdgt_bc_props_class_init (glWdgtBCPropsClass *class)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) class;

	parent_class = g_type_class_peek_parent (class);

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
gl_wdgt_bc_props_instance_init (glWdgtBCProps *prop)
{
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

/***************************************************************************/
/* New widget.                                                             */
/***************************************************************************/
GtkWidget *
gl_wdgt_bc_props_new (void)
{
	glWdgtBCProps *prop;

	prop = g_object_new (gl_wdgt_bc_props_get_type (), NULL);

	gl_wdgt_bc_props_construct (prop);

	return GTK_WIDGET (prop);
}

/*------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                  */
/*------------------------------------------------------------------------*/
static void
gl_wdgt_bc_props_construct (glWdgtBCProps *prop)
{
	GtkWidget  *wvbox, *whbox, *wlabel;
	ColorGroup *cg;
	GdkColor   *gdk_color;

	wvbox = GTK_WIDGET (prop);

	/* ---- Color line ---- */
	whbox = gl_hig_hbox_new ();
	gl_hig_vbox_add_widget (GL_HIG_VBOX(wvbox), whbox);

	/* Line Color Label */
	prop->color_label = gtk_label_new (_("Color:"));
	gtk_misc_set_alignment (GTK_MISC (prop->color_label), 0, 0.5);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), prop->color_label);

	/* Line Color picker widget */
        cg = color_group_fetch ("line_color_group", NULL);
        gdk_color = gl_color_to_gdk_color (gl_prefs->default_line_color);
        prop->color_picker = color_combo_new (NULL, _("Default"), gdk_color, cg);
	color_combo_box_set_preview_relief (COLOR_COMBO(prop->color_picker),
					    GTK_RELIEF_NORMAL);
        g_free (gdk_color);
	g_signal_connect_swapped (G_OBJECT (prop->color_picker), "color_changed",
				  G_CALLBACK (changed_cb),
				  G_OBJECT (prop));
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), prop->color_picker);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the widget has changed.       */
/*--------------------------------------------------------------------------*/
static void
changed_cb (glWdgtBCProps *prop)
{
	/* Emit our "changed" signal */
	g_signal_emit (G_OBJECT (prop), wdgt_bc_props_signals[CHANGED], 0);
}

/***************************************************************************/
/* query values from controls.                                             */
/***************************************************************************/
void
gl_wdgt_bc_props_get_params (glWdgtBCProps *prop,
			     guint         *color)
{
	GdkColor *gdk_color;
	gboolean  is_default;

	gdk_color = color_combo_get_color (COLOR_COMBO(prop->color_picker),
					   &is_default);

	if (is_default) {
		*color = gl_prefs->default_line_color;
	} else {
		*color = gl_color_from_gdk_color (gdk_color);
	}
}

/***************************************************************************/
/* fill in values and ranges for controls.                                 */
/***************************************************************************/
void
gl_wdgt_bc_props_set_params (glWdgtBCProps *prop,
			     guint          color)
{
	GdkColor *gdk_color;

	gdk_color = gl_color_to_gdk_color (color);
	color_combo_set_color (COLOR_COMBO(prop->color_picker), gdk_color);
	g_free (gdk_color);
}

/****************************************************************************/
/* Set size group for internal labels                                       */
/****************************************************************************/
void
gl_wdgt_bc_props_set_label_size_group (glWdgtBCProps   *prop,
				       GtkSizeGroup    *label_size_group)
{
	gtk_size_group_add_widget (label_size_group, prop->color_label);
}

