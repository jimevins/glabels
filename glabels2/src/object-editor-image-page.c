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

static void img_radio_toggled_cb                (glObjectEditor        *editor);



/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare image page.                                             */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_prepare_image_page (glObjectEditor *editor)
{
	GdkColor     *gdk_color;
	GtkSizeGroup *label_size_group;
	GtkWidget    *ge;

	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
	editor->priv->img_page_vbox    = glade_xml_get_widget (editor->priv->gui,
							       "img_page_vbox");
	editor->priv->img_file_radio   = glade_xml_get_widget (editor->priv->gui,
							       "img_file_radio");
	editor->priv->img_key_radio    = glade_xml_get_widget (editor->priv->gui,
							       "img_key_radio");
	editor->priv->img_pixmap_entry = glade_xml_get_widget (editor->priv->gui,
							      "img_pixmap_entry");
	editor->priv->img_key_combo    = glade_xml_get_widget (editor->priv->gui,
							       "img_key_combo");
	editor->priv->img_key_entry    = glade_xml_get_widget (editor->priv->gui,
							       "img_key_entry");

	/* Align label widths */
	label_size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget (label_size_group, editor->priv->img_file_radio);
	gtk_size_group_add_widget (label_size_group, editor->priv->img_key_radio);

	/* Un-hide */
	gtk_widget_show_all (editor->priv->img_page_vbox);

	/* Connect signals */
	ge = gnome_file_entry_gtk_entry(GNOME_FILE_ENTRY(editor->priv->img_pixmap_entry));
	g_signal_connect_swapped (G_OBJECT (ge),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->img_key_entry),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->img_file_radio),
				  "toggled",
				  G_CALLBACK (img_radio_toggled_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->img_key_radio),
				  "toggled",
				  G_CALLBACK (img_radio_toggled_cb),
				  G_OBJECT (editor));

	gl_debug (DEBUG_EDITOR, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  image radio callback.                                          */
/*--------------------------------------------------------------------------*/
static void
img_radio_toggled_cb (glObjectEditor *editor)
{
        gl_debug (DEBUG_WDGT, "START");
 
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->img_file_radio))) {
                gtk_widget_set_sensitive (editor->priv->img_pixmap_entry, TRUE);
                gtk_widget_set_sensitive (editor->priv->img_key_combo, FALSE);
        } else {
                gtk_widget_set_sensitive (editor->priv->img_pixmap_entry, FALSE);
                gtk_widget_set_sensitive (editor->priv->img_key_combo, TRUE);
        }
 
        /* Emit our "changed" signal */
        g_signal_emit (G_OBJECT (editor), gl_object_editor_signals[CHANGED], 0);
 
        gl_debug (DEBUG_WDGT, "END");
}

/*****************************************************************************/
/* Set image.                                                                */
/*****************************************************************************/
void
gl_object_editor_set_image (glObjectEditor      *editor,
			    gboolean             merge_flag,
			    glTextNode          *text_node)
{
        gint pos;
 
        gl_debug (DEBUG_EDITOR, "START");
 
	g_signal_handlers_block_by_func (G_OBJECT (editor->priv->img_pixmap_entry),
					 G_CALLBACK (gl_object_editor_changed_cb),
					 editor);
	g_signal_handlers_block_by_func (G_OBJECT (editor->priv->img_key_entry),
					 G_CALLBACK (gl_object_editor_changed_cb),
					 editor);

        gtk_widget_set_sensitive (editor->priv->img_key_radio, merge_flag);
 
        if (!text_node->field_flag) {
 
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                              (editor->priv->img_file_radio), TRUE); 
                gtk_widget_set_sensitive (editor->priv->img_pixmap_entry, TRUE);
                gtk_widget_set_sensitive (editor->priv->img_key_combo, FALSE);
 
                if (text_node->data != NULL ) {
                        gnome_file_entry_set_filename (GNOME_FILE_ENTRY(editor->priv->img_pixmap_entry),
                                                       text_node->data);
                } else {
                        gnome_file_entry_set_filename (GNOME_FILE_ENTRY(editor->priv->img_pixmap_entry),
                                                       "");
                }
        } else {
                                                                                
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                              (editor->priv->img_key_radio), TRUE);
                                                                                
                gtk_widget_set_sensitive (editor->priv->img_pixmap_entry, FALSE);
                gtk_widget_set_sensitive (editor->priv->img_key_combo, TRUE);
                                                                                
                gtk_editable_delete_text (GTK_EDITABLE (editor->priv->img_key_entry), 0, -1);
                pos = 0;
                gtk_editable_insert_text (GTK_EDITABLE (editor->priv->img_key_entry),
                                          text_node->data,
                                          strlen (text_node->data),
                                          &pos);
        }
                                                                                
	g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->img_pixmap_entry),
					   G_CALLBACK (gl_object_editor_changed_cb),
					   editor);
	g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->img_key_entry),
					   G_CALLBACK (gl_object_editor_changed_cb),
					   editor);
                                                                                
        gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Query image.                                                              */
/*****************************************************************************/
glTextNode *
gl_object_editor_get_image (glObjectEditor      *editor)
{
        glTextNode *text_node;
 
        gl_debug (DEBUG_EDITOR, "START");
 
        text_node = g_new0(glTextNode,1);
 
        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->img_file_radio))) {
                text_node->field_flag = FALSE;
                text_node->data =
                        gnome_pixmap_entry_get_filename (GNOME_PIXMAP_ENTRY(editor->priv->img_pixmap_entry));
        } else {
                text_node->field_flag = TRUE;
                text_node->data =
                    gtk_editable_get_chars (GTK_EDITABLE (editor->priv->img_key_entry),
                                            0, -1);
        }
 
	gl_debug (DEBUG_EDITOR, "text_node: field_flag=%d, data=%s",
		  text_node->field_flag, text_node->data);

        gl_debug (DEBUG_EDITOR, "END");
 
        return text_node;
}

