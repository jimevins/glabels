/*
 *  mini-preview.c
 *  Copyright (C) 2001-2009  Jim Evins <evins@snaught.com>.
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

#include "mini-preview.h"

#include <math.h>
#include <glib/gi18n.h>

#include <libglabels.h>
#include "cairo-label-path.h"
#include "marshal.h"
#include "color.h"
#include "print.h"

#include "debug.h"


/*===========================================*/
/* Private macros and constants.             */
/*===========================================*/

#define MARGIN 2
#define SHADOW_OFFSET 3

#define ARROW_SCALE 0.35
#define UP_FONT_FAMILY "Sans"
#define UP_SCALE 0.15

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
        CLICKED,
        PRESSED,
        RELEASED,
        LAST_SIGNAL
};

typedef struct {
        gdouble x;
        gdouble y;
} LabelCenter;

struct _glMiniPreviewPrivate {

        GtkWidget      *canvas;

        lglTemplate    *template;
        gint            labels_per_sheet;
        LabelCenter    *centers;

        gint            highlight_first;
        gint            highlight_last;

        gboolean        dragging;
        gint            first_i;
        gint            last_i;
        gint            prev_i;

        gboolean        draw_arrow_flag;
        gboolean        rotate_flag;

        gboolean        update_scheduled_flag;

        glLabel        *label;
        gint            page;
        gint            n_sheets;
        gint            n_copies;
        gint            first;
        gint            last;
        gboolean        collate_flag;
        gboolean        outline_flag;
        gboolean        reverse_flag;
        gboolean        crop_marks_flag;
};


/*===========================================*/
/* Private globals                           */
/*===========================================*/

static gint mini_preview_signals[LAST_SIGNAL] = { 0 };


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void     gl_mini_preview_finalize       (GObject                *object);

static void     gl_mini_preview_construct      (glMiniPreview          *this,
                                                gint                    height,
                                                gint                    width);

static gboolean button_press_event_cb          (GtkWidget              *widget,
                                                GdkEventButton         *event);
static gboolean motion_notify_event_cb         (GtkWidget              *widget,
                                                GdkEventMotion         *event);
static gboolean button_release_event_cb        (GtkWidget              *widget,
                                                GdkEventButton         *event);


static gboolean expose_event_cb                (GtkWidget              *widget,
                                                GdkEventExpose         *event,
                                                glMiniPreview          *this);
static void     style_set_cb                   (GtkWidget              *widget,
                                                GtkStyle               *previous_style,
                                                glMiniPreview          *this);

static void     redraw                         (glMiniPreview          *this);
static void     draw                           (glMiniPreview          *this,
                                                cairo_t                *cr);

static void     draw_shadow                    (glMiniPreview          *this,
                                                cairo_t                *cr,
                                                gdouble                 x,
                                                gdouble                 y,
                                                gdouble                 width,
                                                gdouble                 height);
static void     draw_paper                     (glMiniPreview          *this,
                                                cairo_t                *cr,
                                                gdouble                 width,
                                                gdouble                 height,
                                                gdouble                 line_width);
static void     draw_labels                    (glMiniPreview          *this,
                                                cairo_t                *cr,
                                                lglTemplate            *template,
                                                gdouble                 line_width);
static void     draw_arrow                     (glMiniPreview          *this,
                                                cairo_t                *cr);

static void     draw_rich_preview              (glMiniPreview          *this,
                                                cairo_t                *cr);


static gint     find_closest_label             (glMiniPreview          *this,
                                                gdouble                 x,
                                                gdouble                 y);

static gdouble  set_transform_and_get_scale    (glMiniPreview          *this,
                                                cairo_t                *cr);


/****************************************************************************/
/* Object infrastructure.                                                   */
/****************************************************************************/
G_DEFINE_TYPE (glMiniPreview, gl_mini_preview, GTK_TYPE_EVENT_BOX);


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_mini_preview_class_init (glMiniPreviewClass *class)
{
        GObjectClass   *object_class = G_OBJECT_CLASS (class);
        GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

        gl_debug (DEBUG_MINI_PREVIEW, "START");

        gl_mini_preview_parent_class = g_type_class_peek_parent (class);

        object_class->finalize = gl_mini_preview_finalize;

        widget_class->button_press_event   = button_press_event_cb;
        widget_class->motion_notify_event  = motion_notify_event_cb;
        widget_class->button_release_event = button_release_event_cb;

        mini_preview_signals[CLICKED] =
            g_signal_new ("clicked",
                          G_OBJECT_CLASS_TYPE(object_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (glMiniPreviewClass, clicked),
                          NULL, NULL,
                          gl_marshal_VOID__INT,
                          G_TYPE_NONE, 1, G_TYPE_INT);

        mini_preview_signals[PRESSED] =
            g_signal_new ("pressed",
                          G_OBJECT_CLASS_TYPE(object_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (glMiniPreviewClass, pressed),
                          NULL, NULL,
                          gl_marshal_VOID__INT_INT,
                          G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_INT);

        mini_preview_signals[RELEASED] =
            g_signal_new ("released",
                          G_OBJECT_CLASS_TYPE(object_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (glMiniPreviewClass, released),
                          NULL, NULL,
                          gl_marshal_VOID__INT_INT,
                          G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_INT);

        gl_debug (DEBUG_MINI_PREVIEW, "END");
}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_mini_preview_init (glMiniPreview *this)
{
        gl_debug (DEBUG_MINI_PREVIEW, "START");

        this->priv = g_new0 (glMiniPreviewPrivate, 1);

        gtk_widget_add_events (GTK_WIDGET (this),
                               GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
                               GDK_POINTER_MOTION_MASK);

        gtk_event_box_set_visible_window (GTK_EVENT_BOX (this), FALSE);

        this->priv->canvas = gtk_drawing_area_new ();
        gtk_widget_set_has_window(this->priv->canvas, FALSE);
        gtk_container_add (GTK_CONTAINER (this), this->priv->canvas);

        g_signal_connect (G_OBJECT (this->priv->canvas), "expose-event",
                          G_CALLBACK (expose_event_cb), this);
        g_signal_connect (G_OBJECT (this->priv->canvas), "style-set",
                          G_CALLBACK (style_set_cb), this);

        gl_debug (DEBUG_MINI_PREVIEW, "END");
}


/*****************************************************************************/
/* Finalize Method.                                                          */
/*****************************************************************************/
static void
gl_mini_preview_finalize (GObject *object)
{
        glMiniPreview *this = GL_MINI_PREVIEW (object);

        gl_debug (DEBUG_MINI_PREVIEW, "START");

        g_return_if_fail (object != NULL);
        g_return_if_fail (GL_IS_MINI_PREVIEW (object));

        if (this->priv->label)
        {
                g_object_unref (this->priv->label);
        }
        lgl_template_free (this->priv->template);
        g_free (this->priv->centers);
        g_free (this->priv);

        G_OBJECT_CLASS (gl_mini_preview_parent_class)->finalize (object);

        gl_debug (DEBUG_MINI_PREVIEW, "END");
}


/*****************************************************************************/
/** New Object Generator.                                                    */
/*****************************************************************************/
GtkWidget *
gl_mini_preview_new (gint height,
                     gint width)
{
        glMiniPreview *this;

        gl_debug (DEBUG_MINI_PREVIEW, "START");

        this = g_object_new (gl_mini_preview_get_type (), NULL);

        gl_mini_preview_construct (this, height, width);

        gl_debug (DEBUG_MINI_PREVIEW, "END");

        return GTK_WIDGET (this);
}


/*--------------------------------------------------------------------------*/
/* Construct composite widget.                                              */
/*--------------------------------------------------------------------------*/
static void
gl_mini_preview_construct (glMiniPreview *this,
                           gint           height,
                           gint           width)
{
        gl_debug (DEBUG_MINI_PREVIEW, "START");

        gtk_widget_set_size_request (GTK_WIDGET (this->priv->canvas), width, height);

        gl_debug (DEBUG_MINI_PREVIEW, "END");
}


/****************************************************************************/
/* Set template for mini-preview to determine geometry.                     */
/****************************************************************************/
void
gl_mini_preview_set_by_name (glMiniPreview *this,
                             const gchar   *name)
{
        lglTemplate *template;

        gl_debug (DEBUG_MINI_PREVIEW, "START");

        /* Fetch template */
        template = lgl_db_lookup_template_from_name (name);

        gl_mini_preview_set_template (this, template);

        lgl_template_free (template);

        gl_debug (DEBUG_MINI_PREVIEW, "END");
}


/****************************************************************************/
/* Set template for mini-preview to determine geometry.                     */
/****************************************************************************/
void
gl_mini_preview_set_template (glMiniPreview     *this,
                              const lglTemplate *template)
{
        const lglTemplateFrame    *frame;
        lglTemplateOrigin         *origins;
        gdouble                    w, h;
        gint                       i;

        gl_debug (DEBUG_MINI_PREVIEW, "START");

        frame = (lglTemplateFrame *)template->frames->data;

        /*
         * Set template
         */
        lgl_template_free (this->priv->template);
        this->priv->template = lgl_template_dup (template);

        /*
         * Set labels per sheet
         */
        this->priv->labels_per_sheet = lgl_template_frame_get_n_labels (frame);

        /*
         * Initialize centers
         */
        g_free (this->priv->centers);
        this->priv->centers = g_new0 (LabelCenter, this->priv->labels_per_sheet);
        origins = lgl_template_frame_get_origins (frame);
        lgl_template_frame_get_size (frame, &w, &h);
        for ( i=0; i<this->priv->labels_per_sheet; i++ )
        {
                this->priv->centers[i].x = origins[i].x + w/2.0;
                this->priv->centers[i].y = origins[i].y + h/2.0;
        }
        g_free (origins);

        /*
         * Redraw modified preview
         */
        redraw (this);

        gl_debug (DEBUG_MINI_PREVIEW, "END");
}


/****************************************************************************/
/* Highlight given label outlines.                                          */
/****************************************************************************/
void
gl_mini_preview_highlight_range (glMiniPreview *this,
                                 gint           first_label,
                                 gint           last_label)
{
        gl_debug (DEBUG_MINI_PREVIEW, "START");

        if ( (first_label != this->priv->highlight_first) ||
             (last_label  != this->priv->highlight_last) )
        {

                this->priv->highlight_first = first_label;
                this->priv->highlight_last =  last_label;

                redraw (this);

        }

        gl_debug (DEBUG_MINI_PREVIEW, "END");
}


/****************************************************************************/
/* Set draw arrow.                                                          */
/****************************************************************************/
void
gl_mini_preview_set_draw_arrow (glMiniPreview     *this,
                                gboolean           draw_arrow_flag)
{
        if ( draw_arrow_flag != this->priv->draw_arrow_flag )
        {
                this->priv->draw_arrow_flag = draw_arrow_flag;
                redraw (this);
        }
}


/****************************************************************************/
/* Set rotate flag.                                                         */
/****************************************************************************/
void
gl_mini_preview_set_rotate (glMiniPreview     *this,
                            gboolean           rotate_flag)
{
        if ( rotate_flag != this->priv->rotate_flag )
        {
                this->priv->rotate_flag = rotate_flag;
                redraw (this);
        }
}


/****************************************************************************/
/* Set label.                                                               */
/****************************************************************************/
void
gl_mini_preview_set_label (glMiniPreview     *this,
                           glLabel           *label)
{
        if ( this->priv->label )
        {
                g_object_unref (this->priv->label);
        }
        this->priv->label = g_object_ref (label);
        redraw (this);
}


/****************************************************************************/
/* Set page number.                                                         */
/****************************************************************************/
void
gl_mini_preview_set_page (glMiniPreview     *this,
                          gint               page)
{
        if ( page != this->priv->page )
        {
                this->priv->page = page;
                redraw (this);
        }
}


/****************************************************************************/
/* Set number of sheets.                                                    */
/****************************************************************************/
void
gl_mini_preview_set_n_sheets (glMiniPreview     *this,
                              gint               n_sheets)
{
        if ( n_sheets != this->priv->n_sheets )
        {
                this->priv->n_sheets = n_sheets;
                redraw (this);
        }
}


/****************************************************************************/
/* Set number of copies (merge only).                                       */
/****************************************************************************/
void
gl_mini_preview_set_n_copies (glMiniPreview     *this,
                              gint               n_copies)
{
        if ( n_copies != this->priv->n_copies )
        {
                this->priv->n_copies = n_copies;
                redraw (this);
        }
}


/****************************************************************************/
/* Set first label number on first sheet.                                   */
/****************************************************************************/
void
gl_mini_preview_set_first (glMiniPreview     *this,
                           gint               first)
{
        if ( first != this->priv->first )
        {
                this->priv->first = first;
                redraw (this);
        }
}


/****************************************************************************/
/* Set last label number on first sheet (non-merge only).                   */
/****************************************************************************/
void
gl_mini_preview_set_last (glMiniPreview     *this,
                          gint               last)
{
        if ( last != this->priv->last )
        {
                this->priv->last = last;
                redraw (this);
        }
}


/****************************************************************************/
/* Set collate flag (merge only).                                           */
/****************************************************************************/
void
gl_mini_preview_set_collate_flag (glMiniPreview     *this,
                                  gboolean           collate_flag)
{
        if ( collate_flag != this->priv->collate_flag )
        {
                this->priv->collate_flag = collate_flag;
                redraw (this);
        }
}


/****************************************************************************/
/* Set outline flag.                                                        */
/****************************************************************************/
void
gl_mini_preview_set_outline_flag (glMiniPreview     *this,
                                  gboolean           outline_flag)
{
        if ( outline_flag != this->priv->outline_flag )
        {
                this->priv->outline_flag = outline_flag;
                redraw (this);
        }
}


/****************************************************************************/
/* Set reverse flag.                                                        */
/****************************************************************************/
void
gl_mini_preview_set_reverse_flag (glMiniPreview     *this,
                                  gboolean           reverse_flag)
{
        if ( reverse_flag != this->priv->reverse_flag )
        {
                this->priv->reverse_flag = reverse_flag;
                redraw (this);
        }
}


/****************************************************************************/
/* Set crop marks flag.                                                     */
/****************************************************************************/
void
gl_mini_preview_set_crop_marks_flag (glMiniPreview     *this,
                                     gboolean           crop_marks_flag)
{
        if ( crop_marks_flag != this->priv->crop_marks_flag )
        {
                this->priv->crop_marks_flag = crop_marks_flag;
                redraw (this);
        }
}


/*--------------------------------------------------------------------------*/
/* Set transformation and return scale.                                     */
/*--------------------------------------------------------------------------*/
static gdouble
set_transform_and_get_scale (glMiniPreview *this,
                             cairo_t       *cr)
{
        lglTemplate   *template = this->priv->template;
        GtkAllocation  allocation;
        gdouble        w, h;
        gdouble        scale;
        gdouble        offset_x, offset_y;

        /* Establish scale and origin. */
        gtk_widget_get_allocation (GTK_WIDGET (this), &allocation);
        w = allocation.width;
        h = allocation.height;

        /* establish scale. */
        scale = MIN( (w - 2*MARGIN - 2*SHADOW_OFFSET)/template->page_width,
                     (h - 2*MARGIN - 2*SHADOW_OFFSET)/template->page_height );

        /* Find offset to center preview. */
        offset_x = (w/scale - template->page_width) / 2.0;
        offset_y = (h/scale - template->page_height) / 2.0;

        /* Set transformation. */
        cairo_scale (cr, scale, scale);
        cairo_translate (cr, offset_x, offset_y);

        return scale;
}


/*--------------------------------------------------------------------------*/
/* Button press event handler                                               */
/*--------------------------------------------------------------------------*/
static gboolean
button_press_event_cb (GtkWidget      *widget,
                       GdkEventButton *event)
{
        glMiniPreview     *this = GL_MINI_PREVIEW (widget);
        GdkWindow         *window;
        cairo_t           *cr;
        gdouble            scale;
        gdouble            x, y;
        gint               i;

        gl_debug (DEBUG_MINI_PREVIEW, "START");

        if ( event->button == 1 )
        {

                window = gtk_widget_get_window (this->priv->canvas);

                cr = gdk_cairo_create (window);

                scale = set_transform_and_get_scale (this, cr);

                x = event->x;
                y = event->y;
                cairo_device_to_user (cr, &x, &y);

                i = find_closest_label (this, x, y);

                g_signal_emit (G_OBJECT(this),
                               mini_preview_signals[CLICKED],
                               0, i);

                this->priv->first_i = i;
                this->priv->last_i  = i;
                g_signal_emit (G_OBJECT(this),
                               mini_preview_signals[PRESSED],
                               0, this->priv->first_i, this->priv->last_i);

                this->priv->dragging = TRUE;
                this->priv->prev_i   = i;

                cairo_destroy (cr);
        }

        gl_debug (DEBUG_MINI_PREVIEW, "END");
        return FALSE;
}


/*--------------------------------------------------------------------------*/
/* Motion notify event handler                                              */
/*--------------------------------------------------------------------------*/
static gboolean
motion_notify_event_cb (GtkWidget      *widget,
                        GdkEventMotion *event)
{
        glMiniPreview *this = GL_MINI_PREVIEW (widget);
        GdkWindow     *window;
        cairo_t       *cr;
        gdouble        scale;
        gdouble        x, y;
        gint           i;

        gl_debug (DEBUG_MINI_PREVIEW, "START");

        if (this->priv->dragging)
        {
                window = gtk_widget_get_window (this->priv->canvas);

                cr = gdk_cairo_create (window);

                scale = set_transform_and_get_scale (this, cr);

                x = event->x;
                y = event->y;
                cairo_device_to_user (cr, &x, &y);

                i = find_closest_label (this, x, y);

                if ( i != this->priv->prev_i )
                {
                        this->priv->last_i = i;

                        g_signal_emit (G_OBJECT(this),
                                       mini_preview_signals[PRESSED],
                                       0,
                                       MIN (this->priv->first_i, this->priv->last_i),
                                       MAX (this->priv->first_i, this->priv->last_i));

                        this->priv->prev_i = i;
                }
                cairo_destroy (cr);
        }

        gl_debug (DEBUG_MINI_PREVIEW, "END");
        return FALSE;
}


/*--------------------------------------------------------------------------*/
/* Button release event handler                                             */
/*--------------------------------------------------------------------------*/
static gboolean
button_release_event_cb (GtkWidget      *widget,
                         GdkEventButton *event)
{
        glMiniPreview *this = GL_MINI_PREVIEW (widget);

        gl_debug (DEBUG_MINI_PREVIEW, "START");

        if ( event->button == 1 )
        {
                this->priv->dragging = FALSE;

        }

        g_signal_emit (G_OBJECT(this),
                       mini_preview_signals[RELEASED],
                       0, this->priv->first_i, this->priv->last_i);

        gl_debug (DEBUG_MINI_PREVIEW, "END");
        return FALSE;
}


/*--------------------------------------------------------------------------*/
/* Find index+1 of label closest to given coordinates.                      */
/*--------------------------------------------------------------------------*/
static gint
find_closest_label (glMiniPreview      *this,
                    gdouble             x,
                    gdouble             y)
{
        gint    i;
        gint    min_i;
        gdouble dx, dy, d2, min_d2;

        dx = x - this->priv->centers[0].x;
        dy = y - this->priv->centers[0].y;
        min_d2 = dx*dx + dy*dy;
        min_i = 0;

        for ( i=1; i<this->priv->labels_per_sheet; i++ )
        {
                dx = x - this->priv->centers[i].x;
                dy = y - this->priv->centers[i].y;
                d2 = dx*dx + dy*dy;

                if ( d2 < min_d2 )
                {
                        min_d2 = d2;
                        min_i  = i;
                }
        }

        return min_i + 1;
}


/*--------------------------------------------------------------------------*/
/* Expose event handler.                                                    */
/*--------------------------------------------------------------------------*/
static gboolean
expose_event_cb (GtkWidget       *widget,
                 GdkEventExpose  *event,
                 glMiniPreview   *this)
{
        GdkWindow     *window;
        cairo_t       *cr;
        GtkAllocation  allocation;

        gl_debug (DEBUG_MINI_PREVIEW, "START");

        this->priv->update_scheduled_flag = FALSE;

        window = gtk_widget_get_window (widget);

        cr = gdk_cairo_create (window);

        cairo_rectangle (cr,
                        event->area.x, event->area.y,
                        event->area.width, event->area.height);
        cairo_clip (cr);

        gtk_widget_get_allocation (widget, &allocation);
        cairo_translate (cr, allocation.x, allocation.y);

        draw (this, cr);

        cairo_destroy (cr);

        gl_debug (DEBUG_MINI_PREVIEW, "END");
        return FALSE;
}


/*--------------------------------------------------------------------------*/
/* Style set handler (updates colors when style/theme changes).             */
/*--------------------------------------------------------------------------*/
static void
style_set_cb (GtkWidget        *widget,
              GtkStyle         *previous_style,
              glMiniPreview    *this)
{
        gl_debug (DEBUG_MINI_PREVIEW, "START");

        redraw (this);

        gl_debug (DEBUG_MINI_PREVIEW, "END");
}


/*--------------------------------------------------------------------------*/
/* Redraw.                                                                  */
/*--------------------------------------------------------------------------*/
static void
redraw (glMiniPreview      *this)
{
        GdkWindow *window;
        GdkRegion *region;

        gl_debug (DEBUG_MINI_PREVIEW, "START");

        window = gtk_widget_get_window (this->priv->canvas);

        if (window)
        {

                if ( !this->priv->update_scheduled_flag )
                {
                        this->priv->update_scheduled_flag = TRUE;

                        region = gdk_drawable_get_clip_region (window);
                        gdk_window_invalidate_region (window, region, TRUE);
                        gdk_region_destroy (region);
                }
        }

        gl_debug (DEBUG_MINI_PREVIEW, "END");
}


/*--------------------------------------------------------------------------*/
/* Draw mini preview.                                                       */
/*--------------------------------------------------------------------------*/
static void
draw (glMiniPreview  *this,
      cairo_t        *cr)
{
        lglTemplate *template = this->priv->template;
        gdouble      scale;
        gdouble      shadow_x, shadow_y;


        gl_debug (DEBUG_MINI_PREVIEW, "START");

        if (template)
        {

                scale = set_transform_and_get_scale (this, cr);

                /* update shadow */
                shadow_x = SHADOW_OFFSET/scale;
                shadow_y = SHADOW_OFFSET/scale;

                draw_shadow (this, cr,
                             shadow_x, shadow_y,
                             template->page_width, template->page_height);

                draw_paper (this, cr,
                            template->page_width, template->page_height,
                            1.0/scale);

                draw_labels (this, cr, template, 2.0/scale);

                if (this->priv->draw_arrow_flag)
                {
                        draw_arrow (this, cr);
                }

                if (this->priv->label)
                {
                        draw_rich_preview (this, cr);
                }

        }

        gl_debug (DEBUG_MINI_PREVIEW, "END");

}


/*--------------------------------------------------------------------------*/
/* Draw page shadow                                                         */
/*--------------------------------------------------------------------------*/
static void
draw_shadow (glMiniPreview      *this,
             cairo_t            *cr,
             gdouble             x,
             gdouble             y,
             gdouble             width,
             gdouble             height)
{
        GtkStyle *style;
        guint     shadow_color;

        gl_debug (DEBUG_MINI_PREVIEW, "START");

        cairo_save (cr);

        cairo_rectangle (cr, x, y, width, height);

        style = gtk_widget_get_style (GTK_WIDGET(this));
        shadow_color = gl_color_from_gdk_color (&style->dark[GTK_STATE_NORMAL]);
        cairo_set_source_rgb (cr, GL_COLOR_RGB_ARGS (shadow_color));

        cairo_fill (cr);

        cairo_restore (cr);

        gl_debug (DEBUG_MINI_PREVIEW, "END");
}


/*--------------------------------------------------------------------------*/
/* Draw page                                                                */
/*--------------------------------------------------------------------------*/
static void
draw_paper (glMiniPreview      *this,
            cairo_t            *cr,
            gdouble             width,
            gdouble             height,
            gdouble             line_width)
{
        GtkStyle                  *style;
        guint                      paper_color, outline_color;

        gl_debug (DEBUG_MINI_PREVIEW, "START");

        cairo_save (cr);

        style = gtk_widget_get_style (GTK_WIDGET(this));
        paper_color   = gl_color_from_gdk_color (&style->light[GTK_STATE_NORMAL]);
        outline_color = gl_color_from_gdk_color (&style->fg[GTK_STATE_NORMAL]);

        cairo_rectangle (cr, 0.0, 0.0, width, height);

        cairo_set_source_rgb (cr, GL_COLOR_RGB_ARGS (paper_color));
        cairo_fill_preserve (cr);

        cairo_set_source_rgb (cr, GL_COLOR_RGB_ARGS (outline_color));
        cairo_set_line_width (cr, line_width);
        cairo_stroke (cr);

        cairo_restore (cr);

        gl_debug (DEBUG_MINI_PREVIEW, "END");
}


/*--------------------------------------------------------------------------*/
/* Draw labels                                                              */
/*--------------------------------------------------------------------------*/
static void
draw_labels (glMiniPreview *this,
             cairo_t       *cr,
             lglTemplate   *template,
             gdouble        line_width)
{
        const lglTemplateFrame    *frame;
        gint                       i, n_labels;
        lglTemplateOrigin         *origins;
        GtkStyle                  *style;
        guint                      base_color;
        guint                      highlight_color, paper_color, outline_color;

        gl_debug (DEBUG_MINI_PREVIEW, "START");

        frame = (lglTemplateFrame *)template->frames->data;

        n_labels = lgl_template_frame_get_n_labels (frame);
        origins  = lgl_template_frame_get_origins (frame);

        style = gtk_widget_get_style (GTK_WIDGET(this));
        base_color      = gl_color_from_gdk_color (&style->base[GTK_STATE_SELECTED]);

        paper_color     = gl_color_from_gdk_color (&style->light[GTK_STATE_NORMAL]);
        highlight_color = gl_color_set_opacity (base_color, 0.10);
        if (this->priv->label)
        {
                /* Outlines are more subtle when doing a rich preview. */
                outline_color   = gl_color_set_opacity (base_color, 0.25);
        }
        else
        {
                outline_color   = gl_color_set_opacity (base_color, 1.00);
        }

        for ( i=0; i < n_labels; i++ ) {

                cairo_save (cr);

                cairo_translate (cr, origins[i].x, origins[i].y);
                gl_cairo_label_path (cr, template, FALSE, FALSE);

                if ( ((i+1) >= this->priv->highlight_first) &&
                     ((i+1) <= this->priv->highlight_last) )
                {
                        cairo_set_source_rgba (cr, GL_COLOR_RGBA_ARGS (highlight_color));
                        cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
                        cairo_fill_preserve (cr);
                }

                cairo_set_line_width (cr, line_width);
                cairo_set_source_rgba (cr, GL_COLOR_RGBA_ARGS (outline_color));
                cairo_stroke (cr);

                cairo_restore (cr);

        }

        g_free (origins);

        gl_debug (DEBUG_MINI_PREVIEW, "END");
}


/*--------------------------------------------------------------------------*/
/* Draw arrow to indicate top of labels.                                    */
/*--------------------------------------------------------------------------*/
static void
draw_arrow  (glMiniPreview      *this,
             cairo_t            *cr)
{
        lglTemplateFrame  *frame;
        lglTemplateOrigin *origins;
        gdouble            width, height, min;
        gdouble            x0, y0;
        GtkStyle          *style;
        guint              base_color, arrow_color;

        PangoLayout          *layout;
        PangoFontDescription *desc;
        PangoRectangle        rect;

        /* Translators: "Up" refers to the direction towards the top of a label. */
        const gchar          *up = _("Up");

        frame = (lglTemplateFrame *)this->priv->template->frames->data;

        lgl_template_frame_get_size (frame, &width, &height);

        if ( width != height )
        {

                origins = lgl_template_frame_get_origins (frame);
                x0 = origins[0].x;
                y0 = origins[0].y;
                min = MIN (width, height);
                g_free (origins);

                cairo_save (cr);

                style       = gtk_widget_get_style (GTK_WIDGET(this));
                base_color  = gl_color_from_gdk_color (&style->base[GTK_STATE_SELECTED]);
                arrow_color = gl_color_set_opacity (base_color, 0.25);
                cairo_set_source_rgba (cr, GL_COLOR_RGBA_ARGS (arrow_color));

                cairo_translate (cr, x0 + width/2, y0 + height/2);
                cairo_scale (cr, 1, -1);
                if ( this->priv->rotate_flag )
                {
                        cairo_rotate (cr, -M_PI/2.0);
                }

                cairo_new_path (cr);
                cairo_move_to (cr, 0, -min*ARROW_SCALE/3);
                cairo_line_to (cr, 0, min*ARROW_SCALE);

                cairo_new_sub_path (cr);
                cairo_move_to (cr, -min*ARROW_SCALE/2, min*ARROW_SCALE/2);
                cairo_line_to (cr, 0, min*ARROW_SCALE);
                cairo_line_to (cr, min*ARROW_SCALE/2, min*ARROW_SCALE/2);

                cairo_set_line_width (cr, 0.25*min*ARROW_SCALE);

                cairo_stroke (cr);

                layout = pango_cairo_create_layout (cr);

                desc = pango_font_description_new ();
                pango_font_description_set_family (desc, UP_FONT_FAMILY);
                pango_font_description_set_weight (desc, PANGO_WEIGHT_BOLD);
                pango_font_description_set_size (desc, min*UP_SCALE*PANGO_SCALE);
                pango_layout_set_font_description (layout, desc);
                pango_font_description_free (desc);

                pango_layout_set_text (layout, up, -1);
                pango_layout_set_width (layout, -1);
                pango_layout_get_pixel_extents (layout, NULL, &rect);

                cairo_move_to (cr, -rect.width/2, -min/4+rect.height/2);

                cairo_scale (cr, 1, -1);
                pango_cairo_show_layout (cr, layout);

                cairo_restore (cr);

        }
}


/*--------------------------------------------------------------------------*/
/* Draw rich preview using print renderers.                                 */
/*--------------------------------------------------------------------------*/
static void
draw_rich_preview (glMiniPreview          *this,
                   cairo_t                *cr)
{
        glMerge      *merge;
        glPrintState  state;

        merge = gl_label_get_merge (this->priv->label);

        if (!merge)
        {
                gl_print_simple_sheet (this->priv->label,
                                       cr,
                                       this->priv->page,
                                       this->priv->n_sheets,
                                       this->priv->first,
                                       this->priv->last,
                                       this->priv->outline_flag,
                                       this->priv->reverse_flag,
                                       this->priv->crop_marks_flag);
        }
        else
        {
                /* FIXME: maybe the renderers should be more self contained.
                 *        This will only work for the first page, since
                 *        previous pages must be rendered to establish
                 *        state.
                 */
                state.i_copy = 0;
                state.p_record = (GList *)gl_merge_get_record_list (merge);

                if (this->priv->collate_flag)
                {
                        gl_print_collated_merge_sheet (this->priv->label,
                                                       cr,
                                                       this->priv->page,
                                                       this->priv->n_copies,
                                                       this->priv->first,
                                                       this->priv->outline_flag,
                                                       this->priv->reverse_flag,
                                                       this->priv->crop_marks_flag,
                                                       &state);
                }
                else
                {
                        gl_print_uncollated_merge_sheet (this->priv->label,
                                                         cr,
                                                         this->priv->page,
                                                         this->priv->n_copies,
                                                         this->priv->first,
                                                         this->priv->outline_flag,
                                                         this->priv->reverse_flag,
                                                         this->priv->crop_marks_flag,
                                                         &state);
                }
        }
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
