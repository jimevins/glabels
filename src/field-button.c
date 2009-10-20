/*
 *  field-button.c
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

#include "field-button.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "field-button-menu.h"
#include "marshal.h"



/*========================================================*/
/* Private types.                                         */
/*========================================================*/

/** GL_FIELD_BUTTON Private fields */
struct _glFieldButtonPrivate {

        GtkWidget  *label;

        GtkWidget  *menu;
};

enum {
        KEY_SELECTED,
        LAST_SIGNAL
};


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static guint signals[LAST_SIGNAL] = {0};


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gl_field_button_finalize      (GObject             *object);

static gboolean
button_press_event_cb (GtkWidget          *widget,
                       GdkEventButton     *event,
                       glFieldButton      *this);

static void
menu_key_selected_cb  (glFieldButtonMenu  *menu,
                       gchar              *key,
                       glFieldButton      *this);

static void
menu_selection_done_cb (GtkMenuShell      *object,
                        glFieldButton     *this);


/*****************************************************************************/
/* Object infrastructure.                                                    */
/*****************************************************************************/
G_DEFINE_TYPE (glFieldButton, gl_field_button, GTK_TYPE_TOGGLE_BUTTON);


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_field_button_class_init (glFieldButtonClass *class)
{
        GObjectClass      *gobject_class = (GObjectClass *) class;

        gl_field_button_parent_class = g_type_class_peek_parent (class);

        gobject_class->finalize = gl_field_button_finalize;

        signals[KEY_SELECTED] =
                g_signal_new ("key_selected",
                              G_OBJECT_CLASS_TYPE (gobject_class),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (glFieldButtonClass, key_selected),
                              NULL, NULL,
                              gl_marshal_VOID__STRING,
                              G_TYPE_NONE, 1, G_TYPE_STRING);
}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_field_button_init (glFieldButton *this)
{
        GtkWidget *hbox;
        GtkWidget *arrow;

        this->priv = g_new0 (glFieldButtonPrivate, 1);

        hbox = gtk_hbox_new (FALSE, 3);
        gtk_container_add (GTK_CONTAINER (this), hbox);
        
        this->priv->label = gtk_label_new ("");
        gtk_misc_set_alignment (GTK_MISC (this->priv->label), 0.0, 0.5);
        gtk_box_pack_start (GTK_BOX (hbox), this->priv->label, TRUE, TRUE, 0);

        arrow = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_IN);
        gtk_box_pack_end (GTK_BOX (hbox), arrow, FALSE, FALSE, 0);

        g_signal_connect (this, "button_press_event",
                          G_CALLBACK(button_press_event_cb), this);
}


/*****************************************************************************/
/* Finalize Method.                                                          */
/*****************************************************************************/
static void
gl_field_button_finalize (GObject *object)
{
        glFieldButton    *this;

        g_return_if_fail (object && IS_GL_FIELD_BUTTON (object));
        this = GL_FIELD_BUTTON (object);

        g_object_ref_sink (this->priv->menu);
        g_free (this->priv);

        G_OBJECT_CLASS (gl_field_button_parent_class)->finalize (object);
}


/*****************************************************************************/
/** New Object Generator.                                                    */
/*****************************************************************************/
GtkWidget *
gl_field_button_new (const gchar  *name)
{
        glFieldButton  *this;

        this = g_object_new (TYPE_GL_FIELD_BUTTON, NULL);

        gtk_label_set_text (GTK_LABEL (this->priv->label), name);

        this->priv->menu = gl_field_button_menu_new ();

        gtk_widget_show_all (this->priv->menu);

        g_signal_connect (this->priv->menu, "key_selected",
                          G_CALLBACK (menu_key_selected_cb), this);
        g_signal_connect (this->priv->menu, "selection_done",
                          G_CALLBACK (menu_selection_done_cb), this);

        return GTK_WIDGET (this);
}


/*****************************************************************************/
/* Set key list.                                                             */
/*****************************************************************************/
void
gl_field_button_set_keys (glFieldButton  *this,
                          GList          *key_list)
{
        gl_field_button_menu_set_keys (GL_FIELD_BUTTON_MENU (this->priv->menu),
                                       key_list);

        gtk_widget_show_all (this->priv->menu);
}


/*****************************************************************************/
/* Menu positioning function.                                                */
/*****************************************************************************/
static void
menu_position_function (GtkMenu  *menu,
                        gint     *x,
                        gint     *y,
                        gboolean *push_in,
                        gpointer  user_data)
{
        glFieldButton  *this = GL_FIELD_BUTTON (user_data);
        GdkWindow    *window;
        gint          x1, y1;
        gint          menu_h, menu_w;

        window = gtk_widget_get_window (GTK_WIDGET (this));

        gdk_window_get_origin (window, &x1, &y1);
        *x = x1 + GTK_WIDGET (this)->allocation.x;
        *y = y1 + GTK_WIDGET (this)->allocation.y +
                GTK_WIDGET (this)->allocation.height;
                
        menu_h = this->priv->menu->allocation.height;
        menu_w = this->priv->menu->allocation.width;

        if ((*y + menu_h) > gdk_screen_height ())
        {
                *y = y1 + GTK_WIDGET (this)->allocation.y - menu_h;
                if ( *y < 0 )
                {
                        *y = gdk_screen_height () - menu_h;
                }
        }

        if ((*x + menu_w) > gdk_screen_width ())
        {
                *x = gdk_screen_width () - menu_w;
        }

        *push_in = TRUE;
}


/*****************************************************************************/
/* Button "button_press_event" callback.                                     */
/*****************************************************************************/
static gboolean
button_press_event_cb (GtkWidget      *widget,
                       GdkEventButton *event,
                       glFieldButton   *this)
{
        switch (event->button)
        {

        case 1:
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (this), TRUE);

                gtk_menu_popup (GTK_MENU (this->priv->menu),
                                NULL, NULL,
                                menu_position_function, this,
                                event->button, event->time);
                break;

        default:
                break;

        }

        return FALSE;
}


/*****************************************************************************/
/* Menu "key selected" callback.                                             */
/*****************************************************************************/
static void
menu_key_selected_cb (glFieldButtonMenu     *menu,
                      gchar                 *field,
                      glFieldButton         *this)
{
        g_signal_emit (this, signals[KEY_SELECTED], 0, field);
}


/*****************************************************************************/
/* Menu "selection done" callback.                                           */
/*****************************************************************************/
static void
menu_selection_done_cb (GtkMenuShell         *object,
                        glFieldButton         *this)
{
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (this), FALSE);
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
