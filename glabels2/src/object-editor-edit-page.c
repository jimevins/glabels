/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

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
#include "color.h"
#include "wdgt-merge-menu.h"
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

static void insert_button_cb (glObjectEditor  *editor);

static void field_selected_cb (glObjectEditor *editor, gchar *field);


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare size page.                                             */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_prepare_edit_page (glObjectEditor       *editor)
{
	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
        gl_util_get_builder_widgets (editor->priv->gui,
                                     "edit_page_vbox",           &editor->priv->edit_page_vbox,
                                     "edit_text_view",           &editor->priv->edit_text_view,
                                     "edit_insert_field_button", &editor->priv->edit_insert_field_button,
                                     NULL);

	editor->priv->edit_insert_field_menu = gl_wdgt_merge_menu_new ();

	/* Un-hide */
	gtk_widget_show_all (editor->priv->edit_page_vbox);

	/* Connect signals */
	g_signal_connect_swapped (G_OBJECT (editor->priv->edit_insert_field_button),
				  "clicked",
				  G_CALLBACK (insert_button_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->edit_insert_field_menu),
				  "field_selected",
				  G_CALLBACK (field_selected_cb),
				  G_OBJECT (editor));

	gl_debug (DEBUG_EDITOR, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Menu item activated callback.                                  */
/*--------------------------------------------------------------------------*/
static void
field_selected_cb (glObjectEditor *editor, gchar *field)
{
        GtkTextBuffer *buffer;
        gchar *field_string;
 
        gl_debug (DEBUG_EDITOR, "START");
 
        field_string = g_strdup_printf ("${%s}", field);
        gl_debug (DEBUG_WDGT, "Inserting %s", field_string);
 
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->priv->edit_text_view));
        gtk_text_buffer_insert_at_cursor (buffer, field_string, -1);
 
        g_free (field_string);
 
        gl_debug (DEBUG_EDITOR, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Insert button callback.                                        */
/*--------------------------------------------------------------------------*/
static void
insert_button_cb (glObjectEditor  *editor)
{
        gl_debug (DEBUG_EDITOR, "START");
 
        gtk_widget_show_all (editor->priv->edit_insert_field_menu);
        gtk_menu_popup (GTK_MENU (editor->priv->edit_insert_field_menu),
                        NULL, NULL, NULL, NULL, 1, gtk_get_current_event_time ());

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

