/*
 *  media-combo-menu-item.c
 *  Copyright (C) 2010  Jim Evins <evins@snaught.com>.
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

#include "media-combo-menu-item.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <libglabels/libglabels.h>
#include "mini-preview-pixbuf-cache.h"
#include "prefs.h"
#include "str-util.h"
#include "marshal.h"



/*===========================================*/
/* Private macros and constants.             */
/*===========================================*/


/*===========================================*/
/* Private types                             */
/*===========================================*/

struct _glMediaComboMenuItemPrivate {

        gchar *name;

};


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void   gl_media_combo_menu_item_finalize    (GObject                *object);


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glMediaComboMenuItem, gl_media_combo_menu_item, GTK_TYPE_MENU_ITEM);


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_media_combo_menu_item_class_init (glMediaComboMenuItemClass *class)
{
	GObjectClass   *gobject_class = G_OBJECT_CLASS (class);

        gl_media_combo_menu_item_parent_class = g_type_class_peek_parent (class);

	gobject_class->finalize = gl_media_combo_menu_item_finalize;
}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_media_combo_menu_item_init (glMediaComboMenuItem *this)
{
	this->priv = g_new0 (glMediaComboMenuItemPrivate, 1);
}


/*****************************************************************************/
/* Finalize Method.                                                          */
/*****************************************************************************/
static void
gl_media_combo_menu_item_finalize (GObject *object)
{
	glMediaComboMenuItem *this = GL_MEDIA_COMBO_MENU_ITEM (object);

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_MEDIA_COMBO_MENU_ITEM (object));

        g_free (this->priv->name);
	g_free (this->priv);

	G_OBJECT_CLASS (gl_media_combo_menu_item_parent_class)->finalize (object);
}


/*****************************************************************************/
/** New Object Generator.                                                    */
/*****************************************************************************/
GtkWidget *
gl_media_combo_menu_item_new (gchar *name)
{
	glMediaComboMenuItem *this;
        GtkWidget            *hbox;
        GtkWidget            *preview;
        GtkWidget            *label;
        GdkPixbuf            *pixbuf;
        lglTemplate          *template;
        lglTemplateFrame     *frame;
        gchar                *size_string;
        gchar                *layout_string;
        gchar                *label_markup;

	this = g_object_new (GL_TYPE_MEDIA_COMBO_MENU_ITEM, NULL);

        this->priv->name = g_strdup (name);

        hbox = gtk_hbox_new (FALSE, 6);
        gtk_container_add (GTK_CONTAINER (this), hbox);

        pixbuf = gl_mini_preview_pixbuf_cache_get_pixbuf (name);
        preview = gtk_image_new_from_pixbuf (pixbuf);
        gtk_box_pack_start (GTK_BOX (hbox), preview, FALSE, FALSE, 0);
        g_object_unref (G_OBJECT (pixbuf));

        template      = lgl_db_lookup_template_from_name (name);
        frame         = (lglTemplateFrame *)template->frames->data;
        size_string   = lgl_template_frame_get_size_description (frame, gl_prefs_model_get_units (gl_prefs));
        layout_string = lgl_template_frame_get_layout_description (frame);
        label_markup  = g_strdup_printf ("<b>%s: %s</b>\n%s\n%s",
                                         name, template->description, size_string, layout_string);
        g_free (size_string);
        g_free (layout_string);
        lgl_template_free (template);

        label = gtk_label_new (NULL);
        gtk_label_set_markup (GTK_LABEL (label), label_markup);
        g_free (label_markup);
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	return GTK_WIDGET (this);
}


/*****************************************************************************/
/* Get family.                                                               */
/*****************************************************************************/
gchar *
gl_media_combo_menu_item_get_name (glMediaComboMenuItem *this)
{
        return g_strdup (this->priv->name);
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
