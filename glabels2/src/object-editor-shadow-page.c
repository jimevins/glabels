/*
 *  object-editor-shadow-page.c
 *  Copyright (C) 2006-2009  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of gLabels.
 *
 *  gLabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gLabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gLabels.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "object-editor.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <math.h>

#include "prefs.h"
#include "color-combo.h"
#include "color.h"
#include "util.h"

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

static void shadow_enable_check_toggled_cb (glObjectEditor        *editor);
static void shadow_color_radio_toggled_cb  (glObjectEditor        *editor);


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare shadow page.                                         */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_prepare_shadow_page (glObjectEditor *editor)
{
	const gchar  *units_string;
	gdouble       climb_rate;
	gint          digits;

	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
        gl_util_get_builder_widgets (editor->priv->builder,
                                     "shadow_page_vbox",      &editor->priv->shadow_page_vbox,
                                     "shadow_enable_check",   &editor->priv->shadow_enable_check,
                                     "shadow_controls_table", &editor->priv->shadow_controls_table,
                                     "shadow_x_spin",         &editor->priv->shadow_x_spin,
                                     "shadow_y_spin",         &editor->priv->shadow_y_spin,
                                     "shadow_x_units_label",  &editor->priv->shadow_x_units_label,
                                     "shadow_y_units_label",  &editor->priv->shadow_y_units_label,
                                     "shadow_color_radio",    &editor->priv->shadow_color_radio,
                                     "shadow_key_radio",      &editor->priv->shadow_key_radio,
                                     "shadow_color_hbox",     &editor->priv->shadow_color_hbox,
                                     "shadow_key_combo",      &editor->priv->shadow_key_combo,
                                     "shadow_opacity_spin",   &editor->priv->shadow_opacity_spin,
                                     NULL);

	editor->priv->shadow_color_combo = gl_color_combo_new (_("Default"),
                                                               GL_COLOR_SHADOW_DEFAULT,
                                                               GL_COLOR_SHADOW_DEFAULT);
        gtk_container_add (GTK_CONTAINER (editor->priv->shadow_color_hbox),
                           editor->priv->shadow_color_combo);

	gl_util_combo_box_add_text_model ( GTK_COMBO_BOX(editor->priv->shadow_key_combo));

	/* Get configuration information */
	units_string = gl_prefs_get_units_string ();
	editor->priv->units_per_point = gl_prefs_get_units_per_point ();
	climb_rate = gl_prefs_get_units_step_size ();
	digits = gl_prefs_get_units_precision ();

	/* Modify widgets based on configuration */
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON(editor->priv->shadow_x_spin), digits);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON(editor->priv->shadow_x_spin),
					climb_rate, 10.0*climb_rate);
	gtk_label_set_text (GTK_LABEL(editor->priv->shadow_x_units_label), units_string);
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON(editor->priv->shadow_y_spin), digits);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON(editor->priv->shadow_y_spin),
					climb_rate, 10.0*climb_rate);
	gtk_label_set_text (GTK_LABEL(editor->priv->shadow_y_units_label), units_string);

	/* Un-hide */
	gtk_widget_show_all (editor->priv->shadow_page_vbox);

	/* Connect signals */
	g_signal_connect_swapped (G_OBJECT (editor->priv->shadow_enable_check),
				  "toggled",
				  G_CALLBACK (shadow_enable_check_toggled_cb),
				  G_OBJECT (editor));				  
	g_signal_connect_swapped (G_OBJECT (editor->priv->shadow_x_spin),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->shadow_y_spin),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->shadow_color_combo),
				  "color_changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->shadow_key_combo),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->shadow_color_radio),
				  "toggled",
				  G_CALLBACK (shadow_color_radio_toggled_cb),
				  G_OBJECT (editor));				  
	g_signal_connect_swapped (G_OBJECT (editor->priv->shadow_key_radio),
				  "toggled",
				  G_CALLBACK (shadow_color_radio_toggled_cb),
				  G_OBJECT (editor));				  
	g_signal_connect_swapped (G_OBJECT (editor->priv->shadow_opacity_spin),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Set shadow parameters.                                                    */
/*****************************************************************************/
void
gl_object_editor_set_shadow_state (glObjectEditor      *editor,
				   gboolean             state)
{
	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->shadow_enable_check),
                                      state);
        gtk_widget_set_sensitive (editor->priv->shadow_controls_table, state);

        editor->priv->stop_signals = FALSE;

	gl_debug (DEBUG_EDITOR, "END");
}


void
gl_object_editor_set_shadow_offset (glObjectEditor      *editor,
				    gdouble              x,
				    gdouble              y)
{
	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

	/* save a copy in internal units */
	editor->priv->shadow_x = x;
	editor->priv->shadow_y = y;

	/* convert internal units to displayed units */
	gl_debug (DEBUG_EDITOR, "internal x,y = %g, %g", x, y);
	x *= editor->priv->units_per_point;
	y *= editor->priv->units_per_point;
	gl_debug (DEBUG_EDITOR, "display x,y = %g, %g", x, y);

	/* Set widget values */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->shadow_x_spin), x);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->shadow_y_spin), y);

        editor->priv->stop_signals = FALSE;

	gl_debug (DEBUG_EDITOR, "END");
}


void
gl_object_editor_set_shadow_color (glObjectEditor      *editor,
				   gboolean             merge_flag,
				   glColorNode         *color_node)
{
	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

	gtk_widget_set_sensitive (editor->priv->shadow_key_radio, merge_flag);

	if ( color_node->color == GL_COLOR_NONE ) {

		gl_color_combo_set_to_default (GL_COLOR_COMBO(editor->priv->shadow_color_combo));

	} else {

		gl_color_combo_set_color (GL_COLOR_COMBO(editor->priv->shadow_color_combo),
                                          color_node->color);

	}
	
	if (!color_node->field_flag) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
						  (editor->priv->shadow_color_radio), TRUE); 
		gtk_widget_set_sensitive (editor->priv->shadow_color_combo, TRUE);
		gtk_widget_set_sensitive (editor->priv->shadow_key_combo, FALSE);
		
	} else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
						  (editor->priv->shadow_key_radio), TRUE); 
		gtk_widget_set_sensitive (editor->priv->shadow_color_combo, FALSE);
		gtk_widget_set_sensitive (editor->priv->shadow_key_combo, TRUE);
		
		gl_util_combo_box_set_active_text (GTK_COMBO_BOX (editor->priv->shadow_key_combo),
						   color_node->key);
	}
	
        editor->priv->stop_signals = FALSE;

	gl_debug (DEBUG_EDITOR, "END");
}


void
gl_object_editor_set_shadow_opacity (glObjectEditor      *editor,
				     gdouble              alpha)
{
	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->shadow_opacity_spin),
				   alpha * 100.0);

        editor->priv->stop_signals = FALSE;

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Set maximum shadow offset.                                                */
/*****************************************************************************/
void
gl_object_editor_set_max_shadow_offset (glObjectEditor      *editor,
					gdouble              x_max,
					gdouble              y_max)
{
	gdouble tmp;

	gl_debug (DEBUG_EDITOR, "START");

        if (editor->priv->shadow_page_vbox)
        {

                editor->priv->stop_signals = TRUE;

                /* save a copy in internal units */
                editor->priv->shadow_x_max = x_max;
                editor->priv->shadow_y_max = y_max;

                /* convert internal units to displayed units */
                gl_debug (DEBUG_EDITOR, "internal x_max,y_max = %g, %g", x_max, y_max);
                x_max *= editor->priv->units_per_point;
                y_max *= editor->priv->units_per_point;
                gl_debug (DEBUG_EDITOR, "display x_max,y_max = %g, %g", x_max, y_max);

                /* Set widget values */
                tmp = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->shadow_x_spin));
                gtk_spin_button_set_range (GTK_SPIN_BUTTON (editor->priv->shadow_x_spin),
                                           -x_max, x_max);
                gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->shadow_x_spin), tmp);
                tmp = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->shadow_y_spin));
                gtk_spin_button_set_range (GTK_SPIN_BUTTON (editor->priv->shadow_y_spin),
                                           -y_max, y_max);
                gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->shadow_y_spin), tmp);

                editor->priv->stop_signals = FALSE;

        }

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Query shadow parameters.                                                  */
/*****************************************************************************/
gboolean
gl_object_editor_get_shadow_state (glObjectEditor      *editor)
{
	gboolean state;

	gl_debug (DEBUG_EDITOR, "START");

	state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->shadow_enable_check));

	gl_debug (DEBUG_EDITOR, "END");

	return state;
}


void
gl_object_editor_get_shadow_offset (glObjectEditor      *editor,
				    gdouble             *x,
				    gdouble             *y)
{
	gl_debug (DEBUG_EDITOR, "START");

	/* Get values from widgets */
	*x = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->shadow_x_spin));
	*y = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->shadow_y_spin));

	/* convert everything back to our internal units (points) */
	*x /= editor->priv->units_per_point;
	*y /= editor->priv->units_per_point;

	/* save a copy in internal units */
	editor->priv->shadow_x = *x;
	editor->priv->shadow_y = *y;

	gl_debug (DEBUG_EDITOR, "END");
}


glColorNode*
gl_object_editor_get_shadow_color (glObjectEditor      *editor)
{
        guint        color;
        gboolean     is_default;
	glColorNode *color_node;
 
	gl_debug (DEBUG_EDITOR, "START");

	color_node = gl_color_node_new_default ();
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->shadow_key_radio))) {
		color_node->field_flag = TRUE;
		color_node->key = 
			gtk_combo_box_get_active_text (GTK_COMBO_BOX (editor->priv->shadow_key_combo));
        } else {
		color_node->field_flag = FALSE;
		color_node->key = NULL;
		color = gl_color_combo_get_color (GL_COLOR_COMBO(editor->priv->shadow_color_combo),
                                                  &is_default);

		if (!is_default) {
                        color_node->color = color;
                }
	}

	gl_debug (DEBUG_EDITOR, "END");

	return color_node;
}


gdouble
gl_object_editor_get_shadow_opacity (glObjectEditor      *editor)
{
	gdouble alpha;

	gl_debug (DEBUG_EDITOR, "START");

	alpha = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->shadow_opacity_spin));

	gl_debug (DEBUG_EDITOR, "END");

	return alpha / 100.0;
}


/*****************************************************************************/
/* PRIVATE. Prefs changed callback.  Update units related items.            */
/*****************************************************************************/
void
shadow_prefs_changed_cb (glObjectEditor *editor)
{
	const gchar  *units_string;
	gdouble       climb_rate;
	gint          digits;

	gl_debug (DEBUG_EDITOR, "START");

        /* Get new configuration information */
        units_string = gl_prefs_get_units_string ();
        editor->priv->units_per_point = gl_prefs_get_units_per_point ();
        climb_rate = gl_prefs_get_units_step_size ();
        digits = gl_prefs_get_units_precision ();

	/* Update characteristics of x_spin/y_spin */
        editor->priv->stop_signals = TRUE;
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON(editor->priv->shadow_x_spin),
				    digits);
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON(editor->priv->shadow_y_spin),
				    digits);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON(editor->priv->shadow_x_spin),
					climb_rate, 10.0*climb_rate);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON(editor->priv->shadow_y_spin),
					climb_rate, 10.0*climb_rate);
        editor->priv->stop_signals = FALSE;

	/* Update units_labels */
	gtk_label_set_text (GTK_LABEL(editor->priv->shadow_x_units_label),
			    units_string);
	gtk_label_set_text (GTK_LABEL(editor->priv->shadow_y_units_label),
			    units_string);

	/* Update values of x_spin/y_spin */
	gl_object_editor_set_shadow_offset (editor,
					    editor->priv->shadow_x,
					    editor->priv->shadow_y);
	gl_object_editor_set_max_shadow_offset (editor,
						editor->priv->shadow_x_max,
						editor->priv->shadow_y_max);

	gl_debug (DEBUG_EDITOR, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  shadow enable check callback.                                  */
/*--------------------------------------------------------------------------*/
static void
shadow_enable_check_toggled_cb (glObjectEditor *editor)
{
	gboolean state;

        if (editor->priv->stop_signals) return;

        gl_debug (DEBUG_EDITOR, "START");

	state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->shadow_enable_check));

        gtk_widget_set_sensitive (editor->priv->shadow_controls_table, state);

        /* Emit our "changed" signal */
        g_signal_emit (G_OBJECT (editor), gl_object_editor_signals[CHANGED], 0);

        gl_debug (DEBUG_EDITOR, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  shadow color radio callback.                                   */
/*--------------------------------------------------------------------------*/
static void
shadow_color_radio_toggled_cb (glObjectEditor *editor)
{
        if (editor->priv->stop_signals) return;

        gl_debug (DEBUG_EDITOR, "START");
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->shadow_color_radio))) {
                gtk_widget_set_sensitive (editor->priv->shadow_color_combo, TRUE);
                gtk_widget_set_sensitive (editor->priv->shadow_key_combo, FALSE);
        } else {
                gtk_widget_set_sensitive (editor->priv->shadow_color_combo, FALSE);
                gtk_widget_set_sensitive (editor->priv->shadow_key_combo, TRUE);
		
	}
 
        /* Emit our "changed" signal */
        g_signal_emit (G_OBJECT (editor), gl_object_editor_signals[CHANGED], 0);
 
        gl_debug (DEBUG_EDITOR, "END");
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
