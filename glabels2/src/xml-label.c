/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label.c:  GLabels xml label module
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
#include "template.h"
#include "base64.h"
#include "xml-label.h"
#include "xml-label-04.h"
#include "xml-template.h"
#include "xml.h"
#include "util.h"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/
#define NAME_SPACE "http://snaught.com/glabels/1.92/"
#define COMPAT01_NAME_SPACE "http://snaught.com/glabels/0.1/"
#define COMPAT04_NAME_SPACE "http://snaught.com/glabels/0.4/"

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

static void           xml_parse_object         (xmlNodePtr        node,
						glLabel          *label);

static glLabelObject *xml_parse_text_props     (xmlNodePtr        node,
						glLabel          *label);

static glLabelObject *xml_parse_box_props      (xmlNodePtr        node,
						glLabel          *label);

static glLabelObject *xml_parse_line_props     (xmlNodePtr        node,
						glLabel          *label);

static glLabelObject *xml_parse_ellipse_props  (xmlNodePtr        node,
						glLabel          *label);

static glLabelObject *xml_parse_image_props    (xmlNodePtr        node,
					        glLabel          *label);

static glLabelObject *xml_parse_barcode_props  (xmlNodePtr        node,
						glLabel          *label);

static void           xml_parse_merge_fields   (xmlNodePtr        node,
						glLabel          *label);

static void           xml_parse_data           (xmlNodePtr        node,
						glLabel          *label);

static void           xml_parse_pixdata        (xmlNodePtr        node,
						glLabel          *label);


static xmlDocPtr      xml_label_to_doc         (glLabel          *label,
						glXMLLabelStatus *status);

static void           xml_create_objects       (xmlNodePtr        root,
						xmlNsPtr          ns,
						glLabel          *label);

static void           xml_create_object        (xmlNodePtr        root,
						xmlNsPtr          ns,
						glLabelObject    *object);

static void           xml_create_text_props    (xmlNodePtr        root,
						xmlNsPtr          ns,
						glLabelObject    *object);

static void           xml_create_box_props     (xmlNodePtr        root,
						xmlNsPtr          ns,
						glLabelObject    *object);

static void           xml_create_line_props    (xmlNodePtr        root,
						xmlNsPtr          ns,
						glLabelObject    *object);

static void           xml_create_ellipse_props (xmlNodePtr        root,
						xmlNsPtr          ns,
						glLabelObject    *object);

static void           xml_create_image_props   (xmlNodePtr        root,
						xmlNsPtr          ns,
						glLabelObject    *object);

static void           xml_create_barcode_props (xmlNodePtr        root,
						xmlNsPtr          ns,
						glLabelObject    *object);

static void           xml_create_merge_fields  (xmlNodePtr        root,
						xmlNsPtr          ns,
						glLabel          *label);

static void           xml_create_data          (xmlNodePtr        root,
						xmlNsPtr          ns,
						glLabel          *label);

static void           xml_create_pixdata       (xmlNodePtr        root,
						xmlNsPtr          ns,
						glLabel          *label,
						gchar            *name);


/****************************************************************************/
/* Open and read label from xml file.                                       */
/****************************************************************************/
glLabel *
gl_xml_label_open (const gchar      *filename,
		   glXMLLabelStatus *status)
{
	xmlDocPtr  doc;
	glLabel   *label;

	gl_debug (DEBUG_XML, "START");

	doc = xmlParseFile (filename);
	if (!doc) {
		g_warning (_("xmlParseFile error"));
		*status = XML_LABEL_ERROR_OPEN_PARSE;
		return NULL;
	}

	label = xml_doc_to_label (doc, status);

	xmlFreeDoc (doc);

	if (label) {
		gl_label_set_filename (label, filename);
		gl_label_clear_modified (label);
	}

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

	doc = xmlParseDoc ((xmlChar *) buffer);
	if (!doc) {
		g_warning (_("xmlParseFile error"));
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
	xmlNodePtr  root, node;
	xmlNsPtr    ns;
	glLabel    *label;

	gl_debug (DEBUG_XML, "START");

	LIBXML_TEST_VERSION;

	*status = XML_LABEL_OK;

	root = xmlDocGetRootElement (doc);
	if (!root || !root->name) {
		g_warning (_("No document root"));
		*status = XML_LABEL_ERROR_OPEN_PARSE;
		return NULL;
	}

	ns = xmlSearchNsByHref (doc, root, NAME_SPACE);
	if (ns != NULL) {
		label = xml_parse_label (root, status);
	} else {
		/* Try compatability mode 0.1 */
		ns = xmlSearchNsByHref (doc, root, COMPAT01_NAME_SPACE);
		if (ns != NULL)	{
			g_warning (_("Importing from glabels 0.1 format"));
			g_warning ("TODO");
			label = NULL; /* TODO */
		} else {
			/* Try compatability mode 0.4 */
			ns = xmlSearchNsByHref (doc, root,
						COMPAT04_NAME_SPACE);
			if (ns != NULL)	{
				g_warning (_("Importing from glabels 0.4 format"));
				label = gl_xml_label_04_parse (root, status);
			} else {
				g_warning (_("bad document, unknown glabels Namespace"));
				*status = XML_LABEL_ERROR_OPEN_PARSE;
				return NULL;
			}
		}
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
			xml_parse_data (node, label);
		}
	}

	/* Pass 2, now extract everything else. */
	for (node = root->xmlChildrenNode; node != NULL; node = node->next) {

		if (xmlStrEqual (node->name, "Sheet")) {
			template = gl_xml_template_parse_sheet (node);
			if (!template) {
				*status = XML_LABEL_UNKNOWN_MEDIA;
				return NULL;
			}
			gl_label_set_template (label, template);
			gl_template_free (&template);
		} else if (xmlStrEqual (node->name, "Objects")) {
			xml_parse_objects (node, label);
		} else if (xmlStrEqual (node->name, "Merge_Fields")) {
			xml_parse_merge_fields (node, label);
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
xml_parse_objects (xmlNodePtr  objects_node,
		   glLabel    *label)
{
	gboolean    rotate_flag;
	xmlNodePtr  node;

	gl_debug (DEBUG_XML, "START");

	rotate_flag = gl_xml_get_prop_boolean (objects_node, "rotate", FALSE);
	gl_label_set_rotate_flag (label, rotate_flag);

	for (node = objects_node->xmlChildrenNode; node != NULL; node = node->next) {

		if (xmlStrEqual (node->name, "Object")) {
			xml_parse_object (node, label);
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
xml_parse_object (xmlNodePtr  object_node,
		  glLabel    *label)
{
	glLabelObject *object;
	gdouble        x, y;
	gchar         *string;
	gdouble        affine[6];

	gl_debug (DEBUG_XML, "START");

	string = xmlGetProp (object_node, "type");

	if ( xmlStrEqual (string, "Text") ) {
		object = xml_parse_text_props (object_node, label);
	} else if ( xmlStrEqual (string, "Box") ) {
		object = xml_parse_box_props (object_node, label);
	} else if ( xmlStrEqual (string, "Line") ) {
		object = xml_parse_line_props (object_node, label);
	} else if ( xmlStrEqual (string, "Ellipse") ) {
		object = xml_parse_ellipse_props (object_node, label);
	} else if ( xmlStrEqual (string, "Image") ) {
		object = xml_parse_image_props (object_node, label);
	} else if ( xmlStrEqual (string, "Barcode") ) {
		object = xml_parse_barcode_props (object_node, label);
	} else {
		g_warning ("Unknown label object type \"%s\"", string);
		g_free (string);
		return;
	}
	g_free (string);


	x = gl_xml_get_prop_double (object_node, "x", 0.0);
	y = gl_xml_get_prop_double (object_node, "y", 0.0);

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
xml_parse_text_props (xmlNodePtr  object_node,
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

	w = gl_xml_get_prop_double (object_node, "w", 0);
	h = gl_xml_get_prop_double (object_node, "h", 0);

	font_family = xmlGetProp (object_node, "font_family");

	font_size = gl_xml_get_prop_double (object_node, "font_size", 0.0);

	string = xmlGetProp (object_node, "font_weight");
	font_weight = gl_util_string_to_weight (string);
	g_free (string);

	font_italic_flag = gl_xml_get_prop_boolean (object_node, "font_italic", FALSE);

	string = xmlGetProp (object_node, "justify");
	just = gl_util_string_to_just (string);
	g_free (string);

	color = gl_xml_get_prop_uint_hex (object_node, "color", 0);

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
				} else if (!xmlNodeIsText) {
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
	gl_label_text_set_props  (GL_LABEL_TEXT(object),
				  font_family, font_size, font_weight,
				  font_italic_flag,
				  color, just);

	gl_text_node_lines_free (&lines);
	g_free (font_family);

	gl_debug (DEBUG_XML, "END");

	return GL_LABEL_OBJECT(object);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Box Node Properties                           */
/*--------------------------------------------------------------------------*/
static glLabelObject *
xml_parse_box_props (xmlNodePtr  node,
		     glLabel    *label)
{
	GObject *object;
	gdouble  line_width;
	guint    line_color, fill_color;
	gdouble  w, h;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_box_new (label);

	w = gl_xml_get_prop_double (node, "w", 0);
	h = gl_xml_get_prop_double (node, "h", 0);

	line_width = gl_xml_get_prop_double (node, "line_width", 1.0);

	line_color = gl_xml_get_prop_uint_hex (node, "line_color", 0);
	fill_color = gl_xml_get_prop_uint_hex (node, "fill_color", 0);

	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);
	gl_label_box_set_line_width (GL_LABEL_BOX(object), line_width);
	gl_label_box_set_line_color (GL_LABEL_BOX(object), line_color);
	gl_label_box_set_fill_color (GL_LABEL_BOX(object), fill_color);

	gl_debug (DEBUG_XML, "END");

	return GL_LABEL_OBJECT(object);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Line Node Properties                          */
/*--------------------------------------------------------------------------*/
static glLabelObject *
xml_parse_line_props (xmlNodePtr  node,
		      glLabel    *label)
{
	GObject *object;
	gdouble  line_width;
	guint    line_color;
	gdouble  w, h;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_line_new (label);

	w = gl_xml_get_prop_double (node, "dx", 0);
	h = gl_xml_get_prop_double (node, "dy", 0);

	line_width = gl_xml_get_prop_double (node, "line_width", 1.0);

	line_color = gl_xml_get_prop_uint_hex (node, "line_color", 0);

	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);
	gl_label_line_set_line_width (GL_LABEL_LINE(object), line_width);
	gl_label_line_set_line_color (GL_LABEL_LINE(object), line_color);

	gl_debug (DEBUG_XML, "END");

	return GL_LABEL_OBJECT(object);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Ellipse Node Properties                       */
/*--------------------------------------------------------------------------*/
static glLabelObject *
xml_parse_ellipse_props (xmlNodePtr  node,
			 glLabel    *label)
{
	GObject *object;
	gdouble  line_width;
	guint    line_color, fill_color;
	gdouble  w, h;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_ellipse_new (label);

	w = gl_xml_get_prop_double (node, "w", 0);
	h = gl_xml_get_prop_double (node, "h", 0);

	line_width = gl_xml_get_prop_double (node, "line_width", 1.0);

	line_color = gl_xml_get_prop_uint_hex (node, "line_color", 0);
	fill_color = gl_xml_get_prop_uint_hex (node, "fill_color", 0);

	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);
	gl_label_ellipse_set_line_width (GL_LABEL_ELLIPSE(object), line_width);
	gl_label_ellipse_set_line_color (GL_LABEL_ELLIPSE(object), line_color);
	gl_label_ellipse_set_fill_color (GL_LABEL_ELLIPSE(object), fill_color);

	gl_debug (DEBUG_XML, "END");

	return GL_LABEL_OBJECT(object);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Image Node Properties                         */
/*--------------------------------------------------------------------------*/
static glLabelObject *
xml_parse_image_props (xmlNodePtr  node,
		       glLabel    *label)
{
	GObject      *object;
	xmlNodePtr    child;
	gdouble       w, h;
	glTextNode   *filename;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_image_new (label);

	w = gl_xml_get_prop_double (node, "w", 0);
	h = gl_xml_get_prop_double (node, "h", 0);

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
xml_parse_barcode_props (xmlNodePtr  node,
			 glLabel    *label)
{
	GObject            *object;
	xmlNodePtr          child;
	gdouble             w, h;
	gchar              *string;
	glTextNode         *text_node;
	glBarcodeStyle      style;
	gboolean            text_flag;
	gboolean            checksum_flag;
	guint               color;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_barcode_new (label);

	w = gl_xml_get_prop_double (node, "w", 0);
	h = gl_xml_get_prop_double (node, "h", 0);

	color = gl_xml_get_prop_uint_hex (node, "color", 0);

	string = xmlGetProp (node, "style");
	style = gl_barcode_text_to_style (string);
	g_free (string);

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
				    style, text_flag, checksum_flag, color);

	gl_text_node_free (&text_node);

	gl_debug (DEBUG_XML, "END");

	return GL_LABEL_OBJECT(object);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML merge fields tag.                                */
/*--------------------------------------------------------------------------*/
static void
xml_parse_merge_fields (xmlNodePtr  node,
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
xml_parse_data (xmlNodePtr  node,
		glLabel    *label)
{
	xmlNodePtr  child;

	gl_debug (DEBUG_XML, "START");

	for (child = node->xmlChildrenNode; child != NULL; child = child->next) {

		if (xmlStrEqual (child->name, "Pixdata")) {
			xml_parse_pixdata (child, label);
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
xml_parse_pixdata (xmlNodePtr  node,
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

/****************************************************************************/
/* Save label to xml label file.                                            */
/****************************************************************************/
void
gl_xml_label_save (glLabel          *label,
		   const gchar      *filename,
		   glXMLLabelStatus *status)
{
	xmlDocPtr doc;
	gint      xml_ret;

	gl_debug (DEBUG_XML, "START");

	doc = xml_label_to_doc (label, status);

	xmlSetDocCompressMode (doc, 9);
	xml_ret = xmlSaveFormatFile (filename, doc, TRUE);
	xmlFreeDoc (doc);
	if (xml_ret == -1) {

		g_warning (_("Problem saving xml file."));
		*status = XML_LABEL_ERROR_SAVE_FILE;

	} else {

		gl_label_set_filename (label, filename);
		gl_label_clear_modified (label);

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
	gchar     *buffer;

	gl_debug (DEBUG_XML, "START");

	doc = xml_label_to_doc (label, status);

	xmlDocDumpMemory (doc, (xmlChar **)&buffer, &size);
	xmlFreeDoc (doc);

	gl_label_clear_modified (label);

	gl_debug (DEBUG_XML, "END");

	return buffer;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Convert label to xml doc structure.                            */
/*--------------------------------------------------------------------------*/
static xmlDocPtr
xml_label_to_doc (glLabel          *label,
		  glXMLLabelStatus *status)
{
	xmlDocPtr   doc;
	xmlNsPtr    ns;
	glTemplate *template;
	glMerge    *merge;

	gl_debug (DEBUG_XML, "START");

	LIBXML_TEST_VERSION;

	doc = xmlNewDoc ("1.0");
	doc->xmlRootNode = xmlNewDocNode (doc, NULL, "Document", NULL);

	ns = xmlNewNs (doc->xmlRootNode, NAME_SPACE, "glabels");
	xmlSetNs (doc->xmlRootNode, ns);

	template = gl_label_get_template (label);
	gl_xml_template_add_sheet (template, doc->xmlRootNode, ns);

	xml_create_objects (doc->xmlRootNode, ns, label);

	merge = gl_label_get_merge (label);
	gl_debug (DEBUG_XML, "merge=%p", merge);
	if (merge != NULL) {
		xml_create_merge_fields (doc->xmlRootNode, ns, label);
		g_object_unref (G_OBJECT(merge));
	}

	xml_create_data (doc->xmlRootNode, ns, label);

	gl_debug (DEBUG_XML, "END");

	*status = XML_LABEL_OK;
	return doc;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label->Objects Node                                    */
/*--------------------------------------------------------------------------*/
static void
xml_create_objects (xmlNodePtr  root,
		    xmlNsPtr    ns,
		    glLabel    *label)
{
	xmlNodePtr  node;
	gboolean    rotate_flag;
	GList      *p;

	gl_debug (DEBUG_XML, "START");

	rotate_flag = gl_label_get_rotate_flag (label);

	node = xmlNewChild (root, ns, "Objects", NULL);
	xmlSetProp (node, "id", "0");
	gl_xml_set_prop_boolean (node, "rotate", rotate_flag);

	for (p = label->objects; p != NULL; p = p->next) {
		xml_create_object (node, ns, GL_LABEL_OBJECT(p->data));
	}

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML label object Node                                      */
/*--------------------------------------------------------------------------*/
static void
xml_create_object (xmlNodePtr     root,
		   xmlNsPtr       ns,
		   glLabelObject *object)
{
	xmlNodePtr  object_node;
	gdouble     x, y;
	gdouble     affine[6];

	gl_debug (DEBUG_XML, "START");

	object_node = xmlNewChild (root, ns, "Object", NULL);

	gl_label_object_get_position (object, &x, &y);
	gl_xml_set_prop_double (object_node, "x", x);
	gl_xml_set_prop_double (object_node, "y", y);

	gl_label_object_get_affine (object, affine);
	gl_xml_set_prop_double (object_node, "a0", affine[0]);
	gl_xml_set_prop_double (object_node, "a1", affine[1]);
	gl_xml_set_prop_double (object_node, "a2", affine[2]);
	gl_xml_set_prop_double (object_node, "a3", affine[3]);
	gl_xml_set_prop_double (object_node, "a4", affine[4]);
	gl_xml_set_prop_double (object_node, "a5", affine[5]);

	if ( GL_IS_LABEL_TEXT(object) ) {
		xml_create_text_props (object_node, ns, object);
	} else if ( GL_IS_LABEL_BOX(object) ) {
		xml_create_box_props (object_node, ns, object);
	} else if ( GL_IS_LABEL_LINE(object) ) {
		xml_create_line_props (object_node, ns, object);
	} else if ( GL_IS_LABEL_ELLIPSE(object) ) {
		xml_create_ellipse_props (object_node, ns, object);
	} else if ( GL_IS_LABEL_IMAGE(object) ) {
		xml_create_image_props (object_node, ns, object);
	} else if ( GL_IS_LABEL_BARCODE(object) ) {
		xml_create_barcode_props (object_node, ns, object);
	} else {
		g_warning ("Unknown label object");
	}

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label->Text Node Properties                            */
/*--------------------------------------------------------------------------*/
static void
xml_create_text_props (xmlNodePtr     object_node,
		       xmlNsPtr       ns,
		       glLabelObject *object)
{
	xmlNodePtr        line_node, field_node, literal_node;
	GList            *lines;
	gdouble           w, h;
	gchar            *font_family;
	gdouble           font_size;
	GnomeFontWeight   font_weight;
	gboolean          font_italic_flag;
	guint             color;
	GtkJustification  just;
	GList            *p_line, *p_node;
	glTextNode       *node_text;

	gl_debug (DEBUG_XML, "START");

	xmlSetProp (object_node, "type", "Text");

	gl_label_text_get_box ( GL_LABEL_TEXT(object), &w, &h);
	lines = gl_label_text_get_lines (GL_LABEL_TEXT(object));
	gl_label_text_get_props (GL_LABEL_TEXT(object),
				 &font_family, &font_size, &font_weight,
				 &font_italic_flag,
				 &color, &just);

	gl_xml_set_prop_double (object_node, "w", w);
	gl_xml_set_prop_double (object_node, "h", h);

	xmlSetProp (object_node, "font_family", font_family);
	gl_xml_set_prop_double (object_node, "font_size", font_size);
	xmlSetProp (object_node, "font_weight",
		    gl_util_weight_to_string (font_weight));
	gl_xml_set_prop_boolean (object_node, "font_italic", font_italic_flag);

	xmlSetProp (object_node, "justify", gl_util_just_to_string (just));

	gl_xml_set_prop_uint_hex (object_node, "color", color);

	for (p_line = lines; p_line != NULL; p_line = p_line->next) {
		line_node = xmlNewChild (object_node, ns, "Line", NULL);

		for (p_node = (GList *) p_line->data; p_node != NULL;
		     p_node = p_node->next) {
			node_text = (glTextNode *) p_node->data;

			if (node_text->field_flag) {
				field_node =
				    xmlNewChild (line_node, ns, "Field", NULL);
				xmlSetProp (field_node, "name",
					    node_text->data);
			} else {
				literal_node =
				    xmlNewChild (line_node, ns,
						 "Literal", node_text->data);
			}

		}

	}

	gl_text_node_lines_free (&lines);
	g_free (font_family);

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label->Box Node Properties                             */
/*--------------------------------------------------------------------------*/
static void
xml_create_box_props (xmlNodePtr     object_node,
		      xmlNsPtr       ns,
		      glLabelObject *object)
{
	gdouble  line_width;
	guint    line_color, fill_color;
	gdouble  w, h;

	gl_debug (DEBUG_XML, "START");

	xmlSetProp (object_node, "type", "Box");

	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_box_get_line_width (GL_LABEL_BOX(object));
	line_color = gl_label_box_get_line_color (GL_LABEL_BOX(object));
	fill_color = gl_label_box_get_fill_color (GL_LABEL_BOX(object));

	gl_xml_set_prop_double (object_node, "w", w);
	gl_xml_set_prop_double (object_node, "h", h);

	gl_xml_set_prop_double (object_node, "line_width", line_width);

	gl_xml_set_prop_uint_hex (object_node, "line_color", line_color);
	gl_xml_set_prop_uint_hex (object_node, "fill_color", fill_color);

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label->Line Node Properties                            */
/*--------------------------------------------------------------------------*/
static void
xml_create_line_props (xmlNodePtr     object_node,
		       xmlNsPtr       ns,
		       glLabelObject *object)
{
	gdouble  line_width;
	guint    line_color;
	gdouble  w, h;

	gl_debug (DEBUG_XML, "START");

	xmlSetProp (object_node, "type", "Line");

	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_line_get_line_width (GL_LABEL_LINE(object));
	line_color = gl_label_line_get_line_color (GL_LABEL_LINE(object));

	gl_xml_set_prop_double (object_node, "dx", w);
	gl_xml_set_prop_double (object_node, "dy", h);

	gl_xml_set_prop_double (object_node, "line_width", line_width);

	gl_xml_set_prop_uint_hex (object_node, "line_color", line_color);

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label->Ellipse Node Properties                         */
/*--------------------------------------------------------------------------*/
static void
xml_create_ellipse_props (xmlNodePtr     object_node,
			  xmlNsPtr       ns,
			  glLabelObject *object)
{
	gdouble  line_width;
	guint    line_color, fill_color;
	gdouble  w, h;

	gl_debug (DEBUG_XML, "START");

	xmlSetProp (object_node, "type", "Ellipse");

	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_ellipse_get_line_width (GL_LABEL_ELLIPSE(object));
	line_color = gl_label_ellipse_get_line_color (GL_LABEL_ELLIPSE(object));
	fill_color = gl_label_ellipse_get_fill_color (GL_LABEL_ELLIPSE(object));

	gl_xml_set_prop_double (object_node, "w", w);
	gl_xml_set_prop_double (object_node, "h", h);

	gl_xml_set_prop_double (object_node, "line_width", line_width);

	gl_xml_set_prop_uint_hex (object_node, "line_color", line_color);
	gl_xml_set_prop_uint_hex (object_node, "fill_color", fill_color);

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label->Image Node Properties                           */
/*--------------------------------------------------------------------------*/
static void
xml_create_image_props (xmlNodePtr     object_node,
			xmlNsPtr       ns,
			glLabelObject *object)
{
	gdouble     w, h;
	glTextNode *filename;
	xmlNodePtr  child;

	gl_debug (DEBUG_XML, "START");

	xmlSetProp (object_node, "type", "Image");

	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	filename = gl_label_image_get_filename (GL_LABEL_IMAGE(object));

	gl_xml_set_prop_double (object_node, "w", w);
	gl_xml_set_prop_double (object_node, "h", h);

	if (filename->field_flag) {
		child = xmlNewChild (object_node, ns, "Field", NULL);
		xmlSetProp (child, "name", filename->data);
	} else {
		child = xmlNewChild (object_node, ns, "File", NULL);
		xmlSetProp (child, "src", filename->data);
	}

	gl_text_node_free (&filename);

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label->Barcode Node Properties                         */
/*--------------------------------------------------------------------------*/
static void
xml_create_barcode_props (xmlNodePtr     object_node,
			  xmlNsPtr       ns,
			  glLabelObject *object)
{
	gdouble     w, h;
	glTextNode          *text_node;
	glBarcodeStyle      style;
	gboolean            text_flag;
	gboolean            checksum_flag;
	guint               color;
	xmlNodePtr          child;

	gl_debug (DEBUG_XML, "START");

	xmlSetProp (object_node, "type", "Barcode");

	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
               
	gl_xml_set_prop_double (object_node, "w", w);
	gl_xml_set_prop_double (object_node, "h", h);
               		

	text_node = gl_label_barcode_get_data (GL_LABEL_BARCODE(object));
	gl_label_barcode_get_props (GL_LABEL_BARCODE(object),
				    &style, &text_flag, &checksum_flag, &color);

	gl_xml_set_prop_uint_hex (object_node, "color", color);

	xmlSetProp (object_node, "style", gl_barcode_style_to_text (style));
	gl_xml_set_prop_boolean (object_node, "text", text_flag);
	gl_xml_set_prop_boolean (object_node, "checksum", checksum_flag);

	if (text_node->field_flag) {
		child = xmlNewChild (object_node, ns, "Field", NULL);
		xmlSetProp (child, "name", text_node->data);
	} else {
		child = xmlNewChild (object_node, ns, "Literal", text_node->data);
	}

	gl_text_node_free (&text_node);

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label Merge Fields Node                                */
/*--------------------------------------------------------------------------*/
static void
xml_create_merge_fields (xmlNodePtr  root,
			 xmlNsPtr    ns,
			 glLabel    *label)
{
	xmlNodePtr  node, child;
	gchar      *string;
	GList      *p;
	glMerge    *merge;

	gl_debug (DEBUG_XML, "START");

	merge = gl_label_get_merge (label);

	node = xmlNewChild (root, ns, "Merge_Fields", NULL);

	string = gl_merge_get_name (merge);
	xmlSetProp (node, "type", string);
	g_free (string);

	string = gl_merge_get_src (merge);
	xmlSetProp (node, "src", string);
	g_free (string);

	g_object_unref (G_OBJECT(merge));

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label Data Node                                        */
/*--------------------------------------------------------------------------*/
static void
xml_create_data (xmlNodePtr  root,
		 xmlNsPtr    ns,
		 glLabel    *label)
{
	xmlNodePtr  node;
	GList      *name_list, *p;
	GHashTable *pixbuf_cache;

	gl_debug (DEBUG_XML, "START");

	node = xmlNewChild (root, ns, "Data", NULL);

	pixbuf_cache = gl_label_get_pixbuf_cache (label);
	name_list = gl_pixbuf_cache_get_name_list (pixbuf_cache);

	for (p = name_list; p != NULL; p=p->next) {
		xml_create_pixdata (node, ns, label, p->data);
	}

	gl_pixbuf_cache_free_name_list (name_list);


	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label Data Pixbuf Node                                 */
/*--------------------------------------------------------------------------*/
static void
xml_create_pixdata (xmlNodePtr  root,
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
		gdk_pixdata_from_pixbuf (pixdata, pixbuf, TRUE);
		stream = gdk_pixdata_serialize (pixdata, &stream_length);
		base64 = gl_base64_encode (stream, stream_length);

		node = xmlNewChild (root, ns, "Pixdata", base64);
		xmlSetProp (node, "name", name);
		xmlSetProp (node, "encoding", "base64");

		gl_pixbuf_cache_remove_pixbuf (pixbuf_cache, name);

		g_free (pixdata->pixel_data);
		g_free (pixdata);
		g_free (stream);
		g_free (base64);
	}


	gl_debug (DEBUG_XML, "END");
}

