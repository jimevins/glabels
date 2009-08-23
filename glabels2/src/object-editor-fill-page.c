/*
 *  object-editor-fill-page.c
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
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkcombobox.h>
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

static void fill_radio_toggled_cb                (glObjectEditor        *editor);


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare fill page.                                             */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_prepare_fill_page (glObjectEditor *editor)
{
	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
        gl_util_get_builder_widgets (editor->priv->builder,
                                     "fill_page_vbox",   &editor->priv->fill_page_vbox,
                                     "fill_color_hbox",  &editor->priv->fill_color_hbox,
                                     "fill_key_combo",   &editor->priv->fill_key_combo,
                                     "fill_key_radio",   &editor->priv->fill_key_radio,
                                     "fill_color_radio", &editor->priv->fill_color_radio,
                                     NULL);
	
	editor->priv->fill_color_combo = gl_color_combo_new (NULL,
                                                             _("No Fill"),
                                                             GL_COLOR_NO_FILL,
                                                             gl_prefs->default_fill_color);
        gtk_container_add (GTK_CONTAINER (editor->priv->fill_color_hbox),
                           editor->priv->fill_color_combo);

	gl_util_combo_box_add_text_model ( GTK_COMBO_BOX(editor->priv->fill_key_combo));

	/* Modify widgets based on configuration */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->fill_color_radio), TRUE);
	gtk_widget_set_sensitive (editor->priv->fill_color_combo, TRUE);
	gtk_widget_set_sensitive (editor->priv->fill_key_combo, FALSE);
	gl_color_combo_set_color (GL_COLOR_COMBO(editor->priv->fill_color_combo),
                                  gl_prefs->default_fill_color);

	/* Un-hide */
	gtk_widget_show_all (editor->priv->fill_page_vbox);

	/* Connect signals */
	g_signal_connect_swapped (G_OBJECT (editor->priv->fill_color_combo),
				  "color_changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->fill_key_combo),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->fill_color_radio),
				  "toggled",
				  G_CALLBACK (fill_radio_toggled_cb),
				  G_OBJECT (editor));				  
	g_signal_connect_swapped (G_OBJECT (editor->priv->fill_key_radio),
				  "toggled",
				  G_CALLBACK (fill_radio_toggled_cb),
				  G_OBJECT (editor));				  
	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Set fill color.                                                           */
/*****************************************************************************/
void
gl_object_editor_set_fill_color (glObjectEditor      *editor,
				 gboolean             merge_flag,
				 glColorNode         *color_node)
{
	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

	gtk_widget_set_sensitive (editor->priv->fill_key_radio, merge_flag);

	if ( color_node->color == GL_COLOR_NONE ) {

		gl_color_combo_set_to_default (GL_COLOR_COMBO(editor->priv->fill_color_combo));

	} else {

		gl_color_combo_set_color (GL_COLOR_COMBO(editor->priv->fill_color_combo),
					   color_node->color);

	}
	
	if (!color_node->field_flag) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
						  (editor->priv->fill_color_radio), TRUE); 
		gtk_widget_set_sensitive (editor->priv->fill_color_combo, TRUE);
		gtk_widget_set_sensitive (editor->priv->fill_key_combo, FALSE);
		
	} else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
						  (editor->priv->fill_key_radio), TRUE); 
		gtk_widget_set_sensitive (editor->priv->fill_color_combo, FALSE);
		gtk_widget_set_sensitive (editor->priv->fill_key_combo, TRUE);
		
		gl_util_combo_box_set_active_text (GTK_COMBO_BOX (editor->priv->fill_key_combo),
						   color_node->key);
	}
	
        editor->priv->stop_signals = FALSE;

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Query fill color.                                                         */
/*****************************************************************************/
glColorNode*
gl_object_editor_get_fill_color (glObjectEditor      *editor)
{
        guint        color;
        gboolean     is_default;
	glColorNode *color_node;
 
	gl_debug (DEBUG_EDITOR, "START");

	color_node = gl_color_node_new_default ();
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->fill_key_radio))) {
		color_node->field_flag = TRUE;
		color_node->key = 
			gtk_combo_box_get_active_text (GTK_COMBO_BOX (editor->priv->fill_key_combo));
        } else {
		color_node->field_flag = FALSE;
		color_node->key = NULL;
		color = gl_color_combo_get_color (GL_COLOR_COMBO(editor->priv->fill_color_combo),
                                                  &is_default);

		if (!is_default) {
                        color_node->color = color;
                }
	}

	gl_debug (DEBUG_EDITOR, "END");

	return color_node;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  fill radio callback.                                           */
/*--------------------------------------------------------------------------*/
static void
fill_radio_toggled_cb (glObjectEditor *editor)
{
        if (editor->priv->stop_signals) return;

        gl_debug (DEBUG_EDITOR, "START");
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->fill_color_radio))) {
                gtk_widget_set_sensitive (editor->priv->fill_color_combo, TRUE);
                gtk_widget_set_sensitive (editor->priv->fill_key_combo, FALSE);
    } else {
                gtk_widget_set_sensitive (editor->priv->fill_color_combo, FALSE);
                gtk_widget_set_sensitive (editor->priv->fill_key_combo, TRUE);
		
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
