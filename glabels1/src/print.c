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
#include <libgnome/gnome-paper.h>
#include <libgnomeprint/gnome-printer.h>

#include "print.h"
#include "label.h"
#include "bc.h"
#include "template.h"
#include "hack.h"

#include "debug.h"

#define GL_PRINT_DEFAULT_PAPER "US-Letter"

#define RED(x)   ( (((x)>>24) & 0xff) / 255.0 )
#define GREEN(x) ( (((x)>>16) & 0xff) / 255.0 )
#define BLUE(x)  ( (((x)>>8)  & 0xff) / 255.0 )
#define ALPHA(x) ( ( (x)      & 0xff) / 255.0 )

/*===========================================*/
/* Private types.                            */
/*===========================================*/
typedef struct _PrintInfo {
	/* gnome print context */
	GnomePrintContext *pc;

	/* gLabels Template */
	glTemplate *template;
	gboolean label_rotate_flag;

} PrintInfo;

/*===========================================*/
/* Private function prototypes.              */
/*===========================================*/
static PrintInfo *print_info_new (GnomePrintMaster * master, glLabel * label);
static void print_info_free (PrintInfo ** pi);

static void print_label (PrintInfo * pi, glLabel * label, gint i,
			 glMergeRecord * record, gboolean outline_flag,
			 gboolean reverse_flag);

static void draw_label (PrintInfo * pi, glLabel * label,
			glMergeRecord * record);

static void draw_text_object (PrintInfo * pi, glLabelObject * object,
			      glMergeRecord * record);
static void draw_box_object (PrintInfo * pi, glLabelObject * object);
static void draw_line_object (PrintInfo * pi, glLabelObject * object);
static void draw_ellipse_object (PrintInfo * pi, glLabelObject * object);
static void draw_image_object (PrintInfo * pi, glLabelObject * object);
static void draw_barcode_object (PrintInfo * pi, glLabelObject * object,
				 glMergeRecord * record);

static void draw_outline (PrintInfo * pi, glLabel * label);
static void clip_to_outline (PrintInfo * pi, glLabel * label);

static void create_rectangle_path (GnomePrintContext * pc,
				   gdouble x0, gdouble y0,
				   gdouble w, gdouble h);
static void create_ellipse_path (GnomePrintContext * pc,
				 gdouble x0, gdouble y0,
				 gdouble rx, gdouble ry);
static void create_rounded_rectangle_path (GnomePrintContext * pc,
					   gdouble x0, gdouble y0,
					   gdouble w, gdouble h, gdouble r);

/*****************************************************************************/
/* Simple (no merge data) print command.                                     */
/*****************************************************************************/
void
gl_print_simple (GnomePrintMaster * master,
		 glLabel * label,
		 gint n_sheets,
		 gint first,
		 gint last,
		 gboolean outline_flag,
		 gboolean reverse_flag)
{
	PrintInfo *pi;
	gint i_sheet, i_label;
	gchar *page_str = NULL;

	pi = print_info_new (master, label);

	for (i_sheet = 0; i_sheet < n_sheets; i_sheet++) {

		page_str = g_strdup_printf ("sheet %d", i_sheet + 1);
		gnome_print_beginpage (pi->pc, page_str);
		g_free (page_str);

		for (i_label = first - 1; i_label < last; i_label++) {

			print_label (pi, label, i_label, NULL,
				     outline_flag, reverse_flag);

		}

		gnome_print_showpage (pi->pc);
	}

	print_info_free (&pi);
}

/*****************************************************************************/
/* Merge print command (collated copies)                                     */
/*****************************************************************************/
void
gl_print_merge_collated (GnomePrintMaster * master,
			 glLabel * label,
			 GList * record_list,
			 gint n_copies,
			 gint first,
			 gboolean outline_flag,
			 gboolean reverse_flag)
{
	PrintInfo *pi;
	gint i_sheet, i_label, n_labels_per_page, i_copy;
	gchar *str = NULL;
	glMergeRecord *record;
	GList *p;

	pi = print_info_new (master, label);

	n_labels_per_page = (pi->template->nx) * (pi->template->ny);

	i_sheet = 0;
	i_label = first - 1;

	for ( p=record_list; p!=NULL; p=p->next ) {
		record = (glMergeRecord *)p->data;
			
		if ( record->select_flag ) {
			for (i_copy = 0; i_copy < n_copies; i_copy++) {

				if ((i_label == 0) || (i_sheet == 0)) {
					str = g_strdup_printf ("sheet %d",
							       ++i_sheet);
					gnome_print_beginpage (pi->pc, str);
					g_free (str);
				}

				print_label (pi, label, i_label, record,
					     outline_flag, reverse_flag);

				i_label = (i_label + 1) % n_labels_per_page;
				if (i_label == 0) {
					gnome_print_showpage (pi->pc);
				}
			}
		}
	}

	if (i_label != 0) {
		gnome_print_showpage (pi->pc);
	}

	print_info_free (&pi);
}

/*****************************************************************************/
/* Merge print command (uncollated copies)                                   */
/*****************************************************************************/
void
gl_print_merge_uncollated (GnomePrintMaster * master,
			   glLabel * label,
			   GList * record_list,
			   gint n_copies,
			   gint first,
			   gboolean outline_flag,
			   gboolean reverse_flag)
{
	PrintInfo *pi;
	gint i_sheet, i_label, n_labels_per_page, i_copy;
	gchar *str = NULL;
	glMergeRecord *record;
	GList *p;

	pi = print_info_new (master, label);

	n_labels_per_page = (pi->template->nx) * (pi->template->ny);

	i_sheet = 0;
	i_label = first - 1;

	for (i_copy = 0; i_copy < n_copies; i_copy++) {

		for ( p=record_list; p!=NULL; p=p->next ) {
			record = (glMergeRecord *)p->data;
			
			if ( record->select_flag ) {


				if ((i_label == 0) || (i_sheet == 0)) {
					str = g_strdup_printf ("sheet %d",
							       ++i_sheet);
					gnome_print_beginpage (pi->pc, str);
					g_free (str);
				}

				print_label (pi, label, i_label, record,
					     outline_flag, reverse_flag);

				i_label = (i_label + 1) % n_labels_per_page;
				if (i_label == 0) {
					gnome_print_showpage (pi->pc);
				}
			}
		}

	}
	if (i_label != 0) {
		gnome_print_showpage (pi->pc);
	}

	print_info_free (&pi);
}

/*****************************************************************************/
/* Batch print.  Call appropriate function above.                            */
/*****************************************************************************/
void
gl_print_batch (GnomePrintMaster * master, glLabel * label,
		gint n_sheets, gint n_copies,
		gboolean outline_flag, gboolean reverse_flag)
{
	gint n_per_page;
	GList *record_list = NULL;

	if ( label->merge_type == GL_MERGE_NONE ) {
		n_per_page = (label->template->nx)*(label->template->ny);

		gl_print_simple (master, label, n_sheets, 1, n_per_page,
				 outline_flag, reverse_flag);
	} else {
		record_list = gl_merge_read_data (label->merge_type,
						  label->merge_fields,
						  label->merge_src);

		gl_print_merge_collated (master, label, record_list,
					 n_copies, 1,
					 outline_flag, reverse_flag);
	}
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  new print info structure                                        */
/*---------------------------------------------------------------------------*/
static PrintInfo *
print_info_new (GnomePrintMaster * master,
		glLabel * label)
{
	const GnomePaper *paper;
	PrintInfo *pi = g_new0 (PrintInfo, 1);
	glTemplate *template = label->template;

	if (template == NULL) {
		WARN ("Undefined template \"%s\"", label->template_name);
		return NULL;
	}

	pi->pc = gnome_print_master_get_context (master);

	if ((template != NULL) && (template->page_size != NULL)) {
		paper = gnome_paper_with_name (template->page_size);
	} else {
		paper = gnome_paper_with_name (GL_PRINT_DEFAULT_PAPER);
	}
	gnome_print_master_set_paper (master, paper);

	pi->template = template;
	pi->label_rotate_flag = label->rotate_flag;

	return pi;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  free print info structure                                       */
/*---------------------------------------------------------------------------*/
static void
print_info_free (PrintInfo ** pi)
{
	gnome_print_context_close ((*pi)->pc);

	g_free (*pi);
	*pi = NULL;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Print i'th label.                                               */
/*---------------------------------------------------------------------------*/
static void
print_label (PrintInfo * pi,
	     glLabel * label,
	     gint i_label,
	     glMergeRecord * record,
	     gboolean outline_flag,
	     gboolean reverse_flag)
{
	gdouble a[6];
	gint ix, iy;

	ix = i_label % (pi->template->nx);
	iy = ((pi->template->ny) - 1) - (i_label / (pi->template->nx));

	gnome_print_gsave (pi->pc);

	/* Transform coordinate system to be relative to upper corner */
	/* of the current label */
	gnome_print_translate (pi->pc,
			       ix * (pi->template->dx) + pi->template->x0,
			       iy * (pi->template->dy) + pi->template->y0);
	if (!label->rotate_flag) {
		art_affine_scale (a, 1.0, -1.0);
		a[5] = label->height;
		gnome_print_concat (pi->pc, a);
	} else {
		gnome_print_rotate (pi->pc, 90.0);
		gnome_print_scale (pi->pc, 1.0, -1.0);
	}
	if ( reverse_flag ) {
		gnome_print_translate (pi->pc, label->width, 0.0);
		art_affine_scale (a, -1.0, 1.0);
		gnome_print_concat (pi->pc, a);
	}
	if (outline_flag) {
		draw_outline (pi, label);
	}
	clip_to_outline (pi, label);
	draw_label (pi, label, record);

	gnome_print_grestore (pi->pc);

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw label.                                                     */
/*---------------------------------------------------------------------------*/
static void
draw_label (PrintInfo * pi,
	    glLabel * label,
	    glMergeRecord * record)
{
	GList *p_obj;
	glLabelObject *object;

	for (p_obj = label->objects; p_obj != NULL; p_obj = p_obj->next) {
		object = (glLabelObject *) p_obj->data;

		if (object->type == GL_LABEL_OBJECT_TEXT) {
			draw_text_object (pi, object, record);
		} else if (object->type == GL_LABEL_OBJECT_BOX) {
			draw_box_object (pi, object);
		} else if (object->type == GL_LABEL_OBJECT_LINE) {
			draw_line_object (pi, object);
		} else if (object->type == GL_LABEL_OBJECT_ELLIPSE) {
			draw_ellipse_object (pi, object);
		} else if (object->type == GL_LABEL_OBJECT_IMAGE) {
			draw_image_object (pi, object);
		} else if (object->type == GL_LABEL_OBJECT_BARCODE) {
			draw_barcode_object (pi, object, record);
		}

	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw text object.                                               */
/*---------------------------------------------------------------------------*/
static void
draw_text_object (PrintInfo * pi,
		  glLabelObject * object,
		  glMergeRecord * record)
{
	GnomeFont *font;
	gchar **line;
	gint i;
	gdouble w;
	gdouble x_offset, y_offset;
	gdouble x, y;
	gchar *text, *utf8_text;

	font = gnome_font_new_closest (object->arg.text.font_family,
				       object->arg.text.font_weight,
				       object->arg.text.font_italic_flag,
				       object->arg.text.font_size);
	gnome_print_setfont (pi->pc, font);

	gnome_print_setrgbcolor (pi->pc,
				 RED (object->arg.text.color),
				 GREEN (object->arg.text.color),
				 BLUE (object->arg.text.color));
	gnome_print_setopacity (pi->pc, ALPHA (object->arg.text.color));

	text = gl_text_node_lines_expand (object->arg.text.lines, record);
	line = g_strsplit (text, "\n", -1);
	g_free (text);

	for (i = 0; line[i] != NULL; i++) {

		utf8_text = gl_hack_text_to_utf8 (line[i]);

		w = gl_hack_get_width_string (font, line[i]);

		switch (object->arg.text.just) {
		case GTK_JUSTIFY_LEFT:
			x_offset = 0.0;
			break;
		case GTK_JUSTIFY_CENTER:
			x_offset = -w / 2.0;
			break;
		case GTK_JUSTIFY_RIGHT:
			x_offset = -w;
			break;
		default:
			x_offset = 0.0;
			break;	/* shouldn't happen */
		}

		y_offset = (i + 1) * object->arg.text.font_size
		    - gnome_font_get_descender (font);

		x = object->x + x_offset;
		y = object->y + y_offset;
		gnome_print_moveto (pi->pc, x, y);

		gnome_print_gsave (pi->pc);
		gnome_print_scale (pi->pc, 1.0, -1.0);
		gnome_print_show (pi->pc, utf8_text);
		gnome_print_grestore (pi->pc);

		g_free (utf8_text);
	}

	g_strfreev (line);

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw box object.                                                */
/*---------------------------------------------------------------------------*/
static void
draw_box_object (PrintInfo * pi,
		 glLabelObject * object)
{
	gdouble x, y, w, h;

	x = object->x;
	y = object->y;
	w = object->arg.box.w;
	h = object->arg.box.h;

	/* Paint fill color */
	create_rectangle_path (pi->pc, x, y, w, h);
	gnome_print_setrgbcolor (pi->pc,
				 RED (object->arg.box.fill_color),
				 GREEN (object->arg.box.fill_color),
				 BLUE (object->arg.box.fill_color));
	gnome_print_setopacity (pi->pc, ALPHA (object->arg.box.fill_color));
	gnome_print_fill (pi->pc);

	/* Draw outline */
	create_rectangle_path (pi->pc, x, y, w, h);
	gnome_print_setrgbcolor (pi->pc,
				 RED (object->arg.box.line_color),
				 GREEN (object->arg.box.line_color),
				 BLUE (object->arg.box.line_color));
	gnome_print_setopacity (pi->pc, ALPHA (object->arg.box.line_color));
	gnome_print_setlinewidth (pi->pc, object->arg.box.line_width);
	gnome_print_stroke (pi->pc);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw line object.                                               */
/*---------------------------------------------------------------------------*/
static void
draw_line_object (PrintInfo * pi,
		  glLabelObject * object)
{
	gdouble x, y, dx, dy;

	x = object->x;
	y = object->y;
	dx = object->arg.line.dx;
	dy = object->arg.line.dy;

	gnome_print_moveto (pi->pc, x, y);
	gnome_print_lineto (pi->pc, x + dx, y + dy);
	gnome_print_setrgbcolor (pi->pc,
				 RED (object->arg.line.line_color),
				 GREEN (object->arg.line.line_color),
				 BLUE (object->arg.line.line_color));
	gnome_print_setopacity (pi->pc, ALPHA (object->arg.line.line_color));
	gnome_print_setlinewidth (pi->pc, object->arg.line.line_width);
	gnome_print_stroke (pi->pc);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw ellipse object.                                            */
/*---------------------------------------------------------------------------*/
static void
draw_ellipse_object (PrintInfo * pi,
		     glLabelObject * object)
{
	gdouble x0, y0, rx, ry;

	rx = object->arg.ellipse.w / 2.0;
	ry = object->arg.ellipse.h / 2.0;
	x0 = object->x + rx;
	y0 = object->y + ry;

	/* Paint fill color */
	create_ellipse_path (pi->pc, x0, y0, rx, ry);
	gnome_print_setrgbcolor (pi->pc,
				 RED (object->arg.ellipse.fill_color),
				 GREEN (object->arg.ellipse.fill_color),
				 BLUE (object->arg.ellipse.fill_color));
	gnome_print_setopacity (pi->pc, ALPHA (object->arg.ellipse.fill_color));
	gnome_print_fill (pi->pc);

	/* Draw outline */
	create_ellipse_path (pi->pc, x0, y0, rx, ry);
	gnome_print_setrgbcolor (pi->pc,
				 RED (object->arg.ellipse.line_color),
				 GREEN (object->arg.ellipse.line_color),
				 BLUE (object->arg.ellipse.line_color));
	gnome_print_setopacity (pi->pc, ALPHA (object->arg.ellipse.line_color));
	gnome_print_setlinewidth (pi->pc, object->arg.ellipse.line_width);
	gnome_print_stroke (pi->pc);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw image object.                                              */
/*---------------------------------------------------------------------------*/
static void
draw_image_object (PrintInfo * pi,
		   glLabelObject * object)
{
	gdouble x, y, w, h;
	GdkPixbuf *pixbuf;

	x = object->x;
	y = object->y;
	w = object->arg.image.w;
	h = object->arg.image.h;

	pixbuf = object->arg.image.image;

	gnome_print_gsave (pi->pc);
	gnome_print_translate (pi->pc, x, y + h);
	gnome_print_scale (pi->pc, w, -h);
	gnome_print_pixbuf (pi->pc, pixbuf);
	gnome_print_grestore (pi->pc);

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw box object.                                                */
/*---------------------------------------------------------------------------*/
static void
draw_barcode_object (PrintInfo * pi,
		     glLabelObject * object,
		     glMergeRecord * record)
{
	glBarcode *gbc;
	glBarcodeLine *line;
	glBarcodeChar *bchar;
	GList *li;
	gdouble x, y, y_offset;
	GnomeFont *font;
	gchar *text, *cstring;

	x = object->x;
	y = object->y;

	text = gl_text_node_expand (object->arg.barcode.text_node, record);
	gbc = gl_barcode (object->arg.barcode.style,
			  object->arg.barcode.text_flag,
			  object->arg.barcode.scale, text);
	g_free (text);

	if (gbc == NULL) {

		font = gnome_font_new_closest (GL_BARCODE_FONT_FAMILY,
					       GL_BARCODE_FONT_WEIGHT,
					       FALSE, 12.0);
		gnome_print_setfont (pi->pc, font);

		gnome_print_setrgbcolor (pi->pc,
					 RED (object->arg.barcode.color),
					 GREEN (object->arg.barcode.color),
					 BLUE (object->arg.barcode.color));
		gnome_print_setopacity (pi->pc,
					ALPHA (object->arg.barcode.color));

		y_offset = 12.0 - gnome_font_get_descender (font);
		gnome_print_moveto (pi->pc, x, y + y_offset);

		gnome_print_gsave (pi->pc);
		gnome_print_scale (pi->pc, 1.0, -1.0);
		gnome_print_show (pi->pc, _("Invalid barcode"));
		gnome_print_grestore (pi->pc);

	} else {

		for (li = gbc->lines; li != NULL; li = li->next) {
			line = (glBarcodeLine *) li->data;

			gnome_print_moveto (pi->pc, x + line->x, y + line->y);
			gnome_print_lineto (pi->pc, x + line->x,
					    y + line->y + line->length);
			gnome_print_setrgbcolor (pi->pc,
						 RED (object->arg.barcode.
						      color),
						 GREEN (object->arg.barcode.
							color),
						 BLUE (object->arg.barcode.
						       color));
			gnome_print_setopacity (pi->pc,
						ALPHA (object->arg.barcode.
						       color));
			gnome_print_setlinewidth (pi->pc, line->width);
			gnome_print_stroke (pi->pc);
		}

		for (li = gbc->chars; li != NULL; li = li->next) {
			bchar = (glBarcodeChar *) li->data;

			font = gnome_font_new_closest (GL_BARCODE_FONT_FAMILY,
						       GL_BARCODE_FONT_WEIGHT,
						       FALSE, bchar->fsize);
			gnome_print_setfont (pi->pc, font);

			gnome_print_setrgbcolor (pi->pc,
						 RED (object->arg.barcode.
						      color),
						 GREEN (object->arg.barcode.
							color),
						 BLUE (object->arg.barcode.
						       color));
			gnome_print_setopacity (pi->pc,
						ALPHA (object->arg.barcode.
						       color));

			y_offset =
			    bchar->y + bchar->fsize -
			    gnome_font_get_descender (font);
			gnome_print_moveto (pi->pc, x + bchar->x, y + y_offset);

			cstring = g_strdup_printf ("%c", bchar->c);
			gnome_print_gsave (pi->pc);
			gnome_print_scale (pi->pc, 1.0, -1.0);
			gnome_print_show (pi->pc, cstring);
			gnome_print_grestore (pi->pc);
			g_free (cstring);

		}

		gl_barcode_free (&gbc);

	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw outline.                                                   */
/*---------------------------------------------------------------------------*/
static void
draw_outline (PrintInfo * pi,
	      glLabel * label)
{
	gdouble w, h, r;
	gdouble r1, r2;

	gnome_print_setrgbcolor (pi->pc, 0.25, 0.25, 0.25);
	gnome_print_setopacity (pi->pc, 1.0);
	gnome_print_setlinewidth (pi->pc, 0.25);

	switch (label->template->style) {

	case GL_TEMPLATE_STYLE_RECT:
		w = label->width;
		h = label->height;
		r = label->template->label_round;
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
		r1 = label->template->label_radius;
		create_ellipse_path (pi->pc, r1, r1, r1, r1);
		gnome_print_stroke (pi->pc);
		break;

	case GL_TEMPLATE_STYLE_CD:
		/* CD style, round label w/ concentric round hole */
		r1 = label->template->label_radius;
		r2 = label->template->label_hole;
		create_ellipse_path (pi->pc, r1, r1, r1, r1);
		gnome_print_stroke (pi->pc);
		create_ellipse_path (pi->pc, r1, r1, r2, r2);
		gnome_print_stroke (pi->pc);
		break;

	default:
		WARN ("Unknown template label style");
		break;
	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Clip to outline.                                                */
/*---------------------------------------------------------------------------*/
static void
clip_to_outline (PrintInfo * pi,
		 glLabel * label)
{
	gdouble w, h, r;
	gdouble r1;

	switch (label->template->style) {

	case GL_TEMPLATE_STYLE_RECT:
		w = label->width;
		h = label->height;
		r = label->template->label_round;
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
	case GL_TEMPLATE_STYLE_CD:
		r1 = label->template->label_radius;
		create_ellipse_path (pi->pc, r1, r1, r1, r1);
		gnome_print_clip (pi->pc);
		break;

	default:
		WARN ("Unknown template label style");
		break;
	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Path creation utilities.                                        */
/*---------------------------------------------------------------------------*/
static void
create_rectangle_path (GnomePrintContext * pc,
		       gdouble x0,
		       gdouble y0,
		       gdouble w,
		       gdouble h)
{
	gnome_print_newpath (pc);
	gnome_print_moveto (pc, x0, y0);
	gnome_print_lineto (pc, x0 + w, y0);
	gnome_print_lineto (pc, x0 + w, y0 + h);
	gnome_print_lineto (pc, x0, y0 + h);
	gnome_print_lineto (pc, x0, y0);
	gnome_print_closepath (pc);
}

static void
create_ellipse_path (GnomePrintContext * pc,
		     gdouble x0,
		     gdouble y0,
		     gdouble rx,
		     gdouble ry)
{
	gdouble x, y;
	gint i_theta;

	gnome_print_newpath (pc);
	gnome_print_moveto (pc, x0 + rx, y0);
	for (i_theta = 2; i_theta <= 360; i_theta += 2) {
		x = x0 + rx * cos (i_theta * M_PI / 180.0);
		y = y0 + ry * sin (i_theta * M_PI / 180.0);
		gnome_print_lineto (pc, x, y);
	}
	gnome_print_closepath (pc);
}

static void
create_rounded_rectangle_path (GnomePrintContext * pc,
			       gdouble x0,
			       gdouble y0,
			       gdouble w,
			       gdouble h,
			       gdouble r)
{
	gdouble x, y;
	gint i_theta;

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
}
