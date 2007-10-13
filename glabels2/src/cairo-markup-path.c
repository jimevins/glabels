/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  cairo_markup_path.c:  Cairo markup path module
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

#include "cairo-markup-path.h"

#include <math.h>
#include <glib.h>

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

static void       gl_cairo_markup_margin_path         (cairo_t                 *cr,
                                                       const lglTemplateMarkup *markup,
                                                       glLabel                 *label);

static void       gl_cairo_markup_margin_rect_path    (cairo_t                 *cr,
                                                       const lglTemplateMarkup *markup,
                                                       glLabel                 *label);

static void       gl_cairo_markup_margin_round_path   (cairo_t                 *cr,
                                                       const lglTemplateMarkup *markup,
                                                       glLabel                 *label);

static void       gl_cairo_markup_margin_cd_path      (cairo_t                 *cr,
                                                       const lglTemplateMarkup *markup,
                                                       glLabel                 *label);

static void       gl_cairo_markup_line_path           (cairo_t                 *cr,
                                                       const lglTemplateMarkup *markup);

static void       gl_cairo_markup_circle_path         (cairo_t                 *cr,
                                                       const lglTemplateMarkup *markup);

static void       gl_cairo_markup_rect_path           (cairo_t                 *cr,
                                                       const lglTemplateMarkup *markup);


/*--------------------------------------------------------------------------*/
/* Create markup path                                                       */
/*--------------------------------------------------------------------------*/
void
gl_cairo_markup_path (cairo_t                 *cr,
                      const lglTemplateMarkup *markup,
                      glLabel                 *label)
{
        gl_debug (DEBUG_PATH, "START");

        switch (markup->type) {
        case LGL_TEMPLATE_MARKUP_MARGIN:
                gl_cairo_markup_margin_path (cr, markup, label);
                break;
        case LGL_TEMPLATE_MARKUP_LINE:
                gl_cairo_markup_line_path (cr, markup);
                break;
        case LGL_TEMPLATE_MARKUP_CIRCLE:
                gl_cairo_markup_circle_path (cr, markup);
                break;
        case LGL_TEMPLATE_MARKUP_RECT:
                gl_cairo_markup_rect_path (cr, markup);
                break;
        default:
                g_message ("Unknown template markup type");
                break;
        }

        gl_debug (DEBUG_PATH, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw margin markup.                                             */
/*---------------------------------------------------------------------------*/
static void
gl_cairo_markup_margin_path (cairo_t                *cr,
                             const lglTemplateMarkup *markup,
                             glLabel                *label)
{
	const lglTemplateFrame *frame;

        gl_debug (DEBUG_PATH, "START");

        frame = (lglTemplateFrame *)label->template->frames->data;

        switch (frame->shape) {

        case LGL_TEMPLATE_FRAME_SHAPE_RECT:
                gl_cairo_markup_margin_rect_path (cr, markup, label);
                break;

        case LGL_TEMPLATE_FRAME_SHAPE_ROUND:
                gl_cairo_markup_margin_round_path (cr, markup, label);
                break;

        case LGL_TEMPLATE_FRAME_SHAPE_CD:
                gl_cairo_markup_margin_cd_path (cr, markup, label);
                break;

        default:
                g_message ("Unknown template label style");
                break;
        }

        gl_debug (DEBUG_PATH, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw simple recangular margin.                                  */
/*---------------------------------------------------------------------------*/
static void
gl_cairo_markup_margin_rect_path (cairo_t                 *cr,
                                  const lglTemplateMarkup *markup,
                                  glLabel                 *label)
{
        const lglTemplateFrame *frame;
        gdouble                 w, h, r, m;

        gl_debug (DEBUG_PATH, "START");

        frame = (lglTemplateFrame *)label->template->frames->data;

        m = markup->margin.size;

        gl_label_get_size (label, &w, &h);
	w = w - 2*m;
	h = h - 2*m;
        r = MAX (frame->rect.r - m, 0.0);

        if ( r == 0.0 )
        {
                cairo_rectangle (cr, m, m, w, h);
        }
        else
        {
                cairo_new_path (cr);
                cairo_arc_negative (cr, m+r,   m+r,   r, 3*G_PI/2, G_PI);
                cairo_arc_negative (cr, m+r,   m+h-r, r, G_PI,     G_PI/2);
                cairo_arc_negative (cr, m+w-r, m+h-r, r, G_PI/2,   0.);
                cairo_arc_negative (cr, m+w-r, m+r,   r, 2*G_PI,   3*G_PI/2);
                cairo_close_path (cr);
        }

        gl_debug (DEBUG_PATH, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw round margin.                                              */
/*---------------------------------------------------------------------------*/
static void
gl_cairo_markup_margin_round_path (cairo_t                 *cr,
                                   const lglTemplateMarkup *markup,
                                   glLabel                 *label)
{
	const lglTemplateFrame *frame;
	gdouble                 r, m;

	gl_debug (DEBUG_PATH, "START");

        frame = (lglTemplateFrame *)label->template->frames->data;

	r = frame->round.r;
	m = markup->margin.size;

        cairo_arc (cr, r, r, r-m, 0, 2*G_PI);
        cairo_close_path (cr);

	gl_debug (DEBUG_PATH, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw CD margins.                                                */
/*---------------------------------------------------------------------------*/
static void
gl_cairo_markup_margin_cd_path (cairo_t                 *cr,
                                const lglTemplateMarkup *markup,
                                glLabel                 *label)
{
	const lglTemplateFrame *frame;
	gdouble                 m, r1, r2;
	gdouble                 theta1, theta2;
	gdouble                 xc, yc;
	gdouble                 w, h;

	gl_debug (DEBUG_PATH, "START");

        frame = (lglTemplateFrame *)label->template->frames->data;

        gl_label_get_size (label, &w, &h);
        xc = w/2.0;
        yc = h/2.0;

	m  = markup->margin.size;
        r1 = frame->cd.r1 - m;
        r2 = frame->cd.r2 + m;


        /*
         * Outer path (may be clipped)
         */
        theta1 = acos ((w-2*m) / (2.0*r1));
        theta2 = asin ((h-2*m) / (2.0*r1));

        cairo_new_path (cr);
        cairo_arc (cr, xc, yc, r1, theta1, theta2);
        cairo_arc (cr, xc, yc, r1, G_PI-theta2, G_PI-theta1);
        cairo_arc (cr, xc, yc, r1, G_PI+theta1, G_PI+theta2);
        cairo_arc (cr, xc, yc, r1, 2*G_PI-theta2, 2*G_PI-theta1);
        cairo_close_path (cr);


        /* Inner path (hole) */
        cairo_new_sub_path (cr);
        cairo_arc (cr, xc, yc, r2, 0.0, 2*G_PI);
        cairo_close_path (cr);

	gl_debug (DEBUG_PATH, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw line markup.                                               */
/*---------------------------------------------------------------------------*/
static void
gl_cairo_markup_line_path (cairo_t                 *cr,
                           const lglTemplateMarkup *markup)
{
	gl_debug (DEBUG_PATH, "START");

        cairo_move_to (cr, markup->line.x1, markup->line.y1);
        cairo_line_to (cr, markup->line.x2, markup->line.y2);

	gl_debug (DEBUG_PATH, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw circle markup.                                             */
/*---------------------------------------------------------------------------*/
static void
gl_cairo_markup_circle_path (cairo_t                 *cr,
                             const lglTemplateMarkup *markup)
{
	gl_debug (DEBUG_PATH, "START");

        cairo_arc (cr,
                   markup->circle.x0, markup->circle.y0,
                   markup->circle.r,
                   0.0, 2*G_PI);
        cairo_close_path (cr);

	gl_debug (DEBUG_PATH, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw rect markup.                                               */
/*---------------------------------------------------------------------------*/
static void
gl_cairo_markup_rect_path (cairo_t                 *cr,
                           const lglTemplateMarkup *markup)
{
        gdouble x1 = markup->rect.x1;
        gdouble y1 = markup->rect.y1;
        gdouble w  = markup->rect.w;
        gdouble h  = markup->rect.h;
        gdouble r  = markup->rect.r;

	gl_debug (DEBUG_PATH, "START");

        if ( r == 0.0 )
        {
                cairo_rectangle (cr, x1, y1, w, h);
        }
        else
        {
                cairo_new_path (cr);
                cairo_arc_negative (cr, x1+r,   y1+r,   r, 3*G_PI/2, G_PI);
                cairo_arc_negative (cr, x1+r,   y1+h-r, r, G_PI,     G_PI/2);
                cairo_arc_negative (cr, x1+w-r, y1+h-r, r, G_PI/2,   0.);
                cairo_arc_negative (cr, x1+w-r, y1+r,   r, 2*G_PI,   3*G_PI/2);
                cairo_close_path (cr);
        }

	gl_debug (DEBUG_PATH, "END");
}

