/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  print.c:  Print module
 *
 *  Copyright (C) 2001-2006  Jim Evins <evins@snaught.com>.
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
#define ARC_COURSE 5  /* Resolution in degrees of small arcs */

#define TICK_OFFSET  2.25
#define TICK_LENGTH 18.0

/*=========================================================================*/
/* Private types.                                                          */
/*=========================================================================*/

typedef struct _PrintInfo {
	/* gnome print context */
	GnomePrintContext *pc;

	/* gnome print configuration */
	GnomePrintConfig *config;

	/* gLabels Template */
	glTemplate *template;
	gboolean label_rotate_flag;

	/* page size */
	gdouble page_width;
	gdouble page_height;

	/* page counter */
	gint sheet;
} PrintInfo;


/*=========================================================================*/
/* Private function prototypes.                                            */
/*=========================================================================*/
static PrintInfo *print_info_new              (GnomePrintJob    *job,
					       glLabel          *label);

static void       print_info_free             (PrintInfo       **pi);


static void       print_page_begin            (PrintInfo        *pi);

static void       print_page_end              (PrintInfo        *pi);

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


static void       create_rectangle_path         (GnomePrintContext *pc,
						 gdouble            x0,
						 gdouble            y0,
						 gdouble            w,
						 gdouble            h);

static void       create_ellipse_path           (GnomePrintContext *pc,
						 gdouble            x0,
						 gdouble            y0,
						 gdouble            rx,
						 gdouble            ry);

static void       create_rounded_rectangle_path (GnomePrintContext *pc,
						 gdouble            x0,
						 gdouble            y0,
						 gdouble            w,
						 gdouble            h,
						 gdouble            r);

static void       create_clipped_circle_path    (GnomePrintContext *pc,
						 gdouble            x0,
						 gdouble            y0,
						 gdouble            w,
						 gdouble            h,
						 gdouble            r);

#ifndef NO_ALPHA_HACK
static guchar *   get_pixels_as_rgb             (const GdkPixbuf   *pixbuf);
#endif


/*****************************************************************************/
/* Simple (no merge data) print command.                                     */
/*****************************************************************************/
void
gl_print_simple (GnomePrintJob    *job,
		 glLabel          *label,
		 gint              n_sheets,
		 gint              first,
		 gint              last,
		 glPrintFlags     *flags)
{
	PrintInfo                 *pi;
	const glTemplateLabelType *label_type;
	gint                       i_sheet, i_label;
	glTemplateOrigin          *origins;

	gl_debug (DEBUG_PRINT, "START");

	pi         = print_info_new (job, label);
	label_type = gl_template_get_first_label_type (pi->template);

	origins = gl_template_get_origins (label_type);

	for (i_sheet = 0; i_sheet < n_sheets; i_sheet++) {

		print_page_begin (pi);
		if (flags->crop_marks) {
			print_crop_marks (pi);
		}

		for (i_label = first - 1; i_label < last; i_label++) {

			print_label (pi, label,
				     origins[i_label].x, origins[i_label].y,
				     NULL, flags->outline, flags->reverse);

		}

		print_page_end (pi);
	}

	g_free (origins);

	print_info_free (&pi);

	gl_debug (DEBUG_PRINT, "END");
}

/*****************************************************************************/
/* Merge print command (collated copies)                                     */
/*****************************************************************************/
void
gl_print_merge_collated (GnomePrintJob    *job,
			 glLabel          *label,
			 gint              n_copies,
			 gint              first,
			 glPrintFlags     *flags)
{
	glMerge                   *merge;
	const GList               *record_list;
	PrintInfo                 *pi;
	const glTemplateLabelType *label_type;
	gint                       i_sheet, i_label, n_labels_per_page, i_copy;
	glMergeRecord             *record;
	GList                     *p;
	glTemplateOrigin          *origins;

	gl_debug (DEBUG_PRINT, "START");

	merge = gl_label_get_merge (label);
	record_list = gl_merge_get_record_list (merge);

	pi = print_info_new (job, label);
	label_type = gl_template_get_first_label_type (pi->template);

	n_labels_per_page = gl_template_get_n_labels (label_type);
	origins = gl_template_get_origins (label_type);

	i_sheet = 0;
	i_label = first - 1;

	for ( p=(GList *)record_list; p!=NULL; p=p->next ) {
		record = (glMergeRecord *)p->data;
			
		if ( record->select_flag ) {
			for (i_copy = 0; i_copy < n_copies; i_copy++) {

				if ((i_label == 0) || (i_sheet == 0)) {
					i_sheet++;
					print_page_begin (pi);
					if (flags->crop_marks) {
						print_crop_marks (pi);
					}
				}

				print_label (pi, label,
					     origins[i_label].x,
					     origins[i_label].y,
					     record,
					     flags->outline, flags->reverse);

				i_label = (i_label + 1) % n_labels_per_page;
				if (i_label == 0) {
					print_page_end (pi);
				}
			}
		}
	}

	if (i_label != 0) {
		print_page_end (pi);
	}

	g_free (origins);

	print_info_free (&pi);

	gl_debug (DEBUG_PRINT, "END");
}

/*****************************************************************************/
/* Merge print command (uncollated copies)                                   */
/*****************************************************************************/
void
gl_print_merge_uncollated (GnomePrintJob    *job,
			   glLabel          *label,
			   gint              n_copies,
			   gint              first,
			   glPrintFlags     *flags)
{
	glMerge                   *merge;
	const GList               *record_list;
	PrintInfo                 *pi;
	const glTemplateLabelType *label_type;
	gint                       i_sheet, i_label, n_labels_per_page, i_copy;
	glMergeRecord             *record;
	GList                     *p;
	glTemplateOrigin          *origins;

	gl_debug (DEBUG_PRINT, "START");

	merge = gl_label_get_merge (label);
	record_list = gl_merge_get_record_list (merge);

	pi = print_info_new (job, label);
	label_type = gl_template_get_first_label_type (pi->template);

	n_labels_per_page = gl_template_get_n_labels (label_type);
	origins = gl_template_get_origins (label_type);

	i_sheet = 0;
	i_label = first - 1;

	for (i_copy = 0; i_copy < n_copies; i_copy++) {

		for ( p=(GList *)record_list; p!=NULL; p=p->next ) {
			record = (glMergeRecord *)p->data;
			
			if ( record->select_flag ) {


				if ((i_label == 0) || (i_sheet == 0)) {
					i_sheet++;
					print_page_begin (pi);
					if (flags->crop_marks) {
						print_crop_marks (pi);
					}
				}

				print_label (pi, label,
					     origins[i_label].x,
					     origins[i_label].y,
					     record,
					     flags->outline, flags->reverse);

				i_label = (i_label + 1) % n_labels_per_page;
				if (i_label == 0) {
					print_page_end (pi);
				}
			}
		}

	}
	if (i_label != 0) {
		print_page_end (pi);
	}

	g_free (origins);

	print_info_free (&pi);

	gl_debug (DEBUG_PRINT, "END");
}

/*****************************************************************************/
/* Batch print.  Call appropriate function above.                            */
/*****************************************************************************/
void
gl_print_batch (GnomePrintJob    *job,
		glLabel          *label,
		gint              n_sheets,
		gint              n_copies,
		gint              first,
		glPrintFlags     *flags)
{
	glMerge                   *merge;
	glTemplate                *template;
	const glTemplateLabelType *label_type;
	gint                       n_per_page;
	
	gl_debug (DEBUG_PRINT, "START");

	merge = gl_label_get_merge (label);
	template = gl_label_get_template (label);
	label_type = gl_template_get_first_label_type (template);

	if ( merge == NULL ) {
		n_per_page = gl_template_get_n_labels(label_type);

		gl_print_simple (job, label, n_sheets, first, n_per_page, flags);
	} else {
		gl_print_merge_collated (job, label, n_copies, first, flags);
	}
	gl_template_free (template);

	gl_debug (DEBUG_PRINT, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  new print info structure                                        */
/*---------------------------------------------------------------------------*/
static PrintInfo *
print_info_new (GnomePrintJob    *job,
		glLabel          *label)
{
	PrintInfo            *pi = g_new0 (PrintInfo, 1);
	glTemplate           *template;

	gl_debug (DEBUG_PRINT, "START");

	g_return_val_if_fail (job && GNOME_IS_PRINT_JOB (job), NULL);
	g_return_val_if_fail (label && GL_IS_LABEL (label), NULL);

	template = gl_label_get_template (label);

	g_return_val_if_fail (template, NULL);
	g_return_val_if_fail (template->page_size, NULL);
	g_return_val_if_fail (template->page_width > 0, NULL);
	g_return_val_if_fail (template->page_height > 0, NULL);

	pi->pc = gnome_print_job_get_context (job);
	pi->config = gnome_print_job_get_config (job);

	gl_debug (DEBUG_PRINT,
		  "setting page size = \"%s\"", template->page_size);

	gnome_print_config_set_length (pi->config,
				       (guchar *)GNOME_PRINT_KEY_PAPER_WIDTH,
				       template->page_width,
				       GNOME_PRINT_PS_UNIT);
	gnome_print_config_set_length (pi->config,
				       (guchar *)GNOME_PRINT_KEY_PAPER_HEIGHT,
				       template->page_height,
				       GNOME_PRINT_PS_UNIT);

	pi->page_width  = template->page_width;
	pi->page_height = template->page_height;

	pi->template = template;
	pi->label_rotate_flag = gl_label_get_rotate_flag (label);

	pi->sheet = 0;

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

	gnome_print_context_close ((*pi)->pc);

	g_free (*pi);
	*pi = NULL;

	gl_debug (DEBUG_PRINT, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Begin a new page.                                               */
/*---------------------------------------------------------------------------*/
static void
print_page_begin (PrintInfo *pi)
{
	gchar *str;

	gl_debug (DEBUG_PRINT, "START");

	pi->sheet++;

	str = g_strdup_printf ("sheet%02d", pi->sheet);
	gnome_print_beginpage (pi->pc, (guchar *)str);
	g_free (str);

	/* Translate and scale, so that our origin is at the upper left. */
	gnome_print_translate (pi->pc, 0.0, pi->page_height);
	gnome_print_scale (pi->pc, 1.0, -1.0);

	gl_debug (DEBUG_PRINT, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  End a page.                                                     */
/*---------------------------------------------------------------------------*/
static void
print_page_end (PrintInfo *pi)
{
	gl_debug (DEBUG_PRINT, "START");

	gnome_print_showpage (pi->pc);

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

	gnome_print_setrgbcolor (pi->pc, 0.0, 0.0, 0.0);
	gnome_print_setopacity (pi->pc, 1.0);
	gnome_print_setlinewidth (pi->pc, 0.25);

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

			gnome_print_moveto (pi->pc, x1, y1);
			gnome_print_lineto (pi->pc, x1, y2);
			gnome_print_stroke (pi->pc);

			gnome_print_moveto (pi->pc, x2, y1);
			gnome_print_lineto (pi->pc, x2, y2);
			gnome_print_stroke (pi->pc);

			gnome_print_moveto (pi->pc, x1, y3);
			gnome_print_lineto (pi->pc, x1, y4);
			gnome_print_stroke (pi->pc);

			gnome_print_moveto (pi->pc, x2, y3);
			gnome_print_lineto (pi->pc, x2, y4);
			gnome_print_stroke (pi->pc);

		}

		for (iy=0; iy < ny; iy++) {

			y1 = ymin + iy*dy;
			y2 = y1 + h;

			x1 = MAX((xmin - TICK_OFFSET), 0.0);
			x2 = MAX((x1 - TICK_LENGTH), 0.0);

			x3 = MIN((xmax + TICK_OFFSET), page_w);
			x4 = MIN((x3 + TICK_LENGTH), page_w);

			gnome_print_moveto (pi->pc, x1, y1);
			gnome_print_lineto (pi->pc, x2, y1);
			gnome_print_stroke (pi->pc);

			gnome_print_moveto (pi->pc, x1, y2);
			gnome_print_lineto (pi->pc, x2, y2);
			gnome_print_stroke (pi->pc);

			gnome_print_moveto (pi->pc, x3, y1);
			gnome_print_lineto (pi->pc, x4, y1);
			gnome_print_stroke (pi->pc);

			gnome_print_moveto (pi->pc, x3, y2);
			gnome_print_lineto (pi->pc, x4, y2);
			gnome_print_stroke (pi->pc);

		}

	}

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

	gnome_print_gsave (pi->pc);

	/* Transform coordinate system to be relative to upper corner */
	/* of the current label */
	gnome_print_translate (pi->pc, x, y);
	if (gl_label_get_rotate_flag (label)) {
		gl_debug (DEBUG_PRINT, "Rotate flag set");
		gnome_print_rotate (pi->pc, -90.0);
		gnome_print_translate (pi->pc, -width, 0.0);
	}
	if ( reverse_flag ) {
		gnome_print_translate (pi->pc, width, 0.0);
		gnome_print_scale (pi->pc, -1.0, 1.0);
	}

	clip_to_outline (pi, label);
	draw_label (pi, label, record);
	if (outline_flag) {
		draw_outline (pi, label);
	}
	clip_punchouts (pi, label);

	gnome_print_grestore (pi->pc);

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
	gdouble affine[6];

	gl_debug (DEBUG_PRINT, "START");

	gl_label_object_get_position (object, &x0, &y0);
	gl_label_object_get_affine (object, affine);

	gnome_print_gsave (pi->pc);

	gnome_print_translate (pi->pc, x0, y0);
	gnome_print_concat (pi->pc, affine);

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

	gnome_print_grestore (pi->pc);

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
	GnomeFont       *font;
	gchar          **line;
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
	GnomeGlyphList  *glyphlist;
	ArtDRect         bbox;
	gdouble          affine[6];
	gdouble          text_line_spacing;
	gboolean         shadow_state;
	gdouble          shadow_x, shadow_y;
	glColorNode     *shadow_color_node;
	gdouble          shadow_opacity;
	guint            shadow_color;


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
	line = g_strsplit (text, "\n", -1);
	g_free (text);

	art_affine_identity (affine);

	if (record && auto_shrink) {
		/* auto shrink text size to keep within text box limits. */
		for (i = 0; line[i] != NULL; i++) {

			font = gnome_font_find_closest_from_weight_slant (
				(guchar *)font_family,
				font_weight,
				font_italic_flag,
				font_size);
			glyphlist = gnome_glyphlist_from_text_dumb (
				font,
				color,
				0.0, 0.0,
				(guchar *)line[i]);
			gnome_glyphlist_bbox (glyphlist, affine, 0, &bbox);
			w = bbox.x1;
			gnome_glyphlist_unref (glyphlist);

			/* If width is too large, iteratively shrink font_size until this
			   line fits the width, or until the font size is ridiculously
			   small. */
			while ( (w > object_w) && (font_size >= 1.0) ) {

				font_size -= 0.5;

				font = gnome_font_find_closest_from_weight_slant (
					(guchar *)font_family,
					font_weight,
					font_italic_flag,
					font_size);
				glyphlist = gnome_glyphlist_from_text_dumb (
					font,
					color,
					0.0, 0.0,
					(guchar *)line[i]);
				gnome_glyphlist_bbox (glyphlist, affine, 0, &bbox);
				w = bbox.x1;
				gnome_glyphlist_unref (glyphlist);
			}
		}

	}

	font = gnome_font_find_closest_from_weight_slant (
                                       (guchar *)font_family,
				       font_weight,
				       font_italic_flag,
				       font_size);
	gnome_print_setfont (pi->pc, font);

	gnome_print_setrgbcolor (pi->pc,
				 GL_COLOR_F_RED (shadow_color),
				 GL_COLOR_F_GREEN (shadow_color),
				 GL_COLOR_F_BLUE (shadow_color));
	gnome_print_setopacity (pi->pc, GL_COLOR_F_ALPHA (shadow_color));

	for (i = 0; line[i] != NULL; i++) {

		glyphlist = gnome_glyphlist_from_text_dumb (font, color,
							    0.0, 0.0,
							    (guchar *)line[i]);

		gnome_glyphlist_bbox (glyphlist, affine, 0, &bbox);
		w = bbox.x1;
		gnome_glyphlist_unref (glyphlist);

		switch (just) {
		case GTK_JUSTIFY_LEFT:
			x_offset = GL_LABEL_TEXT_MARGIN;
			break;
		case GTK_JUSTIFY_CENTER:
			x_offset = (object_w - GL_LABEL_TEXT_MARGIN - w) / 2.0;
			break;
		case GTK_JUSTIFY_RIGHT:
			x_offset = object_w - GL_LABEL_TEXT_MARGIN - w;
			break;
		default:
			x_offset = 0.0;
			break;	/* shouldn't happen */
		}
                x_offset += shadow_x;

		/* Work out the y position to the BOTTOM of the first line */
		y_offset = GL_LABEL_TEXT_MARGIN +
			   + gnome_font_get_descender (font)
	       		   + (i + 1) * font_size * text_line_spacing
                           + shadow_y;

		/* Remove any text line spacing from the first row. */
		y_offset -= font_size * (text_line_spacing - 1);


		gnome_print_moveto (pi->pc, x_offset, y_offset);

		gnome_print_gsave (pi->pc);
		gnome_print_scale (pi->pc, 1.0, -1.0);
		gnome_print_show (pi->pc, (guchar *)line[i]);
		gnome_print_grestore (pi->pc);
	}

	gnome_print_setrgbcolor (pi->pc,
				 GL_COLOR_F_RED (color),
				 GL_COLOR_F_GREEN (color),
				 GL_COLOR_F_BLUE (color));
	gnome_print_setopacity (pi->pc, GL_COLOR_F_ALPHA (color));

	for (i = 0; line[i] != NULL; i++) {

		glyphlist = gnome_glyphlist_from_text_dumb (font, color,
							    0.0, 0.0,
							    (guchar *)line[i]);

		gnome_glyphlist_bbox (glyphlist, affine, 0, &bbox);
		w = bbox.x1;
		gnome_glyphlist_unref (glyphlist);

		switch (just) {
		case GTK_JUSTIFY_LEFT:
			x_offset = GL_LABEL_TEXT_MARGIN;
			break;
		case GTK_JUSTIFY_CENTER:
			x_offset = (object_w - GL_LABEL_TEXT_MARGIN - w) / 2.0;
			break;
		case GTK_JUSTIFY_RIGHT:
			x_offset = object_w - GL_LABEL_TEXT_MARGIN - w;
			break;
		default:
			x_offset = 0.0;
			break;	/* shouldn't happen */
		}

		/* Work out the y position to the BOTTOM of the first line */
		y_offset = GL_LABEL_TEXT_MARGIN +
			   + gnome_font_get_descender (font)
	       		   + (i + 1) * font_size * text_line_spacing;

		/* Remove any text line spacing from the first row. */
		y_offset -= font_size * (text_line_spacing - 1);


		gnome_print_moveto (pi->pc, x_offset, y_offset);

		gnome_print_gsave (pi->pc);
		gnome_print_scale (pi->pc, 1.0, -1.0);
		gnome_print_show (pi->pc, (guchar *)line[i]);
		gnome_print_grestore (pi->pc);
	}

	g_strfreev (line);

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
		create_rectangle_path (pi->pc, shadow_x, shadow_y, w, h);
		gnome_print_setrgbcolor (pi->pc,
					 GL_COLOR_F_RED (shadow_fill_color),
					 GL_COLOR_F_GREEN (shadow_fill_color),
					 GL_COLOR_F_BLUE (shadow_fill_color));
		gnome_print_setopacity (pi->pc, GL_COLOR_F_ALPHA (shadow_fill_color));
		gnome_print_fill (pi->pc);

		/* Draw outline shadow */
		create_rectangle_path (pi->pc, shadow_x, shadow_y, w, h);
		gnome_print_setrgbcolor (pi->pc,
					 GL_COLOR_F_RED (shadow_line_color),
					 GL_COLOR_F_GREEN (shadow_line_color),
					 GL_COLOR_F_BLUE (shadow_line_color));
		gnome_print_setopacity (pi->pc, GL_COLOR_F_ALPHA (shadow_line_color));
		gnome_print_setlinewidth (pi->pc, line_width);
		gnome_print_stroke (pi->pc);
	}

	/* Paint fill color */
	create_rectangle_path (pi->pc, 0.0, 0.0, w, h);
	gnome_print_setrgbcolor (pi->pc,
				 GL_COLOR_F_RED (fill_color),
				 GL_COLOR_F_GREEN (fill_color),
				 GL_COLOR_F_BLUE (fill_color));
	gnome_print_setopacity (pi->pc, GL_COLOR_F_ALPHA (fill_color));
	gnome_print_fill (pi->pc);

	/* Draw outline */
	create_rectangle_path (pi->pc, 0.0, 0.0, w, h);
	gnome_print_setrgbcolor (pi->pc,
				 GL_COLOR_F_RED (line_color),
				 GL_COLOR_F_GREEN (line_color),
				 GL_COLOR_F_BLUE (line_color));
	gnome_print_setopacity (pi->pc, GL_COLOR_F_ALPHA (line_color));
	gnome_print_setlinewidth (pi->pc, line_width);
	gnome_print_stroke (pi->pc);

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
		gnome_print_moveto (pi->pc, shadow_x, shadow_y);
		gnome_print_lineto (pi->pc, shadow_x + w, shadow_y + h);
		gnome_print_setrgbcolor (pi->pc,
					 GL_COLOR_F_RED (shadow_line_color),
					 GL_COLOR_F_GREEN (shadow_line_color),
					 GL_COLOR_F_BLUE (shadow_line_color));
		gnome_print_setopacity (pi->pc, GL_COLOR_F_ALPHA (shadow_line_color));
		gnome_print_setlinewidth (pi->pc, line_width);
		gnome_print_stroke (pi->pc);
	}

	gnome_print_moveto (pi->pc, 0.0, 0.0);
	gnome_print_lineto (pi->pc, w, h);
	gnome_print_setrgbcolor (pi->pc,
				 GL_COLOR_F_RED (line_color),
				 GL_COLOR_F_GREEN (line_color),
				 GL_COLOR_F_BLUE (line_color));
	gnome_print_setopacity (pi->pc, GL_COLOR_F_ALPHA (line_color));
	gnome_print_setlinewidth (pi->pc, line_width);
	gnome_print_stroke (pi->pc);

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
		create_ellipse_path (pi->pc, x0+shadow_x, y0+shadow_y, rx, ry);
		gnome_print_setrgbcolor (pi->pc,
					 GL_COLOR_F_RED (shadow_fill_color),
					 GL_COLOR_F_GREEN (shadow_fill_color),
					 GL_COLOR_F_BLUE (shadow_fill_color));
		gnome_print_setopacity (pi->pc, GL_COLOR_F_ALPHA (shadow_fill_color));
		gnome_print_fill (pi->pc);

		/* Draw outline shadow */
		create_ellipse_path (pi->pc, x0+shadow_x, y0+shadow_y, rx, ry);
		gnome_print_setrgbcolor (pi->pc,
					 GL_COLOR_F_RED (shadow_line_color),
					 GL_COLOR_F_GREEN (shadow_line_color),
					 GL_COLOR_F_BLUE (shadow_line_color));
		gnome_print_setopacity (pi->pc, GL_COLOR_F_ALPHA (shadow_line_color));
		gnome_print_setlinewidth (pi->pc, line_width);
		gnome_print_stroke (pi->pc);
	}

	/* Paint fill color */
	create_ellipse_path (pi->pc, x0, y0, rx, ry);
	gnome_print_setrgbcolor (pi->pc,
				 GL_COLOR_F_RED (fill_color),
				 GL_COLOR_F_GREEN (fill_color),
				 GL_COLOR_F_BLUE (fill_color));
	gnome_print_setopacity (pi->pc, GL_COLOR_F_ALPHA (fill_color));
	gnome_print_fill (pi->pc);

	/* Draw outline */
	create_ellipse_path (pi->pc, x0, y0, rx, ry);
	gnome_print_setrgbcolor (pi->pc,
				 GL_COLOR_F_RED (line_color),
				 GL_COLOR_F_GREEN (line_color),
				 GL_COLOR_F_BLUE (line_color));
	gnome_print_setopacity (pi->pc, GL_COLOR_F_ALPHA (line_color));
	gnome_print_setlinewidth (pi->pc, line_width);
	gnome_print_stroke (pi->pc);

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
	guchar *image_data;
	gint image_w, image_h, image_stride;
	gboolean image_alpha_flag;
	gint ret;

	gl_debug (DEBUG_PRINT, "START");

	gl_label_object_get_size     (GL_LABEL_OBJECT(object), &w, &h);

	pixbuf = gl_label_image_get_pixbuf (object, record);
	image_data = gdk_pixbuf_get_pixels (pixbuf);
	image_w = gdk_pixbuf_get_width (pixbuf);
	image_h = gdk_pixbuf_get_height (pixbuf);
	image_stride = gdk_pixbuf_get_rowstride(pixbuf);
	image_alpha_flag = gdk_pixbuf_get_has_alpha(pixbuf);

	gnome_print_gsave (pi->pc);
	gnome_print_translate (pi->pc, 0.0, h);
	gnome_print_scale (pi->pc, w, -h);
	if (image_alpha_flag) {
#ifndef NO_ALPHA_HACK
		guchar *image_data2;

		image_data2 = get_pixels_as_rgb (pixbuf);
	        ret = gnome_print_rgbimage (pi->pc, image_data2,
					    image_w, image_h, image_stride);
		g_free (image_data2);
#else
	        ret = gnome_print_rgbaimage (pi->pc, image_data,
					    image_w, image_h, image_stride);
#endif
	} else {
	        ret = gnome_print_rgbimage (pi->pc, image_data,
					    image_w, image_h, image_stride);
	}
	gnome_print_grestore (pi->pc);

	gl_debug (DEBUG_PRINT, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw box object.                                                */
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
	GnomeFont          *font;
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
	gbc = gl_barcode_new (id, text_flag, checksum_flag, w, h, text);
	g_free (text);
	gl_text_node_free (&text_node);
	g_free (id);

	if (gbc == NULL) {

		font = gnome_font_find_closest_from_weight_slant (
			(guchar *)GL_BARCODE_FONT_FAMILY,
			GL_BARCODE_FONT_WEIGHT,
			FALSE, 12.0);
		gnome_print_setfont (pi->pc, font);

		gnome_print_setrgbcolor (pi->pc,
					 GL_COLOR_F_RED (color),
					 GL_COLOR_F_GREEN (color),
					 GL_COLOR_F_BLUE (color));
		gnome_print_setopacity (pi->pc,
					GL_COLOR_F_ALPHA (color));

		y_offset = 12.0 - fabs (gnome_font_get_descender (font));
		gnome_print_moveto (pi->pc, 0.0, y_offset);

		gnome_print_gsave (pi->pc);
		gnome_print_scale (pi->pc, 1.0, -1.0);
		gnome_print_show (pi->pc, (guchar *)_("Invalid barcode data"));
		gnome_print_grestore (pi->pc);

	} else {

		for (li = gbc->lines; li != NULL; li = li->next) {
			line = (glBarcodeLine *) li->data;

			gnome_print_moveto (pi->pc, line->x, line->y);
			gnome_print_lineto (pi->pc, line->x, line->y + line->length);
			gnome_print_setrgbcolor (pi->pc,
						 GL_COLOR_F_RED (color),
						 GL_COLOR_F_GREEN (color),
						 GL_COLOR_F_BLUE (color));
			gnome_print_setopacity (pi->pc,
						GL_COLOR_F_ALPHA (color));
			gnome_print_setlinewidth (pi->pc, line->width);
			gnome_print_stroke (pi->pc);
		}

		for (li = gbc->chars; li != NULL; li = li->next) {
			bchar = (glBarcodeChar *) li->data;

			font = gnome_font_find_closest_from_weight_slant (
				(guchar *)GL_BARCODE_FONT_FAMILY,
				GL_BARCODE_FONT_WEIGHT,
				FALSE, bchar->fsize);
			gnome_print_setfont (pi->pc, font);

			gnome_print_setrgbcolor (pi->pc,
						 GL_COLOR_F_RED (color),
						 GL_COLOR_F_GREEN (color),
						 GL_COLOR_F_BLUE (color));
			gnome_print_setopacity (pi->pc,
						GL_COLOR_F_ALPHA (color));

			y_offset =
			    bchar->fsize - fabs (gnome_font_get_descender (font));

			gnome_print_moveto (pi->pc, bchar->x, bchar->y+y_offset);

			cstring = g_strdup_printf ("%c", bchar->c);
			gnome_print_gsave (pi->pc);
			gnome_print_scale (pi->pc, 1.0, -1.0);
			gnome_print_show (pi->pc, (guchar *)cstring);
			gnome_print_grestore (pi->pc);
			g_free (cstring);

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

	gnome_print_setrgbcolor (pi->pc, 0.0, 0.0, 0.0);
	gnome_print_setopacity (pi->pc, 1.0);
	gnome_print_setlinewidth (pi->pc, 0.25);

	switch (label_type->shape) {

	case GL_TEMPLATE_SHAPE_RECT:
		gl_label_get_size (label, &w, &h);
		r = label_type->size.rect.r;
		if (r == 0.0) {
			/* simple rectangle */
			create_rectangle_path (pi->pc, 0.0, 0.0, w, h);
		} else {
			/* rectangle with rounded corners */
			create_rounded_rectangle_path (pi->pc, 0.0, 0.0,
						       w, h, r);
		}
		gnome_print_stroke (pi->pc);
		break;

	case GL_TEMPLATE_SHAPE_ROUND:
		/* Round style */
		r1 = label_type->size.round.r;
		create_ellipse_path (pi->pc, r1, r1, r1, r1);
		gnome_print_stroke (pi->pc);
		break;

	case GL_TEMPLATE_SHAPE_CD:
		if ((label_type->size.cd.h == 0) && (label_type->size.cd.w == 0)) {
			/* CD style, round label w/ concentric round hole */
			r1 = label_type->size.cd.r1;
			r2 = label_type->size.cd.r2;
			create_ellipse_path (pi->pc, r1, r1, r1, r1);
			gnome_print_stroke (pi->pc);
			create_ellipse_path (pi->pc, r1, r1, r2, r2);
			gnome_print_stroke (pi->pc);
		} else {
			/* Business Card CD style, clipped round label w/ hole */
			gl_label_get_size (label, &w, &h);
			r1 = label_type->size.cd.r1;
			r2 = label_type->size.cd.r2;
			create_clipped_circle_path (pi->pc, w/2, h/2, w, h, r1);
			gnome_print_stroke (pi->pc);
			create_ellipse_path (pi->pc, w/2, h/2, r2, r2);
			gnome_print_stroke (pi->pc);
		}
		break;

	default:
		g_message ("Unknown template label style");
		break;
	}

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
	gdouble                    r1;
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
			create_rectangle_path (pi->pc,
					       -x_waste, -y_waste,
					       w+2*x_waste, h+2*y_waste);
		} else {
			/* rectangle with rounded corners */
			create_rounded_rectangle_path (pi->pc,
						       -x_waste, -y_waste,
						       w+2*x_waste, h+2*y_waste, r);
		}
		gnome_print_clip (pi->pc);
		break;

	case GL_TEMPLATE_SHAPE_ROUND:
		r1 = label_type->size.round.r;
		waste = label_type->size.round.waste;
		create_ellipse_path (pi->pc, r1, r1, r1+waste, r1+waste);
		gnome_print_clip (pi->pc);
		break;

	case GL_TEMPLATE_SHAPE_CD:
		waste = label_type->size.cd.waste;
		if ((label_type->size.cd.h == 0) && (label_type->size.cd.w == 0)) {
			/* CD style, round label w/ concentric round hole */
			r1 = label_type->size.cd.r1;
			create_ellipse_path (pi->pc, r1, r1, r1+waste, r1+waste);
		} else {
			/* Business Card CD style, clipped round label w/ hole */
			gl_label_get_size (label, &w, &h);
			r1 = label_type->size.cd.r1;
			create_clipped_circle_path (pi->pc,
						    w/2, h/2,
						    w+2*waste, h+2*waste,
						    r1+waste);
		}
		gnome_print_clip (pi->pc);
		break;

	default:
		g_message ("Unknown template label style");
		break;
	}

	gl_debug (DEBUG_PRINT, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Clip punchouts.  (Save some ink by not printing in CD holes)    */
/*                                                                           */
/* Ideally this would be done in clip_to_outline, but I am not sure how to   */
/* invert the region for gnome_print_clip, so instead, I will just draw      */
/* a white circle on top of everything else.                                 */
/*---------------------------------------------------------------------------*/
static void
clip_punchouts (PrintInfo *pi,
		glLabel   *label)
{
	const glTemplateLabelType *label_type;
	gdouble                    w, h, r2;
	gdouble                    waste;

	gl_debug (DEBUG_PRINT, "START");

	label_type = gl_template_get_first_label_type (pi->template);

	switch (label_type->shape) {

	case GL_TEMPLATE_SHAPE_RECT:
	case GL_TEMPLATE_SHAPE_ROUND:
		break;

	case GL_TEMPLATE_SHAPE_CD:
		gl_label_get_size (label, &w, &h);
		waste = label_type->size.cd.waste;
		r2    = label_type->size.cd.r2;
		create_ellipse_path (pi->pc, w/2, h/2, r2-waste, r2-waste);
		gnome_print_setrgbcolor (pi->pc, 1.0, 1.0, 1.0);
		gnome_print_setopacity (pi->pc, 1.0);
		gnome_print_fill (pi->pc);
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
create_rectangle_path (GnomePrintContext *pc,
		       gdouble            x0,
		       gdouble            y0,
		       gdouble            w,
		       gdouble            h)
{
	gl_debug (DEBUG_PRINT, "START");

	gnome_print_newpath (pc);
	gnome_print_moveto (pc, x0, y0);
	gnome_print_lineto (pc, x0 + w, y0);
	gnome_print_lineto (pc, x0 + w, y0 + h);
	gnome_print_lineto (pc, x0, y0 + h);
	gnome_print_lineto (pc, x0, y0);
	gnome_print_closepath (pc);

	gl_debug (DEBUG_PRINT, "END");
}

static void
create_ellipse_path (GnomePrintContext *pc,
		     gdouble            x0,
		     gdouble            y0,
		     gdouble            rx,
		     gdouble            ry)
{
	gdouble x, y;
	gint i_theta;

	gl_debug (DEBUG_PRINT, "START");

	gnome_print_newpath (pc);
	gnome_print_moveto (pc, x0 + rx, y0);
	for (i_theta = ARC_FINE; i_theta <= 360; i_theta += ARC_FINE) {
		x = x0 + rx * cos (i_theta * G_PI / 180.0);
		y = y0 + ry * sin (i_theta * G_PI / 180.0);
		gnome_print_lineto (pc, x, y);
	}
	gnome_print_closepath (pc);

	gl_debug (DEBUG_PRINT, "END");
}

static void
create_rounded_rectangle_path (GnomePrintContext *pc,
			       gdouble            x0,
			       gdouble            y0,
			       gdouble            w,
			       gdouble            h,
			       gdouble            r)
{
	gdouble x, y;
	gint i_theta;

	gl_debug (DEBUG_PRINT, "START");

	gnome_print_newpath (pc);

	gnome_print_moveto (pc, x0 + r, y0);
	for (i_theta = ARC_COURSE; i_theta <= 90; i_theta += ARC_COURSE) {
		x = x0 + r - r * sin (i_theta * G_PI / 180.0);
		y = y0 + r - r * cos (i_theta * G_PI / 180.0);
		gnome_print_lineto (pc, x, y);
	}
	for (i_theta = 0; i_theta <= 90; i_theta += ARC_COURSE) {
		x = x0 + r - r * cos (i_theta * G_PI / 180.0);
		y = y0 + (h - r) + r * sin (i_theta * G_PI / 180.0);
		gnome_print_lineto (pc, x, y);
	}
	for (i_theta = 0; i_theta <= 90; i_theta += ARC_COURSE) {
		x = x0 + (w - r) + r * sin (i_theta * G_PI / 180.0);
		y = y0 + (h - r) + r * cos (i_theta * G_PI / 180.0);
		gnome_print_lineto (pc, x, y);
	}
	for (i_theta = 0; i_theta <= 90; i_theta += ARC_COURSE) {
		x = x0 + (w - r) + r * cos (i_theta * G_PI / 180.0);
		y = y0 + r - r * sin (i_theta * G_PI / 180.0);
		gnome_print_lineto (pc, x, y);
	}
	gnome_print_lineto (pc, x0 + r, y0);

	gnome_print_closepath (pc);

	gl_debug (DEBUG_PRINT, "END");
}

static void
create_clipped_circle_path (GnomePrintContext *pc,
			    gdouble            x0,
			    gdouble            y0,
			    gdouble            w,
			    gdouble            h,
			    gdouble            r)
{
	gdouble x, y;
	gdouble theta1, theta2;
	gint    i_theta;

	gl_debug (DEBUG_PRINT, "START");

	theta1 = (180.0/G_PI) * acos (w / (2.0*r));
	theta2 = (180.0/G_PI) * asin (h / (2.0*r));

	gnome_print_newpath (pc);

	x = x0 + r * cos (theta1 * G_PI / 180.0);
	y = y0 + r * sin (theta1 * G_PI / 180.0);
	gnome_print_moveto (pc, x, y);

	for ( i_theta = theta1 + ARC_FINE; i_theta < theta2; i_theta +=ARC_FINE ) {
		x = x0 + r * cos (i_theta * G_PI / 180.0);
		y = y0 + r * sin (i_theta * G_PI / 180.0);
		gnome_print_lineto (pc, x, y);
	}

	x = x0 + r * cos (theta2 * G_PI / 180.0);
	y = y0 + r * sin (theta2 * G_PI / 180.0);
	gnome_print_lineto (pc, x, y);

	if ( fabs (theta2 - 90.0) > GNOME_CANVAS_EPSILON ) {
		x = x0 + r * cos ((180-theta2) * G_PI / 180.0);
		y = y0 + r * sin ((180-theta2) * G_PI / 180.0);
		gnome_print_lineto (pc, x, y);
	}

	for ( i_theta = 180-theta2+ARC_FINE; i_theta < (180-theta1); i_theta +=ARC_FINE ) {
		x = x0 + r * cos (i_theta * G_PI / 180.0);
		y = y0 + r * sin (i_theta * G_PI / 180.0);
		gnome_print_lineto (pc, x, y);
	}

	x = x0 + r * cos ((180-theta1) * G_PI / 180.0);
	y = y0 + r * sin ((180-theta1) * G_PI / 180.0);
	gnome_print_lineto (pc, x, y);

	if ( fabs (theta1) > GNOME_CANVAS_EPSILON ) {
		x = x0 + r * cos ((180+theta1) * G_PI / 180.0);
		y = y0 + r * sin ((180+theta1) * G_PI / 180.0);
		gnome_print_lineto (pc, x, y);
	}

	for ( i_theta = 180+theta1+ARC_FINE; i_theta < (180+theta2); i_theta +=ARC_FINE ) {
		x = x0 + r * cos (i_theta * G_PI / 180.0);
		y = y0 + r * sin (i_theta * G_PI / 180.0);
		gnome_print_lineto (pc, x, y);
	}

	x = x0 + r * cos ((180+theta2) * G_PI / 180.0);
	y = y0 + r * sin ((180+theta2) * G_PI / 180.0);
	gnome_print_lineto (pc, x, y);

	if ( fabs (theta2 - 90.0) > GNOME_CANVAS_EPSILON ) {
		x = x0 + r * cos ((360-theta2) * G_PI / 180.0);
		y = y0 + r * sin ((360-theta2) * G_PI / 180.0);
		gnome_print_lineto (pc, x, y);
	}

	for ( i_theta = 360-theta2+ARC_FINE; i_theta < (360-theta1); i_theta +=ARC_FINE ) {
		x = x0 + r * cos (i_theta * G_PI / 180.0);
		y = y0 + r * sin (i_theta * G_PI / 180.0);
		gnome_print_lineto (pc, x, y);
	}

	if ( fabs (theta1) > GNOME_CANVAS_EPSILON ) {
		x = x0 + r * cos ((360-theta1) * G_PI / 180.0);
		y = y0 + r * sin ((360-theta1) * G_PI / 180.0);
		gnome_print_lineto (pc, x, y);
	}

	x = x0 + r * cos (theta1 * G_PI / 180.0);
	y = y0 + r * sin (theta1 * G_PI / 180.0);
	gnome_print_lineto (pc, x, y);

	gnome_print_closepath (pc);

	gl_debug (DEBUG_PRINT, "END");
}

#ifndef NO_ALPHA_HACK
/*---------------------------------------------------------------------------*/
/* PRIVATE.  Extract a copy of rgba pixels, removing alpha by compositing    */
/* with a white background.                                                  */
/*                                                                           */
/* This is currently needed due to the lousy job gnome-print does in         */
/* rendering images with alpha channels to PS.  This sacrafices the ability  */
/* to do compositing of images with other items in the background.           */
/*---------------------------------------------------------------------------*/
static guchar *
get_pixels_as_rgb (const GdkPixbuf *pixbuf)
{
	gint             bits_per_sample, channels;
	gboolean         has_alpha;
	gint             width, height, rowstride;
	gulong           bytes;
	guchar          *buf_src, *buf_dest;
	guchar          *p_src, *p_dest;
	gint             ix, iy;
	guchar           r, g, b, a;
	gdouble          alpha, beta;

	gl_debug (DEBUG_PRINT, "START");

	g_return_val_if_fail (pixbuf && GDK_IS_PIXBUF (pixbuf), NULL);

	/* extract pixels and parameters from pixbuf. */
	buf_src         = gdk_pixbuf_get_pixels (pixbuf);
	bits_per_sample = gdk_pixbuf_get_bits_per_sample (pixbuf);
	channels        = gdk_pixbuf_get_n_channels (pixbuf);
	has_alpha       = gdk_pixbuf_get_has_alpha (pixbuf);
	width           = gdk_pixbuf_get_width (pixbuf);
	height          = gdk_pixbuf_get_height (pixbuf);
	rowstride       = gdk_pixbuf_get_rowstride (pixbuf);

	/* validate assumptions about pixbuf. */
        g_return_val_if_fail (buf_src, NULL);
        g_return_val_if_fail (bits_per_sample == 8, NULL);
        g_return_val_if_fail (channels == 4, NULL);
	g_return_val_if_fail (has_alpha, NULL);
        g_return_val_if_fail (width > 0, NULL);
        g_return_val_if_fail (height > 0, NULL);
        g_return_val_if_fail (rowstride > 0, NULL);

	/* Allocate a destination buffer */
	bytes = height * rowstride;
	gl_debug (DEBUG_PRINT, "bytes = %d", bytes);
	buf_dest = g_try_malloc (bytes);
	if (!buf_dest) {
		return NULL;
	}
	gl_debug (DEBUG_PRINT, "buf_dest = %x", buf_dest);

	/* Copy pixels, transforming rgba to rgb by compositing with a white bg. */
	p_src  = buf_src;
	p_dest = buf_dest;
	for ( iy=0; iy < height; iy++ ) {
	
		p_src  = buf_src + iy*rowstride;
		p_dest = buf_dest + iy*rowstride;

		for ( ix=0; ix < width; ix++ ) {

			r = *p_src++;
			g = *p_src++;
			b = *p_src++;
			a = *p_src++;

			alpha = a / 255.0;
			beta  = 1.0 - alpha;

			*p_dest++ = (guchar) (alpha*r + beta*255 + 0.5);
			*p_dest++ = (guchar) (alpha*g + beta*255 + 0.5);
			*p_dest++ = (guchar) (alpha*b + beta*255 + 0.5);

		}

	}

	gl_debug (DEBUG_PRINT, "START");

	return buf_dest;
}
#endif
