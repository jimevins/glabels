/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  mini-preview-pixbuf.c:  mini preview pixbuf module
 *
 *  Copyright (C) 2006  Jim Evins <evins@snaught.com>.
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

#include "mini-preview-pixbuf.h"
#include "cairo-label-path.h"

#include <cairo.h>
#include <math.h>

#include "debug.h"

/*===========================================*/
/* Private macros and constants.             */
/*===========================================*/

#define PAPER_RGB_ARGS          0.95,  0.95,  0.95
#define PAPER_OUTLINE_RGB_ARGS  0.0,   0.0,   0.0
#define LABEL_RGB_ARGS          1.0,   1.0,   1.0
#define LABEL_OUTLINE_RGB_ARGS  0.25,  0.25,  0.25

#define PAPER_OUTLINE_WIDTH_PIXELS  1.0
#define LABEL_OUTLINE_WIDTH_PIXELS  1.0

/*===========================================*/
/* Private types                             */
/*===========================================*/


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void draw_paper                (cairo_t           *cr,
				       lglTemplate       *template,
				       gdouble            scale);

static void draw_label_outlines       (cairo_t           *cr,
				       lglTemplate       *template,
				       gdouble            scale);

static void draw_label_outline        (cairo_t           *cr,
				       lglTemplate       *template,
				       gdouble            x0,
				       gdouble            y0);




/****************************************************************************/
/* Create new pixbuf with mini preview of template                          */
/****************************************************************************/
GdkPixbuf *
gl_mini_preview_pixbuf_new (lglTemplate *template,
			    gint         width,
			    gint         height)
{
	cairo_surface_t   *surface;
	cairo_t           *cr;
	GdkPixbuf         *pixbuf;
	gdouble            scale;
	gdouble            w, h;
	gdouble            offset_x, offset_y;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	/* Create pixbuf and cairo context. */
	pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, width, height);
	surface = cairo_image_surface_create_for_data (gdk_pixbuf_get_pixels (pixbuf),
						       CAIRO_FORMAT_RGB24,
						       gdk_pixbuf_get_width (pixbuf),
						       gdk_pixbuf_get_height (pixbuf),
						       gdk_pixbuf_get_rowstride (pixbuf));

	cr = cairo_create (surface);
	cairo_surface_destroy (surface);

	/* Clear pixbuf */
	cairo_save (cr);
        cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
        cairo_paint (cr);
	cairo_restore (cr);

	cairo_set_antialias (cr, CAIRO_ANTIALIAS_GRAY);

	/* Set scale and offset */
	w = width - 1;
	h = height - 1;
	if ( (w/template->page_width) > (h/template->page_height) ) {
		scale = h / template->page_height;
	} else {
		scale = w / template->page_width;
	}
	offset_x = (width/scale - template->page_width) / 2.0;
	offset_y = (height/scale - template->page_height) / 2.0;
	cairo_identity_matrix (cr);
        cairo_scale (cr, scale, scale);
	cairo_translate (cr, offset_x, offset_y);

	/* Draw paper and label outlines */
	draw_paper (cr, template, scale);
	draw_label_outlines (cr, template, scale);

	/* Cleanup */
	cairo_destroy (cr);

	gl_debug (DEBUG_MINI_PREVIEW, "END");

	return pixbuf;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Draw paper and paper outline.                                  */
/*--------------------------------------------------------------------------*/
static void
draw_paper (cairo_t           *cr,
	    lglTemplate       *template,
	    gdouble            scale)
{
	gl_debug (DEBUG_MINI_PREVIEW, "START");

	cairo_save (cr);
	cairo_rectangle (cr, 0.0, 0.0, template->page_width, template->page_height);
	cairo_set_source_rgb (cr, PAPER_RGB_ARGS);
	cairo_fill_preserve (cr);
	cairo_set_line_width (cr, PAPER_OUTLINE_WIDTH_PIXELS/scale);
	cairo_set_source_rgb (cr, PAPER_OUTLINE_RGB_ARGS);
	cairo_stroke (cr);
	cairo_restore (cr);

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Draw label outlines.                                           */
/*--------------------------------------------------------------------------*/
static void
draw_label_outlines (cairo_t           *cr,
		     lglTemplate       *template,
		     gdouble            scale)
{
	const lglTemplateFrame *frame;
	gint                    i, n_labels;
	lglTemplateOrigin      *origins;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	cairo_save (cr);

	cairo_set_line_width (cr, LABEL_OUTLINE_WIDTH_PIXELS/scale);

	frame = lgl_template_get_first_frame (template);

	n_labels = lgl_template_frame_get_n_labels (frame);
	origins  = lgl_template_frame_get_origins (frame);

	for ( i=0; i < n_labels; i++ ) {

		draw_label_outline(cr, template, origins[i].x, origins[i].y);

	}

	g_free (origins);

	cairo_restore (cr);

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Draw label outline.                                            */
/*--------------------------------------------------------------------------*/
static void
draw_label_outline (cairo_t           *cr,
		    lglTemplate       *template,
		    gdouble            x0,
		    gdouble            y0)
{
	gl_debug (DEBUG_MINI_PREVIEW, "START");

	cairo_save (cr);

        cairo_translate (cr, x0, y0);

        gl_cairo_label_path (cr, template, FALSE, FALSE);

	cairo_set_source_rgb (cr, LABEL_RGB_ARGS);
        cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
        cairo_fill_preserve (cr);

	cairo_set_source_rgb (cr, LABEL_OUTLINE_RGB_ARGS);
	cairo_stroke (cr);

	cairo_restore (cr);

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

