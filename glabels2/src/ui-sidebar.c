/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/**
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

#include "ui-sidebar.h"

#include <glib/gi18n.h>

#include "ui-util.h"
#include "object-editor.h"
#include "stock.h"

#include "debug.h"

/*===========================================================================*/
/* Private macros and constants.                                             */
/*===========================================================================*/

#define DEFAULT_SIDEBAR_WIDTH 340

/*===========================================================================*/
/* Private data types                                                        */
/*===========================================================================*/

struct _glUISidebarPrivate {

	glView              *view;

	GtkWidget           *child;
	GtkWidget           *empty_child;
};

/*===========================================================================*/
/* Private globals                                                           */
/*===========================================================================*/


/*===========================================================================*/
/* Local function prototypes                                                 */
/*===========================================================================*/

static void     gl_ui_sidebar_finalize      (GObject              *object);

static void     gl_ui_sidebar_construct     (glUISidebar          *sidebar);

static void     selection_changed_cb        (glView               *view,
					     glUISidebar          *sidebar);



/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glUISidebar, gl_ui_sidebar, GTK_TYPE_VBOX);

static void
gl_ui_sidebar_class_init (glUISidebarClass *class)
{
	GObjectClass   *object_class     = G_OBJECT_CLASS (class);

	gl_debug (DEBUG_UI, "START");

	gl_ui_sidebar_parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_ui_sidebar_finalize;

	gl_debug (DEBUG_UI, "END");
}

static void
gl_ui_sidebar_init (glUISidebar *sidebar)
{
	gl_debug (DEBUG_UI, "START");

	sidebar->priv = g_new0 (glUISidebarPrivate, 1);

	gl_debug (DEBUG_UI, "END");
}

static void
gl_ui_sidebar_finalize (GObject *object)
{
	glUISidebar *sidebar = GL_UI_SIDEBAR (object);

	gl_debug (DEBUG_UI, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_UI_SIDEBAR (object));

	if (sidebar->priv->view) {
		g_object_unref (G_OBJECT(sidebar->priv->view));
	}
	g_free (sidebar->priv);

	G_OBJECT_CLASS (gl_ui_sidebar_parent_class)->finalize (object);

	gl_debug (DEBUG_UI, "END");
}

/****************************************************************************/
/* Create a NEW sidebar.                                                    */
/****************************************************************************/
GtkWidget *
gl_ui_sidebar_new (void)
{
	glUISidebar *sidebar;

	gl_debug (DEBUG_UI, "START");

	sidebar = g_object_new (gl_ui_sidebar_get_type (), NULL);

	gtk_widget_set_size_request (GTK_WIDGET (sidebar), DEFAULT_SIDEBAR_WIDTH, -1);

	gl_ui_sidebar_construct (sidebar);

	gl_debug (DEBUG_UI, "END");

	return GTK_WIDGET(sidebar);
}

/******************************************************************************/
/* Initialize property toolbar.                                               */
/******************************************************************************/
static void
gl_ui_sidebar_construct (glUISidebar       *sidebar)
{
	gl_debug (DEBUG_UI, "START");

	sidebar->priv->empty_child = gl_object_editor_new (GL_STOCK_PROPERTIES,
							   _("Object properties"),
							   GL_OBJECT_EDITOR_EMPTY,
							   NULL);

	sidebar->priv->child = gtk_widget_ref (sidebar->priv->empty_child);
	gtk_widget_show (sidebar->priv->child);
	gtk_container_add (GTK_CONTAINER(sidebar), sidebar->priv->child);

	gtk_widget_set_sensitive (GTK_WIDGET (sidebar), FALSE);

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

	gtk_widget_set_sensitive (GTK_WIDGET (sidebar), TRUE);

	sidebar->priv->view = GL_VIEW (g_object_ref (G_OBJECT (view)));

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

	gtk_container_remove (GTK_CONTAINER(sidebar), sidebar->priv->child);

	if (gl_view_is_selection_empty (view) || !gl_view_is_selection_atomic (view)) {

		sidebar->priv->child = gtk_widget_ref (sidebar->priv->empty_child);
		
	} else {

		sidebar->priv->child = gtk_widget_ref (gl_view_get_editor (view));

	}

	gtk_widget_show (sidebar->priv->child);

	gtk_box_pack_start (GTK_BOX(sidebar), sidebar->priv->child, TRUE, TRUE, 0);

	gl_debug (DEBUG_UI, "END");
}

