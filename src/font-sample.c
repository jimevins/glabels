/*
 *  font-sample.c
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

#include "font-sample.h"

#include "marshal.h"
#include "color.h"


/*===========================================*/
/* Private macros and constants.             */
/*===========================================*/

#define MARGIN 2


/*===========================================*/
/* Private types                             */
/*===========================================*/

struct _glFontSamplePrivate {

        gchar *sample_text;

        gchar *font_family;
};


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void       gl_font_sample_finalize     (GObject        *object);

static void       style_set_cb                (GtkWidget      *widget,
                                               GtkStyle       *previous_style);

static void       redraw                      (glFontSample   *this);

static gboolean   expose_event_cb             (GtkWidget      *widget,
                                               GdkEventExpose *event);

static void       draw_sample                 (glFontSample   *this,
                                               cairo_t        *cr);



/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glFontSample, gl_font_sample, GTK_TYPE_DRAWING_AREA);


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_font_sample_class_init (glFontSampleClass *class)
{
	GObjectClass   *gobject_class = G_OBJECT_CLASS (class);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

        gl_font_sample_parent_class = g_type_class_peek_parent (class);

	gobject_class->finalize    = gl_font_sample_finalize;

        widget_class->expose_event = expose_event_cb;
        widget_class->style_set    = style_set_cb;
}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_font_sample_init (glFontSample *this)
{
        GTK_WIDGET_SET_FLAGS (GTK_WIDGET (this), GTK_NO_WINDOW);

	this->priv = g_new0 (glFontSamplePrivate, 1);
}


/*****************************************************************************/
/* Finalize Method.                                                          */
/*****************************************************************************/
static void
gl_font_sample_finalize (GObject *object)
{
	glFontSample *this = GL_FONT_SAMPLE (object);

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_FONT_SAMPLE (object));

        g_free (this->priv->sample_text);
        g_free (this->priv->font_family);
	g_free (this->priv);

	G_OBJECT_CLASS (gl_font_sample_parent_class)->finalize (object);
}


/*****************************************************************************/
/** New Object Generator.                                                    */
/*****************************************************************************/
GtkWidget *
gl_font_sample_new (gint         w,
                    gint         h,
                    const gchar *sample_text,
                    const gchar *font_family)
{
	glFontSample *this;

	this = g_object_new (GL_TYPE_FONT_SAMPLE, NULL);

        this->priv->sample_text = g_strdup (sample_text);
        this->priv->font_family = g_strdup (font_family);

        gtk_widget_set_size_request (GTK_WIDGET (this), w, h);

	return GTK_WIDGET (this);
}


/*--------------------------------------------------------------------------*/
/* Style set handler (updates colors when style/theme changes).             */
/*--------------------------------------------------------------------------*/
static void
style_set_cb (GtkWidget        *widget,
              GtkStyle         *previous_style)
{
        redraw (GL_FONT_SAMPLE (widget));
}


/*****************************************************************************/
/* Request redraw.                                                           */
/*****************************************************************************/
static void
redraw (glFontSample  *this)
{
        GdkWindow *window;
	GdkRegion *region;

        window = gtk_widget_get_window (GTK_WIDGET (this));

        if (window)
        {
                /* redraw the cairo canvas forcing an expose event */
                region = gdk_drawable_get_clip_region (window);
                gdk_window_invalidate_region (window, region, TRUE);
                gdk_region_destroy (region);
        }
}


/*****************************************************************************/
/* "Expose event" callback.                                                  */
/*****************************************************************************/
static gboolean
expose_event_cb (GtkWidget      *widget,
                 GdkEventExpose *event)
{
        GdkWindow *window;
	cairo_t   *cr;

        window = gtk_widget_get_window (widget);

	cr = gdk_cairo_create (window);

	cairo_rectangle (cr,
			event->area.x, event->area.y,
			event->area.width, event->area.height);
	cairo_clip (cr);

        cairo_translate (cr, widget->allocation.x, widget->allocation.y);

	draw_sample (GL_FONT_SAMPLE (widget), cr);

	cairo_destroy (cr);

	return FALSE;
}


/*****************************************************************************/
/* Draw sample.                                                              */
/*****************************************************************************/
static void
draw_sample (glFontSample *this,
             cairo_t       *cr)
{
        GtkStyle             *style;
        gdouble               w, h;
        guint                 fill_color, line_color;
        PangoLayout          *layout;
        PangoFontDescription *desc;
        PangoRectangle        ink_rect, logical_rect;
        gdouble               layout_x, layout_y, layout_width, layout_height;



        w = GTK_WIDGET (this)->allocation.width;
        h = GTK_WIDGET (this)->allocation.height;


        style = gtk_widget_get_style (GTK_WIDGET (this));
        if ( GTK_WIDGET_IS_SENSITIVE (GTK_WIDGET (this)) )
        {
                fill_color = gl_color_from_gdk_color (&style->light[GTK_STATE_NORMAL]);
                line_color = gl_color_from_gdk_color (&style->fg[GTK_STATE_NORMAL]);
        }
        else
        {
                fill_color = GL_COLOR_NONE;
                line_color = gl_color_from_gdk_color (&style->fg[GTK_STATE_INSENSITIVE]);
        }


        cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

        cairo_rectangle( cr, 0, 0, w-1, h-1 );

        cairo_set_source_rgba (cr, GL_COLOR_RGBA_ARGS (fill_color));
        cairo_fill_preserve( cr );

        cairo_set_source_rgb (cr, GL_COLOR_RGB_ARGS (line_color));
        cairo_set_line_width (cr, 1.0);
        cairo_stroke (cr);


        cairo_set_antialias (cr, CAIRO_ANTIALIAS_DEFAULT);


        layout = pango_cairo_create_layout (cr);

        desc = pango_font_description_new ();
        pango_font_description_set_family (desc, this->priv->font_family);
        pango_font_description_set_weight (desc, PANGO_WEIGHT_NORMAL);
        pango_font_description_set_style  (desc, PANGO_STYLE_NORMAL);
        pango_font_description_set_size   (desc, 0.6 * h * PANGO_SCALE);
        
        pango_layout_set_font_description (layout, desc);
        pango_font_description_free       (desc);

        pango_layout_set_text (layout, this->priv->sample_text, -1);
        pango_layout_set_width (layout, -1);
        pango_layout_get_pixel_extents (layout, &ink_rect, &logical_rect);
        layout_width  = MAX (logical_rect.width, ink_rect.width);
        layout_height = MAX (logical_rect.height, ink_rect.height);

        layout_x = (w - layout_width) / 2.0;
        layout_y = (h - layout_height) / 2.0;

        if (ink_rect.x < logical_rect.x)
        {
                layout_x += logical_rect.x - ink_rect.x;
        }

        if (ink_rect.y < logical_rect.y)
        {
                layout_y += logical_rect.y - ink_rect.y;
        }

        cairo_set_source_rgba (cr, GL_COLOR_RGBA_ARGS (line_color));
        cairo_move_to (cr, layout_x, layout_y);
        pango_cairo_show_layout (cr, layout);

        g_object_unref (layout);
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
