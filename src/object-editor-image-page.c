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
#include <gtk/gtkimage.h>
#include <gtk/gtkfilechooserbutton.h>
#include <gtk/gtkcombobox.h>
#include <gtk/gtktogglebutton.h>
#include <math.h>

#include "prefs.h"
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

static void img_radio_toggled_cb                (glObjectEditor        *editor);

static void update_preview_cb                   (GtkFileChooser *file_chooser,
						 gpointer        data);
static void add_image_filters_to_chooser        (GtkFileChooser *file_chooser);

static void img_selection_changed_cb (glObjectEditor *editor);


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare image page.                                             */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_prepare_image_page (glObjectEditor *editor)
{
	GtkWidget     *preview;


	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
	editor->priv->img_page_vbox    = glade_xml_get_widget (editor->priv->gui,
							       "img_page_vbox");
	editor->priv->img_file_radio   = glade_xml_get_widget (editor->priv->gui,
							       "img_file_radio");
	editor->priv->img_key_radio    = glade_xml_get_widget (editor->priv->gui,
							       "img_key_radio");
	editor->priv->img_file_button  = glade_xml_get_widget (editor->priv->gui,
							       "img_file_button");
	editor->priv->img_key_combo    = glade_xml_get_widget (editor->priv->gui,
							       "img_key_combo");

	gl_util_combo_box_add_text_model ( GTK_COMBO_BOX(editor->priv->img_key_combo));

	/* Modify file button properties. */
	add_image_filters_to_chooser (GTK_FILE_CHOOSER (editor->priv->img_file_button));
	preview = gtk_image_new ();
	gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER (editor->priv->img_file_button), preview);
	g_signal_connect (G_OBJECT (editor->priv->img_file_button),
			  "update-preview",
			  G_CALLBACK (update_preview_cb), preview);

	/* Un-hide */
	gtk_widget_show_all (editor->priv->img_page_vbox);

	/* Connect signals */
	g_signal_connect_swapped (G_OBJECT (editor->priv->img_file_button),
				  "selection-changed",
				  G_CALLBACK (img_selection_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->img_key_combo),
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
        if (editor->priv->stop_signals) return;

        gl_debug (DEBUG_WDGT, "START");
 
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->img_file_radio))) {
                gtk_widget_set_sensitive (editor->priv->img_file_button, TRUE);
                gtk_widget_set_sensitive (editor->priv->img_key_combo, FALSE);
        } else {
                gtk_widget_set_sensitive (editor->priv->img_file_button, FALSE);
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
        gl_debug (DEBUG_EDITOR, "START");
 
        editor->priv->stop_signals = TRUE;

        gtk_widget_set_sensitive (editor->priv->img_key_radio, merge_flag);
 
        if (!text_node->field_flag || !merge_flag) {
 
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                              (editor->priv->img_file_radio), TRUE); 
                gtk_widget_set_sensitive (editor->priv->img_file_button, TRUE);
                gtk_widget_set_sensitive (editor->priv->img_key_combo, FALSE);
 
                if (text_node->data != NULL ) {
			gtk_file_chooser_set_filename (GTK_FILE_CHOOSER(editor->priv->img_file_button),
                                                       text_node->data);
                } else {
			gtk_file_chooser_unselect_all (GTK_FILE_CHOOSER(editor->priv->img_file_button));
                }
        } else {
                                                                                
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                              (editor->priv->img_key_radio), TRUE);
                                                                                
                gtk_widget_set_sensitive (editor->priv->img_file_button, FALSE);
                gtk_widget_set_sensitive (editor->priv->img_key_combo, TRUE);
                                                                                
		gl_util_combo_box_set_active_text (GTK_COMBO_BOX (editor->priv->img_key_combo),
						   text_node->data);
        }
                                                                                
        editor->priv->stop_signals = FALSE;
                                                                                
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
			gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(editor->priv->img_file_button));
        } else {
                text_node->field_flag = TRUE;
                text_node->data =
			gtk_combo_box_get_active_text (GTK_COMBO_BOX (editor->priv->img_key_combo));
        }
 
	gl_debug (DEBUG_EDITOR, "text_node: field_flag=%d, data=%s",
		  text_node->field_flag, text_node->data);

        gl_debug (DEBUG_EDITOR, "END");
 
        return text_node;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Update preview image.                                          */
/*--------------------------------------------------------------------------*/
static void
update_preview_cb (GtkFileChooser *file_chooser, gpointer data)
{
        GtkWidget *preview;
        char *filename;
        GdkPixbuf *pixbuf;
        gboolean have_preview;

        preview = GTK_WIDGET (data);
        filename = gtk_file_chooser_get_preview_filename (file_chooser);

        if (filename) {
                pixbuf = gdk_pixbuf_new_from_file_at_size (filename, 128, 128, NULL);
                have_preview = (pixbuf != NULL);
                g_free (filename);

                gtk_image_set_from_pixbuf (GTK_IMAGE (preview), pixbuf);
                if (pixbuf)
                        gdk_pixbuf_unref (pixbuf);

                gtk_file_chooser_set_preview_widget_active (file_chooser,
                                                            have_preview);
        } else {
                gtk_file_chooser_set_preview_widget_active (file_chooser, FALSE);
        }
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add filters to file chooser.                                   */
/*           Based on eog's eog_file_selection_add_filter().                */
/*--------------------------------------------------------------------------*/
static void
add_image_filters_to_chooser (GtkFileChooser *chooser)
{
 	GtkFileFilter *all_file_filter;
	GtkFileFilter *all_img_filter;
	GSList        *formats;
	GSList        *it;
	GtkFileFilter *filter;
	GSList        *filters = NULL;
	gchar        **mime_types, **pattern, *tmp;
	int            i;

	/* All Files Filter */
	all_file_filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (all_file_filter, _("All Files"));
	gtk_file_filter_add_pattern (all_file_filter, "*");

	/* All Image Filter */
	all_img_filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (all_img_filter, _("All Images"));

	/* Individual image filters */
	formats = gdk_pixbuf_get_formats ();
	for (it = formats; it != NULL; it = it->next) {
		gchar *filter_name;
		GdkPixbufFormat *format;
		filter = gtk_file_filter_new ();
		gchar *description, *name;

		format = (GdkPixbufFormat*) it->data;

		/* Filter name: First description then file extension, eg. "The PNG-Format (*.png)".*/
		description = gdk_pixbuf_format_get_description (format);
		name = gdk_pixbuf_format_get_name (format);
		filter_name = g_strdup_printf (_("%s (*.%s)"), description, name);
		g_free (description);
		g_free (name);
		gtk_file_filter_set_name (filter, filter_name);
		g_free (filter_name);

		mime_types = gdk_pixbuf_format_get_mime_types ((GdkPixbufFormat *) it->data);
		for (i = 0; mime_types[i] != NULL; i++) {
			gtk_file_filter_add_mime_type (filter, mime_types[i]);
			gtk_file_filter_add_mime_type (all_img_filter, mime_types[i]);
		}
		g_strfreev (mime_types);
 
		pattern = gdk_pixbuf_format_get_extensions ((GdkPixbufFormat *) it->data);
		for (i = 0; pattern[i] != NULL; i++) {
			tmp = g_strconcat ("*.", pattern[i], NULL);
			gtk_file_filter_add_pattern (filter, tmp);
			gtk_file_filter_add_pattern (all_img_filter, tmp);
			g_free (tmp);
		}
		g_strfreev (pattern);

		filters = g_slist_prepend (filters, filter);
	}
	g_slist_free (formats);

	/* Add filter to filechooser */
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), all_file_filter);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), all_img_filter);
	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (chooser), all_img_filter);

	for (it = filters; it != NULL; it = it->next) {
		gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser),
					     GTK_FILE_FILTER (it->data));
	}
	g_slist_free (filters);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE. Selection changed callback.                                     */
/*--------------------------------------------------------------------------*/
static void
img_selection_changed_cb (glObjectEditor *editor)
{
        gchar *filename;

        if (editor->priv->stop_signals) return;

	gl_debug (DEBUG_EDITOR, "START");

        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(editor->priv->img_file_button));
        if (filename != NULL)
        {
                /* Emit our "changed" signal */
                g_signal_emit (G_OBJECT (editor), gl_object_editor_signals[CHANGED], 0);
        }
        g_free (filename);

	gl_debug (DEBUG_EDITOR, "END");
}

