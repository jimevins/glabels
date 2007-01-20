/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/**
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  property-bar.c:  gLabels property bar
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

#include "ui-property-bar.h"

#include <glib/gi18n.h>
#include <glade/glade-xml.h>
#include <gtk/gtkcombobox.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtktoolbar.h>
#include <gtk/gtktoggletoolbutton.h>
#include <gtk/gtktooltips.h>
#include <string.h>

#include "util.h"
#include "mygal/widget-color-combo.h"
#include "stock-pixmaps/stockpixbufs.h"
#include "prefs.h"
#include "color.h"

#include "debug.h"

/*============================================================================*/
/* Private macros and constants.                                              */
/*============================================================================*/

/*============================================================================*/
/* Private data types                                                         */
/*============================================================================*/

struct _glUIPropertyBarPrivate {

	glView     *view;

	GladeXML   *gui;

	GtkWidget  *tool_bar;

	/* Font selection */
	GtkWidget  *font_family_combo;
	GtkWidget  *font_size_spin;
	GtkWidget  *font_bold_toggle;
	GtkWidget  *font_italic_toggle;

	/* Text alignemnt radios */
	GtkWidget  *text_align_left_radio;
	GtkWidget  *text_align_center_radio;
	GtkWidget  *text_align_right_radio;

	/* Color combos */
	GtkWidget  *text_color_combo;
	GtkWidget  *fill_color_combo;
	GtkWidget  *line_color_combo;

	/* Line width */
	GtkWidget  *line_width_spin;

	gboolean    stop_signals;

};


/*============================================================================*/
/* Private globals                                                            */
/*============================================================================*/

static GtkHBoxClass *parent_class;

/*============================================================================*/
/* Local function prototypes                                                  */
/*============================================================================*/

static void     gl_ui_property_bar_class_init    (glUIPropertyBarClass *class);
static void     gl_ui_property_bar_instance_init (glUIPropertyBar      *property_bar);
static void     gl_ui_property_bar_finalize      (GObject              *object);

static void     gl_ui_property_bar_construct     (glUIPropertyBar      *property_bar);

static void     selection_changed_cb             (glUIPropertyBar      *property_bar);

static void     font_family_changed_cb           (GtkComboBox          *combo,
						  glUIPropertyBar      *property_bar);

static void     font_size_changed_cb             (GtkSpinButton        *spin,
						  glUIPropertyBar      *property_bar);

static void     text_color_changed_cb            (ColorCombo           *cc,
						  GdkColor             *gdk_color,
						  gboolean              custom,
						  gboolean              by_user,
						  gboolean              is_default,
						  glUIPropertyBar      *property_bar);

static void     fill_color_changed_cb            (ColorCombo           *cc,
						  GdkColor             *gdk_color,
						  gboolean              custom,
						  gboolean              by_user,
						  gboolean              is_default,
						  glUIPropertyBar      *property_bar);

static void     line_color_changed_cb            (ColorCombo           *cc,
						  GdkColor             *gdk_color,
						  gboolean              custom,
						  gboolean              by_user,
						  gboolean              is_default,
						  glUIPropertyBar      *property_bar);

static void     line_width_changed_cb            (GtkSpinButton        *spin,
						  glUIPropertyBar      *property_bar);

static void     font_bold_toggled_cb             (GtkToggleToolButton  *toggle,
						  glUIPropertyBar      *property_bar);
						  
static void     font_italic_toggled_cb           (GtkToggleToolButton  *toggle,
						  glUIPropertyBar      *property_bar);
						  
static void     text_align_toggled_cb            (GtkToggleToolButton  *toggle,
						  glUIPropertyBar      *property_bar);
						  
static void     set_doc_items_sensitive          (glUIPropertyBar      *property_bar,
						  gboolean              state);

static void     set_text_items_sensitive         (glUIPropertyBar      *property_bar,
						  gboolean              state);

static void     set_fill_items_sensitive         (glUIPropertyBar      *property_bar,
						  gboolean              state);

static void     set_line_color_items_sensitive   (glUIPropertyBar      *property_bar,
						  gboolean              state);

static void     set_line_width_items_sensitive   (glUIPropertyBar      *property_bar,
						  gboolean              state);


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
GType
gl_ui_property_bar_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (glUIPropertyBarClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_ui_property_bar_class_init,
			NULL,
			NULL,
			sizeof (glUIPropertyBar),
			0,
			(GInstanceInitFunc) gl_ui_property_bar_instance_init,
			NULL
		};

		type = g_type_register_static (GTK_TYPE_HBOX,
					       "glUIPropertyBar", &info, 0);
	}

	return type;
}

static void
gl_ui_property_bar_class_init (glUIPropertyBarClass *class)
{
	GObjectClass   *object_class     = (GObjectClass *) class;

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_ui_property_bar_finalize;

	gl_debug (DEBUG_PROPERTY_BAR, "END");
}

static void
gl_ui_property_bar_instance_init (glUIPropertyBar *property_bar)
{
	gl_debug (DEBUG_PROPERTY_BAR, "START");

	property_bar->priv = g_new0 (glUIPropertyBarPrivate, 1);

	property_bar->priv->gui = glade_xml_new (GLABELS_GLADE_DIR "property-bar.glade",
						 "property_toolbar",
						 NULL);

	if (!property_bar->priv->gui) {
		g_critical ("Could not open property-bar.glade. gLabels may not be installed correctly!");
		return;
	}

	gl_debug (DEBUG_PROPERTY_BAR, "END");
}

static void
gl_ui_property_bar_finalize (GObject *object)
{
	glUIPropertyBar *property_bar;

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_UI_PROPERTY_BAR (object));

	property_bar = GL_UI_PROPERTY_BAR (object);

	if (property_bar->priv->view) {
		g_object_unref (G_OBJECT(property_bar->priv->view));
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);

	g_free (property_bar->priv);

	gl_debug (DEBUG_PROPERTY_BAR, "END");
}

/****************************************************************************/
/* Create a NEW property_bar.                                               */
/****************************************************************************/
GtkWidget *
gl_ui_property_bar_new (void)
{
	glUIPropertyBar *property_bar;

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	property_bar = g_object_new (GL_TYPE_UI_PROPERTY_BAR, NULL);

	gl_ui_property_bar_construct (property_bar);

	gl_debug (DEBUG_PROPERTY_BAR, "END");

	return GTK_WIDGET (property_bar);
}

/******************************************************************************/
/* Initialize property toolbar.                                               */
/******************************************************************************/
static void
gl_ui_property_bar_construct (glUIPropertyBar   *property_bar)
{
	GList      *family_names = NULL;
	GList      *family_node;
	GdkColor   *gdk_color;

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	property_bar->priv->stop_signals = TRUE;

	property_bar->priv->tool_bar = glade_xml_get_widget (property_bar->priv->gui,
							     "property_toolbar");
	gtk_container_add (GTK_CONTAINER (property_bar), property_bar->priv->tool_bar);

	property_bar->priv->font_family_combo =
		glade_xml_get_widget (property_bar->priv->gui, "font_family_combo");
	property_bar->priv->font_size_spin =
		glade_xml_get_widget (property_bar->priv->gui, "font_size_spin");
	property_bar->priv->font_bold_toggle =
		glade_xml_get_widget (property_bar->priv->gui, "font_bold_toggle");
	property_bar->priv->font_italic_toggle =
		glade_xml_get_widget (property_bar->priv->gui, "font_italic_toggle");
	property_bar->priv->text_align_left_radio =
		glade_xml_get_widget (property_bar->priv->gui, "text_align_left_radio");
	property_bar->priv->text_align_center_radio =
		glade_xml_get_widget (property_bar->priv->gui, "text_align_center_radio");
	property_bar->priv->text_align_right_radio =
		glade_xml_get_widget (property_bar->priv->gui, "text_align_right_radio");
	property_bar->priv->text_color_combo =
		glade_xml_get_widget (property_bar->priv->gui, "text_color_combo");
	property_bar->priv->fill_color_combo =
		glade_xml_get_widget (property_bar->priv->gui, "fill_color_combo");
	property_bar->priv->line_color_combo =
		glade_xml_get_widget (property_bar->priv->gui, "line_color_combo");
	property_bar->priv->line_width_spin =
		glade_xml_get_widget (property_bar->priv->gui, "line_width_spin");

	set_doc_items_sensitive (property_bar, FALSE);

	/* Font family entry widget */
	gl_util_combo_box_add_text_model (GTK_COMBO_BOX (property_bar->priv->font_family_combo));
	family_names = gl_util_get_font_family_list ();
	gl_util_combo_box_set_strings (GTK_COMBO_BOX (property_bar->priv->font_family_combo),
				       family_names);
	gtk_widget_set_size_request (property_bar->priv->font_family_combo, 200, -1);

	/* Make sure we have a valid font.  if not provide a good default. */
	family_node = g_list_find_custom (family_names,
					  gl_prefs->default_font_family,
					  (GCompareFunc)g_utf8_collate);
	if (family_node) {
		gtk_combo_box_set_active (GTK_COMBO_BOX (property_bar->priv->font_family_combo),
					  g_list_position (family_names,
							   family_node));
	} else {
		gtk_combo_box_set_active (GTK_COMBO_BOX (property_bar->priv->font_family_combo), 0);
	}
	gl_util_font_family_list_free (family_names);

	g_signal_connect (G_OBJECT (property_bar->priv->font_family_combo),
			  "changed", G_CALLBACK (font_family_changed_cb), property_bar);

	/* Font size entry widget */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(property_bar->priv->font_size_spin),
				   gl_prefs->default_font_size);

	g_signal_connect (G_OBJECT (property_bar->priv->font_size_spin),
			  "changed", G_CALLBACK (font_size_changed_cb), property_bar);


	/* Bold and Italic toggles */
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (property_bar->priv->font_bold_toggle),
					   (gl_prefs->default_font_weight == PANGO_WEIGHT_BOLD));
	g_signal_connect (G_OBJECT (property_bar->priv->font_bold_toggle),
			  "toggled", G_CALLBACK (font_bold_toggled_cb), property_bar);
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (property_bar->priv->font_italic_toggle),
					   gl_prefs->default_font_italic_flag);
	g_signal_connect (G_OBJECT (property_bar->priv->font_italic_toggle),
			  "toggled", G_CALLBACK (font_italic_toggled_cb), property_bar);


	/* Text alignment radio group */
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (property_bar->priv->text_align_left_radio),
					   (gl_prefs->default_text_alignment == PANGO_ALIGN_LEFT));
	g_signal_connect (G_OBJECT (property_bar->priv->text_align_left_radio),
			  "toggled", G_CALLBACK (text_align_toggled_cb), property_bar);
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (property_bar->priv->text_align_center_radio),
					   (gl_prefs->default_text_alignment == PANGO_ALIGN_CENTER));
	g_signal_connect (G_OBJECT (property_bar->priv->text_align_center_radio),
			  "toggled", G_CALLBACK (text_align_toggled_cb), property_bar);
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (property_bar->priv->text_align_right_radio),
					   (gl_prefs->default_text_alignment == PANGO_ALIGN_RIGHT));
	g_signal_connect (G_OBJECT (property_bar->priv->text_align_right_radio),
			  "toggled", G_CALLBACK (text_align_toggled_cb), property_bar);

	/* Text color widget */
	gdk_color = gl_color_to_gdk_color (gl_prefs->default_text_color);
	color_combo_set_color (COLOR_COMBO (property_bar->priv->text_color_combo), gdk_color);
	g_free (gdk_color);
	g_signal_connect (G_OBJECT (property_bar->priv->text_color_combo),
			  "color_changed",
			  G_CALLBACK (text_color_changed_cb), property_bar);

	/* Fill color widget */
	gdk_color = gl_color_to_gdk_color (gl_prefs->default_fill_color);
	color_combo_set_color (COLOR_COMBO (property_bar->priv->fill_color_combo), gdk_color);
	g_free (gdk_color);
	g_signal_connect (G_OBJECT (property_bar->priv->fill_color_combo),
			  "color_changed",
			  G_CALLBACK (fill_color_changed_cb), property_bar);

	/* Line color widget */
	gdk_color = gl_color_to_gdk_color (gl_prefs->default_line_color);
	color_combo_set_color (COLOR_COMBO (property_bar->priv->line_color_combo), gdk_color);
	g_free (gdk_color);
	g_signal_connect (G_OBJECT (property_bar->priv->line_color_combo),
			  "color_changed",
			  G_CALLBACK (line_color_changed_cb), property_bar);

	/* Line width entry widget */
	g_signal_connect (G_OBJECT (property_bar->priv->line_width_spin),
			  "changed",
			  G_CALLBACK (line_width_changed_cb), property_bar);

	property_bar->priv->stop_signals = FALSE;

	gl_debug (DEBUG_PROPERTY_BAR, "END");
}

/****************************************************************************/
/* Fill widgets with default values.                                        */
/****************************************************************************/
static void
reset_to_default_properties (glView *view,
			     glUIPropertyBar *property_bar)
{
	GList     *family_names;
	gchar     *good_font_family;
	GdkColor  *gdk_color;

	/* Make sure we have a valid font.  if not provide a good default. */
	family_names = gl_util_get_font_family_list ();
	if (g_list_find_custom (family_names,
				view->default_font_family,
				(GCompareFunc)g_utf8_collate)) {
		good_font_family = g_strdup (view->default_font_family);
	} else {
		if (family_names != NULL) {
			good_font_family = g_strdup (family_names->data); /* 1st entry */
		} else {
			good_font_family = NULL;
		}
	}
	gl_util_combo_box_set_active_text (GTK_COMBO_BOX (property_bar->priv->font_family_combo),
					   good_font_family);
	g_free (good_font_family);
	gl_util_font_family_list_free (family_names);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON(property_bar->priv->font_size_spin),
				   view->default_font_size);

	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (property_bar->priv->font_bold_toggle),
					   (view->default_font_weight == PANGO_WEIGHT_BOLD));
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (property_bar->priv->font_italic_toggle),
					   view->default_font_italic_flag);

	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (property_bar->priv->text_align_left_radio),
					   (view->default_text_alignment == PANGO_ALIGN_LEFT));
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (property_bar->priv->text_align_center_radio),
					   (view->default_text_alignment == PANGO_ALIGN_CENTER));
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (property_bar->priv->text_align_right_radio),
					   (view->default_text_alignment == PANGO_ALIGN_RIGHT));

	gdk_color = gl_color_to_gdk_color (view->default_text_color);
	color_combo_set_color (COLOR_COMBO(property_bar->priv->text_color_combo), gdk_color);
	g_free (gdk_color);

	gdk_color = gl_color_to_gdk_color (view->default_fill_color);
	color_combo_set_color (COLOR_COMBO(property_bar->priv->fill_color_combo), gdk_color);
	g_free (gdk_color);

	gdk_color = gl_color_to_gdk_color (view->default_line_color);
	color_combo_set_color (COLOR_COMBO(property_bar->priv->line_color_combo), gdk_color);
	g_free (gdk_color);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON(property_bar->priv->line_width_spin),
				   view->default_line_width);
}

/****************************************************************************/
/* Set view associated with property_bar.                                   */
/****************************************************************************/
void
gl_ui_property_bar_set_view (glUIPropertyBar *property_bar,
			     glView          *view)
{
	glLabel   *label;

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));
	label = view->label;
	g_return_if_fail (label && GL_IS_LABEL (label));

	set_doc_items_sensitive (property_bar, TRUE);

	property_bar->priv->view = GL_VIEW (g_object_ref (G_OBJECT (view)));

	reset_to_default_properties (view, property_bar);

	g_signal_connect_swapped (G_OBJECT(view), "selection_changed",
				  G_CALLBACK(selection_changed_cb), property_bar);

	g_signal_connect_swapped (G_OBJECT(view->label), "changed",
				  G_CALLBACK(selection_changed_cb), property_bar);

	gl_debug (DEBUG_PROPERTY_BAR, "END");
}

/****************************************************************************/
/** Set visiblity of property bar's tooltips.                               */
/****************************************************************************/
void
gl_ui_property_bar_set_tooltips (glUIPropertyBar *property_bar,
				 gboolean         state)
{
	GtkTooltipsData *data;

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	g_return_if_fail (property_bar && GL_IS_UI_PROPERTY_BAR(property_bar));

	/* HACK: peek into one of our widgets to get the tooltips group created by libglade. */
	data = gtk_tooltips_data_get (property_bar->priv->font_size_spin);
	g_return_if_fail (data);

	if (state) {
		gtk_tooltips_enable (data->tooltips);
	} else {
		gtk_tooltips_disable (data->tooltips);
	}

	gl_debug (DEBUG_PROPERTY_BAR, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  View "selection state changed" callback.                        */
/*---------------------------------------------------------------------------*/
static void
update_text_properties (glView *view,
			glUIPropertyBar *property_bar)
{
	gboolean        can_text, is_first_object;
	gboolean        is_same_font_family, is_same_font_size;
	gboolean        is_same_text_color, is_same_is_italic;
	gboolean        is_same_is_bold, is_same_align;
	GList          *p;
	glLabelObject  *object;
	gchar          *selection_font_family, *font_family;
	gdouble         selection_font_size, font_size;
	guint           selection_text_color, text_color;
	glColorNode    *text_color_node;
	gboolean        selection_is_italic, is_italic;
	gboolean        selection_is_bold, is_bold;
	PangoAlignment  selection_align, align;
	GdkColor       *gdk_color;

	can_text = gl_view_can_selection_text (view);
	set_text_items_sensitive (property_bar, can_text);

	if (!can_text) 
		return;

	is_same_is_italic =
	is_same_is_bold =
	is_same_align =
	is_same_text_color =
	is_same_font_size =
	is_same_font_family = TRUE;
	selection_font_family = NULL;
	selection_font_size = -1;
	is_first_object = TRUE;
	
	for (p = view->selected_object_list; p != NULL; p = p->next) {

		object = gl_view_object_get_object(GL_VIEW_OBJECT (p->data));
		if (!gl_label_object_can_text (object)) 
			continue;

		font_family = gl_label_object_get_font_family (object);
		if (font_family != NULL) {
			if (selection_font_family == NULL)
				selection_font_family = g_strdup (font_family);
			else 
				if (strcmp (font_family, selection_font_family) != 0) 
					is_same_font_family = FALSE;
			g_free (font_family);
		}	

		font_size = gl_label_object_get_font_size (object);
		
		text_color_node = gl_label_object_get_text_color (object);
		if (text_color_node->field_flag) {
			/* If a merge field is set we use the default color for merged color*/
			text_color = GL_COLOR_MERGE_DEFAULT;
			
		} else {
			text_color = text_color_node->color;
		}
		gl_color_node_free (&text_color_node);
		
		is_italic = gl_label_object_get_font_italic_flag (object);
		is_bold = gl_label_object_get_font_weight (object) == PANGO_WEIGHT_BOLD;
		align = gl_label_object_get_text_alignment (object);

		if (is_first_object) {
			selection_font_size = font_size;
			selection_text_color = text_color;
			selection_is_italic = is_italic;
			selection_is_bold = is_bold;
			selection_align = align;
		} else {
			if (font_size != selection_font_size) 
				is_same_font_size = FALSE;
			if (text_color != selection_text_color)
				is_same_text_color = FALSE;
			if (is_italic != selection_is_italic)
				is_same_is_italic = FALSE;
			if (is_bold != selection_is_bold)
				is_same_is_bold = FALSE;
			if (align != selection_align)
				is_same_align = FALSE;
		}
		is_first_object = FALSE;
	}

	if (is_same_font_family && (selection_font_family != NULL)) 
		gl_debug (DEBUG_PROPERTY_BAR, "same font family = %s", 
			  selection_font_family);
	gl_util_combo_box_set_active_text (GTK_COMBO_BOX (property_bar->priv->font_family_combo),
					   is_same_font_family?selection_font_family:"");
	g_free (selection_font_family);

	if (is_same_font_size) {
		gl_debug (DEBUG_PROPERTY_BAR, "same font size = %g", 
			  selection_font_size);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (property_bar->priv->font_size_spin),
					   selection_font_size);
	} else {
		gtk_entry_set_text (GTK_ENTRY (property_bar->priv->font_size_spin), "");
	}

	if (is_same_text_color) {
		gl_debug (DEBUG_PROPERTY_BAR, "same text color = %08x", selection_text_color);
		gdk_color = gl_color_to_gdk_color (selection_text_color);
		color_combo_set_color (COLOR_COMBO (property_bar->priv->text_color_combo),
				       gdk_color);
		g_free (gdk_color);
	}

	if (is_same_is_italic)  
		gl_debug (DEBUG_PROPERTY_BAR, "same italic flag = %d", 
			  selection_is_italic);
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (property_bar->priv->font_italic_toggle),
					   selection_is_italic && is_same_is_italic);

	if (is_same_is_bold)  
		gl_debug (DEBUG_PROPERTY_BAR, "same bold flag = %d",
			  selection_is_bold);
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (property_bar->priv->font_bold_toggle),
					   selection_is_bold && is_same_is_bold);

	if (is_same_align) 
		gl_debug (DEBUG_PROPERTY_BAR, "same align");
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (property_bar->priv->text_align_left_radio),
					   (selection_align == PANGO_ALIGN_LEFT) &&
					   is_same_align);
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (property_bar->priv->text_align_center_radio),
					   (selection_align == PANGO_ALIGN_CENTER) &&
					   is_same_align);
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (property_bar->priv->text_align_right_radio),
					   (selection_align == PANGO_ALIGN_RIGHT) &&
					   is_same_align);
}

static void
update_fill_color (glView *view,
		   glUIPropertyBar *property_bar)
{
	gboolean can, is_first_object;
	gboolean is_same_fill_color;
	GList *p;
	glLabelObject *object;
	guint selection_fill_color, fill_color;
	GdkColor *gdk_color;
	glColorNode *fill_color_node;

	can = gl_view_can_selection_fill (view);
	set_fill_items_sensitive (property_bar, can);

	if (!can) 
		return;

	is_same_fill_color = TRUE;
	is_first_object = TRUE;
	
	for (p = view->selected_object_list; p != NULL; p = p->next) {

		object = gl_view_object_get_object(GL_VIEW_OBJECT (p->data));
		if (!gl_label_object_can_fill (object)) 
			continue;

		fill_color_node = gl_label_object_get_fill_color (object);
		if (fill_color_node->field_flag) {
			/* If a merge field is set we use the default color for merged color*/
			fill_color = GL_COLOR_FILL_MERGE_DEFAULT;
			
		} else {
			fill_color = fill_color_node->color;
		}
		gl_color_node_free (&fill_color_node);

		if (is_first_object) {
			selection_fill_color = fill_color;
		} else {
			if (fill_color != selection_fill_color)
				is_same_fill_color = FALSE;
		}
		is_first_object = FALSE;
	}

	if (is_same_fill_color) {
		gl_debug (DEBUG_PROPERTY_BAR, "same fill color = %08x", selection_fill_color);
		gdk_color = gl_color_to_gdk_color (selection_fill_color);
		color_combo_set_color (COLOR_COMBO (property_bar->priv->fill_color_combo),
				       gdk_color);
		g_free (gdk_color);
	}
}

static void
update_line_color (glView *view,
		   glUIPropertyBar *property_bar)
{
	gboolean can, is_first_object;
	gboolean is_same_line_color;
	GList *p;
	glLabelObject *object;
	guint selection_line_color, line_color;
	glColorNode *line_color_node;
	GdkColor *gdk_color;

	can = gl_view_can_selection_line_color (view);
	set_line_color_items_sensitive (property_bar, can);

	if (!can) 
		return;

	is_same_line_color = TRUE;
	is_first_object = TRUE;
	
	for (p = view->selected_object_list; p != NULL; p = p->next) {

		object = gl_view_object_get_object(GL_VIEW_OBJECT (p->data));
		if (!gl_label_object_can_line_color (object)) 
			continue;

		line_color_node = gl_label_object_get_line_color (object);
		if (line_color_node->field_flag) {
			/* If a merge field is set we use the default color for merged color*/
			line_color = GL_COLOR_MERGE_DEFAULT;
			
		} else {
			line_color = line_color_node->color;
		}
		gl_color_node_free (&line_color_node);

		if (is_first_object) {
			selection_line_color = line_color;
		} else {
			if (line_color != selection_line_color)
				is_same_line_color = FALSE;
		}
		is_first_object = FALSE;
	}

	if (is_same_line_color) {
		gl_debug (DEBUG_PROPERTY_BAR, "same line color = %08x", selection_line_color);
		gdk_color = gl_color_to_gdk_color (selection_line_color);
		color_combo_set_color (COLOR_COMBO (property_bar->priv->line_color_combo),
				       gdk_color);
		g_free (gdk_color);
	}
}

static void
update_line_width (glView *view,
		   glUIPropertyBar *property_bar)
{
	gboolean can, is_first_object;
	gboolean is_same_line_width;
	GList *p;
	glLabelObject *object;
	gdouble selection_line_width, line_width;

	can = gl_view_can_selection_line_width (view);
	set_line_width_items_sensitive (property_bar, can);

	if (!can) 
		return;

	is_same_line_width = TRUE;
	is_first_object = TRUE;
	
	for (p = view->selected_object_list; p != NULL; p = p->next) {

		object = gl_view_object_get_object(GL_VIEW_OBJECT (p->data));
		if (!gl_label_object_can_line_width (object)) 
			continue;

		line_width = gl_label_object_get_line_width (object);

		if (is_first_object) {
			selection_line_width = line_width;
		} else {
			if (line_width != selection_line_width)
				is_same_line_width = FALSE;
		}
		is_first_object = FALSE;
	}

	if (is_same_line_width) {
		gl_debug (DEBUG_PROPERTY_BAR, "same line width = %g", selection_line_width);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (property_bar->priv->line_width_spin),
					   selection_line_width);
	} else {
		gtk_entry_set_text (GTK_ENTRY (property_bar->priv->line_width_spin), "");
	}
}

static void 
selection_changed_cb (glUIPropertyBar *property_bar)
{
	glView *view = property_bar->priv->view;
	
	gl_debug (DEBUG_PROPERTY_BAR, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (property_bar && GL_IS_UI_PROPERTY_BAR (property_bar));

	property_bar->priv->stop_signals = TRUE;

	if (gl_view_is_selection_empty (view)) {

		/* No selection: make all controls active. */
		reset_to_default_properties (view, property_bar);
		set_doc_items_sensitive (property_bar, TRUE);

	} else {

		update_text_properties (view, property_bar);
		update_fill_color (view, property_bar);
		update_line_color (view, property_bar);
		update_line_width (view, property_bar);

	}

	property_bar->priv->stop_signals = FALSE;

	gl_debug (DEBUG_PROPERTY_BAR, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Font family entry changed.                                     */
/*--------------------------------------------------------------------------*/
static void
font_family_changed_cb (GtkComboBox     *combo,
			glUIPropertyBar *property_bar)
{
	gchar *font_family;

	if (property_bar->priv->stop_signals)
		return;

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(property_bar->priv->view->label),
					 selection_changed_cb,
					 property_bar);

	font_family = gtk_combo_box_get_active_text (GTK_COMBO_BOX (combo));
	if ( strlen(font_family) ) {
		gl_view_set_selection_font_family (property_bar->priv->view,
						   font_family);
		gl_view_set_default_font_family   (property_bar->priv->view,
						   font_family);
	}
	g_free (font_family);

	g_signal_handlers_unblock_by_func (G_OBJECT(property_bar->priv->view->label),
					   selection_changed_cb,
					   property_bar);

	gl_debug (DEBUG_PROPERTY_BAR, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Font size spin button changed.                                 */
/*--------------------------------------------------------------------------*/
static void
font_size_changed_cb (GtkSpinButton        *spin,
		      glUIPropertyBar      *property_bar)
{
	gdouble font_size;

	if (property_bar->priv->stop_signals)
		return;

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(property_bar->priv->view->label),
					 selection_changed_cb,
					 property_bar);

	font_size = gtk_spin_button_get_value (spin);

	gl_view_set_selection_font_size (property_bar->priv->view,
					 font_size);
	gl_view_set_default_font_size   (property_bar->priv->view,
					 font_size);

	g_signal_handlers_unblock_by_func (G_OBJECT(property_bar->priv->view->label),
					   selection_changed_cb,
					   property_bar);

	gl_debug (DEBUG_PROPERTY_BAR, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Text color combo changed.                                      */
/*--------------------------------------------------------------------------*/
static void
text_color_changed_cb (ColorCombo           *cc,
		       GdkColor             *gdk_color,
		       gboolean              custom,
		       gboolean              by_user,
		       gboolean              is_default,
		       glUIPropertyBar      *property_bar)
{
	glColorNode *text_color_node;

	if (property_bar->priv->stop_signals)
		return;

	g_return_if_fail (property_bar && GL_IS_UI_PROPERTY_BAR (property_bar));

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(property_bar->priv->view->label),
					 selection_changed_cb,
					 property_bar);

	text_color_node = gl_color_node_new_default ();
	text_color_node->color = gl_color_from_gdk_color (gdk_color);
	
	gl_debug (DEBUG_PROPERTY_BAR, "Color=%08x, Custom=%d, By_User=%d, Is_default=%d",
		  text_color_node->color, custom, by_user, is_default);

	if (is_default) {
		text_color_node->color = gl_prefs->default_text_color;
		gl_view_set_selection_text_color (property_bar->priv->view,
						  text_color_node);
		gl_view_set_default_text_color   (property_bar->priv->view,
						  gl_prefs->default_text_color);

	} else {

		gl_view_set_selection_text_color (property_bar->priv->view,
						  text_color_node);
		gl_view_set_default_text_color   (property_bar->priv->view,
						  text_color_node->color);

	}

	gl_color_node_free (&text_color_node);
	
	g_signal_handlers_unblock_by_func (G_OBJECT(property_bar->priv->view->label),
					   selection_changed_cb,
					   property_bar);

	gl_debug (DEBUG_PROPERTY_BAR, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Fill color combo changed.                                      */
/*--------------------------------------------------------------------------*/
static void
fill_color_changed_cb (ColorCombo           *cc,
		       GdkColor             *gdk_color,
		       gboolean              custom,
		       gboolean              by_user,
		       gboolean              is_default,
		       glUIPropertyBar      *property_bar)
{
	glColorNode *fill_color_node;

	if (property_bar->priv->stop_signals)
		return;

	g_return_if_fail (property_bar && GL_IS_UI_PROPERTY_BAR (property_bar));

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(property_bar->priv->view->label),
					 selection_changed_cb,
					 property_bar);

	fill_color_node = gl_color_node_new_default ();

	fill_color_node->color = gl_color_from_gdk_color (gdk_color);

	gl_debug (DEBUG_PROPERTY_BAR, "Color=%08x, Custom=%d, By_User=%d, Is_default=%d",
		  fill_color_node->color, custom, by_user, is_default);

	if (is_default) {

		fill_color_node->color = GL_COLOR_NONE;
		gl_view_set_selection_fill_color (property_bar->priv->view,
						  fill_color_node);
		gl_view_set_default_fill_color   (property_bar->priv->view,
						  fill_color_node->color);

	} else {

		gl_view_set_selection_fill_color (property_bar->priv->view,
						  fill_color_node);
		gl_view_set_default_fill_color   (property_bar->priv->view,
						  fill_color_node->color);

	}
	gl_color_node_free (&fill_color_node);
	
	g_signal_handlers_unblock_by_func (G_OBJECT(property_bar->priv->view->label),
					   selection_changed_cb,
					   property_bar);

	gl_debug (DEBUG_PROPERTY_BAR, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Line color combo changed.                                      */
/*--------------------------------------------------------------------------*/
static void
line_color_changed_cb (ColorCombo           *cc,
		       GdkColor             *gdk_color,
		       gboolean              custom,
		       gboolean              by_user,
		       gboolean              is_default,
		       glUIPropertyBar      *property_bar)
{
	glColorNode *line_color_node;

	if (property_bar->priv->stop_signals)
		return;

	g_return_if_fail (property_bar && GL_IS_UI_PROPERTY_BAR (property_bar));

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(property_bar->priv->view->label),
					 selection_changed_cb,
					 property_bar);

	line_color_node = gl_color_node_new_default ();
	line_color_node->color = gl_color_from_gdk_color (gdk_color);

	gl_debug (DEBUG_PROPERTY_BAR, "Color=%08x, Custom=%d, By_User=%d, Is_default=%d",
		  line_color_node->color, custom, by_user, is_default);

	if (is_default) {
		line_color_node->color = GL_COLOR_NONE;
		gl_view_set_selection_line_color (property_bar->priv->view,
						  line_color_node);
		gl_view_set_default_line_color   (property_bar->priv->view,
						  line_color_node->color);

	} else {

		gl_view_set_selection_line_color (property_bar->priv->view,
						  line_color_node);
		gl_view_set_default_line_color   (property_bar->priv->view,
						  line_color_node->color);

	}
	gl_color_node_free (&line_color_node);

	g_signal_handlers_unblock_by_func (G_OBJECT(property_bar->priv->view->label),
					   selection_changed_cb,
					   property_bar);

	gl_debug (DEBUG_PROPERTY_BAR, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Line width spin button changed.                                */
/*--------------------------------------------------------------------------*/
static void
line_width_changed_cb (GtkSpinButton        *spin,
		       glUIPropertyBar      *property_bar)
{
	gdouble line_width;

	if (property_bar->priv->stop_signals)
		return;

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	if (property_bar->priv->view) {

		g_signal_handlers_block_by_func (G_OBJECT(property_bar->priv->view->label),
						 selection_changed_cb,
						 property_bar);

		line_width = gtk_spin_button_get_value (spin);

		gl_view_set_selection_line_width (property_bar->priv->view,
						  line_width);
		gl_view_set_default_line_width   (property_bar->priv->view,
						  line_width);

		g_signal_handlers_unblock_by_func (G_OBJECT(property_bar->priv->view->label),
						   selection_changed_cb,
						   property_bar);

	}

	gl_debug (DEBUG_PROPERTY_BAR, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Font bold toggled callback.                                     */
/*---------------------------------------------------------------------------*/
static void
font_bold_toggled_cb (GtkToggleToolButton  *toggle,
		      glUIPropertyBar      *property_bar)
{
	gboolean        state;
	PangoWeight     weight;


	if (property_bar->priv->stop_signals)
		return;

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(property_bar->priv->view->label),
					 selection_changed_cb,
					 property_bar);

	state = gtk_toggle_tool_button_get_active (toggle);

	weight = state ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL;

	gl_view_set_selection_font_weight (property_bar->priv->view, weight);
	gl_view_set_default_font_weight   (property_bar->priv->view, weight);

	g_signal_handlers_unblock_by_func (G_OBJECT(property_bar->priv->view->label),
					   selection_changed_cb,
					   property_bar);

	gl_debug (DEBUG_PROPERTY_BAR, "END");
}
						  
/*---------------------------------------------------------------------------*/
/* PRIVATE.  Font italic toggled callback.                                   */
/*---------------------------------------------------------------------------*/
static void
font_italic_toggled_cb (GtkToggleToolButton  *toggle,
			glUIPropertyBar      *property_bar)
{
	gboolean state;

	if (property_bar->priv->stop_signals)
		return;

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(property_bar->priv->view->label),
					 selection_changed_cb,
					 property_bar);

	state = gtk_toggle_tool_button_get_active (toggle);

	gl_view_set_selection_font_italic_flag (property_bar->priv->view, state);
	gl_view_set_default_font_italic_flag   (property_bar->priv->view, state);

	g_signal_handlers_unblock_by_func (G_OBJECT(property_bar->priv->view->label),
					   selection_changed_cb,
					   property_bar);

	gl_debug (DEBUG_PROPERTY_BAR, "END");
}
						  
/*---------------------------------------------------------------------------*/
/* PRIVATE.  Text align toggled callback.                                    */
/*---------------------------------------------------------------------------*/
static void
text_align_toggled_cb (GtkToggleToolButton  *toggle,
		       glUIPropertyBar      *property_bar)
{
	if (property_bar->priv->stop_signals)
		return;

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(property_bar->priv->view->label),
					 selection_changed_cb,
					 property_bar);

	if (gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (property_bar->priv->text_align_left_radio)))
	{		
		gl_view_set_selection_text_alignment (property_bar->priv->view,
						      PANGO_ALIGN_LEFT);
		gl_view_set_default_text_alignment   (property_bar->priv->view,
						      PANGO_ALIGN_LEFT);
	}

	if (gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (property_bar->priv->text_align_center_radio)))
	{		
		gl_view_set_selection_text_alignment (property_bar->priv->view,
						      PANGO_ALIGN_CENTER);
		gl_view_set_default_text_alignment   (property_bar->priv->view,
						      PANGO_ALIGN_CENTER);
	}

	if (gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (property_bar->priv->text_align_right_radio)))
	{
		gl_view_set_selection_text_alignment (property_bar->priv->view,
						      PANGO_ALIGN_RIGHT);
		gl_view_set_default_text_alignment   (property_bar->priv->view,
						      PANGO_ALIGN_RIGHT);
	}

	g_signal_handlers_unblock_by_func (G_OBJECT(property_bar->priv->view->label),
					   selection_changed_cb,
					   property_bar);

	gl_debug (DEBUG_PROPERTY_BAR, "END");
}


/*****************************************************************************/
/* Construct color combo "Custom widget".                                    */
/*****************************************************************************/
GtkWidget *
gl_ui_property_bar_construct_color_combo (gchar *name,
					  gchar *string1,
					  gchar *string2,
					  gint   int1,
					  gint   int2)
{
	GtkWidget  *color_combo;
	ColorGroup *cg;
	gchar      *cg_name;
	guint       color;
	GdkColor   *gdk_color;
	gchar      *no_color;
	GdkPixbuf  *pixbuf = NULL;

	switch (int1) {

	case 0:
		cg_name  = "text_color_group";
		color    = gl_prefs->default_text_color;
		no_color = _("Default");
		pixbuf = gdk_pixbuf_new_from_inline (-1, stock_text_24, FALSE, NULL);
		break;

	case 2:
		cg_name  = "line_color_group";
		color    = gl_prefs->default_line_color;
		no_color = _("No line");
		pixbuf = gdk_pixbuf_new_from_inline (-1, stock_pencil_24, FALSE, NULL);
		break;

	case 1:
	default:
		cg_name  = "fill_color_group";
		color    = gl_prefs->default_fill_color;
		no_color = _("No fill");
		pixbuf = gdk_pixbuf_new_from_inline (-1, stock_bucket_fill_24, FALSE, NULL);
		break;

	}

	cg = color_group_fetch (cg_name, NULL);
        gdk_color = gl_color_to_gdk_color (color);
	color_combo = color_combo_new (pixbuf, no_color, gdk_color, cg);
        g_free (gdk_color);

	color_combo_box_set_preview_relief (COLOR_COMBO(color_combo), GTK_RELIEF_NORMAL);

	g_object_unref (G_OBJECT (pixbuf));

	return color_combo;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set sensitivity of doc controls.                                */
/*---------------------------------------------------------------------------*/
static void
set_doc_items_sensitive (glUIPropertyBar      *property_bar,
			 gboolean              state)
{
	gtk_widget_set_sensitive (property_bar->priv->font_family_combo,       state);
	gtk_widget_set_sensitive (property_bar->priv->font_size_spin,          state);
	gtk_widget_set_sensitive (property_bar->priv->font_bold_toggle,        state);
	gtk_widget_set_sensitive (property_bar->priv->font_italic_toggle,      state);
	gtk_widget_set_sensitive (property_bar->priv->text_align_left_radio,   state);
	gtk_widget_set_sensitive (property_bar->priv->text_align_center_radio, state);
	gtk_widget_set_sensitive (property_bar->priv->text_align_right_radio,  state);
	gtk_widget_set_sensitive (property_bar->priv->text_color_combo,        state);
	gtk_widget_set_sensitive (property_bar->priv->fill_color_combo,        state);
	gtk_widget_set_sensitive (property_bar->priv->line_color_combo,        state);
	gtk_widget_set_sensitive (property_bar->priv->line_width_spin,         state);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set sensitivity of text related controls.                       */
/*---------------------------------------------------------------------------*/
static void
set_text_items_sensitive (glUIPropertyBar      *property_bar,
			  gboolean              state)
{
	gtk_widget_set_sensitive (property_bar->priv->font_family_combo,       state);
	gtk_widget_set_sensitive (property_bar->priv->font_size_spin,          state);
	gtk_widget_set_sensitive (property_bar->priv->font_bold_toggle,        state);
	gtk_widget_set_sensitive (property_bar->priv->font_italic_toggle,      state);
	gtk_widget_set_sensitive (property_bar->priv->text_align_left_radio,   state);
	gtk_widget_set_sensitive (property_bar->priv->text_align_center_radio, state);
	gtk_widget_set_sensitive (property_bar->priv->text_align_right_radio,  state);
	gtk_widget_set_sensitive (property_bar->priv->text_color_combo,        state);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set sensitivity of fill related controls.                       */
/*---------------------------------------------------------------------------*/
static void
set_fill_items_sensitive (glUIPropertyBar      *property_bar,
			  gboolean              state)
{
	gtk_widget_set_sensitive (property_bar->priv->fill_color_combo,        state);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set sensitivity of line color related controls.                 */
/*---------------------------------------------------------------------------*/
static void
set_line_color_items_sensitive (glUIPropertyBar      *property_bar,
				gboolean              state)
{
	gtk_widget_set_sensitive (property_bar->priv->line_color_combo,        state);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set sensitivity of line width related controls.                 */
/*---------------------------------------------------------------------------*/
static void
set_line_width_items_sensitive (glUIPropertyBar      *property_bar,
				gboolean              state)
{
	gtk_widget_set_sensitive (property_bar->priv->line_width_spin,         state);
}


