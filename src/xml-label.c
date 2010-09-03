/*
 *  xml-label.c
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

#include "xml-label.h"

#include <glib/gi18n.h>
#include <glib.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xinclude.h>
#include <gdk-pixbuf/gdk-pixdata.h>

#include <libglabels.h>
#include "label.h"
#include "label-object.h"
#include "label-text.h"
#include "label-box.h"
#include "label-line.h"
#include "label-ellipse.h"
#include "label-image.h"
#include "label-barcode.h"
#include "bc-backends.h"
#include "xml-label-04.h"
#include "str-util.h"
#include "prefs.h"

#include "debug.h"


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/
#define COMPAT01_NAME_SPACE "http://snaught.com/glabels/0.1/"
#define COMPAT04_NAME_SPACE "http://snaught.com/glabels/0.4/"
#define COMPAT20_NAME_SPACE "http://snaught.com/glabels/2.0/"
#define COMPAT22_NAME_SPACE "http://snaught.com/glabels/2.2/"


/*========================================================*/
/* Private types.                                         */
/*========================================================*/


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static glLabel       *xml_doc_to_label         (xmlDocPtr         doc,
						glXMLLabelStatus *status);

static glLabel       *xml_parse_label          (xmlNodePtr        root,
						glXMLLabelStatus *status);

static void           xml_parse_objects        (xmlNodePtr        node,
						glLabel          *label);

static void           xml_parse_object_text    (xmlNodePtr        node,
						glLabel          *label);

static void           xml_parse_object_box     (xmlNodePtr        node,
						glLabel          *label);

static void           xml_parse_object_ellipse (xmlNodePtr        node,
						glLabel          *label);

static void           xml_parse_object_line    (xmlNodePtr        node,
						glLabel          *label);

static void           xml_parse_object_image   (xmlNodePtr        node,
					        glLabel          *label);

static void           xml_parse_object_barcode (xmlNodePtr        node,
						glLabel          *label);

static void           xml_parse_merge_fields   (xmlNodePtr        node,
						glLabel          *label);

static void           xml_parse_data           (xmlNodePtr        node,
						glLabel          *label);

static void           xml_parse_pixdata        (xmlNodePtr        node,
						glLabel          *label);

static void           xml_parse_file_node      (xmlNodePtr        node,
						glLabel          *label);

static void           xml_parse_toplevel_span  (xmlNodePtr        node,
						glLabelObject    *object);

static void           xml_parse_affine_attrs   (xmlNodePtr        node,
						glLabelObject    *object);

static void           xml_parse_shadow_attrs   (xmlNodePtr        node,
						glLabelObject    *object);

static xmlDocPtr      xml_label_to_doc         (glLabel          *label,
						glXMLLabelStatus *status);

static void           xml_create_objects       (xmlNodePtr        parent,
						xmlNsPtr          ns,
						glLabel          *label);

static void           xml_create_object_text   (xmlNodePtr        parent,
						xmlNsPtr          ns,
						glLabelObject    *object);

static void           xml_create_object_box    (xmlNodePtr        parent,
						xmlNsPtr          ns,
						glLabelObject    *object);

static void           xml_create_object_line   (xmlNodePtr        parent,
						xmlNsPtr          ns,
						glLabelObject    *object);

static void           xml_create_object_ellipse(xmlNodePtr        parent,
						xmlNsPtr          ns,
						glLabelObject    *object);

static void           xml_create_object_image  (xmlNodePtr        parent,
						xmlNsPtr          ns,
						glLabelObject    *object);

static void           xml_create_object_barcode(xmlNodePtr        parent,
						xmlNsPtr          ns,
						glLabelObject    *object);

static void           xml_create_merge_fields  (xmlNodePtr        parent,
						xmlNsPtr          ns,
						glLabel          *label);

static void           xml_create_data          (xmlDocPtr         doc,
                                                xmlNodePtr        parent,
						xmlNsPtr          ns,
						glLabel          *label);

static void           xml_create_pixdata       (xmlNodePtr        parent,
						xmlNsPtr          ns,
						glLabel          *label,
						gchar            *name);

static void           xml_create_file_svg      (xmlDocPtr         doc,
                                                xmlNodePtr        parent,
						xmlNsPtr          ns,
						glLabel          *label,
						gchar            *name);

static void           xml_create_toplevel_span (xmlNodePtr        node,
						xmlNsPtr          ns,
						glLabelText      *object_text);

static void           xml_create_affine_attrs  (xmlNodePtr        node,
						glLabelObject    *object);

static void           xml_create_shadow_attrs  (xmlNodePtr        node,
						glLabelObject    *object);


/****************************************************************************/
/* Open and read label from xml file.                                       */
/****************************************************************************/
glLabel *
gl_xml_label_open (const gchar      *utf8_filename,
		   glXMLLabelStatus *status)
{
	xmlDocPtr  doc;
	glLabel   *label;
	gchar 	  *filename;

	gl_debug (DEBUG_XML, "START");

	filename = g_filename_from_utf8 (utf8_filename, -1, NULL, NULL, NULL);
	g_return_val_if_fail (filename, NULL);

        doc = xmlReadFile (filename, NULL, XML_PARSE_HUGE);
	if (!doc) {
		g_message (_("xmlParseFile error"));
		*status = XML_LABEL_ERROR_OPEN_PARSE;
		return NULL;
	}

	xmlXIncludeProcess (doc);
	xmlReconciliateNs (doc, xmlDocGetRootElement (doc));

	label = xml_doc_to_label (doc, status);

	xmlFreeDoc (doc);

	if (label) {
		gl_label_set_filename (label, utf8_filename);
		gl_label_clear_modified (label);
	}

	g_free (filename);
	gl_debug (DEBUG_XML, "END");

	return label;
}


/****************************************************************************/
/* Read label from xml buffer.                                              */
/****************************************************************************/
glLabel *
gl_xml_label_open_buffer (const gchar      *buffer,
			  glXMLLabelStatus *status)
{
	xmlDocPtr  doc;
	glLabel   *label;

	gl_debug (DEBUG_XML, "START");

        doc = xmlReadDoc ((xmlChar *) buffer, NULL, NULL, XML_PARSE_HUGE);
	if (!doc) {
		g_message (_("xmlParseFile error"));
		*status = XML_LABEL_ERROR_OPEN_PARSE;
		return NULL;
	}

	label = xml_doc_to_label (doc, status);

	xmlFreeDoc (doc);

	if (label) {
		gl_label_clear_modified (label);
	}

	gl_debug (DEBUG_XML, "END");

	return label;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse xml doc structure and create label.                      */
/*--------------------------------------------------------------------------*/
static glLabel *
xml_doc_to_label (xmlDocPtr         doc,
		  glXMLLabelStatus *status)
{
	xmlNodePtr  root;
	glLabel    *label;

	gl_debug (DEBUG_XML, "START");

	LIBXML_TEST_VERSION;

	*status = XML_LABEL_OK;

	root = xmlDocGetRootElement (doc);
	if (!root || !root->name) {
		g_message (_("No document root"));
		*status = XML_LABEL_ERROR_OPEN_PARSE;
		return NULL;
	}

        /* Try compatability mode 0.1 */
        if (xmlSearchNsByHref (doc, root, (xmlChar *)COMPAT01_NAME_SPACE))
	{
                g_message (_("Importing from glabels 0.1 format"));
                g_message ("TODO");
                label = NULL; /* TODO */
                return label;
        }

        /* Try compatability mode 0.4 */
        if (xmlSearchNsByHref (doc, root, (xmlChar *)COMPAT04_NAME_SPACE))
	{
                g_message (_("Importing from glabels 0.4 format"));
                label = gl_xml_label_04_parse (root, status);
                return label;
        }

        /* Test for current namespaces. */
        if ( !xmlSearchNsByHref (doc, root, (xmlChar *)COMPAT20_NAME_SPACE) &&
             !xmlSearchNsByHref (doc, root, (xmlChar *)COMPAT22_NAME_SPACE) &&
             !xmlSearchNsByHref (doc, root, (xmlChar *)LGL_XML_NAME_SPACE) )
        {
                g_message (_("Unknown glabels Namespace -- Using %s"),
                           LGL_XML_NAME_SPACE);
        }

        label = xml_parse_label (root, status);
        if (label)
        {
                gl_label_set_compression (label, xmlGetDocCompressMode (doc));
        }

	gl_debug (DEBUG_XML, "END");

	return label;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse xml root node and create label.                          */
/*--------------------------------------------------------------------------*/
static glLabel *
xml_parse_label (xmlNodePtr        root,
		 glXMLLabelStatus *status)
{
	xmlNodePtr   child_node;
	glLabel     *label;
	lglTemplate *template;

	gl_debug (DEBUG_XML, "START");

	*status = XML_LABEL_OK;

	if (!lgl_xml_is_node (root, "Glabels-document")) {
		g_message (_("Bad root node = \"%s\""), root->name);
		*status = XML_LABEL_ERROR_OPEN_PARSE;
		return NULL;
	}

	label = GL_LABEL(gl_label_new ());

	/* Pass 1, extract data nodes to pre-load cache. */
	for (child_node = root->xmlChildrenNode; child_node != NULL; child_node = child_node->next) {
		if (lgl_xml_is_node (child_node, "Data")) {
			xml_parse_data (child_node, label);
		}
	}

	/* Pass 2, now extract everything else. */
	for (child_node = root->xmlChildrenNode;
             child_node != NULL;
	     child_node = child_node->next) {

		if (lgl_xml_is_node (child_node, "Template")) {
			template = lgl_xml_template_parse_template_node (child_node);
			if (!template) {
				g_object_unref (label);
				*status = XML_LABEL_UNKNOWN_MEDIA;
				return NULL;
			}
			lgl_db_register_template (template);
			gl_label_set_template (label, template, FALSE);
			lgl_template_free (template);
		} else if (lgl_xml_is_node (child_node, "Objects")) {
			xml_parse_objects (child_node, label);
		} else if (lgl_xml_is_node (child_node, "Merge")) {
			xml_parse_merge_fields (child_node, label);
		} else if (lgl_xml_is_node (child_node, "Data")) {
			/* Handled in pass 1. */
		} else {
			if (!xmlNodeIsText (child_node)) {
				g_message (_("bad node in Document node =  \"%s\""),
					   child_node->name);
				g_object_unref (label);
				*status = XML_LABEL_ERROR_OPEN_PARSE;
				return NULL;
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
xml_parse_objects (xmlNodePtr  node,
		   glLabel    *label)
{
	gboolean    rotate_flag;
	xmlNodePtr  child;

	gl_debug (DEBUG_XML, "START");

	rotate_flag = lgl_xml_get_prop_boolean (node, "rotate", FALSE);
	gl_label_set_rotate_flag (label, rotate_flag, FALSE);

	for (child = node->xmlChildrenNode; child != NULL; child = child->next) {

		if (lgl_xml_is_node (child, "Object-text")) {
			xml_parse_object_text (child, label);
		} else if (lgl_xml_is_node (child, "Object-box")) {
			xml_parse_object_box (child, label);
		} else if (lgl_xml_is_node (child, "Object-ellipse")) {
			xml_parse_object_ellipse (child, label);
		} else if (lgl_xml_is_node (child, "Object-line")) {
			xml_parse_object_line (child, label);
		} else if (lgl_xml_is_node (child, "Object-image")) {
			xml_parse_object_image (child, label);
		} else if (lgl_xml_is_node (child, "Object-barcode")) {
			xml_parse_object_barcode (child, label);
		} else {
			if (!xmlNodeIsText (child)) {
				g_message (_("bad node =  \"%s\""), child->name);
				break;
			}
		}
	}

	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Objects->Object-text Node                            */
/*--------------------------------------------------------------------------*/
static void
xml_parse_object_text (xmlNodePtr  node,
		       glLabel    *label)
{
	GObject          *object;
	gdouble           x, y;
	gdouble           w, h;
	gchar            *string;
	PangoAlignment    align;
	gboolean          auto_shrink;
	xmlNodePtr        child;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_text_new (label, FALSE);

	/* position attrs */
	x = lgl_xml_get_prop_length (node, "x", 0.0);
	y = lgl_xml_get_prop_length (node, "y", 0.0);
	gl_label_object_set_position (GL_LABEL_OBJECT(object), x, y, FALSE);

	/* implied size attrs */
	w = lgl_xml_get_prop_length (node, "w", 0);
	h = lgl_xml_get_prop_length (node, "h", 0);
	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h, FALSE);

	/* justify attr */
	string = lgl_xml_get_prop_string (node, "justify", NULL);
	align = gl_str_util_string_to_align (string);
	g_free (string);
	gl_label_object_set_text_alignment (GL_LABEL_OBJECT(object), align, FALSE);

	/* auto_shrink attr */
	auto_shrink = lgl_xml_get_prop_boolean (node, "auto_shrink", FALSE);
	gl_label_text_set_auto_shrink (GL_LABEL_TEXT(object), auto_shrink, FALSE);

	/* affine attrs */
	xml_parse_affine_attrs (node, GL_LABEL_OBJECT(object));

	/* shadow attrs */
	xml_parse_shadow_attrs (node, GL_LABEL_OBJECT(object));

	/* Process children */
	for (child = node->xmlChildrenNode; child != NULL; child = child->next) {
		if (lgl_xml_is_node (child, "Span")) {
			xml_parse_toplevel_span (child, GL_LABEL_OBJECT(object));
			break;
		} else {
			if (!xmlNodeIsText (child)) {
				g_message ("Unexpected Object-text child: \"%s\"",
					   child->name);
			}
		}
	}

	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Objects->Object-box Node                             */
/*--------------------------------------------------------------------------*/
static void
xml_parse_object_box (xmlNodePtr  node,
		      glLabel    *label)
{
	GObject      *object;
	gdouble       x, y;
	gdouble       w, h;
	gdouble       line_width;
	glColorNode  *line_color_node;
	gchar        *string;
	glColorNode  *fill_color_node;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_box_new (label, FALSE);

	/* position attrs */
	x = lgl_xml_get_prop_length (node, "x", 0.0);
	y = lgl_xml_get_prop_length (node, "y", 0.0);
	gl_label_object_set_position (GL_LABEL_OBJECT(object), x, y, FALSE);

	/* size attrs */
	w = lgl_xml_get_prop_length (node, "w", 0);
	h = lgl_xml_get_prop_length (node, "h", 0);
	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h, FALSE);

	/* line attrs */
	line_width = lgl_xml_get_prop_length (node, "line_width", 1.0);
	gl_label_object_set_line_width (GL_LABEL_OBJECT(object), line_width, FALSE);
	
	line_color_node = gl_color_node_new_default ();
	string = lgl_xml_get_prop_string (node, "line_color_field", NULL);
	if ( string ) {
		line_color_node->field_flag = TRUE;
		line_color_node->key = string;
	} else {
		line_color_node->color = lgl_xml_get_prop_uint (node, "line_color", 0);
	}
	gl_label_object_set_line_color (GL_LABEL_OBJECT(object), line_color_node, FALSE);
	gl_color_node_free (&line_color_node);


	/* fill attrs */
	fill_color_node = gl_color_node_new_default ();
	string = lgl_xml_get_prop_string (node, "fill_color_field", NULL);
	if ( string ) {
		fill_color_node->field_flag = TRUE;
		fill_color_node->key = string;
	} else {
		fill_color_node->color = lgl_xml_get_prop_uint (node, "fill_color", 0);
	}
	gl_label_object_set_fill_color (GL_LABEL_OBJECT(object), fill_color_node, FALSE);
	gl_color_node_free (&fill_color_node);
	
	/* affine attrs */
	xml_parse_affine_attrs (node, GL_LABEL_OBJECT(object));

	/* shadow attrs */
	xml_parse_shadow_attrs (node, GL_LABEL_OBJECT(object));

	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Objects->Object-ellipse Node                         */
/*--------------------------------------------------------------------------*/
static void
xml_parse_object_ellipse (xmlNodePtr  node,
			  glLabel    *label)
{
	GObject     *object;
	gdouble      x, y;
	gdouble      w, h;
	gdouble      line_width;
	glColorNode *line_color_node;
	gchar       *string;
	glColorNode *fill_color_node;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_ellipse_new (label, FALSE);

	/* position attrs */
	x = lgl_xml_get_prop_length (node, "x", 0.0);
	y = lgl_xml_get_prop_length (node, "y", 0.0);
	gl_label_object_set_position (GL_LABEL_OBJECT(object), x, y, FALSE);

	/* size attrs */
	w = lgl_xml_get_prop_length (node, "w", 0);
	h = lgl_xml_get_prop_length (node, "h", 0);
	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h, FALSE);

	/* line attrs */
	line_width = lgl_xml_get_prop_length (node, "line_width", 1.0);
	gl_label_object_set_line_width (GL_LABEL_OBJECT(object), line_width, FALSE);

	line_color_node = gl_color_node_new_default ();
	string = lgl_xml_get_prop_string (node, "line_color_field", NULL);
	if ( string ) {
		line_color_node->field_flag = TRUE;
		line_color_node->key = string;
	} else {
		line_color_node->color = lgl_xml_get_prop_uint (node, "line_color", 0);		
	}
	gl_label_object_set_line_color (GL_LABEL_OBJECT(object), line_color_node, FALSE);
	gl_color_node_free (&line_color_node);


	/* fill attrs */
	fill_color_node = gl_color_node_new_default ();
	string = lgl_xml_get_prop_string (node, "fill_color_field", NULL);
	if ( string ) {
		fill_color_node->field_flag = TRUE;
		fill_color_node->key = string;
	} else {
		fill_color_node->color = lgl_xml_get_prop_uint (node, "fill_color", 0);
	}
	gl_label_object_set_fill_color (GL_LABEL_OBJECT(object), fill_color_node, FALSE);
	gl_color_node_free (&fill_color_node);

	/* affine attrs */
	xml_parse_affine_attrs (node, GL_LABEL_OBJECT(object));

	/* shadow attrs */
	xml_parse_shadow_attrs (node, GL_LABEL_OBJECT(object));

	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Objects->Object-line Node                            */
/*--------------------------------------------------------------------------*/
static void
xml_parse_object_line (xmlNodePtr  node,
		       glLabel    *label)
{
	GObject     *object;
	gdouble      x, y;
	gdouble      dx, dy;
	gdouble      line_width;
	glColorNode *line_color_node;
	gchar       *string;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_line_new (label, FALSE);

	/* position attrs */
	x = lgl_xml_get_prop_length (node, "x", 0.0);
	y = lgl_xml_get_prop_length (node, "y", 0.0);
	gl_label_object_set_position (GL_LABEL_OBJECT(object), x, y, FALSE);

	/* length attrs */
	dx = lgl_xml_get_prop_length (node, "dx", 0);
	dy = lgl_xml_get_prop_length (node, "dy", 0);
	gl_label_object_set_size (GL_LABEL_OBJECT(object), dx, dy, FALSE);

	/* line attrs */
	line_width = lgl_xml_get_prop_length (node, "line_width", 1.0);
	gl_label_object_set_line_width (GL_LABEL_OBJECT(object), line_width, FALSE);
	
	line_color_node = gl_color_node_new_default ();
	string = lgl_xml_get_prop_string (node, "line_color_field", NULL);
	if ( string ) {
		line_color_node->field_flag = TRUE;
		line_color_node->key = string;
	} else {
		line_color_node->color = lgl_xml_get_prop_uint (node, "line_color", 0);		
	}
	gl_label_object_set_line_color (GL_LABEL_OBJECT(object), line_color_node, FALSE);
	gl_color_node_free (&line_color_node);

	/* affine attrs */
	xml_parse_affine_attrs (node, GL_LABEL_OBJECT(object));

	/* shadow attrs */
	xml_parse_shadow_attrs (node, GL_LABEL_OBJECT(object));

	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Objects->Object-image Node                           */
/*--------------------------------------------------------------------------*/
static void
xml_parse_object_image (xmlNodePtr  node,
			glLabel    *label)
{
	GObject      *object;
	gdouble       x, y;
	gdouble       w, h;
	gchar        *string;
	glTextNode   *filename;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_image_new (label, FALSE);

	/* position attrs */
	x = lgl_xml_get_prop_length (node, "x", 0.0);
	y = lgl_xml_get_prop_length (node, "y", 0.0);
	gl_label_object_set_position (GL_LABEL_OBJECT(object), x, y, FALSE);

	/* src or field attr */
	string = lgl_xml_get_prop_string (node, "src", NULL);
	if ( string ) {
		filename = g_new0 (glTextNode, 1);
		filename->field_flag = FALSE;
		filename->data = g_strdup ((gchar *)string);
		gl_label_image_set_filename (GL_LABEL_IMAGE(object), filename, FALSE);
		gl_text_node_free (&filename);
		xmlFree (string);
	} else {
		string = lgl_xml_get_prop_string (node, "field", NULL);
		if ( string ) {
			filename = g_new0 (glTextNode, 1);
			filename->field_flag = TRUE;
			filename->data = g_strdup ((gchar *)string);
			gl_label_image_set_filename (GL_LABEL_IMAGE(object), filename, FALSE);
			gl_text_node_free (&filename);
			xmlFree (string);
		} else {
			g_message ("Missing Object-image src or field attr");
		}
	}

	/* size attrs */
	w = lgl_xml_get_prop_length (node, "w", 0);
	h = lgl_xml_get_prop_length (node, "h", 0);
	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h, FALSE);

	/* affine attrs */
	xml_parse_affine_attrs (node, GL_LABEL_OBJECT(object));

	/* shadow attrs */
	xml_parse_shadow_attrs (node, GL_LABEL_OBJECT(object));

	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Objects->Object-barcode Node                         */
/*--------------------------------------------------------------------------*/
static void
xml_parse_object_barcode (xmlNodePtr  node,
			  glLabel    *label)
{
	GObject             *object;
	gdouble              x, y;
	gdouble              w, h;
	gchar               *string;
	glTextNode          *text_node;
	gchar               *backend_id;
	gchar               *id;
        glLabelBarcodeStyle *style;
	glColorNode         *color_node;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_barcode_new (label, FALSE);

	/* position attrs */
	x = lgl_xml_get_prop_length (node, "x", 0.0);
	y = lgl_xml_get_prop_length (node, "y", 0.0);
	gl_label_object_set_position (GL_LABEL_OBJECT(object), x, y, FALSE);

	/* size attrs */
	w = lgl_xml_get_prop_length (node, "w", 0);
	h = lgl_xml_get_prop_length (node, "h", 0);
	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h, FALSE);

	/* style attrs */
        style = gl_label_barcode_style_new ();
	backend_id = lgl_xml_get_prop_string (node, "backend", NULL);
	id = lgl_xml_get_prop_string (node, "style", NULL);
        if ( !backend_id )
        {
                backend_id = g_strdup (gl_barcode_backends_guess_backend_id (id));
        }
        gl_label_barcode_style_set_backend_id (style, backend_id);
        gl_label_barcode_style_set_style_id (style, id);
	style->text_flag = lgl_xml_get_prop_boolean (node, "text", FALSE);
	style->checksum_flag = lgl_xml_get_prop_boolean (node, "checksum", TRUE);
	style->format_digits = lgl_xml_get_prop_uint (node, "format", 10);
	gl_label_barcode_set_style (GL_LABEL_BARCODE(object), style, FALSE);
	g_free (backend_id);
	g_free (id);
        gl_label_barcode_style_free (style);
	
	color_node = gl_color_node_new_default ();
	string = lgl_xml_get_prop_string (node, "color_field", NULL);
	if ( string ) {
		color_node->field_flag = TRUE;
		color_node->key = string;
	} else {
		color_node->color = lgl_xml_get_prop_uint (node, "color", 0);		
	}
	gl_label_object_set_line_color (GL_LABEL_OBJECT(object), color_node, FALSE);
	gl_color_node_free (&color_node);

	/* data or field attr */
	string = lgl_xml_get_prop_string (node, "data", NULL);
	if ( string ) {
		text_node = g_new0 (glTextNode, 1);
		text_node->field_flag = FALSE;
		text_node->data = string;
		gl_label_barcode_set_data (GL_LABEL_BARCODE(object), text_node, FALSE);
		gl_text_node_free (&text_node);
	} else {
		string = lgl_xml_get_prop_string (node, "field", NULL);
		if ( string ) {
			text_node = g_new0 (glTextNode, 1);
			text_node->field_flag = TRUE;
			text_node->data = string;
			gl_label_barcode_set_data (GL_LABEL_BARCODE(object), text_node, FALSE);
			gl_text_node_free (&text_node);
		} else {
			g_message ("Missing Object-barcode data or field attr");
		}
	}

	/* affine attrs */
	xml_parse_affine_attrs (node, GL_LABEL_OBJECT(object));

	/* shadow attrs */
	xml_parse_shadow_attrs (node, GL_LABEL_OBJECT(object));

	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML merge fields tag.                                    */
/*--------------------------------------------------------------------------*/
static void
xml_parse_merge_fields (xmlNodePtr  node,
			glLabel    *label)
{
	gchar      *string;
	glMerge    *merge;

	gl_debug (DEBUG_XML, "START");

	string = lgl_xml_get_prop_string (node, "type", NULL);
	merge = gl_merge_new (string);
	g_free (string);

        if (merge)
        {
                string = lgl_xml_get_prop_string (node, "src", NULL);
                gl_merge_set_src (merge, string);
                g_free (string);

                gl_label_set_merge (label, merge, FALSE);

                g_object_unref (G_OBJECT(merge));
        }

	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML data tag.                                            */
/*--------------------------------------------------------------------------*/
static void
xml_parse_data (xmlNodePtr  node,
		glLabel    *label)
{
	xmlNodePtr  child;

	gl_debug (DEBUG_XML, "START");

	for (child = node->xmlChildrenNode; child != NULL; child = child->next) {

		if (lgl_xml_is_node (child, "Pixdata")) {
			xml_parse_pixdata (child, label);
		} else if (lgl_xml_is_node (child, "File")) {
			xml_parse_file_node (child, label);
		} else {
			if (!xmlNodeIsText (child)) {
				g_message (_("bad node in Data node =  \"%s\""),
					   child->name);
			}
		}
	}

	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML embedded Pixdata node.                               */
/*--------------------------------------------------------------------------*/
static void
xml_parse_pixdata (xmlNodePtr  node,
		   glLabel    *label)
{
	gchar      *name, *base64;
	guchar     *stream;
	gsize       stream_length;
	gboolean    ret;
	GdkPixdata *pixdata;
	GdkPixbuf  *pixbuf;
	GHashTable *pixbuf_cache;

	gl_debug (DEBUG_XML, "START");

	name = lgl_xml_get_prop_string (node, "name", NULL);
	base64 = lgl_xml_get_node_content (node);

	stream = g_base64_decode ((gchar *)base64, &stream_length);
	pixdata = g_new0 (GdkPixdata, 1);
	ret = gdk_pixdata_deserialize (pixdata, stream_length, stream, NULL);

	if (ret) {
		pixbuf = gdk_pixbuf_from_pixdata (pixdata, TRUE, NULL);

		pixbuf_cache = gl_label_get_pixbuf_cache (label);
		gl_pixbuf_cache_add_pixbuf (pixbuf_cache, (gchar *)name, pixbuf);
	}

	g_free (name);
	g_free (base64);

	g_free (stream);
	g_free (pixdata);

	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML embedded File node.                                  */
/*--------------------------------------------------------------------------*/
static void
xml_parse_file_node (xmlNodePtr  node,
                     glLabel    *label)
{
	gchar      *name, *format;
        gchar      *content;
	GHashTable *svg_cache;

	name    = lgl_xml_get_prop_string (node, "name", NULL);
	format  = lgl_xml_get_prop_string (node, "format", NULL);

        if ( format && (lgl_str_utf8_casecmp (format, "SVG") == 0) )
        {
                content = lgl_xml_get_node_content (node);

		svg_cache = gl_label_get_svg_cache (label);
                gl_svg_cache_add_svg (svg_cache, name, content);

                g_free (content);
        }
        else
        {
                g_message (_("Unknown embedded file format: \"%s\""), format);
                
        }

        g_free (name);
        g_free (format);
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse top-level Span tag.                                      */
/*--------------------------------------------------------------------------*/
static void
xml_parse_toplevel_span  (xmlNodePtr        node,
			  glLabelObject    *object)
{
	gchar            *font_family;
	gdouble           font_size;
	PangoWeight       font_weight;
	gboolean          font_italic_flag;
	glColorNode      *color_node;
	gdouble           text_line_spacing;
	gchar            *string;
	GList            *lines, *text_nodes;
	xmlNodePtr        child;
	glTextNode       *text_node;
	GRegex					 *strip_regex;

	gl_debug (DEBUG_XML, "START");

	/* Font family attr */
	font_family = lgl_xml_get_prop_string (node, "font_family", "Sans");
	gl_label_object_set_font_family (object, font_family, FALSE);
	g_free (font_family);

	/* Font size attr */
	font_size = lgl_xml_get_prop_double (node, "font_size", 0.0);
	gl_label_object_set_font_size (object, font_size, FALSE);

	/* Font weight attr */
	string = lgl_xml_get_prop_string (node, "font_weight", NULL);
	font_weight = gl_str_util_string_to_weight (string);
	g_free (string);
	gl_label_object_set_font_weight (object, font_weight, FALSE);

	/* Font italic flag attr */
	font_italic_flag = lgl_xml_get_prop_boolean (node, "font_italic", FALSE);
	gl_label_object_set_font_italic_flag (object, font_italic_flag, FALSE);

	/* Text color attr */
	color_node = gl_color_node_new_default ();
	string = lgl_xml_get_prop_string (node, "color_field", NULL);
	if ( string ) {
		color_node->field_flag = TRUE;
		color_node->key = string;
	} else {
		color_node->color = lgl_xml_get_prop_uint (node, "color", 0);		
	}
	gl_label_object_set_text_color (object, color_node, FALSE);
	gl_color_node_free (&color_node);
	
	/* Text line spacing attr  */
	text_line_spacing = lgl_xml_get_prop_double (node, "line_spacing", 1.0);
	gl_label_object_set_text_line_spacing (object, text_line_spacing, FALSE); 

	/* Now descend children, and build lines of text nodes */
	lines = NULL;
	text_nodes = NULL;
	strip_regex = g_regex_new("\\A\\n\\s*|\\n\\s*\\Z", 0, 0, NULL);
	for (child = node->xmlChildrenNode; child != NULL; child = child->next) {

		if (lgl_xml_is_node (child, "Span")) {

			g_message ("Unexpected rich text (not supported, yet!)");

		} else if (lgl_xml_is_node (child, "Field")) {

			/* Field node */
			string = lgl_xml_get_prop_string (child, "name", NULL);
			text_node = g_new0 (glTextNode, 1);
			text_node->field_flag = TRUE;
			text_node->data = string;
			text_nodes = g_list_append (text_nodes, text_node);

		} else if (lgl_xml_is_node (child, "NL")) {

			/* Store line. */
			lines = g_list_append (lines, text_nodes);
			text_nodes = NULL;

    } else if (xmlNodeIsText (child)) {
			gchar *orig_data = lgl_xml_get_node_content (child); 
			gchar *data;

			/* Literal text */

			/* Stip out white space before and after */
			data = g_regex_replace(strip_regex, orig_data, -1, 0, "", 0, NULL);
			g_free (orig_data);

			text_node = g_new0 (glTextNode, 1);
			text_node->field_flag = FALSE;
			text_node->data = g_strdup ((gchar *)data);
			text_nodes = g_list_append (text_nodes, text_node);

			g_free (data);

		} else {
			g_message ("Unexpected Span child: \"%s\"", child->name);
		}

	}
  g_free (strip_regex);
	if ( text_nodes ) {
		/* Store last line. */
		lines = g_list_append (lines, text_nodes);
		text_nodes = NULL;
	}
	gl_label_text_set_lines (GL_LABEL_TEXT(object), lines, FALSE);
	gl_text_node_lines_free (&lines);

	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse affine attributes.                                       */
/*--------------------------------------------------------------------------*/
static void
xml_parse_affine_attrs (xmlNodePtr        node,
			glLabelObject    *object)
{
	gdouble           a[6];
        cairo_matrix_t    matrix;

	a[0] = lgl_xml_get_prop_double (node, "a0", 0.0);
	a[1] = lgl_xml_get_prop_double (node, "a1", 0.0);
	a[2] = lgl_xml_get_prop_double (node, "a2", 0.0);
	a[3] = lgl_xml_get_prop_double (node, "a3", 0.0);
	a[4] = lgl_xml_get_prop_double (node, "a4", 0.0);
	a[5] = lgl_xml_get_prop_double (node, "a5", 0.0);

        cairo_matrix_init (&matrix, a[0], a[1], a[2], a[3], a[4], a[5]);

	gl_label_object_set_matrix (object, &matrix);
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse shadow attributes.                                       */
/*--------------------------------------------------------------------------*/
static void
xml_parse_shadow_attrs (xmlNodePtr        node,
			glLabelObject    *object)
{
	gboolean         shadow_state;
	gdouble          shadow_x;
	gdouble          shadow_y;
	glColorNode     *shadow_color_node;
	gdouble          shadow_opacity;
	gchar           *string;

	shadow_state = lgl_xml_get_prop_boolean (node, "shadow", FALSE);
	gl_label_object_set_shadow_state (object, shadow_state, FALSE);

	if (shadow_state)
	{
		shadow_x = lgl_xml_get_prop_length (node, "shadow_x", 0.0);
		shadow_y = lgl_xml_get_prop_length (node, "shadow_y", 0.0);
		gl_label_object_set_shadow_offset (object, shadow_x, shadow_y, FALSE);
		
		shadow_color_node = gl_color_node_new_default ();
		string = lgl_xml_get_prop_string (node, "shadow_color_field", NULL);
		if ( string ) {
			shadow_color_node->field_flag = TRUE;
			shadow_color_node->key = string;
		} else {
			shadow_color_node->color = lgl_xml_get_prop_uint (node, "shadow_color", 0);		
		}
		gl_label_object_set_shadow_color (object, shadow_color_node, FALSE);
		gl_color_node_free (&shadow_color_node);

		shadow_opacity = lgl_xml_get_prop_double (node, "shadow_opacity", 1.0);
		gl_label_object_set_shadow_opacity (object, shadow_opacity, FALSE);
	}
}


/****************************************************************************/
/* Save label to xml label file.                                            */
/****************************************************************************/
void
gl_xml_label_save (glLabel          *label,
		   const gchar      *utf8_filename,
		   glXMLLabelStatus *status)
{
	xmlDocPtr doc;
	gint      xml_ret;
	gchar 	  *filename;

	gl_debug (DEBUG_XML, "START");

	doc = xml_label_to_doc (label, status);

	filename = g_filename_from_utf8 (utf8_filename, -1, NULL, NULL, NULL);
	if (!filename)
		g_message (_("Utf8 conversion error."));
	else {
		xmlSetDocCompressMode (doc, gl_label_get_compression (label));
		xml_ret = xmlSaveFormatFile (filename, doc, TRUE);
		xmlFreeDoc (doc);
		if (xml_ret == -1) {

			g_message (_("Problem saving xml file."));
			*status = XML_LABEL_ERROR_SAVE_FILE;

		} else {

			gl_label_set_filename (label, utf8_filename);
			gl_label_clear_modified (label);

		}
		g_free (filename);
	}

	gl_debug (DEBUG_XML, "END");
}


/****************************************************************************/
/* Save label to xml buffer.                                                */
/****************************************************************************/
gchar *
gl_xml_label_save_buffer (glLabel          *label,
			  glXMLLabelStatus *status)
{
	xmlDocPtr  doc;
	gint       size;
	guchar    *buffer;

	gl_debug (DEBUG_XML, "START");

	doc = xml_label_to_doc (label, status);

	xmlDocDumpMemory (doc, &buffer, &size);
	xmlFreeDoc (doc);

	gl_label_clear_modified (label);

	gl_debug (DEBUG_XML, "END");

	return (gchar *)buffer;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Convert label to xml doc structure.                            */
/*--------------------------------------------------------------------------*/
static xmlDocPtr
xml_label_to_doc (glLabel          *label,
		  glXMLLabelStatus *status)
{
        lglUnits           units;
	xmlDocPtr          doc;
	xmlNsPtr           ns;
        const lglTemplate *template;
	glMerge           *merge;

	gl_debug (DEBUG_XML, "START");

	LIBXML_TEST_VERSION;

        units = gl_prefs_model_get_units (gl_prefs);
        lgl_xml_set_default_units (units);

	doc = xmlNewDoc ((xmlChar *)"1.0");
	doc->xmlRootNode = xmlNewDocNode (doc, NULL, (xmlChar *)"Glabels-document", NULL);

	ns = xmlNewNs (doc->xmlRootNode, (xmlChar *)LGL_XML_NAME_SPACE, NULL);
	xmlSetNs (doc->xmlRootNode, ns);

        template = gl_label_get_template (label);
	lgl_xml_template_create_template_node (template, doc->xmlRootNode, ns);

	xml_create_objects (doc->xmlRootNode, ns, label);

	merge = gl_label_get_merge (label);
	gl_debug (DEBUG_XML, "merge=%p", merge);
	if (merge != NULL) {
		xml_create_merge_fields (doc->xmlRootNode, ns, label);
		g_object_unref (G_OBJECT(merge));
	}

	xml_create_data (doc, doc->xmlRootNode, ns, label);

	gl_debug (DEBUG_XML, "END");

	*status = XML_LABEL_OK;
	return doc;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Objects Node                                           */
/*--------------------------------------------------------------------------*/
static void
xml_create_objects (xmlNodePtr  parent,
		    xmlNsPtr    ns,
		    glLabel    *label)
{
	xmlNodePtr     node;
        gboolean       rotate_flag;
        const GList   *object_list;
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_XML, "START");

        rotate_flag = gl_label_get_rotate_flag (label);
        object_list = gl_label_get_object_list (label);

	node = xmlNewChild (parent, ns, (xmlChar *)"Objects", NULL);
	lgl_xml_set_prop_string (node, "id", "0");
	lgl_xml_set_prop_boolean (node, "rotate", rotate_flag);

	for (p = (GList *)object_list; p != NULL; p = p->next) {

		object = GL_LABEL_OBJECT(p->data);

		if ( GL_IS_LABEL_TEXT(object) ) {
			xml_create_object_text (node, ns, object);
		} else if ( GL_IS_LABEL_BOX(object) ) {
			xml_create_object_box (node, ns, object);
		} else if ( GL_IS_LABEL_ELLIPSE(object) ) {
			xml_create_object_ellipse (node, ns, object);
		} else if ( GL_IS_LABEL_LINE(object) ) {
			xml_create_object_line (node, ns, object);
		} else if ( GL_IS_LABEL_IMAGE(object) ) {
			xml_create_object_image (node, ns, object);
		} else if ( GL_IS_LABEL_BARCODE(object) ) {
			xml_create_object_barcode (node, ns, object);
		} else {
			g_message ("Unknown label object");
		}

	}

	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Objects->Object-text Node                              */
/*--------------------------------------------------------------------------*/
static void
xml_create_object_text (xmlNodePtr     parent,
			xmlNsPtr       ns,
			glLabelObject *object)
{
	xmlNodePtr        node;
	gdouble           x, y;
	gdouble           w, h;
	PangoAlignment    align;
	gboolean          auto_shrink;

	gl_debug (DEBUG_XML, "START");

	node = xmlNewChild (parent, ns, (xmlChar *)"Object-text", NULL);

	/* position attrs */
	gl_label_object_get_position (object, &x, &y);
	lgl_xml_set_prop_length (node, "x", x);
	lgl_xml_set_prop_length (node, "y", y);

	/* size attrs */
	gl_label_object_get_raw_size ( object, &w, &h);
	lgl_xml_set_prop_length (node, "w", w);
	lgl_xml_set_prop_length (node, "h", h);

	/* justify attr */
	align = gl_label_object_get_text_alignment (object);
	lgl_xml_set_prop_string (node, "justify", gl_str_util_align_to_string (align));

	/* auto_shrink attr */
	auto_shrink = gl_label_text_get_auto_shrink (GL_LABEL_TEXT (object));
	lgl_xml_set_prop_boolean (node, "auto_shrink", auto_shrink);

	/* affine attrs */
	xml_create_affine_attrs (node, object);

	/* shadow attrs */
	xml_create_shadow_attrs (node, object);

	/* Add children */
	xml_create_toplevel_span (node, ns, GL_LABEL_TEXT(object));

	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Objects->Object-box Node                               */
/*--------------------------------------------------------------------------*/
static void
xml_create_object_box (xmlNodePtr     parent,
		       xmlNsPtr       ns,
		       glLabelObject *object)
{
	xmlNodePtr        node;
	gdouble           x, y;
	gdouble           w, h;
	gdouble           line_width;
	glColorNode      *line_color_node;
	glColorNode *fill_color_node;

	gl_debug (DEBUG_XML, "START");

	node = xmlNewChild (parent, ns, (xmlChar *)"Object-box", NULL);

	/* position attrs */
	gl_label_object_get_position (object, &x, &y);
	lgl_xml_set_prop_length (node, "x", x);
	lgl_xml_set_prop_length (node, "y", y);

	/* size attrs */
	gl_label_object_get_size (object, &w, &h);
	lgl_xml_set_prop_length (node, "w", w);
	lgl_xml_set_prop_length (node, "h", h);

	/* line attrs */
	line_width = gl_label_object_get_line_width (GL_LABEL_OBJECT(object));
	lgl_xml_set_prop_length (node, "line_width", line_width);
	
	line_color_node = gl_label_object_get_line_color (GL_LABEL_OBJECT(object));
	if (line_color_node->field_flag)
	{
		lgl_xml_set_prop_string (node, "line_color_field", line_color_node->key);
	}
	else
	{
		lgl_xml_set_prop_uint_hex (node, "line_color", line_color_node->color);
	}
	gl_color_node_free (&line_color_node);

	/* fill attrs (color or field) */
	fill_color_node = gl_label_object_get_fill_color (GL_LABEL_OBJECT(object));
	if (fill_color_node->field_flag)
	{
		lgl_xml_set_prop_string (node, "fill_color_field", fill_color_node->key);
	}
	else
	{
		lgl_xml_set_prop_uint_hex (node, "fill_color", fill_color_node->color);
	}
	gl_color_node_free (&fill_color_node);

	/* affine attrs */
	xml_create_affine_attrs (node, object);

	/* shadow attrs */
	xml_create_shadow_attrs (node, object);

	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Objects->Object-ellipse Node                           */
/*--------------------------------------------------------------------------*/
static void
xml_create_object_ellipse (xmlNodePtr     parent,
			   xmlNsPtr       ns,
			   glLabelObject *object)
{
	xmlNodePtr        node;
	gdouble           x, y;
	gdouble           w, h;
	gdouble           line_width;
	glColorNode      *line_color_node;
	glColorNode *fill_color_node;

	gl_debug (DEBUG_XML, "START");

	node = xmlNewChild (parent, ns, (xmlChar *)"Object-ellipse", NULL);

	/* position attrs */
	gl_label_object_get_position (object, &x, &y);
	lgl_xml_set_prop_length (node, "x", x);
	lgl_xml_set_prop_length (node, "y", y);

	/* size attrs */
	gl_label_object_get_size (object, &w, &h);
	lgl_xml_set_prop_length (node, "w", w);
	lgl_xml_set_prop_length (node, "h", h);

	/* line attrs */
	line_width = gl_label_object_get_line_width (GL_LABEL_OBJECT(object));
	lgl_xml_set_prop_length (node, "line_width", line_width);
	
	line_color_node = gl_label_object_get_line_color (GL_LABEL_OBJECT(object));
	if (line_color_node->field_flag)
	{
		lgl_xml_set_prop_string (node, "line_color_field", line_color_node->key);
	}
	else
	{
		lgl_xml_set_prop_uint_hex (node, "line_color", line_color_node->color);
	}
	gl_color_node_free (&line_color_node);


	/* fill attrs (color or field) */
	fill_color_node = gl_label_object_get_fill_color (GL_LABEL_OBJECT(object));
	if (fill_color_node->field_flag)
	{
		lgl_xml_set_prop_string (node, "fill_color_field", fill_color_node->key);
	}
	else
	{
		lgl_xml_set_prop_uint_hex (node, "fill_color", fill_color_node->color);
	}
	gl_color_node_free (&fill_color_node);

	/* affine attrs */
	xml_create_affine_attrs (node, object);

	/* shadow attrs */
	xml_create_shadow_attrs (node, object);

	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Objects->Object-line Node                              */
/*--------------------------------------------------------------------------*/
static void
xml_create_object_line (xmlNodePtr     parent,
			xmlNsPtr       ns,
			glLabelObject *object)
{
	xmlNodePtr        node;
	gdouble           x, y;
	gdouble           dx, dy;
	gdouble           line_width;
	glColorNode      *line_color_node;

	gl_debug (DEBUG_XML, "START");

	node = xmlNewChild (parent, ns, (xmlChar *)"Object-line", NULL);

	/* position attrs */
	gl_label_object_get_position (object, &x, &y);
	lgl_xml_set_prop_length (node, "x", x);
	lgl_xml_set_prop_length (node, "y", y);

	/* length attrs */
	gl_label_object_get_size (object, &dx, &dy);
	lgl_xml_set_prop_length (node, "dx", dx);
	lgl_xml_set_prop_length (node, "dy", dy);

	/* line attrs */
	line_width = gl_label_object_get_line_width (GL_LABEL_OBJECT(object));
	lgl_xml_set_prop_length (node, "line_width", line_width);
	
	line_color_node = gl_label_object_get_line_color (GL_LABEL_OBJECT(object));
	if (line_color_node->field_flag)
	{
		lgl_xml_set_prop_string (node, "line_color_field", line_color_node->key);
	}
	else
	{
		lgl_xml_set_prop_uint_hex (node, "line_color", line_color_node->color);
	}
	gl_color_node_free (&line_color_node);


	/* affine attrs */
	xml_create_affine_attrs (node, object);

	/* shadow attrs */
	xml_create_shadow_attrs (node, object);

	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Objects->Object-image Node                             */
/*--------------------------------------------------------------------------*/
static void
xml_create_object_image (xmlNodePtr     parent,
			 xmlNsPtr       ns,
			 glLabelObject *object)
{
	xmlNodePtr        node;
	gdouble           x, y;
	gdouble           w, h;
	glTextNode       *filename;

	gl_debug (DEBUG_XML, "START");

	node = xmlNewChild (parent, ns, (xmlChar *)"Object-image", NULL);

	/* position attrs */
	gl_label_object_get_position (object, &x, &y);
	lgl_xml_set_prop_length (node, "x", x);
	lgl_xml_set_prop_length (node, "y", y);

	/* size attrs */
	gl_label_object_get_size (object, &w, &h);
	lgl_xml_set_prop_length (node, "w", w);
	lgl_xml_set_prop_length (node, "h", h);

	/* src OR field attr */
	filename = gl_label_image_get_filename (GL_LABEL_IMAGE(object));
	if (filename->field_flag) {
		lgl_xml_set_prop_string (node, "field", filename->data);
	} else {
		lgl_xml_set_prop_string (node, "src", filename->data);
	}
	gl_text_node_free (&filename);

	/* affine attrs */
	xml_create_affine_attrs (node, object);

	/* shadow attrs */
	xml_create_shadow_attrs (node, object);

	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Objects->Object-barcode Node                           */
/*--------------------------------------------------------------------------*/
static void
xml_create_object_barcode (xmlNodePtr     parent,
			   xmlNsPtr       ns,
			   glLabelObject *object)
{
	xmlNodePtr           node;
	gdouble              x, y;
	gdouble              w, h;
	glTextNode          *text_node;
        glLabelBarcodeStyle *style;
	glColorNode         *color_node;

	gl_debug (DEBUG_XML, "START");

	node = xmlNewChild (parent, ns, (xmlChar *)"Object-barcode", NULL);

	/* position attrs */
	gl_label_object_get_position (object, &x, &y);
	lgl_xml_set_prop_length (node, "x", x);
	lgl_xml_set_prop_length (node, "y", y);

	/* size attrs */
	gl_label_object_get_raw_size (object, &w, &h);
	lgl_xml_set_prop_length (node, "w", w);
	lgl_xml_set_prop_length (node, "h", h);

	/* Barcode properties attrs */
	style = gl_label_barcode_get_style (GL_LABEL_BARCODE(object));
	lgl_xml_set_prop_string (node, "backend", style->backend_id);
	lgl_xml_set_prop_string (node, "style", style->id);
	lgl_xml_set_prop_boolean (node, "text", style->text_flag);
	lgl_xml_set_prop_boolean (node, "checksum", style->checksum_flag);
	
	color_node = gl_label_object_get_line_color (GL_LABEL_OBJECT(object));
	if (color_node->field_flag)
	{
		lgl_xml_set_prop_string (node, "color_field", color_node->key);
	}
	else
	{
		lgl_xml_set_prop_uint_hex (node, "color", color_node->color);
	}
	gl_color_node_free (&color_node);


	/* data OR field attr */
	text_node = gl_label_barcode_get_data (GL_LABEL_BARCODE(object));
	if (text_node->field_flag) {
		lgl_xml_set_prop_string (node, "field", text_node->data);
	        lgl_xml_set_prop_int (node, "format", style->format_digits);
	} else {
		lgl_xml_set_prop_string (node, "data", text_node->data);
	}
	gl_text_node_free (&text_node);

	/* affine attrs */
	xml_create_affine_attrs (node, object);

	/* shadow attrs */
	xml_create_shadow_attrs (node, object);

        gl_label_barcode_style_free (style);

	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label Merge Fields Node                                */
/*--------------------------------------------------------------------------*/
static void
xml_create_merge_fields (xmlNodePtr  parent,
			 xmlNsPtr    ns,
			 glLabel    *label)
{
	xmlNodePtr  node;
	gchar      *string;
	glMerge    *merge;

	gl_debug (DEBUG_XML, "START");

	merge = gl_label_get_merge (label);

	node = xmlNewChild (parent, ns, (xmlChar *)"Merge", NULL);

	string = gl_merge_get_name (merge);
	lgl_xml_set_prop_string (node, "type", string);
	g_free (string);

	string = gl_merge_get_src (merge);
	lgl_xml_set_prop_string (node, "src", string);
	g_free (string);

	g_object_unref (G_OBJECT(merge));

	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label Data Node                                        */
/*--------------------------------------------------------------------------*/
static void
xml_create_data (xmlDocPtr   doc,
                 xmlNodePtr  parent,
		 xmlNsPtr    ns,
		 glLabel    *label)
{
	xmlNodePtr  node;
	GHashTable *cache;
	GList      *name_list, *p;

	gl_debug (DEBUG_XML, "START");

	node = xmlNewChild (parent, ns, (xmlChar *)"Data", NULL);

	cache = gl_label_get_pixbuf_cache (label);
	name_list = gl_pixbuf_cache_get_name_list (cache);

	for (p = name_list; p != NULL; p=p->next) {
		xml_create_pixdata (node, ns, label, p->data);
	}

	gl_pixbuf_cache_free_name_list (name_list);


	cache = gl_label_get_svg_cache (label);
	name_list = gl_svg_cache_get_name_list (cache);

	for (p = name_list; p != NULL; p=p->next) {
		xml_create_file_svg (doc, node, ns, label, p->data);
	}

	gl_pixbuf_cache_free_name_list (name_list);


	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label Data embedded Pixdata Node                       */
/*--------------------------------------------------------------------------*/
static void
xml_create_pixdata (xmlNodePtr  parent,
		    xmlNsPtr    ns,
		    glLabel    *label,
		    gchar      *name)
{
	xmlNodePtr  node;
	GHashTable *pixbuf_cache;
	GdkPixbuf  *pixbuf;
	GdkPixdata *pixdata;
	guchar     *stream;
	guint       stream_length;
	gchar      *base64;

	gl_debug (DEBUG_XML, "START");

	pixbuf_cache = gl_label_get_pixbuf_cache (label);

	pixbuf = gl_pixbuf_cache_get_pixbuf (pixbuf_cache, name);
	if ( pixbuf != NULL ) {

		pixdata = g_new0 (GdkPixdata, 1);
		gdk_pixdata_from_pixbuf (pixdata, pixbuf, FALSE);
		stream = gdk_pixdata_serialize (pixdata, &stream_length);
		base64 = g_base64_encode (stream, stream_length);

		node = xmlNewChild (parent, ns, (xmlChar *)"Pixdata", (xmlChar *)base64);
		lgl_xml_set_prop_string (node, "name", name);
		lgl_xml_set_prop_string (node, "encoding", "Base64");

		gl_pixbuf_cache_remove_pixbuf (pixbuf_cache, name);

		g_free (pixdata);
		g_free (stream);
		g_free (base64);
	}


	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label Data embedded SVG file Node                      */
/*--------------------------------------------------------------------------*/
static void
xml_create_file_svg (xmlDocPtr   doc,
                     xmlNodePtr  parent,
                     xmlNsPtr    ns,
                     glLabel    *label,
                     gchar      *name)
{
	xmlNodePtr  node;
	xmlNodePtr  cdata_section_node;
	GHashTable *svg_cache;
        gchar      *svg_data;

	gl_debug (DEBUG_XML, "START");

	svg_cache = gl_label_get_svg_cache (label);

	svg_data = gl_svg_cache_get_contents (svg_cache, name);
	if ( svg_data != NULL ) {

		node = xmlNewChild (parent, ns, (xmlChar *)"File", NULL);
		lgl_xml_set_prop_string (node, "name", name);
		lgl_xml_set_prop_string (node, "format", "SVG");

                cdata_section_node = xmlNewCDataBlock (doc, (xmlChar *)svg_data, strlen (svg_data));
                xmlAddChild (node, cdata_section_node);

		g_free (svg_data);
	}


	gl_debug (DEBUG_XML, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Create top-level Span node.                                    */
/*--------------------------------------------------------------------------*/
static void
xml_create_toplevel_span (xmlNodePtr        parent,
			  xmlNsPtr          ns,
			  glLabelText      *object_text)
{
	xmlNodePtr        node;
	gchar            *font_family;
	gdouble           font_size;
	PangoWeight       font_weight;
	gboolean          font_italic_flag;
	glColorNode      *color_node;
	PangoAlignment    align;
	gdouble           text_line_spacing;
	GList            *lines, *p_line, *p_node;
	glTextNode       *text_node;
	xmlNodePtr        child;

	node = xmlNewChild (parent, ns, (xmlChar *)"Span", NULL);

	/* All span attrs at top level. */
	font_family = gl_label_object_get_font_family (GL_LABEL_OBJECT(object_text));
	font_size = gl_label_object_get_font_size (GL_LABEL_OBJECT(object_text));
	text_line_spacing = gl_label_object_get_text_line_spacing (GL_LABEL_OBJECT(object_text));
	font_weight = gl_label_object_get_font_weight (GL_LABEL_OBJECT(object_text));
	font_italic_flag = gl_label_object_get_font_italic_flag (GL_LABEL_OBJECT(object_text));
	
	color_node = gl_label_object_get_text_color (GL_LABEL_OBJECT(object_text));
	if (color_node->field_flag)
	{
		lgl_xml_set_prop_string (node, "color_field", color_node->key);
	}
	else
	{
		lgl_xml_set_prop_uint_hex (node, "color", color_node->color);
	}
	gl_color_node_free (&color_node);
	
	align = gl_label_object_get_text_alignment (GL_LABEL_OBJECT(object_text));
	lgl_xml_set_prop_string (node, "font_family", font_family);
	lgl_xml_set_prop_double (node, "font_size", font_size);
	lgl_xml_set_prop_string (node, "font_weight", gl_str_util_weight_to_string (font_weight));
	lgl_xml_set_prop_boolean (node, "font_italic", font_italic_flag);
	
	lgl_xml_set_prop_double (node, "line_spacing", text_line_spacing);

	/* Build children. */
	lines = gl_label_text_get_lines (GL_LABEL_TEXT(object_text));
	for (p_line = lines; p_line != NULL; p_line = p_line->next) {

		for (p_node = (GList *) p_line->data; p_node != NULL;
		     p_node = p_node->next) {
			text_node = (glTextNode *) p_node->data;

			if (text_node->field_flag) {
				child = xmlNewChild (node, ns, (xmlChar *)"Field", NULL);
				lgl_xml_set_prop_string (child, "name", text_node->data);
			} else {
				xmlNodeAddContent (node, (xmlChar *)text_node->data);
			}

		}

		if ( p_line->next ) {
			child = xmlNewChild (node, ns, (xmlChar *)"NL", NULL);
		}

	}

	gl_text_node_lines_free (&lines);
	g_free (font_family);

}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Create affine attributes.                                      */
/*--------------------------------------------------------------------------*/
static void
xml_create_affine_attrs (xmlNodePtr        node,
			 glLabelObject    *object)
{
        cairo_matrix_t matrix;

	gl_label_object_get_matrix (object, &matrix);

	lgl_xml_set_prop_double (node, "a0", matrix.xx);
	lgl_xml_set_prop_double (node, "a1", matrix.yx);
	lgl_xml_set_prop_double (node, "a2", matrix.xy);
	lgl_xml_set_prop_double (node, "a3", matrix.yy);
	lgl_xml_set_prop_double (node, "a4", matrix.x0);
	lgl_xml_set_prop_double (node, "a5", matrix.y0);
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Create shadow attributes.                                      */
/*--------------------------------------------------------------------------*/
static void
xml_create_shadow_attrs (xmlNodePtr        node,
			 glLabelObject    *object)
{
	gboolean          shadow_state;
	gdouble           shadow_x;
	gdouble           shadow_y;
	glColorNode      *shadow_color_node;
	gdouble           shadow_opacity;

	shadow_state = gl_label_object_get_shadow_state (object);

	if (shadow_state)
	{
		lgl_xml_set_prop_boolean (node, "shadow", shadow_state);

		gl_label_object_get_shadow_offset (object, &shadow_x, &shadow_y);
		lgl_xml_set_prop_length (node, "shadow_x", shadow_x);
		lgl_xml_set_prop_length (node, "shadow_y", shadow_y);
		
		shadow_color_node = gl_label_object_get_shadow_color (object);
		if (shadow_color_node->field_flag)
		{
			lgl_xml_set_prop_string (node, "shadow_color_field", shadow_color_node->key);
		}
		else
		{
			lgl_xml_set_prop_uint_hex (node, "shadow_color", shadow_color_node->color);
		}
		gl_color_node_free (&shadow_color_node);

		shadow_opacity = gl_label_object_get_shadow_opacity (object);
		lgl_xml_set_prop_double (node, "shadow_opacity", shadow_opacity);
	}
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
