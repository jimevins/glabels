/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  mdi.c:  GLabels MDI module
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

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#include "mdi.h"
#include "menu.h"
#include "display.h"
#include "label.h"
#include "util.h"

#define DONT_DISPLAY_EXTENSION

/*============================================*/
/* Private types.                             */
/*============================================*/
typedef struct _Document Document;
struct _Document {
	glLabel *label;
	gchar *filename;
	gint id;
};

/*============================================*/
/* Private globals.                           */
/*============================================*/
static GnomeMDI *mdi = NULL;
static gint i_unnamed = 0;
static gint next_id = 0;

/*===========================================*/
/* Private function prototypes.              */
/*===========================================*/
static void set_filename (GnomeMDIChild * child, const gchar * filename);

static gboolean remove_child_handler (GnomeMDI * mdi, GnomeMDIChild * child,
				      gpointer data);

static GnomeMDIChild *child_creator (const char *name, glLabel * label,
				     const char *abs_filename);

static void child_destroyed (GnomeMDIChild * child, gpointer data);

static void app_created_handler (GnomeMDI * mdi, GnomeApp * app, gpointer data);

static GtkWidget *view_creator (GnomeMDIChild * child, gpointer data);

static gchar *construct_name (GnomeMDIChild * child, const gchar * filename);

static gboolean close_save (GnomeMDIChild * child);

static gboolean close_save_as_dialog (GnomeMDIChild * child);

static void close_save_as_ok_cb (GtkWidget * widget, GtkFileSelection * fsel);

static void close_save_as_cancel_cb (GtkWidget * widget, GtkFileSelection * fsel);

static void close_save_as_destroy_cb (GtkWidget * widget, gboolean * destroy_flag);

/*--------------------------------------------------------------------------*/
/* Initialize mdi module.                                                   */
/*--------------------------------------------------------------------------*/
GnomeMDI *
gl_mdi_init (void)
{
	mdi = GNOME_MDI (gnome_mdi_new (PACKAGE, "gLabels"));

	gl_menu_install_menus (mdi);
	gl_menu_install_toolbar (mdi);

	gtk_signal_connect (GTK_OBJECT (mdi), "destroy",
			    GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

	gtk_signal_connect (GTK_OBJECT (mdi), "remove_child",
			    GTK_SIGNAL_FUNC (remove_child_handler), NULL);

	gtk_signal_connect (GTK_OBJECT (mdi), "app-created",
			    GTK_SIGNAL_FUNC (app_created_handler), NULL);

	return mdi;
}

/*--------------------------------------------------------------------------*/
/* Get active window/app.                                                   */
/*--------------------------------------------------------------------------*/
GnomeApp *
gl_mdi_get_active_window (void)
{
	return gnome_mdi_get_active_window (mdi);
}

/*--------------------------------------------------------------------------*/
/* Create a new view/child with a blank document.                           */
/*--------------------------------------------------------------------------*/
gboolean
gl_mdi_new_child (const gchar * tmplt_name,
		  gboolean rotate_flag)
{
	gchar *name = NULL;
	GnomeMDIChild *child;

	i_unnamed++;
	name = g_strdup_printf ("Label%d", i_unnamed);
	child = child_creator (name,
			       gl_label_new_with_template (tmplt_name,
							   rotate_flag), NULL);
	g_free (name);
	if (child == NULL) {
		return FALSE;
	}

	gnome_mdi_add_child (mdi, child);
	gnome_mdi_add_view (mdi, child);

	return TRUE;
}

/*--------------------------------------------------------------------------*/
/* Create a new view, from xml document.                                    */
/*--------------------------------------------------------------------------*/
gboolean
gl_mdi_new_child_from_xml_file (const gchar * filename)
{
	GList *p_child;
	GnomeMDIChild *child;
	Document *document;
	gchar *abs_filename, *name;
	glLabel *label = NULL;
	glLabelStatus status;
	GtkWidget *wdialog;
	gchar *msg;
	GnomeApp *app;

	abs_filename = gl_util_make_absolute (filename);

	for (p_child = mdi->children; p_child != NULL; p_child = p_child->next) {
		child = GNOME_MDI_CHILD (p_child->data);
		document = gtk_object_get_user_data (GTK_OBJECT (child));
		if (document->filename != NULL) {
			if (strcmp (document->filename, abs_filename) == 0) {
				msg =
				    g_strdup_printf (_
						     ("Revert to saved copy of %s?"),
						     abs_filename);
				wdialog =
				    gnome_message_box_new (msg,
							   GNOME_MESSAGE_BOX_QUESTION,
							   GNOME_STOCK_BUTTON_YES,
							   GNOME_STOCK_BUTTON_NO,
							   NULL);
				g_free (msg);
				app =
				    gnome_mdi_get_app_from_view (GTK_WIDGET
								 (child->views->
								  data));
				gtk_window_set_transient_for (GTK_WINDOW
							      (wdialog),
							      GTK_WINDOW (app));
				if (gnome_dialog_run (GNOME_DIALOG (wdialog)) ==
				    0) {
					gnome_mdi_remove_child (mdi, child,
								FALSE);
					break;
				} else {
					g_free (abs_filename);
					return TRUE;
				}
			}
		}
	}

	status = gl_label_open_xml (&label, abs_filename);
	if (status < LABEL_OK) {
		g_free (abs_filename);
		return FALSE;
	}
	if (status == LABEL_UNKNOWN_MEDIA) {
		app = gl_mdi_get_active_window ();
		wdialog =
		    gnome_error_dialog_parented (_
						 ("Unknown media type. Using default."),
						 GTK_WINDOW (app));
		gtk_window_set_modal (GTK_WINDOW (wdialog), TRUE);
	}

	name = construct_name (NULL, abs_filename);
	child = child_creator (name, label, abs_filename);
	g_free (name);
	if (child == NULL) {
		g_free (abs_filename);
		return FALSE;
	}

	gnome_mdi_add_child (mdi, child);
	gnome_mdi_add_view (mdi, child);

	g_free (abs_filename);
	return TRUE;
}

/*--------------------------------------------------------------------------*/
/* Save view to xml file.                                                   */
/*--------------------------------------------------------------------------*/
gboolean
gl_mdi_save_child_to_xml_file (GnomeMDIChild * child,
			       const gchar * filename)
{
	glDisplay *display;
	glLabel *label;

	display = gl_mdi_get_display (child);
	label = display->label;
	if (gl_label_save_xml (label, filename) == LABEL_OK) {
		gl_display_clear_modified (display);
		set_filename (child, filename);
		return TRUE;
	} else {
		return FALSE;
	}

}

/*--------------------------------------------------------------------------*/
/* Close view.                                                              */
/*--------------------------------------------------------------------------*/
void
gl_mdi_close_child (GnomeMDIChild * child)
{
	if (child != NULL) {
		gnome_mdi_remove_child (mdi, child, FALSE);
	}
}

/*--------------------------------------------------------------------------*/
/* Close all views, and exit if successful.                                  */
/*--------------------------------------------------------------------------*/
void
gl_mdi_close_all (void)
{
	if (gnome_mdi_remove_all (mdi, FALSE)) {
		gtk_object_destroy (GTK_OBJECT (mdi));
	}
}

/*--------------------------------------------------------------------------*/
/* PRIVATE. Set filename associated with document.                          */
/*--------------------------------------------------------------------------*/
void
set_filename (GnomeMDIChild * child,
	      const gchar * filename)
{
	Document *document;
	gchar *name;

	document = gtk_object_get_user_data (GTK_OBJECT (child));

	g_free (document->filename);
	document->filename = g_strdup (filename);

	name = construct_name (child, filename);
	gnome_mdi_child_set_name (child, name);
	g_free (name);
}

/*--------------------------------------------------------------------------*/
/* Get filename associated with document.                                   */
/*--------------------------------------------------------------------------*/
gchar *
gl_mdi_get_filename (GnomeMDIChild * child)
{
	Document *document;

	document = gtk_object_get_user_data (GTK_OBJECT (child));

	return g_strdup (document->filename);
}

/*--------------------------------------------------------------------------*/
/* Get base filename of document.                                           */
/*--------------------------------------------------------------------------*/
gchar *
gl_mdi_get_basename (GnomeMDIChild * child)
{
	Document *document;

	document = gtk_object_get_user_data (GTK_OBJECT (child));

	if (document->filename != NULL) {
		return g_strdup (g_basename (document->filename));
	} else {
		return NULL;
	}
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
GList *
gl_mdi_get_displays_in_current_window (void)
{
	GnomeApp *app;
	GList *p_child;
	GnomeMDIChild *child;
	GtkWidget *view;
	GList *views = NULL;

	app = gnome_mdi_get_active_window (mdi);

	for (p_child = mdi->children; p_child != NULL; p_child = p_child->next) {
		child = GNOME_MDI_CHILD (p_child->data);
		view = GTK_WIDGET (child->views->data);
		if (gnome_mdi_get_app_from_view (view) == app) {
			views = g_list_prepend (views, view);
		}
	}

	return views;
}

/*--------------------------------------------------------------------------*/
/* Get label display associated with document.                              */
/*--------------------------------------------------------------------------*/
glDisplay *
gl_mdi_get_display (GnomeMDIChild * child)
{
	if (child == NULL)
		return NULL;
	if (child->views == NULL)
		return NULL;
	return GL_DISPLAY (child->views->data);
}

/*--------------------------------------------------------------------------*/
/* Get label associated with document.                                      */
/*--------------------------------------------------------------------------*/
glLabel *
gl_mdi_get_label (GnomeMDIChild * child)
{
	glDisplay *display;

	display = gl_mdi_get_display (child);
	return display->label;
}

/*--------------------------------------------------------------------------*/
/* Get uniq id associated with document.                                    */
/*--------------------------------------------------------------------------*/
gint
gl_mdi_get_id (GnomeMDIChild * child)
{
	Document *document;

	document = gtk_object_get_user_data (GTK_OBJECT (child));

	return document->id;
}

/*--------------------------------------------------------------------------*/
/* Find child associated with given id.  This allows us to verify that a    */
/* child still exists from a non-modal dialog, e.g. if you close a child    */
/* while you have a print or save as dialog open for that child.            */
/*--------------------------------------------------------------------------*/
GnomeMDIChild *
gl_mdi_find_child (gint id)
{
	GList *p_child;
	GnomeMDIChild *child;
	Document *document;

	for (p_child = mdi->children; p_child != NULL; p_child = p_child->next) {
		child = GNOME_MDI_CHILD (p_child->data);
		document = gtk_object_get_user_data (GTK_OBJECT (child));
		if (document->id == id) {
			return child;
		}
	}

	return NULL;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Create a new child.                                            */
/*--------------------------------------------------------------------------*/
static GnomeMDIChild *
child_creator (const char *name,
	       glLabel * label,
	       const char *abs_filename)
{
	GnomeMDIGenericChild *child;
	Document *document;

	document = g_new0 (Document, 1);

	document->label = label;
	document->filename = g_strdup (abs_filename);
	document->id = next_id++;

	child = gnome_mdi_generic_child_new (name);

	gtk_object_set_user_data (GTK_OBJECT (child), document);

	gnome_mdi_generic_child_set_view_creator (child, view_creator, NULL);

	gtk_signal_connect (GTK_OBJECT (child), "destroy",
			    GTK_SIGNAL_FUNC (child_destroyed), document);

	return GNOME_MDI_CHILD (child);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Handle remove child signal.                                    */
/*--------------------------------------------------------------------------*/
static gboolean
remove_child_handler (GnomeMDI * mdi,
		      GnomeMDIChild * child,
		      gpointer data)
{
	GtkWidget *wdialog;
	gchar *msg;
	GnomeApp *app = gnome_mdi_get_active_window (mdi);
	Document *document;
	glDisplay *display;

	display = gl_mdi_get_display (child);
	document = gtk_object_get_user_data (GTK_OBJECT (child));

	if (gl_display_modified (display)) {
		msg =
		    g_strdup_printf (_
				     ("``%s'' has been modified.\n\nDo you wish to save it?"),
				     child->name);
		wdialog =
		    gnome_message_box_new (msg, GNOME_MESSAGE_BOX_WARNING,
					   "Save", _("Don't save"),
					   GNOME_STOCK_BUTTON_CANCEL, NULL);
		g_free (msg);
		gtk_window_set_transient_for (GTK_WINDOW (wdialog),
					      GTK_WINDOW (app));
		switch (gnome_dialog_run (GNOME_DIALOG (wdialog))) {

		case 0:	/* Yes / Save */
			if (document->filename != NULL) {
				return close_save (child);
			} else {
				return close_save_as_dialog (child);
			}

		case 1:	/* No / Don't save */
			return TRUE;

		default:	/* Cancel operation */
			return FALSE;
		}
	} else {
		return TRUE;
	}
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Handle destroy child signal.                                   */
/*--------------------------------------------------------------------------*/
static void
child_destroyed (GnomeMDIChild * child,
		 gpointer data)
{
	Document *document = data;

	gl_label_free (&document->label);
	g_free (document->filename);
	document->filename = NULL;

	g_free (document);
	gtk_object_set_user_data (GTK_OBJECT (child), NULL);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Handle "app-created" signal.                                   */
/*--------------------------------------------------------------------------*/
static void
app_created_handler (GnomeMDI * mdi,
		     GnomeApp * app,
		     gpointer data)
{
	GtkWidget *status;

	status = gnome_appbar_new (FALSE, TRUE, GNOME_PREFERENCES_NEVER);
	gnome_app_set_statusbar (app, status);

	gnome_app_install_menu_hints (app,
				      gnome_mdi_get_menubar_info (mdi->
								  active_window));

	gl_menu_install_tools_toolbar (mdi, app);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  View creator.                                                  */
/*--------------------------------------------------------------------------*/
static GtkWidget *
view_creator (GnomeMDIChild * child,
	      gpointer data)
{
	GtkWidget *widget;
	Document *document;

	document = gtk_object_get_user_data (GTK_OBJECT (child));

	widget = gl_display_new (document->label);
	gtk_widget_show_all (widget);

	return widget;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct a child name, based on filename.                     */
/*--------------------------------------------------------------------------*/
static gchar *
construct_name (GnomeMDIChild * my_child,
		const gchar * filename)
{
	gchar *name, *test_name, *final_name;
	gint test_name_length;
	GList *p_child;
	gint duplicates = 0;
	GnomeMDIChild *child;

	name = g_strdup (g_basename (filename));
#ifdef DONT_DISPLAY_EXTENSION
	{
		gchar *p_extension = strrchr (name, '.');
		if (p_extension != NULL)
			*p_extension = 0;	/* wipe out extension */
	}
#endif
	test_name = g_strdup_printf ("%s <", name);
	test_name_length = strlen (test_name);

	for (p_child = mdi->children; p_child != NULL; p_child = p_child->next) {
		child = GNOME_MDI_CHILD (p_child->data);
		if (my_child != child) {
			if (strcmp (child->name, name) == 0) {
				duplicates++;
			}
			if (strncmp (child->name, test_name, test_name_length)
			    == 0) {
				duplicates++;
			}
		}
	}
	g_free (test_name);

	if (duplicates > 0) {
		final_name = g_strdup_printf ("%s <%d>", name, duplicates + 1);
	} else {
		final_name = g_strdup (name);
	}

	g_free (name);
	return final_name;

}

/*=================================================================*/
/* PRIVATE.  Save a document on close.                             */
/*=================================================================*/
static gboolean
close_save (GnomeMDIChild * child)
{
	Document *document = gtk_object_get_user_data (GTK_OBJECT (child));
	GtkWidget *dlg;
	GnomeApp *app = gnome_mdi_get_active_window (mdi);

	if (gl_mdi_save_child_to_xml_file (child, document->filename)) {
		return TRUE;
	} else {
		dlg = gnome_error_dialog_parented (_("Cannot save file"),
						   GTK_WINDOW (app));
		gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);
		return FALSE;
	}
}

/*=================================================================*/
/* PRIVATE.  Special modal "Save As" dialog for closing children.  */
/*=================================================================*/
static gboolean
close_save_as_dialog (GnomeMDIChild * child)
{
	GtkFileSelection *fsel;
	GnomeApp *app = gnome_mdi_get_active_window (mdi);
	gint id;
	gboolean flag = FALSE;
	gboolean destroy_flag = FALSE;

	g_return_val_if_fail (child != NULL, FALSE);
	g_return_val_if_fail (app != NULL, FALSE);

	fsel = GTK_FILE_SELECTION (gtk_file_selection_new (_("Save As")));
	gtk_window_set_modal (GTK_WINDOW (fsel), TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (fsel), GTK_WINDOW (app));
	gtk_window_set_title (GTK_WINDOW (fsel), _("Close / Save label as"));

	id = gl_mdi_get_id (child);
	gtk_object_set_data (GTK_OBJECT (fsel), "id", GINT_TO_POINTER (id));
	gtk_object_set_data (GTK_OBJECT (fsel), "flag", &flag);

	gtk_signal_connect (GTK_OBJECT (fsel->ok_button), "clicked",
			    GTK_SIGNAL_FUNC (close_save_as_ok_cb), fsel);

	gtk_signal_connect (GTK_OBJECT (fsel->cancel_button), "clicked",
			    GTK_SIGNAL_FUNC (close_save_as_cancel_cb), fsel);

	gtk_signal_connect (GTK_OBJECT (fsel), "destroy",
			    GTK_SIGNAL_FUNC (close_save_as_destroy_cb),
			    &destroy_flag);

	/* show the dialog */
	gtk_widget_show (GTK_WIDGET (fsel));

	/* Hold here and process events until we are done with this dialog. */
	gtk_main ();

	/* Destroy dialog if not already destroyed. */
	if (!destroy_flag) {
		/* Disconnect our destroy callback first, so that we don't kill the
		 * current gtk_main() loop. */
		gtk_signal_disconnect_by_func (GTK_OBJECT (fsel),
					       GTK_SIGNAL_FUNC
					       (close_save_as_destroy_cb),
					       &destroy_flag);
		gtk_widget_destroy (GTK_WIDGET (fsel));
	}

	/* Return flag as set by one of the above callbacks, TRUE = saved */
	return flag;
}

/*==============================================*/
/* PRIVATE.  "Save As" ok button callback.      */
/*==============================================*/
static void
close_save_as_ok_cb (GtkWidget * widget,
		     GtkFileSelection * fsel)
{
	gchar *raw_filename, *filename;
	GtkWidget *dlg;
	gint id =
	    GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (fsel), "id"));
	GnomeMDIChild *child = gl_mdi_find_child (id);
	gboolean *flag = gtk_object_get_data (GTK_OBJECT (fsel), "flag");

	g_return_if_fail (GTK_IS_FILE_SELECTION (fsel));

	if (child == NULL) {
		dlg = gnome_error_dialog (_("Label no longer valid!"));
		gtk_window_set_modal (GTK_WINDOW (dlg), TRUE);
		*flag = TRUE;
		gtk_widget_hide (GTK_WIDGET (fsel));
		gtk_main_quit ();
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
			*flag = TRUE;
			gtk_widget_hide (GTK_WIDGET (fsel));
			gtk_main_quit ();
		}

		g_free (filename);
	}

	g_free (raw_filename);
}

/*==============================================*/
/* PRIVATE.  "Save As" cancel button callback.  */
/*==============================================*/
static void
close_save_as_cancel_cb (GtkWidget * widget,
			 GtkFileSelection * fsel)
{
	gboolean *flag = gtk_object_get_data (GTK_OBJECT (fsel), "flag");

	g_return_if_fail (GTK_IS_FILE_SELECTION (fsel));

	*flag = FALSE;
	gtk_widget_hide (GTK_WIDGET (fsel));
	gtk_main_quit ();
}

/*==============================================*/
/* PRIVATE.  "Save As" destroy callback.        */
/*==============================================*/
static void
close_save_as_destroy_cb (GtkWidget * widget,
			  gboolean * destroy_flag)
{
	*destroy_flag = TRUE;
	gtk_main_quit ();
}
