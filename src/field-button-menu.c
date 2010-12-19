/*
 *  field-button-menu.c
 *  Copyright (C) 2008-2009  Jim Evins <evins@snaught.com>.
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

#include "field-button-menu.h"

#include <string.h>
#include <gtk/gtk.h>

#include "marshal.h"

#include "debug.h"

#define MAX_MENU_ROWS 25

/*===========================================*/
/* Private types                             */
/*===========================================*/


struct _glFieldButtonMenuPrivate {

        GList *menu_items;
};

enum {
        KEY_SELECTED,
        LAST_SIGNAL
};

typedef void (*glFieldButtonMenuSignal) (GObject * object, gpointer data);


/*===========================================*/
/* Private globals                           */
/*===========================================*/

static gint signals[LAST_SIGNAL] = { 0 };


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_field_button_menu_finalize      (GObject              *object);


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glFieldButtonMenu, gl_field_button_menu, GTK_TYPE_MENU)


static void
gl_field_button_menu_class_init (glFieldButtonMenuClass *class)
{
        GObjectClass *object_class = G_OBJECT_CLASS (class);

        gl_debug (DEBUG_FIELD_BUTTON, "START");

        gl_field_button_menu_parent_class = g_type_class_peek_parent (class);

        object_class->finalize = gl_field_button_menu_finalize;

        signals[KEY_SELECTED] =
            g_signal_new ("key_selected",
                          G_OBJECT_CLASS_TYPE(object_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (glFieldButtonMenuClass, key_selected),
                          NULL, NULL,
                          gl_marshal_VOID__STRING,
                          G_TYPE_NONE, 1, G_TYPE_STRING);

        gl_debug (DEBUG_FIELD_BUTTON, "END");
}


static void
gl_field_button_menu_init (glFieldButtonMenu *this)
{
        gl_debug (DEBUG_FIELD_BUTTON, "START");

        this->priv = g_new0 (glFieldButtonMenuPrivate, 1);

        gl_debug (DEBUG_FIELD_BUTTON, "END");
}


static void
gl_field_button_menu_finalize (GObject *object)
{
        glFieldButtonMenu *this = GL_FIELD_BUTTON_MENU (object);
        GList           *p;
        GtkWidget       *menu_item;
        gchar           *key;

        gl_debug (DEBUG_FIELD_BUTTON, "START");

        g_return_if_fail (object != NULL);
        g_return_if_fail (GL_IS_FIELD_BUTTON_MENU (object));

        for ( p = this->priv->menu_items; p != NULL; p = p->next )
        {
                menu_item = GTK_WIDGET (p->data);
                key = g_object_get_data (G_OBJECT (menu_item), "key");
                g_free (key);
        }
        g_list_free (this->priv->menu_items);
        g_free (this->priv);

        G_OBJECT_CLASS (gl_field_button_menu_parent_class)->finalize (object);

        gl_debug (DEBUG_FIELD_BUTTON, "END");
}


GtkWidget *
gl_field_button_menu_new (void)
{
        glFieldButtonMenu *this;

        gl_debug (DEBUG_FIELD_BUTTON, "START");

        this = g_object_new (gl_field_button_menu_get_type (), NULL);

        gl_debug (DEBUG_FIELD_BUTTON, "END");

        return GTK_WIDGET (this);
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Menu item activation callback.                                 */
/*--------------------------------------------------------------------------*/
static void
activate_cb (GtkMenuItem       *menu_item,
             glFieldButtonMenu *this)
{
        gchar *key;

        gl_debug (DEBUG_FIELD_BUTTON, "START");

        key = g_object_get_data (G_OBJECT (menu_item), "key");
        gl_debug (DEBUG_FIELD_BUTTON, "Key activated: \"%s\"\n", key );

        g_signal_emit (G_OBJECT (this), signals[KEY_SELECTED], 0, key);

        gl_debug (DEBUG_FIELD_BUTTON, "END");
}


/****************************************************************************/
/* set key names.                                                           */
/****************************************************************************/
void
gl_field_button_menu_set_keys (glFieldButtonMenu *this,
                               GList             *key_list)
{
        GList     *p;
        GtkWidget *menu_item;
        gchar     *key;
        gint       i, i_row, i_col;

        gl_debug (DEBUG_FIELD_BUTTON, "START");

        /*
         * Remove all old menu items.
         */
        for ( p = this->priv->menu_items; p != NULL; p = p->next )
        {
                menu_item = GTK_WIDGET (p->data);
                key = g_object_get_data (G_OBJECT (menu_item), "key");
                g_free (key);
                gtk_container_remove (GTK_CONTAINER (this), menu_item);
        }
        g_list_free (this->priv->menu_items);
        this->priv->menu_items = NULL;
        gtk_widget_unrealize (GTK_WIDGET (this)); /* Start over with new Gdk resources. */

        /*
         * Add new menu items.
         */
        for ( p = key_list, i = 0; p != NULL; p = p->next, i++ )
        {
                if ( p->data && strlen (p->data) )
                {

                        gl_debug (DEBUG_FIELD_BUTTON, "Adding key: %s", p->data);

                        menu_item = gtk_menu_item_new_with_label (p->data);
                        gtk_widget_show (menu_item);
                        g_object_set_data (G_OBJECT (menu_item), "key", g_strdup (p->data));
                        g_signal_connect (G_OBJECT (menu_item), "activate", 
                                          G_CALLBACK (activate_cb), this);
                        this->priv->menu_items =
                                g_list_append (this->priv->menu_items, menu_item);

                        i_row = i % MAX_MENU_ROWS;
                        i_col = i / MAX_MENU_ROWS;
                        gtk_menu_attach (GTK_MENU (this), menu_item, i_col, i_col+1, i_row, i_row+1);
                }
        }

        gl_debug (DEBUG_FIELD_BUTTON, "END");
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
