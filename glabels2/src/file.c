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

#include "xml-label.h"
#include "file.h"
#include "recent.h"
#include "hig.h"
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
static void create_new_dialog_widgets (glHigDialog       *dlg);
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
gl_file_new (GtkWindow *window)
{
	GtkWidget    *dlg;

	gl_debug (DEBUG_FILE, "START");

	g_return_if_fail (window != NULL);

	dlg = gl_hig_dialog_new_with_buttons (_("New Label or Card"),
					      window,
					      GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
					      GTK_STOCK_OK, GTK_RESPONSE_OK,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      NULL);

	create_new_dialog_widgets (GL_HIG_DIALOG (dlg));

	g_object_set_data (G_OBJECT (dlg), "parent_window", window);

	g_signal_connect (G_OBJECT(dlg), "response",
			  G_CALLBACK (new_response), dlg);

        gtk_window_set_resizable (GTK_WINDOW (dlg), FALSE);
	gtk_widget_show_all (GTK_WIDGET (dlg));

	gl_debug (DEBUG_FILE, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create widgets.                                                 */
/*---------------------------------------------------------------------------*/
static void
create_new_dialog_widgets (glHigDialog *dlg)
{
	GtkWidget *wframe, *template_entry, *rotate_sel;

	gl_debug (DEBUG_FILE, "START");

	wframe = gl_hig_category_new (_("Media Type"));
	gl_hig_dialog_add_widget (dlg, wframe);

	template_entry = gl_wdgt_media_select_new ();
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wframe), template_entry);

	wframe = gl_hig_category_new (_("Label orientation"));
	gl_hig_dialog_add_widget (dlg, wframe);

	rotate_sel = gl_wdgt_rotate_label_new ();
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wframe), rotate_sel);

	g_object_set_data (G_OBJECT (dlg), "template_entry", template_entry);
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
	GtkWidget  *template_entry, *rotate_sel, *new_window;
	glTemplate *template;
	glLabel    *label;
	gint        ret;
	glWindow   *window;

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

		template = gl_template_from_name (sheet_name);

		label = GL_LABEL(gl_label_new ());
		gl_label_set_template (label, template);
		gl_label_set_rotate_flag (label, rotate_flag);

		window =
			GL_WINDOW (g_object_get_data (G_OBJECT (dlg),
						      "parent_window"));
		if ( gl_window_is_empty (window) ) {
			gl_window_set_label (window, label);
		} else {
			new_window = gl_window_new_from_label (label);
			gtk_widget_show_all (new_window);
		}
		

		break;
	}

	gtk_widget_destroy (GTK_WIDGET (dlg));

	gl_debug (DEBUG_FILE, "END");
}

/*****************************************************************************/
/* "Open" menu callback.                                                     */
/*****************************************************************************/
void
gl_file_open (GtkWindow *window)
{
	GtkFileSelection *fsel;

	gl_debug (DEBUG_FILE, "START");

	g_return_if_fail (window != NULL);

	fsel = GTK_FILE_SELECTION (gtk_file_selection_new (_("Open")));
	gtk_window_set_transient_for (GTK_WINDOW (fsel), window);
	gtk_window_set_title (GTK_WINDOW (fsel), _("Open label"));

	g_object_set_data (G_OBJECT (fsel), "parent_window", window);

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
	gint              ret;
	GnomeRecentModel *recent;
	GtkWindow        *window;

	gl_debug (DEBUG_FILE, "START");

	g_return_if_fail (GTK_IS_FILE_SELECTION (fsel));

	/* get the filename */
	filename = g_strdup (gtk_file_selection_get_filename (fsel));

	if (!filename || g_file_test (filename, G_FILE_TEST_IS_DIR)) {

		dlg = gl_hig_alert_new (GTK_WINDOW(fsel),
					GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_WARNING,
					GTK_BUTTONS_CLOSE,
					_("Empty file name selection"),
					_("Please select a file or supply a valid file name"));

		gtk_dialog_run (GTK_DIALOG (dlg));
		gtk_widget_destroy (dlg);

	} else {

		if (!g_file_test (filename, G_FILE_TEST_IS_REGULAR)) {

			dlg = gl_hig_alert_new (GTK_WINDOW(fsel),
						GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_WARNING,
						GTK_BUTTONS_CLOSE,
						_("File does not exist"),
						_("Please select a file or supply a valid file name"));

			gtk_dialog_run (GTK_DIALOG (dlg));
			gtk_widget_destroy (dlg);


		} else {
		
			window = g_object_get_data (G_OBJECT(fsel),
						    "parent_window");

			if ( gl_file_open_real (filename, window) ) {
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
		     GtkWindow       *window)
{
	gl_debug (DEBUG_FILE, "");

	return gl_file_open_real (filename, window);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Open a file.                                                    */
/*---------------------------------------------------------------------------*/
gboolean
gl_file_open_real (const gchar     *filename,
		   GtkWindow       *window)
{
	gchar            *abs_filename;
	glLabel          *label;
	glXMLLabelStatus  status;
	GnomeRecentModel *recent;
	gint              ret;
	GtkWidget        *new_window;

	gl_debug (DEBUG_FILE, "START");

	abs_filename = gl_util_make_absolute (filename);
	label = gl_xml_label_open (abs_filename, &status);
	if (!label) {
		GtkWidget *dlg;
		gchar *primary_msg;

		gl_debug (DEBUG_FILE, "couldn't open file");

		primary_msg = g_strdup_printf (_("Could not open file \"%s\""),
					       filename);

		dlg = gl_hig_alert_new (window,
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

		if ( gl_window_is_empty (GL_WINDOW(window)) ) {
			gl_window_set_label (GL_WINDOW(window), label);
		} else {
			new_window = gl_window_new_from_label (label);
			gtk_widget_show_all (new_window);
		}

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
gl_file_save (glLabel   *label,
	      GtkWindow *window)
{
	glXMLLabelStatus  status;
	GError           *error = NULL;
	gchar            *filename = NULL;
	GnomeRecentModel *recent;

	gl_debug (DEBUG_FILE, "");

	g_return_val_if_fail (label != NULL, FALSE);
	
	if (gl_label_is_untitled (label))
	{
		gl_debug (DEBUG_FILE, "Untitled");

		return gl_file_save_as (label, window);
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

		dialog = gl_hig_alert_new (window,
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
gl_file_save_as (glLabel   *label,
		 GtkWindow *window)
{
	GtkFileSelection *fsel;
	gboolean          saved_flag = FALSE;
	gboolean          destroy_flag = FALSE;
	gchar            *name, *title;

	gl_debug (DEBUG_FILE, "START");

	g_return_val_if_fail (label && GL_IS_LABEL(label), FALSE);
	g_return_val_if_fail (window && GTK_IS_WINDOW(window), FALSE);

	name = gl_label_get_short_name (label);
	title = g_strdup_printf (_("Save \"%s\" as"), name);
	g_free (name);

	fsel = GTK_FILE_SELECTION (gtk_file_selection_new (title));
	gtk_window_set_modal (GTK_WINDOW (fsel), TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (fsel), window);

	g_object_set_data (G_OBJECT (fsel), "label", label);
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

	g_free (title);

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
	glLabel          *label;
	glXMLLabelStatus  status;
	GnomeRecentModel *recent;
	gboolean         *saved_flag;
	gchar            *primary_msg;
	gboolean          cancel_flag = FALSE;

	gl_debug (DEBUG_FILE, "START");

	g_return_if_fail (GTK_IS_FILE_SELECTION (fsel));

	label = g_object_get_data (G_OBJECT(fsel), "label");
	saved_flag = g_object_get_data (G_OBJECT(fsel), "saved_flag");

	/* get the filename */
	raw_filename = g_strdup (gtk_file_selection_get_filename (fsel));

	gl_debug (DEBUG_FILE, "raw_filename = \"%s\"", raw_filename);

	if (!raw_filename || g_file_test (raw_filename, G_FILE_TEST_IS_DIR)) {

		dlg = gl_hig_alert_new (GTK_WINDOW(fsel),
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

			dlg = gl_hig_alert_new (GTK_WINDOW(fsel),
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

				dlg = gl_hig_alert_new (GTK_WINDOW(fsel),
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
gboolean
gl_file_close (glWindow *window)
{
	glView  *view;
	glLabel *label;
	gboolean close = TRUE;

	gl_debug (DEBUG_FILE, "START");

	g_return_val_if_fail (window && GL_IS_WINDOW(window), TRUE);

	if ( !gl_window_is_empty (window) ) {

		view = GL_VIEW(window->view);
		label = view->label;

		if (gl_label_is_modified (label))	{
			GtkWidget *msgbox, *w;
			gchar *fname = NULL, *msg = NULL;
			gint ret;
			gboolean exiting;

			fname = gl_label_get_short_name (label);
			
			msg = g_strdup_printf (_("Save changes to document \"%s\" before closing?"),
					       fname);
			
			msgbox = gl_hig_alert_new (GTK_WINDOW(window),
						   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						   GTK_MESSAGE_WARNING,
						   GTK_BUTTONS_NONE,
						   msg,
						   _("Your changes will be lost if you don't save them."));

			gtk_dialog_add_button (GTK_DIALOG (msgbox),
					       _("Close without saving"),
					       GTK_RESPONSE_NO);

			gtk_dialog_add_button (GTK_DIALOG (msgbox),
					       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);

			gtk_dialog_add_button (GTK_DIALOG (msgbox),
					       GTK_STOCK_SAVE, GTK_RESPONSE_YES);

			gtk_dialog_set_default_response	(GTK_DIALOG (msgbox), GTK_RESPONSE_YES);

			gtk_window_set_resizable (GTK_WINDOW (msgbox), FALSE);

			ret = gtk_dialog_run (GTK_DIALOG (msgbox));
		
			gtk_widget_destroy (msgbox);

			g_free (fname);
			g_free (msg);
		
			switch (ret)
			{
			case GTK_RESPONSE_YES:
				close = gl_file_save (label,
						      GTK_WINDOW(window));
				break;
			case GTK_RESPONSE_NO:
				close = TRUE;
				break;
			default:
				close = FALSE;
			}

			gl_debug (DEBUG_FILE, "CLOSE: %s", close ? "TRUE" : "FALSE");
		}

	}

	if (close) {
		gtk_widget_destroy (GTK_WIDGET(window));

		if ( gl_window_get_window_list () == NULL ) {
			
			gl_debug (DEBUG_FILE, "All windows closed.");
	
			bonobo_main_quit ();
		}

	}

	gl_debug (DEBUG_FILE, "END");

	return close;
}

/*****************************************************************************/
/* "Exit" menu callback.                                                     */
/*****************************************************************************/
void
gl_file_exit (void)
{
	const GList *window_list;
	GList       *p, *p_next;

	gl_debug (DEBUG_FILE, "START");

	window_list = gl_window_get_window_list ();

	for (p=(GList *)window_list; p != NULL; p=p_next) {
		p_next = p->next;

		gl_file_close (GL_WINDOW(p->data));
	}

	gl_debug (DEBUG_FILE, "END");
}
