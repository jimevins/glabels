/*
 *  object-editor-bc-page.c
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
#include "field-button.h"
#include "combo-util.h"
#include "builder-util.h"
#include "bc-backends.h"

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

static void backend_changed_cb          (glObjectEditor       *editor);
static void style_changed_cb            (glObjectEditor       *editor);
static void bc_radio_toggled_cb         (glObjectEditor       *editor);
static void data_digits_spin_changed_cb (glObjectEditor       *editor);


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare size page.                                             */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_prepare_bc_page (glObjectEditor       *editor)
{
	GList        *backends = NULL;
        GList        *p;

	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
        gl_builder_util_get_widgets (editor->priv->builder,
                                     "bc_page_vbox",      &editor->priv->bc_page_vbox,
                                     "bc_backend_combo",  &editor->priv->bc_backend_combo,
                                     "bc_style_combo",    &editor->priv->bc_style_combo,
                                     "bc_text_check",     &editor->priv->bc_text_check,
                                     "bc_cs_check",       &editor->priv->bc_cs_check,
                                     "bc_color_hbox",     &editor->priv->bc_color_hbox,
                                     "bc_key_hbox",       &editor->priv->bc_key_hbox,
                                     "bc_key_radio",      &editor->priv->bc_key_radio,
                                     "bc_color_radio",    &editor->priv->bc_color_radio,
                                     "data_format_label", &editor->priv->data_format_label,
                                     "data_ex_label",     &editor->priv->data_ex_label,
                                     "data_digits_label", &editor->priv->data_digits_label,
                                     "data_digits_spin",  &editor->priv->data_digits_spin,
                                     NULL);

	editor->priv->data_format_fixed_flag = FALSE;

	editor->priv->bc_color_combo = gl_color_combo_new (_("Default"),
                                                           GL_COLOR_BC_DEFAULT,
                                                           gl_prefs_model_get_default_line_color (gl_prefs));
        gtk_box_pack_start (GTK_BOX (editor->priv->bc_color_hbox),
                            editor->priv->bc_color_combo,
                            FALSE, FALSE, 0);

        editor->priv->bc_key_combo = gl_field_button_new (NULL);
        gtk_box_pack_start (GTK_BOX (editor->priv->bc_key_hbox),
                            editor->priv->bc_key_combo,
                            TRUE, TRUE, 0);

	gl_combo_util_add_text_model ( GTK_COMBO_BOX(editor->priv->bc_backend_combo));
	gl_combo_util_add_text_model ( GTK_COMBO_BOX(editor->priv->bc_style_combo));

	/* Load barcode backends */
	backends = gl_barcode_backends_get_backend_list ();
	gl_combo_util_set_strings (GTK_COMBO_BOX(editor->priv->bc_backend_combo),
                                   backends);

        /* Kludge: Load styles for each backend once, so that when they are loaded for real the size of
         * of the widget has already been established and does't cause the sidebar to change size. */
        for ( p = backends; p != NULL; p=p->next )
        {
                gl_object_editor_load_bc_styles (editor, gl_barcode_backends_backend_name_to_id ((gchar *)p->data));
        }

	gl_barcode_backends_free_backend_list (backends);

	/* Modify widgets */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->bc_color_radio), TRUE);
	gtk_widget_set_sensitive (editor->priv->bc_color_combo, TRUE);
	gtk_widget_set_sensitive (editor->priv->bc_key_combo, FALSE);
	
	/* Un-hide */
	gtk_widget_show_all (editor->priv->bc_page_vbox);

	/* Connect signals */
	g_signal_connect_swapped (G_OBJECT (editor->priv->bc_backend_combo),
				  "changed",
				  G_CALLBACK (backend_changed_cb),
				  G_OBJECT (editor));
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
				  G_CALLBACK (data_digits_spin_changed_cb),
				  G_OBJECT (editor));

	gl_debug (DEBUG_EDITOR, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when backend has changed.                         */
/*--------------------------------------------------------------------------*/
static void
backend_changed_cb (glObjectEditor       *editor)
{
        gchar          *backend_name = NULL;
	const gchar    *backend_id;
        const gchar    *style_name;

        if (editor->priv->stop_signals) return;

        backend_name =
		gtk_combo_box_get_active_text (GTK_COMBO_BOX (editor->priv->bc_backend_combo));

        backend_id = gl_barcode_backends_backend_name_to_id (backend_name);

        gl_object_editor_load_bc_styles (editor, backend_id);

        style_name = gl_barcode_backends_style_id_to_name (backend_id, NULL);
        gl_combo_util_set_active_text (GTK_COMBO_BOX (editor->priv->bc_style_combo),
                                       style_name);

        g_free (backend_name);
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when style has changed.                           */
/*--------------------------------------------------------------------------*/
static void
style_changed_cb (glObjectEditor       *editor)
{
        gchar          *backend_name = NULL;
	const gchar    *backend_id;
        gchar          *style_string = NULL;
	const gchar    *id;
	gchar          *ex_string = NULL;
	guint           digits;

        if (editor->priv->stop_signals) return;

        backend_name =
		gtk_combo_box_get_active_text (GTK_COMBO_BOX (editor->priv->bc_backend_combo));

        backend_id = gl_barcode_backends_backend_name_to_id (backend_name);

        style_string =
		gtk_combo_box_get_active_text (GTK_COMBO_BOX (editor->priv->bc_style_combo));

        /* Don't emit if entry is empty. */
        if ( style_string && *style_string != 0 ) {
                id = gl_barcode_backends_style_name_to_id (backend_id, style_string);
                                                                                
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(editor->priv->bc_text_check),
					      gl_barcode_backends_style_can_text (backend_id, id));
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(editor->priv->bc_cs_check),
					      gl_barcode_backends_style_can_csum (backend_id, id));
                                                                                
                gtk_widget_set_sensitive (editor->priv->bc_text_check,
                                          gl_barcode_backends_style_text_optional (backend_id, id));
                gtk_widget_set_sensitive (editor->priv->bc_cs_check,
                                          gl_barcode_backends_style_csum_optional (backend_id, id));
                                                                                
		editor->priv->data_format_fixed_flag = !gl_barcode_backends_style_can_freeform (backend_id, id);
                digits = gl_barcode_backends_style_get_prefered_n(backend_id, id);
                gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->data_digits_spin), digits);
                                                                                
		ex_string = gl_barcode_backends_style_default_digits (backend_id, id, digits);
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
 
                gl_object_editor_changed_cb (editor);
        }
                                                                                
        g_free (style_string);
        g_free (ex_string);
}
                                                                                

/*****************************************************************************/
/* Load barcode styles based on backend_id.                                  */
/*****************************************************************************/
void
gl_object_editor_load_bc_styles (glObjectEditor      *editor,
                                 const gchar         *backend_id)
{
        GList       *styles;
 
	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

	styles = gl_barcode_backends_get_styles_list (backend_id);
	gl_combo_util_set_strings (GTK_COMBO_BOX(editor->priv->bc_style_combo), styles);
	gl_barcode_backends_free_styles_list (styles);

        editor->priv->stop_signals = FALSE;

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Set barcode style.                                                        */
/*****************************************************************************/
void
gl_object_editor_set_bc_style (glObjectEditor            *editor,
			       const glLabelBarcodeStyle *bc_style)
{
	const gchar *backend_name;
	const gchar *style_name;
	gchar       *ex_string;
        gint         format_digits;
 
	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

        backend_name = gl_barcode_backends_backend_id_to_name (bc_style->backend_id);
        style_name   = gl_barcode_backends_style_id_to_name (bc_style->backend_id, bc_style->id);

	gl_combo_util_set_active_text (GTK_COMBO_BOX (editor->priv->bc_backend_combo),
                                       backend_name);
 	gl_combo_util_set_active_text (GTK_COMBO_BOX (editor->priv->bc_style_combo),
                                       style_name);
 
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->bc_text_check),
                                      bc_style->text_flag);
 
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->bc_cs_check),
                                      bc_style->checksum_flag);

	gtk_widget_set_sensitive (editor->priv->bc_text_check,
				  gl_barcode_backends_style_text_optional (bc_style->backend_id, bc_style->id));
	gtk_widget_set_sensitive (editor->priv->bc_cs_check,
				  gl_barcode_backends_style_csum_optional (bc_style->backend_id, bc_style->id));

	editor->priv->data_format_fixed_flag = !gl_barcode_backends_style_can_freeform (bc_style->backend_id, bc_style->id);

        format_digits = bc_style->format_digits;
	if (editor->priv->data_format_fixed_flag) {
		format_digits = gl_barcode_backends_style_get_prefered_n (bc_style->backend_id, bc_style->id);
	}

	ex_string = gl_barcode_backends_style_default_digits (bc_style->backend_id, bc_style->id, format_digits);
	gtk_label_set_text (GTK_LABEL(editor->priv->data_ex_label), ex_string);
	g_free (ex_string);

        gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->data_digits_spin), format_digits);

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
glLabelBarcodeStyle *
gl_object_editor_get_bc_style (glObjectEditor      *editor)
{
        gchar       *backend_name;
        const gchar *backend_id;
        gchar       *style_name;
        const gchar *id;

        glLabelBarcodeStyle *bc_style;

	gl_debug (DEBUG_EDITOR, "START");
                                                                                
        backend_name = gtk_combo_box_get_active_text (GTK_COMBO_BOX (editor->priv->bc_backend_combo));
        backend_id = gl_barcode_backends_backend_name_to_id (backend_name);

        style_name = gtk_combo_box_get_active_text (GTK_COMBO_BOX (editor->priv->bc_style_combo));
        id = gl_barcode_backends_style_name_to_id (backend_id, style_name);

        bc_style = gl_label_barcode_style_new ();

        gl_label_barcode_style_set_backend_id (bc_style, backend_id);
        gl_label_barcode_style_set_style_id (bc_style, id);

        bc_style->text_flag     = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->bc_text_check));
        bc_style->checksum_flag = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->bc_cs_check));
	bc_style->format_digits = gtk_spin_button_get_value (GTK_SPIN_BUTTON(editor->priv->data_digits_spin));

        g_free (backend_name);
        g_free (style_name);

	gl_debug (DEBUG_EDITOR, "END");

        return bc_style;
}


/*****************************************************************************/
/* Set bc color.                                                             */
/*****************************************************************************/
void
gl_object_editor_set_bc_color (glObjectEditor      *editor,
			       gboolean             merge_flag,
			       glColorNode         *color_node)
{
	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

	gtk_widget_set_sensitive (editor->priv->bc_key_radio, merge_flag);

	if ( color_node->color == GL_COLOR_NONE ) {

		gl_color_combo_set_to_default (GL_COLOR_COMBO(editor->priv->bc_color_combo));

	} else {

		gl_color_combo_set_color (GL_COLOR_COMBO(editor->priv->bc_color_combo),
                                          color_node->color);
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
		
		gl_field_button_set_key (GL_FIELD_BUTTON (editor->priv->bc_key_combo),
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
	guint         color;
	glColorNode  *color_node;
	gboolean      is_default;

	gl_debug (DEBUG_EDITOR, "START");

	color_node = gl_color_node_new_default ();
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->bc_key_radio))) {
		color_node->field_flag = TRUE;
		color_node->color = gl_prefs_model_get_default_line_color (gl_prefs);
		color_node->key = 
			gl_field_button_get_key (GL_FIELD_BUTTON (editor->priv->bc_key_combo));
	} else {
		color_node->field_flag = FALSE;
		color_node->key = NULL;
		color = gl_color_combo_get_color (GL_COLOR_COMBO(editor->priv->bc_color_combo),
                                                  &is_default);

		if (is_default) {
			color_node->color = gl_prefs_model_get_default_line_color (gl_prefs);
		} else {
			color_node->color = color;
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
 
        gl_object_editor_changed_cb (editor);
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  digits spin changed callback.                                  */
/*--------------------------------------------------------------------------*/
static void
data_digits_spin_changed_cb (glObjectEditor *editor)
{
        gchar          *backend_name;
        const gchar    *backend_id;
        gchar          *style_name;
        const gchar    *id;
        guint           digits;
        gchar          *ex_string;

        if (editor->priv->stop_signals) return;

        backend_name = gtk_combo_box_get_active_text (GTK_COMBO_BOX (editor->priv->bc_backend_combo));
        backend_id = gl_barcode_backends_backend_name_to_id (backend_name);

        style_name = gtk_combo_box_get_active_text (GTK_COMBO_BOX (editor->priv->bc_style_combo));
        if ( *style_name != 0 ) {
                id = gl_barcode_backends_style_name_to_id (backend_id, style_name);

                digits = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->data_digits_spin));
                ex_string = gl_barcode_backends_style_default_digits (backend_id, id, digits);
                gtk_label_set_text (GTK_LABEL(editor->priv->data_ex_label), ex_string);
        }

        g_free (backend_name);
        g_free (style_name);
        g_free (ex_string);

        gl_object_editor_changed_cb (editor);
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
