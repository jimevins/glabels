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

static void align_toggle_cb (GtkToggleButton *toggle,
			     glObjectEditor  *editor);


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare size page.                                             */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_prepare_text_page (glObjectEditor       *editor)
{
	GList        *family_names = NULL;
	GtkSizeGroup *label_size_group;
	GtkWidget    *label;

	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
	editor->priv->text_page_vbox =
		glade_xml_get_widget (editor->priv->gui, "text_page_vbox");
	editor->priv->text_family_entry =
		glade_xml_get_widget (editor->priv->gui, "text_family_entry");
	editor->priv->text_family_combo =
		glade_xml_get_widget (editor->priv->gui, "text_family_combo");
	editor->priv->text_size_spin =
		glade_xml_get_widget (editor->priv->gui, "text_size_spin");
	editor->priv->text_bold_toggle =
		glade_xml_get_widget (editor->priv->gui, "text_bold_toggle");
	editor->priv->text_italic_toggle =
		glade_xml_get_widget (editor->priv->gui, "text_italic_toggle");
	editor->priv->text_color_combo =
		glade_xml_get_widget (editor->priv->gui, "text_color_combo");
	editor->priv->text_left_toggle =
		glade_xml_get_widget (editor->priv->gui, "text_left_toggle");
	editor->priv->text_center_toggle =
		glade_xml_get_widget (editor->priv->gui, "text_center_toggle");
	editor->priv->text_right_toggle =
		glade_xml_get_widget (editor->priv->gui, "text_right_toggle");
	editor->priv->text_line_spacing_spin =
		glade_xml_get_widget (editor->priv->gui, "text_line_spacing_spin");

	/* Load family names */
	family_names = gnome_font_family_list ();
	gtk_combo_set_popdown_strings (GTK_COMBO(editor->priv->text_family_combo),
				       family_names);
	gnome_font_family_list_free (family_names);

	/* Align label widths */
	label_size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	label = glade_xml_get_widget (editor->priv->gui, "text_family_label");
	gtk_size_group_add_widget (label_size_group, label);
	label = glade_xml_get_widget (editor->priv->gui, "text_size_label");
	gtk_size_group_add_widget (label_size_group, label);
	label = glade_xml_get_widget (editor->priv->gui, "text_style_label");
	gtk_size_group_add_widget (label_size_group, label);
	label = glade_xml_get_widget (editor->priv->gui, "text_color_label");
	gtk_size_group_add_widget (label_size_group, label);
	label = glade_xml_get_widget (editor->priv->gui, "text_align_label");
	gtk_size_group_add_widget (label_size_group, label);
	label = glade_xml_get_widget (editor->priv->gui, "text_line_spacing_label");
	gtk_size_group_add_widget (label_size_group, label);

	/* Un-hide */
	gtk_widget_show_all (editor->priv->text_page_vbox);

	/* Connect signals */
	g_signal_connect_swapped (G_OBJECT (editor->priv->text_family_entry),
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

	gl_debug (DEBUG_EDITOR, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Alignment togglebutton callback.                               */
/*--------------------------------------------------------------------------*/
static void
align_toggle_cb (GtkToggleButton *toggle,
		 glObjectEditor  *editor)
{
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
	gchar    *good_font_family;

	gl_debug (DEBUG_EDITOR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->text_family_entry),
					 gl_object_editor_changed_cb,
					 editor);

        /* Make sure we have a valid font family.  if not provide a good default. */
        family_names = gnome_font_family_list ();
        if (g_list_find_custom (family_names, font_family, (GCompareFunc)g_utf8_collate)) {
                good_font_family = g_strdup (font_family);
        } else {
                if (family_names != NULL) {
                        good_font_family = g_strdup (family_names->data); /* 1st entry */
                } else {
                        good_font_family = NULL;
                }
        }
        gnome_font_family_list_free (family_names);
        gtk_entry_set_text (GTK_ENTRY (editor->priv->text_family_entry), good_font_family);
        g_free (good_font_family);

	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->text_family_entry),
					   gl_object_editor_changed_cb,
					   editor);

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

	font_family =
		gtk_editable_get_chars (GTK_EDITABLE (editor->priv->text_family_entry),
					0, -1);

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

	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->text_size_spin),
					 gl_object_editor_changed_cb,
					 editor);

        gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->text_size_spin),
                                   font_size);

	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->text_size_spin),
					   gl_object_editor_changed_cb,
					   editor);

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
				  GnomeFontWeight      font_weight)
{
	gl_debug (DEBUG_EDITOR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->text_bold_toggle),
					 gl_object_editor_changed_cb,
					 editor);

        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->text_bold_toggle),
                                      (font_weight == GNOME_FONT_BOLD));

	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->text_bold_toggle),
					   gl_object_editor_changed_cb,
					   editor);

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Query font weight.                                                        */
/*****************************************************************************/
GnomeFontWeight
gl_object_editor_get_font_weight (glObjectEditor      *editor)
{
	GnomeFontWeight font_weight;

	gl_debug (DEBUG_EDITOR, "START");

        if (gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON (editor->priv->text_bold_toggle))) {
                font_weight = GNOME_FONT_BOLD;
        } else {
                font_weight = GNOME_FONT_BOOK;
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

	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->text_italic_toggle),
					 gl_object_editor_changed_cb,
					 editor);

        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->text_italic_toggle),
                                      font_italic_flag);


	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->text_italic_toggle),
					   gl_object_editor_changed_cb,
					   editor);

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
				     GtkJustification     just)
{
	gl_debug (DEBUG_EDITOR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->text_left_toggle),
					 align_toggle_cb,
					 editor);
	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->text_center_toggle),
					 align_toggle_cb,
					 editor);
	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->text_right_toggle),
					 align_toggle_cb,
					 editor);

        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->text_left_toggle),
                                      (just == GTK_JUSTIFY_LEFT));
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->text_center_toggle),
                                      (just == GTK_JUSTIFY_CENTER));
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->priv->text_right_toggle),
                                      (just == GTK_JUSTIFY_RIGHT));

	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->text_left_toggle),
					   align_toggle_cb,
					   editor);
	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->text_center_toggle),
					   align_toggle_cb,
					   editor);
	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->text_right_toggle),
					   align_toggle_cb,
					   editor);

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Query text alignment.                                                     */
/*****************************************************************************/
GtkJustification
gl_object_editor_get_text_alignment (glObjectEditor      *editor)
{
	GtkJustification just;

	gl_debug (DEBUG_EDITOR, "START");

        if (gtk_toggle_button_get_active
            (GTK_TOGGLE_BUTTON (editor->priv->text_left_toggle))) {
                just = GTK_JUSTIFY_LEFT;
        } else
            if (gtk_toggle_button_get_active
                (GTK_TOGGLE_BUTTON (editor->priv->text_right_toggle))) {
                just = GTK_JUSTIFY_RIGHT;
        } else
            if (gtk_toggle_button_get_active
                (GTK_TOGGLE_BUTTON (editor->priv->text_center_toggle))) {
                just = GTK_JUSTIFY_CENTER;
        } else {
                just = GTK_JUSTIFY_LEFT;       /* Should not happen. */
        }

	gl_debug (DEBUG_EDITOR, "END");

	return just;
}

/*****************************************************************************/
/* Set text color.                                                           */
/*****************************************************************************/
void
gl_object_editor_set_text_color (glObjectEditor      *editor,
				 guint                text_color)
{
	GdkColor *gdk_color;

	gl_debug (DEBUG_EDITOR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->text_color_combo),
					 gl_object_editor_changed_cb,
					 editor);

        gdk_color = gl_color_to_gdk_color (text_color);
        color_combo_set_color (COLOR_COMBO(editor->priv->text_color_combo), gdk_color);
        g_free (gdk_color);

	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->text_color_combo),
					   gl_object_editor_changed_cb,
					   editor);

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Query text color.                                                         */
/*****************************************************************************/
guint
gl_object_editor_get_text_color (glObjectEditor      *editor)
{
	GdkColor  *gdk_color;
	guint     color;
	gboolean  is_default;

	gl_debug (DEBUG_EDITOR, "START");

        gdk_color = color_combo_get_color (COLOR_COMBO(editor->priv->text_color_combo),
                                           &is_default);
                                                                                
        if (is_default) {
                color = gl_prefs->default_text_color;
        } else {
                color = gl_color_from_gdk_color (gdk_color);
        }
                                                                                

	gl_debug (DEBUG_EDITOR, "END");

	return color;
}


/*****************************************************************************/
/* Set text line spacing.                                                    */
/*****************************************************************************/
void
gl_object_editor_set_text_line_spacing (glObjectEditor      *editor,
				        gdouble              text_line_spacing)
{
	gl_debug (DEBUG_EDITOR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->text_line_spacing_spin),
					 gl_object_editor_changed_cb,
					 editor);

        gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->text_line_spacing_spin),
                                   text_line_spacing);

	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->text_line_spacing_spin),
					   gl_object_editor_changed_cb,
					   editor);

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

