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

#include "glabels.h"
#include "xml-label.h"
#include "file.h"
#include "mdi.h"
#include "recent.h"
#include "alert.h"
#include "util.h"
#include "wdgt-media-select.h"
#include "wdgt-rotate-label.h"
#include "debug.h"

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/* Saved state for new dialog */
static gchar   *page_size   = NULL;
static gchar   *sheet_name  = NULL;
static gboolean rotate_flag = FALSE;

/* Saved state of file selectors */
static gchar *open_path = NULL;
static gchar *save_path = NULL;

/*===========================================*/
/* Local function prototypes.                */
/*===========================================*/
static void create_new_dialog_widgets (GtkDialog         *dlg);
static void new_template_changed      (glWdgtMediaSelect *select,
				       gpointer           data);
static void new_response              (GtkDialog         *dlg,
				       gint               response,
				       gpointer           user_data);
static void open_ok                   (GtkWidget         *widget,
				       GtkFileSelection  *fsel);

static void save_as_ok_cb             (GtkWidget         *widget,
				       GtkFileSelection  *fsel);
static void save_as_cancel_cb         (GtkWidget         *widget,
				       GtkFileSelection  *fsel);
static void save_as_destroy_cb        (GtkWidget         *widget,
				       gboolean          *destroy_flag);


/*****************************************************************************/
/* "New" menu callback.                                                      */
/*****************************************************************************/
void
gl_file_new (void)
{
	GtkWidget    *dlg;
	BonoboWindow *win = glabels_get_active_window ();

	gl_debug (DEBUG_FILE, "START");

	g_return_if_fail (glabels_mdi != NULL);
	g_return_if_fail (win != NULL);

	dlg = gtk_dialog_new_with_buttons (_("New Label or Card"),
					   GTK_WINDOW (win),
					   GTK_DIALOG_DESTROY_WITH_PARENT,
					   GTK_STOCK_OK, GTK_RESPONSE_OK,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   NULL);

	create_new_dialog_widgets (GTK_DIALOG (dlg));

	g_signal_connect (G_OBJECT(dlg), "response",
			  G_CALLBACK (new_response), dlg);

	gtk_widget_show_all (GTK_WIDGET (dlg));

	gl_debug (DEBUG_FILE, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create widgets.                                                 */
/*---------------------------------------------------------------------------*/
static void
create_new_dialog_widgets (GtkDialog *dlg)
{
	GtkWidget *wframe, *wvbox, *template_entry, *rotate_sel;

	gl_debug (DEBUG_FILE, "START");

	wframe = gtk_frame_new (_("Media Type"));
	gtk_box_pack_start (GTK_BOX (dlg->vbox), wframe, FALSE, FALSE, 0);

	wvbox = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox), 10);
	gtk_container_add (GTK_CONTAINER (wframe), wvbox);

	template_entry = gl_wdgt_media_select_new ();
	gtk_container_add (GTK_CONTAINER (wvbox), template_entry);

	wframe = gtk_frame_new (_("Label orientation"));
	gtk_box_pack_start (GTK_BOX (dlg->vbox), wframe, FALSE, FALSE, 0);

	wvbox = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox), 10);
	gtk_container_add (GTK_CONTAINER (wframe), wvbox);

	rotate_sel = gl_wdgt_rotate_label_new ();
	gtk_box_pack_start (GTK_BOX (wvbox), rotate_sel, FALSE, FALSE, 0);

	g_object_set_data (G_OBJECT (dlg), "template_entry",
			     template_entry);
	g_object_set_data (G_OBJECT (dlg), "rotate_sel", rotate_sel);

	g_signal_connect (G_OBJECT (template_entry), "changed",
			  G_CALLBACK (new_template_changed), rotate_sel);

	if (page_size != NULL) {
		gl_wdgt_media_select_set_page_size (GL_WDGT_MEDIA_SELECT (template_entry),
					       page_size);
	}
	if (sheet_name != NULL) {
		gl_wdgt_media_select_set_name (GL_WDGT_MEDIA_SELECT (template_entry),
					  sheet_name);
		gl_wdgt_rotate_label_set_template_name (GL_WDGT_ROTATE_LABEL
						    (rotate_sel), sheet_name);
	} else {
		sheet_name =
		    gl_wdgt_media_select_get_name (GL_WDGT_MEDIA_SELECT (template_entry));
		gl_wdgt_rotate_label_set_template_name (GL_WDGT_ROTATE_LABEL
						    (rotate_sel), sheet_name);
	}
	gl_wdgt_rotate_label_set_state (GL_WDGT_ROTATE_LABEL (rotate_sel), rotate_flag);

	gl_debug (DEBUG_FILE, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  New template changed callback.                                  */
/*---------------------------------------------------------------------------*/
static void
new_template_changed (glWdgtMediaSelect *select,
		      gpointer           data)
{
	glWdgtRotateLabel *rotate_sel = GL_WDGT_ROTATE_LABEL (data);
	gchar             *name;

	gl_debug (DEBUG_FILE, "START");

	name = gl_wdgt_media_select_get_name (GL_WDGT_MEDIA_SELECT (select));

	gl_wdgt_rotate_label_set_template_name (GL_WDGT_ROTATE_LABEL (rotate_sel),
						name);

	g_free (name);

	gl_debug (DEBUG_FILE, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  New "ok" button callback.                                       */
/*---------------------------------------------------------------------------*/
static void
new_response (GtkDialog *dlg,
	      gint       response,
	      gpointer   user_data)
{
	GtkWidget  *template_entry, *rotate_sel;
	glMDIChild *new_child = NULL;
	gint        ret;

	gl_debug (DEBUG_FILE, "START");

	switch (response) {
	case GTK_RESPONSE_OK:
		template_entry =
			GTK_WIDGET (g_object_get_data (G_OBJECT (dlg),
							 "template_entry"));
		rotate_sel = GTK_WIDGET (g_object_get_data (G_OBJECT (dlg),
							      "rotate_sel"));

		if (page_size != NULL)
			g_free (page_size);
		page_size =
			gl_wdgt_media_select_get_page_size (GL_WDGT_MEDIA_SELECT (template_entry));

		if (sheet_name != NULL)
			g_free (sheet_name);
		sheet_name =
			gl_wdgt_media_select_get_name (GL_WDGT_MEDIA_SELECT (template_entry));

		rotate_flag =
			gl_wdgt_rotate_label_get_state (GL_WDGT_ROTATE_LABEL (rotate_sel));

		new_child = gl_mdi_child_new (sheet_name, rotate_flag);
		gl_debug (DEBUG_FILE, "template set.");

		ret = bonobo_mdi_add_child (BONOBO_MDI (glabels_mdi),
					    BONOBO_MDI_CHILD (new_child));
		g_return_if_fail (ret != FALSE);
		gl_debug (DEBUG_FILE, "Child added.");

		ret = bonobo_mdi_add_view (BONOBO_MDI (glabels_mdi),
					   BONOBO_MDI_CHILD (new_child));
		g_return_if_fail (ret != FALSE);
		gl_debug (DEBUG_FILE, "View added.");

		gtk_widget_grab_focus (GTK_WIDGET (glabels_get_active_view ()));
		break;
	}

	gtk_widget_destroy (GTK_WIDGET (dlg));

	gl_debug (DEBUG_FILE, "END");
}

/*****************************************************************************/
/* "Open" menu callback.                                                     */
/*****************************************************************************/
void
gl_file_open (glMDIChild *active_child)
{
	GtkFileSelection *fsel;
	BonoboWindow     *app = glabels_get_active_window ();

	gl_debug (DEBUG_FILE, "START");

	g_return_if_fail (app != NULL);

	fsel = GTK_FILE_SELECTION (gtk_file_selection_new (_("Open")));
	gtk_window_set_transient_for (GTK_WINDOW (fsel), GTK_WINDOW (app));
	gtk_window_set_title (GTK_WINDOW (fsel), _("Open label"));

	g_signal_connect (G_OBJECT (fsel->ok_button), "clicked",
			  G_CALLBACK (open_ok), fsel);

	g_signal_connect_swapped (G_OBJECT (fsel->cancel_button), "clicked",
				  G_CALLBACK (gtk_widget_destroy),
				  G_OBJECT (fsel));

	/* Recover state of open dialog */
	if (open_path != NULL) {
		gtk_file_selection_set_filename (fsel, open_path);
	}

	/* show the dialog */
	gtk_widget_show (GTK_WIDGET (fsel));

	gl_debug (DEBUG_FILE, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Open "O.K." button callback.                                    */
/*---------------------------------------------------------------------------*/
static void
open_ok (GtkWidget        *widget,
	 GtkFileSelection *fsel)
{
	gchar            *filename;
	GtkWidget        *dlg;
	glMDIChild       *new_child = NULL;
	gint              ret;
	GnomeRecentModel *recent;

	gl_debug (DEBUG_FILE, "START");

	g_return_if_fail (GTK_IS_FILE_SELECTION (fsel));

	/* get the filename */
	filename = g_strdup (gtk_file_selection_get_filename (fsel));

	if (!filename || g_file_test (filename, G_FILE_TEST_IS_DIR)) {

		dlg = gl_alert_dialog_new (GTK_WINDOW(fsel),
					   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					   GTK_MESSAGE_WARNING,
					   GTK_BUTTONS_CLOSE,
					   _("Empty file name selection"),
					   _("Please select a file or supply a valid file name"));

		gtk_dialog_run (GTK_DIALOG (dlg));
		gtk_widget_destroy (dlg);

	} else {

		if (!g_file_test (filename, G_FILE_TEST_IS_REGULAR)) {

			dlg = gl_alert_dialog_new (GTK_WINDOW(fsel),
						   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						   GTK_MESSAGE_WARNING,
						   GTK_BUTTONS_CLOSE,
						   _("File does not exist"),
						   _("Please select a file or supply a valid file name"));

			gtk_dialog_run (GTK_DIALOG (dlg));
			gtk_widget_destroy (dlg);


		} else {
		
			if ( gl_file_open_real (filename, GTK_WINDOW(fsel)) ) {
				gtk_widget_destroy (GTK_WIDGET (fsel));
			}

		}

	}

	g_free (filename);

	gl_debug (DEBUG_FILE, "END");
}

/*****************************************************************************/
/* "Open recent" menu callback.                                              */
/*****************************************************************************/
gboolean
gl_file_open_recent (GnomeRecentView *view,
		     const gchar     *filename,
		     BonoboWindow    *win)
{
	gl_debug (DEBUG_FILE, "");

	return gl_file_open_real (filename, GTK_WINDOW(win));
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Open a file.                                                    */
/*---------------------------------------------------------------------------*/
gboolean
gl_file_open_real (const gchar     *filename,
		   GtkWindow       *win)
{
	gchar            *abs_filename;
	glMDIChild       *new_child = NULL;
	GnomeRecentModel *recent;
	gint              ret;

	gl_debug (DEBUG_FILE, "START");

	abs_filename = gl_util_make_absolute (filename);
	new_child = gl_mdi_child_new_with_uri (filename, NULL);
	if (!new_child) {
		GtkWidget *dlg;
		gchar *primary_msg;

		gl_debug (DEBUG_FILE, "couldn't open file");

		primary_msg = g_strdup_printf (_("Could not open file \"%s\""),
					       filename);

		dlg = gl_alert_dialog_new (GTK_WINDOW(win),
					   GTK_DIALOG_DESTROY_WITH_PARENT,
					   GTK_MESSAGE_ERROR,
					   GTK_BUTTONS_CLOSE,
					   primary_msg,
					   _("Not a supported file format"));

		g_free (primary_msg);

		gtk_dialog_run (GTK_DIALOG (dlg));
		gtk_widget_destroy (dlg);

		g_free (abs_filename);

		gl_debug (DEBUG_FILE, "END false");

		return FALSE;

	} else {

		ret = bonobo_mdi_add_child (BONOBO_MDI (glabels_mdi),
					    BONOBO_MDI_CHILD (new_child));
		g_return_if_fail (ret != FALSE);
		gl_debug (DEBUG_FILE, "Child added.");

		ret = bonobo_mdi_add_view (BONOBO_MDI (glabels_mdi),
					   BONOBO_MDI_CHILD (new_child));
		g_return_if_fail (ret != FALSE);
		gl_debug (DEBUG_FILE, "View added.");

		gtk_widget_grab_focus (GTK_WIDGET (glabels_get_active_view ()));

		recent = gl_recent_get_model ();
		gnome_recent_model_add (recent, abs_filename);

		if (open_path != NULL)
			g_free (open_path);
		open_path = g_path_get_dirname (abs_filename);
		if (open_path != NULL)
			open_path = g_strconcat (open_path, "/", NULL);

		g_free (abs_filename);

		gl_debug (DEBUG_FILE, "END true");

		return TRUE;

	}
}

/*****************************************************************************/
/* "Save" menu callback.                                                     */
/*****************************************************************************/
gboolean
gl_file_save (glMDIChild *child)
{
	glXMLLabelStatus  status;
	glLabel          *label = NULL;
	GError           *error = NULL;
	gchar            *filename = NULL;
	GnomeRecentModel *recent;

	gl_debug (DEBUG_FILE, "");

	g_return_val_if_fail (child != NULL, FALSE);
	
	label = child->label;
	g_return_val_if_fail (label != NULL, FALSE);
	
	if (gl_label_is_untitled (label))
	{
		gl_debug (DEBUG_FILE, "Untitled");

		return gl_file_save_as (child);
	}

	if (!gl_label_is_modified (label))	
	{
		gl_debug (DEBUG_FILE, "Not modified");

		return TRUE;
	}
	
	filename = gl_label_get_filename (label);
	g_return_val_if_fail (filename != NULL, FALSE);
	
	gl_xml_label_save (label, filename, &status);

	if (status != XML_LABEL_OK)
	{
		GtkWidget *dialog;
		gchar *primary_msg;

		gl_debug (DEBUG_FILE, "FAILED");

		primary_msg = g_strdup_printf (_("Could not save file \"%s\""),
					       filename);

		dialog = gl_alert_dialog_new (GTK_WINDOW(glabels_get_active_window()),
					      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					      GTK_MESSAGE_ERROR,
					      GTK_BUTTONS_CLOSE,
					      primary_msg,
					      _("Error encountered during save.  The file is still not saved."));

		g_free (primary_msg);

		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);

		g_free (filename);

		return FALSE;
	}	
	else
	{
		gl_debug (DEBUG_FILE, "OK");

		recent = gl_recent_get_model ();
		gnome_recent_model_add (recent, filename);

		g_free (filename);

		return TRUE;
	}
}

/*****************************************************************************/
/* "Save As" menu callback.                                                  */
/*****************************************************************************/
gboolean
gl_file_save_as (glMDIChild *child)
{
	GtkFileSelection *fsel;
	BonoboWindow     *app = glabels_get_active_window ();
	gboolean          saved_flag = FALSE;
	gboolean          destroy_flag = FALSE;

	gl_debug (DEBUG_FILE, "START");

	g_return_val_if_fail (child != NULL, FALSE);
	g_return_val_if_fail (app != NULL, FALSE);


	fsel = GTK_FILE_SELECTION (gtk_file_selection_new (_("Save label as")));
	gtk_window_set_modal (GTK_WINDOW (fsel), TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (fsel), GTK_WINDOW (app));

	g_object_set_data (G_OBJECT (fsel), "child", child);
	g_object_set_data (G_OBJECT (fsel), "saved_flag", &saved_flag);

	g_signal_connect (G_OBJECT (fsel->ok_button), "clicked",
			  G_CALLBACK (save_as_ok_cb), fsel);

	g_signal_connect (G_OBJECT (fsel->cancel_button), "clicked",
			  G_CALLBACK (save_as_cancel_cb), fsel);

	g_signal_connect (G_OBJECT (fsel), "destroy",
			  G_CALLBACK (save_as_destroy_cb), &destroy_flag);

	/* Recover proper state of save-as dialog */
	if (save_path != NULL) {
		gtk_file_selection_set_filename (fsel, save_path);
	}

	/* show the dialog */
	gtk_widget_show (GTK_WIDGET (fsel));

	/* Hold here and process events until we are done with this dialog. */
	gtk_main ();

	/* Destroy dialog if not already destroyed. */
	if (!destroy_flag) {
		/* Disconnect our destroy callback first, so that we don't
		 * kill the current gtk_main() loop. */
		g_signal_handlers_disconnect_by_func (GTK_OBJECT (fsel),
						      G_CALLBACK (save_as_destroy_cb),
						      &destroy_flag);
		gtk_widget_destroy (GTK_WIDGET (fsel));
	}

	gl_debug (DEBUG_FILE, "END");

	/* Return flag as set by one of the above callbacks, TRUE = saved */
	return saved_flag;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Save As" ok button callback.                                   */
/*---------------------------------------------------------------------------*/
static void
save_as_ok_cb (GtkWidget        *widget,
	       GtkFileSelection *fsel)
{
	gchar            *raw_filename, *filename;
	GtkWidget        *dlg;
	glMDIChild       *child;
	glLabel          *label;
	glXMLLabelStatus  status;
	GnomeRecentModel *recent;
	gboolean         *saved_flag;
	gchar            *primary_msg;
	gboolean          cancel_flag = FALSE;

	gl_debug (DEBUG_FILE, "START");

	g_return_if_fail (GTK_IS_FILE_SELECTION (fsel));

	child = g_object_get_data (G_OBJECT(fsel), "child");
	saved_flag = g_object_get_data (G_OBJECT(fsel), "saved_flag");

	g_return_if_fail (child != NULL);
	g_return_if_fail (GL_IS_MDI_CHILD (child));
	gl_debug (DEBUG_FILE, "Got child");

	label = child->label;

	/* get the filename */
	raw_filename = g_strdup (gtk_file_selection_get_filename (fsel));

	gl_debug (DEBUG_FILE, "raw_filename = \"%s\"", raw_filename);

	if (!raw_filename || g_file_test (raw_filename, G_FILE_TEST_IS_DIR)) {

		dlg = gl_alert_dialog_new (GTK_WINDOW(fsel),
					   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					   GTK_MESSAGE_WARNING,
					   GTK_BUTTONS_CLOSE,
					   _("Empty file name selection"),
					   _("Please supply a valid file name"));

		gtk_dialog_run (GTK_DIALOG (dlg));
		gtk_widget_destroy (dlg);

	} else {

		filename = gl_util_add_extension (raw_filename);

		gl_debug (DEBUG_FILE, "filename = \"%s\"", filename);

		if (g_file_test (filename, G_FILE_TEST_IS_REGULAR)) {
			gint ret;

			primary_msg = g_strdup_printf (_("Overwrite file \"%s\"?"),
						       filename);

			dlg = gl_alert_dialog_new (GTK_WINDOW(fsel),
						   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						   GTK_MESSAGE_QUESTION,
						   GTK_BUTTONS_YES_NO,
						   primary_msg,
						   _("File already exists."));
			
			g_free (primary_msg);

			ret = gtk_dialog_run (GTK_DIALOG (dlg));
			if ( ret == GTK_RESPONSE_NO ) {
				cancel_flag = TRUE;
			}
			gtk_widget_destroy (dlg);
		}

		if (!cancel_flag) {

			gl_xml_label_save (label, filename, &status);

			gl_debug (DEBUG_FILE, "status of save = %d", status);

			if ( status != XML_LABEL_OK ) {

				primary_msg = g_strdup_printf (_("Could not save file \"%s\""),
							       filename);

				dlg = gl_alert_dialog_new (GTK_WINDOW(fsel),
							   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
							   GTK_MESSAGE_ERROR,
							   GTK_BUTTONS_CLOSE,
							   primary_msg,
							   _("Error encountered during save.  The file is still not saved."));

				g_free (primary_msg);

				gtk_dialog_run (GTK_DIALOG (dlg));
				gtk_widget_destroy (dlg);

			} else {

				*saved_flag = TRUE;

				recent = gl_recent_get_model ();
				gnome_recent_model_add (recent, filename);

				if (save_path != NULL)
					g_free (save_path);
				save_path = g_path_get_dirname (filename);
				if (save_path != NULL)
					save_path = g_strconcat (save_path, "/", NULL);

				gtk_widget_destroy (GTK_WIDGET (fsel));
			}

		}

		g_free (filename);
	}

	g_free (raw_filename);

	gl_debug (DEBUG_FILE, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Save As" cancel button callback.                               */
/*---------------------------------------------------------------------------*/
static void
save_as_cancel_cb (GtkWidget        *widget,
		   GtkFileSelection *fsel)
{
	gboolean *saved_flag = g_object_get_data (G_OBJECT (fsel), "saved_flag");

	g_return_if_fail (GTK_IS_FILE_SELECTION (fsel));

	*saved_flag = FALSE;
	gtk_widget_hide (GTK_WIDGET (fsel));
	gtk_main_quit ();
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Save As" destroy callback.                                     */
/*---------------------------------------------------------------------------*/
static void
save_as_destroy_cb (GtkWidget *widget,
		    gboolean  *destroy_flag)
{
	*destroy_flag = TRUE;
	gtk_main_quit ();
}


/*****************************************************************************/
/* "Close" menu callback.                                                    */
/*****************************************************************************/
void
gl_file_close (GtkWidget *view)
{
	gint            ret;
	BonoboMDIChild *child;

	gl_debug (DEBUG_FILE, "START");

	g_return_if_fail (view != NULL);

	child = bonobo_mdi_get_child_from_view (view);
	g_return_if_fail (child != NULL);

	if (g_list_length (bonobo_mdi_child_get_views (child)) > 1)
	{		
		ret = bonobo_mdi_remove_view (BONOBO_MDI (glabels_mdi), view, FALSE);
		gl_debug (DEBUG_FILE, "View removed.");
	}
	else
	{
		ret = bonobo_mdi_remove_child (BONOBO_MDI (glabels_mdi), child, FALSE);
		gl_debug (DEBUG_FILE, "Child removed.");
	}

	if (ret)
		gl_mdi_set_active_window_title (BONOBO_MDI (glabels_mdi));

	if (bonobo_mdi_get_active_child (BONOBO_MDI (glabels_mdi)) == NULL)
	{
		gl_mdi_set_active_window_verbs_sensitivity (BONOBO_MDI (glabels_mdi));
	}

	gl_debug (DEBUG_FILE, "END");
}

/*****************************************************************************/
/* "Close all"                                                               */
/*****************************************************************************/
gboolean
gl_file_close_all (void)
{
	gboolean ret;

	gl_debug (DEBUG_FILE, "START");

	ret = bonobo_mdi_remove_all (BONOBO_MDI (glabels_mdi), FALSE);

	if (bonobo_mdi_get_active_child (BONOBO_MDI (glabels_mdi)) == NULL)
	{
		gl_mdi_set_active_window_verbs_sensitivity (BONOBO_MDI (glabels_mdi));
	}

	gl_debug (DEBUG_FILE, "END");

	return ret;
}

/*****************************************************************************/
/* "Exit" menu callback.                                                     */
/*****************************************************************************/
void
gl_file_exit (void)
{
	gl_debug (DEBUG_FILE, "START");
	
	if (!gl_file_close_all ())
		return;

	gl_debug (DEBUG_FILE, "All files closed.");
	
	/* We need to disconnect the signal because mdi "destroy" event
	   is connected to gl_file_exit ( i.e. this function ). */
	g_signal_handlers_disconnect_by_func (G_OBJECT (glabels_mdi),
					      G_CALLBACK (gl_file_exit), NULL);
	
	gl_prefs_save_settings ();

	gl_debug (DEBUG_FILE, "Unref glabels_mdi.");

	g_object_unref (G_OBJECT (glabels_mdi));

	gl_debug (DEBUG_FILE, "Unref glabels_mdi: DONE");

	gtk_main_quit ();

	gl_debug (DEBUG_FILE, "END");
}
