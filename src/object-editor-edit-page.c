/*
 *  object-editor-edit-page.c
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
#include "color.h"
#include "field-button.h"
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

static void key_selected_cb (glObjectEditor *editor, gchar *key);


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare size page.                                             */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_prepare_edit_page (glObjectEditor       *editor)
{
	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
        gl_builder_util_get_widgets (editor->priv->builder,
                                     "edit_page_vbox",           &editor->priv->edit_page_vbox,
                                     "edit_text_view",           &editor->priv->edit_text_view,
                                     "edit_insert_field_vbox",   &editor->priv->edit_insert_field_vbox,
                                     NULL);

	editor->priv->edit_insert_field_button = gl_field_button_new (_("Insert merge field"));
        gtk_box_pack_start (GTK_BOX (editor->priv->edit_insert_field_vbox),
                            editor->priv->edit_insert_field_button, FALSE, FALSE, 0);

        gtk_widget_set_can_focus (editor->priv->edit_insert_field_button, FALSE);
        gtk_widget_set_can_default (editor->priv->edit_insert_field_button, FALSE);

	/* Un-hide */
	gtk_widget_show_all (editor->priv->edit_page_vbox);

	/* Connect signals */
	g_signal_connect_swapped (G_OBJECT (editor->priv->edit_insert_field_button),
				  "key_selected",
				  G_CALLBACK (key_selected_cb),
				  G_OBJECT (editor));

	gl_debug (DEBUG_EDITOR, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Menu item activated callback.                                  */
/*--------------------------------------------------------------------------*/
static void
key_selected_cb (glObjectEditor *editor, gchar *key)
{
        GtkTextBuffer *buffer;
        gchar *field_string;
 
        gl_debug (DEBUG_EDITOR, "START");
 
        field_string = g_strdup_printf ("${%s}", key);
        gl_debug (DEBUG_WDGT, "Inserting %s", field_string);
 
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->priv->edit_text_view));
        gtk_text_buffer_insert_at_cursor (buffer, field_string, -1);
 
        g_free (field_string);
 
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



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
