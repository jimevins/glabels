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

        gboolean    label_is_key;
        gchar      *key;

        GtkWidget  *label;

        GtkWidget  *menu;
};

enum {
        KEY_SELECTED,
        CHANGED,
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
G_DEFINE_TYPE (glFieldButton, gl_field_button, GTK_TYPE_TOGGLE_BUTTON)


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

        signals[CHANGED] =
                g_signal_new ("changed",
                              G_OBJECT_CLASS_TYPE (gobject_class),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (glFieldButtonClass, changed),
                              NULL, NULL,
                              gl_marshal_VOID__VOID,
                              G_TYPE_NONE, 0);
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

        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3);
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

        if ( name )
        {
                gtk_label_set_text (GTK_LABEL (this->priv->label), name);
        }
        else
        {
                this->priv->label_is_key = TRUE;
        }

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
        this->priv->key = g_strdup (key_list->data);
        if ( this->priv->label_is_key )
        {
                gtk_label_set_text (GTK_LABEL (this->priv->label), key_list->data);
        }

        gtk_widget_show_all (this->priv->menu);
}


/*****************************************************************************/
/* Set current key.                                                          */
/*****************************************************************************/
void
gl_field_button_set_key (glFieldButton   *this,
                         const gchar     *key)
{
        g_free (this->priv->key);
        this->priv->key = g_strdup (key);

        if ( this->priv->label_is_key )
        {
                gtk_label_set_text (GTK_LABEL (this->priv->label), key);
        }
}


/*****************************************************************************/
/* Get current key.                                                        */
/*****************************************************************************/
gchar *
gl_field_button_get_key (glFieldButton   *this)
{
        return g_strdup (this->priv->key);
}


/*****************************************************************************/
/* Menu positioning function.                                                */
/*****************************************************************************/
static void
menu_position_function (GtkMenu       *menu,
                        gint          *x,
                        gint          *y,
                        gboolean      *push_in,
                        glFieldButton *this)
{
        GdkScreen          *screen;
        gint                w_screen, h_screen;
        GdkWindow          *window;
        gint                x_window, y_window;
        GtkAllocation       allocation;
        gint                x_this, y_this, h_this;
        GtkRequisition      menu_requisition;
        gint                h_menu, w_menu;

        /*
         * Screen size
         */
        screen = gtk_widget_get_screen (GTK_WIDGET (this));
        w_screen = gdk_screen_get_width (screen);
        h_screen = gdk_screen_get_height (screen);

        /*
         * Absolute position of "this" window on screen.
         */
        window = gtk_widget_get_window (GTK_WIDGET (this));
        gdk_window_get_origin (window, &x_window, &y_window);

        /*
         *  Position and size of "this" inside window
         */
        gtk_widget_get_allocation (GTK_WIDGET (this), &allocation);
        x_this = allocation.x;
        y_this = allocation.y;
        h_this = allocation.height;

        /*
         * Size of menu.
         */
        gtk_widget_get_preferred_size (this->priv->menu, NULL, &menu_requisition);
        h_menu = menu_requisition.height;
        w_menu = menu_requisition.width;

        /*
         * Default position anchored to lower left corner of "this".
         */
        *x = x_window + x_this;
        *y = y_window + y_this + h_this;
                
        /*
         * Adjust vertical position if menu if extends past bottom of screen.
         */
        if ( (*y + h_menu) > h_screen )
        {
                *y = y_window + y_this - h_menu;

                if ( *y < 0 )
                {
                        *y = h_screen - h_menu;
                }
        }

        /*
         * Adjust horizontal position if menu if extends past edge of screen.
         */
        if ( (*x + w_menu) > w_screen )
        {
                *x = w_screen - w_menu;
        }


        *push_in = TRUE;
}


/*****************************************************************************/
/* Button "button_press_event" callback.                                     */
/*****************************************************************************/
static gboolean
button_press_event_cb (GtkWidget      *widget,
                       GdkEventButton *event,
                       glFieldButton  *this)
{
        switch (event->button)
        {

        case 1:
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (this), TRUE);

                gtk_menu_popup (GTK_MENU (this->priv->menu),
                                NULL, NULL,
                                (GtkMenuPositionFunc)menu_position_function, this,
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
                      gchar                 *key,
                      glFieldButton         *this)
{
        if (this->priv->label_is_key)
        {
                gtk_label_set_text (GTK_LABEL (this->priv->label), key);
        }

        g_free (this->priv->key);
        this->priv->key = g_strdup (key);

        g_signal_emit (this, signals[KEY_SELECTED], 0, key);
        g_signal_emit (this, signals[CHANGED], 0);
}


/*****************************************************************************/
/* Menu "selection done" callback.                                           */
/*****************************************************************************/
static void
menu_selection_done_cb (GtkMenuShell         *object,
                        glFieldButton        *this)
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
