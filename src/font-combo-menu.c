/*
 *  font-combo-menu.c
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

#include "font-combo-menu.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "font-combo-menu-item.h"
#include "font-util.h"
#include "font-history.h"
#include "marshal.h"


/*===========================================*/
/* Private macros and constants.             */
/*===========================================*/


/*===========================================*/
/* Private types                             */
/*===========================================*/

struct _glFontComboMenuPrivate {

        gchar     *font_family;

        GtkWidget *recent_menu_item;
        GtkWidget *recent_sub_menu;

};

enum {
        FONT_CHANGED,
        LAST_SIGNAL
};

gchar *standard_families[] = { "Sans", "Serif", "Monospace", NULL };

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static guint signals[LAST_SIGNAL] = {0};


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void       gl_font_combo_menu_finalize (GObject             *object);

static void       menu_item_activate_cb       (glFontComboMenuItem *item,
                                               glFontComboMenu     *this);

static GtkWidget *new_font_sub_menu           (glFontComboMenu     *this,
                                               const GList         *list);

static void       font_history_changed_cb     (glFontComboMenu     *this);



/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glFontComboMenu, gl_font_combo_menu, GTK_TYPE_MENU);


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_font_combo_menu_class_init (glFontComboMenuClass *class)
{
	GObjectClass   *gobject_class = G_OBJECT_CLASS (class);

        gl_font_combo_menu_parent_class = g_type_class_peek_parent (class);

	gobject_class->finalize = gl_font_combo_menu_finalize;

        signals[FONT_CHANGED] =
                g_signal_new ("font_changed",
                              G_OBJECT_CLASS_TYPE (gobject_class),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (glFontComboMenuClass, font_changed),
                              NULL, NULL,
                              gl_marshal_VOID__VOID,
                              G_TYPE_NONE, 0);
}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_font_combo_menu_init (glFontComboMenu *this)
{
        gint          i;
        GtkWidget    *menu_item;
        GtkWidget    *sub_menu;
        const GList  *list;
        GList        *p;

	this->priv = g_new0 (glFontComboMenuPrivate, 1);


        for ( i = 0; standard_families[i] != NULL; i++ )
        {
                menu_item = gl_font_combo_menu_item_new (standard_families[i]);
                gtk_menu_shell_append (GTK_MENU_SHELL (this), menu_item);
                g_signal_connect (menu_item, "activate",
                                  G_CALLBACK (menu_item_activate_cb), this);
        }

        menu_item = gtk_separator_menu_item_new ();
        gtk_menu_shell_append (GTK_MENU_SHELL (this), menu_item);


        menu_item = gtk_menu_item_new_with_label (_("Recent fonts"));
        gtk_menu_shell_append (GTK_MENU_SHELL (this), menu_item);

        list = gl_font_history_model_get_family_list (gl_font_history);
        sub_menu = new_font_sub_menu (this, list);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), sub_menu);
        gtk_widget_set_sensitive (menu_item, list != NULL);

        this->priv->recent_menu_item = menu_item;
        this->priv->recent_sub_menu  = sub_menu;

        menu_item = gtk_separator_menu_item_new ();
        gtk_menu_shell_append (GTK_MENU_SHELL (this), menu_item);


        menu_item = gtk_menu_item_new_with_label (_("Proportional fonts"));
        gtk_menu_shell_append (GTK_MENU_SHELL (this), menu_item);

        list = gl_font_util_get_proportional_families ();
        sub_menu = new_font_sub_menu (this, list);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), sub_menu);
        gtk_widget_set_sensitive (menu_item, list != NULL);

        menu_item = gtk_menu_item_new_with_label (_("Fixed-width fonts"));
        gtk_menu_shell_append (GTK_MENU_SHELL (this), menu_item);

        list = gl_font_util_get_fixed_width_families ();
        sub_menu = new_font_sub_menu (this, list);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), sub_menu);
        gtk_widget_set_sensitive (menu_item, list != NULL);

        menu_item = gtk_menu_item_new_with_label (_("All fonts"));
        gtk_menu_shell_append (GTK_MENU_SHELL (this), menu_item);

        list = gl_font_util_get_all_families ();
        sub_menu = new_font_sub_menu (this, list);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), sub_menu);
        gtk_widget_set_sensitive (menu_item, list != NULL);


        gtk_widget_show_all (GTK_WIDGET (this));


        g_signal_connect_swapped (gl_font_history, "changed",
                                  G_CALLBACK (font_history_changed_cb), this);

}


/*****************************************************************************/
/* Finalize Method.                                                          */
/*****************************************************************************/
static void
gl_font_combo_menu_finalize (GObject *object)
{
	glFontComboMenu *this = GL_FONT_COMBO_MENU (object);

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_FONT_COMBO_MENU (object));

	g_free (this->priv);

	G_OBJECT_CLASS (gl_font_combo_menu_parent_class)->finalize (object);
}


/*****************************************************************************/
/** New Object Generator.                                                    */
/*****************************************************************************/
GtkWidget *
gl_font_combo_menu_new (void)
{
	glFontComboMenu *this;

	this = g_object_new (gl_font_combo_menu_get_type (), NULL);

	return GTK_WIDGET (this);
}


/*****************************************************************************/
/* menu_item activate callback.                                              */
/*****************************************************************************/
static void menu_item_activate_cb (glFontComboMenuItem *item,
                                   glFontComboMenu     *this)
{
        this->priv->font_family = gl_font_combo_menu_item_get_family (item);

        g_signal_emit (this, signals[FONT_CHANGED], 0);

        gtk_widget_hide (GTK_WIDGET (this));
}


/*****************************************************************************/
/* Get font family name.                                                     */
/*****************************************************************************/
gchar *
gl_font_combo_menu_get_family (glFontComboMenu *this)
{
        return g_strdup (this->priv->font_family);
}


/*****************************************************************************/
/* Create a new font sub menu from font list.                                */
/*****************************************************************************/
static GtkWidget *
new_font_sub_menu (glFontComboMenu *this,
                   const GList     *list)
{
        GtkWidget   *menu;
        GtkWidget   *menu_item;
        GList       *p;

        menu = gtk_menu_new ();

        for ( p = (GList *)list; p != NULL; p = p->next )
        {
                menu_item = gl_font_combo_menu_item_new (p->data);
                gtk_widget_show_all (menu_item);
                gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
                g_signal_connect (menu_item, "activate",
                                  G_CALLBACK (menu_item_activate_cb), this);
        }

        gtk_widget_show (menu);
        return menu;
}


/*****************************************************************************/
/* Font history changed callback.                                            */
/*****************************************************************************/
static void
font_history_changed_cb (glFontComboMenu     *this)
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
        list = gl_font_history_model_get_family_list (gl_font_history);
        this->priv->recent_sub_menu = new_font_sub_menu (this, list);

        /*
         * Attach to top-level menu item
         */
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (this->priv->recent_menu_item),
                                   this->priv->recent_sub_menu);
        gtk_widget_set_sensitive (this->priv->recent_menu_item, list != NULL);

        gl_font_history_model_free_family_list (list);
}




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
