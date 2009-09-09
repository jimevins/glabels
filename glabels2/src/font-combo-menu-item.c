/*
 *  font-combo-menu-item.c
 *  Copyright (C) 2009  Jim Evins <evins@snaught.com>.
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

#include "font-combo-menu-item.h"

#include "font-sample.h"
#include "marshal.h"
#include <glib/gi18n.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkseparatormenuitem.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>



/*===========================================*/
/* Private macros and constants.             */
/*===========================================*/

#define SAMPLE_W 32
#define SAMPLE_H 24


/*===========================================*/
/* Private types                             */
/*===========================================*/

struct _glFontComboMenuItemPrivate {

        gchar *font_family;

};


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_font_combo_menu_item_finalize    (GObject                *object);


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glFontComboMenuItem, gl_font_combo_menu_item, GTK_TYPE_MENU_ITEM);


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_font_combo_menu_item_class_init (glFontComboMenuItemClass *class)
{
	GObjectClass   *gobject_class = G_OBJECT_CLASS (class);

        gl_font_combo_menu_item_parent_class = g_type_class_peek_parent (class);

	gobject_class->finalize = gl_font_combo_menu_item_finalize;
}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_font_combo_menu_item_init (glFontComboMenuItem *this)
{
	this->priv = g_new0 (glFontComboMenuItemPrivate, 1);
}


/*****************************************************************************/
/* Finalize Method.                                                          */
/*****************************************************************************/
static void
gl_font_combo_menu_item_finalize (GObject *object)
{
	glFontComboMenuItem *this = GL_FONT_COMBO_MENU_ITEM (object);

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_FONT_COMBO_MENU_ITEM (object));

        g_free (this->priv->font_family);
	g_free (this->priv);

	G_OBJECT_CLASS (gl_font_combo_menu_item_parent_class)->finalize (object);
}


/*****************************************************************************/
/** New Object Generator.                                                    */
/*****************************************************************************/
GtkWidget *
gl_font_combo_menu_item_new (gchar *font_family)
{
	glFontComboMenuItem *this;
        GtkWidget           *hbox;
        GtkWidget           *sample;
        GtkWidget           *label;
        gchar               *markup;

	this = g_object_new (GL_TYPE_FONT_COMBO_MENU_ITEM, NULL);

        this->priv->font_family = g_strdup (font_family);

        hbox = gtk_hbox_new (FALSE, 6);
        gtk_container_add (GTK_CONTAINER (this), hbox);

        sample = gl_font_sample_new (SAMPLE_W, SAMPLE_H, "Aa", font_family);
        gtk_box_pack_start (GTK_BOX (hbox), sample, FALSE, FALSE, 0);

        label = gtk_label_new (font_family);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

        markup = g_strdup_printf ("<span font_family=\"%s\" size=\"x-large\">ABCDEFGHIJKLMNOPQRSTUVWXYZ\nabcdefghijklmnopqrstuvwxyz\n0123456789</span>",
                                  font_family);
        gtk_widget_set_tooltip_markup (GTK_WIDGET (this), markup);
        g_free (markup);

	return GTK_WIDGET (this);
}


/*****************************************************************************/
/* Get family.                                                               */
/*****************************************************************************/
gchar *
gl_font_combo_menu_item_get_family (glFontComboMenuItem *this)
{
        return g_strdup (this->priv->font_family);
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
