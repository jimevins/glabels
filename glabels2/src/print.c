/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  print.c:  Print module
 *
 *  Copyright (C) 2001  Jim Evins <evins@snaught.com>.
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

#include <math.h>
#include <time.h>
#include <ctype.h>
#include <gtk/gtk.h>
#include <libgnomeprint/gnome-print-paper.h>
#include <libgnomeprintui/gnome-printer-dialog.h>

#include "print.h"
#include "label.h"
#include "label-text.h"
#include "label-box.h"
#include "label-line.h"
#include "label-ellipse.h"
#include "label-image.h"
#include "label-barcode.h"
#include "bc.h"
#include "template.h"
#include "color.h"

#include "debug.h"

#define GL_PRINT_DEFAULT_PAPER "US Letter"

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
static PrintInfo *print_info_new              (GnomePrintMaster *master,
					       glLabel          *label);

static void       print_info_free             (PrintInfo       **pi);


static void       print_page_begin            (PrintInfo        *pi);

static void       print_page_end              (PrintInfo        *pi);

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
					       glLabelBox       *object);

static void       draw_line_object            (PrintInfo        *pi,
					       glLabelLine      *object);

static void       draw_ellipse_object         (PrintInfo        *pi,
					       glLabelEllipse   *object);

static void       draw_image_object           (PrintInfo        *pi,
					       glLabelImage     *object);

static void       draw_barcode_object         (PrintInfo        *pi,
					       glLabelBarcode   *object,
					       glMergeRecord    *record);


static void       draw_outline                (PrintInfo        *pi,
					       glLabel          *label);

static void       clip_to_outline             (PrintInfo        *pi,
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

/*****************************************************************************/
/* Simple (no merge data) print command.                                     */
/*****************************************************************************/
void
gl_print_simple (GnomePrintMaster *master,
		 glLabel          *label,
		 gint              n_sheets,
		 gint              first,
		 gint              last,
		 gboolean          outline_flag,
		 gboolean          reverse_flag)
{
	PrintInfo *pi;
	gint i_sheet, i_label;
	glTemplateOrigin *origins;

	gl_debug (DEBUG_PRINT, "START");

	pi = print_info_new (master, label);

	origins = gl_template_get_origins (pi->template);

	for (i_sheet = 0; i_sheet < n_sheets; i_sheet++) {

		print_page_begin (pi);

		for (i_label = first - 1; i_label < last; i_label++) {

			print_label (pi, label,
				     origins[i_label].x, origins[i_label].y,
				     NULL, outline_flag, reverse_flag);

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
gl_print_merge_collated (GnomePrintMaster *master,
			 glLabel          *label,
			 GList            *record_list,
			 gint              n_copies,
			 gint              first,
			 gboolean          outline_flag,
			 gboolean          reverse_flag)
{
	PrintInfo *pi;
	gint i_sheet, i_label, n_labels_per_page, i_copy;
	glMergeRecord *record;
	GList *p;
	glTemplateOrigin *origins;

	gl_debug (DEBUG_PRINT, "START");

	pi = print_info_new (master, label);

	n_labels_per_page = gl_template_get_n_labels (pi->template);
	origins = gl_template_get_origins (pi->template);

	i_sheet = 0;
	i_label = first - 1;

	for ( p=record_list; p!=NULL; p=p->next ) {
		record = (glMergeRecord *)p->data;
			
		if ( record->select_flag ) {
			for (i_copy = 0; i_copy < n_copies; i_copy++) {

				if ((i_label == 0) || (i_sheet == 0)) {
					print_page_begin (pi);
				}

				print_label (pi, label,
					     origins[i_label].x,
					     origins[i_label].y,
					     record,
					     outline_flag, reverse_flag);

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
gl_print_merge_uncollated (GnomePrintMaster *master,
			   glLabel          *label,
			   GList            *record_list,
			   gint              n_copies,
			   gint              first,
			   gboolean          outline_flag,
			   gboolean          reverse_flag)
{
	PrintInfo *pi;
	gint i_sheet, i_label, n_labels_per_page, i_copy;
	glMergeRecord *record;
	GList *p;
	glTemplateOrigin *origins;

	gl_debug (DEBUG_PRINT, "START");

	pi = print_info_new (master, label);

	n_labels_per_page = gl_template_get_n_labels (pi->template);
	origins = gl_template_get_origins (pi->template);

	i_sheet = 0;
	i_label = first - 1;

	for (i_copy = 0; i_copy < n_copies; i_copy++) {

		for ( p=record_list; p!=NULL; p=p->next ) {
			record = (glMergeRecord *)p->data;
			
			if ( record->select_flag ) {


				if ((i_label == 0) || (i_sheet == 0)) {
					print_page_begin (pi);
				}

				print_label (pi, label,
					     origins[i_label].x,
					     origins[i_label].y,
					     record,
					     outline_flag, reverse_flag);

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
gl_print_batch (GnomePrintMaster *master,
		glLabel          *label,
		gint              n_sheets,
		gint              n_copies,
		gboolean          outline_flag,
		gboolean          reverse_flag)
{
	gint n_per_page;
	GList *record_list = NULL;
	glMerge *merge;
	glTemplate *template;
	
	gl_debug (DEBUG_PRINT, "START");

	merge = gl_label_get_merge (label);
	template = gl_label_get_template (label);

	if ( merge->type == GL_MERGE_NONE ) {
		n_per_page = gl_template_get_n_labels(template);

		gl_print_simple (master, label, n_sheets, 1, n_per_page,
				 outline_flag, reverse_flag);
	} else {
		record_list = gl_merge_read_data (merge->type,
						  merge->field_defs,
						  merge->src);

		gl_print_merge_collated (master, label, record_list,
					 n_copies, 1,
					 outline_flag, reverse_flag);
	}
	gl_merge_free (&merge);
	gl_template_free (&template);

	gl_debug (DEBUG_PRINT, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  new print info structure                                        */
/*---------------------------------------------------------------------------*/
static PrintInfo *
print_info_new (GnomePrintMaster *master,
		glLabel          *label)
{
	PrintInfo *pi = g_new0 (PrintInfo, 1);
	glTemplate *template;
	const GnomePrintPaper *paper = NULL;

	gl_debug (DEBUG_PRINT, "START");

	template = gl_label_get_template (label);

	if (template == NULL) {
		g_warning ("Label has no template");
		return NULL;
	}

	pi->pc = gnome_print_master_get_context (master);
	pi->config = gnome_print_master_get_config (master);

	if ((template != NULL) && (template->page_size != NULL)) {

		gl_debug (DEBUG_PRINT,
			  "setting page size = \"%s\"", template->page_size);


		/* Currently cannot set page size directly from name, */
		/* since we must set Ids not Names and there is no */
		/* way to do the reverse lookup of Id from Name. */
		/* Sometimes they are the same, but not always */
		/* (e.g. for the name "US Letter" id="USLetter" */
		/* So we use the "Custom" Id. */
		paper = gnome_print_paper_get_by_name (template->page_size);
		gnome_print_config_set (pi->config,
					GNOME_PRINT_KEY_PAPER_SIZE,
					"Custom");
		gnome_print_config_set_length (pi->config,
					       GNOME_PRINT_KEY_PAPER_WIDTH,
					       paper->width,
					       GNOME_PRINT_PS_UNIT);
		gnome_print_config_set_length (pi->config,
					       GNOME_PRINT_KEY_PAPER_HEIGHT,
					       paper->height,
					       GNOME_PRINT_PS_UNIT);
	} else {
		g_warning ("Cannot determine correct page size.");
		paper = gnome_print_paper_get_by_name (GL_PRINT_DEFAULT_PAPER);
		gnome_print_config_set (pi->config,
					GNOME_PRINT_KEY_PAPER_SIZE,
					GL_PRINT_DEFAULT_PAPER);
	}
	pi->page_width = paper->width;
	pi->page_height = paper->height;

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

	gl_template_free (&(*pi)->template);

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

	pi->sheet++;

	str = g_strdup_printf ("sheet%02d", pi->sheet);
	gnome_print_beginpage (pi->pc, str);
	g_free (str);

	/* Translate and scale, so that our origin is at the upper left. */
	gnome_print_translate (pi->pc, 0.0, pi->page_height);
	gnome_print_scale (pi->pc, 1.0, -1.0);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  End a page.                                                     */
/*---------------------------------------------------------------------------*/
static void
print_page_end (PrintInfo *pi)
{
	gnome_print_showpage (pi->pc);
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
	gdouble width, height;
	glTemplate *template;

	gl_debug (DEBUG_PRINT, "START");

	template = gl_label_get_template (label);

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
	if (outline_flag) {
		draw_outline (pi, label);
	}
	clip_to_outline (pi, label);
	draw_label (pi, label, record);

	gnome_print_grestore (pi->pc);

	gl_template_free (&template);

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
	gdouble x0, y0, w, h;
	glLabelObjectFlip flip;

	gl_debug (DEBUG_PRINT, "START");

	gl_label_object_get_position (object, &x0, &y0);
	gl_label_object_get_size (object, &w, &h);
	flip = gl_label_object_get_flip (object);

	gnome_print_gsave (pi->pc);

	gnome_print_translate (pi->pc, x0, y0);

	if ( flip & GL_LABEL_OBJECT_FLIP_HORIZ ) {
		gnome_print_translate (pi->pc, w, 0.0);
		gnome_print_scale (pi->pc, -1.0, 1.0);
	}
	if ( flip & GL_LABEL_OBJECT_FLIP_VERT ) {
		gnome_print_translate (pi->pc, 0.0, h);
		gnome_print_scale (pi->pc, 1.0, -1.0);
	}

	if (GL_IS_LABEL_TEXT(object)) {
		draw_text_object (pi, GL_LABEL_TEXT(object), record);
	} else if (GL_IS_LABEL_BOX(object)) {
		draw_box_object (pi, GL_LABEL_BOX(object));
	} else if (GL_IS_LABEL_LINE(object)) {
		draw_line_object (pi, GL_LABEL_LINE(object));
	} else if (GL_IS_LABEL_ELLIPSE(object)) {
		draw_ellipse_object (pi, GL_LABEL_ELLIPSE(object));
	} else if (GL_IS_LABEL_IMAGE(object)) {
		draw_image_object (pi, GL_LABEL_IMAGE(object));
	} else if (GL_IS_LABEL_BARCODE(object)) {
		draw_barcode_object (pi, GL_LABEL_BARCODE(object),
				     record);
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
	GnomeFont *font;
	gchar **line;
	gint i;
	gdouble x_offset, y_offset, w, object_w, object_h;
	gchar *text;
	GList *lines;
	gchar *font_family;
	gdouble font_size;
	GnomeFontWeight font_weight;
	gboolean font_italic_flag;
	guint color;
	GtkJustification just;
	GnomeGlyphList *glyphlist;
	ArtDRect bbox;
	gdouble affine[6];


	gl_debug (DEBUG_PRINT, "START");

	gl_label_object_get_size (GL_LABEL_OBJECT(object), &object_w, &object_h);
	lines = gl_label_text_get_lines (object);
	gl_label_text_get_props (object,
				 &font_family, &font_size, &font_weight,
				 &font_italic_flag,
				 &color, &just);

	font = gnome_font_find_closest_from_weight_slant (
                                       font_family,
				       font_weight,
				       font_italic_flag,
				       font_size);
	gnome_print_setfont (pi->pc, font);

	gnome_print_setrgbcolor (pi->pc,
				 GL_COLOR_F_RED (color),
				 GL_COLOR_F_GREEN (color),
				 GL_COLOR_F_BLUE (color));
	gnome_print_setopacity (pi->pc, GL_COLOR_F_ALPHA (color));

	text = gl_text_node_lines_expand (lines, record);
	line = g_strsplit (text, "\n", -1);
	g_free (text);

	art_affine_identity (affine);

	for (i = 0; line[i] != NULL; i++) {

		glyphlist = gnome_glyphlist_from_text_dumb (font, color,
							    0.0, 0.0,
							    line[i]);

		gnome_glyphlist_bbox (glyphlist, affine, 0, &bbox);
		w = bbox.x1;

		switch (just) {
		case GTK_JUSTIFY_LEFT:
			x_offset = 0.0;
			break;
		case GTK_JUSTIFY_CENTER:
			x_offset = (object_w - w) / 2.0;
			break;
		case GTK_JUSTIFY_RIGHT:
			x_offset = object_w - w;
			break;
		default:
			x_offset = 0.0;
			break;	/* shouldn't happen */
		}

		y_offset = (i + 1) * font_size
		    + gnome_font_get_descender (font);

		gnome_print_moveto (pi->pc, x_offset, y_offset);

		gnome_print_gsave (pi->pc);
		gnome_print_scale (pi->pc, 1.0, -1.0);
		gnome_print_show (pi->pc, line[i]);
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
		 glLabelBox *object)
{
	gdouble w, h;
	gdouble line_width;
	guint line_color, fill_color;

	gl_debug (DEBUG_PRINT, "START");

	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_box_get_line_width (object);
	line_color = gl_label_box_get_line_color (object);
	fill_color = gl_label_box_get_fill_color (object);

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
		  glLabelLine *object)
{
	gdouble w, h;
	gdouble line_width;
	guint line_color;

	gl_debug (DEBUG_PRINT, "START");

	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_line_get_line_width (object);
	line_color = gl_label_line_get_line_color (object);

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
		     glLabelEllipse *object)
{
	gdouble x0, y0, rx, ry, w, h;
	gdouble line_width;
	guint line_color, fill_color;

	gl_debug (DEBUG_PRINT, "START");

	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_ellipse_get_line_width (object);
	line_color = gl_label_ellipse_get_line_color (object);
	fill_color = gl_label_ellipse_get_fill_color (object);

	rx = w / 2.0;
	ry = h / 2.0;
	x0 = rx;
	y0 = ry;

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
draw_image_object (PrintInfo    *pi,
		   glLabelImage *object)
{
	gdouble w, h;
	const GdkPixbuf *pixbuf;
	guchar *image_data;
	gint image_w, image_h, image_stride;
	gboolean image_alpha_flag;
	gint ret;

	gl_debug (DEBUG_PRINT, "START");

	gl_label_object_get_size     (GL_LABEL_OBJECT(object), &w, &h);

	pixbuf = gl_label_image_get_pixbuf (object);
	image_data = gdk_pixbuf_get_pixels (pixbuf);
	image_w = gdk_pixbuf_get_width (pixbuf);
	image_h = gdk_pixbuf_get_height (pixbuf);
	image_stride = gdk_pixbuf_get_rowstride(pixbuf);
	image_alpha_flag = gdk_pixbuf_get_has_alpha(pixbuf);

	gnome_print_gsave (pi->pc);
	gnome_print_translate (pi->pc, 0.0, h);
	gnome_print_scale (pi->pc, w, -h);
	if (image_alpha_flag) {
	        ret = gnome_print_rgbaimage (pi->pc, image_data,
					     image_w, image_h, image_stride);
		gl_debug (DEBUG_PRINT, "Ret a = %d", ret);
	} else {
	        ret = gnome_print_rgbimage (pi->pc, image_data,
					    image_w, image_h, image_stride);
		gl_debug (DEBUG_PRINT, "Ret = %d", ret);
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
	glBarcode *gbc;
	glBarcodeLine *line;
	glBarcodeChar *bchar;
	GList *li;
	gdouble y_offset;
	GnomeFont *font;
	gchar *text, *cstring;
	glTextNode          *text_node;
	glBarcodeStyle      style;
	gboolean            text_flag;
	guint               color;
	gdouble             scale;

	gl_debug (DEBUG_PRINT, "START");

	text_node = gl_label_barcode_get_data (object);
	gl_label_barcode_get_props (object,
				    &style, &text_flag, &color, &scale);

	text = gl_text_node_expand (text_node, record);
	gbc = gl_barcode_new (style, text_flag, scale, text);
	g_free (text);
	gl_text_node_free (&text_node);

	if (gbc == NULL) {

		font = gnome_font_find_closest_from_weight_slant (
                                               GL_BARCODE_FONT_FAMILY,
					       GL_BARCODE_FONT_WEIGHT,
					       FALSE, 12.0);
		gnome_print_setfont (pi->pc, font);

		gnome_print_setrgbcolor (pi->pc,
					 GL_COLOR_F_RED (color),
					 GL_COLOR_F_GREEN (color),
					 GL_COLOR_F_BLUE (color));
		gnome_print_setopacity (pi->pc,
					GL_COLOR_F_ALPHA (color));

		y_offset = 12.0 - gnome_font_get_descender (font);
		gnome_print_moveto (pi->pc, 0.0, y_offset);

		gnome_print_gsave (pi->pc);
		gnome_print_scale (pi->pc, 1.0, -1.0);
		gnome_print_show (pi->pc, _("Invalid barcode"));
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
				                       GL_BARCODE_FONT_FAMILY,
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
			    bchar->y + bchar->fsize -
			    gnome_font_get_descender (font);
			gnome_print_moveto (pi->pc, bchar->x, y_offset);

			cstring = g_strdup_printf ("%c", bchar->c);
			gnome_print_gsave (pi->pc);
			gnome_print_scale (pi->pc, 1.0, -1.0);
			gnome_print_show (pi->pc, cstring);
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
	gdouble w, h, r;
	gdouble r1, r2;
	glTemplate *template;

	gl_debug (DEBUG_PRINT, "START");

	template = gl_label_get_template (label);

	gnome_print_setrgbcolor (pi->pc, 0.25, 0.25, 0.25);
	gnome_print_setopacity (pi->pc, 1.0);
	gnome_print_setlinewidth (pi->pc, 0.25);

	switch (template->label.style) {

	case GL_TEMPLATE_STYLE_RECT:
		gl_label_get_size (label, &w, &h);
		r = template->label.rect.r;
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

	case GL_TEMPLATE_STYLE_ROUND:
		/* Round style */
		r1 = template->label.round.r;
		create_ellipse_path (pi->pc, r1, r1, r1, r1);
		gnome_print_stroke (pi->pc);
		break;

	case GL_TEMPLATE_STYLE_CD:
		/* CD style, round label w/ concentric round hole */
		r1 = template->label.cd.r1;
		r2 = template->label.cd.r2;
		create_ellipse_path (pi->pc, r1, r1, r1, r1);
		gnome_print_stroke (pi->pc);
		create_ellipse_path (pi->pc, r1, r1, r2, r2);
		gnome_print_stroke (pi->pc);
		break;

	default:
		g_warning ("Unknown template label style");
		break;
	}

	gl_template_free (&template);

	gl_debug (DEBUG_PRINT, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Clip to outline.                                                */
/*---------------------------------------------------------------------------*/
static void
clip_to_outline (PrintInfo *pi,
		 glLabel   *label)
{
	gdouble w, h, r;
	gdouble r1;
	glTemplate *template;

	gl_debug (DEBUG_PRINT, "START");

	template = gl_label_get_template (label);

	switch (template->label.style) {

	case GL_TEMPLATE_STYLE_RECT:
		gl_label_get_size (label, &w, &h);
		r = template->label.rect.r;
		if (r == 0.0) {
			/* simple rectangle */
			create_rectangle_path (pi->pc, 0.0, 0.0, w, h);
		} else {
			/* rectangle with rounded corners */
			create_rounded_rectangle_path (pi->pc, 0.0, 0.0,
						       w, h, r);
		}
		gnome_print_clip (pi->pc);
		break;

	case GL_TEMPLATE_STYLE_ROUND:
		r1 = template->label.round.r;
		create_ellipse_path (pi->pc, r1, r1, r1, r1);
		gnome_print_clip (pi->pc);
		break;

	case GL_TEMPLATE_STYLE_CD:
		r1 = template->label.cd.r1;
		create_ellipse_path (pi->pc, r1, r1, r1, r1);
		gnome_print_clip (pi->pc);
		break;

	default:
		g_warning ("Unknown template label style");
		break;
	}

	gl_template_free (&template);

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
	for (i_theta = 2; i_theta <= 360; i_theta += 2) {
		x = x0 + rx * cos (i_theta * M_PI / 180.0);
		y = y0 + ry * sin (i_theta * M_PI / 180.0);
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
	for (i_theta = 5; i_theta <= 90; i_theta += 5) {
		x = x0 + r - r * sin (i_theta * M_PI / 180.0);
		y = y0 + r - r * cos (i_theta * M_PI / 180.0);
		gnome_print_lineto (pc, x, y);
	}
	for (i_theta = 0; i_theta <= 90; i_theta += 5) {
		x = x0 + r - r * cos (i_theta * M_PI / 180.0);
		y = y0 + (h - r) + r * sin (i_theta * M_PI / 180.0);
		gnome_print_lineto (pc, x, y);
	}
	for (i_theta = 0; i_theta <= 90; i_theta += 5) {
		x = x0 + (w - r) + r * sin (i_theta * M_PI / 180.0);
		y = y0 + (h - r) + r * cos (i_theta * M_PI / 180.0);
		gnome_print_lineto (pc, x, y);
	}
	for (i_theta = 0; i_theta <= 90; i_theta += 5) {
		x = x0 + (w - r) + r * cos (i_theta * M_PI / 180.0);
		y = y0 + r - r * sin (i_theta * M_PI / 180.0);
		gnome_print_lineto (pc, x, y);
	}
	gnome_print_lineto (pc, x0 + r, y0);

	gnome_print_closepath (pc);

	gl_debug (DEBUG_PRINT, "END");
}
