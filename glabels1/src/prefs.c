/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prefs.c:  Application preferences module
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

#include "mdi.h"
#include "propertybox.h"
#include "prefs.h"

#include "debug.h"

/*============================================*/
/* Private types.                             */
/*============================================*/

typedef struct {
	glPrefsUnits default_units;
	gchar *default_page_size;
} Prefs;

/*============================================*/
/* Private globals.                           */
/*============================================*/
static GtkWidget *dialog = NULL;

static Prefs *prefs = NULL;

static GtkWidget *unit_pts_radio, *unit_inches_radio, *unit_mm_radio;
static GtkWidget *page_size_us_letter_radio, *page_size_a4_radio;

/*============================================*/
/* Private function prototypes.               */
/*============================================*/
static void apply_cb (glPropertyBox * propertybox, gint arg1, gpointer user_data);

static void prefs_destroy_cb (void);

static void prefs_changed_cb (void);

static Prefs *read_prefs (void);

static void sync_prefs (Prefs * p);

static void free_prefs (Prefs * p);

static void add_general_page (GtkWidget * dialog);

static void set_general_page_from_prefs (Prefs * p);

static void set_prefs_from_general_page (Prefs * p);

/****************************************************************************/
/* Create and display preferences dialog.                                   */
/****************************************************************************/
void
gl_prefs_cb (GtkWidget * widget,
	     gpointer data)
{
	GnomeApp *app = gnome_mdi_get_active_window (GNOME_MDI (data));

	if (prefs == NULL) {
		prefs = read_prefs ();
	}

	if (dialog == NULL) {
		dialog = gl_property_box_new ();
		gtk_window_set_title (GTK_WINDOW (dialog),
				      _("Glabels: Preferences"));
		gtk_window_set_transient_for (GTK_WINDOW (dialog),
					      GTK_WINDOW (app));

		add_general_page (dialog);

		gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
				    GTK_SIGNAL_FUNC (prefs_destroy_cb), NULL);

		gtk_signal_connect (GTK_OBJECT (dialog), "delete_event",
				    GTK_SIGNAL_FUNC (prefs_destroy_cb), NULL);

		gtk_signal_connect (GTK_OBJECT (dialog), "apply",
				    GTK_SIGNAL_FUNC (apply_cb), NULL);

		gtk_widget_show_all (GTK_WIDGET (dialog));
	}
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  "Apply" button callback.                                       */
/*--------------------------------------------------------------------------*/
static void
apply_cb (glPropertyBox * propertybox,
	  gint arg1,
	  gpointer user_data)
{
	set_prefs_from_general_page (prefs);
	sync_prefs (prefs);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback to finish cleaning up after tearing down dialog.      */
/*--------------------------------------------------------------------------*/
static void
prefs_destroy_cb (void)
{
	dialog = NULL;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the dialog has changed.       */
/*--------------------------------------------------------------------------*/
static void
prefs_changed_cb (void)
{
	gl_property_box_changed (GL_PROPERTY_BOX (dialog));
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Read and set preferences from disk.                            */
/*--------------------------------------------------------------------------*/
static Prefs *
read_prefs (void)
{
	Prefs *p;
	gchar *string;
	gboolean dflt;

	p = g_new0 (Prefs, 1);

	gnome_config_push_prefix ("/glabels/General/");

	string = gnome_config_get_string_with_default ("Units=Inches", &dflt);
	if (dflt)
		gnome_config_set_string ("Units", "Inches");
	if (g_strcasecmp (string, "Points") == 0) {
		p->default_units = GL_PREFS_UNITS_PTS;
	} else if (g_strcasecmp (string, "Inches") == 0) {
		p->default_units = GL_PREFS_UNITS_INCHES;
	} else if (g_strcasecmp (string, "Millimeters") == 0) {
		p->default_units = GL_PREFS_UNITS_MM;
	}
	g_free (string);

	p->default_page_size =
	    gnome_config_get_string_with_default ("Page_size=US-Letter", &dflt);
	if (dflt)
		gnome_config_set_string ("Page_size", "US-Letter");

	gnome_config_pop_prefix ();

	return p;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE. Sync preferences with disk.                                     */
/*--------------------------------------------------------------------------*/
static void
sync_prefs (Prefs * p)
{
	gnome_config_push_prefix ("/glabels/General/");
	switch (p->default_units) {
	case GL_PREFS_UNITS_PTS:
		gnome_config_set_string ("Units", "Points");
		break;
	case GL_PREFS_UNITS_INCHES:
		gnome_config_set_string ("Units", "Inches");
		break;
	case GL_PREFS_UNITS_MM:
		gnome_config_set_string ("Units", "Millimeters");
		break;
	default:
		WARN ("Illegal units");
		break;
	}
	gnome_config_set_string ("Page_size", p->default_page_size);
	gnome_config_pop_prefix ();
	gnome_config_sync ();
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Free previously allocated preferences structure.               */
/*--------------------------------------------------------------------------*/
static void
free_prefs (Prefs * p)
{
	g_free (p);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE. Create and add the general page tab with controls to the dialog.*/
/*--------------------------------------------------------------------------*/
static void
add_general_page (GtkWidget * dialog)
{
	GtkWidget *wframe, *wlabel, *wvbox, *wvbox1;
	GSList *radio_group = NULL;

	wlabel = gtk_label_new (_("General"));

	wvbox = gtk_vbox_new (FALSE, 0);

	/* ----- Display Units Frame ------------------------------------ */
	wframe = gtk_frame_new (_("Display units"));
	gtk_container_set_border_width (GTK_CONTAINER (wframe), 10);
	gtk_box_pack_start (GTK_BOX (wvbox), wframe, FALSE, FALSE, 0);

	wvbox1 = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox1), 10);
	gtk_container_add (GTK_CONTAINER (wframe), wvbox1);

	radio_group = NULL;

	unit_pts_radio =
	    gtk_radio_button_new_with_label (radio_group, _("Points"));
	radio_group =
	    gtk_radio_button_group (GTK_RADIO_BUTTON (unit_pts_radio));
	gtk_box_pack_start (GTK_BOX (wvbox1), unit_pts_radio, FALSE, FALSE, 0);

	unit_inches_radio =
	    gtk_radio_button_new_with_label (radio_group, _("Inches"));
	radio_group =
	    gtk_radio_button_group (GTK_RADIO_BUTTON (unit_inches_radio));
	gtk_box_pack_start (GTK_BOX (wvbox1), unit_inches_radio, FALSE, FALSE,
			    0);

	unit_mm_radio =
	    gtk_radio_button_new_with_label (radio_group, _("Millimeters"));
	radio_group = gtk_radio_button_group (GTK_RADIO_BUTTON (unit_mm_radio));
	gtk_box_pack_start (GTK_BOX (wvbox1), unit_mm_radio, FALSE, FALSE, 0);

	/* ----- Display Units Frame ------------------------------------ */
	wframe = gtk_frame_new (_("Default page size"));
	gtk_container_set_border_width (GTK_CONTAINER (wframe), 10);
	gtk_box_pack_start (GTK_BOX (wvbox), wframe, FALSE, FALSE, 0);

	wvbox1 = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox1), 10);
	gtk_container_add (GTK_CONTAINER (wframe), wvbox1);

	radio_group = NULL;

	page_size_us_letter_radio =
	    gtk_radio_button_new_with_label (radio_group, "US-Letter");
	radio_group =
	    gtk_radio_button_group (GTK_RADIO_BUTTON
				    (page_size_us_letter_radio));
	gtk_box_pack_start (GTK_BOX (wvbox1), page_size_us_letter_radio, FALSE,
			    FALSE, 0);

	page_size_a4_radio =
	    gtk_radio_button_new_with_label (radio_group, "A4");
	radio_group =
	    gtk_radio_button_group (GTK_RADIO_BUTTON (page_size_a4_radio));
	gtk_box_pack_start (GTK_BOX (wvbox1), page_size_a4_radio,
			    FALSE, FALSE, 0);

	gl_property_box_append_page (GL_PROPERTY_BOX (dialog), wvbox, wlabel);

	set_general_page_from_prefs (prefs);

	gtk_signal_connect (GTK_OBJECT (unit_pts_radio), "toggled",
			    GTK_SIGNAL_FUNC (prefs_changed_cb), NULL);
	gtk_signal_connect (GTK_OBJECT (unit_inches_radio), "toggled",
			    GTK_SIGNAL_FUNC (prefs_changed_cb), NULL);
	gtk_signal_connect (GTK_OBJECT (unit_mm_radio), "toggled",
			    GTK_SIGNAL_FUNC (prefs_changed_cb), NULL);

	gtk_signal_connect (GTK_OBJECT (page_size_us_letter_radio), "toggled",
			    GTK_SIGNAL_FUNC (prefs_changed_cb), NULL);
	gtk_signal_connect (GTK_OBJECT (page_size_a4_radio), "toggled",
			    GTK_SIGNAL_FUNC (prefs_changed_cb), NULL);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Initialize general tab controls from current prefs.            */
/*--------------------------------------------------------------------------*/
static void
set_general_page_from_prefs (Prefs * p)
{
	switch (p->default_units) {
	case GL_PREFS_UNITS_PTS:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (unit_pts_radio), TRUE);
		break;
	case GL_PREFS_UNITS_INCHES:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (unit_inches_radio), TRUE);
		break;
	case GL_PREFS_UNITS_MM:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (unit_mm_radio),
					      TRUE);
		break;
	default:
		WARN ("Illegal units");
		break;
	}

	if (g_strcasecmp (p->default_page_size, "US-Letter") == 0) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (page_size_us_letter_radio),
					      TRUE);
	} else if (g_strcasecmp (p->default_page_size, "A4") == 0) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (page_size_a4_radio), TRUE);
	} else {
		WARN ("Illegal page size: %s", p->default_page_size);
	}
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Set current preferences based on general controls.             */
/*--------------------------------------------------------------------------*/
static void
set_prefs_from_general_page (Prefs * p)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (unit_pts_radio))) {
		p->default_units = GL_PREFS_UNITS_PTS;
	} else
	    if (gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (unit_inches_radio))) {
		p->default_units = GL_PREFS_UNITS_INCHES;
	} else
	    if (gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (unit_mm_radio))) {
		p->default_units = GL_PREFS_UNITS_MM;
	}

	if (gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON (page_size_us_letter_radio))) {
		g_free (p->default_page_size);
		p->default_page_size = g_strdup ("US-Letter");
	} else
	    if (gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (page_size_a4_radio))) {
		g_free (p->default_page_size);
		p->default_page_size = g_strdup ("A4");
	}

}

/****************************************************************************/
/* Get desired units.                                                       */
/****************************************************************************/
glPrefsUnits
gl_prefs_get_units(void)
{
	return prefs->default_units;
}

/****************************************************************************/
/* Get desired units per point.                                             */
/****************************************************************************/
gdouble
gl_prefs_get_units_per_point (void)
{
	if (prefs == NULL) {
		prefs = read_prefs ();
	}

	switch (prefs->default_units) {
	case GL_PREFS_UNITS_PTS:
		return 1.0;	/* points */
	case GL_PREFS_UNITS_INCHES:
		return 1.0 / 72.0;	/* inches */
	case GL_PREFS_UNITS_MM:
		return 0.35277778;	/* mm */
	default:
		WARN ("Illegal units");	/* Should not happen */
		return 1.0;
	}
}

/****************************************************************************/
/* Get precision for desired units.                                         */
/****************************************************************************/
gint
gl_prefs_get_units_precision (void)
{
	if (prefs == NULL) {
		prefs = read_prefs ();
	}

	switch (prefs->default_units) {
	case GL_PREFS_UNITS_PTS:
		return 1;	/* points */
	case GL_PREFS_UNITS_INCHES:
		return 3;	/* inches */
	case GL_PREFS_UNITS_MM:
		return 1;	/* mm */
	default:
		WARN ("Illegal units");	/* Should not happen */
		return 1.0;
	}
}

/****************************************************************************/
/* Get step size for desired units.                                         */
/****************************************************************************/
gdouble
gl_prefs_get_units_step_size (void)
{
	if (prefs == NULL) {
		prefs = read_prefs ();
	}

	switch (prefs->default_units) {
	case GL_PREFS_UNITS_PTS:
		return 0.1;	/* points */
	case GL_PREFS_UNITS_INCHES:
		return 0.001;	/* inches */
	case GL_PREFS_UNITS_MM:
		return 0.1;	/* mm */
	default:
		WARN ("Illegal units");	/* Should not happen */
		return 1.0;
	}
}

/****************************************************************************/
/* Get string representing desired units.                                   */
/****************************************************************************/
const gchar *
gl_prefs_get_units_string (void)
{
	if (prefs == NULL) {
		prefs = read_prefs ();
	}

	switch (prefs->default_units) {
	case GL_PREFS_UNITS_PTS:
		return _("points");
	case GL_PREFS_UNITS_INCHES:
		return _("inches");
	case GL_PREFS_UNITS_MM:
		return _("mm");
	default:
		WARN ("Illegal units");	/* Should not happen */
		return _("points");
	}
}

/****************************************************************************/
/* Get default page size.                                                   */
/****************************************************************************/
const gchar *
gl_prefs_get_page_size (void)
{
	if (prefs == NULL) {
		prefs = read_prefs ();
	}

	return prefs->default_page_size;
}
