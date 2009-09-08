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

#include "font-combo-menu.h"
#include <glib/gi18n.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkarrow.h>
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
G_DEFINE_TYPE (glFontCombo, gl_font_combo, GTK_TYPE_TOGGLE_BUTTON);


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_font_combo_class_init (glFontComboClass *class)
{
        GObjectClass      *gobject_class = (GObjectClass *) class;
        GtkWidgetClass    *widget_class  = (GtkWidgetClass *) class;
        glFontComboClass  *object_class  = (glFontComboClass *) class;

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

        hbox = gtk_hbox_new (FALSE, 3);
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

        this->priv->font_family = g_strdup (font_family);

        gtk_label_set_text (GTK_LABEL (this->priv->label), font_family);

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

        this->priv->font_family = g_strdup (font_family);

        gtk_label_set_text (GTK_LABEL (this->priv->label), font_family);
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
menu_position_function (GtkMenu  *menu,
                        gint     *x,
                        gint     *y,
                        gboolean *push_in,
                        gpointer  user_data)
{
        glFontCombo *this = GL_FONT_COMBO (user_data);
        gint          x1, y1;
        gint          menu_h, menu_w;

        gdk_window_get_origin (GTK_WIDGET (this)->window, &x1, &y1);
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
                       glFontCombo   *this)
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
