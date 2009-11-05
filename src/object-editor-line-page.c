/*
 *  object-editor-line-page.c
 *  Copyright (C) 2003-2009  Jim Evins <evins@snaught.com>.
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
#include "combo-util.h"
#include "builder-util.h"

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
static void line_radio_toggled_cb               (glObjectEditor        *editor);


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare line page.                                             */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_prepare_line_page (glObjectEditor *editor)
{
	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
        gl_builder_util_get_widgets (editor->priv->builder,
                                     "line_page_vbox",   &editor->priv->line_page_vbox,
                                     "line_width_spin",  &editor->priv->line_width_spin,
                                     "line_color_hbox",  &editor->priv->line_color_hbox,
                                     "line_key_combo",   &editor->priv->line_key_combo,
                                     "line_key_radio",   &editor->priv->line_key_radio,
                                     "line_color_radio", &editor->priv->line_color_radio,
                                     NULL);

	editor->priv->line_color_combo = gl_color_combo_new (_("No Line"),
                                                             GL_COLOR_NO_LINE,
                                                             gl_prefs_model_get_default_line_color (gl_prefs));
        gtk_box_pack_start (GTK_BOX (editor->priv->line_color_hbox),
                            editor->priv->line_color_combo,
                            FALSE, FALSE, 0);

	gl_combo_util_add_text_model ( GTK_COMBO_BOX(editor->priv->line_key_combo));

	/* Modify widgets based on configuration */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->line_color_radio), TRUE);
	gtk_widget_set_sensitive (editor->priv->line_color_combo, TRUE);
    gtk_widget_set_sensitive (editor->priv->line_key_combo, FALSE);	
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->line_width_spin),
				   gl_prefs_model_get_default_line_width (gl_prefs));
	gl_color_combo_set_color (GL_COLOR_COMBO(editor->priv->line_color_combo),
                                  gl_prefs_model_get_default_line_color (gl_prefs));

	/* Un-hide */
	gtk_widget_show_all (editor->priv->line_page_vbox);

	/* Connect signals */
	g_signal_connect_swapped (G_OBJECT (editor->priv->line_width_spin),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->line_color_combo),
				  "color_changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->line_key_combo),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->line_color_radio),
				  "toggled",
				  G_CALLBACK (line_radio_toggled_cb),
				  G_OBJECT (editor));				  
	g_signal_connect_swapped (G_OBJECT (editor->priv->line_key_radio),
				  "toggled",
				  G_CALLBACK (line_radio_toggled_cb),
				  G_OBJECT (editor));

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Set line width.                                                           */
/*****************************************************************************/
void
gl_object_editor_set_line_width (glObjectEditor      *editor,
				 gdouble              width)
{
	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

	/* Set widget values */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->line_width_spin),
				   width);

        editor->priv->stop_signals = FALSE;

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Query line width.                                                         */
/*****************************************************************************/
gdouble
gl_object_editor_get_line_width (glObjectEditor      *editor)
{
	gdouble w;
 
	gl_debug (DEBUG_EDITOR, "START");

	w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(editor->priv->line_width_spin));

	gl_debug (DEBUG_EDITOR, "END");

	return w;
}


/*****************************************************************************/
/* Set line color.                                                           */
/*****************************************************************************/
void
gl_object_editor_set_line_color (glObjectEditor      *editor,
				 gboolean             merge_flag,
				 glColorNode         *color_node)
{
	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

	gl_debug (DEBUG_EDITOR, "color field %s(%d) / %X", color_node->key, color_node->field_flag, color_node->color);
	gtk_widget_set_sensitive (editor->priv->line_key_radio, merge_flag);

	if ( color_node->color == GL_COLOR_NONE ) {

		gl_color_combo_set_to_default (GL_COLOR_COMBO(editor->priv->line_color_combo));

	} else {

		gl_color_combo_set_color (GL_COLOR_COMBO(editor->priv->line_color_combo),
					   color_node->color);

	}	
	
	if (!color_node->field_flag || !merge_flag) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
						  (editor->priv->line_color_radio), TRUE); 
		gtk_widget_set_sensitive (editor->priv->line_color_combo, TRUE);
		gtk_widget_set_sensitive (editor->priv->line_key_combo, FALSE);
		
	} else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
						  (editor->priv->line_key_radio), TRUE); 
		gtk_widget_set_sensitive (editor->priv->line_color_combo, FALSE);
		gtk_widget_set_sensitive (editor->priv->line_key_combo, TRUE);
		
		gl_combo_util_set_active_text (GTK_COMBO_BOX (editor->priv->line_key_combo),
                                               color_node->key);
	}

        editor->priv->stop_signals = FALSE;

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Query line color.                                                         */
/*****************************************************************************/
glColorNode*
gl_object_editor_get_line_color (glObjectEditor      *editor)
{
        guint        color;
        gboolean     is_default;
	glColorNode *color_node;
 
	gl_debug (DEBUG_EDITOR, "START");

	color_node = gl_color_node_new_default ();
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->line_key_radio))) {
		color_node->field_flag = TRUE;
		color_node->key = 
			gtk_combo_box_get_active_text (GTK_COMBO_BOX (editor->priv->line_key_combo));
        } else {
		color_node->field_flag = FALSE;
		color_node->key = NULL;
		color = gl_color_combo_get_color (GL_COLOR_COMBO(editor->priv->line_color_combo),
                                                  &is_default);

		if (!is_default) {
                        color_node->color = color;
                }
	}
	
	gl_debug (DEBUG_EDITOR, "END");

	return color_node;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  line color radio callback.                                     */
/*--------------------------------------------------------------------------*/
static void
line_radio_toggled_cb (glObjectEditor *editor)
{
        if (editor->priv->stop_signals) return;

        gl_debug (DEBUG_EDITOR, "START");
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->line_color_radio))) {
                gtk_widget_set_sensitive (editor->priv->line_color_combo, TRUE);
                gtk_widget_set_sensitive (editor->priv->line_key_combo, FALSE);
        } else {
                gtk_widget_set_sensitive (editor->priv->line_color_combo, FALSE);
                gtk_widget_set_sensitive (editor->priv->line_key_combo, TRUE);
		
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
