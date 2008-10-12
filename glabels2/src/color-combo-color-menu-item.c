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

#include "marshal.h"
#include <glib/gi18n.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkseparatormenuitem.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkimage.h>
#include "color.h"


/*===========================================*/
/* Private macros and constants.             */
/*===========================================*/


/*===========================================*/
/* Private types                             */
/*===========================================*/

struct _glColorComboColorMenuItemPrivate {

        gint id;
};


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_color_combo_color_menu_item_finalize    (GObject                *object);

static GdkPixbuf *
create_color_pixbuf (gdouble         w,
                     gdouble         h,
                     guint           color);

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
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

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
        GdkPixbuf                 *pixbuf;

	this = g_object_new (GL_TYPE_COLOR_COMBO_COLOR_MENU_ITEM, NULL);

        this->priv->id = id;

        pixbuf = create_color_pixbuf (16, 16, color);
        gtk_container_add (GTK_CONTAINER (this),
                           gtk_image_new_from_pixbuf (pixbuf));

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
        GdkPixbuf                 *pixbuf;

        this->priv->id = id;

        pixbuf = create_color_pixbuf (16, 16, color);
        gtk_image_set_from_pixbuf (GTK_IMAGE (gtk_bin_get_child (GTK_BIN (this))),
                                   pixbuf);

        gtk_widget_set_tooltip_text (GTK_WIDGET (this), tip);
 }


/*****************************************************************************/
/* Create new pixbuf with color preview.                                     */
/*****************************************************************************/
static GdkPixbuf *
create_color_pixbuf (gdouble         w,
                     gdouble         h,
                     guint           color)
{
        cairo_surface_t   *surface;
        cairo_t           *cr;
        GdkPixbuf         *pixbuf;

        surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);

        cr = cairo_create (surface);

        cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

        cairo_save (cr);
        cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
        cairo_paint (cr);
        cairo_restore (cr);

        cairo_rectangle( cr, 2, 2, w-3, h-3 );

        cairo_set_source_rgba (cr,
                               GL_COLOR_F_RED   (color),
                               GL_COLOR_F_GREEN (color),
                               GL_COLOR_F_BLUE  (color),
                               GL_COLOR_F_ALPHA (color));
        cairo_fill_preserve( cr );

        cairo_set_line_width (cr, 1.0);
        cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
        cairo_stroke (cr);

        cairo_destroy( cr );

        pixbuf = gl_util_cairo_convert_to_pixbuf (surface);
        cairo_surface_destroy (surface);

        return pixbuf;
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
