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

static void data_radio_toggled_cb                (glObjectEditor        *editor);



/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare data page.                                             */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_prepare_data_page (glObjectEditor *editor)
{
	GdkColor     *gdk_color;

	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
	editor->priv->data_page_vbox     = glade_xml_get_widget (editor->priv->gui,
								 "data_page_vbox");
	editor->priv->data_literal_radio = glade_xml_get_widget (editor->priv->gui,
								 "data_literal_radio");
	editor->priv->data_key_radio     = glade_xml_get_widget (editor->priv->gui,
								 "data_key_radio");
	editor->priv->data_text_entry    = glade_xml_get_widget (editor->priv->gui,
								 "data_text_entry");
	editor->priv->data_key_combo     = glade_xml_get_widget (editor->priv->gui,
								 "data_key_combo");
	editor->priv->data_key_entry     = glade_xml_get_widget (editor->priv->gui,
								 "data_key_entry");

	/* Un-hide */
	gtk_widget_show_all (editor->priv->data_page_vbox);

	/* Connect signals */
	g_signal_connect_swapped (G_OBJECT (editor->priv->data_text_entry),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->data_key_entry),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->data_literal_radio),
				  "toggled",
				  G_CALLBACK (data_radio_toggled_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->data_key_radio),
				  "toggled",
				  G_CALLBACK (data_radio_toggled_cb),
				  G_OBJECT (editor));

	gl_debug (DEBUG_EDITOR, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  data radio callback.                                           */
/*--------------------------------------------------------------------------*/
static void
data_radio_toggled_cb (glObjectEditor *editor)
{
        gl_debug (DEBUG_WDGT, "START");
 
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->data_literal_radio))) {
                gtk_widget_set_sensitive (editor->priv->data_text_entry, TRUE);
                gtk_widget_set_sensitive (editor->priv->data_key_combo, FALSE);
        } else {
                gtk_widget_set_sensitive (editor->priv->data_text_entry, FALSE);
                gtk_widget_set_sensitive (editor->priv->data_key_combo, TRUE);
        }
 
        /* Emit our "changed" signal */
        g_signal_emit (G_OBJECT (editor), gl_object_editor_signals[CHANGED], 0);
 
        gl_debug (DEBUG_WDGT, "END");
}

/*****************************************************************************/
/* Set data.                                                                 */
/*****************************************************************************/
void
gl_object_editor_set_data (glObjectEditor      *editor,
			    gboolean            merge_flag,
			    glTextNode         *text_node)
{
        gint pos;
 
        gl_debug (DEBUG_EDITOR, "START");
 
	g_signal_handlers_block_by_func (G_OBJECT (editor->priv->data_text_entry),
					 G_CALLBACK (gl_object_editor_changed_cb),
					 editor);
	g_signal_handlers_block_by_func (G_OBJECT (editor->priv->data_key_entry),
					 G_CALLBACK (gl_object_editor_changed_cb),
					 editor);

        gtk_widget_set_sensitive (editor->priv->data_key_radio, merge_flag);
 
        if (!text_node->field_flag) {
 
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                              (editor->priv->data_literal_radio), TRUE); 
                gtk_widget_set_sensitive (editor->priv->data_text_entry, TRUE);
                gtk_widget_set_sensitive (editor->priv->data_key_combo, FALSE);
 
                gtk_editable_delete_text (GTK_EDITABLE (editor->priv->data_text_entry), 0, -1);
                pos = 0;
                if (text_node->data != NULL ) {
			gtk_editable_insert_text (GTK_EDITABLE (editor->priv->data_text_entry),
						  text_node->data,
						  strlen (text_node->data),
						  &pos);
                }

        } else {
                                                                                
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                              (editor->priv->data_key_radio), TRUE);
                                                                                
                gtk_widget_set_sensitive (editor->priv->data_text_entry, FALSE);
                gtk_widget_set_sensitive (editor->priv->data_key_combo, TRUE);
                                                                                
                gtk_editable_delete_text (GTK_EDITABLE (editor->priv->data_key_entry), 0, -1);
                pos = 0;
                gtk_editable_insert_text (GTK_EDITABLE (editor->priv->data_key_entry),
                                          text_node->data,
                                          strlen (text_node->data),
                                          &pos);
        }
                                                                                

	g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->data_text_entry),
					   G_CALLBACK (gl_object_editor_changed_cb),
					   editor);
	g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->data_key_entry),
					   G_CALLBACK (gl_object_editor_changed_cb),
					   editor);

        gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Query data.                                                              */
/*****************************************************************************/
glTextNode *
gl_object_editor_get_data (glObjectEditor      *editor)
{
        glTextNode *text_node;
 
        gl_debug (DEBUG_EDITOR, "START");
 
        text_node = g_new0(glTextNode,1);
 
        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->data_literal_radio))) {
                text_node->field_flag = FALSE;
                text_node->data =
                    gtk_editable_get_chars (GTK_EDITABLE (editor->priv->data_text_entry),
                                            0, -1);
        } else {
                text_node->field_flag = TRUE;
                text_node->data =
                    gtk_editable_get_chars (GTK_EDITABLE (editor->priv->data_key_entry),
                                            0, -1);
        }
 
	gl_debug (DEBUG_EDITOR, "text_node: field_flag=%d, data=%s",
		  text_node->field_flag, text_node->data);

        gl_debug (DEBUG_EDITOR, "END");
 
        return text_node;
}

