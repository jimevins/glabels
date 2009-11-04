/*
 *  print.c
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

#include "print.h"

#include <glib/gi18n.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#include <libglabels/libglabels.h>
#include "label.h"
#include "cairo-label-path.h"

#include "debug.h"


/*===========================================*/
/* Private macros and constants.             */
/*===========================================*/

#define OUTLINE_RGB_ARGS          0.0,   0.0,   0.0

#define OUTLINE_WIDTH 0.25

#define TICK_OFFSET  2.25
#define TICK_LENGTH 18.0


/*=========================================================================*/
/* Private types.                                                          */
/*=========================================================================*/

typedef struct _PrintInfo {
        cairo_t    *cr;

	/* gLabels Template */
	lglTemplate *template;
	gboolean     label_rotate_flag;

	/* page size */
	gdouble page_width;
	gdouble page_height;

} PrintInfo;


/*=========================================================================*/
/* Private function prototypes.                                            */
/*=========================================================================*/
static PrintInfo *print_info_new              (cairo_t          *cr,
					       glLabel          *label);

static void       print_info_free             (PrintInfo       **pi);

static void       print_crop_marks            (PrintInfo        *pi);

static void       print_label                 (PrintInfo        *pi,
					       glLabel          *label,
					       gdouble           x,
					       gdouble           y,
					       glMergeRecord    *record,
					       gboolean          outline_flag,
					       gboolean          reverse_flag);


static void       draw_outline                (PrintInfo        *pi,
					       glLabel          *label);

static void       clip_to_outline             (PrintInfo        *pi,
					       glLabel          *label);


/*****************************************************************************/
/* Print simple sheet (no merge data) command.                               */
/*****************************************************************************/
void
gl_print_simple_sheet (glLabel          *label,
                       cairo_t          *cr,
                       gint              page,
                       gint              n_sheets,
                       gint              first,
                       gint              last,
                       gboolean          outline_flag,
                       gboolean          reverse_flag,
                       gboolean          crop_marks_flag)
{
	PrintInfo              *pi;
	const lglTemplateFrame *frame;
	gint                    i_label;
	lglTemplateOrigin      *origins;

	gl_debug (DEBUG_PRINT, "START");

	pi         = print_info_new (cr, label);

        frame = (lglTemplateFrame *)pi->template->frames->data;
	origins = lgl_template_frame_get_origins (frame);

        if (crop_marks_flag) {
                print_crop_marks (pi);
        }

        for (i_label = first - 1; i_label < last; i_label++) {

                print_label (pi, label,
                             origins[i_label].x, origins[i_label].y,
                             NULL, outline_flag, reverse_flag);

        }

	g_free (origins);

	print_info_free (&pi);

	gl_debug (DEBUG_PRINT, "END");
}


/*****************************************************************************/
/* Print collated merge sheet command                                        */
/*****************************************************************************/
void
gl_print_collated_merge_sheet   (glLabel          *label,
                                 cairo_t          *cr,
                                 gint              page,
                                 gint              n_copies,
                                 gint              first,
                                 gboolean          outline_flag,
                                 gboolean          reverse_flag,
                                 gboolean          crop_marks_flag,
                                 glPrintState     *state)
{
	glMerge                   *merge;
	const GList               *record_list;
	PrintInfo                 *pi;
	const lglTemplateFrame    *frame;
	gint                       i_label, n_labels_per_page, i_copy;
	glMergeRecord             *record;
	GList                     *p;
	lglTemplateOrigin         *origins;

	gl_debug (DEBUG_PRINT, "START");

	merge = gl_label_get_merge (label);
	record_list = gl_merge_get_record_list (merge);

	pi = print_info_new (cr, label);
        frame = (lglTemplateFrame *)pi->template->frames->data;

	n_labels_per_page = lgl_template_frame_get_n_labels (frame);
	origins = lgl_template_frame_get_origins (frame);

        if (crop_marks_flag) {
                print_crop_marks (pi);
        }

        if (page == 0)
        {
                state->i_copy  = 0;
                state->p_record = (GList *)record_list;

                i_label = first - 1;
        }
        else
        {
                i_label = 0;
        }


	for ( p=(GList *)state->p_record; p!=NULL; p=p->next ) {
		record = (glMergeRecord *)p->data;
			
		if ( record->select_flag ) {
			for (i_copy = state->i_copy; i_copy < n_copies; i_copy++) {

				print_label (pi, label,
					     origins[i_label].x,
					     origins[i_label].y,
					     record,
					     outline_flag, reverse_flag);

				i_label++;
                                if (i_label == n_labels_per_page)
                                {
                                        g_free (origins);
                                        print_info_free (&pi);

                                        state->i_copy = (i_copy+1) % n_copies;
                                        if (state->i_copy == 0)
                                        {
                                                state->p_record = p->next;
                                        }
                                        else
                                        {
                                                state->p_record = p;
                                        }
                                        return;
                                }
			}
                        state->i_copy = 0;
		}
	}

        g_free (origins);
        print_info_free (&pi);

	gl_debug (DEBUG_PRINT, "END");
}


/*****************************************************************************/
/* Print uncollated merge sheet command                                      */
/*****************************************************************************/
void
gl_print_uncollated_merge_sheet (glLabel          *label,
                                 cairo_t          *cr,
                                 gint              page,
                                 gint              n_copies,
                                 gint              first,
                                 gboolean          outline_flag,
                                 gboolean          reverse_flag,
                                 gboolean          crop_marks_flag,
                                 glPrintState     *state)
{
	glMerge                   *merge;
	const GList               *record_list;
	PrintInfo                 *pi;
	const lglTemplateFrame    *frame;
	gint                       i_label, n_labels_per_page, i_copy;
	glMergeRecord             *record;
	GList                     *p;
	lglTemplateOrigin         *origins;

	gl_debug (DEBUG_PRINT, "START");

	merge = gl_label_get_merge (label);
	record_list = gl_merge_get_record_list (merge);

	pi = print_info_new (cr, label);
        frame = (lglTemplateFrame *)pi->template->frames->data;

	n_labels_per_page = lgl_template_frame_get_n_labels (frame);
	origins = lgl_template_frame_get_origins (frame);

        if (crop_marks_flag) {
                print_crop_marks (pi);
        }

        if (page == 0)
        {
                state->i_copy  = 0;
                state->p_record = (GList *)record_list;

                i_label = first - 1;
        }
        else
        {
                i_label = 0;
        }

	for (i_copy = state->i_copy; i_copy < n_copies; i_copy++) {

		for ( p=state->p_record; p!=NULL; p=p->next ) {
			record = (glMergeRecord *)p->data;
			
			if ( record->select_flag ) {

                                print_label (pi, label,
					     origins[i_label].x,
					     origins[i_label].y,
					     record,
					     outline_flag, reverse_flag);

				i_label++;
                                if (i_label == n_labels_per_page)
                                {
                                        g_free (origins);
                                        print_info_free (&pi);

                                        state->p_record = p->next;
                                        if (state->p_record == NULL)
                                        {
                                                state->p_record = (GList *)record_list;
                                                state->i_copy = i_copy + 1;
                                        }
                                        else
                                        {
                                                state->i_copy = i_copy;
                                        }
                                        return;
                                }
			}
		}
                state->p_record = (GList *)record_list;

	}

	g_free (origins);
	print_info_free (&pi);

	gl_debug (DEBUG_PRINT, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  new print info structure                                        */
/*---------------------------------------------------------------------------*/
static PrintInfo *
print_info_new (cairo_t          *cr,
		glLabel          *label)
{
	PrintInfo            *pi = g_new0 (PrintInfo, 1);

	gl_debug (DEBUG_PRINT, "START");

	g_return_val_if_fail (label && GL_IS_LABEL (label), NULL);

	g_return_val_if_fail (label->template, NULL);
	g_return_val_if_fail (label->template->paper_id, NULL);
	g_return_val_if_fail (label->template->page_width > 0, NULL);
	g_return_val_if_fail (label->template->page_height > 0, NULL);

	pi->cr = cr;

	gl_debug (DEBUG_PRINT,
		  "setting page size = \"%s\"", label->template->paper_id);

	pi->page_width  = label->template->page_width;
	pi->page_height = label->template->page_height;

	pi->template = label->template;
	pi->label_rotate_flag = label->rotate_flag;

	gl_debug (DEBUG_PRINT, "END");

	return pi;
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  free print info structure                                       */
/*---------------------------------------------------------------------------*/
static void
print_info_free (PrintInfo **pi)
{
	gl_debug (DEBUG_PRINT, "START");


	g_free (*pi);
	*pi = NULL;

	gl_debug (DEBUG_PRINT, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Print crop tick marks.                                          */
/*---------------------------------------------------------------------------*/
static void
print_crop_marks (PrintInfo *pi)
{
	const lglTemplateFrame    *frame;
	gdouble                    w, h, page_w, page_h;
	GList                     *p;
	lglTemplateLayout         *layout;
	gdouble                    xmin, ymin, xmax, ymax, dx, dy;
	gdouble                    x1, y1, x2, y2, x3, y3, x4, y4;
	gint                       ix, iy, nx, ny;

	gl_debug (DEBUG_PRINT, "START");

        frame = (lglTemplateFrame *)pi->template->frames->data;

	lgl_template_frame_get_size (frame, &w, &h);

	page_w = pi->page_width;
	page_h = pi->page_height;

        cairo_save (pi->cr);

        cairo_set_source_rgb (pi->cr, OUTLINE_RGB_ARGS);
	cairo_set_line_width (pi->cr, OUTLINE_WIDTH);

	for (p=frame->all.layouts; p != NULL; p=p->next) {

		layout = (lglTemplateLayout *)p->data;

		xmin = layout->x0;
		ymin = layout->y0;
		xmax = layout->x0 + layout->dx*(layout->nx - 1) + w;
		ymax = layout->y0 + layout->dy*(layout->ny - 1) + h;

		dx = layout->dx;
		dy = layout->dy;

		nx = layout->nx;
		ny = layout->ny;

		for (ix=0; ix < nx; ix++) {

			x1 = xmin + ix*dx;
			x2 = x1 + w;

			y1 = MAX((ymin - TICK_OFFSET), 0.0);
			y2 = MAX((y1 - TICK_LENGTH), 0.0);

			y3 = MIN((ymax + TICK_OFFSET), page_h);
			y4 = MIN((y3 + TICK_LENGTH), page_h);

			cairo_move_to (pi->cr, x1, y1);
			cairo_line_to (pi->cr, x1, y2);
			cairo_stroke  (pi->cr);

			cairo_move_to (pi->cr, x2, y1);
			cairo_line_to (pi->cr, x2, y2);
			cairo_stroke  (pi->cr);

			cairo_move_to (pi->cr, x1, y3);
			cairo_line_to (pi->cr, x1, y4);
			cairo_stroke  (pi->cr);

			cairo_move_to (pi->cr, x2, y3);
			cairo_line_to (pi->cr, x2, y4);
			cairo_stroke  (pi->cr);

		}

		for (iy=0; iy < ny; iy++) {

			y1 = ymin + iy*dy;
			y2 = y1 + h;

			x1 = MAX((xmin - TICK_OFFSET), 0.0);
			x2 = MAX((x1 - TICK_LENGTH), 0.0);

			x3 = MIN((xmax + TICK_OFFSET), page_w);
			x4 = MIN((x3 + TICK_LENGTH), page_w);

			cairo_move_to (pi->cr, x1, y1);
			cairo_line_to (pi->cr, x2, y1);
			cairo_stroke  (pi->cr);

			cairo_move_to (pi->cr, x1, y2);
			cairo_line_to (pi->cr, x2, y2);
			cairo_stroke  (pi->cr);

			cairo_move_to (pi->cr, x3, y1);
			cairo_line_to (pi->cr, x4, y1);
			cairo_stroke  (pi->cr);

			cairo_move_to (pi->cr, x3, y2);
			cairo_line_to (pi->cr, x4, y2);
			cairo_stroke  (pi->cr);

		}

	}

        cairo_restore (pi->cr);

	gl_debug (DEBUG_PRINT, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Print i'th label.                                               */
/*---------------------------------------------------------------------------*/
static void
print_label (PrintInfo     *pi,
	     glLabel       *label,
	     gdouble        x,
	     gdouble        y,
	     glMergeRecord *record,
	     gboolean       outline_flag,
	     gboolean       reverse_flag)
{
	const lglTemplateFrame *frame;
	gdouble                 width, height;

	gl_debug (DEBUG_PRINT, "START");

        frame = (lglTemplateFrame *)pi->template->frames->data;

	gl_label_get_size (label, &width, &height);

	cairo_save (pi->cr);

	/* Transform coordinate system to be relative to upper corner */
	/* of the current label */
	cairo_translate (pi->cr, x, y);

	clip_to_outline (pi, label);

	cairo_save (pi->cr);

        /* Special transformations. */
	if (label->rotate_flag) {
		gl_debug (DEBUG_PRINT, "Rotate flag set");
		cairo_rotate (pi->cr, M_PI/2.0);
		cairo_translate (pi->cr, 0.0, -height);
	}
	if ( reverse_flag ) {
		cairo_translate (pi->cr, width, 0.0);
		cairo_scale (pi->cr, -1.0, 1.0);
	}

        gl_label_draw (label, pi->cr, FALSE, record);

	cairo_restore (pi->cr); /* From special transformations. */

	if (outline_flag) {
		draw_outline (pi, label);
	}

	cairo_restore (pi->cr); /* From translation. */

	gl_debug (DEBUG_PRINT, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw outline.                                                   */
/*---------------------------------------------------------------------------*/
static void
draw_outline (PrintInfo *pi,
	      glLabel   *label)
{
	gl_debug (DEBUG_PRINT, "START");

        cairo_save (pi->cr);

	cairo_set_source_rgb (pi->cr, OUTLINE_RGB_ARGS);
	cairo_set_line_width (pi->cr, OUTLINE_WIDTH);

        gl_cairo_label_path (pi->cr, label->template, FALSE, FALSE);

        cairo_stroke (pi->cr);

        cairo_restore (pi->cr);

	gl_debug (DEBUG_PRINT, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Clip to outline.                                                */
/*---------------------------------------------------------------------------*/
static void
clip_to_outline (PrintInfo *pi,
		 glLabel   *label)
{
	gl_debug (DEBUG_PRINT, "START");

        gl_cairo_label_path (pi->cr, label->template, FALSE, TRUE);

        cairo_set_fill_rule (pi->cr, CAIRO_FILL_RULE_EVEN_ODD);
        cairo_clip (pi->cr);

	gl_debug (DEBUG_PRINT, "END");
}





/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
