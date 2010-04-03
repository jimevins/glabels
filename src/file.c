/*
 *  file.c
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

#include "file.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <string.h>

#include "xml-label.h"
#include "recent.h"
#include "file-util.h"
#include "window.h"
#include "new-label-dialog.h"
#include <libglabels.h>

#include "debug.h"


/*===========================================*/
/* Private globals                           */
/*===========================================*/

/* Saved state for new dialog */
static gchar   *page_size   = NULL;
static gchar   *category    = NULL;
static gchar   *sheet_name  = NULL;
static gboolean rotate_flag = FALSE;

/* Saved state of file selectors */
static gchar *open_path = NULL;
static gchar *save_path = NULL;


/*===========================================*/
/* Local function prototypes.                */
/*===========================================*/
static void new_complete                     (GtkDialog         *dialog,
					      gpointer           user_data);

static void properties_complete              (GtkDialog         *dialog,
					      gpointer           user_data);

static void open_response                    (GtkDialog         *chooser,
					      gint               response,
					      glWindow          *window);
static void save_as_response                 (GtkDialog         *chooser,
					      gint               response,
					      glLabel           *label);


/*****************************************************************************/
/* "New" menu callback.                                                      */
/*****************************************************************************/
void
gl_file_new (glWindow  *window)
{
	GtkWidget    *dialog;

	gl_debug (DEBUG_FILE, "START");

	g_return_if_fail (window && GTK_IS_WINDOW (window));

	dialog = gl_new_label_dialog_new (GTK_WINDOW (window));
	gtk_window_set_title (GTK_WINDOW (dialog), _("New Label or Card"));

	g_object_set_data (G_OBJECT (dialog), "parent_window", window);

	g_signal_connect (G_OBJECT(dialog), "complete", G_CALLBACK (new_complete), dialog);

	if (page_size != NULL) {
		gl_new_label_dialog_set_filter_parameters (GL_NEW_LABEL_DIALOG (dialog),
							    page_size,
							    category);
	}
	if (sheet_name != NULL) {
		gl_new_label_dialog_set_template_name (GL_NEW_LABEL_DIALOG (dialog),
					  sheet_name);
	}
	gl_new_label_dialog_set_rotate_state (GL_NEW_LABEL_DIALOG (dialog), rotate_flag);

	gtk_widget_show_all (GTK_WIDGET (dialog));

	gl_debug (DEBUG_FILE, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  New "ok" button callback.                                       */
/*---------------------------------------------------------------------------*/
static void
new_complete (GtkDialog *dialog,
	      gpointer   user_data)
{
	lglTemplate *template;
	glLabel     *label;
	glWindow    *window;
	GtkWidget   *new_window;

	gl_debug (DEBUG_FILE, "START");

        gl_new_label_dialog_get_filter_parameters (GL_NEW_LABEL_DIALOG (dialog),
                                                   &page_size,
                                                   &category);

        if (sheet_name != NULL)
        {
                g_free (sheet_name);
        }

        sheet_name = gl_new_label_dialog_get_template_name (GL_NEW_LABEL_DIALOG (dialog));

        rotate_flag = gl_new_label_dialog_get_rotate_state (GL_NEW_LABEL_DIALOG (dialog));

        template = lgl_db_lookup_template_from_name (sheet_name);

        label = GL_LABEL(gl_label_new ());
        gl_label_set_template (label, template, FALSE);
        gl_label_set_rotate_flag (label, rotate_flag, FALSE);

        lgl_template_free (template);

        window = GL_WINDOW (g_object_get_data (G_OBJECT (dialog), "parent_window"));
        if ( gl_window_is_empty (window) )
        {
                gl_window_set_label (window, label);
        }
        else
        {
                new_window = gl_window_new_from_label (label);
                gtk_widget_show_all (new_window);
        }
		
	gl_debug (DEBUG_FILE, "END");
}


/*****************************************************************************/
/* "Properties" menu callback.                                               */
/*****************************************************************************/
void
gl_file_properties (glLabel   *label,
		    glWindow  *window)
{
        const lglTemplate *template;
        gboolean           rotate_flag;
	GtkWidget         *dialog;
        gchar             *name;

	gl_debug (DEBUG_FILE, "START");

        g_return_if_fail (label && GL_IS_LABEL (label));
	g_return_if_fail (window && GTK_IS_WINDOW (window));

	dialog = gl_new_label_dialog_new (GTK_WINDOW (window));
	gtk_window_set_title (GTK_WINDOW (dialog), _("Label properties"));

	g_object_set_data (G_OBJECT (dialog), "label", label);

	g_signal_connect (G_OBJECT(dialog), "complete", G_CALLBACK (properties_complete), dialog);

        template    = gl_label_get_template (label);
        rotate_flag = gl_label_get_rotate_flag (label);

        if (template->paper_id != NULL) {
                gl_new_label_dialog_set_filter_parameters (GL_NEW_LABEL_DIALOG (dialog),
                                                           template->paper_id,
                                                           NULL);
        }
        name = lgl_template_get_name (template);
        if (name != NULL) {
                gl_new_label_dialog_set_template_name (GL_NEW_LABEL_DIALOG (dialog), name);
        }
        g_free (name);

        gl_new_label_dialog_set_rotate_state (GL_NEW_LABEL_DIALOG (dialog), rotate_flag);

	gtk_widget_show_all (GTK_WIDGET (dialog));

	gl_debug (DEBUG_FILE, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Properties "ok" button callback.                                */
/*---------------------------------------------------------------------------*/
static void
properties_complete (GtkDialog *dialog,
                     gpointer   user_data)
{
	lglTemplate *template;
	glLabel     *label;

	gl_debug (DEBUG_FILE, "START");

        gl_new_label_dialog_get_filter_parameters (GL_NEW_LABEL_DIALOG (dialog),
                                                   &page_size,
                                                   &category);

        if (sheet_name != NULL)
        {
                g_free (sheet_name);
        }

        sheet_name = gl_new_label_dialog_get_template_name (GL_NEW_LABEL_DIALOG (dialog));

        rotate_flag = gl_new_label_dialog_get_rotate_state (GL_NEW_LABEL_DIALOG (dialog));

        template = lgl_db_lookup_template_from_name (sheet_name);

        label = GL_LABEL(g_object_get_data (G_OBJECT (dialog), "label"));

        gl_label_set_template (label, template, TRUE);
        gl_label_set_rotate_flag (label, rotate_flag, TRUE);

	gl_debug (DEBUG_FILE, "END");
}


/*****************************************************************************/
/* "Open" menu callback.                                                     */
/*****************************************************************************/
void
gl_file_open (glWindow  *window)
{
	GtkWidget     *chooser;
	GtkFileFilter *filter;

	gl_debug (DEBUG_FILE, "START");

	g_return_if_fail (window != NULL);

	chooser = gtk_file_chooser_dialog_new ("Open label",
					       GTK_WINDOW (window),
					       GTK_FILE_CHOOSER_ACTION_OPEN,
					       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					       GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					       NULL);

	/* Recover state of open dialog */
	if (open_path != NULL) {
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser),
						     open_path);
	}

	filter = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_filter_set_name (filter, _("All files"));
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter, "*.glabels");
	gtk_file_filter_set_name (filter, _("gLabels documents"));
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (chooser), filter);

	g_signal_connect (G_OBJECT (chooser), "response",
			  G_CALLBACK (open_response), window);

	/* show the dialog */
	gtk_widget_show (GTK_WIDGET (chooser));

	gl_debug (DEBUG_FILE, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Open "response" callback.                                       */
/*---------------------------------------------------------------------------*/
static void
open_response (GtkDialog     *chooser,
	       gint           response,
	       glWindow      *window)
{
	gchar            *raw_filename;
	gchar 		 *filename;
	GtkWidget        *dialog;

	gl_debug (DEBUG_FILE, "START");

	g_return_if_fail (chooser && GTK_IS_FILE_CHOOSER (chooser));
	g_return_if_fail (window && GTK_IS_WINDOW (window));

	switch (response) {

	case GTK_RESPONSE_ACCEPT:
		/* get the filename */
		raw_filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(chooser));
		filename = g_filename_to_utf8 (raw_filename, -1, NULL, NULL, NULL);

		if (!raw_filename || 
		    !filename || 
		    g_file_test (raw_filename, G_FILE_TEST_IS_DIR)) {

			dialog = gtk_message_dialog_new (GTK_WINDOW(chooser),
						      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						      GTK_MESSAGE_WARNING,
						      GTK_BUTTONS_CLOSE,
						      _("Empty file name selection"));
			gtk_message_dialog_format_secondary_text (
				GTK_MESSAGE_DIALOG (dialog),
				_("Please select a file or supply a valid file name"));

			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);

		} else {

			if (!g_file_test (raw_filename, G_FILE_TEST_IS_REGULAR)) {

				dialog = gtk_message_dialog_new (GTK_WINDOW(chooser),
							      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
							      GTK_MESSAGE_WARNING,
							      GTK_BUTTONS_CLOSE,
							      _("File does not exist"));
				gtk_message_dialog_format_secondary_text (
					GTK_MESSAGE_DIALOG (dialog),
					_("Please select a file or supply a valid file name"));

				gtk_dialog_run (GTK_DIALOG (dialog));
				gtk_widget_destroy (dialog);


			} else {
		
				if ( gl_file_open_real (filename, window) ) {
					gtk_widget_destroy (GTK_WIDGET (chooser));
				}

			}

		}

		g_free (filename);
		g_free (raw_filename);
		break;

	default:
		gtk_widget_destroy (GTK_WIDGET (chooser));
		break;

	}

	gl_debug (DEBUG_FILE, "END");
}


/*****************************************************************************/
/* "Open recent" menu callback.                                              */
/*****************************************************************************/
void
gl_file_open_recent (const gchar     *filename,
		     glWindow        *window)
{
	gl_debug (DEBUG_FILE, "");

	if (filename) {
		gl_debug (DEBUG_FILE, "open recent: %s", filename);

		gl_file_open_real (filename, window);
	}
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Open a file.                                                    */
/*---------------------------------------------------------------------------*/
gboolean
gl_file_open_real (const gchar     *filename,
		   glWindow        *window)
{
	gchar            *abs_filename;
	glLabel          *label;
	glXMLLabelStatus  status;
	GtkWidget        *new_window;

	gl_debug (DEBUG_FILE, "START");

	abs_filename = gl_file_util_make_absolute (filename);
	label = gl_xml_label_open (abs_filename, &status);
	if (!label) {
		GtkWidget *dialog;

		gl_debug (DEBUG_FILE, "couldn't open file");

		dialog = gtk_message_dialog_new (GTK_WINDOW (window),
					      GTK_DIALOG_DESTROY_WITH_PARENT,
					      GTK_MESSAGE_ERROR,
					      GTK_BUTTONS_CLOSE,
					      _("Could not open file \"%s\""),
					      filename);
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
							  _("Not a supported file format"));

		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);

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

		gl_recent_add_utf8_filename (abs_filename);

		if (open_path != NULL)
			g_free (open_path);
		open_path = g_path_get_dirname (abs_filename);
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
	      glWindow  *window)
{
	glXMLLabelStatus  status;
	gchar            *filename = NULL;

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

		gl_debug (DEBUG_FILE, "FAILED");

		dialog = gtk_message_dialog_new (GTK_WINDOW (window),
					      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					      GTK_MESSAGE_ERROR,
					      GTK_BUTTONS_CLOSE,
					      _("Could not save file \"%s\""),
					      filename);
		gtk_message_dialog_format_secondary_text (
			GTK_MESSAGE_DIALOG (dialog),
			_("Error encountered during save.  The file is still not saved."));

		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);

		g_free (filename);

		return FALSE;
	}	
	else
	{
		gl_debug (DEBUG_FILE, "OK");

		gl_recent_add_utf8_filename (filename);

		g_free (filename);

		return TRUE;
	}
}


/*****************************************************************************/
/* "Save As" menu callback.                                                  */
/*****************************************************************************/
gboolean
gl_file_save_as (glLabel   *label,
		 glWindow  *window)
{
	GtkWidget        *chooser;
	GtkFileFilter    *filter;
	gboolean          saved_flag = FALSE;
	gchar            *name, *title;

	gl_debug (DEBUG_FILE, "START");

	g_return_val_if_fail (label && GL_IS_LABEL(label), FALSE);
	g_return_val_if_fail (window && GL_IS_WINDOW(window), FALSE);

	name = gl_label_get_short_name (label);
	title = g_strdup_printf (_("Save \"%s\" as"), name);
	g_free (name);

	chooser = gtk_file_chooser_dialog_new (title,
					       GTK_WINDOW (window),
					       GTK_FILE_CHOOSER_ACTION_SAVE,
					       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					       GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					       NULL);

	gtk_window_set_modal (GTK_WINDOW (chooser), TRUE);

	g_free (title);

	/* Recover proper state of save-as dialog */
	if (save_path != NULL) {
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser),
						     save_path);
	}

	filter = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_filter_set_name (filter, _("All files"));
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter, "*.glabels");
	gtk_file_filter_set_name (filter, _("gLabels documents"));
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (chooser), filter);

	g_signal_connect (G_OBJECT (chooser), "response",
			  G_CALLBACK (save_as_response), label);

	g_object_set_data (G_OBJECT (chooser), "saved_flag", &saved_flag);

	/* show the dialog */
	gtk_widget_show (GTK_WIDGET (chooser));

	/* Hold here and process events until we are done with this dialog. */
	/* This is so we can return a boolean result of our save attempt.   */
	gtk_main ();

	gl_debug (DEBUG_FILE, "END");

	/* Return flag as set by one of the above callbacks, TRUE = saved */
	return saved_flag;
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Save As" ok button callback.                                   */
/*---------------------------------------------------------------------------*/
static void
save_as_response (GtkDialog     *chooser,
		  gint           response,
		  glLabel       *label)
{
	gchar            *raw_filename, *filename, *full_filename;
	GtkWidget        *dialog;
	glXMLLabelStatus  status;
	gboolean         *saved_flag;
	gboolean          cancel_flag = FALSE;

	gl_debug (DEBUG_FILE, "START");

	g_return_if_fail (GTK_IS_FILE_CHOOSER (chooser));

	saved_flag = g_object_get_data (G_OBJECT(chooser), "saved_flag");

	switch (response) {

	case GTK_RESPONSE_ACCEPT:
		/* get the filename */
		raw_filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(chooser));

		gl_debug (DEBUG_FILE, "raw_filename = \"%s\"", raw_filename);

		if (!raw_filename || g_file_test (raw_filename, G_FILE_TEST_IS_DIR)) {

			dialog = gtk_message_dialog_new (GTK_WINDOW(chooser),
						      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						      GTK_MESSAGE_WARNING,
						      GTK_BUTTONS_CLOSE,
						      _("Empty file name selection"));
			gtk_message_dialog_format_secondary_text (
				GTK_MESSAGE_DIALOG (dialog),
				_("Please supply a valid file name"));

			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);

		} else {

			full_filename = gl_file_util_add_extension (raw_filename);

			filename = g_filename_to_utf8 (full_filename, -1,
						       NULL, NULL, NULL);

			gl_debug (DEBUG_FILE, "filename = \"%s\"", filename);

			if (g_file_test (full_filename, G_FILE_TEST_IS_REGULAR)) {
				gint ret;

				dialog = gtk_message_dialog_new (GTK_WINDOW(chooser),
							      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
							      GTK_MESSAGE_QUESTION,
							      GTK_BUTTONS_YES_NO,
							      _("Overwrite file \"%s\"?"),
							       filename);
				gtk_message_dialog_format_secondary_text (
					GTK_MESSAGE_DIALOG (dialog),
					_("File already exists."));

				ret = gtk_dialog_run (GTK_DIALOG (dialog));
				if ( ret == GTK_RESPONSE_NO ) {
					cancel_flag = TRUE;
				}
				gtk_widget_destroy (dialog);
			}

			if (!cancel_flag) {

				gl_xml_label_save (label, filename, &status);

				gl_debug (DEBUG_FILE, "status of save = %d", status);

				if ( status != XML_LABEL_OK ) {

					dialog = gtk_message_dialog_new (
						GTK_WINDOW(chooser),
						GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_ERROR,
						GTK_BUTTONS_CLOSE,
						_("Could not save file \"%s\""),
						filename);
					gtk_message_dialog_format_secondary_text (
						GTK_MESSAGE_DIALOG (dialog),
						_("Error encountered during save.  The file is still not saved."));

					gtk_dialog_run (GTK_DIALOG (dialog));
					gtk_widget_destroy (dialog);

				} else {

					*saved_flag = TRUE;

					gl_recent_add_utf8_filename (filename);

					if (save_path != NULL)
						g_free (save_path);
					save_path = g_path_get_dirname (filename);

					gtk_widget_destroy (GTK_WIDGET (chooser));
					gtk_main_quit ();
				}

			}

			g_free (filename);
			g_free (full_filename);
		}

		g_free (raw_filename);
		break;

	default:
		*saved_flag = FALSE;
		gtk_widget_destroy (GTK_WIDGET (chooser));
		gtk_main_quit ();
		break;

	}

	gl_debug (DEBUG_FILE, "END");
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
			GtkWidget *dialog;
			gchar *fname = NULL;
			gint ret;

			fname = gl_label_get_short_name (label);
			
			dialog = gtk_message_dialog_new (GTK_WINDOW(window),
							 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
							 GTK_MESSAGE_WARNING,
							 GTK_BUTTONS_NONE,
							 _("Save changes to document \"%s\" before closing?"),
							 fname);
			gtk_message_dialog_format_secondary_text (
				GTK_MESSAGE_DIALOG (dialog),
				_("Your changes will be lost if you don't save them."));

			gtk_dialog_add_button (GTK_DIALOG (dialog),
					       _("Close without saving"),
					       GTK_RESPONSE_NO);

			gtk_dialog_add_button (GTK_DIALOG (dialog),
					       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);

			gtk_dialog_add_button (GTK_DIALOG (dialog),
					       GTK_STOCK_SAVE, GTK_RESPONSE_YES);

			gtk_dialog_set_default_response	(GTK_DIALOG (dialog), GTK_RESPONSE_YES);

			gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

			ret = gtk_dialog_run (GTK_DIALOG (dialog));
		
			gtk_widget_destroy (dialog);

			g_free (fname);
		
			switch (ret)
			{
			case GTK_RESPONSE_YES:
				close = gl_file_save (label, window);
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
	
			gtk_main_quit ();
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



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
