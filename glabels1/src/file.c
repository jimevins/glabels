/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  file.c:  FILE menu dialog module
 *
 *  Copyright (C) 2001  Jim Evins <evins@snaught.com>.
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
#include <string.h>

#include "file.h"
#include "mdi.h"
#include "util.h"
#include "media_select.h"
#include "rotate_select.h"

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/* Saved state for new dialog */
static gchar *page_size = NULL;
static gchar *sheet_name = NULL;
static gboolean rotate_flag = FALSE;

/* Saved state of file selectors */
static gchar *open_path = NULL;
static gchar *save_path = NULL;

/*===========================================*/
/* Local function prototypes.                */
/*===========================================*/
static void create_new_dialog_widgets (GnomeDialog * new_dlg);
static void new_template_changed (glMediaSelect * select,
				  gpointer data);
static void new_ok (GtkWidget * widget,
		    gpointer data);
static void open_ok (GtkWidget * widget,
		     GtkFileSelection * fsel);
static void save_as_ok (GtkWidget * widget,
			GtkFileSelection * fsel);

/*****************************************************************************/
/* "New" menu callback.                                                      */
/*****************************************************************************/
void
gl_file_new_cb (GtkWidget * widget,
		gpointer data)
{
	GtkWidget *new_dlg;
	GnomeApp *app = gnome_mdi_get_active_window (GNOME_MDI (data));

	g_return_if_fail (app != NULL);

	new_dlg = gnome_dialog_new (_("New Label or Card"),
				    GNOME_STOCK_BUTTON_OK,
				    GNOME_STOCK_BUTTON_CANCEL, NULL);
	gtk_window_set_transient_for (GTK_WINDOW (new_dlg), GTK_WINDOW (app));

	create_new_dialog_widgets (GNOME_DIALOG (new_dlg));

	gnome_dialog_button_connect (GNOME_DIALOG (new_dlg), 0,
				     GTK_SIGNAL_FUNC (new_ok), new_dlg);

	gnome_dialog_button_connect_object (GNOME_DIALOG (new_dlg), 1,
					    GTK_SIGNAL_FUNC
					    (gtk_widget_destroy),
					    GTK_OBJECT (new_dlg));

	gtk_widget_show_all (GTK_WIDGET (new_dlg));
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create widgets to enter a bond by sn and idate.                 */
/*---------------------------------------------------------------------------*/
static void
create_new_dialog_widgets (GnomeDialog * new_dlg)
{
	GtkWidget *wframe, *wvbox, *template_entry, *rotate_sel;

	wframe = gtk_frame_new (_("Media Type"));
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (new_dlg)->vbox),
			    wframe, FALSE, FALSE, 0);

	wvbox = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox), 10);
	gtk_container_add (GTK_CONTAINER (wframe), wvbox);

	template_entry = gl_media_select_new ();
	gtk_container_add (GTK_CONTAINER (wvbox), template_entry);

	wframe = gtk_frame_new (_("Label orientation"));
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (new_dlg)->vbox),
			    wframe, FALSE, FALSE, 0);

	wvbox = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox), 10);
	gtk_container_add (GTK_CONTAINER (wframe), wvbox);

	rotate_sel = gl_rotate_select_new ();
	gtk_box_pack_start (GTK_BOX (wvbox), rotate_sel, FALSE, FALSE, 0);

	gtk_object_set_data (GTK_OBJECT (new_dlg), "template_entry",
			     template_entry);
	gtk_object_set_data (GTK_OBJECT (new_dlg), "rotate_sel", rotate_sel);

	gtk_signal_connect (GTK_OBJECT (template_entry), "changed",
			    GTK_SIGNAL_FUNC (new_template_changed), rotate_sel);

	if (page_size != NULL) {
		gl_media_select_set_page_size (GL_MEDIA_SELECT (template_entry),
					       page_size);
	}
	if (sheet_name != NULL) {
		gl_media_select_set_name (GL_MEDIA_SELECT (template_entry),
					  sheet_name);
		gl_rotate_select_set_template_name (GL_ROTATE_SELECT
						    (rotate_sel), sheet_name);
	} else {
		sheet_name =
		    gl_media_select_get_name (GL_MEDIA_SELECT (template_entry));
		gl_rotate_select_set_template_name (GL_ROTATE_SELECT
						    (rotate_sel), sheet_name);
	}
	gl_rotate_select_set_state (GL_ROTATE_SELECT (rotate_sel), rotate_flag);

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  New template changed callback.                                  */
/*---------------------------------------------------------------------------*/
static void
new_template_changed (glMediaSelect * select,
		      gpointer data)
{
	glRotateSelect *rotate_sel = GL_ROTATE_SELECT (data);
	gchar *name;

	name = gl_media_select_get_name (GL_MEDIA_SELECT (select));

	gl_rotate_select_set_template_name (GL_ROTATE_SELECT (rotate_sel),
					    name);

	g_free (name);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  New "ok" button callback.                                       */
/*---------------------------------------------------------------------------*/
static void
new_ok (GtkWidget * widget,
	gpointer data)
{
	GnomeDialog *new_dlg = GNOME_DIALOG (data);
	GtkWidget *template_entry, *rotate_sel;

	template_entry =
	    GTK_WIDGET (gtk_object_get_data
			(GTK_OBJECT (new_dlg), "template_entry"));
	rotate_sel =
	    GTK_WIDGET (gtk_object_get_data
			(GTK_OBJECT (new_dlg), "rotate_sel"));

	if (page_size != NULL)
		g_free (page_size);
	page_size =
	    gl_media_select_get_page_size (GL_MEDIA_SELECT (template_entry));

	if (sheet_name != NULL)
		g_free (sheet_name);
	sheet_name =
	    gl_media_select_get_name (GL_MEDIA_SELECT (template_entry));

	rotate_flag =
	    gl_rotate_select_get_state (GL_ROTATE_SELECT (rotate_sel));

	gl_mdi_new_child (sheet_name, rotate_flag);

	gtk_widget_destroy (GTK_WIDGET (new_dlg));
}

/*****************************************************************************/
/* "Open" menu callback.                                                     */
/*****************************************************************************/
void
gl_file_open_cb (GtkWidget * widget,
		 gpointer data)
{
	GtkFileSelection *fsel;
	GnomeApp *app = gnome_mdi_get_active_window (GNOME_MDI (data));

	g_return_if_fail (app != NULL);

	fsel = GTK_FILE_SELECTION (gtk_file_selection_new (_("Open")));
	gtk_window_set_transient_for (GTK_WINDOW (fsel), GTK_WINDOW (app));
	gtk_window_set_title (GTK_WINDOW (fsel), _("Open label"));

	gtk_signal_connect (GTK_OBJECT (fsel->ok_button), "clicked",
			    GTK_SIGNAL_FUNC (open_ok), fsel);

	gtk_signal_connect_object (GTK_OBJECT (fsel->cancel_button), "clicked",
				   GTK_SIGNAL_FUNC (gtk_widget_destroy),
				   GTK_OBJECT (fsel));

	/* Recover state of open dialog */
	if (open_path != NULL) {
		gtk_file_selection_set_filename (fsel, open_path);
	}

	/* show the dialog */
	gtk_widget_show (GTK_WIDGET (fsel));
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Open "O.K." button callback.                                    */
/*---------------------------------------------------------------------------*/
static void
open_ok (GtkWidget * widget,
	 GtkFileSelection * fsel)
{
	gchar *filename;
	GtkWidget *dlg;

	g_return_if_fail (GTK_IS_FILE_SELECTION (fsel));

	/* get the filename */
	filename = g_strdup (gtk_file_selection_get_filename (fsel));
	if (filename) {
		if (!gl_mdi_new_child_from_xml_file (filename)) {

			dlg =
			    gnome_error_dialog_parented (_("Cannot open file"),
							 GTK_WINDOW (fsel));
			gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);

		} else {

			gtk_widget_destroy (GTK_WIDGET (fsel));

			if (open_path != NULL)
				g_free (open_path);
			open_path = g_dirname (filename);
			if (open_path != NULL)
				open_path = g_strconcat (open_path, "/", NULL);

		}
	}

	g_free (filename);
}

/*****************************************************************************/
/* "Save" menu callback.                                                     */
/*****************************************************************************/
void
gl_file_save_cb (GtkWidget * widget,
		 gpointer data)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (GNOME_MDI (data));
	GnomeApp *app = gnome_mdi_get_active_window (GNOME_MDI (data));
	gchar *filename;

	g_return_if_fail (child != NULL);
	g_return_if_fail (app != NULL);

	filename = gl_mdi_get_filename (child);
	if (!filename) {
		gl_file_save_as_cb (widget, data);
	} else {
		if (!gl_mdi_save_child_to_xml_file (child, filename)) {
			gnome_error_dialog_parented (_("Cannot save file"),
						     GTK_WINDOW (app));
		}
	}

	g_free (filename);
}

/*****************************************************************************/
/* "Save As" menu callback.                                                  */
/*****************************************************************************/
void
gl_file_save_as_cb (GtkWidget * widget,
		    gpointer data)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (GNOME_MDI (data));
	GtkFileSelection *fsel;
	GnomeApp *app = gnome_mdi_get_active_window (GNOME_MDI (data));
	gint id;

	g_return_if_fail (child != NULL);
	g_return_if_fail (app != NULL);

	fsel = GTK_FILE_SELECTION (gtk_file_selection_new (_("Save As")));
	id = gl_mdi_get_id (child);
	gtk_object_set_data (GTK_OBJECT (fsel), "id", GINT_TO_POINTER (id));

	gtk_window_set_transient_for (GTK_WINDOW (fsel), GTK_WINDOW (app));
	gtk_window_set_title (GTK_WINDOW (fsel), _("Save label as"));

	gtk_signal_connect (GTK_OBJECT (fsel->ok_button), "clicked",
			    GTK_SIGNAL_FUNC (save_as_ok), fsel);

	gtk_signal_connect_object (GTK_OBJECT (fsel->cancel_button), "clicked",
				   GTK_SIGNAL_FUNC (gtk_widget_destroy),
				   GTK_OBJECT (fsel));

	/* Recover proper state of save-as dialog */
	if (save_path != NULL) {
		gtk_file_selection_set_filename (fsel, save_path);
	}

	/* show the dialog */
	gtk_widget_show (GTK_WIDGET (fsel));

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Save As" ok button callback.                                   */
/*---------------------------------------------------------------------------*/
static void
save_as_ok (GtkWidget * widget,
	    GtkFileSelection * fsel)
{
	gchar *raw_filename, *filename;
	GtkWidget *dlg;
	gint id =
	    GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (fsel), "id"));
	GnomeMDIChild *child = gl_mdi_find_child (id);

	g_return_if_fail (GTK_IS_FILE_SELECTION (fsel));

	if (child == NULL) {
		gtk_widget_destroy (GTK_WIDGET (fsel));
		dlg = gnome_error_dialog (_("Label no longer valid!"));
		gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);
		return;
	}

	/* get the filename */
	raw_filename = g_strdup (gtk_file_selection_get_filename (fsel));

	if (!raw_filename || (raw_filename[strlen (raw_filename) - 1] == '/')) {

		dlg = gnome_warning_dialog_parented (_("Must supply file name"),
						     GTK_WINDOW (fsel));
		gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);

	} else {

		filename = gl_util_add_extension (raw_filename);

		if (!gl_mdi_save_child_to_xml_file (child, filename)) {

			dlg =
			    gnome_error_dialog_parented (_
							 ("Error writing file"),
							 GTK_WINDOW (fsel));
			gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);

		} else {

			if (save_path != NULL)
				g_free (save_path);
			save_path = g_dirname (filename);
			if (save_path != NULL)
				save_path = g_strconcat (save_path, "/", NULL);

			gtk_widget_destroy (GTK_WIDGET (fsel));
		}

		g_free (filename);
	}

	g_free (raw_filename);
}

/*****************************************************************************/
/* "Close" menu callback.                                                    */
/*****************************************************************************/
void
gl_file_close_cb (GtkWidget * widget,
		  gpointer data)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (GNOME_MDI (data));

	gl_mdi_close_child (child);
}

/*****************************************************************************/
/* "Exit" menu callback.                                                     */
/*****************************************************************************/
void
gl_file_exit_cb (GtkWidget * widget,
		 gpointer data)
{
	gl_mdi_close_all ();
}
