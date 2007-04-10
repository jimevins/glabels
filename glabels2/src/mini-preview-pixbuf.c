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

#include <cairo.h>
#include <math.h>

#include "debug.h"

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
				       const glTemplate  *template,
				       gdouble            scale);

static void draw_label_outlines       (cairo_t           *cr,
				       const glTemplate  *template,
				       gdouble            scale);

static void draw_label_outline        (cairo_t           *cr,
				       const glTemplate  *template,
				       gdouble            x0,
				       gdouble            y0);

static void draw_rect_label_outline   (cairo_t           *cr,
				       const glTemplate  *template,
				       gdouble            x0,
				       gdouble            y0);

static void draw_round_label_outline  (cairo_t           *cr,
				       const glTemplate  *template,
				       gdouble            x0,
				       gdouble            y0);

static void draw_cd_label_outline     (cairo_t           *cr,
				       const glTemplate  *template,
				       gdouble            x0,
				       gdouble            y0);




/****************************************************************************/
/* Create new pixbuf with mini preview of template                          */
/****************************************************************************/
GdkPixbuf *
gl_mini_preview_pixbuf_new (glTemplate *template,
			    gint        width,
			    gint        height)
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
	    const glTemplate  *template,
	    gdouble            scale)
{
	gl_debug (DEBUG_MINI_PREVIEW, "START");

	cairo_save (cr);
	cairo_rectangle (cr, 0.0, 0.0, template->page_width, template->page_height);
	cairo_set_source_rgb (cr, 0.95, 0.95, 0.95);
	cairo_fill_preserve (cr);
	cairo_set_line_width (cr, 1/scale);
	cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
	cairo_stroke (cr);
	cairo_restore (cr);

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Draw label outlines.                                           */
/*--------------------------------------------------------------------------*/
static void
draw_label_outlines (cairo_t           *cr,
		     const glTemplate  *template,
		     gdouble            scale)
{
	const glTemplateLabelType *label_type;
	gint                       i, n_labels;
	glTemplateOrigin          *origins;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	cairo_save (cr);

	cairo_set_line_width (cr, 1.0/scale);

	label_type = gl_template_get_first_label_type (template);

	n_labels = gl_template_get_n_labels (label_type);
	origins  = gl_template_get_origins (label_type);

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
		    const glTemplate  *template,
		    gdouble            x0,
		    gdouble            y0)
{
	const glTemplateLabelType *label_type;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	cairo_save (cr);

	label_type = gl_template_get_first_label_type (template);

	switch (label_type->shape) {

	case GL_TEMPLATE_SHAPE_RECT:
		draw_rect_label_outline (cr, template, x0, y0);
		break;

	case GL_TEMPLATE_SHAPE_ROUND:
		draw_round_label_outline (cr, template, x0, y0);
		break;

	case GL_TEMPLATE_SHAPE_CD:
		draw_cd_label_outline (cr, template, x0, y0);
		break;

	default:
		g_message ("Unknown label style");
		break;
	}

	cairo_restore (cr);

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Draw rectangular label outline.                                */
/*--------------------------------------------------------------------------*/
static void
draw_rect_label_outline (cairo_t           *cr,
			 const glTemplate  *template,
			 gdouble            x0,
			 gdouble            y0)
{
	const glTemplateLabelType *label_type;
	gdouble                    w, h;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	cairo_save (cr);

	label_type = gl_template_get_first_label_type (template);
	gl_template_get_label_size (label_type, &w, &h);

	cairo_rectangle (cr, x0, y0, w, h);

	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
        cairo_fill_preserve (cr);

	cairo_set_source_rgb (cr, 0.25, 0.25, 0.25);
	cairo_stroke (cr);

	cairo_restore (cr);

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Draw round label outline.                                      */
/*--------------------------------------------------------------------------*/
static void
draw_round_label_outline (cairo_t           *cr,
			  const glTemplate  *template,
			  gdouble            x0,
			  gdouble            y0)
{
	const glTemplateLabelType *label_type;
	gdouble                    w, h;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	cairo_save (cr);

	label_type = gl_template_get_first_label_type (template);
	gl_template_get_label_size (label_type, &w, &h);

	cairo_arc (cr, x0+w/2, y0+h/2, w/2, 0.0, 2*M_PI);

	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
        cairo_fill_preserve (cr);

	cairo_set_source_rgb (cr, 0.25, 0.25, 0.25);
	cairo_stroke (cr);

	cairo_restore (cr);

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Draw cd label outline.                                         */
/*--------------------------------------------------------------------------*/
static void
draw_cd_label_outline (cairo_t           *cr,
		       const glTemplate  *template,
		       gdouble            x0,
		       gdouble            y0)
{
	const glTemplateLabelType *label_type;
	gdouble                    w, h;
	gdouble                    xc, yc;
	gdouble                    r1, r2;
        gdouble                    theta1, theta2;


	gl_debug (DEBUG_MINI_PREVIEW, "START");

	cairo_save (cr);

	label_type = gl_template_get_first_label_type (template);
	gl_template_get_label_size (label_type, &w, &h);

	xc = x0 + w/2.0;
	yc = y0 + h/2.0;

	r1 = label_type->size.cd.r1;
	r2 = label_type->size.cd.r2;

        theta1 = acos (w / (2.0*r1));
        theta2 = asin (h / (2.0*r1));

        /* Outer radius, may be clipped in the case of business card CDs. */
        /* Do as a series of 4 arcs, to account for clipping. */
        cairo_new_path (cr);
        cairo_arc (cr, xc, yc, r1, theta1, theta2);
        cairo_arc (cr, xc, yc, r1, M_PI-theta2, M_PI-theta1);
        cairo_arc (cr, xc, yc, r1, M_PI+theta1, M_PI+theta2);
        cairo_arc (cr, xc, yc, r1, 2*M_PI-theta2, 2*M_PI-theta1);
        cairo_close_path (cr);

	/* Hole */
        cairo_new_sub_path (cr);
	cairo_arc (cr, xc, yc, r2, 0.0, 2*M_PI);

	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
        cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
        cairo_fill_preserve (cr);

	cairo_set_source_rgb (cr, 0.25, 0.25, 0.25);
	cairo_stroke (cr);
	
	cairo_restore (cr);

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}



