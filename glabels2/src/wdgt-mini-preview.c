/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_mini_preview.c:  mini preview widget module
 *
 *  Copyright (C) 2001-2007  Jim Evins <evins@snaught.com>.
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

#include "wdgt-mini-preview.h"

#include <math.h>

#include "cairo-label-path.h"
#include "marshal.h"
#include "color.h"

#include "debug.h"

/*===========================================*/
/* Private macros and constants.             */
/*===========================================*/

#define PAPER_RGB_ARGS          1.0,   1.0,   1.0
#define PAPER_OUTLINE_RGB_ARGS  0.0,   0.0,   0.0
#define LABEL_OUTLINE_RGB_ARGS  0.5,   0.5,   0.5

#define SHADOW_X_OFFSET 5
#define SHADOW_Y_OFFSET 5

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CLICKED,
	PRESSED,
	LAST_SIGNAL
};

typedef struct {
	gdouble x;
	gdouble y;
} LabelCenter;

struct _glWdgtMiniPreviewPrivate {

	gint            height;
	gint            width;

	glTemplate     *template;
	gdouble         scale;
	gdouble         offset_x;
	gdouble         offset_y;
	gint            labels_per_sheet;
	LabelCenter    *centers;

	gint            highlight_first;
	gint            highlight_last;

	gboolean        dragging;
	gint            first_i;
	gint            last_i;
	gint            prev_i;
};

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static gint wdgt_mini_preview_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_wdgt_mini_preview_finalize      (GObject                *object);

static void gl_wdgt_mini_preview_construct     (glWdgtMiniPreview      *preview,
						gint                    height,
						gint                    width);

static gboolean expose_event_cb                (GtkWidget              *widget,
						GdkEventExpose         *event);
static void style_set_cb                       (GtkWidget              *widget,
						GtkStyle               *previous_style);
static gboolean button_press_event_cb          (GtkWidget              *widget,
						GdkEventButton         *event);
static gboolean motion_notify_event_cb         (GtkWidget              *widget,
						GdkEventMotion         *event);
static gboolean button_release_event_cb        (GtkWidget              *widget,
						GdkEventButton         *event);


static void redraw                             (GtkWidget              *widget);
static void draw                               (glWdgtMiniPreview      *preview,
						cairo_t                *cr);

static void draw_shadow                        (glWdgtMiniPreview      *preview,
						cairo_t                *cr,
						gdouble  		     x,
						gdouble                 y,
						gdouble                 width,
						gdouble                 height);
static void draw_paper                         (glWdgtMiniPreview      *preview,
						cairo_t                *cr,
						gdouble                 width,
						gdouble                 height,
						gdouble                 line_width);
static void draw_labels                        (glWdgtMiniPreview      *preview,
						cairo_t                *cr,
						glTemplate             *template,
						gdouble                 line_width);

static gint find_closest_label                 (glWdgtMiniPreview      *preview,
						gdouble                 x,
						gdouble                 y);



/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glWdgtMiniPreview, gl_wdgt_mini_preview, GTK_TYPE_DRAWING_AREA);


static void
gl_wdgt_mini_preview_class_init (glWdgtMiniPreviewClass *class)
{
	GObjectClass   *object_class = G_OBJECT_CLASS (class);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	gl_wdgt_mini_preview_parent_class = gtk_type_class (gtk_hbox_get_type ());

	object_class->finalize = gl_wdgt_mini_preview_finalize;

	widget_class->expose_event         = expose_event_cb;
	widget_class->style_set            = style_set_cb;
	widget_class->button_press_event   = button_press_event_cb;
	widget_class->motion_notify_event  = motion_notify_event_cb;
	widget_class->button_release_event = button_release_event_cb;

	wdgt_mini_preview_signals[CLICKED] =
	    g_signal_new ("clicked",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glWdgtMiniPreviewClass, clicked),
			  NULL, NULL,
			  gl_marshal_VOID__INT,
			  G_TYPE_NONE, 1, G_TYPE_INT);

	wdgt_mini_preview_signals[PRESSED] =
	    g_signal_new ("pressed",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glWdgtMiniPreviewClass, pressed),
			  NULL, NULL,
			  gl_marshal_VOID__INT_INT,
			  G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_INT);

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

static void
gl_wdgt_mini_preview_init (glWdgtMiniPreview *preview)
{
	gl_debug (DEBUG_MINI_PREVIEW, "START");

	preview->priv = g_new0 (glWdgtMiniPreviewPrivate, 1);

	gtk_widget_add_events (GTK_WIDGET (preview),
			       GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
			       GDK_POINTER_MOTION_MASK);

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

static void
gl_wdgt_mini_preview_finalize (GObject *object)
{
	glWdgtMiniPreview *preview = GL_WDGT_MINI_PREVIEW (object);

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WDGT_MINI_PREVIEW (object));

	gl_template_free (preview->priv->template);
	g_free (preview->priv->centers);
	g_free (preview->priv);

	G_OBJECT_CLASS (gl_wdgt_mini_preview_parent_class)->finalize (object);

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

GtkWidget *
gl_wdgt_mini_preview_new (gint height,
			  gint width)
{
	glWdgtMiniPreview *preview;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	preview = g_object_new (gl_wdgt_mini_preview_get_type (), NULL);

	gl_wdgt_mini_preview_construct (preview, height, width);

	gl_debug (DEBUG_MINI_PREVIEW, "END");

	return GTK_WIDGET (preview);
}

/*--------------------------------------------------------------------------*/
/* Construct composite widget.                                              */
/*--------------------------------------------------------------------------*/
static void
gl_wdgt_mini_preview_construct (glWdgtMiniPreview *preview,
				gint               height,
				gint               width)
{
	gl_debug (DEBUG_MINI_PREVIEW, "START");

	preview->priv->height = height;
	preview->priv->width  = width;

	gtk_widget_set_size_request (GTK_WIDGET (preview), width, height);

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

/****************************************************************************/
/* Set label for mini-preview to determine geometry.                        */
/****************************************************************************/
void gl_wdgt_mini_preview_set_label_by_name (glWdgtMiniPreview *preview,
					     const gchar       *name)
{
	glTemplate *template;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	/* Fetch template */
	template = gl_template_from_name (name);

	gl_wdgt_mini_preview_set_template (preview, template);

	gl_template_free (template);

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

/****************************************************************************/
/* Set label for mini-preview to determine geometry.                        */
/****************************************************************************/
void gl_wdgt_mini_preview_set_template (glWdgtMiniPreview *preview,
					const glTemplate  *template)
{
	const glTemplateLabelType *label_type;
	glTemplateOrigin          *origins;
	gdouble                    w, h;
	gint                       i;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	label_type = gl_template_get_first_label_type (template);

	/*
	 * Set template
	 */
	gl_template_free (preview->priv->template);
	preview->priv->template = gl_template_dup (template);

	/*
	 * Set scale and offsets
	 */
	w = preview->priv->width - 4 - 2*SHADOW_X_OFFSET;
	h = preview->priv->height - 4 - 2*SHADOW_Y_OFFSET;
	if ( (w/template->page_width) > (h/template->page_height) ) {
		preview->priv->scale = h / template->page_height;
	} else {
		preview->priv->scale = w / template->page_width;
	}
	preview->priv->offset_x = (preview->priv->width/preview->priv->scale - template->page_width) / 2.0;
	preview->priv->offset_y = (preview->priv->height/preview->priv->scale - template->page_height) / 2.0;

	/*
	 * Set labels per sheet
	 */
	preview->priv->labels_per_sheet = gl_template_get_n_labels (label_type);

	/*
	 * Initialize centers
	 */
	g_free (preview->priv->centers);
	preview->priv->centers = g_new0 (LabelCenter, preview->priv->labels_per_sheet);
	origins = gl_template_get_origins (label_type);
	gl_template_get_label_size (label_type, &w, &h);
	for ( i=0; i<preview->priv->labels_per_sheet; i++ )
	{
		preview->priv->centers[i].x = origins[i].x + w/2.0;
		preview->priv->centers[i].y = origins[i].y + h/2.0;
	}
	g_free (origins);

	/*
	 * Redraw modified preview
	 */
	redraw( GTK_WIDGET (preview));

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

/****************************************************************************/
/* Highlight given label outlines.                                          */
/****************************************************************************/
void
gl_wdgt_mini_preview_highlight_range (glWdgtMiniPreview *preview,
				      gint               first_label,
				      gint               last_label)
{
	gl_debug (DEBUG_MINI_PREVIEW, "START");

	preview->priv->highlight_first = first_label;
	preview->priv->highlight_last =  last_label;

	redraw( GTK_WIDGET (preview));

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}


/*--------------------------------------------------------------------------*/
/* Expose event handler.                                                    */
/*--------------------------------------------------------------------------*/
static gboolean
expose_event_cb (GtkWidget       *widget,
		 GdkEventExpose  *event)
{
	cairo_t *cr;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	cr = gdk_cairo_create (widget->window);

	cairo_rectangle (cr,
			event->area.x, event->area.y,
			event->area.width, event->area.height);
	cairo_clip (cr);
	
	draw (GL_WDGT_MINI_PREVIEW (widget), cr);

	cairo_destroy (cr);

	gl_debug (DEBUG_MINI_PREVIEW, "END");
	return FALSE;
}

/*--------------------------------------------------------------------------*/
/* Button press event handler                                               */
/*--------------------------------------------------------------------------*/
static gboolean
button_press_event_cb (GtkWidget      *widget,
		       GdkEventButton *event)
{
	glWdgtMiniPreview *preview = GL_WDGT_MINI_PREVIEW (widget);
	cairo_t           *cr;
	gdouble            x, y;
	gint               i;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	if ( event->button == 1 )
	{
		cr = gdk_cairo_create (widget->window);

		/* Set transformation. */
		cairo_identity_matrix (cr);
		cairo_scale (cr, preview->priv->scale, preview->priv->scale);
		cairo_translate (cr, preview->priv->offset_x, preview->priv->offset_y);

		x = event->x;
		y = event->y;
		cairo_device_to_user (cr, &x, &y);

		i = find_closest_label (preview, x, y);

		g_signal_emit (G_OBJECT(preview),
			       wdgt_mini_preview_signals[CLICKED],
			       0, i);

		preview->priv->first_i = i;
		preview->priv->last_i  = i;
		g_signal_emit (G_OBJECT(preview),
			       wdgt_mini_preview_signals[PRESSED],
			       0, preview->priv->first_i, preview->priv->last_i);

		preview->priv->dragging = TRUE;
		preview->priv->prev_i   = i;

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
	glWdgtMiniPreview *preview = GL_WDGT_MINI_PREVIEW (widget);
	cairo_t           *cr;
	gdouble            x, y;
	gint               i;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	if (preview->priv->dragging)
	{
		cr = gdk_cairo_create (widget->window);

		/* Set transformation. */
		cairo_identity_matrix (cr);
		cairo_scale (cr, preview->priv->scale, preview->priv->scale);
		cairo_translate (cr, preview->priv->offset_x, preview->priv->offset_y);

		x = event->x;
		y = event->y;
		cairo_device_to_user (cr, &x, &y);

		i = find_closest_label (preview, x, y);

		if ( i != preview->priv->prev_i )
		{
			preview->priv->last_i = i;

			g_signal_emit (G_OBJECT(preview),
				       wdgt_mini_preview_signals[PRESSED],
				       0,
				       MIN (preview->priv->first_i, preview->priv->last_i),
				       MAX (preview->priv->first_i, preview->priv->last_i));

			preview->priv->prev_i = i;
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
	glWdgtMiniPreview *preview = GL_WDGT_MINI_PREVIEW (widget);
	
	gl_debug (DEBUG_MINI_PREVIEW, "START");

	if ( event->button == 1 )
	{
		preview->priv->dragging = FALSE;

	}

	gl_debug (DEBUG_MINI_PREVIEW, "END");
	return FALSE;
}

/*--------------------------------------------------------------------------*/
/* Style set handler (updates colors when style/theme changes).             */
/*--------------------------------------------------------------------------*/
static void
style_set_cb (GtkWidget        *widget,
	      GtkStyle         *previous_style)
{
	gl_debug (DEBUG_MINI_PREVIEW, "START");

	redraw( widget );

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

/*--------------------------------------------------------------------------*/
/* Redraw.                                                                  */
/*--------------------------------------------------------------------------*/
static void
redraw (GtkWidget *widget)
{
	GdkRegion *region;
	
	gl_debug (DEBUG_MINI_PREVIEW, "START");

	if (widget->window)
	{

		region = gdk_drawable_get_clip_region (widget->window);

		gdk_window_invalidate_region (widget->window, region, TRUE);
		gdk_window_process_updates (widget->window, TRUE);

		gdk_region_destroy (region);
	}

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

/*--------------------------------------------------------------------------*/
/* Find index+1 of label closest to given coordinates.                      */
/*--------------------------------------------------------------------------*/
static gint
find_closest_label (glWdgtMiniPreview      *preview,
		    gdouble                 x,
		    gdouble                 y)
{
	gint    i;
	gint    min_i;
	gdouble dx, dy, d2, min_d2;

	dx = x - preview->priv->centers[0].x;
	dy = y - preview->priv->centers[0].y;
	min_d2 = dx*dx + dy*dy;
	min_i = 0;

	for ( i=1; i<preview->priv->labels_per_sheet; i++ )
	{
		dx = x - preview->priv->centers[i].x;
		dy = y - preview->priv->centers[i].y;
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
/* Draw mini preview.                                                       */
/*--------------------------------------------------------------------------*/
static void
draw (glWdgtMiniPreview  *preview,
      cairo_t            *cr)
{
	glTemplate *template = preview->priv->template;
	gdouble     shadow_x, shadow_y;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	if (template)
	{

		/* Set transformation. */
		cairo_identity_matrix (cr);
		cairo_scale (cr, preview->priv->scale, preview->priv->scale);
		cairo_translate (cr, preview->priv->offset_x, preview->priv->offset_y);


		/* update shadow */
		shadow_x = SHADOW_X_OFFSET/preview->priv->scale;
		shadow_y = SHADOW_Y_OFFSET/preview->priv->scale;

		draw_shadow (preview, cr,
			     shadow_x, shadow_y,
			     template->page_width, template->page_height);

		draw_paper (preview, cr,
			    template->page_width, template->page_height,
			    1.0/preview->priv->scale);

		draw_labels (preview, cr, template, 1.0/preview->priv->scale);
			     
	}

	gl_debug (DEBUG_MINI_PREVIEW, "END");

}


/*--------------------------------------------------------------------------*/
/* Draw page shadow                                                         */
/*--------------------------------------------------------------------------*/
static void
draw_shadow (glWdgtMiniPreview      *preview,
	     cairo_t                *cr,
	     gdouble  		     x,
	     gdouble                 y,
	     gdouble                 width,
	     gdouble                 height)
{
	GtkStyle *style;
	guint     shadow_color;

        gl_debug (DEBUG_MINI_PREVIEW, "START");

	cairo_save (cr);

	cairo_rectangle (cr, x, y, width, height);

	style = gtk_widget_get_style (GTK_WIDGET(preview));
	shadow_color = gl_color_from_gdk_color (&style->bg[GTK_STATE_ACTIVE]);
	cairo_set_source_rgb (cr, GL_COLOR_RGB_ARGS (shadow_color));

        cairo_fill (cr);

        cairo_restore (cr);

        gl_debug (DEBUG_MINI_PREVIEW, "END");
}

/*--------------------------------------------------------------------------*/
/* Draw page                                                                */
/*--------------------------------------------------------------------------*/
static void
draw_paper (glWdgtMiniPreview      *preview,
	    cairo_t                *cr,
	    gdouble                 width,
	    gdouble                 height,
	    gdouble                 line_width)
{
	cairo_save (cr);

        gl_debug (DEBUG_MINI_PREVIEW, "START");

	cairo_rectangle (cr, 0.0, 0.0, width, height);

	cairo_set_source_rgb (cr, PAPER_RGB_ARGS);
        cairo_fill_preserve (cr);

	cairo_set_source_rgb (cr, PAPER_OUTLINE_RGB_ARGS);
	cairo_set_line_width (cr, line_width);
        cairo_stroke (cr);

        cairo_restore (cr);

        gl_debug (DEBUG_MINI_PREVIEW, "END");
}

/*--------------------------------------------------------------------------*/
/* Draw labels                                                              */
/*--------------------------------------------------------------------------*/
static void
draw_labels (glWdgtMiniPreview *preview,
	     cairo_t           *cr,
	     glTemplate        *template,
	     gdouble            line_width)
{
        const glTemplateLabelType *label_type;
        gint                       i, n_labels;
        glTemplateOrigin          *origins;
	GtkStyle                  *style;
	guint                      highlight_color;

        gl_debug (DEBUG_MINI_PREVIEW, "START");

        label_type = gl_template_get_first_label_type (template);

        n_labels = gl_template_get_n_labels (label_type);
        origins  = gl_template_get_origins (label_type);

	style = gtk_widget_get_style (GTK_WIDGET(preview));
	highlight_color = gl_color_from_gdk_color (&style->base[GTK_STATE_SELECTED]);

        for ( i=0; i < n_labels; i++ ) {

		cairo_save (cr);

                cairo_translate (cr, origins[i].x, origins[i].y);
                gl_cairo_label_path (cr, template, FALSE, FALSE);

		if ( ((i+1) >= preview->priv->highlight_first) &&
		     ((i+1) <= preview->priv->highlight_last) )
		{
			cairo_set_source_rgb (cr, GL_COLOR_RGB_ARGS (highlight_color));
		}
		else
		{
			cairo_set_source_rgb (cr, PAPER_RGB_ARGS);
		}
		cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
		cairo_fill_preserve (cr);

		cairo_set_line_width (cr, line_width);
		cairo_set_source_rgb (cr, LABEL_OUTLINE_RGB_ARGS);
		cairo_stroke (cr);

		cairo_restore (cr);

        }

        g_free (origins);

        gl_debug (DEBUG_MINI_PREVIEW, "END");
}

