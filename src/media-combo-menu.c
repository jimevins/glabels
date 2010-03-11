/*
 *  media-combo-menu.c
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

#include "media-combo-menu.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <libglabels/libglabels.h>
#include "media-combo-menu-item.h"
#include "template-history.h"
#include "marshal.h"


/*===========================================*/
/* Private macros and constants.             */
/*===========================================*/


/*===========================================*/
/* Private types                             */
/*===========================================*/

struct _glMediaComboMenuPrivate {

        gchar     *media;

        GtkWidget *recent_menu_item;
        GtkWidget *recent_sub_menu;

};

enum {
        MEDIA_CHANGED,
        LAST_SIGNAL
};


/*===========================================*/
/* Private globals                           */
/*===========================================*/

static guint signals[LAST_SIGNAL] = {0};


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void       gl_media_combo_menu_finalize (GObject             *object);

static void       menu_item_activate_cb        (glMediaComboMenuItem *item,
                                                glMediaComboMenu     *this);

static GtkWidget *new_brand_sub_menu           (glMediaComboMenu     *this,
                                                const gchar          *paper_id,
                                                const GList          *brand_list);

static GtkWidget *new_part_sub_menu            (glMediaComboMenu     *this,
                                                const GList          *part_list);

static void       media_history_changed_cb     (glMediaComboMenu     *this);



/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glMediaComboMenu, gl_media_combo_menu, GTK_TYPE_MENU);


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_media_combo_menu_class_init (glMediaComboMenuClass *class)
{
	GObjectClass   *gobject_class = G_OBJECT_CLASS (class);

        gl_media_combo_menu_parent_class = g_type_class_peek_parent (class);

	gobject_class->finalize = gl_media_combo_menu_finalize;

        signals[MEDIA_CHANGED] =
                g_signal_new ("media_changed",
                              G_OBJECT_CLASS_TYPE (gobject_class),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (glMediaComboMenuClass, media_changed),
                              NULL, NULL,
                              gl_marshal_VOID__VOID,
                              G_TYPE_NONE, 0);
}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_media_combo_menu_init (glMediaComboMenu *this)
{
        GtkWidget    *menu_item;
        GtkWidget    *sub_menu;
        const GList  *list;
        GList        *paper_id_list, *p_id;
        gchar        *paper_id, *paper_name;
        GList        *brand_list;

	this->priv = g_new0 (glMediaComboMenuPrivate, 1);


        menu_item = gtk_menu_item_new_with_label (_("Recent templates"));
        gtk_menu_shell_append (GTK_MENU_SHELL (this), menu_item);

        list = gl_template_history_model_get_name_list (gl_template_history);
        sub_menu = new_part_sub_menu (this, list);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), sub_menu);
        gtk_widget_set_sensitive (menu_item, list != NULL);

        this->priv->recent_menu_item = menu_item;
        this->priv->recent_sub_menu  = sub_menu;

        menu_item = gtk_separator_menu_item_new ();
        gtk_menu_shell_append (GTK_MENU_SHELL (this), menu_item);

        paper_id_list = lgl_db_get_paper_id_list ();

        for ( p_id = paper_id_list; p_id; p_id = p_id->next )
        {
                paper_id = p_id->data;
                paper_name = lgl_db_lookup_paper_name_from_id (paper_id);

                brand_list = lgl_db_get_brand_list (paper_id, NULL);
                if ( brand_list )
                {
                        menu_item = gtk_menu_item_new_with_label (paper_name);
                        gtk_menu_shell_append (GTK_MENU_SHELL (this), menu_item);

                        sub_menu = new_brand_sub_menu (this, paper_id, brand_list);
                        gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), sub_menu);
                }
                lgl_db_free_brand_list ( brand_list );
        }

        lgl_db_free_paper_id_list (paper_id_list);

        g_signal_connect_swapped (gl_template_history, "changed",
                                  G_CALLBACK (media_history_changed_cb), this);

}


/*****************************************************************************/
/* Finalize Method.                                                          */
/*****************************************************************************/
static void
gl_media_combo_menu_finalize (GObject *object)
{
	glMediaComboMenu *this = GL_MEDIA_COMBO_MENU (object);

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_MEDIA_COMBO_MENU (object));

	g_free (this->priv);

	G_OBJECT_CLASS (gl_media_combo_menu_parent_class)->finalize (object);
}


/*****************************************************************************/
/** New Object Generator.                                                    */
/*****************************************************************************/
GtkWidget *
gl_media_combo_menu_new (void)
{
	glMediaComboMenu *this;

	this = g_object_new (gl_media_combo_menu_get_type (), NULL);

	return GTK_WIDGET (this);
}


/*****************************************************************************/
/* menu_item activate callback.                                              */
/*****************************************************************************/
static void menu_item_activate_cb (glMediaComboMenuItem *item,
                                   glMediaComboMenu     *this)
{
        this->priv->media = gl_media_combo_menu_item_get_name (item);

        g_signal_emit (this, signals[MEDIA_CHANGED], 0);

        gtk_widget_hide (GTK_WIDGET (this));
}


/*****************************************************************************/
/* Get media name.                                                           */
/*****************************************************************************/
gchar *
gl_media_combo_menu_get_name (glMediaComboMenu *this)
{
        return g_strdup (this->priv->media);
}


/*****************************************************************************/
/* Create a new brand sub menu from media list.                              */
/*****************************************************************************/
static GtkWidget *
new_brand_sub_menu (glMediaComboMenu *this,
                    const gchar      *paper_id,
                    const GList      *brand_list)
{
        GtkWidget   *menu;
        GtkWidget   *menu_item;
        GtkWidget    *sub_menu;
        GList       *p_brand;
        gchar       *brand;
        GList       *part_list;

        menu = gtk_menu_new ();

        for ( p_brand = (GList *)brand_list; p_brand; p_brand = p_brand->next )
        {
                brand = p_brand->data;

                part_list = lgl_db_get_template_name_list_all (brand, paper_id, NULL);
                if ( part_list )
                {
                        menu_item = gtk_menu_item_new_with_label (brand);
                        gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

                        sub_menu = new_part_sub_menu (this, part_list);
                        gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), sub_menu);
                }
                lgl_db_free_brand_list ( part_list );
        }

        gtk_widget_show (menu);

        return menu;
}


/*****************************************************************************/
/* Create a new part sub menu from part list.                                */
/*****************************************************************************/
static GtkWidget *
new_part_sub_menu (glMediaComboMenu *this,
                   const GList      *part_list)
{
        GtkWidget   *menu;
        GtkWidget   *menu_item;
        GList       *p;

        menu = gtk_menu_new ();

        for ( p = (GList *)part_list; p != NULL; p = p->next )
        {
                menu_item = gl_media_combo_menu_item_new (p->data);
                gtk_widget_show_all (menu_item);
                gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
                g_signal_connect (menu_item, "activate",
                                  G_CALLBACK (menu_item_activate_cb), this);
        }

        gtk_widget_show (menu);
        return menu;
}


/*****************************************************************************/
/* Media history changed callback.                                           */
/*****************************************************************************/
static void
media_history_changed_cb (glMediaComboMenu     *this)
{
        GList *list;

        /*
         * Remove old sub menu
         */
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (this->priv->recent_menu_item),
                                   NULL);

        /*
         * Build new sub menu
         */
        list = gl_template_history_model_get_name_list (gl_template_history);
        this->priv->recent_sub_menu = new_part_sub_menu (this, list);

        /*
         * Attach to top-level menu item
         */
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (this->priv->recent_menu_item),
                                   this->priv->recent_sub_menu);
        gtk_widget_set_sensitive (this->priv->recent_menu_item, list != NULL);

        gl_template_history_model_free_name_list (list);
}




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
