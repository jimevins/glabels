/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  window.c:  a gLabels app window
 *
 *  Copyright (C) 2002  Jim Evins <evins@snaught.com>.
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

#include "ui.h"
#include "window.h"
#include "util.h"
#include "xml-label.h"
#include "prefs.h"

#include "debug.h"

/*============================================================================*/
/* Private macros and constants.                                              */
/*============================================================================*/

#define DEFAULT_WINDOW_WIDTH  500
#define DEFAULT_WINDOW_HEIGHT 375

#define CURSOR_INFO_WIDTH     150
#define ZOOM_INFO_WIDTH        50

/*============================================================================*/
/* Private globals                                                            */
/*============================================================================*/
static BonoboWindowClass *parent_class;

static GList *window_list = NULL;


/*============================================================================*/
/* Local function prototypes                                                  */
/*============================================================================*/

static void     gl_window_class_init   (glWindowClass *class);
static void     gl_window_init         (glWindow      *window);
static void     gl_window_finalize     (GObject       *object);
static void     gl_window_destroy      (GtkObject     *gtk_object);

static void     set_window_title       (glWindow *window,
					glLabel  *label);

static gboolean window_delete_event_cb (glWindow      *window,
					GdkEvent      *event,
					gpointer       user_data);

static void     selection_changed_cb   (glView        *view,
					glWindow      *window);

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


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
guint
gl_window_get_type (void)
{
	static guint window_type = 0;

	if (!window_type) {
		GTypeInfo window_info = {
			sizeof (glWindowClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_window_class_init,
			NULL,
			NULL,
			sizeof (glWindow),
			0,
			(GInstanceInitFunc) gl_window_init,
		};

		window_type =
		    g_type_register_static (bonobo_window_get_type (),
					    "glWindow",
					    &window_info, 0);
	}

	return window_type;
}

static void
gl_window_class_init (glWindowClass *class)
{
	GObjectClass   *object_class     = (GObjectClass *) class;
	GtkObjectClass *gtk_object_class = (GtkObjectClass *) class;

	gl_debug (DEBUG_WINDOW, "START");

	parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_window_finalize;

	gtk_object_class->destroy = gl_window_destroy;

	gl_debug (DEBUG_WINDOW, "END");
}

static void
gl_window_init (glWindow *window)
{
	BonoboUIContainer *ui_container;
	BonoboUIComponent *ui_component;

	gl_debug (DEBUG_WINDOW, "START");

	ui_container = bonobo_window_get_ui_container(BONOBO_WINDOW(window));
	ui_component = bonobo_ui_component_new_default ();
	bonobo_ui_component_set_container (ui_component,
					   BONOBO_OBJREF (ui_container),
					   NULL);

	window->cursor_info = gtk_label_new (NULL);
	gtk_widget_set_size_request (window->cursor_info, CURSOR_INFO_WIDTH, -1);
	window->cursor_info_frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME(window->cursor_info_frame), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER(window->cursor_info_frame), window->cursor_info);
	gtk_widget_show_all (window->cursor_info_frame);

	window->zoom_info = gtk_label_new (NULL);
	gtk_widget_set_size_request (window->zoom_info, ZOOM_INFO_WIDTH, -1);
	window->zoom_info_frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME(window->zoom_info_frame), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER(window->zoom_info_frame), window->zoom_info);
	gtk_widget_show_all (window->zoom_info_frame);

	gl_ui_init (ui_component,
		    BONOBO_WINDOW (window),
		    window->cursor_info_frame,
		    window->zoom_info_frame);

	gtk_window_set_default_size (GTK_WINDOW (window),
				     DEFAULT_WINDOW_WIDTH,
				     DEFAULT_WINDOW_HEIGHT);

	g_signal_connect (G_OBJECT(window), "delete-event",
			  G_CALLBACK(window_delete_event_cb), NULL);
	
	window->uic  = ui_component;
	window->view = NULL;

	window_list = g_list_append (window_list, window);

	gl_debug (DEBUG_WINDOW, "END");
}

static void
gl_window_finalize (GObject *object)
{
	glWindow *window;

	gl_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WINDOW (object));

	window = GL_WINDOW (object);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_WINDOW, "END");
}

static void
gl_window_destroy (GtkObject *gtk_object)
{
	glWindow *window;

	gl_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (gtk_object != NULL);
	g_return_if_fail (GL_IS_WINDOW (gtk_object));

	window = GL_WINDOW (gtk_object);
	window_list = g_list_remove (window_list, window);

	if (GTK_OBJECT_CLASS (parent_class)->destroy) {
		GTK_OBJECT_CLASS (parent_class)->destroy (gtk_object);
	}

	gl_debug (DEBUG_WINDOW, "END");
}


/****************************************************************************/
/* Create an app window.                                                    */
/****************************************************************************/
GtkWidget *
gl_window_new (void)
{
	glWindow *window;

	gl_debug (DEBUG_WINDOW, "START");

	window = g_object_new (gl_window_get_type (),
			       "win_name", "glabels",
			       "title",    _("(none) - glabels"),
			       NULL);

	gl_debug (DEBUG_WINDOW, "window=%p", window);
	gl_debug (DEBUG_WINDOW, "view=%p", window->view);

	gl_debug (DEBUG_WINDOW, "END");

	return GTK_WIDGET(window);
}

/****************************************************************************/
/* Create an app window from a label.                                       */
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
/* Create an app window from a file.                                        */
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

	abs_filename = gl_util_make_absolute (filename);
	label = gl_xml_label_open (filename, &status);
	g_free (abs_filename);

	gl_window_set_label (window, label);

	gl_debug (DEBUG_WINDOW, "END");

	return GTK_WIDGET(window);
}

/****************************************************************************/
/* Is window empty?                                                         */
/****************************************************************************/
gboolean
gl_window_is_empty (glWindow    *window)
{
	g_return_val_if_fail (GL_IS_WINDOW (window), FALSE);

	gl_debug (DEBUG_WINDOW, "return %d", (window->view == NULL) );
	return ( window->view == NULL );
}

/****************************************************************************/
/* Create view from label and place in window.                              */
/****************************************************************************/
void
gl_window_set_label (glWindow    *window,
		     glLabel     *label)
{
	gchar *string;

	gl_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (GL_IS_WINDOW (window));
	g_return_if_fail (GL_IS_LABEL (label));

	gl_label_clear_modified (label);

	set_window_title (window, label);

	if ( window->view != NULL ) {
		gtk_widget_destroy (window->view);
		window->view = NULL;
	}

	window->view = gl_view_new (label);
	bonobo_window_set_contents (BONOBO_WINDOW(window), window->view);

	gtk_widget_show_all (window->view);

	if (gl_prefs->grid_visible) {
		gl_view_show_grid (GL_VIEW(window->view));
	} else {
		gl_view_hide_grid (GL_VIEW(window->view));
	}

	if (gl_prefs->markup_visible) {
		gl_view_show_markup (GL_VIEW(window->view));
	} else {
		gl_view_hide_markup (GL_VIEW(window->view));
	}

	gl_ui_update_all (window->uic, GL_VIEW(window->view));

	string = g_strdup_printf ("%3.0f%%",
				  100.0*gl_view_get_zoom (GL_VIEW(window->view)));
	gtk_label_set_text (GTK_LABEL(window->zoom_info), string);
	g_free (string);

	g_signal_connect (G_OBJECT(window->view), "selection_changed",
			  G_CALLBACK(selection_changed_cb), window);

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

	gl_debug (DEBUG_WINDOW, "END");
}

/****************************************************************************/
/* Return list of app windows.                                              */
/****************************************************************************/
const GList *
gl_window_get_window_list (void)
{
	gl_debug (DEBUG_WINDOW, "");
	return window_list;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set window title based on name and state of label.              */
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
		title = g_strdup_printf ("%s %s - glabels",
					 name, _("(modified)"));
	} else {
		title = g_strdup_printf ("%s - glabels", name);
	}

	gtk_window_set_title (GTK_WINDOW(window), title);

	g_free (name);
	g_free (title);

	gl_debug (DEBUG_WINDOW, "END");
}

/*-------------------------------------------------------------------------*/
/* PRIVATE.  Window "delete-event" callback.                               */
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
/* PRIVATE.  View "selection state changed" callback.                        */
/*---------------------------------------------------------------------------*/
static void 
selection_changed_cb (glView   *view,
		      glWindow *window)
{
	gl_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (window && GL_IS_WINDOW (window));

	gl_ui_update_selection_verbs (window->uic, view);

	gl_debug (DEBUG_WINDOW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  View "zoom state changed" callback.                             */
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

	gl_ui_update_zoom_verbs (window->uic, view);

	gl_debug (DEBUG_WINDOW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  View "pointer moved" callback.                                  */
/*---------------------------------------------------------------------------*/
static void
pointer_moved_cb (glView   *view,
		  gdouble   x,
		  gdouble   y,
		  glWindow *window)
{
	gchar *string;
	gdouble units_per_point;
	gint    units_precision;

	gl_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (window && GL_IS_WINDOW (window));

	units_per_point = gl_prefs_get_units_per_point ();
	units_precision = gl_prefs_get_units_precision ();

	string = g_strdup_printf ("%.*f, %.*f",
				  units_precision, x*units_per_point,
				  units_precision, y*units_per_point);
	gtk_label_set_text (GTK_LABEL(window->cursor_info), string);
	g_free (string);

	gl_debug (DEBUG_WINDOW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  View "pointer exit" callback.                                   */
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
/* PRIVATE.  Label "name changed" callback.                                  */
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
/* PRIVATE.  Label "modified state changed" callback.                        */
/*---------------------------------------------------------------------------*/
static void 
modified_changed_cb (glLabel  *label,
		     glWindow *window)
{
	gl_debug (DEBUG_WINDOW, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));
	g_return_if_fail (window && GL_IS_WINDOW (window));

	set_window_title (window, label);

	gl_ui_update_modified_verbs (window->uic, label);

	gl_debug (DEBUG_WINDOW, "END");
}

