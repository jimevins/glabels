/*
 *  color-combo.c
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

#include "color-combo.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "color-combo-menu.h"
#include "color-swatch.h"
#include "color.h"
#include "marshal.h"


#define IMAGE_W 24
#define IMAGE_H 24


/*========================================================*/
/* Private types.                                         */
/*========================================================*/

/** GL_COLOR_COMBO Private fields */
struct _glColorComboPrivate {

        guint       color;
        gboolean    is_default_flag;

        guint       default_color;

        GtkWidget  *swatch;

        GtkWidget  *menu;
};

enum {
        COLOR_CHANGED,
        LAST_SIGNAL
};


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static guint signals[LAST_SIGNAL] = {0};


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gl_color_combo_finalize      (GObject             *object);

static gboolean
button_press_event_cb (GtkWidget      *widget,
                       GdkEventButton *event,
                       glColorCombo   *this);

static void
menu_color_changed_cb (glColorComboMenu   *object,
                       guint               color,
                       gboolean            is_default,
                       glColorCombo       *this);

static void
menu_selection_done_cb (GtkMenuShell      *object,
                        glColorCombo      *this);


/*****************************************************************************/
/* Object infrastructure.                                                    */
/*****************************************************************************/
G_DEFINE_TYPE (glColorCombo, gl_color_combo, GTK_TYPE_TOGGLE_BUTTON);


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_color_combo_class_init (glColorComboClass *class)
{
        GObjectClass       *gobject_class = (GObjectClass *) class;

        gl_color_combo_parent_class = g_type_class_peek_parent (class);

        gobject_class->finalize = gl_color_combo_finalize;

        signals[COLOR_CHANGED] =
                g_signal_new ("color_changed",
                              G_OBJECT_CLASS_TYPE (gobject_class),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (glColorComboClass, color_changed),
                              NULL, NULL,
                              gl_marshal_VOID__UINT_BOOLEAN,
                              G_TYPE_NONE,
                              2, G_TYPE_POINTER, G_TYPE_BOOLEAN);

}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_color_combo_init (glColorCombo *this)
{
        GtkWidget *hbox;
        GtkWidget *arrow;

        this->priv = g_new0 (glColorComboPrivate, 1);

        hbox = gtk_hbox_new (FALSE, 3);
        gtk_container_add (GTK_CONTAINER (this), hbox);
        
        this->priv->swatch = gl_color_swatch_new (IMAGE_W, IMAGE_H, GL_COLOR_NONE);
        gtk_box_pack_start (GTK_BOX (hbox), this->priv->swatch, TRUE, TRUE, 0);

        arrow = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_IN);
        gtk_box_pack_end (GTK_BOX (hbox), arrow, FALSE, FALSE, 0);

        g_signal_connect (this, "button_press_event",
                          G_CALLBACK(button_press_event_cb), this);
}


/*****************************************************************************/
/* Finalize Method.                                                          */
/*****************************************************************************/
static void
gl_color_combo_finalize (GObject *object)
{
        glColorCombo    *this;

        g_return_if_fail (object && IS_GL_COLOR_COMBO (object));
        this = GL_COLOR_COMBO (object);

        g_object_ref_sink (this->priv->menu);
        g_free (this->priv);

        G_OBJECT_CLASS (gl_color_combo_parent_class)->finalize (object);
}


/*****************************************************************************/
/** New Object Generator.                                                    */
/*****************************************************************************/
GtkWidget *
gl_color_combo_new (const gchar  *default_label,
                    guint         default_color,
                    guint         color)
{
        glColorCombo *this;

        this = g_object_new (TYPE_GL_COLOR_COMBO, NULL);

        if (!default_label)
        {
                default_label = _("Default Color");
        }

        this->priv->default_color = default_color;
        this->priv->color = color;

        gl_color_swatch_set_color (GL_COLOR_SWATCH (this->priv->swatch), color);

        this->priv->menu = gl_color_combo_menu_new (default_label,
                                                    color);
        gtk_widget_show_all (this->priv->menu);

        g_signal_connect (this->priv->menu, "color_changed",
                          G_CALLBACK (menu_color_changed_cb), this);
        g_signal_connect (this->priv->menu, "selection_done",
                          G_CALLBACK (menu_selection_done_cb), this);

        return GTK_WIDGET (this);
}


/*****************************************************************************/
/* Set color.                                                                */
/*****************************************************************************/
void
gl_color_combo_set_color (glColorCombo  *this,
                          guint          color)
{
        this->priv->color = color;

        gl_color_swatch_set_color (GL_COLOR_SWATCH (this->priv->swatch), color);
}


/*****************************************************************************/
/* Set to default color.                                                     */
/*****************************************************************************/
void
gl_color_combo_set_to_default (glColorCombo  *this)
{
        gl_color_combo_set_color (this, this->priv->default_color);
}

/*****************************************************************************/
/* Get color.                                                                */
/*****************************************************************************/
guint
gl_color_combo_get_color (glColorCombo  *this,
                          gboolean      *is_default)
{
        if (is_default)
        {
                *is_default = this->priv->is_default_flag;
        }

        return this->priv->color;
}


/*****************************************************************************/
/* Menu positioning function.                                                */
/*****************************************************************************/
static void
menu_position_function (GtkMenu       *menu,
                        gint          *x,
                        gint          *y,
                        gboolean      *push_in,
                        glColorCombo  *this)
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
        gtk_widget_size_request (this->priv->menu, &menu_requisition);
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
                       glColorCombo   *this)
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
/* Menu "color changed" callback.                                            */
/*****************************************************************************/
static void
menu_color_changed_cb (glColorComboMenu     *object,
                       guint                 color,
                       gboolean              is_default,
                       glColorCombo         *this)
{
        if (is_default)
        {
                this->priv->color = this->priv->default_color;
        }
        else
        {
                this->priv->color = color;
        }
        this->priv->is_default_flag = is_default;

        gl_color_swatch_set_color (GL_COLOR_SWATCH (this->priv->swatch),
                                   this->priv->color);

        g_signal_emit (this, signals[COLOR_CHANGED], 0,
                       this->priv->color,
                       this->priv->is_default_flag);
}


/*****************************************************************************/
/* Menu "color changed" callback.                                            */
/*****************************************************************************/
static void
menu_selection_done_cb (GtkMenuShell         *object,
                        glColorCombo         *this)
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
