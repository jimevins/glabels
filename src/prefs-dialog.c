/*
 *  prefs-dialog.c
 *  Copyright (C) 2001-2009  Jim Evins <evins@snaught.com>.
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

#include "prefs-dialog.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "prefs.h"
#include "color-combo.h"
#include "color.h"
#include "font-combo.h"
#include "font-util.h"
#include "builder-util.h"

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
	GtkBuilder      *builder;

	/* Units properties */
	GtkWidget	*units_points_radio;
	GtkWidget	*units_inches_radio;
	GtkWidget	*units_mm_radio;

	/* Page size properties */
	GtkWidget	*page_size_us_letter_radio;
	GtkWidget	*page_size_a4_radio;

	/* Default text properties */
	GtkWidget       *text_family_hbox;
	GtkWidget       *text_family_combo;
	GtkWidget       *text_size_spin;
	GtkWidget       *text_bold_toggle;
	GtkWidget       *text_italic_toggle;
	GtkWidget       *text_color_hbox;
	GtkWidget       *text_color_combo;
	GtkWidget       *text_left_toggle;
	GtkWidget       *text_center_toggle;
	GtkWidget       *text_right_toggle;
	GtkWidget       *text_line_spacing_spin;

	/* Default line properties */
	GtkWidget       *line_width_spin;
	GtkWidget       *line_color_hbox;
	GtkWidget       *line_color_combo;

	/* Default fill properties */
	GtkWidget       *fill_color_hbox;
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
        gchar        *builder_filename;
        static gchar *object_ids[] = { "prefs_notebook",
                                       "adjustment1",  "adjustment2",  "adjustment3",
                                       NULL };
        GError *error = NULL;

	gl_debug (DEBUG_PREFS, "START");

	dialog->priv = g_new0 (glPrefsDialogPrivate, 1);

        dialog->priv->builder = gtk_builder_new ();
        builder_filename = g_build_filename (GLABELS_DATA_DIR, "ui", "prefs-dialog.ui", NULL);
        gtk_builder_add_objects_from_file (dialog->priv->builder, builder_filename, object_ids, &error);
        g_free (builder_filename);
	if (error) {
		g_critical ("%s\n\ngLabels may not be installed correctly!", error->message);
                g_error_free (error);
		return;
	}

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

	if (dialog->priv->builder)
        {
		g_object_unref (G_OBJECT (dialog->priv->builder));
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
        GtkWidget *vbox;
	GtkWidget *notebook;

	g_return_if_fail (GL_IS_PREFS_DIALOG (dialog));
	g_return_if_fail (dialog->priv != NULL);

        vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

        gl_builder_util_get_widgets (dialog->priv->builder,
                                     "prefs_notebook", &notebook,
                                     NULL);
	gtk_box_pack_start (GTK_BOX (vbox), notebook, FALSE, FALSE, 0);

	construct_locale_page (dialog);
	construct_object_page (dialog);

	update_locale_page_from_prefs (dialog);
	update_object_page_from_prefs (dialog);

        gtk_widget_show_all (vbox);   
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

        gl_builder_util_get_widgets (dialog->priv->builder,
                                     "units_points_radio",        &dialog->priv->units_points_radio,
                                     "units_inches_radio",        &dialog->priv->units_inches_radio,
                                     "units_mm_radio",            &dialog->priv->units_mm_radio,
                                     "page_size_us_letter_radio", &dialog->priv->page_size_us_letter_radio,
                                     "page_size_a4_radio",        &dialog->priv->page_size_a4_radio,
                                     NULL);

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
        gl_builder_util_get_widgets (dialog->priv->builder,
                                     "text_family_hbox",       &dialog->priv->text_family_hbox,
                                     "text_size_spin",         &dialog->priv->text_size_spin,
                                     "text_bold_toggle",       &dialog->priv->text_bold_toggle,
                                     "text_italic_toggle",     &dialog->priv->text_italic_toggle,
                                     "text_color_hbox",        &dialog->priv->text_color_hbox,
                                     "text_left_toggle",       &dialog->priv->text_left_toggle,
                                     "text_center_toggle",     &dialog->priv->text_center_toggle,
                                     "text_right_toggle",      &dialog->priv->text_right_toggle,
                                     "text_line_spacing_spin", &dialog->priv->text_line_spacing_spin,
                                     "line_width_spin",        &dialog->priv->line_width_spin,
                                     "line_color_hbox",        &dialog->priv->line_color_hbox,
                                     "fill_color_hbox",        &dialog->priv->fill_color_hbox,
                                     NULL);

        dialog->priv->text_family_combo = gl_font_combo_new ("Sans");

	dialog->priv->text_color_combo = gl_color_combo_new (_("Default"),
                                                             GL_COLOR_TEXT_DEFAULT,
                                                             gl_prefs_model_get_default_text_color (gl_prefs));
	dialog->priv->line_color_combo = gl_color_combo_new (_("No Line"),
                                                             GL_COLOR_NO_LINE,
                                                             gl_prefs_model_get_default_line_color (gl_prefs));
	dialog->priv->fill_color_combo = gl_color_combo_new (_("No Fill"),
                                                             GL_COLOR_NO_FILL,
                                                             gl_prefs_model_get_default_fill_color (gl_prefs));

        gtk_box_pack_start (GTK_BOX (dialog->priv->text_family_hbox),
                            dialog->priv->text_family_combo,
                            FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (dialog->priv->text_color_hbox),
                            dialog->priv->text_color_combo,
                            FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (dialog->priv->line_color_hbox),
                            dialog->priv->line_color_combo,
                            FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (dialog->priv->fill_color_hbox),
                            dialog->priv->fill_color_combo,
                            FALSE, FALSE, 0);

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

	switch (gl_prefs_model_get_units (gl_prefs)) {
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

	if ( g_ascii_strcasecmp(gl_prefs_model_get_default_page_size (gl_prefs), US_LETTER_ID) == 0)
        {
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON(dialog->priv->page_size_us_letter_radio), TRUE);
	}
        else if ( g_ascii_strcasecmp(gl_prefs_model_get_default_page_size (gl_prefs), A4_ID) == 0)
        {
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON(dialog->priv->page_size_a4_radio), TRUE);
	}
        else
        {
		g_message ("Unknown default page size"); /* Shouldn't happen */
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON(dialog->priv->page_size_a4_radio), TRUE);
	}

	dialog->priv->stop_signals = FALSE;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Update object page widgets from current prefs.                 */
/*--------------------------------------------------------------------------*/
static void
update_object_page_from_prefs (glPrefsDialog *dialog)
{
 
	dialog->priv->stop_signals = TRUE;

	gl_font_combo_set_family (GL_FONT_COMBO (dialog->priv->text_family_combo),
                                  gl_prefs_model_get_default_font_family (gl_prefs));

        gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->priv->text_size_spin),
                                   gl_prefs_model_get_default_font_size (gl_prefs));
 
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->text_bold_toggle),
                                      (gl_prefs_model_get_default_font_weight (gl_prefs) == PANGO_WEIGHT_BOLD));
 
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->text_italic_toggle),
                                      gl_prefs_model_get_default_font_italic_flag (gl_prefs));
 
        gl_color_combo_set_color (GL_COLOR_COMBO(dialog->priv->text_color_combo),
                                  gl_prefs_model_get_default_text_color (gl_prefs));

        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->text_left_toggle),
                                 (gl_prefs_model_get_default_text_alignment (gl_prefs) == GTK_JUSTIFY_LEFT));
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->text_center_toggle),
                                 (gl_prefs_model_get_default_text_alignment (gl_prefs) == GTK_JUSTIFY_CENTER));
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->text_right_toggle),
                                 (gl_prefs_model_get_default_text_alignment (gl_prefs) == GTK_JUSTIFY_RIGHT));

        gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->priv->text_line_spacing_spin),
                                   gl_prefs_model_get_default_text_line_spacing (gl_prefs));

        gtk_spin_button_set_value (GTK_SPIN_BUTTON (dialog->priv->line_width_spin),
                                   gl_prefs_model_get_default_line_width (gl_prefs));
 
        gl_color_combo_set_color (GL_COLOR_COMBO(dialog->priv->line_color_combo),
                                  gl_prefs_model_get_default_line_color (gl_prefs));


        gl_color_combo_set_color (GL_COLOR_COMBO(dialog->priv->fill_color_combo),
                                  gl_prefs_model_get_default_fill_color (gl_prefs));


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
		gl_prefs_model_set_units  (gl_prefs, LGL_UNITS_POINT);
	}
	if (gtk_toggle_button_get_active (
		    GTK_TOGGLE_BUTTON(dialog->priv->units_inches_radio)))
        {
		gl_prefs_model_set_units  (gl_prefs, LGL_UNITS_INCH);
	}
	if (gtk_toggle_button_get_active (
		    GTK_TOGGLE_BUTTON(dialog->priv->units_mm_radio)))
        {
		gl_prefs_model_set_units  (gl_prefs, LGL_UNITS_MM);
	}

	if (gtk_toggle_button_get_active (
		    GTK_TOGGLE_BUTTON(dialog->priv->page_size_us_letter_radio)))
        {
		gl_prefs_model_set_default_page_size  (gl_prefs, US_LETTER_ID);
	}
	if (gtk_toggle_button_get_active (
		    GTK_TOGGLE_BUTTON(dialog->priv->page_size_a4_radio)))
        {
		gl_prefs_model_set_default_page_size  (gl_prefs, A4_ID);
	}
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Update prefs from current state of object page widgets.        */
/*--------------------------------------------------------------------------*/
static void
update_prefs_from_object_page (glPrefsDialog *dialog)
{
	guint     color;
	gboolean  is_default;

	if (dialog->priv->stop_signals) return;

        gl_prefs_model_set_default_font_family (gl_prefs,
                gl_font_combo_get_family (GL_FONT_COMBO (dialog->priv->text_family_combo)));
        gl_prefs_model_set_default_font_size (gl_prefs,
                gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->text_size_spin)));

        if (gtk_toggle_button_get_active
            (GTK_TOGGLE_BUTTON (dialog->priv->text_bold_toggle)))
        {
                gl_prefs_model_set_default_font_weight (gl_prefs, PANGO_WEIGHT_BOLD);
        }
        else
        {
                gl_prefs_model_set_default_font_weight (gl_prefs, PANGO_WEIGHT_NORMAL);
        }

        gl_prefs_model_set_default_font_italic_flag (gl_prefs,
                gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
                                              (dialog->priv->text_italic_toggle)));

        color = gl_color_combo_get_color (GL_COLOR_COMBO(dialog->priv->text_color_combo),
                                          &is_default);
        if (!is_default)
        {
                gl_prefs_model_set_default_text_color (gl_prefs, color);
        }

        if (gtk_toggle_button_get_active
            (GTK_TOGGLE_BUTTON (dialog->priv->text_left_toggle)))
        {
                gl_prefs_model_set_default_text_alignment (gl_prefs, GTK_JUSTIFY_LEFT);
        }
        else if (gtk_toggle_button_get_active
                 (GTK_TOGGLE_BUTTON (dialog->priv->text_right_toggle)))
        {
                gl_prefs_model_set_default_text_alignment (gl_prefs, GTK_JUSTIFY_RIGHT);
        }
        else if (gtk_toggle_button_get_active
                 (GTK_TOGGLE_BUTTON (dialog->priv->text_center_toggle)))
        {
                gl_prefs_model_set_default_text_alignment (gl_prefs, GTK_JUSTIFY_CENTER);
        }
        else
        {
		/* Should not happen. */
                gl_prefs_model_set_default_text_alignment (gl_prefs, GTK_JUSTIFY_LEFT);
        }
                                                                                

        gl_prefs_model_set_default_text_line_spacing (gl_prefs,
                gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->text_line_spacing_spin)));

        gl_prefs_model_set_default_line_width (gl_prefs,
                gtk_spin_button_get_value (GTK_SPIN_BUTTON(dialog->priv->line_width_spin)));

        color = gl_color_combo_get_color (GL_COLOR_COMBO(dialog->priv->line_color_combo),
                                          &is_default);
        if (!is_default)
        {
                gl_prefs_model_set_default_line_color (gl_prefs, color);
        }


        color = gl_color_combo_get_color (GL_COLOR_COMBO(dialog->priv->fill_color_combo),
                                          &is_default);
        if (!is_default)
        {
                gl_prefs_model_set_default_fill_color (gl_prefs, color);
        }
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
