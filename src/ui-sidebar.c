/*
 *  ui-sidebar.c
 *  Copyright (C) 2003-2009  Jim Evins <evins@snaught.com>.
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

	GtkWidget           *editor;
};

/*===========================================================================*/
/* Private globals                                                           */
/*===========================================================================*/


/*===========================================================================*/
/* Local function prototypes                                                 */
/*===========================================================================*/

static void     gl_ui_sidebar_finalize      (GObject              *object);

static void     gl_ui_sidebar_construct     (glUISidebar          *sidebar);


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

	sidebar->priv->editor = gl_object_editor_new ();
	gtk_widget_show (sidebar->priv->editor);

	gtk_container_add (GTK_CONTAINER(sidebar), sidebar->priv->editor);

	gl_debug (DEBUG_UI, "END");
}


/****************************************************************************/
/* Set label associated with sidebar.                                       */
/****************************************************************************/
void
gl_ui_sidebar_set_label (glUISidebar *sidebar,
                         glLabel     *label)
{
	gl_debug (DEBUG_UI, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));

        gl_object_editor_set_label (GL_OBJECT_EDITOR (sidebar->priv->editor), label);

	gl_debug (DEBUG_UI, "END");
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
