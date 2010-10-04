/*
 *  mini-label-preview.c
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

#include "mini-label-preview.h"

#include <libglabels.h>
#include "cairo-label-path.h"
#include "color.h"
#include "marshal.h"


/*===========================================*/
/* Private macros and constants.             */
/*===========================================*/

#define MARGIN 2
#define SHADOW_OFFSET 3


/*===========================================*/
/* Private types                             */
/*===========================================*/

struct _glMiniLabelPreviewPrivate {

        lglTemplate *template;
        gboolean     rotate_flag;
};


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void       gl_mini_label_preview_finalize (GObject              *object);

static void       style_set_cb                   (GtkWidget            *widget,
                                                  GtkStyle             *previous_style);

static void       redraw                         (glMiniLabelPreview   *this);

static gboolean   draw_cb                        (GtkWidget            *widget,
                                                  cairo_t              *cr);

static void       draw_preview                   (glMiniLabelPreview   *this,
                                                  cairo_t              *cr);


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glMiniLabelPreview, gl_mini_label_preview, GTK_TYPE_DRAWING_AREA);


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_mini_label_preview_class_init (glMiniLabelPreviewClass *class)
{
	GObjectClass   *gobject_class = G_OBJECT_CLASS (class);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

        gl_mini_label_preview_parent_class = g_type_class_peek_parent (class);

	gobject_class->finalize    = gl_mini_label_preview_finalize;

        widget_class->style_set    = style_set_cb;
        widget_class->draw         = draw_cb;
}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_mini_label_preview_init (glMiniLabelPreview *this)
{
        gtk_widget_set_has_window (GTK_WIDGET (this), FALSE);

	this->priv = g_new0 (glMiniLabelPreviewPrivate, 1);
}


/*****************************************************************************/
/* Finalize Method.                                                          */
/*****************************************************************************/
static void
gl_mini_label_preview_finalize (GObject *object)
{
	glMiniLabelPreview *this = GL_MINI_LABEL_PREVIEW (object);

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_MINI_LABEL_PREVIEW (object));

        lgl_template_free (this->priv->template);
	g_free (this->priv);

	G_OBJECT_CLASS (gl_mini_label_preview_parent_class)->finalize (object);
}


/*****************************************************************************/
/** New Object Generator.                                                    */
/*****************************************************************************/
GtkWidget *
gl_mini_label_preview_new (gint         w,
                           gint         h)
{
	glMiniLabelPreview *this;

	this = g_object_new (GL_TYPE_MINI_LABEL_PREVIEW, NULL);

        gtk_widget_set_size_request (GTK_WIDGET (this), w, h);

	return GTK_WIDGET (this);
}


/*****************************************************************************/
/* Set label by name.                                                        */
/*****************************************************************************/
void
gl_mini_label_preview_set_by_name (glMiniLabelPreview *this,
                                   gchar              *name,
                                   gboolean            rotate_flag)
{
        if (!name)
        {
                lgl_template_free (this->priv->template);
                this->priv->template = NULL;
        }
        else
        {
                this->priv->template = lgl_db_lookup_template_from_name (name);
                this->priv->rotate_flag = rotate_flag;
        }

        redraw (this);
}


/*--------------------------------------------------------------------------*/
/* Style set handler (updates colors when style/theme changes).             */
/*--------------------------------------------------------------------------*/
static void
style_set_cb (GtkWidget        *widget,
              GtkStyle         *previous_style)
{
        redraw (GL_MINI_LABEL_PREVIEW (widget));
}


/*****************************************************************************/
/* Request redraw.                                                           */
/*****************************************************************************/
static void
redraw (glMiniLabelPreview  *this)
{
        GdkWindow     *window;
        GtkAllocation  allocation;

        window = gtk_widget_get_window (GTK_WIDGET (this));

        if (window)
        {
                gtk_widget_get_allocation (GTK_WIDGET (this), &allocation);
                gdk_window_invalidate_rect (window, &allocation, FALSE);
        }
}


/*****************************************************************************/
/* "Expose event" callback.                                                  */
/*****************************************************************************/
static gboolean
draw_cb (GtkWidget      *widget,
         cairo_t        *cr)
{

	draw_preview (GL_MINI_LABEL_PREVIEW (widget), cr);

	return FALSE;
}


/*****************************************************************************/
/* Draw sample.                                                              */
/*****************************************************************************/
static void
draw_preview (glMiniLabelPreview *this,
              cairo_t            *cr)
{
        GtkAllocation           allocation;
        GtkStyle               *style;
        gdouble                 w, h;
        guint                   fill_color, line_color, shadow_color;
        const lglTemplateFrame *frame;
        gdouble                 w_label, h_label;
        gdouble                 scale;


        gtk_widget_get_allocation (GTK_WIDGET (this), &allocation);
        w = allocation.width;
        h = allocation.height;


        style = gtk_widget_get_style (GTK_WIDGET (this));
        if ( gtk_widget_is_sensitive (GTK_WIDGET (this)) )
        {
                fill_color   = gl_color_from_gdk_color (&style->light[GTK_STATE_NORMAL]);
                line_color   = gl_color_from_gdk_color (&style->fg[GTK_STATE_NORMAL]);
                shadow_color = gl_color_from_gdk_color (&style->dark[GTK_STATE_NORMAL]);
        }
        else
        {
                fill_color   = GL_COLOR_NONE;
                line_color   = gl_color_from_gdk_color (&style->fg[GTK_STATE_INSENSITIVE]);
                shadow_color = GL_COLOR_NONE;
        }


        if (this->priv->template == NULL)
        {
                return;
        }

        frame = (lglTemplateFrame *)this->priv->template->frames->data;

        if (this->priv->rotate_flag)
        {
                lgl_template_frame_get_size (frame, &h_label, &w_label);
        }
        else
        {
                lgl_template_frame_get_size (frame, &w_label, &h_label);
        }

        scale = MIN( (w - 2*MARGIN - 2*SHADOW_OFFSET)/w_label,
                     (h - 2*MARGIN - 2*SHADOW_OFFSET)/h_label );


        cairo_translate (cr, w/2.0, h/2.0);
        cairo_scale (cr, scale, scale);
        cairo_translate (cr, -w_label/2.0, -h_label/2.0);


        /*
         * Shadow
         */
        cairo_save (cr);
        cairo_translate (cr, SHADOW_OFFSET/scale, SHADOW_OFFSET/scale);
        gl_cairo_label_path (cr, this->priv->template, this->priv->rotate_flag, FALSE);

        cairo_set_source_rgba (cr, GL_COLOR_RGBA_ARGS (shadow_color));
        cairo_fill (cr);
        cairo_restore (cr);

        /*
         * Label + outline
         */
        gl_cairo_label_path (cr, this->priv->template, this->priv->rotate_flag, FALSE);

        cairo_set_source_rgba (cr, GL_COLOR_RGBA_ARGS (fill_color));
        cairo_fill_preserve (cr);

        cairo_set_line_width (cr, 1.0/scale);
        cairo_set_source_rgba (cr, GL_COLOR_RGBA_ARGS (line_color));
        cairo_stroke (cr);

}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
