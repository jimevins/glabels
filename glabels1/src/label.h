/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label.h:  GLabels label module header file
 *
 *  Copyright (C) 2001-2002  Jim Evins <evins@snaught.com>.
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
#ifndef __LABEL_H__
#define __LABEL_H__

#include <gtk/gtk.h>
#include <libgnomeprint/gnome-font.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "merge.h"
#include "text_node.h"
#include "template.h"
#include "bc.h"

typedef struct {

	gchar *template_name;
	glTemplate *template;
	gboolean rotate_flag;

	gdouble width, height;

	GList *objects;

	glMergeType merge_type;
	gchar *merge_src;
	GList *merge_fields;	/* Merge Field Definitions */

} glLabel;

typedef enum {
	GL_LABEL_OBJECT_TEXT,
	GL_LABEL_OBJECT_BOX,
	GL_LABEL_OBJECT_LINE,
	GL_LABEL_OBJECT_ELLIPSE,
	GL_LABEL_OBJECT_IMAGE,
	GL_LABEL_OBJECT_BARCODE,
	GL_LABEL_OBJECT_N_TYPES
} glLabelObjectType;

typedef struct {

	glLabel *parent;

	glLabelObjectType type;
	gdouble x, y;

	union {

		struct {
			GList *lines;	/* list of glLabelTextNode lists */
			gchar *font_family;
			gdouble font_size;
			GnomeFontWeight font_weight;
			gboolean font_italic_flag;
			GtkJustification just;
			guint color;
		} text;

		struct {
			gdouble w, h;
			gdouble line_width;
			guint line_color;
			guint fill_color;
		} box;

		struct {
			gdouble dx, dy;
			gdouble line_width;
			guint line_color;
		} line;

		struct {
			gdouble w, h;
			gdouble line_width;
			guint line_color;
			guint fill_color;
		} ellipse;

		struct {
			gchar *filename;
			gdouble w, h;
			GdkPixbuf *image;
		} image;

		struct {
			glTextNode *text_node;
			glBarcodeStyle style;
			guint color;
			gboolean text_flag;
			gdouble scale;
		} barcode;

	} arg;

} glLabelObject;

typedef enum {
	LABEL_UNKNOWN_MEDIA = 1,
	LABEL_OK = 0,
	LABEL_ERROR_OPEN_XML_PARSE = -10,
	LABEL_ERROR_SAVE_XML_FILE = -20,
} glLabelStatus;

extern glLabel *gl_label_new (void);
extern glLabel *gl_label_new_with_template (const gchar * tmplt_name,
					    gboolean rotate_flag);
extern void gl_label_free (glLabel ** label);

extern glLabelStatus gl_label_open_xml (glLabel ** label,
					const gchar * filename);
extern glLabelStatus gl_label_open_xml_buffer (glLabel ** label,
					       const gchar * buffer);

extern glLabelStatus gl_label_save_xml (glLabel * label,
					const gchar * filename);

glLabelStatus gl_label_save_xml_buffer (glLabel * label,
					gchar ** buffer);

extern glLabelObject *gl_label_object_new (glLabel * label,
					   glLabelObjectType type);

extern glLabelObject *gl_label_object_new_from_object (glLabel * label,
						       glLabelObject * object);

extern void gl_label_object_free (glLabelObject ** object);

extern void gl_label_object_raise_to_front (glLabelObject * object);
extern void gl_label_object_lower_to_back (glLabelObject * object);

#endif
