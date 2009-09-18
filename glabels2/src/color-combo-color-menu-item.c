/*
 *  color-combo-color-menu-item.c
 *  Copyright (C) 2008  Jim Evins <evins@snaught.com>.
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

#include "color-combo-color-menu-item.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "color-swatch.h"
#include "color.h"
#include "marshal.h"



/*===========================================*/
/* Private macros and constants.             */
/*===========================================*/

#define SIZE 20


/*===========================================*/
/* Private types                             */
/*===========================================*/

struct _glColorComboColorMenuItemPrivate {

        gint       id;

        GtkWidget *swatch;
};


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_color_combo_color_menu_item_finalize    (GObject                *object);


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glColorComboColorMenuItem, gl_color_combo_color_menu_item, GTK_TYPE_MENU_ITEM);


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_color_combo_color_menu_item_class_init (glColorComboColorMenuItemClass *class)
{
	GObjectClass   *gobject_class = G_OBJECT_CLASS (class);

        gl_color_combo_color_menu_item_parent_class = g_type_class_peek_parent (class);

	gobject_class->finalize = gl_color_combo_color_menu_item_finalize;
}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_color_combo_color_menu_item_init (glColorComboColorMenuItem *this)
{
	this->priv = g_new0 (glColorComboColorMenuItemPrivate, 1);
}


/*****************************************************************************/
/* Finalize Method.                                                          */
/*****************************************************************************/
static void
gl_color_combo_color_menu_item_finalize (GObject *object)
{
	glColorComboColorMenuItem *this = GL_COLOR_COMBO_COLOR_MENU_ITEM (object);

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_COLOR_COMBO_COLOR_MENU_ITEM (object));

	g_free (this->priv);

	G_OBJECT_CLASS (gl_color_combo_color_menu_item_parent_class)->finalize (object);
}


/*****************************************************************************/
/** New Object Generator.                                                    */
/*****************************************************************************/
GtkWidget *
gl_color_combo_color_menu_item_new (gint         id,
                                    guint        color,
                                    const gchar *tip)
{
	glColorComboColorMenuItem *this;

	this = g_object_new (GL_TYPE_COLOR_COMBO_COLOR_MENU_ITEM, NULL);

        this->priv->id = id;

        this->priv->swatch = gl_color_swatch_new (SIZE, SIZE, color);
        gtk_container_add (GTK_CONTAINER (this), this->priv->swatch);

        gtk_widget_set_tooltip_text (GTK_WIDGET (this), tip);

	return GTK_WIDGET (this);
}


/*****************************************************************************/
/* Set color.                                                                */
/*****************************************************************************/
void
gl_color_combo_color_menu_item_set_color(glColorComboColorMenuItem *this,
                                         gint                       id,
                                         guint                      color,
                                         const gchar               *tip)
{
        this->priv->id = id;

        gl_color_swatch_set_color (GL_COLOR_SWATCH (this->priv->swatch), color);

        gtk_widget_set_tooltip_text (GTK_WIDGET (this), tip);
}


/*****************************************************************************/
/* Get id.                                                                   */
/*****************************************************************************/
gint
gl_color_combo_color_menu_item_get_id (glColorComboColorMenuItem *this)
{
        return this->priv->id;
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
