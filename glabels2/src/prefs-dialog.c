/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prefs-dialog.c:  Preferences dialog module
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

#include "prefs-dialog.h"

#include <glib/gi18n.h>
#include <glade/glade-xml.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkcombobox.h>
#include <gtk/gtkspinbutton.h>

#include "prefs.h"
#include "hig.h"
#include "mygal/widget-color-combo.h"
#include "color.h"
#include "util.h"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

#define US_LETTER_ID "US-Letter"
#define A4_ID        "A4"

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glPrefsDialogPrivate
{
	GladeXML   *gui;

	/* Units properties */
	GtkWidget	*units_points_radio;
	GtkWidget	*units_inches_radio;
	GtkWidget	*units_mm_radio;

	/* Page size properties */
	GtkWidget	*page_size_us_letter_radio;
	GtkWidget	*page_size_a4_radio;

	/* Default text properties */
	GtkWidget       *text_family_combo;
	GtkWidget       *text_size_spin;
	GtkWidget       *text_bold_toggle;
	GtkWidget       *text_italic_toggle;
	GtkWidget       *text_color_combo;
	GtkWidget       *text_left_toggle;
	GtkWidget       *text_center_toggle;
	GtkWidget       *text_right_toggle;
	GtkWidget       *text_line_spacing_spin;

	/* Default line properties */
	GtkWidget       *line_width_spin;
	GtkWidget       *line_color_combo;

	/* Default fill properties */
	GtkWidget       *fill_color_combo;

        /* Prevent recursion */
	gboolean    stop_signals;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gl_prefs_dialog_finalize   	  (GObject            *object);
static void gl_prefs_dialog_construct     (glPrefsDialog      *dialog);

static void response_cb                   (glPrefsDialog      *dialog,
					   gint                response,
					   gpointer            user_data);

static void construct_locale_page         (glPrefsDialog      *dialog);
static void construct_object_page         (glPrefsDialog      *dialog);

static void align_toggle_cb               (GtkToggleButton    *toggle,
					   glPrefsDialog      *dialog);
                                                                                
static void update_locale_page_from_prefs (glPrefsDialog      *dialog);
static void update_object_page_from_prefs (glPrefsDialog      *dialog);

static void update_prefs_from_locale_page (glPrefsDialog      *dialog);
static void update_prefs_from_object_page (glPrefsDialog      *dialog);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glPrefsDialog, gl_prefs_dialog, GTK_TYPE_DIALOG);

static void
gl_prefs_dialog_class_init (glPrefsDialogClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	gl_debug (DEBUG_PREFS, "");
	
  	gl_prefs_dialog_parent_class = g_type_class_peek_parent (class);

  	object_class->finalize = gl_prefs_dialog_finalize;  	
}

static void
gl_prefs_dialog_init (glPrefsDialog *dialog)
{
	gl_debug (DEBUG_PREFS, "START");

	dialog->priv = g_new0 (glPrefsDialogPrivate, 1);

	dialog->priv->gui = glade_xml_new (GLABELS_GLADE_DIR "prefs-dialog.glade",
					"prefs_notebook",
					NULL);

	if (!dialog->priv->gui)
        {
		g_critical ("Could not open prefs-dialog.glade. gLabels may not be installed correctly!");
		return;
	}

	gtk_container_set_border_width (GTK_CONTAINER(dialog), GL_HIG_PAD2);

	gtk_dialog_set_has_separator (GTK_DIALOG(dialog), FALSE);
	gtk_dialog_add_button (GTK_DIALOG(dialog), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CLOSE);

	g_signal_connect(G_OBJECT (dialog), "response",
			 G_CALLBACK (response_cb), NULL);

        gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
        gtk_window_set_title (GTK_WINDOW (dialog), _("gLabels Preferences"));
        gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

	gl_debug (DEBUG_PREFS, "END");
}

static void 
gl_prefs_dialog_finalize (GObject *object)
{
	glPrefsDialog* dialog = GL_PREFS_DIALOG (object);
	
	gl_debug (DEBUG_PREFS, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_PREFS_DIALOG (dialog));
	g_return_if_fail (dialog->priv != NULL);

	if (dialog->priv->gui)
        {
		g_object_unref (G_OBJECT (dialog->priv->gui));
	}
	g_free (dialog->priv);

	G_OBJECT_CLASS (gl_prefs_dialog_parent_class)->finalize (object);

	gl_debug (DEBUG_PREFS, "END");
}

/*****************************************************************************/
/* NEW preferences dialog.                                                   */
/*****************************************************************************/
GtkWidget*
gl_prefs_dialog_new (GtkWindow *parent)
{
	GtkWidget *dialog;

	gl_debug (DEBUG_PREFS, "START");
	gl_debug (DEBUG_PREFS, "page size = \"%s\"", gl_prefs->default_page_size);

	dialog = GTK_WIDGET (g_object_new (GL_TYPE_PREFS_DIALOG, NULL));

	if (parent)
		gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);
	
	gl_prefs_dialog_construct (GL_PREFS_DIALOG(dialog));


	gl_debug (DEBUG_PREFS, "END");

	return dialog;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                     */
/*---------------------------------------------------------------------------*/
static void
gl_prefs_dialog_construct (glPrefsDialog *dialog)
{
	GtkWidget *notebook;

	g_return_if_fail (GL_IS_PREFS_DIALOG (dialog));
	g_return_if_fail (dialog->priv != NULL);

	notebook = glade_xml_get_widget (dialog->priv->gui, "prefs_notebook");
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), notebook, FALSE, FALSE, 0);

	construct_locale_page (dialog);
	construct_object_page (dialog);

	update_locale_page_from_prefs (dialog);
	update_object_page_from_prefs (dialog);

        gtk_widget_show_all (GTK_DIALOG (dialog)->vbox);   
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Response" callback.                                            */
/*---------------------------------------------------------------------------*/
static void
response_cb (glPrefsDialog *dialog,
	     gint          response,
	     gpointer      user_data)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail(dialog != NULL);
	g_return_if_fail(GTK_IS_DIALOG(dialog));

	switch(response) {
	case GTK_RESPONSE_CLOSE:
		gtk_widget_hide (GTK_WIDGET(dialog));
		break;
	case GTK_RESPONSE_DELETE_EVENT:
		break;
	default:
		g_print ("response = %d", response);
		g_assert_not_reached ();
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Build Locale Properties Notebook Tab                           */
/*--------------------------------------------------------------------------*/
static void
construct_locale_page (glPrefsDialog *dialog)
{

	dialog->priv->units_points_radio =
		glade_xml_get_widget (dialog->priv->gui, "units_points_radio");

	dialog->priv->units_inches_radio =
		glade_xml_get_widget (dialog->priv->gui, "units_inches_radio");

	dialog->priv->units_mm_radio =
		glade_xml_get_widget (dialog->priv->gui, "units_mm_radio");

	dialog->priv->page_size_us_letter_radio =
		glade_xml_get_widget (dialog->priv->gui, "page_size_us_letter_radio");

	dialog->priv->page_size_a4_radio =
		glade_xml_get_widget (dialog->priv->gui, "page_size_a4_radio");

	g_signal_connect_swapped (
		G_OBJECT(dialog->priv->units_points_radio),
		"toggled", G_CALLBACK(update_prefs_from_locale_page), G_OBJECT(dialog));
	g_signal_connect_swapped (
		G_OBJECT(dialog->priv->units_inches_radio),
		"toggled", G_CALLBACK(update_prefs_from_locale_page), G_OBJECT(dialog));
	g_signal_connect_swapped (
		G_OBJECT(dialog->priv->units_mm_radio),
		"toggled", G_CALLBACK(update_prefs_from_locale_page), G_OBJECT(dialog));
	g_signal_connect_swapped (
		G_OBJECT(dialog->priv->page_size_us_letter_radio),
		"toggled", G_CALLBACK(update_prefs_from_locale_page), G_OBJECT(dialog));
	g_signal_connect_swapped (
		G_OBJECT(dialog->priv->page_size_a4_radio),
		"toggled", G_CALLBACK(update_prefs_from_locale_page), G_OBJECT(dialog));
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Build Default Object Properties Notebook Tab                   */
/*--------------------------------------------------------------------------*/
static void
construct_object_page (glPrefsDialog *dialog)
{
        GList    *family_names;

	dialog->priv->text_family_combo =
		glade_xml_get_widget (dialog->priv->gui, "text_family_combo");
	dialog->priv->text_size_spin =
		glade_xml_get_widget (dialog->priv->gui, "text_size_spin");
	dialog->priv->text_bold_toggle =
		glade_xml_get_widget (dialog->priv->gui, "text_bold_toggle");
	dialog->priv->text_italic_toggle =
		glade_xml_get_widget (dialog->priv->gui, "text_italic_toggle");
	dialog->priv->text_color_combo =
		glade_xml_get_widget (dialog->priv->gui, "text_color_combo");
	dialog->priv->text_left_toggle =
		glade_xml_get_widget (dialog->priv->gui, "text_left_toggle");
	dialog->priv->text_center_toggle =
		glade_xml_get_widget (dialog->priv->gui, "text_center_toggle");
	dialog->priv->text_right_toggle =
		glade_xml_get_widget (dialog->priv->gui, "text_right_toggle");
	dialog->priv->text_line_spacing_spin =
		glade_xml_get_widget (dialog->priv->gui, "text_line_spacing_spin");

	dialog->priv->line_width_spin =
		glade_xml_get_widget (dialog->priv->gui, "line_width_spin");
	dialog->priv->line_color_combo =
		glade_xml_get_widget (dialog->priv->gui, "line_color_combo");

	dialog->priv->fill_color_combo =
		glade_xml_get_widget (dialog->priv->gui, "fill_color_combo");

	gl_util_combo_box_add_text_model (GTK_COMBO_BOX (dialog->priv->text_family_combo));

        /* Load family names */
        family_names = gl_util_get_font_family_list ();
	gl_util_combo_box_set_strings (GTK_COMBO_BOX (dialog->priv->text_family_combo),
				       family_names);
        gl_util_font_family_list_free (family_names);
                                                                                

	g_signal_connect_swapped (G_OBJECT(dialog->priv->text_family_combo),
				  "changed",
				  G_CALLBACK(update_prefs_from_object_page),
				  G_OBJECT(dialog));
	g_signal_connect_swapped (G_OBJECT(dialog->priv->text_size_spin),
				  "changed",
				  G_CALLBACK(update_prefs_from_object_page),
				  G_OBJECT(dialog));
	g_signal_connect_swapped (G_OBJECT(dialog->priv->text_bold_toggle),
				  "toggled",
				  G_CALLBACK(update_prefs_from_object_page),
				  G_OBJECT(dialog));
	g_signal_connect_swapped (G_OBJECT(dialog->priv->text_italic_toggle),
				  "toggled",
				  G_CALLBACK(update_prefs_from_object_page),
				  G_OBJECT(dialog));
	g_signal_connect_swapped (G_OBJECT(dialog->priv->text_color_combo),
				  "color_changed",
				  G_CALLBACK(update_prefs_from_object_page),
				  G_OBJECT(dialog));

	g_signal_connect (G_OBJECT(dialog->priv->text_left_toggle),
			  "toggled",
			  G_CALLBACK(align_toggle_cb),
			  G_OBJECT(dialog));
	g_signal_connect (G_OBJECT(dialog->priv->text_center_toggle),
			  "toggled",
			  G_CALLBACK(align_toggle_cb),
			  G_OBJECT(dialog));
	g_signal_connect (G_OBJECT(dialog->priv->text_right_toggle),
			  "toggled",
			  G_CALLBACK(align_toggle_cb),
			  G_OBJECT(dialog));

	g_signal_connect_swapped (G_OBJECT(dialog->priv->text_line_spacing_spin),
				  "changed",
				  G_CALLBACK(update_prefs_from_object_page),
				  G_OBJECT(dialog));

	g_signal_connect_swapped (G_OBJECT(dialog->priv->line_width_spin),
				  "changed",
				  G_CALLBACK(update_prefs_from_object_page),
				  G_OBJECT(dialog));
	g_signal_connect_swapped (G_OBJECT(dialog->priv->line_color_combo),
				  "color_changed",
				  G_CALLBACK(update_prefs_from_object_page),
				  G_OBJECT(dialog));

	g_signal_connect_swapped (G_OBJECT(dialog->priv->fill_color_combo),
				  "color_changed",
				  G_CALLBACK(update_prefs_from_object_page),
				  G_OBJECT(dialog));
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Alignment togglebutton callback.                               */
/*--------------------------------------------------------------------------*/
static void
align_toggle_cb (GtkToggleButton *toggle,
                 glPrefsDialog   *dialog)
{
        if (gtk_toggle_button_get_active (toggle))
        {
  
                if (GTK_WIDGET (toggle) == GTK_WIDGET (dialog->priv->text_left_toggle))
                {
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                                      (dialog->priv->text_center_toggle),
                                                      FALSE);
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                                      (dialog->priv->text_right_toggle),
                                                      FALSE);
                }
                else if (GTK_WIDGET (toggle) == GTK_WIDGET (dialog->priv->text_center_toggle))
                {
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                                      (dialog->priv->text_left_toggle),
                                                      FALSE);
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                                      (dialog->priv->text_right_toggle),
                                                      FALSE);
                }
                else if (GTK_WIDGET (toggle) == GTK_WIDGET (dialog->priv->text_right_toggle))
                {
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                                      (dialog->priv->text_left_toggle),
                                                      FALSE);
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                                      (dialog->priv->text_center_toggle),
                                                      FALSE);
                }
                                                                                
		update_prefs_from_object_page (dialog);
        }
                                                                                
}
                                                                                

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Update locale page widgets from current prefs.                 */
/*--------------------------------------------------------------------------*/
static void
update_locale_page_from_prefs (glPrefsDialog *dialog)
{
	dialog->priv->stop_signals = TRUE;

	switch (gl_prefs->units) {
	case LGL_UNITS_POINT:
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON(dialog->priv->units_points_radio),
			TRUE);
		break;
	case LGL_UNITS_INCH:
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON(dialog->priv->units_inches_radio),
			TRUE);
		break;
	case LGL_UNITS_MM:
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON(dialog->priv->units_mm_radio),
			TRUE);
		break;
	default:
		g_message ("Illegal units");	/* Should not happen */
		break;
	}

	if ( g_strcasecmp(gl_prefs->default_page_size, US_LETTER_ID) == 0)
        {
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON(dialog->priv->page_size_us_letter_radio), TRUE);
	}
        else if ( g_strcasecmp(gl_prefs->default_page_size, A4_ID) == 0)
        {
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON(dialog->priv->page_size_a4_radio), TRUE);
	}
        else
        {
		g_message ("Unknown default page size"); /* Shouldn't happen */
	}

	dialog->priv->stop_signals = FALSE;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Update object page widgets from current prefs.                 */
/*--------------------------------------------------------------------------*/
static void
update_object_page_from_prefs (glPrefsDialog *dialog)
{
        GList    *family_names;
        gchar    *good_font_family;
	GdkColor *gdk_color;
 
	dialog->priv->stop_signals = TRUE;


        /* Make sure we have a valid font family.  if not provide a good default. */
        family_names = gl_util_get_font_family_list ();
        if (g_list_find_custom (family_names,
				gl_prefs->default_font_family,
				(GCompareFunc)g_utf8_collate))
        {
                good_font_family = g_strdup (gl_prefs->default_font_family);
        }
        else
        {
                if (family_names != NULL)
                {
                        good_font_family = g_strdup (family_names->data); /* 1st entry */
                }
                else
                {
                        good_font_family = NULL;
                }
        }
        gl_util_font_family_list_free (family_names);
	gl_util_combo_box_set_active_text (GTK_COMBO_BOX (dialog->priv->text_family_combo),
					   good_font_family);
        g_free (good_font_family);

        gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->priv->text_size_spin),
                                   gl_prefs->default_font_size);
 
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->text_bold_toggle),
                                      (gl_prefs->default_font_weight == PANGO_WEIGHT_BOLD));
 
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->text_italic_toggle),
                                      gl_prefs->default_font_italic_flag);
 
        gdk_color = gl_color_to_gdk_color (gl_prefs->default_text_color);
        color_combo_set_color (COLOR_COMBO(dialog->priv->text_color_combo), gdk_color);
        g_free (gdk_color);

        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->text_left_toggle),
                                 (gl_prefs->default_text_alignment == GTK_JUSTIFY_LEFT));
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->text_center_toggle),
                                 (gl_prefs->default_text_alignment == GTK_JUSTIFY_CENTER));
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->text_right_toggle),
                                 (gl_prefs->default_text_alignment == GTK_JUSTIFY_RIGHT));

        gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->priv->text_line_spacing_spin),
                                   gl_prefs->default_text_line_spacing);

        gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->priv->line_width_spin),
                                   gl_prefs->default_line_width);
 
	gdk_color = gl_color_to_gdk_color (gl_prefs->default_line_color);
        color_combo_set_color (COLOR_COMBO(dialog->priv->line_color_combo), gdk_color);
        g_free (gdk_color);


	gdk_color = gl_color_to_gdk_color (gl_prefs->default_fill_color);
        color_combo_set_color (COLOR_COMBO(dialog->priv->fill_color_combo), gdk_color);
        g_free (gdk_color);


	dialog->priv->stop_signals = FALSE;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Update prefs from current state of locale page widgets.        */
/*--------------------------------------------------------------------------*/
static void
update_prefs_from_locale_page (glPrefsDialog *dialog)
{
	if (dialog->priv->stop_signals) return;

	if (gtk_toggle_button_get_active (
		    GTK_TOGGLE_BUTTON(dialog->priv->units_points_radio)))
        {
		gl_prefs->units = LGL_UNITS_POINT;
	}
	if (gtk_toggle_button_get_active (
		    GTK_TOGGLE_BUTTON(dialog->priv->units_inches_radio)))
        {
		gl_prefs->units = LGL_UNITS_INCH;
	}
	if (gtk_toggle_button_get_active (
		    GTK_TOGGLE_BUTTON(dialog->priv->units_mm_radio)))
        {
		gl_prefs->units = LGL_UNITS_MM;
	}

	if (gtk_toggle_button_get_active (
		    GTK_TOGGLE_BUTTON(dialog->priv->page_size_us_letter_radio)))
        {
		gl_prefs->default_page_size = US_LETTER_ID;
	}
	if (gtk_toggle_button_get_active (
		    GTK_TOGGLE_BUTTON(dialog->priv->page_size_a4_radio)))
        {
		gl_prefs->default_page_size = A4_ID;
	}

	gl_prefs_model_save_settings (gl_prefs);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Update prefs from current state of object page widgets.        */
/*--------------------------------------------------------------------------*/
static void
update_prefs_from_object_page (glPrefsDialog *dialog)
{
	GdkColor *gdk_color;
	gboolean  is_default;

	if (dialog->priv->stop_signals) return;

        g_free (gl_prefs->default_font_family);
        gl_prefs->default_font_family =
		gtk_combo_box_get_active_text (GTK_COMBO_BOX (dialog->priv->text_family_combo));
        gl_prefs->default_font_size =
                gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->text_size_spin));

        if (gtk_toggle_button_get_active
            (GTK_TOGGLE_BUTTON (dialog->priv->text_bold_toggle)))
        {
                gl_prefs->default_font_weight = PANGO_WEIGHT_BOLD;
        }
        else
        {
                gl_prefs->default_font_weight = PANGO_WEIGHT_NORMAL;
        }

        gl_prefs->default_font_italic_flag =
                gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
                                              (dialog->priv->text_italic_toggle));

        gdk_color = color_combo_get_color (COLOR_COMBO(dialog->priv->text_color_combo),
                                           &is_default);
        if (!is_default)
        {
                gl_prefs->default_text_color = gl_color_from_gdk_color (gdk_color);
        }

        if (gtk_toggle_button_get_active
            (GTK_TOGGLE_BUTTON (dialog->priv->text_left_toggle)))
        {
                gl_prefs->default_text_alignment = GTK_JUSTIFY_LEFT;
        }
        else if (gtk_toggle_button_get_active
                 (GTK_TOGGLE_BUTTON (dialog->priv->text_right_toggle)))
        {
                gl_prefs->default_text_alignment = GTK_JUSTIFY_RIGHT;
        }
        else if (gtk_toggle_button_get_active
                 (GTK_TOGGLE_BUTTON (dialog->priv->text_center_toggle)))
        {
                gl_prefs->default_text_alignment = GTK_JUSTIFY_CENTER;
        }
        else
        {
		/* Should not happen. */
                gl_prefs->default_text_alignment = GTK_JUSTIFY_LEFT;
        }
                                                                                

        gl_prefs->default_text_line_spacing =
                gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->text_line_spacing_spin));

        gl_prefs->default_line_width =
                gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->line_width_spin));

        gdk_color = color_combo_get_color (COLOR_COMBO(dialog->priv->line_color_combo),
                                           &is_default);
        if (!is_default)
        {
                gl_prefs->default_line_color = gl_color_from_gdk_color (gdk_color);
        }


        gdk_color = color_combo_get_color (COLOR_COMBO(dialog->priv->fill_color_combo),
                                           &is_default);
        if (!is_default)
        {
                gl_prefs->default_fill_color = gl_color_from_gdk_color (gdk_color);
        }

	gl_prefs_model_save_settings (gl_prefs);
}


	
