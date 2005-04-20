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

#include "object-editor.h"

#include <glib/gi18n.h>
#include <gtk/gtktextview.h>
#include <gtk/gtkcombobox.h>
#include <math.h>

#include "prefs.h"
#include "mygal/widget-color-combo.h"
#include "color.h"

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

static void insert_button_cb (glObjectEditor  *editor);


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare size page.                                             */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_prepare_edit_page (glObjectEditor       *editor)
{
	GList        *family_names = NULL;
	GtkWidget    *label;

	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
	editor->priv->edit_page_vbox =
		glade_xml_get_widget (editor->priv->gui, "edit_page_vbox");
	editor->priv->edit_text_view =
		glade_xml_get_widget (editor->priv->gui, "edit_text_view");
	editor->priv->edit_key_label =
		glade_xml_get_widget (editor->priv->gui, "edit_key_label");
	editor->priv->edit_key_combo =
		glade_xml_get_widget (editor->priv->gui, "edit_key_combo");
	editor->priv->edit_insert_field_button =
		glade_xml_get_widget (editor->priv->gui, "edit_insert_field_button");

	gl_util_combo_box_add_text_model ( GTK_COMBO_BOX(editor->priv->edit_key_combo));

	/* Un-hide */
	gtk_widget_show_all (editor->priv->edit_page_vbox);

	/* Connect signals */
	g_signal_connect_swapped (G_OBJECT (editor->priv->edit_insert_field_button),
				  "clicked",
				  G_CALLBACK (insert_button_cb),
				  G_OBJECT (editor));

	gl_debug (DEBUG_EDITOR, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Alignment togglebutton callback.                               */
/*--------------------------------------------------------------------------*/
static void
insert_button_cb (glObjectEditor  *editor)
{
        GtkTextBuffer *buffer;
        gchar *key, *field;
 
        gl_debug (DEBUG_EDITOR, "START");
 
	key = gtk_combo_box_get_active_text (GTK_COMBO_BOX (editor->priv->edit_key_combo));
        field = g_strdup_printf ("${%s}", key);
        gl_debug (DEBUG_WDGT, "Inserting %s", field);
 
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->priv->edit_text_view));
        gtk_text_buffer_insert_at_cursor (buffer, field, -1);
 
        g_free (field);
        g_free (key);
 
 
        gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Set text buffer as model for text view/editor.                            */
/*****************************************************************************/
void
gl_object_editor_set_text_buffer (glObjectEditor      *editor,
				  GtkTextBuffer       *buffer)
{
        gl_debug (DEBUG_EDITOR, "START");

        gtk_text_view_set_buffer (GTK_TEXT_VIEW(editor->priv->edit_text_view), buffer);

        gl_debug (DEBUG_EDITOR, "END");
}

