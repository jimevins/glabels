/*
 *  object-editor-text-page.c
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
#include <gtk/gtkspinbutton.h>
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

static void align_toggle_cb (GtkToggleButton *toggle,
			     glObjectEditor  *editor);
static void text_radio_toggled_cb              (glObjectEditor        *editor);


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare size page.                                             */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_prepare_text_page (glObjectEditor       *editor)
{
	GList        *family_names = NULL;

	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
        gl_util_get_builder_widgets (editor->priv->builder,
                                     "text_page_vbox",         &editor->priv->text_page_vbox,
                                     "text_family_combo",      &editor->priv->text_family_combo,
                                     "text_size_spin",         &editor->priv->text_size_spin,
                                     "text_bold_toggle",       &editor->priv->text_bold_toggle,
                                     "text_italic_toggle",     &editor->priv->text_italic_toggle,
                                     "text_color_hbox",        &editor->priv->text_color_hbox,
                                     "text_color_radio",       &editor->priv->text_color_radio,
                                     "text_color_key_radio",   &editor->priv->text_color_key_radio,
                                     "text_color_key_combo",   &editor->priv->text_color_key_combo,
                                     "text_left_toggle",       &editor->priv->text_left_toggle,
                                     "text_center_toggle",     &editor->priv->text_center_toggle,
                                     "text_right_toggle",      &editor->priv->text_right_toggle,
                                     "text_line_spacing_spin", &editor->priv->text_line_spacing_spin,
                                     "text_auto_shrink_check", &editor->priv->text_auto_shrink_check,
                                     NULL);

	editor->priv->text_color_combo = gl_color_combo_new (_("Default"),
                                                             GL_COLOR_TEXT_DEFAULT,
                                                             gl_prefs->default_text_color);
        gtk_box_pack_start (GTK_BOX (editor->priv->text_color_hbox),
                            editor->priv->text_color_combo,
                            FALSE, FALSE, 0);

	gl_util_combo_box_add_text_model ( GTK_COMBO_BOX(editor->priv->text_family_combo));
	gl_util_combo_box_add_text_model ( GTK_COMBO_BOX(editor->priv->text_color_key_combo));

	/* Load family names */
	family_names = gl_util_get_font_family_list ();
	gl_util_combo_box_set_strings (GTK_COMBO_BOX(editor->priv->text_family_combo),
				       family_names);

	/* Modify widgets */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->text_color_radio), TRUE);
	gtk_widget_set_sensitive (editor->priv->text_color_combo, TRUE);
        gtk_widget_set_sensitive (editor->priv->text_color_key_combo, FALSE);

	/* Un-hide */
	gtk_widget_show_all (editor->priv->text_page_vbox);

	/* Connect signals */
	g_signal_connect_swapped (G_OBJECT (editor->priv->text_family_combo),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->text_size_spin),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->text_bold_toggle),
				  "toggled",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->text_italic_toggle),
				  "toggled",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->text_color_combo),
				  "color_changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->text_color_key_combo),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->text_color_radio),
				  "toggled",
				  G_CALLBACK (text_radio_toggled_cb),
				  G_OBJECT (editor));				  
	g_signal_connect_swapped (G_OBJECT (editor->priv->text_color_key_radio),
				  "toggled",
				  G_CALLBACK (text_radio_toggled_cb),
				  G_OBJECT (editor));

	g_signal_connect (G_OBJECT (editor->priv->text_left_toggle),
			  "toggled",
			  G_CALLBACK (align_toggle_cb),
			  G_OBJECT (editor));
	g_signal_connect (G_OBJECT (editor->priv->text_center_toggle),
			  "toggled",
			  G_CALLBACK (align_toggle_cb),
			  G_OBJECT (editor));
	g_signal_connect (G_OBJECT (editor->priv->text_right_toggle),
			  "toggled",
			  G_CALLBACK (align_toggle_cb),
			  G_OBJECT (editor));

	g_signal_connect_swapped (G_OBJECT (editor->priv->text_line_spacing_spin),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));

	g_signal_connect_swapped (G_OBJECT (editor->priv->text_auto_shrink_check),
				  "toggled",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));

	gl_debug (DEBUG_EDITOR, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Alignment togglebutton callback.                               */
/*--------------------------------------------------------------------------*/
static void
align_toggle_cb (GtkToggleButton *toggle,
		 glObjectEditor  *editor)
{
        if (editor->priv->stop_signals) return;

        if (gtk_toggle_button_get_active (toggle)) {
 
                if (GTK_WIDGET (toggle) == GTK_WIDGET (editor->priv->text_left_toggle)) {
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                                      (editor->priv->text_center_toggle),
                                                      FALSE);
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                                      (editor->priv->text_right_toggle),
                                                      FALSE);
                } else if (GTK_WIDGET (toggle) ==
                           GTK_WIDGET (editor->priv->text_center_toggle)) {
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                                      (editor->priv->text_left_toggle),
                                                      FALSE);
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                                      (editor->priv->text_right_toggle),
                                                      FALSE);
                } else if (GTK_WIDGET (toggle) ==
                           GTK_WIDGET (editor->priv->text_right_toggle)) {
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                                      (editor->priv->text_left_toggle),
                                                      FALSE);
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                                      (editor->priv->text_center_toggle),
                                                      FALSE);
                }

		/* Emit our "changed" signal */
		g_signal_emit (G_OBJECT (editor), gl_object_editor_signals[CHANGED], 0);
        }

}


/*****************************************************************************/
/* Set font family.                                                          */
/*****************************************************************************/
void
gl_object_editor_set_font_family (glObjectEditor      *editor,
				  const gchar         *font_family)
{
        GList    *family_names;
	gchar    *old_font_family;
	gchar    *good_font_family;

	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

	old_font_family = gtk_combo_box_get_active_text (GTK_COMBO_BOX (editor->priv->text_family_combo));

        if ( !old_font_family || g_utf8_collate( old_font_family, font_family ) )
        {

                /* Make sure we have a valid font family.  if not provide a good default. */
                family_names = gl_util_get_font_family_list ();
                if (g_list_find_custom (family_names, font_family, (GCompareFunc)g_utf8_collate)) {
                        good_font_family = g_strdup (font_family);
                } else {
                        if (family_names != NULL) {
                                good_font_family = g_strdup (family_names->data); /* 1st entry */
                        } else {
                                good_font_family = NULL;
                        }
                }
                gl_util_combo_box_set_active_text (GTK_COMBO_BOX (editor->priv->text_family_combo), good_font_family);
                g_free (good_font_family);
        }

        g_free (old_font_family);

        editor->priv->stop_signals = FALSE;

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Query font family.                                                        */
/*****************************************************************************/
gchar *
gl_object_editor_get_font_family (glObjectEditor      *editor)
{
	gchar *font_family;

	gl_debug (DEBUG_EDITOR, "START");

	font_family = gtk_combo_box_get_active_text (GTK_COMBO_BOX (editor->priv->text_family_combo));

	gl_debug (DEBUG_EDITOR, "END");

	return font_family;
}


/*****************************************************************************/
/* Set font size.                                                            */
/*****************************************************************************/
void
gl_object_editor_set_font_size (glObjectEditor      *editor,
				gdouble              font_size)
{
	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

        gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->text_size_spin),
                                   font_size);

        editor->priv->stop_signals = FALSE;

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Query font size.                                                          */
/*****************************************************************************/
gdouble
gl_object_editor_get_font_size (glObjectEditor      *editor)
{
	gdouble font_size;

	gl_debug (DEBUG_EDITOR, "START");

	font_size =
		gtk_spin_button_get_value (GTK_SPIN_BUTTON(editor->priv->text_size_spin));

	gl_debug (DEBUG_EDITOR, "END");

	return font_size;
}


/*****************************************************************************/
/* Set font weight.                                                          */
/*****************************************************************************/
void
gl_object_editor_set_font_weight (glObjectEditor      *editor,
				  PangoWeight          font_weight)
{
	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->text_bold_toggle),
                                      (font_weight == PANGO_WEIGHT_BOLD));

        editor->priv->stop_signals = FALSE;

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Query font weight.                                                        */
/*****************************************************************************/
PangoWeight
gl_object_editor_get_font_weight (glObjectEditor      *editor)
{
	PangoWeight font_weight;

	gl_debug (DEBUG_EDITOR, "START");

        if (gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON (editor->priv->text_bold_toggle))) {
                font_weight = PANGO_WEIGHT_BOLD;
        } else {
                font_weight = PANGO_WEIGHT_NORMAL;
        }

	gl_debug (DEBUG_EDITOR, "END");

	return font_weight;
}


/*****************************************************************************/
/* Set font italic flag.                                                     */
/*****************************************************************************/
void
gl_object_editor_set_font_italic_flag (glObjectEditor      *editor,
				       gboolean             font_italic_flag)
{
	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->text_italic_toggle),
                                      font_italic_flag);

        editor->priv->stop_signals = FALSE;

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Query font italic flag.                                                   */
/*****************************************************************************/
gboolean
gl_object_editor_get_font_italic_flag (glObjectEditor      *editor)
{
	gboolean italic_flag;

	gl_debug (DEBUG_EDITOR, "START");

	italic_flag =
		gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
					      (editor->priv->text_italic_toggle));

	gl_debug (DEBUG_EDITOR, "END");

	return italic_flag;
}


/*****************************************************************************/
/* Set text alignment.                                                       */
/*****************************************************************************/
void
gl_object_editor_set_text_alignment (glObjectEditor      *editor,
				     PangoAlignment       align)
{
	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->text_left_toggle),
                                      (align == PANGO_ALIGN_LEFT));
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->text_center_toggle),
                                      (align == PANGO_ALIGN_CENTER));
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->text_right_toggle),
                                      (align == PANGO_ALIGN_RIGHT));

        editor->priv->stop_signals = FALSE;

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Query text alignment.                                                     */
/*****************************************************************************/
PangoAlignment
gl_object_editor_get_text_alignment (glObjectEditor      *editor)
{
	PangoAlignment align;

	gl_debug (DEBUG_EDITOR, "START");

        if (gtk_toggle_button_get_active
            (GTK_TOGGLE_BUTTON (editor->priv->text_left_toggle))) {
                align = PANGO_ALIGN_LEFT;
        } else
            if (gtk_toggle_button_get_active
                (GTK_TOGGLE_BUTTON (editor->priv->text_right_toggle))) {
                align = PANGO_ALIGN_RIGHT;
        } else
            if (gtk_toggle_button_get_active
                (GTK_TOGGLE_BUTTON (editor->priv->text_center_toggle))) {
                align = PANGO_ALIGN_CENTER;
        } else {
                align = PANGO_ALIGN_LEFT;       /* Should not happen. */
        }

	gl_debug (DEBUG_EDITOR, "END");

	return align;
}


/*****************************************************************************/
/* Set text color.                                                           */
/*****************************************************************************/
void
gl_object_editor_set_text_color (glObjectEditor      *editor,
				 gboolean             merge_flag,
				 glColorNode         *text_color_node)
{
	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

	gl_debug (DEBUG_EDITOR, "color field %s(%d) / %X", text_color_node->key, text_color_node->field_flag, text_color_node->color);
	
	gtk_widget_set_sensitive (editor->priv->text_color_key_radio, merge_flag);

	if ( text_color_node->color == GL_COLOR_NONE ) {

		gl_color_combo_set_to_default (GL_COLOR_COMBO(editor->priv->text_color_combo));

	} else {
		
                gl_color_combo_set_color (GL_COLOR_COMBO(editor->priv->text_color_combo),
                                          text_color_node->color);
	}
	
	if (!text_color_node->field_flag) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
						  (editor->priv->text_color_radio), TRUE); 
		gtk_widget_set_sensitive (editor->priv->text_color_combo, TRUE);
		gl_debug (DEBUG_EDITOR, "color field false 0");
		gtk_widget_set_sensitive (editor->priv->text_color_key_combo, FALSE);
		
	} else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
						  (editor->priv->text_color_key_radio), TRUE); 
		gtk_widget_set_sensitive (editor->priv->text_color_combo, FALSE);
		gtk_widget_set_sensitive (editor->priv->text_color_key_combo, TRUE);
		
		gl_debug (DEBUG_EDITOR, "color field true 1");
		gl_util_combo_box_set_active_text (GTK_COMBO_BOX (editor->priv->text_color_key_combo), "");
		gl_debug (DEBUG_EDITOR, "color field true 2");
	}

        editor->priv->stop_signals = FALSE;

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Query text color.                                                         */
/*****************************************************************************/
glColorNode*
gl_object_editor_get_text_color (glObjectEditor      *editor)
{
	guint        color;
	glColorNode *color_node;
	gboolean     is_default;

	gl_debug (DEBUG_EDITOR, "START");

	color_node = gl_color_node_new_default ();
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->text_color_key_radio))) {
		color_node->field_flag = TRUE;
		color_node->color = gl_prefs->default_text_color;
		color_node->key = 
			gtk_combo_box_get_active_text (GTK_COMBO_BOX (editor->priv->text_color_key_combo));
        } else {
		color_node->field_flag = FALSE;
		color_node->key = NULL;
		color = gl_color_combo_get_color (GL_COLOR_COMBO(editor->priv->text_color_combo),
                                                  &is_default);

                if (is_default) {
                        color_node->color = gl_prefs->default_text_color;
                } else {
                        color_node->color = color;
                }
	}      

	gl_debug (DEBUG_EDITOR, "END");

	return color_node;
}


/*****************************************************************************/
/* Set text line spacing.                                                    */
/*****************************************************************************/
void
gl_object_editor_set_text_line_spacing (glObjectEditor      *editor,
				        gdouble              text_line_spacing)
{
	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

        gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->text_line_spacing_spin),
                                   text_line_spacing);

        editor->priv->stop_signals = FALSE;

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Query text line spacing.                                                  */
/*****************************************************************************/
gdouble
gl_object_editor_get_text_line_spacing (glObjectEditor      *editor)
{
	gdouble text_line_spacing;

	gl_debug (DEBUG_EDITOR, "START");

	text_line_spacing = 
		gtk_spin_button_get_value (GTK_SPIN_BUTTON(editor->priv->text_line_spacing_spin));

	gl_debug (DEBUG_EDITOR, "END");

	return text_line_spacing;
}


/*****************************************************************************/
/* Set auto shrink checkbox.                                                 */
/*****************************************************************************/
void
gl_object_editor_set_text_auto_shrink (glObjectEditor      *editor,
				       gboolean             auto_shrink)
{
	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->text_auto_shrink_check),
                                      auto_shrink);

        editor->priv->stop_signals = FALSE;

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Query auto shrink checkbox.                                               */
/*****************************************************************************/
gboolean    gl_object_editor_get_text_auto_shrink (glObjectEditor      *editor)
{
	gboolean auto_shrink;

	gl_debug (DEBUG_EDITOR, "START");

	auto_shrink = 
		gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->text_auto_shrink_check));

	gl_debug (DEBUG_EDITOR, "END");

	return auto_shrink;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  color radio callback.                                           */
/*--------------------------------------------------------------------------*/
static void
text_radio_toggled_cb (glObjectEditor *editor)
{
        if (editor->priv->stop_signals) return;

        gl_debug (DEBUG_EDITOR, "START");
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->priv->text_color_radio))) {
                gtk_widget_set_sensitive (editor->priv->text_color_combo, TRUE);
                gtk_widget_set_sensitive (editor->priv->text_color_key_combo, FALSE);
        } else {
                gtk_widget_set_sensitive (editor->priv->text_color_combo, FALSE);
                gtk_widget_set_sensitive (editor->priv->text_color_key_combo, TRUE);
		
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
