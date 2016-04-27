/*
 *  window.c
 *  Copyright (C) 2002-2009  Jim Evins <evins@snaught.com>.
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

#include "window.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "ui.h"
#include "ui-commands.h"
#include "file-util.h"
#include "xml-label.h"
#include "prefs.h"
#include "file.h"
#include "units-util.h"

#include "debug.h"


/*===========================================================================*/
/* Private macros and constants.                                             */
/*===========================================================================*/

#define DEFAULT_WINDOW_WIDTH  788
#define DEFAULT_WINDOW_HEIGHT 600

#define CURSOR_INFO_WIDTH     150
#define ZOOM_INFO_WIDTH        75


/*===========================================================================*/
/* Private globals                                                           */
/*===========================================================================*/

static GList *window_list = NULL;


/*===========================================================================*/
/* Local function prototypes                                                 */
/*===========================================================================*/

static void     gl_window_finalize     (GObject       *object);
static void     gl_window_dispose      (GObject       *object);

static void     set_window_title       (glWindow *window,
					glLabel  *label);

static gboolean window_delete_event_cb (glWindow      *window,
					GdkEvent      *event,
					gpointer       user_data);

static void     selection_changed_cb   (glLabel       *label,
					glWindow      *window);

static void   context_menu_activate_cb (glView       *view,
					gint          button,
					guint32       activate_time,
					glWindow     *window);

static void     zoom_changed_cb        (glView        *view,
					gdouble        zoom,
					glWindow      *window);

static void     pointer_moved_cb       (glView        *view,
					gdouble        x,
					gdouble        y,
					glWindow      *window);

static void     pointer_exit_cb        (glView        *view,
					glWindow      *window);

static void     name_changed_cb        (glLabel       *label,
					glWindow      *window);

static void     modified_changed_cb    (glLabel       *label,
					glWindow      *window);

static void     clipboard_changed_cb   (GtkClipboard  *clipboard,
                                        GdkEvent      *event,
					glWindow      *window);

static void     focus_widget_changed_cb(GtkWindow     *gtk_window,
                                        GtkWidget     *widget,
					glWindow      *window);

static void     set_copy_paste_sensitivity  (glWindow      *window,
                                             GtkWidget     *focus_widget);

static void     label_changed_cb       (glLabel       *label,
					glWindow      *window);


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glWindow, gl_window, GTK_TYPE_WINDOW)


static void
gl_window_class_init (glWindowClass *class)
{
	GObjectClass   *object_class     = G_OBJECT_CLASS (class);

	gl_debug (DEBUG_WINDOW, "START");

	gl_window_parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_window_finalize;
	object_class->dispose  = gl_window_dispose;

	gl_debug (DEBUG_WINDOW, "END");
}


static void
gl_window_init (glWindow *window)
{
	GtkWidget        *vbox1;
	GtkUIManager     *ui;
	GtkWidget        *status_hbox;

	gl_debug (DEBUG_WINDOW, "START");

	vbox1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox1);

	window->ui = ui = gl_ui_new (window);
	gtk_box_pack_start (GTK_BOX (vbox1),
			    gtk_ui_manager_get_widget (ui, "/MenuBar"),
			    FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox1),
			    gtk_ui_manager_get_widget (ui, "/MainToolBar"),
			    FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox1),
			    gtk_ui_manager_get_widget (ui, "/DrawingToolBar"),
			    FALSE, FALSE, 0);

	window->hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (GTK_BOX (vbox1), window->hbox, TRUE, TRUE, 0);

	window->sidebar = GL_UI_SIDEBAR (gl_ui_sidebar_new ());
	gtk_box_pack_end (GTK_BOX (window->hbox), GTK_WIDGET (window->sidebar), FALSE, FALSE, 0);

	window->property_bar = GL_UI_PROPERTY_BAR (gl_ui_property_bar_new ());
	gtk_box_pack_start (GTK_BOX (vbox1), GTK_WIDGET (window->property_bar), FALSE, FALSE, 0);

	status_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (GTK_BOX (vbox1), status_hbox, FALSE, FALSE, 0);

	window->status_bar = gtk_statusbar_new ();
	gtk_box_pack_start (GTK_BOX (status_hbox),
			    window->status_bar,
			    TRUE, TRUE, 0);
	window->zoom_info = gtk_label_new (NULL);
	gtk_widget_set_size_request (window->zoom_info, ZOOM_INFO_WIDTH, -1);
	window->zoom_info_frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME(window->zoom_info_frame), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER(window->zoom_info_frame), window->zoom_info);
	gtk_widget_show_all (window->zoom_info_frame);
	gtk_box_pack_end (GTK_BOX (status_hbox),
			  window->zoom_info_frame,
			  FALSE, FALSE, 0);

	window->cursor_info = gtk_label_new (NULL);
	gtk_widget_set_size_request (window->cursor_info, CURSOR_INFO_WIDTH, -1);
	window->cursor_info_frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME(window->cursor_info_frame), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER(window->cursor_info_frame), window->cursor_info);
	gtk_widget_show_all (window->cursor_info_frame);
	gtk_box_pack_end (GTK_BOX (status_hbox),
			  window->cursor_info_frame,
			  FALSE, FALSE, 0);

	gtk_window_set_default_size (GTK_WINDOW (window),
				     DEFAULT_WINDOW_WIDTH,
				     DEFAULT_WINDOW_HEIGHT);

	g_signal_connect (G_OBJECT(window), "delete-event",
			  G_CALLBACK(window_delete_event_cb), NULL);
	
	window->menu_tips_context_id =
		gtk_statusbar_get_context_id (GTK_STATUSBAR (window->status_bar), "menu_tips");

	window->print_settings = NULL;
	window->merge_dialog = NULL;
	window->context_menu = GTK_MENU (gtk_ui_manager_get_widget (ui, "/ContextMenu"));
	window->empty_selection_context_menu =
		GTK_MENU (gtk_ui_manager_get_widget (ui, "/EmptySelectionContextMenu"));

	window->view = NULL;

	window_list = g_list_append (window_list, window);

	gl_debug (DEBUG_WINDOW, "END");
}


static void
gl_window_finalize (GObject *object)
{
	gl_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WINDOW (object));

	G_OBJECT_CLASS (gl_window_parent_class)->finalize (object);

	gl_debug (DEBUG_WINDOW, "END");
}


static void
gl_window_dispose (GObject *object)
{
	glWindow          *window;
        GtkClipboard      *clipboard;

	gl_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WINDOW (object));

	window = GL_WINDOW (object);
	window_list = g_list_remove (window_list, window);

        if (window->ui) {
		gl_ui_unref(window->ui);
		window->ui = NULL;
        }

        if (window->label)
        {
                clipboard = gtk_widget_get_clipboard (GTK_WIDGET (window),
                                                      GDK_SELECTION_CLIPBOARD);

                g_signal_handlers_disconnect_by_func (G_OBJECT (clipboard),
                                                      G_CALLBACK (clipboard_changed_cb),
                                                      window);

		g_object_unref (window->label);
        }

	if (G_OBJECT_CLASS (gl_window_parent_class)->dispose) {
		G_OBJECT_CLASS (gl_window_parent_class)->dispose (object);
	}

	gl_debug (DEBUG_WINDOW, "END");
}


/****************************************************************************/
/** Create a glabels window.                                                */
/****************************************************************************/
GtkWidget *
gl_window_new (void)
{
	glWindow *window;

	gl_debug (DEBUG_WINDOW, "START");

	window = g_object_new (GL_TYPE_WINDOW,
			       "title",    _("(none) - gLabels"),
			       NULL);

	gl_debug (DEBUG_WINDOW, "window=%p", window);
	gl_debug (DEBUG_WINDOW, "view=%p", window->view);

	gl_debug (DEBUG_WINDOW, "END");

	return GTK_WIDGET(window);
}


/****************************************************************************/
/** Create a glabels window from a label.                                   */
/****************************************************************************/
GtkWidget*
gl_window_new_from_label (glLabel *label)
{
	glWindow *window;

	gl_debug (DEBUG_WINDOW, "START");

	window = GL_WINDOW (gl_window_new ());

	gl_window_set_label (window, label);

	gl_debug (DEBUG_WINDOW, "END");

	return GTK_WIDGET(window);
}


/****************************************************************************/
/** Create a glabels window from a glabels file.                            */
/****************************************************************************/
GtkWidget*
gl_window_new_from_file (const gchar *filename)
{
	glWindow         *window;
	glLabel          *label;
	gchar            *abs_filename;
	glXMLLabelStatus  status;

	gl_debug (DEBUG_WINDOW, "START");

	window = GL_WINDOW (gl_window_new ());

	abs_filename = gl_file_util_make_absolute (filename);
	label = gl_xml_label_open (abs_filename, &status);
	g_free (abs_filename);

	if (label) {
		gl_window_set_label (window, label);
	}

	gl_debug (DEBUG_WINDOW, "END");

	return GTK_WIDGET(window);
}


/****************************************************************************/
/** Is window empty?                                                        */
/****************************************************************************/
gboolean
gl_window_is_empty (glWindow    *window)
{
	g_return_val_if_fail (GL_IS_WINDOW (window), FALSE);

	gl_debug (DEBUG_WINDOW, "return %d", (window->view == NULL) );
	return ( window->view == NULL );
}


/****************************************************************************/
/** Create view from label and place in window.                             */
/****************************************************************************/
void
gl_window_set_label (glWindow    *window,
		     glLabel     *label)
{
	gchar             *string;
        GtkClipboard      *clipboard;
        GtkWidget         *focus_widget;

	gl_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (GL_IS_WINDOW (window));
	g_return_if_fail (GL_IS_LABEL (label));

        window->label = g_object_ref (label);

	gl_label_clear_modified (label);

	set_window_title (window, label);

	if ( window->view != NULL ) {
		gtk_widget_destroy (window->view);
		window->view = NULL;
	}

	window->view = gl_view_new (label);
	gtk_box_pack_start (GTK_BOX (window->hbox), window->view, TRUE, TRUE, 0);

	gtk_widget_show_all (window->view);

	gl_view_zoom_to_fit (GL_VIEW(window->view));

	if (gl_prefs_model_get_grid_visible (gl_prefs)) {
		gl_view_show_grid (GL_VIEW(window->view));
	} else {
		gl_view_hide_grid (GL_VIEW(window->view));
	}

	if (gl_prefs_model_get_markup_visible (gl_prefs)) {
		gl_view_show_markup (GL_VIEW(window->view));
	} else {
		gl_view_hide_markup (GL_VIEW(window->view));
	}

	gl_ui_update_all (window->ui, GL_VIEW(window->view));

	gl_ui_property_bar_set_label (window->property_bar, window->label);
	gl_ui_sidebar_set_label (window->sidebar, window->label);

	string = g_strdup_printf ("%3.0f%%",
				  100.0*gl_view_get_zoom (GL_VIEW(window->view)));
	gtk_label_set_text (GTK_LABEL(window->zoom_info), string);
	g_free (string);


        clipboard = gtk_widget_get_clipboard (GTK_WIDGET (window),
                                              GDK_SELECTION_CLIPBOARD);


	g_signal_connect (G_OBJECT(window->label), "selection_changed",
			  G_CALLBACK(selection_changed_cb), window);

	g_signal_connect (G_OBJECT(window->view), "context_menu_activate",
			  G_CALLBACK(context_menu_activate_cb), window);

	g_signal_connect (G_OBJECT(window->view), "zoom_changed",
			  G_CALLBACK(zoom_changed_cb), window);

	g_signal_connect (G_OBJECT(window->view), "pointer_moved",
			  G_CALLBACK(pointer_moved_cb), window);

	g_signal_connect (G_OBJECT(window->view), "pointer_exit",
			  G_CALLBACK(pointer_exit_cb), window);

	g_signal_connect (G_OBJECT(label), "name_changed",
			  G_CALLBACK(name_changed_cb), window);

	g_signal_connect (G_OBJECT(label), "modified_changed",
			  G_CALLBACK(modified_changed_cb), window);

	g_signal_connect (G_OBJECT(clipboard), "owner_change",
			  G_CALLBACK(clipboard_changed_cb), window);

	g_signal_connect (G_OBJECT(window), "set_focus",
			  G_CALLBACK(focus_widget_changed_cb), window);

        /* Initialize "Paste" sensitivity. */
        focus_widget = gtk_window_get_focus (GTK_WINDOW (window));
        set_copy_paste_sensitivity (window, focus_widget);

	g_signal_connect (G_OBJECT(label), "changed",
			  G_CALLBACK(label_changed_cb), window);

	gl_debug (DEBUG_WINDOW, "END");
}


/****************************************************************************/
/** Return list of glabels windows.                                         */
/****************************************************************************/
const GList *
gl_window_get_window_list (void)
{
	gl_debug (DEBUG_WINDOW, "");
	return window_list;
}


/*---------------------------------------------------------------------------*/
/** PRIVATE.  Set window title based on name and state of label.             */
/*---------------------------------------------------------------------------*/
static void 
set_window_title (glWindow *window,
		  glLabel  *label)
{
	gchar *name, *title;

	gl_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (window && GL_IS_WINDOW (window));
	g_return_if_fail (label && GL_IS_LABEL (label));

	name = gl_label_get_short_name (label);
	g_return_if_fail (name != NULL);

	if (gl_label_is_modified (label)) {
		title = g_strdup_printf ("%s %s - gLabels",
					 name, _("(modified)"));
	} else {
		title = g_strdup_printf ("%s - gLabels", name);
	}

	gtk_window_set_title (GTK_WINDOW(window), title);

	g_free (name);
	g_free (title);

	gl_debug (DEBUG_WINDOW, "END");
}


/*-------------------------------------------------------------------------*/
/** PRIVATE.  Window "delete-event" callback.                              */
/*-------------------------------------------------------------------------*/
static gboolean
window_delete_event_cb (glWindow      *window,
			GdkEvent      *event,
			gpointer       user_data)
{
	gl_debug (DEBUG_WINDOW, "START");

	g_return_val_if_fail (window && GL_IS_WINDOW (window), TRUE);

	gl_file_close (window);

	gl_debug (DEBUG_WINDOW, "END");

	return TRUE;
}


/*---------------------------------------------------------------------------*/
/** PRIVATE.  View "selection state changed" callback.                       */
/*---------------------------------------------------------------------------*/
static void 
selection_changed_cb (glLabel  *label,
		      glWindow *window)
{
	gl_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));
	g_return_if_fail (window && GL_IS_WINDOW (window));

        gl_ui_update_selection_verbs (window->ui, GL_VIEW (window->view), TRUE);

	gl_debug (DEBUG_WINDOW, "END");
}


/*---------------------------------------------------------------------------*/
/** PRIVATE.  View "context menu activate" callback.                         */
/*---------------------------------------------------------------------------*/
static void
context_menu_activate_cb (glView       *view,
			  gint          button,
			  guint32       activate_time,
			  glWindow     *window)
{
        gl_debug (DEBUG_WINDOW, "START");

        g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (window && GL_IS_WINDOW (window));

        if (gl_label_is_selection_empty (view->label)) {

		gtk_menu_popup (GTK_MENU (window->empty_selection_context_menu),
				NULL, NULL, NULL, NULL, button, activate_time);

        } else {

		gtk_menu_popup (GTK_MENU (window->context_menu),
				NULL, NULL, NULL, NULL, button, activate_time);

        }

        gl_debug (DEBUG_WINDOW, "END");
}


/*---------------------------------------------------------------------------*/
/** PRIVATE.  View "zoom state changed" callback.                            */
/*---------------------------------------------------------------------------*/
static void 
zoom_changed_cb (glView   *view,
		 gdouble   zoom,
		 glWindow *window)
{
	gchar *string;

	gl_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (window && GL_IS_WINDOW (window));

	string = g_strdup_printf ("%3.0f%%", 100.0*zoom);
	gtk_label_set_text (GTK_LABEL(window->zoom_info), string);
	g_free (string);

	gl_ui_update_zoom_verbs (window->ui, view);

	gl_debug (DEBUG_WINDOW, "END");
}


/*---------------------------------------------------------------------------*/
/** PRIVATE.  View "pointer moved" callback.                                 */
/*---------------------------------------------------------------------------*/
static void
pointer_moved_cb (glView   *view,
		  gdouble   x,
		  gdouble   y,
		  glWindow *window)
{
	gchar    *string;
        lglUnits  units;
	gdouble   units_per_point;
	gint      units_precision;

	gl_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (window && GL_IS_WINDOW (window));

        units = gl_prefs_model_get_units (gl_prefs);
	units_per_point = lgl_units_get_units_per_point (units);
	units_precision = gl_units_util_get_precision (units);

	string = g_strdup_printf ("%.*f, %.*f",
				  units_precision, x*units_per_point,
				  units_precision, y*units_per_point);
	gtk_label_set_text (GTK_LABEL(window->cursor_info), string);
	g_free (string);

	gl_debug (DEBUG_WINDOW, "END");
}


/*---------------------------------------------------------------------------*/
/** PRIVATE.  View "pointer exit" callback.                                  */
/*---------------------------------------------------------------------------*/
static void
pointer_exit_cb (glView   *view,
		 glWindow *window)
{
	gl_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (window && GL_IS_WINDOW (window));

	gtk_label_set_text (GTK_LABEL(window->cursor_info), "");

	gl_debug (DEBUG_WINDOW, "END");
}


/*---------------------------------------------------------------------------*/
/** PRIVATE.  Label "name changed" callback.                                 */
/*---------------------------------------------------------------------------*/
static void 
name_changed_cb (glLabel  *label,
		 glWindow *window)
{
	gl_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));
	g_return_if_fail (window && GL_IS_WINDOW (window));

	set_window_title (window, label);

	gl_debug (DEBUG_WINDOW, "END");
}


/*---------------------------------------------------------------------------*/
/** PRIVATE.  Label "modified state changed" callback.                       */
/*---------------------------------------------------------------------------*/
static void 
modified_changed_cb (glLabel  *label,
		     glWindow *window)
{
	gl_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));
	g_return_if_fail (window && GL_IS_WINDOW (window));

	set_window_title (window, label);

	gl_ui_update_modified_verbs (window->ui, label);

	gl_debug (DEBUG_WINDOW, "END");
}


/*---------------------------------------------------------------------------*/
/** PRIVATE.  Label "changed" callback.                                      */
/*---------------------------------------------------------------------------*/
static void
label_changed_cb (glLabel  *label,
                  glWindow *window)
{
	gl_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));
	g_return_if_fail (window && GL_IS_WINDOW (window));

	gl_ui_update_undo_redo_verbs (window->ui, label);

	gl_debug (DEBUG_WINDOW, "END");
}



/*---------------------------------------------------------------------------*/
/** PRIVATE.  Clipboard "owner change" callback.                             */
/*---------------------------------------------------------------------------*/
static void
clipboard_changed_cb (GtkClipboard *clipboard,
                      GdkEvent     *event,
                      glWindow     *window)
{
        GtkWidget    *focus_widget;

	gl_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (window && GL_IS_WINDOW (window));

        focus_widget = gtk_window_get_focus (GTK_WINDOW (window));
        set_copy_paste_sensitivity (window, focus_widget);

	gl_debug (DEBUG_WINDOW, "END");
}


/*---------------------------------------------------------------------------*/
/** PRIVATE.  Window "set-focus" callback.                                   */
/*---------------------------------------------------------------------------*/
static void
focus_widget_changed_cb (GtkWindow *gtk_window,
                         GtkWidget *widget,
                         glWindow  *window)
{
	gl_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (window && GL_IS_WINDOW (window));

        if (widget)
        {
                gl_debug (DEBUG_WINDOW, "SET-FOCUS %x %s\n",
                          widget,
                          G_OBJECT_TYPE_NAME (widget));

                set_copy_paste_sensitivity (window, widget);
        }

	gl_debug (DEBUG_WINDOW, "END");
}


/*---------------------------------------------------------------------------*/
/** PRIVATE.  Set paste sensitivity.                                         */
/*---------------------------------------------------------------------------*/
static void
set_copy_paste_sensitivity (glWindow  *window,
                            GtkWidget *focus_widget)
{
	gl_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (window && GL_IS_WINDOW (window));

        if ( focus_widget == GL_VIEW(window->view)->canvas )
        {
                gl_ui_update_selection_verbs (window->ui, GL_VIEW (window->view), TRUE);

                gl_ui_update_paste_verbs (window->ui, gl_label_can_paste (window->label));
        }
        else
        {
                gl_ui_update_selection_verbs (window->ui, GL_VIEW (window->view), FALSE);
                gl_ui_update_paste_verbs (window->ui, FALSE);
        }

	gl_debug (DEBUG_WINDOW, "END");
}


/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
