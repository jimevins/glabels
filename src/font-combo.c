/*
 *  font-combo.c
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

#include "font-combo.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "font-combo-menu.h"
#include "font-util.h"
#include "marshal.h"



/*========================================================*/
/* Private types.                                         */
/*========================================================*/

/** GL_FONT_COMBO Private fields */
struct _glFontComboPrivate {

        gchar      *font_family;

        GtkWidget  *label;

        GtkWidget  *menu;
};

enum {
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

static void gl_font_combo_finalize      (GObject             *object);

static gboolean
button_press_event_cb (GtkWidget      *widget,
                       GdkEventButton *event,
                       glFontCombo    *this);

static void
menu_font_changed_cb   (glFontComboMenu  *menu,
                        glFontCombo      *this);

static void
menu_selection_done_cb (GtkMenuShell     *object,
                        glFontCombo      *this);


/*****************************************************************************/
/* Object infrastructure.                                                    */
/*****************************************************************************/
G_DEFINE_TYPE (glFontCombo, gl_font_combo, GTK_TYPE_TOGGLE_BUTTON)


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_font_combo_class_init (glFontComboClass *class)
{
        GObjectClass      *gobject_class = (GObjectClass *) class;

        gl_font_combo_parent_class = g_type_class_peek_parent (class);

        gobject_class->finalize = gl_font_combo_finalize;

        signals[CHANGED] =
                g_signal_new ("changed",
                              G_OBJECT_CLASS_TYPE (gobject_class),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (glFontComboClass, changed),
                              NULL, NULL,
                              gl_marshal_VOID__VOID,
                              G_TYPE_NONE, 0);
}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_font_combo_init (glFontCombo *this)
{
        GtkWidget *hbox;
        GtkWidget *arrow;

        this->priv = g_new0 (glFontComboPrivate, 1);

        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3);
        gtk_container_add (GTK_CONTAINER (this), hbox);
        
        this->priv->label = gtk_label_new ("");
        gtk_misc_set_alignment (GTK_MISC (this->priv->label), 0.0, 0.5);
	gtk_widget_set_size_request (this->priv->label, 180, -1);
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
gl_font_combo_finalize (GObject *object)
{
        glFontCombo    *this;

        g_return_if_fail (object && IS_GL_FONT_COMBO (object));
        this = GL_FONT_COMBO (object);

        g_free (this->priv->font_family);
        g_object_ref_sink (this->priv->menu);
        g_free (this->priv);

        G_OBJECT_CLASS (gl_font_combo_parent_class)->finalize (object);
}


/*****************************************************************************/
/** New Object Generator.                                                    */
/*****************************************************************************/
GtkWidget *
gl_font_combo_new (const gchar  *font_family)
{
        glFontCombo  *this;

        this = g_object_new (TYPE_GL_FONT_COMBO, NULL);

        this->priv->font_family = gl_font_util_validate_family (font_family);

        gtk_label_set_text (GTK_LABEL (this->priv->label), this->priv->font_family);

        this->priv->menu = gl_font_combo_menu_new ();

        gtk_widget_show_all (this->priv->menu);

        g_signal_connect (this->priv->menu, "font_changed",
                          G_CALLBACK (menu_font_changed_cb), this);
        g_signal_connect (this->priv->menu, "selection_done",
                          G_CALLBACK (menu_selection_done_cb), this);

        return GTK_WIDGET (this);
}


/*****************************************************************************/
/** Set relief style.                                                        */
/*****************************************************************************/
void
gl_font_combo_set_relief( glFontCombo    *this,
                          GtkReliefStyle  relief )
{
        gtk_button_set_relief (GTK_BUTTON (this), relief);
}


/*****************************************************************************/
/* Set font family.                                                          */
/*****************************************************************************/
void
gl_font_combo_set_family (glFontCombo  *this,
                          const gchar  *font_family)
{

        this->priv->font_family = gl_font_util_validate_family (font_family);

        gtk_label_set_text (GTK_LABEL (this->priv->label), this->priv->font_family);
}


/*****************************************************************************/
/* Get font family.                                                          */
/*****************************************************************************/
gchar *
gl_font_combo_get_family (glFontCombo  *this)
{
        return g_strdup (this->priv->font_family);
}


/*****************************************************************************/
/* Menu positioning function.                                                */
/*****************************************************************************/
static void
menu_position_function (GtkMenu      *menu,
                        gint         *x,
                        gint         *y,
                        gboolean     *push_in,
                        glFontCombo  *this)
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
                       glFontCombo   *this)
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
/* Menu "font changed" callback.                                             */
/*****************************************************************************/
static void
menu_font_changed_cb (glFontComboMenu     *menu,
                      glFontCombo         *this)
{
        this->priv->font_family = gl_font_combo_menu_get_family (menu);

        gtk_label_set_text (GTK_LABEL (this->priv->label), this->priv->font_family);

        g_signal_emit (this, signals[CHANGED], 0);
}


/*****************************************************************************/
/* Menu "selection done" callback.                                           */
/*****************************************************************************/
static void
menu_selection_done_cb (GtkMenuShell         *object,
                        glFontCombo         *this)
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
