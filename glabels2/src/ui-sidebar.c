/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  ui-sidebar.c:  Object property sidebar
 *
 *  Copyright (C) 2003  Jim Evins <evins@snaught.com>.
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

#include <libgnomeprint/gnome-font.h>

#include "ui-sidebar.h"
#include "ui-util.h"
#include "object-editor.h"
#include "stock.h"

#include "debug.h"

/*============================================================================*/
/* Private macros and constants.                                              */
/*============================================================================*/

#define DEFAULT_SIDEBAR_WIDTH 300

/*============================================================================*/
/* Private globals                                                            */
/*============================================================================*/

static GObjectClass *parent_class;

static gchar* doc_verbs [] = {
	"/commands/PropertyEditor",

	NULL
};

/*============================================================================*/
/* Local function prototypes                                                  */
/*============================================================================*/

static void     gl_ui_sidebar_class_init    (glUISidebarClass     *class);
static void     gl_ui_sidebar_instance_init (glUISidebar          *sidebar);
static void     gl_ui_sidebar_finalize      (GObject              *object);

static void     gl_ui_sidebar_construct     (glUISidebar          *sidebar,
					     BonoboUIComponent    *ui_component);

static void     selection_changed_cb        (glView               *view,
					     glUISidebar          *sidebar);



/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
guint
gl_ui_sidebar_get_type (void)
{
	static guint sidebar_type = 0;

	if (!sidebar_type) {
		GTypeInfo sidebar_info = {
			sizeof (glUISidebarClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_ui_sidebar_class_init,
			NULL,
			NULL,
			sizeof (glUISidebar),
			0,
			(GInstanceInitFunc) gl_ui_sidebar_instance_init,
		};

		sidebar_type =
			g_type_register_static (GTK_TYPE_VBOX,
						"glUISidebar",
						&sidebar_info, 0);
	}

	return sidebar_type;
}

static void
gl_ui_sidebar_class_init (glUISidebarClass *class)
{
	GObjectClass   *object_class     = (GObjectClass *) class;

	gl_debug (DEBUG_UI, "START");

	parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_ui_sidebar_finalize;

	gl_debug (DEBUG_UI, "END");
}

static void
gl_ui_sidebar_instance_init (glUISidebar *sidebar)
{
	gl_debug (DEBUG_UI, "START");

	sidebar->view = NULL;

	gl_debug (DEBUG_UI, "END");
}

static void
gl_ui_sidebar_finalize (GObject *object)
{
	glUISidebar *sidebar;

	gl_debug (DEBUG_UI, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_UI_SIDEBAR (object));

	sidebar = GL_UI_SIDEBAR (object);

	if (sidebar->view) {
		g_object_unref (G_OBJECT(sidebar->view));
		sidebar = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_UI, "END");
}

/****************************************************************************/
/* Create a NEW sidebar.                                                    */
/****************************************************************************/
GObject *
gl_ui_sidebar_new (BonoboUIComponent *ui_component)
{
	glUISidebar *sidebar;

	gl_debug (DEBUG_UI, "START");

	sidebar = g_object_new (gl_ui_sidebar_get_type (), NULL);

	gtk_widget_set_size_request (GTK_WIDGET (sidebar), DEFAULT_SIDEBAR_WIDTH, -1);

	gl_ui_sidebar_construct (sidebar, ui_component);

	gl_debug (DEBUG_UI, "END");

	return G_OBJECT(sidebar);
}

/******************************************************************************/
/* Initialize property toolbar.                                               */
/******************************************************************************/
static void
gl_ui_sidebar_construct (glUISidebar       *sidebar,
			 BonoboUIComponent *ui_component)
{
	gl_debug (DEBUG_UI, "START");

	sidebar->ui_component = ui_component;

	gl_ui_util_insert_widget (ui_component,
				  GTK_WIDGET (sidebar),
				  "/PropertySidebar/PropertyEditor");

	sidebar->empty_child = gl_object_editor_new (GL_STOCK_PROPERTIES,
						     _("Object properties"),
						     GL_OBJECT_EDITOR_EMPTY,
						     NULL);

	sidebar->child = gtk_widget_ref (sidebar->empty_child);
	gtk_widget_show (sidebar->child);
	gtk_container_add (GTK_CONTAINER(sidebar), sidebar->child);

	gl_ui_util_set_verb_list_sensitive (ui_component, doc_verbs, FALSE);

	gl_debug (DEBUG_UI, "END");
}

/****************************************************************************/
/* Set view associated with sidebar.                                        */
/****************************************************************************/
void
gl_ui_sidebar_set_view (glUISidebar *sidebar,
			glView      *view)
{
	gl_debug (DEBUG_UI, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	gl_ui_util_set_verb_list_sensitive (sidebar->ui_component, doc_verbs, TRUE);

	sidebar->view = GL_VIEW (g_object_ref (G_OBJECT (view)));

	g_signal_connect (G_OBJECT(view), "selection_changed",
			  G_CALLBACK(selection_changed_cb), sidebar);

	gl_debug (DEBUG_UI, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  View "selection state changed" callback.                        */
/*---------------------------------------------------------------------------*/
static void 
selection_changed_cb (glView      *view,
		      glUISidebar *sidebar)
{
	gl_debug (DEBUG_UI, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (sidebar && GL_IS_UI_SIDEBAR (sidebar));

	gtk_container_remove (GTK_CONTAINER(sidebar), sidebar->child);

	if (gl_view_is_selection_empty (view) || !gl_view_is_selection_atomic (view)) {

		sidebar->child = gtk_widget_ref (sidebar->empty_child);
		
	} else {

		sidebar->child = gtk_widget_ref (gl_view_get_editor (view));

	}

	gtk_widget_show (sidebar->child);
#if 0
	gtk_container_add (GTK_CONTAINER(sidebar), sidebar->child);
#else
	gtk_box_pack_start (GTK_BOX(sidebar), sidebar->child, TRUE, TRUE, 0);
#endif

	gl_debug (DEBUG_UI, "END");
}

