/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  object-editor.c:  object properties editor module
 *
 *  Copyright (C) 2003  Jim Evins <evins@snaught.com>.
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

#include <gnome.h>
#include <math.h>

#include "object-editor.h"
#include "prefs.h"

#include "object-editor-private.h"

#include "debug.h"

/*===========================================*/
/* Private macros                            */
/*===========================================*/

/*===========================================*/
/* Private data types                        */
/*===========================================*/

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare position page.                                         */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_prepare_position_page (glObjectEditor *editor)
{
	const gchar  *units_string;
	gdouble       climb_rate;
	gint          digits;
	GtkSizeGroup *label_size_group;
	GtkWidget    *label;

	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
	editor->priv->pos_page_vbox = glade_xml_get_widget (editor->priv->gui,
							      "pos_page_vbox");
	editor->priv->pos_x_spin    = glade_xml_get_widget (editor->priv->gui,
							      "pos_x_spin");
	editor->priv->pos_y_spin    = glade_xml_get_widget (editor->priv->gui,
							      "pos_y_spin");
	editor->priv->pos_x_units_label = glade_xml_get_widget (editor->priv->gui,
								  "pos_x_units_label");
	editor->priv->pos_y_units_label = glade_xml_get_widget (editor->priv->gui,
								  "pos_y_units_label");

	/* Get configuration information */
	units_string = gl_prefs_get_units_string ();
	editor->priv->units_per_point = gl_prefs_get_units_per_point ();
	climb_rate = gl_prefs_get_units_step_size ();
	digits = gl_prefs_get_units_precision ();

	/* Modify widgets based on configuration */
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON(editor->priv->pos_x_spin), digits);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON(editor->priv->pos_x_spin),
					climb_rate, 10.0*climb_rate);
	gtk_label_set_text (GTK_LABEL(editor->priv->pos_x_units_label), units_string);
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON(editor->priv->pos_y_spin), digits);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON(editor->priv->pos_y_spin),
					climb_rate, 10.0*climb_rate);
	gtk_label_set_text (GTK_LABEL(editor->priv->pos_y_units_label), units_string);

	/* Align label widths */
	label_size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	label = glade_xml_get_widget (editor->priv->gui, "pos_x_label");
	gtk_size_group_add_widget (label_size_group, label);
	label = glade_xml_get_widget (editor->priv->gui, "pos_y_label");
	gtk_size_group_add_widget (label_size_group, label);

	/* Un-hide */
	gtk_widget_show_all (editor->priv->pos_page_vbox);

	/* Connect signals */
	g_signal_connect_swapped (G_OBJECT (editor->priv->pos_x_spin),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->pos_y_spin),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Set position.                                                             */
/*****************************************************************************/
void
gl_object_editor_set_position (glObjectEditor      *editor,
				 gdouble                x,
				 gdouble                y)
{
	gl_debug (DEBUG_EDITOR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->pos_x_spin),
					 gl_object_editor_changed_cb,
					 editor);
	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->pos_y_spin),
					 gl_object_editor_changed_cb,
					 editor);

	/* convert internal units to displayed units */
	gl_debug (DEBUG_EDITOR, "internal x,y = %g, %g", x, y);
	x *= editor->priv->units_per_point;
	y *= editor->priv->units_per_point;
	gl_debug (DEBUG_EDITOR, "display x,y = %g, %g", x, y);

	/* Set widget values */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->pos_x_spin), x);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->pos_y_spin), y);

	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->pos_x_spin),
					   gl_object_editor_changed_cb,
					   editor);
	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->pos_y_spin),
					   gl_object_editor_changed_cb,
					   editor);

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Set maximum position.                                                     */
/*****************************************************************************/
void
gl_object_editor_set_max_position (glObjectEditor      *editor,
				     gdouble                x_max,
				     gdouble                y_max)
{
	gdouble tmp;

	gl_debug (DEBUG_EDITOR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->pos_x_spin),
					 gl_object_editor_changed_cb,
					 editor);
	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->pos_y_spin),
					 gl_object_editor_changed_cb,
					 editor);

	/* convert internal units to displayed units */
	gl_debug (DEBUG_EDITOR, "internal x_max,y_max = %g, %g", x_max, y_max);
	x_max *= editor->priv->units_per_point;
	y_max *= editor->priv->units_per_point;
	gl_debug (DEBUG_EDITOR, "display x_max,y_max = %g, %g", x_max, y_max);

	/* Set widget values */
	tmp = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->pos_x_spin));
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (editor->priv->pos_x_spin),
				   -x_max, 2.0*x_max);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->pos_x_spin), tmp);
	tmp = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->pos_y_spin));
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (editor->priv->pos_y_spin),
				   -y_max, 2.0*y_max);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->pos_y_spin), tmp);

	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->pos_x_spin),
					   gl_object_editor_changed_cb,
					   editor);
	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->pos_y_spin),
					   gl_object_editor_changed_cb,
					   editor);

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Query position.                                                           */
/*****************************************************************************/
void
gl_object_editor_get_position (glObjectEditor      *editor,
				 gdouble               *x,
				 gdouble               *y)
{
	gl_debug (DEBUG_EDITOR, "START");

	/* Get values from widgets */
	*x = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->pos_x_spin));
	*y = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->pos_y_spin));

	/* convert everything back to our internal units (points) */
	*x /= editor->priv->units_per_point;
	*y /= editor->priv->units_per_point;

	gl_debug (DEBUG_EDITOR, "END");
}

