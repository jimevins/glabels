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
#include <gtk/gtklabel.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtkcombobox.h>
#include <gtk/gtktogglebutton.h>
#include <math.h>

#include "prefs.h"
#include "mygal/widget-color-combo.h"
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

static void style_changed_cb     (glObjectEditor       *editor);
static void bc_radio_toggled_cb  (glObjectEditor       *editor);


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare size page.                                             */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_prepare_bc_page (glObjectEditor       *editor)
{
	GList        *styles = NULL;

	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
	editor->priv->bc_page_vbox =
		glade_xml_get_widget (editor->priv->gui, "bc_page_vbox");
	editor->priv->bc_style_combo =
		glade_xml_get_widget (editor->priv->gui, "bc_style_combo");
	editor->priv->bc_text_check =
		glade_xml_get_widget (editor->priv->gui, "bc_text_check");
	editor->priv->bc_cs_check =
		glade_xml_get_widget (editor->priv->gui, "bc_cs_check");
	editor->priv->bc_color_combo =
		glade_xml_get_widget (editor->priv->gui, "bc_color_combo");
	editor->priv->bc_key_combo = 
		glade_xml_get_widget (editor->priv->gui, "bc_key_combo");	
	editor->priv->bc_key_radio = 
		glade_xml_get_widget (editor->priv->gui, "bc_key_radio");	
	editor->priv->bc_color_radio = 
		glade_xml_get_widget (editor->priv->gui, "bc_color_radio");	
	editor->priv->data_format_label =
		glade_xml_get_widget (editor->priv->gui, "data_format_label");
	editor->priv->data_ex_label =
		glade_xml_get_widget (editor->priv->gui, "data_ex_label");
	editor->priv->data_digits_label =
		glade_xml_get_widget (editor->priv->gui, "data_digits_label");
	editor->priv->data_digits_spin =
		glade_xml_get_widget (editor->priv->gui, "data_digits_spin");

	editor->priv->data_format_fixed_flag = FALSE;

	gl_util_combo_box_add_text_model ( GTK_COMBO_BOX(editor->priv->bc_style_combo));
	gl_util_combo_box_add_text_model ( GTK_COMBO_BOX(editor->priv->bc_key_combo));

	/* Load barcode styles */
	styles = gl_barcode_get_styles_list ();
	gl_util_combo_box_set_strings (GTK_COMBO_BOX(editor->priv->bc_style_combo),
				       styles);
	gl_barcode_free_styles_list (styles);

	/* Modify widgets */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->bc_color_radio), TRUE);
	gtk_widget_set_sensitive (editor->priv->bc_color_combo, TRUE);
	gtk_widget_set_sensitive (editor->priv->bc_key_combo, FALSE);
	
	/* Un-hide */
	gtk_widget_show_all (editor->priv->bc_page_vbox);

	/* Connect signals */
	g_signal_connect_swapped (G_OBJECT (editor->priv->bc_style_combo),
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
	g_signal_connect_swapped (G_OBJECT (editor->priv->bc_key_combo),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->bc_color_radio),
				  "toggled",
				  G_CALLBACK (bc_radio_toggled_cb),
				  G_OBJECT (editor));				  
	g_signal_connect_swapped (G_OBJECT (editor->priv->bc_key_radio),
				  "toggled",
				  G_CALLBACK (bc_radio_toggled_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->data_digits_spin),
				  "changed",
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
        gchar          *style_string = NULL;
	const gchar    *id;
	gchar          *ex_string = NULL;
	guint           digits;

        if (editor->priv->stop_signals) return;

        style_string =
		gtk_combo_box_get_active_text (GTK_COMBO_BOX (editor->priv->bc_style_combo));
                                                                                
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
                                                                                
		editor->priv->data_format_fixed_flag = !gl_barcode_can_freeform (id);
		digits = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->data_digits_spin));
		if (editor->priv->data_format_fixed_flag) {
			digits = gl_barcode_get_prefered_n(id);
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->data_digits_spin), 
						   digits);
		}
                                                                                
		ex_string = gl_barcode_default_digits (id, digits);
		gtk_label_set_text (GTK_LABEL(editor->priv->data_ex_label), ex_string);

		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->data_literal_radio))) {
			gtk_widget_set_sensitive (editor->priv->data_format_label, FALSE);
			gtk_widget_set_sensitive (editor->priv->data_ex_label, FALSE);
			gtk_widget_set_sensitive (editor->priv->data_digits_label, FALSE);
			gtk_widget_set_sensitive (editor->priv->data_digits_spin, FALSE);
		} else {
			gtk_widget_set_sensitive (editor->priv->data_format_label, TRUE);
			gtk_widget_set_sensitive (editor->priv->data_ex_label, TRUE);
			gtk_widget_set_sensitive (editor->priv->data_digits_label,
						  !editor->priv->data_format_fixed_flag);
			gtk_widget_set_sensitive (editor->priv->data_digits_spin,
						  !editor->priv->data_format_fixed_flag);
		}
 
                /* Emit our "changed" signal */
                g_signal_emit (G_OBJECT (editor), gl_object_editor_signals[CHANGED], 0);
        }
                                                                                
        g_free (style_string);
        g_free (ex_string);
}
                                                                                
/*****************************************************************************/
/* Set barcode style.                                                        */
/*****************************************************************************/
void
gl_object_editor_set_bc_style (glObjectEditor      *editor,
			       gchar               *id,
			       gboolean             text_flag,
			       gboolean             checksum_flag,
			       guint                format_digits)
{
	const gchar *style_string;
	gchar       *ex_string;
 
	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

        style_string = gl_barcode_id_to_name (id);
 
	gl_util_combo_box_set_active_text (GTK_COMBO_BOX (editor->priv->bc_style_combo),
					   style_string);
 
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->bc_text_check),
                                      text_flag);
 
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->bc_cs_check),
                                      checksum_flag);

	gtk_widget_set_sensitive (editor->priv->bc_text_check,
				  gl_barcode_text_optional (id));
	gtk_widget_set_sensitive (editor->priv->bc_cs_check,
				  gl_barcode_csum_optional (id));

	editor->priv->data_format_fixed_flag = !gl_barcode_can_freeform (id);

	if (editor->priv->data_format_fixed_flag) {
		format_digits = gl_barcode_get_prefered_n (id);
	}

	ex_string = gl_barcode_default_digits (id, format_digits);
	gtk_label_set_text (GTK_LABEL(editor->priv->data_ex_label), ex_string);
	g_free (ex_string);

        gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->data_digits_spin), 
				   format_digits);

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->data_literal_radio))) {
		gtk_widget_set_sensitive (editor->priv->data_format_label, FALSE);
		gtk_widget_set_sensitive (editor->priv->data_ex_label, FALSE);
		gtk_widget_set_sensitive (editor->priv->data_digits_label, FALSE);
		gtk_widget_set_sensitive (editor->priv->data_digits_spin, FALSE);
        } else {
		gtk_widget_set_sensitive (editor->priv->data_format_label, TRUE);
		gtk_widget_set_sensitive (editor->priv->data_ex_label, TRUE);
		gtk_widget_set_sensitive (editor->priv->data_digits_label,
					  !editor->priv->data_format_fixed_flag);
		gtk_widget_set_sensitive (editor->priv->data_digits_spin,
					  !editor->priv->data_format_fixed_flag);
	}
 
        editor->priv->stop_signals = FALSE;

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Query barcode style.                                                      */
/*****************************************************************************/
void
gl_object_editor_get_bc_style (glObjectEditor      *editor,
			       gchar              **id,
			       gboolean            *text_flag,
			       gboolean            *checksum_flag,
			       guint               *format_digits)
{
        gchar *style_string;

	gl_debug (DEBUG_EDITOR, "START");
                                                                                
        style_string =
		gtk_combo_box_get_active_text (GTK_COMBO_BOX (editor->priv->bc_style_combo));
        *id = g_strdup (gl_barcode_name_to_id (style_string));
                                                                                
        *text_flag =
            gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->bc_text_check));
                                                                                
        *checksum_flag =
            gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->bc_cs_check));
                                                                                

	*format_digits =
		gtk_spin_button_get_value (GTK_SPIN_BUTTON(editor->priv->data_digits_spin));

        g_free (style_string);

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Set bc color.                                                             */
/*****************************************************************************/
void
gl_object_editor_set_bc_color (glObjectEditor      *editor,
			       gboolean             merge_flag,
			       glColorNode         *color_node)
{
	GdkColor *gdk_color;

	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

	gtk_widget_set_sensitive (editor->priv->bc_key_radio, merge_flag);

	if ( color_node->color == GL_COLOR_NONE ) {

		color_combo_set_color_to_default (COLOR_COMBO(editor->priv->bc_color_combo));

	} else {

		gdk_color = gl_color_to_gdk_color (color_node->color);
		color_combo_set_color (COLOR_COMBO(editor->priv->bc_color_combo),
					   gdk_color);
		g_free (gdk_color);
	}
	
	if (!color_node->field_flag || !merge_flag) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
						  (editor->priv->bc_color_radio), TRUE); 
		gtk_widget_set_sensitive (editor->priv->bc_color_combo, TRUE);
		gtk_widget_set_sensitive (editor->priv->bc_key_combo, FALSE);
		
	} else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
						  (editor->priv->bc_key_radio), TRUE); 
		gtk_widget_set_sensitive (editor->priv->bc_color_combo, FALSE);
		gtk_widget_set_sensitive (editor->priv->bc_key_combo, TRUE);
		
		gl_util_combo_box_set_active_text (GTK_COMBO_BOX (editor->priv->bc_key_combo),
						   color_node->key);
	}	
	
        editor->priv->stop_signals = FALSE;
	
	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Query bc color.                                                           */
/*****************************************************************************/
glColorNode*
gl_object_editor_get_bc_color (glObjectEditor      *editor)
{
	GdkColor     *gdk_color;
	glColorNode  *color_node;
	gboolean      is_default;

	gl_debug (DEBUG_EDITOR, "START");

	color_node = gl_color_node_new_default ();
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->bc_key_radio))) {
		color_node->field_flag = TRUE;
		color_node->color = gl_prefs->default_line_color;
		color_node->key = 
			gtk_combo_box_get_active_text (GTK_COMBO_BOX (editor->priv->bc_key_combo));
	} else {
		color_node->field_flag = FALSE;
		color_node->key = NULL;
		gdk_color = color_combo_get_color (COLOR_COMBO(editor->priv->bc_color_combo),
                                           &is_default);

		if (is_default) {
			color_node->color = gl_prefs->default_line_color;
		} else {
			color_node->color = gl_color_from_gdk_color (gdk_color);
		}
	}
	
	gl_debug (DEBUG_EDITOR, "END");

	return color_node;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  barcode color radio callback.                                  */
/*--------------------------------------------------------------------------*/
static void
bc_radio_toggled_cb (glObjectEditor *editor)
{
        if (editor->priv->stop_signals) return;

        gl_debug (DEBUG_EDITOR, "START");
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->bc_color_radio))) {
                gtk_widget_set_sensitive (editor->priv->bc_color_combo, TRUE);
                gtk_widget_set_sensitive (editor->priv->bc_key_combo, FALSE);
    } else {
                gtk_widget_set_sensitive (editor->priv->bc_color_combo, FALSE);
                gtk_widget_set_sensitive (editor->priv->bc_key_combo, TRUE);
		
	}
 
    /* Emit our "changed" signal */
    g_signal_emit (G_OBJECT (editor), gl_object_editor_signals[CHANGED], 0);
 
    gl_debug (DEBUG_EDITOR, "END");
}
