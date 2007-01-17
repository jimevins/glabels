/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  print.c:  Print module
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

#include "print.h"

#include <glib/gi18n.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <gtk/gtkenums.h>

#include "label.h"
#include "label-text.h"
#include "label-box.h"
#include "label-line.h"
#include "label-ellipse.h"
#include "label-image.h"
#include "label-barcode.h"
#include "bc.h"
#include <libglabels/template.h>
#include "color.h"

#include "debug.h"

#define ARC_FINE   2  /* Resolution in degrees of large arcs */

#define FONT_SCALE (72.0/96.0)
#define TICK_OFFSET  2.25
#define TICK_LENGTH 18.0

/*=========================================================================*/
/* Private types.                                                          */
/*=========================================================================*/

typedef struct _PrintInfo {
        cairo_t    *cr;

	/* gLabels Template */
	glTemplate *template;
	gboolean    label_rotate_flag;

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


static void       draw_label                  (PrintInfo        *pi,
					       glLabel          *label,
					       glMergeRecord    *record);


static void       draw_object                 (PrintInfo        *pi,
					       glLabelObject    *object,
					       glMergeRecord    *record);

static void       draw_text_object            (PrintInfo        *pi,
					       glLabelText      *object,
					       glMergeRecord    *record);

static void       draw_box_object             (PrintInfo        *pi,
					       glLabelBox       *object,
					       glMergeRecord    *record);

static void       draw_line_object            (PrintInfo        *pi,
					       glLabelLine      *object,
					       glMergeRecord    *record);						   

static void       draw_ellipse_object         (PrintInfo        *pi,
					       glLabelEllipse   *object,
					       glMergeRecord    *record);

static void       draw_image_object           (PrintInfo        *pi,
					       glLabelImage     *object,
					       glMergeRecord    *record);

static void       draw_barcode_object         (PrintInfo        *pi,
					       glLabelBarcode   *object,
					       glMergeRecord    *record);


static void       draw_outline                (PrintInfo        *pi,
					       glLabel          *label);

static void       clip_to_outline             (PrintInfo        *pi,
					       glLabel          *label);

static void       clip_punchouts              (PrintInfo        *pi,
					       glLabel          *label);


static void       create_rounded_rectangle_path (cairo_t           *cr,
						 gdouble            x0,
						 gdouble            y0,
						 gdouble            w,
						 gdouble            h,
						 gdouble            r);

static void       create_ellipse_path           (cairo_t           *cr,
						 gdouble            x0,
						 gdouble            y0,
						 gdouble            rx,
						 gdouble            ry);

static void       create_cd_path                (cairo_t           *cr,
						 gdouble            x0,
						 gdouble            y0,
						 gdouble            w,
						 gdouble            h,
						 gdouble            r1,
                                                 gdouble            r2);


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
	PrintInfo                 *pi;
	const glTemplateLabelType *label_type;
	gint                       i_label;
	glTemplateOrigin          *origins;

	gl_debug (DEBUG_PRINT, "START");

	pi         = print_info_new (cr, label);

	label_type = gl_template_get_first_label_type (pi->template);
	origins = gl_template_get_origins (label_type);

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
	const glTemplateLabelType *label_type;
	gint                       i_label, n_labels_per_page, i_copy;
	glMergeRecord             *record;
	GList                     *p;
	glTemplateOrigin          *origins;

	gl_debug (DEBUG_PRINT, "START");

	merge = gl_label_get_merge (label);
	record_list = gl_merge_get_record_list (merge);

	pi = print_info_new (cr, label);
	label_type = gl_template_get_first_label_type (pi->template);

	n_labels_per_page = gl_template_get_n_labels (label_type);
	origins = gl_template_get_origins (label_type);

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
	const glTemplateLabelType *label_type;
	gint                       i_label, n_labels_per_page, i_copy;
	glMergeRecord             *record;
	GList                     *p;
	glTemplateOrigin          *origins;

	gl_debug (DEBUG_PRINT, "START");

	merge = gl_label_get_merge (label);
	record_list = gl_merge_get_record_list (merge);

	pi = print_info_new (cr, label);
	label_type = gl_template_get_first_label_type (pi->template);

	n_labels_per_page = gl_template_get_n_labels (label_type);
	origins = gl_template_get_origins (label_type);

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
	glTemplate           *template;

	gl_debug (DEBUG_PRINT, "START");

	g_return_val_if_fail (label && GL_IS_LABEL (label), NULL);

	template = gl_label_get_template (label);

	g_return_val_if_fail (template, NULL);
	g_return_val_if_fail (template->page_size, NULL);
	g_return_val_if_fail (template->page_width > 0, NULL);
	g_return_val_if_fail (template->page_height > 0, NULL);

	pi->cr = cr;

	gl_debug (DEBUG_PRINT,
		  "setting page size = \"%s\"", template->page_size);

	pi->page_width  = template->page_width;
	pi->page_height = template->page_height;

	pi->template = template;
	pi->label_rotate_flag = gl_label_get_rotate_flag (label);

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

	gl_template_free ((*pi)->template);
	(*pi)->template = NULL;

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
	const glTemplateLabelType *label_type;
	gdouble                    w, h, page_w, page_h;
	GList                     *p;
	glTemplateLayout          *layout;
	gdouble                    xmin, ymin, xmax, ymax, dx, dy;
	gdouble                    x1, y1, x2, y2, x3, y3, x4, y4;
	gint                       ix, iy, nx, ny;

	gl_debug (DEBUG_PRINT, "START");

	label_type = gl_template_get_first_label_type (pi->template);

	gl_template_get_label_size (label_type, &w, &h);

	page_w = pi->page_width;
	page_h = pi->page_height;

        cairo_save (pi->cr);

        cairo_set_source_rgba (pi->cr, 0.0, 0.0, 0.0, 1.0);
	cairo_set_line_width  (pi->cr, 0.25);

	for (p=label_type->layouts; p != NULL; p=p->next) {

		layout = (glTemplateLayout *)p->data;

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
	const glTemplateLabelType *label_type;
	gdouble                    width, height;

	gl_debug (DEBUG_PRINT, "START");

	label_type = gl_template_get_first_label_type (pi->template);

	gl_label_get_size (label, &width, &height);

	cairo_save (pi->cr);

	/* Transform coordinate system to be relative to upper corner */
	/* of the current label */
	cairo_translate (pi->cr, x, y);
	if (gl_label_get_rotate_flag (label)) {
		gl_debug (DEBUG_PRINT, "Rotate flag set");
		cairo_rotate (pi->cr, -M_PI/2.0);
		cairo_translate (pi->cr, -width, 0.0);
	}
	if ( reverse_flag ) {
		cairo_translate (pi->cr, width, 0.0);
		cairo_scale (pi->cr, -1.0, 1.0);
	}

	clip_to_outline (pi, label);
	draw_label (pi, label, record);
	if (outline_flag) {
		draw_outline (pi, label);
	}

	cairo_restore (pi->cr);

	gl_debug (DEBUG_PRINT, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw label.                                                     */
/*---------------------------------------------------------------------------*/
static void
draw_label (PrintInfo     *pi,
	    glLabel       *label,
	    glMergeRecord *record)
{
	GList *p_obj;
	glLabelObject *object;

	gl_debug (DEBUG_PRINT, "START");

	for (p_obj = label->objects; p_obj != NULL; p_obj = p_obj->next) {
		object = (glLabelObject *) p_obj->data;

		draw_object (pi, object, record);
	}

	gl_debug (DEBUG_PRINT, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw object.                                                    */
/*---------------------------------------------------------------------------*/
static void
draw_object (PrintInfo     *pi,
	     glLabelObject *object,
	     glMergeRecord *record)
{
	gdouble x0, y0;
	gdouble a[6];
        cairo_matrix_t matrix;

	gl_debug (DEBUG_PRINT, "START");

	gl_label_object_get_position (object, &x0, &y0);
	gl_label_object_get_affine (object, a);
        cairo_matrix_init (&matrix, a[0], a[1], a[2], a[3], a[4], a[5]);

	cairo_save (pi->cr);

	cairo_translate (pi->cr, x0, y0);
	cairo_transform (pi->cr, &matrix);


	if (GL_IS_LABEL_TEXT(object)) {
		draw_text_object (pi, GL_LABEL_TEXT(object), record);
	} else if (GL_IS_LABEL_BOX(object)) {
		draw_box_object (pi, GL_LABEL_BOX(object), record);
	} else if (GL_IS_LABEL_LINE(object)) {
		draw_line_object (pi, GL_LABEL_LINE(object), record);
	} else if (GL_IS_LABEL_ELLIPSE(object)) {
		draw_ellipse_object (pi, GL_LABEL_ELLIPSE(object), record);
	} else if (GL_IS_LABEL_IMAGE(object)) {
		draw_image_object (pi, GL_LABEL_IMAGE(object), record);
	} else if (GL_IS_LABEL_BARCODE(object)) {
		draw_barcode_object (pi, GL_LABEL_BARCODE(object), record);
	}

	cairo_restore (pi->cr);

	gl_debug (DEBUG_PRINT, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw text object.                                               */
/*---------------------------------------------------------------------------*/
static void
draw_text_object (PrintInfo     *pi,
		  glLabelText   *object,
		  glMergeRecord *record)
{
	gint             i;
	gdouble          x_offset, y_offset, w, object_w, object_h;
	gchar           *text;
	GList           *lines;
	gchar           *font_family;
	gdouble          font_size;
	GnomeFontWeight  font_weight;
	gboolean         font_italic_flag;
	guint            color;
	glColorNode     *color_node;
	GtkJustification just;
	gboolean         auto_shrink;
	ArtDRect         bbox;
	gdouble          affine[6];
	gdouble          text_line_spacing;
	gboolean         shadow_state;
	gdouble          shadow_x, shadow_y;
	glColorNode     *shadow_color_node;
	gdouble          shadow_opacity;
	guint            shadow_color;
        PangoAlignment   alignment;
        PangoStyle       style;
        PangoLayout     *layout;
        PangoFontDescription *desc;


	gl_debug (DEBUG_PRINT, "START");

	gl_label_object_get_size (GL_LABEL_OBJECT(object), &object_w, &object_h);
	lines = gl_label_text_get_lines (object);
	font_family = gl_label_object_get_font_family (GL_LABEL_OBJECT(object));
	font_size = gl_label_object_get_font_size (GL_LABEL_OBJECT(object));
	font_weight = gl_label_object_get_font_weight (GL_LABEL_OBJECT(object));
	font_italic_flag = gl_label_object_get_font_italic_flag (GL_LABEL_OBJECT(object));

	color_node = gl_label_object_get_text_color (GL_LABEL_OBJECT(object));
	color = gl_color_node_expand (color_node, record);
	gl_color_node_free (&color_node);
	
	just = gl_label_object_get_text_alignment (GL_LABEL_OBJECT(object));
	text_line_spacing =
		gl_label_object_get_text_line_spacing (GL_LABEL_OBJECT(object));
	auto_shrink = gl_label_text_get_auto_shrink (object);

	shadow_state = gl_label_object_get_shadow_state (GL_LABEL_OBJECT (object));
	gl_label_object_get_shadow_offset (GL_LABEL_OBJECT (object), &shadow_x, &shadow_y);
	shadow_color_node = gl_label_object_get_shadow_color (GL_LABEL_OBJECT (object));
	if (shadow_color_node->field_flag)
	{
		shadow_color_node->color = GL_COLOR_SHADOW_MERGE_DEFAULT;
	}
	shadow_opacity = gl_label_object_get_shadow_opacity (GL_LABEL_OBJECT (object));
	shadow_color = gl_color_shadow (shadow_color_node->color, shadow_opacity, color);
	gl_color_node_free (&shadow_color_node);

	text = gl_text_node_lines_expand (lines, record);

        switch (just) {
        case GTK_JUSTIFY_LEFT:
                alignment = PANGO_ALIGN_LEFT;
                break;
        case GTK_JUSTIFY_CENTER:
                alignment = PANGO_ALIGN_CENTER;
                break;
        case GTK_JUSTIFY_RIGHT:
                alignment = PANGO_ALIGN_RIGHT;
                break;
        default:
                alignment = PANGO_ALIGN_LEFT;
                break;	/* shouldn't happen */
        }

        style = font_italic_flag ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL;


	if (record && auto_shrink) {

		/* auto shrink text size to keep within text box limits. */

                layout = pango_cairo_create_layout (pi->cr);

                desc = pango_font_description_new ();
                pango_font_description_set_family (desc, font_family);
                pango_font_description_set_weight (desc, font_weight);
                pango_font_description_set_style  (desc, style);
                pango_font_description_set_size   (desc, font_size * PANGO_SCALE * FONT_SCALE);
                pango_layout_set_font_description (layout, desc);
                pango_font_description_free       (desc);

                pango_layout_set_text (layout, text, -1);
                w = pango_layout_get_width (layout) / PANGO_SCALE;

                g_object_unref (layout);

                if ( w > object_w )
                {
                        /* Scale down. */
                        font_size *= (object_w-2*GL_LABEL_TEXT_MARGIN)/w;

                        /* Round down to nearest 1/2 point */
                        font_size = (int)(font_size*2.0) / 2.0;

                        /* don't get ridiculously small. */
                        if (font_size < 1.0)
                        {
                                font_size = 1.0;
                        }
                }
        }


        layout = pango_cairo_create_layout (pi->cr);

        desc = pango_font_description_new ();
        pango_font_description_set_family (desc, font_family);
        pango_font_description_set_weight (desc, font_weight);
        pango_font_description_set_style  (desc, style);
        pango_font_description_set_size   (desc, font_size * PANGO_SCALE * FONT_SCALE);
        pango_layout_set_font_description (layout, desc);
        pango_font_description_free       (desc);

        pango_layout_set_text (layout, text, -1);
        pango_layout_set_spacing (layout, font_size * (text_line_spacing-1) * PANGO_SCALE);
        pango_layout_set_width (layout, object_w * PANGO_SCALE);
        pango_layout_set_alignment (layout, alignment);

        if (shadow_state)
        {
                cairo_set_source_rgba (pi->cr,
                                       GL_COLOR_F_RED (shadow_color),
                                       GL_COLOR_F_GREEN (shadow_color),
                                       GL_COLOR_F_BLUE (shadow_color),
                                       GL_COLOR_F_ALPHA (shadow_color));

                cairo_move_to (pi->cr, shadow_x + GL_LABEL_TEXT_MARGIN, shadow_y);
                pango_cairo_show_layout (pi->cr, layout);
        }

        cairo_set_source_rgba (pi->cr,
                               GL_COLOR_F_RED (color),
                               GL_COLOR_F_GREEN (color),
                               GL_COLOR_F_BLUE (color),
                               GL_COLOR_F_ALPHA (color));

        cairo_move_to (pi->cr, GL_LABEL_TEXT_MARGIN, 0);
        pango_cairo_show_layout (pi->cr, layout);

        g_object_unref (layout);


	gl_text_node_lines_free (&lines);
	g_free (font_family);

	gl_debug (DEBUG_PRINT, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw box object.                                                */
/*---------------------------------------------------------------------------*/
static void
draw_box_object (PrintInfo  *pi,
		 glLabelBox *object,
		 glMergeRecord  *record)
{
	gdouble      w, h;
	gdouble      line_width;
	guint        line_color;
	glColorNode *line_color_node;
	glColorNode *fill_color_node;
	guint        fill_color;
	gboolean     shadow_state;
	gdouble      shadow_x, shadow_y;
	glColorNode *shadow_color_node;
	gdouble      shadow_opacity;
	guint        shadow_line_color;
	guint        shadow_fill_color;

	gl_debug (DEBUG_PRINT, "START");

	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_object_get_line_width (GL_LABEL_OBJECT(object));
	
	line_color_node = gl_label_object_get_line_color (GL_LABEL_OBJECT(object));
	fill_color_node = gl_label_object_get_fill_color (GL_LABEL_OBJECT(object));
	line_color = gl_color_node_expand (line_color_node, record);
	fill_color = gl_color_node_expand (fill_color_node, record);
	gl_color_node_free (&line_color_node);
	gl_color_node_free (&fill_color_node);

	shadow_state = gl_label_object_get_shadow_state (GL_LABEL_OBJECT (object));
	gl_label_object_get_shadow_offset (GL_LABEL_OBJECT (object), &shadow_x, &shadow_y);
	shadow_color_node = gl_label_object_get_shadow_color (GL_LABEL_OBJECT (object));
	if (shadow_color_node->field_flag)
	{
		shadow_color_node->color = GL_COLOR_SHADOW_MERGE_DEFAULT;
	}
	shadow_opacity = gl_label_object_get_shadow_opacity (GL_LABEL_OBJECT (object));
	shadow_line_color = gl_color_shadow (shadow_color_node->color, shadow_opacity, line_color);
	shadow_fill_color = gl_color_shadow (shadow_color_node->color, shadow_opacity, fill_color);
	gl_color_node_free (&shadow_color_node);
	
	if (shadow_state)
	{
		/* Draw fill shadow */
		cairo_rectangle (pi->cr, shadow_x, shadow_y, w, h);

		cairo_set_source_rgba (pi->cr,
                                       GL_COLOR_F_RED (shadow_fill_color),
                                       GL_COLOR_F_GREEN (shadow_fill_color),
                                       GL_COLOR_F_BLUE (shadow_fill_color),
                                       GL_COLOR_F_ALPHA (shadow_fill_color));
		cairo_fill_preserve (pi->cr);

		/* Draw outline shadow */
		cairo_set_source_rgba (pi->cr,
                                       GL_COLOR_F_RED (shadow_line_color),
                                       GL_COLOR_F_GREEN (shadow_line_color),
                                       GL_COLOR_F_BLUE (shadow_line_color),
                                       GL_COLOR_F_ALPHA (shadow_line_color));
		cairo_set_line_width (pi->cr, line_width);
		cairo_stroke (pi->cr);
	}

	/* Paint fill color */
	cairo_rectangle (pi->cr, 0.0, 0.0, w, h);
        cairo_set_source_rgba (pi->cr,
                               GL_COLOR_F_RED (fill_color),
                               GL_COLOR_F_GREEN (fill_color),
                               GL_COLOR_F_BLUE (fill_color),
                               GL_COLOR_F_ALPHA (fill_color));
	cairo_fill_preserve (pi->cr);

	/* Draw outline */
        cairo_set_source_rgba (pi->cr,
                               GL_COLOR_F_RED (line_color),
                               GL_COLOR_F_GREEN (line_color),
                               GL_COLOR_F_BLUE (line_color),
                               GL_COLOR_F_ALPHA (line_color));
        cairo_set_line_width (pi->cr, line_width);
        cairo_stroke (pi->cr);

	gl_debug (DEBUG_PRINT, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw line object.                                               */
/*---------------------------------------------------------------------------*/
static void
draw_line_object (PrintInfo   *pi,
		  glLabelLine *object,
		  glMergeRecord  *record)
{
	gdouble      w, h;
	gdouble      line_width;
	guint        line_color;
	glColorNode *line_color_node;
	gboolean     shadow_state;
	gdouble      shadow_x, shadow_y;
	glColorNode *shadow_color_node;
	gdouble      shadow_opacity;
	guint        shadow_line_color;

	gl_debug (DEBUG_PRINT, "START");

	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_object_get_line_width (GL_LABEL_OBJECT(object));
	
	line_color_node = gl_label_object_get_line_color (GL_LABEL_OBJECT(object));
	line_color = gl_color_node_expand (line_color_node, record);
	gl_color_node_free (&line_color_node);

	shadow_state = gl_label_object_get_shadow_state (GL_LABEL_OBJECT (object));
	gl_label_object_get_shadow_offset (GL_LABEL_OBJECT (object), &shadow_x, &shadow_y);
	shadow_color_node = gl_label_object_get_shadow_color (GL_LABEL_OBJECT (object));
	if (shadow_color_node->field_flag)
	{
		shadow_color_node->color = GL_COLOR_SHADOW_MERGE_DEFAULT;
	}
	shadow_opacity = gl_label_object_get_shadow_opacity (GL_LABEL_OBJECT (object));
	shadow_line_color = gl_color_shadow (shadow_color_node->color, shadow_opacity, line_color);
	gl_color_node_free (&shadow_color_node);

	if (shadow_state)
	{
		cairo_move_to (pi->cr, shadow_x, shadow_y);
		cairo_line_to (pi->cr, shadow_x + w, shadow_y + h);
		cairo_set_source_rgba (pi->cr,
                                       GL_COLOR_F_RED (shadow_line_color),
                                       GL_COLOR_F_GREEN (shadow_line_color),
                                       GL_COLOR_F_BLUE (shadow_line_color),
                                       GL_COLOR_F_ALPHA (shadow_line_color));
		cairo_set_line_width (pi->cr, line_width);
		cairo_stroke (pi->cr);
	}

	cairo_move_to (pi->cr, 0.0, 0.0);
	cairo_line_to (pi->cr, w, h);
	cairo_set_source_rgba (pi->cr,
                               GL_COLOR_F_RED (line_color),
                               GL_COLOR_F_GREEN (line_color),
                               GL_COLOR_F_BLUE (line_color),
                               GL_COLOR_F_ALPHA (line_color));
	cairo_set_line_width (pi->cr, line_width);
	cairo_stroke (pi->cr);

	gl_debug (DEBUG_PRINT, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw ellipse object.                                            */
/*---------------------------------------------------------------------------*/
static void
draw_ellipse_object (PrintInfo      *pi,
		     glLabelEllipse *object,
                     glMergeRecord  *record)
{
	gdouble      x0, y0, rx, ry, w, h;
	gdouble      line_width;
	glColorNode *line_color_node;
	glColorNode *fill_color_node;
	guint        line_color;
	guint        fill_color;
	gboolean     shadow_state;
	gdouble      shadow_x, shadow_y;
	glColorNode *shadow_color_node;
	gdouble      shadow_opacity;
	guint        shadow_line_color;
	guint        shadow_fill_color;

	gl_debug (DEBUG_PRINT, "START");

	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_object_get_line_width (GL_LABEL_OBJECT(object));
	
	line_color_node = gl_label_object_get_line_color (GL_LABEL_OBJECT(object));
	fill_color_node = gl_label_object_get_fill_color (GL_LABEL_OBJECT(object));
	line_color = gl_color_node_expand (line_color_node, record);
	fill_color = gl_color_node_expand (fill_color_node, record);
	gl_color_node_free (&line_color_node);
	gl_color_node_free (&fill_color_node);

	rx = w / 2.0;
	ry = h / 2.0;
	x0 = rx;
	y0 = ry;

	shadow_state = gl_label_object_get_shadow_state (GL_LABEL_OBJECT (object));
	gl_label_object_get_shadow_offset (GL_LABEL_OBJECT (object), &shadow_x, &shadow_y);
	shadow_color_node = gl_label_object_get_shadow_color (GL_LABEL_OBJECT (object));
	if (shadow_color_node->field_flag)
	{
		shadow_color_node->color = GL_COLOR_SHADOW_MERGE_DEFAULT;
	}
	shadow_opacity = gl_label_object_get_shadow_opacity (GL_LABEL_OBJECT (object));
	shadow_line_color = gl_color_shadow (shadow_color_node->color, shadow_opacity, line_color);
	shadow_fill_color = gl_color_shadow (shadow_color_node->color, shadow_opacity, fill_color);
	gl_color_node_free (&shadow_color_node);
	
	if (shadow_state)
	{
		/* Draw fill shadow */
		create_ellipse_path (pi->cr, x0+shadow_x, y0+shadow_y, rx, ry);
		cairo_set_source_rgba (pi->cr,
                                       GL_COLOR_F_RED (shadow_fill_color),
                                       GL_COLOR_F_GREEN (shadow_fill_color),
                                       GL_COLOR_F_BLUE (shadow_fill_color),
                                       GL_COLOR_F_ALPHA (shadow_fill_color));
		cairo_fill_preserve (pi->cr);

		/* Draw outline shadow */
		cairo_set_source_rgba (pi->cr,
                                       GL_COLOR_F_RED (shadow_line_color),
                                       GL_COLOR_F_GREEN (shadow_line_color),
                                       GL_COLOR_F_BLUE (shadow_line_color),
                                       GL_COLOR_F_ALPHA (shadow_line_color));
		cairo_set_line_width (pi->cr, line_width);
		cairo_stroke (pi->cr);
	}

	/* Paint fill color */
	create_ellipse_path (pi->cr, x0, y0, rx, ry);
	cairo_set_source_rgba (pi->cr,
                               GL_COLOR_F_RED (fill_color),
                               GL_COLOR_F_GREEN (fill_color),
                               GL_COLOR_F_BLUE (fill_color),
                               GL_COLOR_F_ALPHA (fill_color));
	cairo_fill_preserve (pi->cr);

	/* Draw outline */
	cairo_set_source_rgba (pi->cr,
                               GL_COLOR_F_RED (line_color),
                               GL_COLOR_F_GREEN (line_color),
                               GL_COLOR_F_BLUE (line_color),
                               GL_COLOR_F_ALPHA (line_color));
	cairo_set_line_width (pi->cr, line_width);
	cairo_stroke (pi->cr);

	gl_debug (DEBUG_PRINT, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw image object.                                              */
/*---------------------------------------------------------------------------*/
static void
draw_image_object (PrintInfo     *pi,
		   glLabelImage  *object,
		   glMergeRecord *record)
{
	gdouble w, h;
	const GdkPixbuf *pixbuf;
	gint image_w, image_h;

	gl_debug (DEBUG_PRINT, "START");

	gl_label_object_get_size     (GL_LABEL_OBJECT(object), &w, &h);

	pixbuf = gl_label_image_get_pixbuf (object, record);
	image_w = gdk_pixbuf_get_width (pixbuf);
	image_h = gdk_pixbuf_get_height (pixbuf);

	cairo_save (pi->cr);
        cairo_rectangle (pi->cr, 0.0, 0.0, w, h);
        cairo_clip (pi->cr);
	cairo_scale (pi->cr, w/image_w, h/image_h);
        gdk_cairo_set_source_pixbuf (pi->cr, (GdkPixbuf *)pixbuf, 0, 0);
        cairo_paint (pi->cr);

	cairo_restore (pi->cr);

	gl_debug (DEBUG_PRINT, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw barcode object.                                            */
/*---------------------------------------------------------------------------*/
static void
draw_barcode_object (PrintInfo      *pi,
		     glLabelBarcode *object,
		     glMergeRecord  *record)
{
	glBarcode          *gbc;
	glBarcodeLine      *line;
	glBarcodeChar      *bchar;
	GList              *li;
	gdouble             y_offset;
        PangoLayout        *layout;
        PangoFontDescription *desc;
	gchar              *text, *cstring;
	glTextNode         *text_node;
	gchar              *id;
	gboolean            text_flag;
	gboolean            checksum_flag;
	guint               color;
	glColorNode        *color_node;
	guint               format_digits;
	gdouble             w, h;

	gl_debug (DEBUG_PRINT, "START");

	text_node = gl_label_barcode_get_data (object);
	gl_label_barcode_get_props (object,
				    &id, &text_flag, &checksum_flag, &format_digits);
					
	color_node = gl_label_object_get_line_color (GL_LABEL_OBJECT(object));
	color = gl_color_node_expand (color_node, record);
	gl_color_node_free (&color_node);
	
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);

	text = gl_text_node_expand (text_node, record);

	if (text == NULL || *text == '\0') {

		g_free(text);
		gl_text_node_free(&text_node);
		g_free(id);

		return;
	}

	gbc = gl_barcode_new (id, text_flag, checksum_flag, w, h, text);
	g_free (text);
	gl_text_node_free (&text_node);
	g_free (id);

        cairo_set_source_rgba (pi->cr,
                               GL_COLOR_F_RED (color),
                               GL_COLOR_F_GREEN (color),
                               GL_COLOR_F_BLUE (color),
                               GL_COLOR_F_ALPHA (color));

	if (gbc == NULL) {

                layout = pango_cairo_create_layout (pi->cr);

                desc = pango_font_description_new ();
                pango_font_description_set_family (desc, GL_BARCODE_FONT_FAMILY);
                pango_font_description_set_size   (desc, 12 * PANGO_SCALE * FONT_SCALE);
                pango_layout_set_font_description (layout, desc);
                pango_font_description_free       (desc);

                pango_layout_set_text (layout, _("Invalid barcode data"), -1);

                pango_cairo_show_layout (pi->cr, layout);

                g_object_unref (layout);

	} else {

		for (li = gbc->lines; li != NULL; li = li->next) {
			line = (glBarcodeLine *) li->data;

			cairo_move_to (pi->cr, line->x, line->y);
			cairo_line_to (pi->cr, line->x, line->y + line->length);
			cairo_set_line_width (pi->cr, line->width);
			cairo_stroke (pi->cr);
		}

		for (li = gbc->chars; li != NULL; li = li->next) {
			bchar = (glBarcodeChar *) li->data;

                        layout = pango_cairo_create_layout (pi->cr);

                        desc = pango_font_description_new ();
                        pango_font_description_set_family (desc, GL_BARCODE_FONT_FAMILY);
                        pango_font_description_set_size   (desc, bchar->fsize * PANGO_SCALE * FONT_SCALE);
                        pango_layout_set_font_description (layout, desc);
                        pango_font_description_free       (desc);

			cstring = g_strdup_printf ("%c", bchar->c);
                        pango_layout_set_text (layout, cstring, -1);
			g_free (cstring);

                        y_offset = 0.2 * bchar->fsize;

			cairo_move_to (pi->cr, bchar->x, bchar->y-y_offset);
                        pango_cairo_show_layout (pi->cr, layout);

                        g_object_unref (layout);

		}

		gl_barcode_free (&gbc);

	}

	gl_debug (DEBUG_PRINT, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw outline.                                                   */
/*---------------------------------------------------------------------------*/
static void
draw_outline (PrintInfo *pi,
	      glLabel   *label)
{
	const glTemplateLabelType *label_type;
	gdouble                    w, h, r;
	gdouble                    r1, r2;

	gl_debug (DEBUG_PRINT, "START");

	label_type = gl_template_get_first_label_type (pi->template);

        cairo_save (pi->cr);

	cairo_set_source_rgba (pi->cr, 0.0, 0.0, 0.0, 1.0);
	cairo_set_line_width  (pi->cr, 0.25);

	switch (label_type->shape) {

	case GL_TEMPLATE_SHAPE_RECT:
		gl_label_get_size (label, &w, &h);
		r = label_type->size.rect.r;
		if (r == 0.0) {
			/* simple rectangle */
			cairo_rectangle (pi->cr, 0.0, 0.0, w, h);
		} else {
			/* rectangle with rounded corners */
			create_rounded_rectangle_path (pi->cr, 0.0, 0.0,
						       w, h, r);
		}
		cairo_stroke (pi->cr);
		break;

	case GL_TEMPLATE_SHAPE_ROUND:
		/* Round style */
		r1 = label_type->size.round.r;
		create_ellipse_path (pi->cr, r1, r1, r1, r1);
		cairo_stroke (pi->cr);
		break;

	case GL_TEMPLATE_SHAPE_CD:
                r1 = label_type->size.cd.r1;
                r2 = label_type->size.cd.r2;
		if ((label_type->size.cd.h == 0) && (label_type->size.cd.w == 0)) {
			/* CD style, round label w/ concentric round hole */
                        create_cd_path (pi->cr, r1, r1, 2*r1, 2*r1, r1, r2);

			cairo_stroke (pi->cr);
		} else {
			/* Business Card CD style, clipped round label w/ hole */
			gl_label_get_size (label, &w, &h);
			create_cd_path (pi->cr, w/2, h/2, w, h, r1, r2);
			cairo_stroke (pi->cr);
		}
		break;

	default:
		g_message ("Unknown template label style");
		break;
	}

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
	const glTemplateLabelType *label_type;
	gdouble                    w, h, r;
	gdouble                    r1, r2;
	gdouble                    waste, x_waste, y_waste;

	gl_debug (DEBUG_PRINT, "START");

	label_type = gl_template_get_first_label_type (pi->template);

	switch (label_type->shape) {

	case GL_TEMPLATE_SHAPE_RECT:
		gl_label_get_size (label, &w, &h);
		r = label_type->size.rect.r;
		x_waste = label_type->size.rect.x_waste;
		y_waste = label_type->size.rect.y_waste;
		if (r == 0.0) {
			/* simple rectangle */
			cairo_rectangle (pi->cr,
                                         -x_waste, -y_waste,
                                         w+2*x_waste, h+2*y_waste);
		} else {
			/* rectangle with rounded corners */
			create_rounded_rectangle_path (pi->cr,
						       -x_waste, -y_waste,
						       w+2*x_waste, h+2*y_waste, r);
		}
		cairo_clip (pi->cr);
		break;

	case GL_TEMPLATE_SHAPE_ROUND:
		r1 = label_type->size.round.r;
		waste = label_type->size.round.waste;
		create_ellipse_path (pi->cr, r1, r1, r1+waste, r1+waste);
		cairo_clip (pi->cr);
		break;

	case GL_TEMPLATE_SHAPE_CD:
		waste = label_type->size.cd.waste;
                r1    = label_type->size.cd.r1;
                r2    = label_type->size.cd.r2;
		if ((label_type->size.cd.h == 0) && (label_type->size.cd.w == 0)) {
			/* CD style, round label w/ concentric round hole */
                        create_cd_path (pi->cr,
                                        r1, r1,
                                        2*(r1+waste), 2*(r1+waste),
                                        r1+waste,
                                        r2-waste);
		} else {
			/* Business Card CD style, clipped round label w/ hole */
			gl_label_get_size (label, &w, &h);
			create_cd_path (pi->cr,
                                        w/2, h/2,
                                        w+2*waste, h+2*waste,
                                        r1+waste,
                                        r2-waste);
		}
		cairo_set_fill_rule (pi->cr, CAIRO_FILL_RULE_EVEN_ODD);
		cairo_clip (pi->cr);
		break;

	default:
		g_message ("Unknown template label style");
		break;
	}

	gl_debug (DEBUG_PRINT, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Path creation utilities.                                        */
/*---------------------------------------------------------------------------*/
static void
create_rounded_rectangle_path (cairo_t           *cr,
			       gdouble            x0,
			       gdouble            y0,
			       gdouble            w,
			       gdouble            h,
			       gdouble            r)
{
	gdouble x, y;
	gint i_theta;

	gl_debug (DEBUG_PRINT, "START");

	cairo_new_path (cr);

        cairo_arc_negative (cr, x0+r,   y0+r,   r, 3*M_PI/2, M_PI);
        cairo_arc_negative (cr, x0+r,   y0+h-r, r, M_PI,     M_PI/2);
        cairo_arc_negative (cr, x0+w-r, y0+h-r, r, M_PI/2,   0.);
        cairo_arc_negative (cr, x0+w-r, y0+r,   r, 2*M_PI,   3*M_PI/2);
	cairo_close_path (cr);

	gl_debug (DEBUG_PRINT, "END");
}

static void
create_ellipse_path (cairo_t           *cr,
		     gdouble            x0,
		     gdouble            y0,
		     gdouble            rx,
		     gdouble            ry)
{
	gdouble x, y;
	gint i_theta;

	gl_debug (DEBUG_PRINT, "START");

	cairo_new_path (cr);
	cairo_move_to (cr, x0 + rx, y0);
	for (i_theta = ARC_FINE; i_theta <= 360; i_theta += ARC_FINE) {
		x = x0 + rx * cos (i_theta * G_PI / 180.0);
		y = y0 + ry * sin (i_theta * G_PI / 180.0);
		cairo_line_to (cr, x, y);
	}
	cairo_close_path (cr);

	gl_debug (DEBUG_PRINT, "END");
}

static void
create_cd_path (cairo_t           *cr,
                gdouble            x0,
                gdouble            y0,
                gdouble            w,
                gdouble            h,
                gdouble            r1,
                gdouble            r2)
{
	gdouble theta1, theta2;

	gl_debug (DEBUG_PRINT, "START");

        /*
         * Outer radius.  (may be clipped)
         */
	theta1 = acos (w / (2.0*r1));
	theta2 = asin (h / (2.0*r1));

	cairo_new_path (cr);
        cairo_arc (cr, x0, y0, r1, theta1,        theta2);
        cairo_arc (cr, x0, y0, r1, M_PI-theta2,   M_PI-theta1);
        cairo_arc (cr, x0, y0, r1, M_PI+theta1,   M_PI+theta2);
        cairo_arc (cr, x0, y0, r1, 2*M_PI-theta2, 2*M_PI-theta1);
	cairo_close_path (cr);

        /*
         * Inner radius.  (hole)
         */
        cairo_new_sub_path (cr);
        cairo_arc (cr, x0, y0, r2, 0.0, 2*M_PI);
	cairo_close_path (cr);

	gl_debug (DEBUG_PRINT, "END");
}
