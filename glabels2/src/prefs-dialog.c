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

#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <glade/glade-xml.h>

#include "prefs-dialog.h"
#include "prefs.h"
#include "mygal/widget-color-combo.h"
#include "color.h"
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
	GtkWidget       *text_family_entry;
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
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static glHigDialogClass* parent_class = NULL;

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gl_prefs_dialog_class_init 	  (glPrefsDialogClass *klass);
static void gl_prefs_dialog_init       	  (glPrefsDialog      *dlg);
static void gl_prefs_dialog_finalize   	  (GObject            *object);
static void gl_prefs_dialog_construct     (glPrefsDialog      *dlg);

static void response_cb                   (glPrefsDialog      *dialog,
					   gint                response,
					   gpointer            user_data);

static void construct_locale_page         (glPrefsDialog      *dlg);
static void construct_object_page         (glPrefsDialog      *dlg);

static void align_toggle_cb               (GtkToggleButton    *toggle,
					   glPrefsDialog      *dlg);
                                                                                
static void update_locale_page_from_prefs (glPrefsDialog      *dlg);
static void update_object_page_from_prefs (glPrefsDialog      *dlg);

static void update_prefs_from_locale_page (glPrefsDialog      *dlg);
static void update_prefs_from_object_page (glPrefsDialog      *dlg);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_prefs_dialog_get_type (void)
{
	static GType type = 0;

	if (!type)
    	{
      		static const GTypeInfo info =
      		{
			sizeof (glPrefsDialogClass),
        		NULL,		/* base_init */
        		NULL,		/* base_finalize */
        		(GClassInitFunc) gl_prefs_dialog_class_init,
        		NULL,           /* class_finalize */
        		NULL,           /* class_data */
        		sizeof (glPrefsDialog),
        		0,              /* n_preallocs */
        		(GInstanceInitFunc) gl_prefs_dialog_init,
			NULL
      		};

     		type = g_type_register_static (GL_TYPE_HIG_DIALOG,
					       "glPrefsDialog", &info, 0);
    	}

	return type;
}

static void
gl_prefs_dialog_class_init (glPrefsDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	gl_debug (DEBUG_PREFS, "");
	
  	parent_class = g_type_class_peek_parent (klass);

  	object_class->finalize = gl_prefs_dialog_finalize;  	
}

static void
gl_prefs_dialog_init (glPrefsDialog *dlg)
{
	gl_debug (DEBUG_PREFS, "START");

	dlg->priv = g_new0 (glPrefsDialogPrivate, 1);

	dlg->priv->gui = glade_xml_new (GLABELS_GLADE_DIR "prefs-dialog.glade",
					"prefs_notebook",
					NULL);

	if (!dlg->priv->gui) {
		g_warning ("Could not open prefs-dialog.glade, reinstall glabels!");
		return;
	}

	gl_debug (DEBUG_PREFS, "END");
}

static void 
gl_prefs_dialog_finalize (GObject *object)
{
	glPrefsDialog* dlg;
	
	gl_debug (DEBUG_PREFS, "START");

	g_return_if_fail (object != NULL);
	
   	dlg = GL_PREFS_DIALOG (object);

	g_return_if_fail (GL_IS_PREFS_DIALOG (dlg));
	g_return_if_fail (dlg->priv != NULL);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	g_free (dlg->priv);

	gl_debug (DEBUG_PREFS, "END");
}

/*****************************************************************************/
/* NEW preferences dialog.                                                   */
/*****************************************************************************/
GtkWidget*
gl_prefs_dialog_new (GtkWindow *parent)
{
	GtkWidget *dlg;

	gl_debug (DEBUG_PREFS, "START");
	gl_debug (DEBUG_PREFS, "page size = \"%s\"", gl_prefs->default_page_size);

	dlg = GTK_WIDGET (g_object_new (GL_TYPE_PREFS_DIALOG, NULL));

	if (parent)
		gtk_window_set_transient_for (GTK_WINDOW (dlg), parent);
	
	gl_prefs_dialog_construct (GL_PREFS_DIALOG(dlg));


	gl_debug (DEBUG_PREFS, "END");

	return dlg;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                     */
/*---------------------------------------------------------------------------*/
static void
gl_prefs_dialog_construct (glPrefsDialog *dlg)
{
	GtkWidget *notebook;

	g_return_if_fail (GL_IS_PREFS_DIALOG (dlg));
	g_return_if_fail (dlg->priv != NULL);

	gtk_dialog_add_button (GTK_DIALOG(dlg),
			       GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);

	gtk_dialog_set_default_response (GTK_DIALOG (dlg), GTK_RESPONSE_CLOSE);

	g_signal_connect(G_OBJECT (dlg), "response",
			 G_CALLBACK (response_cb), NULL);

	notebook = glade_xml_get_widget (dlg->priv->gui, "prefs_notebook");
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dlg), notebook);

	construct_locale_page (dlg);
	construct_object_page (dlg);

	update_locale_page_from_prefs (dlg);
	update_object_page_from_prefs (dlg);

        gtk_widget_show_all (GTK_DIALOG (dlg)->vbox);   

        gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);
        gtk_window_set_title (GTK_WINDOW (dlg), _("gLabels Preferences"));
        gtk_window_set_resizable (GTK_WINDOW (dlg), FALSE);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Response" callback.                                            */
/*---------------------------------------------------------------------------*/
static void
response_cb (glPrefsDialog *dlg,
	     gint          response,
	     gpointer      user_data)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail(dlg != NULL);
	g_return_if_fail(GTK_IS_DIALOG(dlg));

	switch(response) {
	case GTK_RESPONSE_CLOSE:
		gtk_widget_hide (GTK_WIDGET(dlg));
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
construct_locale_page (glPrefsDialog *dlg)
{

	dlg->priv->units_points_radio =
		glade_xml_get_widget (dlg->priv->gui, "units_points_radio");

	dlg->priv->units_inches_radio =
		glade_xml_get_widget (dlg->priv->gui, "units_inches_radio");

	dlg->priv->units_mm_radio =
		glade_xml_get_widget (dlg->priv->gui, "units_mm_radio");

	dlg->priv->page_size_us_letter_radio =
		glade_xml_get_widget (dlg->priv->gui, "page_size_us_letter_radio");

	dlg->priv->page_size_a4_radio =
		glade_xml_get_widget (dlg->priv->gui, "page_size_a4_radio");

	g_signal_connect_swapped (
		G_OBJECT(dlg->priv->units_points_radio),
		"toggled", G_CALLBACK(update_prefs_from_locale_page), G_OBJECT(dlg));
	g_signal_connect_swapped (
		G_OBJECT(dlg->priv->units_inches_radio),
		"toggled", G_CALLBACK(update_prefs_from_locale_page), G_OBJECT(dlg));
	g_signal_connect_swapped (
		G_OBJECT(dlg->priv->units_mm_radio),
		"toggled", G_CALLBACK(update_prefs_from_locale_page), G_OBJECT(dlg));
	g_signal_connect_swapped (
		G_OBJECT(dlg->priv->page_size_us_letter_radio),
		"toggled", G_CALLBACK(update_prefs_from_locale_page), G_OBJECT(dlg));
	g_signal_connect_swapped (
		G_OBJECT(dlg->priv->page_size_a4_radio),
		"toggled", G_CALLBACK(update_prefs_from_locale_page), G_OBJECT(dlg));
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Build Default Object Properties Notebook Tab                   */
/*--------------------------------------------------------------------------*/
static void
construct_object_page (glPrefsDialog *dlg)
{
        GList    *family_names;

	dlg->priv->text_family_entry =
		glade_xml_get_widget (dlg->priv->gui, "text_family_entry");
	dlg->priv->text_family_combo =
		glade_xml_get_widget (dlg->priv->gui, "text_family_combo");
	dlg->priv->text_size_spin =
		glade_xml_get_widget (dlg->priv->gui, "text_size_spin");
	dlg->priv->text_bold_toggle =
		glade_xml_get_widget (dlg->priv->gui, "text_bold_toggle");
	dlg->priv->text_italic_toggle =
		glade_xml_get_widget (dlg->priv->gui, "text_italic_toggle");
	dlg->priv->text_color_combo =
		glade_xml_get_widget (dlg->priv->gui, "text_color_combo");
	dlg->priv->text_left_toggle =
		glade_xml_get_widget (dlg->priv->gui, "text_left_toggle");
	dlg->priv->text_center_toggle =
		glade_xml_get_widget (dlg->priv->gui, "text_center_toggle");
	dlg->priv->text_right_toggle =
		glade_xml_get_widget (dlg->priv->gui, "text_right_toggle");
	dlg->priv->text_line_spacing_spin =
		glade_xml_get_widget (dlg->priv->gui, "text_line_spacing_spin");

	dlg->priv->line_width_spin =
		glade_xml_get_widget (dlg->priv->gui, "line_width_spin");
	dlg->priv->line_color_combo =
		glade_xml_get_widget (dlg->priv->gui, "line_color_combo");

	dlg->priv->fill_color_combo =
		glade_xml_get_widget (dlg->priv->gui, "fill_color_combo");

        /* Load family names */
        family_names = gnome_font_family_list ();
        gtk_combo_set_popdown_strings (GTK_COMBO(dlg->priv->text_family_combo),
				       family_names);
        gnome_font_family_list_free (family_names);
                                                                                

	g_signal_connect_swapped (G_OBJECT(dlg->priv->text_family_entry),
				  "changed",
				  G_CALLBACK(update_prefs_from_object_page),
				  G_OBJECT(dlg));
	g_signal_connect_swapped (G_OBJECT(dlg->priv->text_size_spin),
				  "changed",
				  G_CALLBACK(update_prefs_from_object_page),
				  G_OBJECT(dlg));
	g_signal_connect_swapped (G_OBJECT(dlg->priv->text_bold_toggle),
				  "toggled",
				  G_CALLBACK(update_prefs_from_object_page),
				  G_OBJECT(dlg));
	g_signal_connect_swapped (G_OBJECT(dlg->priv->text_italic_toggle),
				  "toggled",
				  G_CALLBACK(update_prefs_from_object_page),
				  G_OBJECT(dlg));
	g_signal_connect_swapped (G_OBJECT(dlg->priv->text_color_combo),
				  "color_changed",
				  G_CALLBACK(update_prefs_from_object_page),
				  G_OBJECT(dlg));

	g_signal_connect (G_OBJECT(dlg->priv->text_left_toggle),
			  "toggled",
			  G_CALLBACK(align_toggle_cb),
			  G_OBJECT(dlg));
	g_signal_connect (G_OBJECT(dlg->priv->text_center_toggle),
			  "toggled",
			  G_CALLBACK(align_toggle_cb),
			  G_OBJECT(dlg));
	g_signal_connect (G_OBJECT(dlg->priv->text_right_toggle),
			  "toggled",
			  G_CALLBACK(align_toggle_cb),
			  G_OBJECT(dlg));

	g_signal_connect_swapped (G_OBJECT(dlg->priv->text_line_spacing_spin),
				  "changed",
				  G_CALLBACK(update_prefs_from_object_page),
				  G_OBJECT(dlg));

	g_signal_connect_swapped (G_OBJECT(dlg->priv->line_width_spin),
				  "changed",
				  G_CALLBACK(update_prefs_from_object_page),
				  G_OBJECT(dlg));
	g_signal_connect_swapped (G_OBJECT(dlg->priv->line_color_combo),
				  "color_changed",
				  G_CALLBACK(update_prefs_from_object_page),
				  G_OBJECT(dlg));

	g_signal_connect_swapped (G_OBJECT(dlg->priv->fill_color_combo),
				  "color_changed",
				  G_CALLBACK(update_prefs_from_object_page),
				  G_OBJECT(dlg));
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Alignment togglebutton callback.                               */
/*--------------------------------------------------------------------------*/
static void
align_toggle_cb (GtkToggleButton *toggle,
                 glPrefsDialog   *dlg)
{
        if (gtk_toggle_button_get_active (toggle)) {
  
                if (GTK_WIDGET (toggle) == GTK_WIDGET (dlg->priv->text_left_toggle)) {
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                                      (dlg->priv->text_center_toggle),
                                                      FALSE);
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                                      (dlg->priv->text_right_toggle),
                                                      FALSE);
                } else if (GTK_WIDGET (toggle) ==
                           GTK_WIDGET (dlg->priv->text_center_toggle)) {
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                                      (dlg->priv->text_left_toggle),
                                                      FALSE);
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                                      (dlg->priv->text_right_toggle),
                                                      FALSE);
                } else if (GTK_WIDGET (toggle) ==
                           GTK_WIDGET (dlg->priv->text_right_toggle)) {
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                                      (dlg->priv->text_left_toggle),
                                                      FALSE);
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                                      (dlg->priv->text_center_toggle),
                                                      FALSE);
                }
                                                                                
		update_prefs_from_object_page (dlg);
        }
                                                                                
}
                                                                                

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Update locale page widgets from current prefs.                 */
/*--------------------------------------------------------------------------*/
static void
update_locale_page_from_prefs (glPrefsDialog *dlg)
{
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->priv->units_points_radio),
		G_CALLBACK(update_prefs_from_locale_page), G_OBJECT(dlg));
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->priv->units_inches_radio),
		G_CALLBACK(update_prefs_from_locale_page), G_OBJECT(dlg));
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->priv->units_mm_radio),
		G_CALLBACK(update_prefs_from_locale_page), G_OBJECT(dlg));
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->priv->page_size_us_letter_radio),
		G_CALLBACK(update_prefs_from_locale_page), G_OBJECT(dlg));
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->priv->page_size_a4_radio),
		G_CALLBACK(update_prefs_from_locale_page), G_OBJECT(dlg));

	switch (gl_prefs->units) {
	case GL_UNITS_POINT:
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON(dlg->priv->units_points_radio),
			TRUE);
		break;
	case GL_UNITS_INCH:
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON(dlg->priv->units_inches_radio),
			TRUE);
		break;
	case GL_UNITS_MM:
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON(dlg->priv->units_mm_radio),
			TRUE);
		break;
	default:
		g_warning ("Illegal units");	/* Should not happen */
		break;
	}

	if ( g_strcasecmp(gl_prefs->default_page_size, US_LETTER_ID) == 0) {
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON(dlg->priv->page_size_us_letter_radio),
			TRUE);
	} else if ( g_strcasecmp(gl_prefs->default_page_size, A4_ID) == 0) {
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON(dlg->priv->page_size_a4_radio),
			TRUE);
	} else {
		g_warning ("Unknown default page size"); /* Shouldn't happen */
	}

	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->priv->units_points_radio),
		G_CALLBACK(update_prefs_from_locale_page), G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->priv->units_inches_radio),
		G_CALLBACK(update_prefs_from_locale_page), G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->priv->units_mm_radio),
		G_CALLBACK(update_prefs_from_locale_page), G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->priv->page_size_us_letter_radio),
		G_CALLBACK(update_prefs_from_locale_page), G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->priv->page_size_a4_radio),
		G_CALLBACK(update_prefs_from_locale_page), G_OBJECT(dlg));
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Update object page widgets from current prefs.                 */
/*--------------------------------------------------------------------------*/
static void
update_object_page_from_prefs (glPrefsDialog *dlg)
{
        GList    *family_names;
        gchar    *good_font_family;
	GdkColor *gdk_color;
 
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->priv->text_family_entry),
		G_CALLBACK(update_prefs_from_object_page), G_OBJECT(dlg));
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->priv->text_size_spin),
		G_CALLBACK(update_prefs_from_object_page), G_OBJECT(dlg));
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->priv->text_bold_toggle),
		G_CALLBACK(update_prefs_from_object_page), G_OBJECT(dlg));
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->priv->text_italic_toggle),
		G_CALLBACK(update_prefs_from_object_page), G_OBJECT(dlg));
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->priv->text_color_combo),
		G_CALLBACK(update_prefs_from_object_page), G_OBJECT(dlg));
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->priv->text_left_toggle),
		G_CALLBACK(align_toggle_cb), G_OBJECT(dlg));
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->priv->text_center_toggle),
		G_CALLBACK(align_toggle_cb), G_OBJECT(dlg));
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->priv->text_right_toggle),
		G_CALLBACK(align_toggle_cb), G_OBJECT(dlg));
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->priv->text_line_spacing_spin),
		G_CALLBACK(update_prefs_from_object_page), G_OBJECT(dlg));
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->priv->line_width_spin),
		G_CALLBACK(update_prefs_from_object_page), G_OBJECT(dlg));
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->priv->line_color_combo),
		G_CALLBACK(update_prefs_from_object_page), G_OBJECT(dlg));
	g_signal_handlers_block_by_func (
		G_OBJECT(dlg->priv->fill_color_combo),
		G_CALLBACK(update_prefs_from_object_page), G_OBJECT(dlg));


        /* Make sure we have a valid font family.  if not provide a good default. */
        family_names = gnome_font_family_list ();
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
        gnome_font_family_list_free (family_names);
        gtk_entry_set_text (GTK_ENTRY (dlg->priv->text_family_entry), good_font_family);
        g_free (good_font_family);

        gtk_spin_button_set_value (GTK_SPIN_BUTTON (dlg->priv->text_size_spin),
                                   gl_prefs->default_font_size);
 
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->text_bold_toggle),
                                      (gl_prefs->default_font_weight == GNOME_FONT_BOLD));
 
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->text_italic_toggle),
                                      gl_prefs->default_font_italic_flag);
 
        gdk_color = gl_color_to_gdk_color (gl_prefs->default_text_color);
        color_combo_set_color (COLOR_COMBO(dlg->priv->text_color_combo), gdk_color);
        g_free (gdk_color);

        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->text_left_toggle),
                                 (gl_prefs->default_text_alignment == GTK_JUSTIFY_LEFT));
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->text_center_toggle),
                                 (gl_prefs->default_text_alignment == GTK_JUSTIFY_CENTER));
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dlg->priv->text_right_toggle),
                                 (gl_prefs->default_text_alignment == GTK_JUSTIFY_RIGHT));

        gtk_spin_button_set_value (GTK_SPIN_BUTTON (dlg->priv->text_line_spacing_spin),
                                   gl_prefs->default_text_line_spacing);

        gtk_spin_button_set_value (GTK_SPIN_BUTTON (dlg->priv->line_width_spin),
                                   gl_prefs->default_line_width);
 
	gdk_color = gl_color_to_gdk_color (gl_prefs->default_line_color);
        color_combo_set_color (COLOR_COMBO(dlg->priv->line_color_combo), gdk_color);
        g_free (gdk_color);


	gdk_color = gl_color_to_gdk_color (gl_prefs->default_fill_color);
        color_combo_set_color (COLOR_COMBO(dlg->priv->fill_color_combo), gdk_color);
        g_free (gdk_color);


	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->priv->text_family_entry),
		G_CALLBACK(update_prefs_from_object_page), G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->priv->text_size_spin),
		G_CALLBACK(update_prefs_from_object_page), G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->priv->text_bold_toggle),
		G_CALLBACK(update_prefs_from_object_page), G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->priv->text_italic_toggle),
		G_CALLBACK(update_prefs_from_object_page), G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->priv->text_color_combo),
		G_CALLBACK(update_prefs_from_object_page), G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->priv->text_left_toggle),
		G_CALLBACK(align_toggle_cb), G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->priv->text_center_toggle),
		G_CALLBACK(align_toggle_cb), G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->priv->text_right_toggle),
		G_CALLBACK(align_toggle_cb), G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->priv->text_line_spacing_spin),
		G_CALLBACK(update_prefs_from_object_page), G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->priv->line_width_spin),
		G_CALLBACK(update_prefs_from_object_page), G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->priv->line_color_combo),
		G_CALLBACK(update_prefs_from_object_page), G_OBJECT(dlg));
	g_signal_handlers_unblock_by_func (
		G_OBJECT(dlg->priv->fill_color_combo),
		G_CALLBACK(update_prefs_from_object_page), G_OBJECT(dlg));
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Update prefs from current state of locale page widgets.        */
/*--------------------------------------------------------------------------*/
static void
update_prefs_from_locale_page (glPrefsDialog *dlg)
{
	if (gtk_toggle_button_get_active (
		    GTK_TOGGLE_BUTTON(dlg->priv->units_points_radio))) {
		gl_prefs->units = GL_UNITS_POINT;
	}
	if (gtk_toggle_button_get_active (
		    GTK_TOGGLE_BUTTON(dlg->priv->units_inches_radio))) {
		gl_prefs->units = GL_UNITS_INCH;
	}
	if (gtk_toggle_button_get_active (
		    GTK_TOGGLE_BUTTON(dlg->priv->units_mm_radio))) {
		gl_prefs->units = GL_UNITS_MM;
	}

	if (gtk_toggle_button_get_active (
		    GTK_TOGGLE_BUTTON(dlg->priv->page_size_us_letter_radio))) {
		gl_prefs->default_page_size = US_LETTER_ID;
	}
	if (gtk_toggle_button_get_active (
		    GTK_TOGGLE_BUTTON(dlg->priv->page_size_a4_radio))) {
		gl_prefs->default_page_size = A4_ID;
	}

	gl_prefs_model_save_settings (gl_prefs);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Update prefs from current state of object page widgets.        */
/*--------------------------------------------------------------------------*/
static void
update_prefs_from_object_page (glPrefsDialog *dlg)
{
	GdkColor *gdk_color;
	gboolean  is_default;


        g_free (gl_prefs->default_font_family);
        gl_prefs->default_font_family =
                gtk_editable_get_chars (GTK_EDITABLE (dlg->priv->text_family_entry),
					0, -1);
        gl_prefs->default_font_size =
                gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->text_size_spin));

        if (gtk_toggle_button_get_active
            (GTK_TOGGLE_BUTTON (dlg->priv->text_bold_toggle))) {
                gl_prefs->default_font_weight = GNOME_FONT_BOLD;
        } else {
                gl_prefs->default_font_weight = GNOME_FONT_BOOK;
        }

        gl_prefs->default_font_italic_flag =
                gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
                                              (dlg->priv->text_italic_toggle));

        gdk_color = color_combo_get_color (COLOR_COMBO(dlg->priv->text_color_combo),
                                           &is_default);
        if (!is_default) {
                gl_prefs->default_text_color = gl_color_from_gdk_color (gdk_color);
        }

        if (gtk_toggle_button_get_active
            (GTK_TOGGLE_BUTTON (dlg->priv->text_left_toggle))) {
                gl_prefs->default_text_alignment = GTK_JUSTIFY_LEFT;
        } else
            if (gtk_toggle_button_get_active
                (GTK_TOGGLE_BUTTON (dlg->priv->text_right_toggle))) {
                gl_prefs->default_text_alignment = GTK_JUSTIFY_RIGHT;
        } else
            if (gtk_toggle_button_get_active
                (GTK_TOGGLE_BUTTON (dlg->priv->text_center_toggle))) {
                gl_prefs->default_text_alignment = GTK_JUSTIFY_CENTER;
        } else {
		/* Should not happen. */
                gl_prefs->default_text_alignment = GTK_JUSTIFY_LEFT;
        }
                                                                                

        gl_prefs->default_text_line_spacing =
                gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->text_line_spacing_spin));

        gl_prefs->default_line_width =
                gtk_spin_button_get_value (GTK_SPIN_BUTTON(dlg->priv->line_width_spin));

        gdk_color = color_combo_get_color (COLOR_COMBO(dlg->priv->line_color_combo),
                                           &is_default);
        if (!is_default) {
                gl_prefs->default_line_color = gl_color_from_gdk_color (gdk_color);
        }


        gdk_color = color_combo_get_color (COLOR_COMBO(dlg->priv->fill_color_combo),
                                           &is_default);
        if (!is_default) {
                gl_prefs->default_fill_color = gl_color_from_gdk_color (gdk_color);
        }

	gl_prefs_model_save_settings (gl_prefs);
}


	
