/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_merge_menu.c:  merge menu widget module
 *
 *  Copyright (C) 2008  Jim Evins <evins@snaught.com>.
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

#include "wdgt-merge-menu.h"

#include <string.h>
#include <gtk/gtkmenuitem.h>

#include "marshal.h"

#include "debug.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/


struct _glWdgtMergeMenuPrivate {

        GList *menu_items;
};

enum {
        FIELD_SELECTED,
        LAST_SIGNAL
};

typedef void (*glWdgtMergeMenuSignal) (GObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static gint signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_wdgt_merge_menu_finalize      (GObject              *object);



/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glWdgtMergeMenu, gl_wdgt_merge_menu, GTK_TYPE_MENU);


static void
gl_wdgt_merge_menu_class_init (glWdgtMergeMenuClass *class)
{
        GObjectClass *object_class = G_OBJECT_CLASS (class);

        gl_debug (DEBUG_MERGE_MENU, "START");

        gl_wdgt_merge_menu_parent_class = g_type_class_peek_parent (class);

        object_class->finalize = gl_wdgt_merge_menu_finalize;

        signals[FIELD_SELECTED] =
            g_signal_new ("field_selected",
                          G_OBJECT_CLASS_TYPE(object_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (glWdgtMergeMenuClass, field_selected),
                          NULL, NULL,
                          gl_marshal_VOID__STRING,
                          G_TYPE_NONE, 1, G_TYPE_STRING);

        gl_debug (DEBUG_MERGE_MENU, "END");
}

static void
gl_wdgt_merge_menu_init (glWdgtMergeMenu *merge_menu)
{
        gl_debug (DEBUG_MERGE_MENU, "START");

        merge_menu->priv = g_new0 (glWdgtMergeMenuPrivate, 1);

        gl_debug (DEBUG_MERGE_MENU, "END");
}

static void
gl_wdgt_merge_menu_finalize (GObject *object)
{
        glWdgtMergeMenu *merge_menu = GL_WDGT_MERGE_MENU (object);
        GList           *p;
        GtkWidget       *menu_item;
        gchar           *field;

        gl_debug (DEBUG_MERGE_MENU, "START");

        g_return_if_fail (object != NULL);
        g_return_if_fail (GL_IS_WDGT_MERGE_MENU (object));

        for ( p = merge_menu->priv->menu_items; p != NULL; p = p->next )
        {
                menu_item = GTK_WIDGET (p->data);
                field = g_object_get_data (G_OBJECT (menu_item), "field");
                g_free (field);
        }
        g_list_free (merge_menu->priv->menu_items);
        g_free (merge_menu->priv);

        G_OBJECT_CLASS (gl_wdgt_merge_menu_parent_class)->finalize (object);

        gl_debug (DEBUG_MERGE_MENU, "END");
}

GtkWidget *
gl_wdgt_merge_menu_new (void)
{
        glWdgtMergeMenu *merge_menu;

        gl_debug (DEBUG_MERGE_MENU, "START");

        merge_menu = g_object_new (gl_wdgt_merge_menu_get_type (), NULL);

        gl_debug (DEBUG_MERGE_MENU, "END");

        return GTK_WIDGET (merge_menu);
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Menu item activation callback.                                 */
/*--------------------------------------------------------------------------*/
static void
activate_cb (GtkMenuItem     *menu_item,
             glWdgtMergeMenu *merge_menu)
{
        gchar *field;

        gl_debug (DEBUG_MERGE_MENU, "START");

        field = g_object_get_data (G_OBJECT (menu_item), "field");
        gl_debug (DEBUG_MERGE_MENU, "Field activated: \"%s\"\n", field );

        g_signal_emit (G_OBJECT (merge_menu), signals[FIELD_SELECTED], 0, field);

        gl_debug (DEBUG_MERGE_MENU, "END");
}

/****************************************************************************/
/* set field names.                                                         */
/****************************************************************************/
void
gl_wdgt_merge_menu_set_fields (glWdgtMergeMenu *merge_menu,
                               GList           *field_list)
{
        GList     *p;
        GtkWidget *menu_item;
        gchar     *field;

        gl_debug (DEBUG_MERGE_MENU, "START");

        /*
         * Remove all old menu items.
         */
        for ( p = merge_menu->priv->menu_items; p != NULL; p = p->next )
        {
                menu_item = GTK_WIDGET (p->data);
                field = g_object_get_data (G_OBJECT (menu_item), "field");
                g_free (field);
                gtk_widget_destroy (menu_item);
        }
        g_list_free (merge_menu->priv->menu_items);
        merge_menu->priv->menu_items = NULL;

        /*
         * Add new menu items.
         */
        for ( p = field_list; p != NULL; p = p->next )
        {
                menu_item = gtk_menu_item_new_with_label (p->data);
                g_object_set_data (G_OBJECT (menu_item), "field", g_strdup (p->data));
                g_signal_connect (G_OBJECT (menu_item), "activate", 
                                  G_CALLBACK (activate_cb), merge_menu);
                gtk_menu_shell_append (GTK_MENU_SHELL (merge_menu), menu_item);
                merge_menu->priv->menu_items =
                        g_list_append (merge_menu->priv->menu_items, menu_item);
        }

        gl_debug (DEBUG_MERGE_MENU, "END");
}

