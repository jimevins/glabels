/*
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

#include <glib/gi18n.h>
#include <libgnomeprint/gnome-font.h>

#include "ui-property-bar.h"
#include "ui-util.h"
#include "mygal/widget-color-combo.h"
#include "stock-pixmaps/stockpixbufs.h"
#include "prefs.h"
#include "color.h"

#include "debug.h"

/*============================================================================*/
/* Private macros and constants.                                              */
/*============================================================================*/

/*============================================================================*/
/* Private globals                                                            */
/*============================================================================*/

static GObjectClass *parent_class;

static gchar* doc_verbs [] = {
	"/commands/PropFontName",
	"/commands/PropFontSize",
	"/commands/PropFontBold",
	"/commands/PropFontItalic",
	"/commands/PropTextAlignLeft",
	"/commands/PropTextAlignCenter",
	"/commands/PropTextAlignRight",
	"/commands/PropFillColor",
	"/commands/PropLineColor",
	"/commands/PropTextColor",
	"/commands/PropLineWidth",

	NULL
};

static gchar* text_verbs [] = {
	"/commands/PropFontName",
	"/commands/PropFontSize",
	"/commands/PropFontBold",
	"/commands/PropFontItalic",
	"/commands/PropTextAlignLeft",
	"/commands/PropTextAlignCenter",
	"/commands/PropTextAlignRight",
	"/commands/PropTextColor",

	NULL
};

static gchar* fill_verbs [] = {
	"/commands/PropFillColor",

	NULL
};

static gchar* line_color_verbs [] = {
	"/commands/PropLineColor",

	NULL
};

static gchar* line_width_verbs [] = {
	"/commands/PropLineWidth",

	NULL
};


/*============================================================================*/
/* Local function prototypes                                                  */
/*============================================================================*/

static void     gl_ui_property_bar_class_init    (glUIPropertyBarClass *class);
static void     gl_ui_property_bar_instance_init (glUIPropertyBar      *property_bar);
static void     gl_ui_property_bar_finalize      (GObject              *object);

static void     gl_ui_property_bar_construct     (glUIPropertyBar      *property_bar,
						  BonoboUIComponent    *ui_component);

static void     selection_changed_cb             (glUIPropertyBar      *property_bar);

static void     font_family_changed_cb           (GtkEntry             *entry,
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

static void prop_item_toggled_cb (BonoboUIComponent           *ui_component,
				  const char                  *path,
				  Bonobo_UIComponent_EventType type,
				  const char                  *state,
				  glUIPropertyBar             *property_bar);

static void null_cmd             (BonoboUIComponent           *ui_component,
				  gpointer                     user_data,
				  const gchar                 *verbname);




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

		type = g_type_register_static (G_TYPE_OBJECT,
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

	property_bar->view = NULL;

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

	if (property_bar->view) {
		g_object_unref (G_OBJECT(property_bar->view));
		property_bar = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_PROPERTY_BAR, "END");
}

/****************************************************************************/
/* Create a NEW property_bar.                                               */
/****************************************************************************/
GObject *
gl_ui_property_bar_new (BonoboUIComponent *ui_component)
{
	glUIPropertyBar *property_bar;

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	property_bar = g_object_new (gl_ui_property_bar_get_type (), NULL);

	gl_ui_property_bar_construct (property_bar, ui_component);

	gl_debug (DEBUG_PROPERTY_BAR, "END");

	return G_OBJECT(property_bar);
}

/******************************************************************************/
/* Initialize property toolbar.                                               */
/******************************************************************************/
static void
gl_ui_property_bar_construct (glUIPropertyBar   *property_bar,
			      BonoboUIComponent *ui_component)
{
	GtkWidget  *wcombo;
	GList      *family_names = NULL;
	GtkObject  *adjust;
	ColorGroup *cg;
	GdkPixbuf  *pixbuf;
	gchar      *good_font_family;
	GdkColor   *gdk_color;

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	property_bar->stop_signals = TRUE;
	property_bar->ui_component = ui_component;

	/* Font family entry widget */
	wcombo = gtk_combo_new ();
	family_names = gnome_font_family_list ();
	gtk_combo_set_popdown_strings (GTK_COMBO (wcombo), family_names);
	property_bar->font_family_entry = GTK_COMBO (wcombo)->entry;
	gtk_combo_set_value_in_list (GTK_COMBO(wcombo), TRUE, TRUE);
	gtk_entry_set_editable (GTK_ENTRY (property_bar->font_family_entry), FALSE);
	gtk_widget_set_size_request (wcombo, 200, -1);

	/* Make sure we have a valid font family.  if not provide a good default. */
	if (g_list_find_custom (family_names,
				gl_prefs->default_font_family,
				(GCompareFunc)g_utf8_collate)) {
		good_font_family = g_strdup (gl_prefs->default_font_family);
	} else {
		if (family_names != NULL) {
			good_font_family = g_strdup (family_names->data); /* 1st entry */
		} else {
			good_font_family = NULL;
		}
	}
	gtk_entry_set_text (GTK_ENTRY (property_bar->font_family_entry), good_font_family);
	g_free (good_font_family);
	gnome_font_family_list_free (family_names);

	g_signal_connect (G_OBJECT (property_bar->font_family_entry),
			  "changed", G_CALLBACK (font_family_changed_cb), property_bar);

	gl_ui_util_insert_widget (ui_component, wcombo, "/PropertyToolbar/PropFontName");

	/* Font size entry widget */
	adjust = gtk_adjustment_new (1.0, 1.0, 250.0, 1.0, 10.0, 10.0);
	property_bar->font_size_spin =  gtk_spin_button_new (GTK_ADJUSTMENT (adjust),
							     1.0, 0);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(property_bar->font_size_spin),
				   gl_prefs->default_font_size);

	g_signal_connect (G_OBJECT (property_bar->font_size_spin),
			  "changed", G_CALLBACK (font_size_changed_cb), property_bar);

	gl_ui_util_insert_widget (ui_component, property_bar->font_size_spin,
				  "/PropertyToolbar/PropFontSize");

	/* Bold and Italic toggles */
	bonobo_ui_component_add_verb (ui_component, "PropFontBold", null_cmd, NULL);
	gl_ui_util_set_verb_state (ui_component, "/commands/PropFontBold",
				   (gl_prefs->default_font_weight == GNOME_FONT_BOLD));
	bonobo_ui_component_add_listener (ui_component, "PropFontBold", 
			(BonoboUIListenerFn)prop_item_toggled_cb, 
			(gpointer)property_bar);
	bonobo_ui_component_add_verb (ui_component, "PropFontItalic", null_cmd, NULL);
	gl_ui_util_set_verb_state (ui_component, "/commands/PropFontItalic",
				   gl_prefs->default_font_italic_flag);
	bonobo_ui_component_add_listener (ui_component, "PropFontItalic", 
			(BonoboUIListenerFn)prop_item_toggled_cb, 
			(gpointer)property_bar);

	/* Text alignment radio group */
	bonobo_ui_component_add_verb (ui_component, "PropTextAlignLeft", null_cmd, NULL);
	gl_ui_util_set_verb_state (ui_component, "/commands/PropTextAlignLeft",
				   (gl_prefs->default_text_alignment == GTK_JUSTIFY_LEFT));
	bonobo_ui_component_add_listener (ui_component, "PropTextAlignLeft", 
			(BonoboUIListenerFn)prop_item_toggled_cb, 
			(gpointer)property_bar);
	bonobo_ui_component_add_verb (ui_component, "PropTextAlignCenter", null_cmd, NULL);
	gl_ui_util_set_verb_state (ui_component, "/commands/PropTextAlignCenter",
				   (gl_prefs->default_text_alignment == GTK_JUSTIFY_CENTER));
	bonobo_ui_component_add_listener (ui_component, "PropTextAlignCenter", 
			(BonoboUIListenerFn)prop_item_toggled_cb, 
			(gpointer)property_bar);
	bonobo_ui_component_add_verb (ui_component, "PropTextAlignRight", null_cmd, NULL);
	gl_ui_util_set_verb_state (ui_component, "/commands/PropTextAlignRight",
				   (gl_prefs->default_text_alignment == GTK_JUSTIFY_RIGHT));
	bonobo_ui_component_add_listener (ui_component, "PropTextAlignRight", 
			(BonoboUIListenerFn)prop_item_toggled_cb, 
			(gpointer)property_bar);

	/* Text color widget */
	cg = color_group_fetch ("text_color_group", NULL);
	pixbuf = gdk_pixbuf_new_from_inline (-1, stock_text_24, FALSE, NULL);
	gdk_color = gl_color_to_gdk_color (gl_prefs->default_text_color);
	property_bar->text_color_combo = color_combo_new (pixbuf, _("Default"),
							  gdk_color, cg);
	g_free (gdk_color);

	g_signal_connect (G_OBJECT (property_bar->text_color_combo),
			  "color_changed",
			  G_CALLBACK (text_color_changed_cb), property_bar);

	gl_ui_util_insert_widget (ui_component, property_bar->text_color_combo,
				  "/PropertyToolbar/PropTextColor");

	/* Fill color widget */
	cg = color_group_fetch ("fill_color_group", NULL);
	pixbuf = gdk_pixbuf_new_from_inline (-1, stock_bucket_fill_24, FALSE, NULL);
	gdk_color = gl_color_to_gdk_color (gl_prefs->default_fill_color);
	property_bar->fill_color_combo = color_combo_new (pixbuf, _("No fill"),
							  gdk_color, cg);
	g_free (gdk_color);

	g_signal_connect (G_OBJECT (property_bar->fill_color_combo),
			  "color_changed",
			  G_CALLBACK (fill_color_changed_cb), property_bar);

	gl_ui_util_insert_widget (ui_component, property_bar->fill_color_combo,
				  "/PropertyToolbar/PropFillColor");

	/* Line color widget */
	cg = color_group_fetch ("line_color_group", NULL);
	pixbuf = gdk_pixbuf_new_from_inline (-1, stock_pencil_24, FALSE, NULL);
	gdk_color = gl_color_to_gdk_color (gl_prefs->default_line_color);
	property_bar->line_color_combo = color_combo_new (pixbuf, _("No line"),
							  gdk_color, cg);
	g_free (gdk_color);

	g_signal_connect (G_OBJECT (property_bar->line_color_combo),
			  "color_changed",
			  G_CALLBACK (line_color_changed_cb), property_bar);

	gl_ui_util_insert_widget (ui_component, property_bar->line_color_combo,
				  "/PropertyToolbar/PropLineColor");

	/* Line width entry widget */
	adjust = gtk_adjustment_new (1.0, 0.25, 4.0, 0.25, 1.0, 1.0);
	property_bar->line_width_spin =  gtk_spin_button_new (GTK_ADJUSTMENT (adjust), 0.25, 2);

	g_signal_connect (G_OBJECT (property_bar->line_width_spin),
			  "changed",
			  G_CALLBACK (line_width_changed_cb), property_bar);

	gl_ui_util_insert_widget (ui_component, property_bar->line_width_spin,
				  "/PropertyToolbar/PropLineWidth");

	gl_ui_util_set_verb_list_sensitive (ui_component, doc_verbs, FALSE);

	property_bar->stop_signals = FALSE;

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

	/* Make sure we have a valid font family.  if not provide a good default. */
	family_names = gnome_font_family_list ();
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
	gtk_entry_set_text (GTK_ENTRY (property_bar->font_family_entry), good_font_family);
	g_free (good_font_family);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON(property_bar->font_size_spin),
				   view->default_font_size);

	gl_ui_util_set_verb_state (property_bar->ui_component,
				   "/commands/PropFontBold",
				   (view->default_font_weight == GNOME_FONT_BOLD));
	gl_ui_util_set_verb_state (property_bar->ui_component,
				   "/commands/PropFontItalic",
				   view->default_font_italic_flag);

	gl_ui_util_set_verb_state (property_bar->ui_component,
				   "/commands/PropTextAlignLeft",
				   (view->default_text_alignment == GTK_JUSTIFY_LEFT));
	gl_ui_util_set_verb_state (property_bar->ui_component,
				   "/commands/PropTextAlignCenter",
				   (view->default_text_alignment == GTK_JUSTIFY_CENTER));
	gl_ui_util_set_verb_state (property_bar->ui_component,
				   "/commands/PropTextAlignRight",
				   (view->default_text_alignment == GTK_JUSTIFY_RIGHT));

	gdk_color = gl_color_to_gdk_color (view->default_text_color);
	color_combo_set_color (COLOR_COMBO(property_bar->text_color_combo), gdk_color);
	g_free (gdk_color);

	gdk_color = gl_color_to_gdk_color (view->default_fill_color);
	color_combo_set_color (COLOR_COMBO(property_bar->fill_color_combo), gdk_color);
	g_free (gdk_color);

	gdk_color = gl_color_to_gdk_color (view->default_line_color);
	color_combo_set_color (COLOR_COMBO(property_bar->line_color_combo), gdk_color);
	g_free (gdk_color);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON(property_bar->line_width_spin),
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

	gl_ui_util_set_verb_list_sensitive (property_bar->ui_component, doc_verbs, TRUE);

	property_bar->view = GL_VIEW (g_object_ref (G_OBJECT (view)));

	reset_to_default_properties (view, property_bar);

	g_signal_connect_swapped (G_OBJECT(view), "selection_changed",
				  G_CALLBACK(selection_changed_cb), property_bar);

	g_signal_connect_swapped (G_OBJECT(view->label), "changed",
				  G_CALLBACK(selection_changed_cb), property_bar);

	gl_debug (DEBUG_PROPERTY_BAR, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  View "selection state changed" callback.                        */
/*---------------------------------------------------------------------------*/

static void
update_text_properties (glView *view,
			glUIPropertyBar *property_bar)
{
	gboolean can_text, is_first_object;
	gboolean is_same_font_family, is_same_font_size;
	gboolean is_same_text_color, is_same_is_italic;
	gboolean is_same_is_bold, is_same_justification;
	GList *p;
	glLabelObject *object;
	gchar *selection_font_family, *font_family;
	gdouble selection_font_size, font_size;
	guint selection_text_color, text_color;
	glColorNode *text_color_node;
	gboolean selection_is_italic, is_italic;
	gboolean selection_is_bold, is_bold;
	GtkJustification selection_justification, justification;
	GdkColor *gdk_color;

	can_text = gl_view_can_selection_text (view);
	gl_ui_util_set_verb_list_sensitive (property_bar->ui_component,
					    text_verbs,
					    can_text);
	if (!can_text) 
		return;

	is_same_is_italic =
	is_same_is_bold =
	is_same_justification =
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
		is_bold = gl_label_object_get_font_weight (object) == GNOME_FONT_BOLD;
		justification = gl_label_object_get_text_alignment (object);

		if (is_first_object) {
			selection_font_size = font_size;
			selection_text_color = text_color;
			selection_is_italic = is_italic;
			selection_is_bold = is_bold;
			selection_justification = justification;
		} else {
			if (font_size != selection_font_size) 
				is_same_font_size = FALSE;
			if (text_color != selection_text_color)
				is_same_text_color = FALSE;
			if (is_italic != selection_is_italic)
				is_same_is_italic = FALSE;
			if (is_bold != selection_is_bold)
				is_same_is_bold = FALSE;
			if (justification != selection_justification)
				is_same_justification = FALSE;
		}
		is_first_object = FALSE;
	}

	if (is_same_font_family && (selection_font_family != NULL)) 
		gl_debug (DEBUG_PROPERTY_BAR, "same font family = %s", 
			  selection_font_family);
	gtk_entry_set_text (GTK_ENTRY (property_bar->font_family_entry),
			    is_same_font_family?selection_font_family:"");
	g_free (selection_font_family);

	if (is_same_font_size) {
		gl_debug (DEBUG_PROPERTY_BAR, "same font size = %g", 
			  selection_font_size);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (property_bar->font_size_spin),
					   selection_font_size);
	} else {
		gtk_entry_set_text (GTK_ENTRY (property_bar->font_size_spin), "");
	}

	if (is_same_text_color) {
		gl_debug (DEBUG_PROPERTY_BAR, "same text color = %08x", selection_text_color);
		gdk_color = gl_color_to_gdk_color (selection_text_color);
		color_combo_set_color (COLOR_COMBO (property_bar->text_color_combo),
				       gdk_color);
		g_free (gdk_color);
	}

	if (is_same_is_italic)  
		gl_debug (DEBUG_PROPERTY_BAR, "same italic flag = %d", 
			  selection_is_italic);
	gl_ui_util_set_verb_state (property_bar->ui_component,
				   "/commands/PropFontItalic",
				   selection_is_italic && is_same_is_italic);

	if (is_same_is_bold)  
		gl_debug (DEBUG_PROPERTY_BAR, "same bold flag = %d",
			  selection_is_bold);
	gl_ui_util_set_verb_state (property_bar->ui_component,
				   "/commands/PropFontBold",
				   selection_is_bold && is_same_is_bold);

	if (is_same_justification) 
		gl_debug (DEBUG_PROPERTY_BAR, "same justification");
	gl_ui_util_set_verb_state (property_bar->ui_component,
				   "/commands/PropTextAlignRight",
				   (selection_justification == GTK_JUSTIFY_RIGHT) &&
				   is_same_justification);
	gl_ui_util_set_verb_state (property_bar->ui_component,
				   "/commands/PropTextAlignLeft",
				   (selection_justification == GTK_JUSTIFY_LEFT) &&
				   is_same_justification);
	gl_ui_util_set_verb_state (property_bar->ui_component,
				   "/commands/PropTextAlignCenter",
				   (selection_justification == GTK_JUSTIFY_CENTER) &&
				   is_same_justification);
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
	gl_ui_util_set_verb_list_sensitive (property_bar->ui_component,
					    fill_verbs,
					    can);
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
		color_combo_set_color (COLOR_COMBO (property_bar->fill_color_combo),
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
	gl_ui_util_set_verb_list_sensitive (property_bar->ui_component,
					    line_color_verbs,
					    can);
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
		color_combo_set_color (COLOR_COMBO (property_bar->line_color_combo),
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
	gl_ui_util_set_verb_list_sensitive (property_bar->ui_component,
					    line_width_verbs,
					    can);
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
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (property_bar->line_width_spin),
					   selection_line_width);
	} else {
		gtk_entry_set_text (GTK_ENTRY (property_bar->line_width_spin), "");
	}
}

static void 
selection_changed_cb (glUIPropertyBar *property_bar)
{
	glView *view = property_bar->view;
	
	gl_debug (DEBUG_PROPERTY_BAR, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (property_bar && GL_IS_UI_PROPERTY_BAR (property_bar));

	property_bar->stop_signals = TRUE;

	if (gl_view_is_selection_empty (view)) {

		/* No selection: make all controls active. */
		reset_to_default_properties (view, property_bar);
		gl_ui_util_set_verb_list_sensitive (property_bar->ui_component,
						    doc_verbs, TRUE);

	} else {

		update_text_properties (view, property_bar);
		update_fill_color (view, property_bar);
		update_line_color (view, property_bar);
		update_line_width (view, property_bar);

	}

	property_bar->stop_signals = FALSE;

	gl_debug (DEBUG_PROPERTY_BAR, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Font family entry changed.                                     */
/*--------------------------------------------------------------------------*/
static void
font_family_changed_cb (GtkEntry        *entry,
			glUIPropertyBar *property_bar)
{
	gchar *font_family;

	if (property_bar->stop_signals)
		return;

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(property_bar->view->label),
					 selection_changed_cb,
					 property_bar);

	font_family = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	if ( strlen(font_family) ) {
		gl_view_set_selection_font_family (property_bar->view,
						   font_family);
		gl_view_set_default_font_family   (property_bar->view,
						   font_family);
	}
	g_free (font_family);

	g_signal_handlers_unblock_by_func (G_OBJECT(property_bar->view->label),
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

	if (property_bar->stop_signals)
		return;

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(property_bar->view->label),
					 selection_changed_cb,
					 property_bar);

	font_size = gtk_spin_button_get_value (spin);

	gl_view_set_selection_font_size (property_bar->view,
					 font_size);
	gl_view_set_default_font_size   (property_bar->view,
					 font_size);

	g_signal_handlers_unblock_by_func (G_OBJECT(property_bar->view->label),
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

	if (property_bar->stop_signals)
		return;

	g_return_if_fail (property_bar && GL_IS_UI_PROPERTY_BAR (property_bar));

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(property_bar->view->label),
					 selection_changed_cb,
					 property_bar);

	text_color_node = gl_color_node_new_default ();
	text_color_node->color = gl_color_from_gdk_color (gdk_color);
	
	gl_debug (DEBUG_PROPERTY_BAR, "Color=%08x, Custom=%d, By_User=%d, Is_default=%d",
		  text_color_node->color, custom, by_user, is_default);

	if (is_default) {
		text_color_node->color = gl_prefs->default_text_color;
		gl_view_set_selection_text_color (property_bar->view,
						  text_color_node);
		gl_view_set_default_text_color   (property_bar->view,
						  gl_prefs->default_text_color);

	} else {

		gl_view_set_selection_text_color (property_bar->view,
						  text_color_node);
		gl_view_set_default_text_color   (property_bar->view,
						  text_color_node->color);

	}

	gl_color_node_free (&text_color_node);
	
	g_signal_handlers_unblock_by_func (G_OBJECT(property_bar->view->label),
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

	if (property_bar->stop_signals)
		return;

	g_return_if_fail (property_bar && GL_IS_UI_PROPERTY_BAR (property_bar));

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(property_bar->view->label),
					 selection_changed_cb,
					 property_bar);

	fill_color_node = gl_color_node_new_default ();

	fill_color_node->color = gl_color_from_gdk_color (gdk_color);

	gl_debug (DEBUG_PROPERTY_BAR, "Color=%08x, Custom=%d, By_User=%d, Is_default=%d",
		  fill_color_node->color, custom, by_user, is_default);

	if (is_default) {

		fill_color_node->color = GL_COLOR_NONE;
		gl_view_set_selection_fill_color (property_bar->view,
						  fill_color_node);
		gl_view_set_default_fill_color   (property_bar->view,
						  fill_color_node->color);

	} else {

		gl_view_set_selection_fill_color (property_bar->view,
						  fill_color_node);
		gl_view_set_default_fill_color   (property_bar->view,
						  fill_color_node->color);

	}
	gl_color_node_free (&fill_color_node);
	
	g_signal_handlers_unblock_by_func (G_OBJECT(property_bar->view->label),
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

	if (property_bar->stop_signals)
		return;

	g_return_if_fail (property_bar && GL_IS_UI_PROPERTY_BAR (property_bar));

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(property_bar->view->label),
					 selection_changed_cb,
					 property_bar);

	line_color_node = gl_color_node_new_default ();
	line_color_node->color = gl_color_from_gdk_color (gdk_color);

	gl_debug (DEBUG_PROPERTY_BAR, "Color=%08x, Custom=%d, By_User=%d, Is_default=%d",
		  line_color_node->color, custom, by_user, is_default);

	if (is_default) {
		line_color_node->color = GL_COLOR_NONE;
		gl_view_set_selection_line_color (property_bar->view,
						  line_color_node);
		gl_view_set_default_line_color   (property_bar->view,
						  line_color_node->color);

	} else {

		gl_view_set_selection_line_color (property_bar->view,
						  line_color_node);
		gl_view_set_default_line_color   (property_bar->view,
						  line_color_node->color);

	}
	gl_color_node_free (&line_color_node);

	g_signal_handlers_unblock_by_func (G_OBJECT(property_bar->view->label),
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

	if (property_bar->stop_signals)
		return;

	gl_debug (DEBUG_PROPERTY_BAR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(property_bar->view->label),
					 selection_changed_cb,
					 property_bar);

	line_width = gtk_spin_button_get_value (spin);

	gl_view_set_selection_line_width (property_bar->view,
					  line_width);
	gl_view_set_default_line_width   (property_bar->view,
					  line_width);

	g_signal_handlers_unblock_by_func (G_OBJECT(property_bar->view->label),
					   selection_changed_cb,
					   property_bar);

	gl_debug (DEBUG_PROPERTY_BAR, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Property bar item toggled callback.                             */
/*---------------------------------------------------------------------------*/
static void
prop_item_toggled_cb (BonoboUIComponent           *ui_component,
		      const char                  *path,
		      Bonobo_UIComponent_EventType type,
		      const char                  *state,
		      glUIPropertyBar             *property_bar)
{
	gboolean s;

	if (property_bar->stop_signals)
		return;

	gl_debug (DEBUG_PROPERTY_BAR, "");

	g_signal_handlers_block_by_func (G_OBJECT(property_bar->view->label),
					 selection_changed_cb,
					 property_bar);

	s = (strcmp (state, "1") == 0);

	/*
	 * Font properties: Bold & Italic
	 */
	if (strcmp (path, "PropFontBold") == 0)
	{		
		GnomeFontWeight weight = s ? GNOME_FONT_BOLD : GNOME_FONT_BOOK;

		gl_view_set_selection_font_weight (property_bar->view, weight);
		gl_view_set_default_font_weight   (property_bar->view, weight);
	}

	if (strcmp (path, "PropFontItalic") == 0)
	{		
		gl_view_set_selection_font_italic_flag (property_bar->view, s);
		gl_view_set_default_font_italic_flag   (property_bar->view, s);
	}


	/*
	 * Text alignment group
	 */
	if (s && (strcmp (path, "PropTextAlignLeft") == 0))
	{		
		gl_view_set_selection_text_alignment (property_bar->view,
						      GTK_JUSTIFY_LEFT);
		gl_view_set_default_text_alignment   (property_bar->view,
						      GTK_JUSTIFY_LEFT);

		/* Adjust state of siblings */
		gl_ui_util_set_verb_state (ui_component,
					   "/commands/PropTextAlignCenter",
					   FALSE);
		gl_ui_util_set_verb_state (ui_component,
					   "/commands/PropTextAlignRight",
					   FALSE);
	}

	if (s && (strcmp (path, "PropTextAlignCenter") == 0))
	{		
		gl_view_set_selection_text_alignment (property_bar->view,
						      GTK_JUSTIFY_CENTER);
		gl_view_set_default_text_alignment   (property_bar->view,
						      GTK_JUSTIFY_CENTER);

		/* Adjust state of siblings */
		gl_ui_util_set_verb_state (ui_component,
					   "/commands/PropTextAlignLeft",
					   FALSE);
		gl_ui_util_set_verb_state (ui_component,
					   "/commands/PropTextAlignRight",
					   FALSE);
	}

	if (s && (strcmp (path, "PropTextAlignRight") == 0))
	{		
		gl_view_set_selection_text_alignment (property_bar->view,
						      GTK_JUSTIFY_RIGHT);
		gl_view_set_default_text_alignment   (property_bar->view,
						      GTK_JUSTIFY_RIGHT);

		/* Adjust state of siblings */
		gl_ui_util_set_verb_state (ui_component,
					   "/commands/PropTextAlignLeft",
					   FALSE);
		gl_ui_util_set_verb_state (ui_component,
					   "/commands/PropTextAlignCenter",
					   FALSE);
	}

	g_signal_handlers_unblock_by_func (G_OBJECT(property_bar->view->label),
					   selection_changed_cb,
					   property_bar);

}

/*---------------------------------------------------------------------------------------*/
/* NULL command handler -- hides bogus "verb not found" errors for toggle toolbar items. */
/*---------------------------------------------------------------------------------------*/
static void
null_cmd (BonoboUIComponent           *ui_component,
	  gpointer                     user_data,
	  const gchar                 *verbname)
{
}
