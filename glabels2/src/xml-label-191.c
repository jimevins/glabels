/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label.c:  GLabels xml label 1.91 compat module
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

#include <config.h>

#include <gnome.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <gdk-pixbuf/gdk-pixdata.h>

#include "label.h"
#include "label-object.h"
#include "label-text.h"
#include "label-box.h"
#include "label-line.h"
#include "label-ellipse.h"
#include "label-image.h"
#include "label-barcode.h"
#include "base64.h"
#include "xml-label.h"
#include "xml-label-04.h"
#include <libglabels/paper.h>
#include <libglabels/template.h>
#include <libglabels/xml-template.h>
#include <libglabels/xml.h>
#include "util.h"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void           xml191_parse_objects        (xmlNodePtr        node,
						   glLabel          *label);

static void           xml191_parse_object         (xmlNodePtr        node,
						   glLabel          *label);

static glLabelObject *xml191_parse_text_props     (xmlNodePtr        node,
						   glLabel          *label);

static glLabelObject *xml191_parse_box_props      (xmlNodePtr        node,
						   glLabel          *label);

static glLabelObject *xml191_parse_line_props     (xmlNodePtr        node,
						   glLabel          *label);

static glLabelObject *xml191_parse_ellipse_props  (xmlNodePtr        node,
						   glLabel          *label);

static glLabelObject *xml191_parse_image_props    (xmlNodePtr        node,
						   glLabel          *label);

static glLabelObject *xml191_parse_barcode_props  (xmlNodePtr        node,
						   glLabel          *label);

static void           xml191_parse_merge_fields   (xmlNodePtr        node,
						   glLabel          *label);

static void           xml191_parse_data           (xmlNodePtr        node,
						   glLabel          *label);

static void           xml191_parse_pixdata        (xmlNodePtr        node,
						   glLabel          *label);

static glTemplate    *xml191_parse_sheet          (xmlNodePtr        node);

static void           xml191_parse_label          (xmlNodePtr        label_node,
						   glTemplate       *template);

static void           xml191_parse_layout         (xmlNodePtr           layout_node,
						   glTemplateLabelType *label_type);

static void           xml191_parse_markup         (xmlNodePtr           markup_node,
						   glTemplateLabelType *label_type);

static void           xml191_parse_alias          (xmlNodePtr        alias_node,
						   glTemplate       *template);



/****************************************************************************/
/* Parse xml root node and create label.                                    */
/****************************************************************************/
glLabel *
gl_xml_label_191_parse (xmlNodePtr        root,
			glXMLLabelStatus *status)
{
	xmlNodePtr  node;
	glLabel    *label;
	glTemplate *template;

	gl_debug (DEBUG_XML, "START");

	*status = XML_LABEL_OK;

	if (!xmlStrEqual (root->name, "Document")) {
		g_warning (_("Bad root node = \"%s\""), root->name);
		*status = XML_LABEL_ERROR_OPEN_PARSE;
		return NULL;
	}

	label = GL_LABEL(gl_label_new ());

	/* Pass 1, extract data nodes to pre-load cache. */
	for (node = root->xmlChildrenNode; node != NULL; node = node->next) {
		if (xmlStrEqual (node->name, "Data")) {
			xml191_parse_data (node, label);
		}
	}

	/* Pass 2, now extract everything else. */
	for (node = root->xmlChildrenNode; node != NULL; node = node->next) {

		if (xmlStrEqual (node->name, "Sheet")) {
			template = xml191_parse_sheet (node);
			if (!template) {
				*status = XML_LABEL_UNKNOWN_MEDIA;
				return NULL;
			}
			gl_template_register (template);
			gl_label_set_template (label, template);
			gl_template_free (template);
		} else if (xmlStrEqual (node->name, "Objects")) {
			xml191_parse_objects (node, label);
		} else if (xmlStrEqual (node->name, "Merge_Fields")) {
			xml191_parse_merge_fields (node, label);
		} else if (xmlStrEqual (node->name, "Data")) {
			/* Handled in pass 1. */
		} else {
			if (!xmlNodeIsText (node)) {
				g_warning (_("bad node in Document node =  \"%s\""),
					   node->name);
			}
		}
	}

	gl_debug (DEBUG_XML, "END");

	return label;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse Objects node.                                            */
/*--------------------------------------------------------------------------*/
static void
xml191_parse_objects (xmlNodePtr  objects_node,
		      glLabel    *label)
{
	gboolean    rotate_flag;
	xmlNodePtr  node;

	gl_debug (DEBUG_XML, "START");

	rotate_flag = gl_xml_get_prop_boolean (objects_node, "rotate", FALSE);
	gl_label_set_rotate_flag (label, rotate_flag);

	for (node = objects_node->xmlChildrenNode; node != NULL; node = node->next) {

		if (xmlStrEqual (node->name, "Object")) {
			xml191_parse_object (node, label);
		} else {
			if (!xmlNodeIsText (node)) {
				g_warning (_("bad node =  \"%s\""), node->name);
			}
		}
	}

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Object Node                                          */
/*--------------------------------------------------------------------------*/
static void
xml191_parse_object (xmlNodePtr  object_node,
		     glLabel    *label)
{
	glLabelObject *object;
	gdouble        x, y;
	gchar         *string;
	gdouble        affine[6];

	gl_debug (DEBUG_XML, "START");

	string = xmlGetProp (object_node, "type");

	if ( xmlStrEqual (string, "Text") ) {
		object = xml191_parse_text_props (object_node, label);
	} else if ( xmlStrEqual (string, "Box") ) {
		object = xml191_parse_box_props (object_node, label);
	} else if ( xmlStrEqual (string, "Line") ) {
		object = xml191_parse_line_props (object_node, label);
	} else if ( xmlStrEqual (string, "Ellipse") ) {
		object = xml191_parse_ellipse_props (object_node, label);
	} else if ( xmlStrEqual (string, "Image") ) {
		object = xml191_parse_image_props (object_node, label);
	} else if ( xmlStrEqual (string, "Barcode") ) {
		object = xml191_parse_barcode_props (object_node, label);
	} else {
		g_warning ("Unknown label object type \"%s\"", string);
		g_free (string);
		return;
	}
	g_free (string);


	x = gl_xml_get_prop_length (object_node, "x", 0.0);
	y = gl_xml_get_prop_length (object_node, "y", 0.0);

	gl_label_object_set_position (object, x, y);


	affine[0] = gl_xml_get_prop_double (object_node, "a0", 0.0);
	affine[1] = gl_xml_get_prop_double (object_node, "a1", 0.0);
	affine[2] = gl_xml_get_prop_double (object_node, "a2", 0.0);
	affine[3] = gl_xml_get_prop_double (object_node, "a3", 0.0);
	affine[4] = gl_xml_get_prop_double (object_node, "a4", 0.0);
	affine[5] = gl_xml_get_prop_double (object_node, "a5", 0.0);

	gl_label_object_set_affine (object, affine);


	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Text Node Properties                          */
/*--------------------------------------------------------------------------*/
static glLabelObject *
xml191_parse_text_props (xmlNodePtr  object_node,
			 glLabel    *label)
{
	GObject          *object;
	GList            *lines;
	gdouble           w, h;
	gchar            *font_family;
	gdouble           font_size;
	GnomeFontWeight   font_weight;
	gboolean          font_italic_flag;
	guint             color;
	GtkJustification  just;
	xmlNodePtr        line_node, text_node;
	glTextNode       *node_text;
	GList            *nodes;
	gchar            *string;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_text_new (label);

	w = gl_xml_get_prop_length (object_node, "w", 0);
	h = gl_xml_get_prop_length (object_node, "h", 0);

	font_family = xmlGetProp (object_node, "font_family");

	font_size = gl_xml_get_prop_double (object_node, "font_size", 0.0);

	string = xmlGetProp (object_node, "font_weight");
	font_weight = gl_util_string_to_weight (string);
	g_free (string);

	font_italic_flag = gl_xml_get_prop_boolean (object_node, "font_italic", FALSE);

	string = xmlGetProp (object_node, "justify");
	just = gl_util_string_to_just (string);
	g_free (string);

	color = gl_xml_get_prop_uint (object_node, "color", 0);

	lines = NULL;
	for (line_node = object_node->xmlChildrenNode;
	     line_node != NULL;
	     line_node = line_node->next) {

		if (xmlStrEqual (line_node->name, "Line")) {

			nodes = NULL;
			for (text_node = line_node->xmlChildrenNode;
			     text_node != NULL; text_node = text_node->next) {

				if (xmlStrEqual (text_node->name, "Field")) {
					node_text = g_new0 (glTextNode, 1);
					node_text->field_flag = TRUE;
					node_text->data = xmlGetProp (text_node, "name");
					nodes =	g_list_append (nodes, node_text);
				} else if (xmlStrEqual (text_node->name, "Literal")) {
					node_text = g_new0 (glTextNode, 1);
					node_text->field_flag = FALSE;
					node_text->data =
						xmlNodeGetContent (text_node);
					nodes =
						g_list_append (nodes, node_text);
				} else if (!xmlNodeIsText (text_node)) {
					g_warning ("Unexpected Text Line child: \"%s\"",
						   text_node->name);
				}

			}
			lines = g_list_append (lines, nodes);

		} else if (!xmlNodeIsText (line_node)) {
			g_warning ("Unexpected Text child: \"%s\"",
				   line_node->name);
		}

	}

	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);
	gl_label_text_set_lines  (GL_LABEL_TEXT(object), lines);
	gl_label_object_set_font_family (GL_LABEL_OBJECT(object), font_family);
	gl_label_object_set_font_size (GL_LABEL_OBJECT(object), font_size);
	gl_label_object_set_font_weight (GL_LABEL_OBJECT(object), font_weight);
	gl_label_object_set_font_italic_flag (GL_LABEL_OBJECT(object), font_italic_flag);
	gl_label_object_set_text_color (GL_LABEL_OBJECT(object), color);
	gl_label_object_set_text_alignment (GL_LABEL_OBJECT(object), just);

	gl_text_node_lines_free (&lines);
	g_free (font_family);

	gl_debug (DEBUG_XML, "END");

	return GL_LABEL_OBJECT(object);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Box Node Properties                           */
/*--------------------------------------------------------------------------*/
static glLabelObject *
xml191_parse_box_props (xmlNodePtr  node,
			glLabel    *label)
{
	GObject *object;
	gdouble  line_width;
	guint    line_color, fill_color;
	gdouble  w, h;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_box_new (label);

	w = gl_xml_get_prop_length (node, "w", 0);
	h = gl_xml_get_prop_length (node, "h", 0);

	line_width = gl_xml_get_prop_length (node, "line_width", 1.0);

	line_color = gl_xml_get_prop_uint (node, "line_color", 0);
	fill_color = gl_xml_get_prop_uint (node, "fill_color", 0);

	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);
	gl_label_object_set_line_width (GL_LABEL_OBJECT(object), line_width);
	gl_label_object_set_line_color (GL_LABEL_OBJECT(object), line_color);
	gl_label_object_set_fill_color (GL_LABEL_OBJECT(object), fill_color);

	gl_debug (DEBUG_XML, "END");

	return GL_LABEL_OBJECT(object);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Line Node Properties                          */
/*--------------------------------------------------------------------------*/
static glLabelObject *
xml191_parse_line_props (xmlNodePtr  node,
			 glLabel    *label)
{
	GObject *object;
	gdouble  line_width;
	guint    line_color;
	gdouble  w, h;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_line_new (label);

	w = gl_xml_get_prop_length (node, "dx", 0);
	h = gl_xml_get_prop_length (node, "dy", 0);

	line_width = gl_xml_get_prop_length (node, "line_width", 1.0);

	line_color = gl_xml_get_prop_uint (node, "line_color", 0);

	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);
	gl_label_object_set_line_width (GL_LABEL_OBJECT(object), line_width);
	gl_label_object_set_line_color (GL_LABEL_OBJECT(object), line_color);

	gl_debug (DEBUG_XML, "END");

	return GL_LABEL_OBJECT(object);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Ellipse Node Properties                       */
/*--------------------------------------------------------------------------*/
static glLabelObject *
xml191_parse_ellipse_props (xmlNodePtr  node,
			    glLabel    *label)
{
	GObject *object;
	gdouble  line_width;
	guint    line_color, fill_color;
	gdouble  w, h;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_ellipse_new (label);

	w = gl_xml_get_prop_length (node, "w", 0);
	h = gl_xml_get_prop_length (node, "h", 0);

	line_width = gl_xml_get_prop_length (node, "line_width", 1.0);

	line_color = gl_xml_get_prop_uint (node, "line_color", 0);
	fill_color = gl_xml_get_prop_uint (node, "fill_color", 0);

	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);
	gl_label_object_set_line_width (GL_LABEL_OBJECT(object), line_width);
	gl_label_object_set_line_color (GL_LABEL_OBJECT(object), line_color);
	gl_label_object_set_fill_color (GL_LABEL_OBJECT(object), fill_color);

	gl_debug (DEBUG_XML, "END");

	return GL_LABEL_OBJECT(object);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Image Node Properties                         */
/*--------------------------------------------------------------------------*/
static glLabelObject *
xml191_parse_image_props (xmlNodePtr  node,
			  glLabel    *label)
{
	GObject      *object;
	xmlNodePtr    child;
	gdouble       w, h;
	glTextNode   *filename;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_image_new (label);

	w = gl_xml_get_prop_length (node, "w", 0);
	h = gl_xml_get_prop_length (node, "h", 0);

	filename = g_new0 (glTextNode, 1);
	for (child = node->xmlChildrenNode; child != NULL; child = child->next) {
		if (xmlStrEqual (child->name, "Field")) {
			filename->field_flag = TRUE;
			filename->data = xmlGetProp (child, "name");
		} else if (xmlStrEqual (child->name, "File")) {
			filename->field_flag = FALSE;
			filename->data = xmlGetProp (child, "src");
		} else if (!xmlNodeIsText (child)) {
			g_warning ("Unexpected Image child: \"%s\"", child->name);
		}
	}

	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);
	gl_label_image_set_filename (GL_LABEL_IMAGE(object), filename);

	gl_text_node_free (&filename);

	gl_debug (DEBUG_XML, "END");

	return GL_LABEL_OBJECT(object);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Barcode Node Properties                       */
/*--------------------------------------------------------------------------*/
static glLabelObject *
xml191_parse_barcode_props (xmlNodePtr  node,
			    glLabel    *label)
{
	GObject            *object;
	xmlNodePtr          child;
	gdouble             w, h;
	gchar              *string;
	glTextNode         *text_node;
	gchar              *id;
	gboolean            text_flag;
	gboolean            checksum_flag;
	guint               color;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_barcode_new (label);

	w = gl_xml_get_prop_length (node, "w", 0);
	h = gl_xml_get_prop_length (node, "h", 0);

	color = gl_xml_get_prop_uint (node, "color", 0);

	id = xmlGetProp (node, "style");

	text_flag = gl_xml_get_prop_boolean (node, "text", FALSE);
	checksum_flag = gl_xml_get_prop_boolean (node, "checksum", TRUE);

	text_node = g_new0 (glTextNode, 1);
	for (child = node->xmlChildrenNode; child != NULL; child = child->next) {
		if (xmlStrEqual (child->name, "Field")) {
			text_node->field_flag = TRUE;
			text_node->data = xmlGetProp (child, "name");
		} else if (xmlStrEqual (child->name, "Literal")) {
			text_node->field_flag = FALSE;
			text_node->data = xmlNodeGetContent (child);
		} else if (!xmlNodeIsText (child)) {
			g_warning ("Unexpected Barcode child: \"%s\"", child->name);
		}
	}

	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);

	gl_label_barcode_set_data (GL_LABEL_BARCODE(object), text_node);
	gl_label_barcode_set_props (GL_LABEL_BARCODE(object),
				    id, text_flag, checksum_flag);
	gl_label_object_set_line_color (GL_LABEL_OBJECT(object), color);

	gl_text_node_free (&text_node);
	g_free (id);

	gl_debug (DEBUG_XML, "END");

	return GL_LABEL_OBJECT(object);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML merge fields tag.                                */
/*--------------------------------------------------------------------------*/
static void
xml191_parse_merge_fields (xmlNodePtr  node,
			   glLabel    *label)
{
	xmlNodePtr  child;
	gchar      *string;
	glMerge    *merge;

	gl_debug (DEBUG_XML, "START");

	string = xmlGetProp (node, "type");
	merge = gl_merge_new (string);
	g_free (string);

	string = xmlGetProp (node, "src");
	gl_merge_set_src (merge, string);
	g_free (string);

	gl_label_set_merge (label, merge);

	g_object_unref (G_OBJECT(merge));

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML data tag.                                            */
/*--------------------------------------------------------------------------*/
static void
xml191_parse_data (xmlNodePtr  node,
		   glLabel    *label)
{
	xmlNodePtr  child;

	gl_debug (DEBUG_XML, "START");

	for (child = node->xmlChildrenNode; child != NULL; child = child->next) {

		if (xmlStrEqual (child->name, "Pixdata")) {
			xml191_parse_pixdata (child, label);
		} else {
			if (!xmlNodeIsText (child)) {
				g_warning (_("bad node in Data node =  \"%s\""),
					   child->name);
			}
		}
	}

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML pixbuf data tag.                                     */
/*--------------------------------------------------------------------------*/
static void
xml191_parse_pixdata (xmlNodePtr  node,
		      glLabel    *label)
{
	gchar      *name, *base64;
	guchar     *stream;
	guint       stream_length;
	gboolean    ret;
	GdkPixdata *pixdata;
	GdkPixbuf  *pixbuf;
	GHashTable *pixbuf_cache;

	gl_debug (DEBUG_XML, "START");

	name = xmlGetProp (node, "name");
	base64 = xmlNodeGetContent (node);

	stream = gl_base64_decode (base64, &stream_length);
	pixdata = g_new0 (GdkPixdata, 1);
	ret = gdk_pixdata_deserialize (pixdata, stream_length, stream, NULL);

	if (ret) {
		pixbuf = gdk_pixbuf_from_pixdata (pixdata, TRUE, NULL);

		pixbuf_cache = gl_label_get_pixbuf_cache (label);
		gl_pixbuf_cache_add_pixbuf (pixbuf_cache, name, pixbuf);
	}

	g_free (name);
	g_free (base64);
	g_free (stream);
	g_free (pixdata);

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML template Node.                                       */
/*--------------------------------------------------------------------------*/
static glTemplate *
xml191_parse_sheet (xmlNodePtr sheet_node)
{
	gchar                 *name, *description, *page_size;
	gdouble                page_width, page_height;
	glTemplate            *template;
	xmlNodePtr             node;
	glPaper               *paper;

	gl_debug (DEBUG_TEMPLATE, "START");

	name  = xmlGetProp (sheet_node, "name");

	page_size = xmlGetProp (sheet_node, "size");
	if (gl_paper_is_id_other (page_size)) {

		page_width = gl_xml_get_prop_length (sheet_node, "width", 0);
		page_height = gl_xml_get_prop_length (sheet_node, "height", 0);

	} else {
		paper = gl_paper_from_id (template->page_size);
		if (paper == NULL) {
			/* This should always be an id, but just in case a name
			   slips by! */
			g_warning (_("Unknown page size id \"%s\", trying as name"),
				   template->page_size);
			paper = gl_paper_from_name (template->page_size);
			g_free (template->page_size);
			template->page_size = g_strdup (paper->id);
		}
		if (paper != NULL) {
			page_width  = paper->width;
			page_height = paper->height;
		} else {
			g_warning (_("Unknown page size id or name \"%s\""),
				   page_size);
		}
		gl_paper_free (paper);
	}

	description = xmlGetProp (sheet_node, "_description");
	if (description != NULL) {
		template->description = gettext (description);
	} else {
		template->description = xmlGetProp (sheet_node, "description");
	}

	template = gl_template_new (name, description,
				    page_size, page_width, page_height);

	for (node = sheet_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (xmlStrEqual (node->name, "Label")) {
			xml191_parse_label (node, template);
		} else if (xmlStrEqual (node->name, "Alias")) {
			xml191_parse_alias (node, template);
		} else {
			if (!xmlNodeIsText (node)) {
				g_warning ("bad node =  \"%s\"", node->name);
			}
		}
	}

	g_free (name);
	g_free (description);
	g_free (page_size);

	gl_debug (DEBUG_TEMPLATE, "END");

	return template;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Sheet->Label Node.                                   */
/*--------------------------------------------------------------------------*/
static void
xml191_parse_label (xmlNodePtr  label_node,
		    glTemplate *template)
{
	gchar                *style;
	glTemplateLabelShape  shape;
	gdouble               w, h, r, r1, r2;
	glTemplateLabelType  *label_type;
	xmlNodePtr            node;

	gl_debug (DEBUG_TEMPLATE, "START");

	style = xmlGetProp (label_node, "style");
	if (xmlStrEqual (style, "rectangle")) {
		shape = GL_TEMPLATE_SHAPE_RECT;
	} else if (xmlStrEqual (style, "round")) {
		shape = GL_TEMPLATE_SHAPE_ROUND;
	} else if (xmlStrEqual (style, "cd")) {
		shape = GL_TEMPLATE_SHAPE_CD;
	} else {
		shape = GL_TEMPLATE_SHAPE_RECT;
		g_warning ("Unknown label style in template");
	}
	g_free (style);

	switch (shape) {

	case GL_TEMPLATE_SHAPE_RECT:
		w = gl_xml_get_prop_length (label_node, "width", 0);
		h = gl_xml_get_prop_length (label_node, "height", 0);
		r = gl_xml_get_prop_length (label_node, "round", 0);
		label_type =
			gl_template_rect_label_type_new ("0", w, h, r, 0.0);
		break;

	case GL_TEMPLATE_SHAPE_ROUND:
		r = gl_xml_get_prop_length (label_node, "round", 0);
		label_type =
			gl_template_round_label_type_new ("0", r, 0.0);
		break;

	case GL_TEMPLATE_SHAPE_CD:
		r1 = gl_xml_get_prop_length (label_node, "radius", 0);
		r2 = gl_xml_get_prop_length (label_node, "hole", 0);
		w  = gl_xml_get_prop_length (label_node, "width", 0);
		h  = gl_xml_get_prop_length (label_node, "height", 0);
		label_type =
			gl_template_cd_label_type_new ("0", r1, r2, w, h, 0.0);
		break;

	default:
		break;

	}

	gl_template_add_label_type (template, label_type);

	for (node = label_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (xmlStrEqual (node->name, "Layout")) {
			xml191_parse_layout (node, label_type);
		} else if (xmlStrEqual (node->name, "Markup")) {
			xml191_parse_markup (node, label_type);
		} else if (!xmlNodeIsText (node)) {
			g_warning ("bad node =  \"%s\"", node->name);
		}
	}

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Sheet->Label->Layout Node.                           */
/*--------------------------------------------------------------------------*/
static void
xml191_parse_layout (xmlNodePtr           layout_node,
		     glTemplateLabelType *label_type)
{
	gint        nx, ny;
	gdouble     x0, y0, dx, dy;
	xmlNodePtr  node;

	gl_debug (DEBUG_TEMPLATE, "START");

	nx = gl_xml_get_prop_int (layout_node, "nx", 1);
	ny = gl_xml_get_prop_int (layout_node, "ny", 1);

	x0 = gl_xml_get_prop_length (layout_node, "x0", 0);
	y0 = gl_xml_get_prop_length (layout_node, "y0", 0);

	dx = gl_xml_get_prop_length (layout_node, "dx", 0);
	dy = gl_xml_get_prop_length (layout_node, "dy", 0);

	for (node = layout_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (!xmlNodeIsText (node)) {
			g_warning ("bad node =  \"%s\"", node->name);
		}
	}

	gl_template_add_layout (label_type,
				gl_template_layout_new (nx, ny, x0, y0, dx, dy));

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Sheet->Label->Markup Node.                           */
/*--------------------------------------------------------------------------*/
static void
xml191_parse_markup (xmlNodePtr  markup_node,
		     glTemplateLabelType *label_type)
{
	gchar      *type;
	gdouble     size;
	gdouble     x1, y1, x2, y2;
	xmlNodePtr  node;

	gl_debug (DEBUG_TEMPLATE, "START");

	type = xmlGetProp (markup_node, "type");
	if (xmlStrEqual (type, "margin")) {

		size = gl_xml_get_prop_length (markup_node, "size", 0);

		gl_template_add_markup (label_type,
					gl_template_markup_margin_new (size));

	} else if (xmlStrEqual (type, "line")) {

		x1 = gl_xml_get_prop_length (markup_node, "x1", 0);
		y1 = gl_xml_get_prop_length (markup_node, "y1", 0);
		x2 = gl_xml_get_prop_length (markup_node, "x2", 0);
		y2 = gl_xml_get_prop_length (markup_node, "y2", 0);

		gl_template_add_markup (label_type,
					gl_template_markup_line_new (x1, y1, x2, y2));
	}
	g_free (type);

	for (node = markup_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (!xmlNodeIsText (node)) {
			g_warning ("bad node =  \"%s\"", node->name);
		}
	}

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Sheet->Alias Node.                                   */
/*--------------------------------------------------------------------------*/
static void
xml191_parse_alias (xmlNodePtr  alias_node,
		    glTemplate *template)
{
	gchar       *name;

	gl_debug (DEBUG_TEMPLATE, "START");

	name = xmlGetProp (alias_node, "name");
	gl_template_add_alias (template, name);
	g_free (name);

	gl_debug (DEBUG_TEMPLATE, "END");
}

