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

static void style_changed_cb (glObjectEditor       *editor);


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare size page.                                             */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_prepare_bc_page (glObjectEditor       *editor)
{
	GList        *styles = NULL;
	GtkSizeGroup *label_size_group;
	GtkWidget    *label;

	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
	editor->priv->bc_page_vbox =
		glade_xml_get_widget (editor->priv->gui, "bc_page_vbox");
	editor->priv->bc_style_entry =
		glade_xml_get_widget (editor->priv->gui, "bc_style_entry");
	editor->priv->bc_style_combo =
		glade_xml_get_widget (editor->priv->gui, "bc_style_combo");
	editor->priv->bc_text_check =
		glade_xml_get_widget (editor->priv->gui, "bc_text_check");
	editor->priv->bc_cs_check =
		glade_xml_get_widget (editor->priv->gui, "bc_cs_check");
	editor->priv->bc_color_combo =
		glade_xml_get_widget (editor->priv->gui, "bc_color_combo");

	/* Load barcode styles */
	styles = gl_barcode_get_styles_list ();
	gtk_combo_set_popdown_strings (GTK_COMBO(editor->priv->bc_style_combo), styles);
	gl_barcode_free_styles_list (styles);

	/* Align label widths */
	label_size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	label = glade_xml_get_widget (editor->priv->gui, "bc_style_label");
	gtk_size_group_add_widget (label_size_group, label);
	label = glade_xml_get_widget (editor->priv->gui, "bc_color_label");
	gtk_size_group_add_widget (label_size_group, label);

	/* Un-hide */
	gtk_widget_show_all (editor->priv->bc_page_vbox);

	/* Connect signals */
	g_signal_connect_swapped (G_OBJECT (editor->priv->bc_style_entry),
				  "changed",
				  G_CALLBACK (style_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->bc_text_check),
				  "toggled",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->bc_cs_check),
				  "toggled",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->bc_color_combo),
				  "color_changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));

	gl_debug (DEBUG_EDITOR, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when style has changed.                           */
/*--------------------------------------------------------------------------*/
static void
style_changed_cb (glObjectEditor       *editor)
{
        gchar          *style_string;
	const gchar    *id;
                                                                                
        style_string =
                gtk_editable_get_chars (GTK_EDITABLE(editor->priv->bc_style_entry), 0, -1);
                                                                                
        /* Don't emit if entry is empty. */
        if ( *style_string != 0 ) {
                id = gl_barcode_name_to_id (style_string);
                                                                                
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(editor->priv->bc_text_check),
					      gl_barcode_can_text (id));
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(editor->priv->bc_cs_check),
					      gl_barcode_can_csum (id));
                                                                                
                gtk_widget_set_sensitive (editor->priv->bc_text_check,
                                          gl_barcode_text_optional (id));
                gtk_widget_set_sensitive (editor->priv->bc_cs_check,
                                          gl_barcode_csum_optional (id));
                                                                                
                                                                                
                /* Emit our "changed" signal */
                g_signal_emit (G_OBJECT (editor), gl_object_editor_signals[CHANGED], 0);
        }
                                                                                
        g_free (style_string);
}
                                                                                
/*****************************************************************************/
/* Set barcode style.                                                        */
/*****************************************************************************/
void
gl_object_editor_set_bc_style (glObjectEditor      *editor,
			       gchar               *id,
			       gboolean             text_flag,
			       gboolean             checksum_flag)
{
	const gchar *style_string;
        gint         pos;
 
	gl_debug (DEBUG_EDITOR, "START");

        g_signal_handlers_block_by_func (G_OBJECT(editor->priv->bc_style_entry),
                                         G_CALLBACK (style_changed_cb),
                                         editor);
        g_signal_handlers_block_by_func (G_OBJECT(editor->priv->bc_text_check),
                                         G_CALLBACK (gl_object_editor_changed_cb),
                                         editor);
        g_signal_handlers_block_by_func (G_OBJECT(editor->priv->bc_cs_check),
                                         G_CALLBACK (gl_object_editor_changed_cb),
                                         editor);

        style_string = gl_barcode_id_to_name (id);
 
        gtk_editable_delete_text (GTK_EDITABLE (editor->priv->bc_style_entry),
                                  0, -1);
 
        pos = 0;
        gtk_editable_insert_text (GTK_EDITABLE (editor->priv->bc_style_entry),
                                  style_string,
                                  strlen (style_string),
                                  &pos);
 
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->bc_text_check),
                                      text_flag);
 
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->bc_cs_check),
                                      checksum_flag);

	gtk_widget_set_sensitive (editor->priv->bc_text_check,
				  gl_barcode_text_optional (id));
	gtk_widget_set_sensitive (editor->priv->bc_cs_check,
				  gl_barcode_csum_optional (id));

        g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->bc_style_entry),
					   G_CALLBACK (style_changed_cb),
					   editor);
        g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->bc_text_check),
					   G_CALLBACK (gl_object_editor_changed_cb),
					   editor);
        g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->bc_cs_check),
					   G_CALLBACK (gl_object_editor_changed_cb),
					   editor);

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Query barcode style.                                                      */
/*****************************************************************************/
void
gl_object_editor_get_bc_style (glObjectEditor      *editor,
			       gchar              **id,
			       gboolean            *text_flag,
			       gboolean            *checksum_flag)
{
        gchar *style_string;

	gl_debug (DEBUG_EDITOR, "START");
                                                                                
        style_string =
                gtk_editable_get_chars (GTK_EDITABLE(editor->priv->bc_style_entry),
                                        0, -1);
        *id = g_strdup (gl_barcode_name_to_id (style_string));
                                                                                
        *text_flag =
            gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->bc_text_check));
                                                                                
        *checksum_flag =
            gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->bc_cs_check));
                                                                                
        g_free (style_string);

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Set bc color.                                                             */
/*****************************************************************************/
void
gl_object_editor_set_bc_color (glObjectEditor      *editor,
			       guint                bc_color)
{
	GdkColor *gdk_color;

	gl_debug (DEBUG_EDITOR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->bc_color_combo),
					 gl_object_editor_changed_cb,
					 editor);

        gdk_color = gl_color_to_gdk_color (bc_color);
        color_combo_set_color (COLOR_COMBO(editor->priv->bc_color_combo), gdk_color);
        g_free (gdk_color);

	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->bc_color_combo),
					   gl_object_editor_changed_cb,
					   editor);

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Query bc color.                                                           */
/*****************************************************************************/
guint
gl_object_editor_get_bc_color (glObjectEditor      *editor)
{
	GdkColor  *gdk_color;
	guint     color;
	gboolean  is_default;

	gl_debug (DEBUG_EDITOR, "START");

        gdk_color = color_combo_get_color (COLOR_COMBO(editor->priv->bc_color_combo),
                                           &is_default);
                                                                                
        if (is_default) {
                color = gl_prefs->default_line_color;
        } else {
                color = gl_color_from_gdk_color (gdk_color);
        }
                                                                                

	gl_debug (DEBUG_EDITOR, "END");

	return color;
}


