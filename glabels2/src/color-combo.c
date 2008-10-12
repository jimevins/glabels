/*
 *  color-combo.c
 *  Copyright (C) 2008  Jim Evins <evins@snaught.com>.
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

#include "color-combo-menu.h"
#include <glib/gi18n.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkarrow.h>
#include "marshal.h"
#include "util.h"
#include "color.h"


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

        GdkPixbuf  *icon;

        GtkWidget  *button;
        GtkWidget  *button_image;
        GtkWidget  *dropdown_button;

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

static void
button_clicked_cb (glColorCombo *this);

static GdkPixbuf *
create_pixbuf (glColorCombo   *this,
               gdouble         w,
               gdouble         h);

static gboolean
dropdown_button_press_event_cb (GtkWidget      *widget,
                                GdkEventButton *event,
                                glColorCombo   *this);

static void
menu_color_changed_cb (glColorComboMenu   *object,
                       guint               color,
                       gboolean            is_default,
                       glColorCombo       *this);


/*****************************************************************************/
/* Object infrastructure.                                                    */
/*****************************************************************************/
G_DEFINE_TYPE (glColorCombo, gl_color_combo, GTK_TYPE_HBOX);


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_color_combo_class_init (glColorComboClass *class)
{
        GObjectClass       *gobject_class = (GObjectClass *) class;
        GtkWidgetClass     *widget_class  = (GtkWidgetClass *) class;
        glColorComboClass  *object_class  = (glColorComboClass *) class;

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
        GtkWidget *button_vbox;
        GtkWidget *arrow;

        gtk_box_set_spacing (GTK_BOX (this), 0);

        this->priv = g_new0 (glColorComboPrivate, 1);

        this->priv->button = gtk_button_new ();
        this->priv->button_image = gtk_image_new ();
        gtk_button_set_image (GTK_BUTTON (this->priv->button),
                              this->priv->button_image);
        gtk_button_set_focus_on_click (GTK_BUTTON (this->priv->button), FALSE);
        g_signal_connect_swapped (this->priv->button, "clicked",
                          G_CALLBACK(button_clicked_cb), this);

        gtk_box_pack_start (GTK_BOX (this), this->priv->button, FALSE, FALSE, 0);

        this->priv->dropdown_button = gtk_button_new ();
        arrow = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_IN);
        gtk_container_add (GTK_CONTAINER (this->priv->dropdown_button), arrow);
        gtk_button_set_relief (GTK_BUTTON (this->priv->dropdown_button), GTK_RELIEF_NONE);
        gtk_button_set_focus_on_click (GTK_BUTTON (this->priv->dropdown_button), FALSE);
        g_signal_connect (this->priv->dropdown_button, "button_press_event",
                          G_CALLBACK(dropdown_button_press_event_cb), this);

        gtk_box_pack_start (GTK_BOX (this), this->priv->dropdown_button, FALSE, FALSE, 0);
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

        if (this->priv->icon)
        {
                g_object_unref (this->priv->icon);
        }

        g_free (this->priv);

        G_OBJECT_CLASS (gl_color_combo_parent_class)->finalize (object);
}


/*****************************************************************************/
/** New Object Generator.                                                    */
/*****************************************************************************/
GtkWidget *
gl_color_combo_new (GdkPixbuf    *icon,
                    const gchar  *default_label,
                    guint         default_color,
                    guint         color)
{
        glColorCombo *this;
        GtkWidget    *separator_menuitem;

        this = g_object_new (TYPE_GL_COLOR_COMBO, NULL);

        if (icon)
        {
                this->priv->icon = g_object_ref (icon);
        }

        if (!default_label)
        {
                default_label = _("Default Color");
        }

        this->priv->default_color = default_color;
        this->priv->color = color;

        this->priv->menu = gl_color_combo_menu_new (default_label,
                                                    color);
        gtk_widget_show_all (this->priv->menu);

        gtk_image_set_from_pixbuf (GTK_IMAGE (this->priv->button_image),
                                   create_pixbuf (this, IMAGE_W, IMAGE_H));

        g_signal_connect (this->priv->menu, "color_changed",
                          G_CALLBACK (menu_color_changed_cb), this);

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

        gtk_image_set_from_pixbuf (GTK_IMAGE (this->priv->button_image),
                                   create_pixbuf (this, IMAGE_W, IMAGE_H));
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
/** Set relief style.                                                        */
/*****************************************************************************/
void
gl_color_combo_set_relief( glColorCombo  *this,
                           GtkReliefStyle relief )
{
        gtk_button_set_relief (GTK_BUTTON (this->priv->button), relief);
}


/*****************************************************************************/
/* Color button "clicked" callback.                                          */
/*****************************************************************************/
static void
button_clicked_cb( glColorCombo *this )
{
        g_signal_emit (this, signals[COLOR_CHANGED], 0,
                       this->priv->color,
                       this->priv->is_default_flag);
}


/*****************************************************************************/
/* Create new pixbuf with color preview.                                     */
/*****************************************************************************/
static GdkPixbuf *
create_pixbuf (glColorCombo   *this,
               gdouble         w,
               gdouble         h)
{
        cairo_surface_t   *surface;
        cairo_t           *cr;
        gdouble            image_w, image_h;
        gdouble            swatch_h;
        GdkPixbuf         *pixbuf;

        surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);

        cr = cairo_create (surface);

        cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

        cairo_save (cr);
        cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
        cairo_paint (cr);
        cairo_restore (cr);

        if (this->priv->icon)
        {
                image_w = gdk_pixbuf_get_width (this->priv->icon);
                image_h = gdk_pixbuf_get_height (this->priv->icon);

                cairo_save (cr);

                cairo_rectangle (cr, 0, 0, w, h);
                cairo_scale (cr, w/image_w, h/image_h);
                gdk_cairo_set_source_pixbuf (cr, this->priv->icon, 0, 0);
                cairo_fill (cr);

                cairo_restore (cr);

                swatch_h = h/5 + 1;
        }
        else
        {
                swatch_h = h;
        }

        cairo_rectangle( cr, 1, h-swatch_h+1, w-2, swatch_h-2 );

        cairo_set_source_rgba (cr,
                               GL_COLOR_F_RED   (this->priv->color),
                               GL_COLOR_F_GREEN (this->priv->color),
                               GL_COLOR_F_BLUE  (this->priv->color),
                               GL_COLOR_F_ALPHA (this->priv->color));
        cairo_fill_preserve( cr );

        cairo_set_line_width (cr, 1.0);
        cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
        cairo_stroke (cr);

        cairo_destroy( cr );

        pixbuf = gl_util_cairo_convert_to_pixbuf (surface);
        cairo_surface_destroy (surface);

        return pixbuf;
}


/*****************************************************************************/
/* Menu positioning function.                                                */
/*****************************************************************************/
static void
menu_position_function (GtkMenu *menu,
                        gint *x,
                        gint *y,
                        gboolean *push_in,
                        gpointer user_data)
{
        glColorCombo *this = GL_COLOR_COMBO (user_data);
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
/* Dropdown button "clicked" callback.                                       */
/*****************************************************************************/
static gboolean
dropdown_button_press_event_cb (GtkWidget      *widget,
                                GdkEventButton *event,
                                glColorCombo   *this)
{
        switch (event->button)
        {

        case 1:
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
/* Menu "color changed" callback.                                          */
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

        gtk_image_set_from_pixbuf (GTK_IMAGE (this->priv->button_image),
                                   create_pixbuf (this, IMAGE_W, IMAGE_H));

        g_signal_emit (this, signals[COLOR_CHANGED], 0,
                       this->priv->color,
                       this->priv->is_default_flag);
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
