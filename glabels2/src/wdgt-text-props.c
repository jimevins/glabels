/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_text_props.c:  text properties widget module
 *
 *  Copyright (C) 2001-2002  Jim Evins <evins@snaught.com>.
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

#include "mygal/widget-color-combo.h"
#include "prefs.h"
#include "wdgt-text-props.h"
#include "marshal.h"
#include "color.h"

#include "debug.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};

typedef void (*glWdgtTextPropsSignal) (GObject *object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GObjectClass *parent_class;

static gint wdgt_text_props_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_wdgt_text_props_class_init    (glWdgtTextPropsClass *class);
static void gl_wdgt_text_props_instance_init (glWdgtTextProps      *text);
static void gl_wdgt_text_props_finalize      (GObject              *object);
static void gl_wdgt_text_props_construct     (glWdgtTextProps      *text);

static void family_changed_cb (GtkEntry        *entry,
			       glWdgtTextProps *text);

static void changed_cb        (glWdgtTextProps *text);

static void just_toggled_cb   (GtkToggleButton *togglebutton,
			       gpointer         user_data);

/*****************************************************************************/
/* Boilerplate Object stuff.                                                 */
/*****************************************************************************/
guint
gl_wdgt_text_props_get_type (void)
{
	static guint wdgt_text_props_type = 0;

	if (!wdgt_text_props_type) {
		GTypeInfo wdgt_text_props_info = {
			sizeof (glWdgtTextPropsClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_wdgt_text_props_class_init,
			NULL,
			NULL,
			sizeof (glWdgtTextProps),
			0,
			(GInstanceInitFunc) gl_wdgt_text_props_instance_init,
		};

		wdgt_text_props_type =
		    g_type_register_static (gl_hig_vbox_get_type (),
					    "glWdgtTextProps",
					    &wdgt_text_props_info, 0);
	}

	return wdgt_text_props_type;
}

static void
gl_wdgt_text_props_class_init (glWdgtTextPropsClass *class)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) class;

	parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_wdgt_text_props_finalize;

	wdgt_text_props_signals[CHANGED] =
	    g_signal_new ("changed",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glWdgtTextPropsClass, changed),
			  NULL, NULL,
			  gl_marshal_VOID__VOID,
			  G_TYPE_NONE, 0);

}

static void
gl_wdgt_text_props_instance_init (glWdgtTextProps *text)
{
	text->font_family_entry = NULL;
	text->font_size_spin = NULL;
	text->font_b_button = NULL;
	text->font_i_button = NULL;

	text->color_picker = NULL;

	text->left_button = NULL;
	text->right_button = NULL;
	text->center_button = NULL;
}

static void
gl_wdgt_text_props_finalize (GObject *object)
{
	glWdgtTextProps *text;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WDGT_TEXT_PROPS (object));

	text = GL_WDGT_TEXT_PROPS (object);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/*****************************************************************************/
/* New widget.                                                               */
/*****************************************************************************/
GtkWidget *
gl_wdgt_text_props_new (void)
{
	glWdgtTextProps *text;

	text = g_object_new (gl_wdgt_text_props_get_type (), NULL);

	gl_wdgt_text_props_construct (text);

	return GTK_WIDGET (text);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE. Construct composite widget.                                     */
/*--------------------------------------------------------------------------*/
static void
gl_wdgt_text_props_construct (glWdgtTextProps *text)
{
	GtkWidget  *wvbox, *whbox, *wcombo, *wbhbox;
	GList      *family_names = NULL;
	GtkObject  *adjust;
	ColorGroup *cg;
	GdkColor   *gdk_color;

	wvbox = GTK_WIDGET (text);

	/* ---- Font line ---- */
	whbox = gl_hig_hbox_new ();
	gl_hig_vbox_add_widget (GL_HIG_VBOX(wvbox), whbox);

	/* Font label */
	text->font_label = gtk_label_new (_("Font:"));
	gtk_misc_set_alignment (GTK_MISC (text->font_label), 0, 0.5);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), text->font_label);

	/* Font family entry widget */
	wcombo = gtk_combo_new ();
	family_names = gnome_font_family_list ();
	gtk_combo_set_popdown_strings (GTK_COMBO (wcombo), family_names);
	gnome_font_family_list_free (family_names);
	text->font_family_entry = GTK_COMBO (wcombo)->entry;
	gtk_combo_set_value_in_list (GTK_COMBO(wcombo), TRUE, FALSE);
	gtk_entry_set_editable (GTK_ENTRY (text->font_family_entry), FALSE);
	gtk_widget_set_size_request (wcombo, 200, -1);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), wcombo);
	g_signal_connect (G_OBJECT (text->font_family_entry),
			  "changed", G_CALLBACK (family_changed_cb), text);

	/* Font size entry widget */
	adjust = gtk_adjustment_new (1.0, 1.0, 250.0, 1.0, 10.0, 10.0);
	text->font_size_spin =
	    gtk_spin_button_new (GTK_ADJUSTMENT (adjust), 1.0, 0);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), text->font_size_spin);
	g_signal_connect_swapped (G_OBJECT (text->font_size_spin), "changed",
				  G_CALLBACK (changed_cb),
				  G_OBJECT (text));

	/* Font weight/italic button widgets */
	text->font_b_button = gtk_toggle_button_new ();
	gtk_container_add (GTK_CONTAINER (text->font_b_button),
			   gtk_image_new_from_stock (GTK_STOCK_BOLD,
						     GTK_ICON_SIZE_BUTTON));
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), text->font_b_button);
	g_signal_connect_swapped (G_OBJECT (text->font_b_button), "toggled",
				  G_CALLBACK (changed_cb),
				  G_OBJECT (text));
	text->font_i_button = gtk_toggle_button_new ();
	gtk_container_add (GTK_CONTAINER (text->font_i_button),
			   gtk_image_new_from_stock (GTK_STOCK_ITALIC,
						     GTK_ICON_SIZE_BUTTON));
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), text->font_i_button);
	g_signal_connect_swapped (G_OBJECT (text->font_i_button), "toggled",
				  G_CALLBACK (changed_cb),
				  G_OBJECT (text));

	/* ---- Color line ---- */
	whbox = gl_hig_hbox_new ();
	gl_hig_vbox_add_widget (GL_HIG_VBOX(wvbox), whbox);

	/* Text Color Label */
	text->color_label = gtk_label_new (_("Color:"));
	gtk_misc_set_alignment (GTK_MISC (text->color_label), 0, 0.5);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), text->color_label);

	/* Text Color picker widget */
        cg = color_group_fetch ("text_color_group", NULL);
        gdk_color = gl_color_to_gdk_color (gl_prefs->default_line_color);
        text->color_picker = color_combo_new (NULL, _("Default"), gdk_color, cg);
	color_combo_box_set_preview_relief (COLOR_COMBO(text->color_picker),
					    GTK_RELIEF_NORMAL);
        g_free (gdk_color);
	g_signal_connect_swapped (G_OBJECT (text->color_picker), "color_changed",
				  G_CALLBACK (changed_cb),
				  G_OBJECT (text));
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), text->color_picker);

	/* ---- Alignment line ---- */
	whbox = gl_hig_hbox_new ();
	gl_hig_vbox_add_widget (GL_HIG_VBOX(wvbox), whbox);

	/* Alignment label */
	text->alignment_label = gtk_label_new (_("Alignment:"));
	gtk_misc_set_alignment (GTK_MISC (text->alignment_label), 0, 0.5);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), text->alignment_label);

	/* Justification entry widget */
	text->left_button = gtk_toggle_button_new ();
	gtk_container_add (GTK_CONTAINER (text->left_button),
			   gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_LEFT,
						     GTK_ICON_SIZE_BUTTON));
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), text->left_button);
	text->center_button = gtk_toggle_button_new ();
	gtk_container_add (GTK_CONTAINER (text->center_button),
			   gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_CENTER,
						     GTK_ICON_SIZE_BUTTON));
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), text->center_button);
	text->right_button = gtk_toggle_button_new ();
	gtk_container_add (GTK_CONTAINER (text->right_button),
			   gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_RIGHT,
						     GTK_ICON_SIZE_BUTTON));
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), text->right_button);

	/* Now connect a callback that makes these toggles mutually exclusive */
	g_signal_connect (G_OBJECT (text->left_button), "toggled",
			  G_CALLBACK (just_toggled_cb), text);
	g_signal_connect (G_OBJECT (text->center_button), "toggled",
			  G_CALLBACK (just_toggled_cb), text);
	g_signal_connect (G_OBJECT (text->right_button), "toggled",
			  G_CALLBACK (just_toggled_cb), text);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change in selection                       */
/*--------------------------------------------------------------------------*/
static void
family_changed_cb (GtkEntry        *entry,
		   glWdgtTextProps *text)
{
	gchar *family_name;

	family_name = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	if ( strlen(family_name) ) {
		/* Emit our "changed" signal */
		g_signal_emit (G_OBJECT (text),
			       wdgt_text_props_signals[CHANGED], 0);
	}
	g_free (family_name);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the widget has changed.       */
/*--------------------------------------------------------------------------*/
static void
changed_cb (glWdgtTextProps *text)
{
	/* Emit our "changed" signal */
	g_signal_emit (G_OBJECT (text), wdgt_text_props_signals[CHANGED], 0);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Justify toggle button callback.                                */
/*--------------------------------------------------------------------------*/
static void
just_toggled_cb (GtkToggleButton *togglebutton,
		 gpointer         user_data)
{
	glWdgtTextProps *text = GL_WDGT_TEXT_PROPS (user_data);

	if (gtk_toggle_button_get_active (togglebutton)) {

		if (GTK_WIDGET (togglebutton) == GTK_WIDGET (text->left_button)) {
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
						      (text->center_button),
						      FALSE);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
						      (text->right_button),
						      FALSE);
		} else if (GTK_WIDGET (togglebutton) ==
			   GTK_WIDGET (text->center_button)) {
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
						      (text->left_button),
						      FALSE);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
						      (text->right_button),
						      FALSE);
		} else if (GTK_WIDGET (togglebutton) ==
			   GTK_WIDGET (text->right_button)) {
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
						      (text->left_button),
						      FALSE);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
						      (text->center_button),
						      FALSE);
		}

	}

	/* Emit our "changed" signal */
	g_signal_emit (G_OBJECT (text), wdgt_text_props_signals[CHANGED], 0);
}

/*****************************************************************************/
/* query values from controls.                                               */
/*****************************************************************************/
void
gl_wdgt_text_props_get_params (glWdgtTextProps  *text,
			       gchar            **font_family,
			       gdouble          *font_size,
			       GnomeFontWeight  *font_weight,
			       gboolean         *font_italic_flag,
			       guint            *color,
			       GtkJustification *just)
{
	GdkColor *gdk_color;
	gboolean  is_default;

	/* ------ Get updated font information ------ */
	*font_family =
	    gtk_editable_get_chars (GTK_EDITABLE (text->font_family_entry), 0,
				    -1);
	*font_size =
	    gtk_spin_button_get_value (GTK_SPIN_BUTTON(text->font_size_spin));
	if (gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON (text->font_b_button))) {
		*font_weight = GNOME_FONT_BOLD;
	} else {
		*font_weight = GNOME_FONT_BOOK;
	}
	*font_italic_flag =
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
					  (text->font_i_button));

	/* ------ Get updated color ------ */
	gdk_color = color_combo_get_color (COLOR_COMBO(text->color_picker),
					   &is_default);

	if (is_default) {
		*color = gl_prefs->default_text_color;
	} else {
		*color = gl_color_from_gdk_color (gdk_color);
	}

	/* ------- Get updated justification ------ */
	if (gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON (text->left_button))) {
		*just = GTK_JUSTIFY_LEFT;
	} else
	    if (gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (text->right_button))) {
		*just = GTK_JUSTIFY_RIGHT;
	} else
	    if (gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (text->center_button))) {
		*just = GTK_JUSTIFY_CENTER;
	} else {
		*just = GTK_JUSTIFY_LEFT;	/* Should not happen. */
	}

}

/*****************************************************************************/
/* fill in values and ranges for controls.                                   */
/*****************************************************************************/
void
gl_wdgt_text_props_set_params (glWdgtTextProps  *text,
			       gchar            *font_family,
			       gdouble          font_size,
			       GnomeFontWeight  font_weight,
			       gboolean         font_italic_flag,
			       guint            color,
			       GtkJustification just)
{
	GList    *family_names;
	gchar    *good_font_family;
	GdkColor *gdk_color;

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

	gtk_entry_set_text (GTK_ENTRY (text->font_family_entry), good_font_family);
	g_free (good_font_family);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (text->font_size_spin),
				   font_size);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (text->font_b_button),
				      (font_weight == GNOME_FONT_BOLD));

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (text->font_i_button),
				      font_italic_flag);

	gdk_color = gl_color_to_gdk_color (color);
	color_combo_set_color (COLOR_COMBO(text->color_picker), gdk_color);
	g_free (gdk_color);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (text->left_button),
				      (just == GTK_JUSTIFY_LEFT));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (text->center_button),
				      (just == GTK_JUSTIFY_CENTER));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (text->right_button),
				      (just == GTK_JUSTIFY_RIGHT));
}

/*****************************************************************************/
/* Set size group for internal labels                                        */
/*****************************************************************************/
void
gl_wdgt_text_props_set_label_size_group (glWdgtTextProps *text,
					 GtkSizeGroup    *label_size_group)
{
	gtk_size_group_add_widget (label_size_group, text->font_label);
	gtk_size_group_add_widget (label_size_group, text->color_label);
	gtk_size_group_add_widget (label_size_group, text->alignment_label);
}

