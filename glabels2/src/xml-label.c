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

#include "label.h"
#include "label-object.h"
#include "label-text.h"
#include "label-box.h"
#include "label-line.h"
#include "label-ellipse.h"
#include "label-image.h"
#include "label-barcode.h"
#include "template.h"
#include "xml-label.h"
#include "xml-label-04.h"
#include "util.h"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/
#define NAME_SPACE "http://snaught.com/glabels/1.90/"
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

static glLabel *xml_doc_to_label (xmlDocPtr doc, glXMLLabelStatus *status);
static glLabel *xml_parse_label (xmlNodePtr root, glXMLLabelStatus *status);
static void xml_parse_objects (xmlNodePtr node, glLabel * label);
static void xml_parse_object(xmlNodePtr node, glLabel *label);
static glLabelObject *xml_parse_text_props (xmlNodePtr node, glLabel *label);
static glLabelObject *xml_parse_box_props (xmlNodePtr node, glLabel *label);
static glLabelObject *xml_parse_line_props (xmlNodePtr node, glLabel *label);
static glLabelObject *xml_parse_ellipse_props (xmlNodePtr node, glLabel *label);
static glLabelObject *xml_parse_image_props (xmlNodePtr node, glLabel *label);
static glLabelObject *xml_parse_barcode_props (xmlNodePtr node, glLabel *label);
static void xml_parse_merge_fields (xmlNodePtr node, glLabel *label);

static xmlDocPtr xml_label_to_doc (glLabel * label, glXMLLabelStatus *status);
static void xml_create_objects (xmlNodePtr root, xmlNsPtr ns, glLabel * label);
static void xml_create_object (xmlNodePtr root, xmlNsPtr ns,
			       glLabelObject * object);
static void xml_create_text_props (xmlNodePtr root, xmlNsPtr ns,
				 glLabelObject * object);
static void xml_create_box_props (xmlNodePtr root, xmlNsPtr ns,
				glLabelObject * object);
static void xml_create_line_props (xmlNodePtr root, xmlNsPtr ns,
				 glLabelObject * object);
static void xml_create_ellipse_props (xmlNodePtr root, xmlNsPtr ns,
				    glLabelObject * object);
static void xml_create_image_props (xmlNodePtr root, xmlNsPtr ns,
				  glLabelObject * object);
static void xml_create_barcode_props (xmlNodePtr root, xmlNsPtr ns,
				    glLabelObject * object);
static void xml_create_merge_fields (xmlNodePtr root, xmlNsPtr ns,
				      glLabel * label);

/****************************************************************************/
/* Open and read label from xml file.                                       */
/****************************************************************************/
glLabel *
gl_xml_label_open (const gchar * filename,
		   glXMLLabelStatus *status)
{
	xmlDocPtr doc;
	glLabel *label;

	gl_debug (DEBUG_XML, "START");

	doc = xmlParseFile (filename);
	if (!doc) {
		g_warning (_("xmlParseFile error"));
		*status = XML_LABEL_ERROR_OPEN_PARSE;
		return NULL;
	}

	label = xml_doc_to_label (doc, status);

	xmlFreeDoc (doc);

	gl_label_set_filename (label, filename);
	gl_label_clear_modified (label);

	gl_debug (DEBUG_XML, "END");

	return label;
}

/****************************************************************************/
/* Read label from xml buffer.                                              */
/****************************************************************************/
glLabel *
gl_xml_label_open_buffer (const gchar * buffer,
			  glXMLLabelStatus *status)
{
	xmlDocPtr doc;
	glLabel *label;

	gl_debug (DEBUG_XML, "START");

	doc = xmlParseDoc ((xmlChar *) buffer);
	if (!doc) {
		g_warning (_("xmlParseFile error"));
		*status = XML_LABEL_ERROR_OPEN_PARSE;
		return NULL;
	}

	label = xml_doc_to_label (doc, status);

	gl_label_clear_modified (label);

	xmlFreeDoc (doc);

	gl_debug (DEBUG_XML, "END");

	return label;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse xml doc structure and create label.                      */
/*--------------------------------------------------------------------------*/
static glLabel *
xml_doc_to_label (xmlDocPtr doc,
		  glXMLLabelStatus *status)
{
	xmlNodePtr root, node;
	xmlNsPtr ns;
	glLabel *label;

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
xml_parse_label (xmlNodePtr       root,
		 glXMLLabelStatus *status)
{
	xmlNodePtr node;
	glLabel *label;
	glTemplate *template;

	gl_debug (DEBUG_XML, "START");

	*status = XML_LABEL_OK;

	if (g_strcasecmp (root->name, "Document") != 0) {
		g_warning (_("Bad root node = \"%s\""), root->name);
		*status = XML_LABEL_ERROR_OPEN_PARSE;
		return NULL;
	}

	label = GL_LABEL(gl_label_new ());

	for (node = root->xmlChildrenNode; node != NULL; node = node->next) {

		if (g_strcasecmp (node->name, "Sheet") == 0) {
			template = gl_template_xml_parse_sheet (node);
			if (!template) {
				*status = XML_LABEL_UNKNOWN_MEDIA;
				return NULL;
			}
			gl_label_set_template (label, template);
			gl_template_free (&template);
		} else if (g_strcasecmp (node->name, "Objects") == 0) {
			xml_parse_objects (node, label);
		} else if (g_strcasecmp (node->name, "Merge_Fields") == 0) {
			xml_parse_merge_fields (node, label);
		} else {
			if (!xmlNodeIsText (node)) {
				g_warning (_("bad node =  \"%s\""), node->name);
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
xml_parse_objects (xmlNodePtr objects_node,
		   glLabel * label)
{
	gboolean rotate_flag;
	xmlNodePtr node;

	gl_debug (DEBUG_XML, "START");

	rotate_flag =
		!(g_strcasecmp (xmlGetProp (objects_node, "rotate"), "false") == 0);
	gl_label_set_rotate_flag (label, rotate_flag);

	for (node = objects_node->xmlChildrenNode; node != NULL; node = node->next) {

		if (g_strcasecmp (node->name, "Object") == 0) {
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
xml_parse_object (xmlNodePtr object_node,
		  glLabel * label)
{
	glLabelObject *object;
	gdouble x, y;
	gchar *type_string;

	gl_debug (DEBUG_XML, "START");

	type_string = xmlGetProp (object_node, "type");

	if ( g_strcasecmp (type_string, "text") == 0 ) {
		object = xml_parse_text_props (object_node, label);
	} else if ( g_strcasecmp (type_string, "box") == 0 ) {
		object = xml_parse_box_props (object_node, label);
	} else if ( g_strcasecmp (type_string, "line") == 0 ) {
		object = xml_parse_line_props (object_node, label);
	} else if ( g_strcasecmp (type_string, "ellipse") == 0 ) {
		object = xml_parse_ellipse_props (object_node, label);
	} else if ( g_strcasecmp (type_string, "image") == 0 ) {
		object = xml_parse_image_props (object_node, label);
	} else if ( g_strcasecmp (type_string, "barcode") == 0 ) {
		object = xml_parse_barcode_props (object_node, label);
	} else {
		g_warning ("Unknown label object type \"%s\"", type_string);
		return;
	}

	x = g_strtod (xmlGetProp (object_node, "x"), NULL);
	y = g_strtod (xmlGetProp (object_node, "y"), NULL);

	gl_label_object_set_position (object, x, y);

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Text Node Properties                          */
/*--------------------------------------------------------------------------*/
static glLabelObject *
xml_parse_text_props (xmlNodePtr object_node,
		      glLabel * label)
{
	GObject *object;
	GList *lines;
	gchar *font_family;
	gdouble font_size;
	GnomeFontWeight font_weight;
	gboolean font_italic_flag;
	guint color;
	GtkJustification just;
	xmlNodePtr line_node, text_node;
	glTextNode *node_text;
	GList *nodes;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_text_new (label);

	font_family = xmlGetProp (object_node, "font_family");
	font_size = g_strtod (xmlGetProp (object_node, "font_size"), NULL);
	font_weight = gl_util_string_to_weight (xmlGetProp (object_node,
							  "font_weight"));
	font_italic_flag =
		!(g_strcasecmp (xmlGetProp (object_node, "font_italic"), "false") ==
		  0);

	just = gl_util_string_to_just (xmlGetProp (object_node, "justify"));

	sscanf (xmlGetProp (object_node, "color"), "%x", &color);

	lines = NULL;
	for (line_node = object_node->xmlChildrenNode;
	     line_node != NULL;
	     line_node = line_node->next) {

		if (g_strcasecmp (line_node->name, "Line") == 0) {

			nodes = NULL;
			for (text_node = line_node->xmlChildrenNode;
			     text_node != NULL; text_node = text_node->next) {

				if (g_strcasecmp (text_node->name, "Field") ==
				    0) {
					node_text = g_new0 (glTextNode, 1);
					node_text->field_flag = TRUE;
					node_text->data =
						xmlGetProp (text_node, "name");
					nodes =
						g_list_append (nodes, node_text);
				} else if (xmlNodeIsText (text_node)) {
					node_text = g_new0 (glTextNode, 1);
					node_text->field_flag = FALSE;
					node_text->data =
						xmlNodeGetContent (text_node);
					nodes =
						g_list_append (nodes, node_text);
				} else {
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

	gl_label_text_set_lines (GL_LABEL_TEXT(object), lines);
	gl_label_text_set_props (GL_LABEL_TEXT(object),
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
xml_parse_box_props (xmlNodePtr node,
		     glLabel * label)
{
	GObject *object;
	gdouble line_width;
	guint line_color, fill_color;
	gdouble w, h;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_box_new (label);

	w = g_strtod (xmlGetProp (node, "w"), NULL);
	h = g_strtod (xmlGetProp (node, "h"), NULL);

	line_width = g_strtod (xmlGetProp (node, "line_width"), NULL);

	sscanf (xmlGetProp (node, "line_color"), "%x", &line_color);
	sscanf (xmlGetProp (node, "fill_color"), "%x", &fill_color);

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
xml_parse_line_props (xmlNodePtr node,
		      glLabel * label)
{
	GObject *object;
	gdouble line_width;
	guint line_color;
	gdouble w, h;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_line_new (label);

	w = g_strtod (xmlGetProp (node, "dx"), NULL);
	h = g_strtod (xmlGetProp (node, "dy"), NULL);

	line_width = g_strtod (xmlGetProp (node, "line_width"), NULL);

	sscanf (xmlGetProp (node, "line_color"), "%x", &line_color);

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
xml_parse_ellipse_props (xmlNodePtr node,
			 glLabel * label)
{
	GObject *object;
	gdouble line_width;
	guint line_color, fill_color;
	gdouble w, h;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_ellipse_new (label);

	w = g_strtod (xmlGetProp (node, "w"), NULL);
	h = g_strtod (xmlGetProp (node, "h"), NULL);

	line_width = g_strtod (xmlGetProp (node, "line_width"), NULL);

	sscanf (xmlGetProp (node, "line_color"), "%x", &line_color);
	sscanf (xmlGetProp (node, "fill_color"), "%x", &fill_color);

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
xml_parse_image_props (xmlNodePtr node,
		       glLabel *label)
{
	GObject *object;
	gdouble w, h;
	gchar   *filename;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_image_new (label);

	w = g_strtod (xmlGetProp (node, "w"), NULL);
	h = g_strtod (xmlGetProp (node, "h"), NULL);

	filename = xmlNodeGetContent (node);

	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);
	gl_label_image_set_filename (GL_LABEL_IMAGE(object), filename);

	g_free (filename);

	gl_debug (DEBUG_XML, "END");

	return GL_LABEL_OBJECT(object);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Barcode Node Properties                       */
/*--------------------------------------------------------------------------*/
static glLabelObject *
xml_parse_barcode_props (xmlNodePtr node,
			 glLabel *label)
{
	GObject *object;
	xmlNodePtr child;
	glTextNode          *text_node;
	glBarcodeStyle      style;
	gboolean            text_flag;
	guint               color;
	gdouble             scale;

	gl_debug (DEBUG_XML, "START");

	object = gl_label_barcode_new (label);

	sscanf (xmlGetProp (node, "color"), "%x", &color);

	style = gl_barcode_text_to_style (xmlGetProp (node, "style"));
	text_flag = !(g_strcasecmp (xmlGetProp (node, "text"), "false") == 0);
	scale = g_strtod (xmlGetProp (node, "scale"), NULL);

	child = node->xmlChildrenNode;
	text_node = g_new0 (glTextNode, 1);
	if (g_strcasecmp (child->name, "Field") == 0) {
		text_node->field_flag = TRUE;
		text_node->data = xmlGetProp (child, "name");
	} else if (xmlNodeIsText (child)) {
		text_node->field_flag = FALSE;
		text_node->data = xmlNodeGetContent (child);
	} else {
		g_warning ("Unexpected Barcode child: \"%s\"", child->name);
	}

	gl_label_barcode_set_data (GL_LABEL_BARCODE(object), text_node);
	gl_label_barcode_set_props (GL_LABEL_BARCODE(object),
				    style, text_flag, color, scale);

	gl_text_node_free (&text_node);

	gl_debug (DEBUG_XML, "END");

	return GL_LABEL_OBJECT(object);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML merge fields tag.                                */
/*--------------------------------------------------------------------------*/
static void
xml_parse_merge_fields (xmlNodePtr node,
			glLabel * label)
{
	xmlNodePtr child;
	glMerge *merge;
	glMergeFieldDefinition *field_def;

	gl_debug (DEBUG_XML, "START");

	merge = gl_merge_new ();

	merge->type = gl_merge_text_to_type (xmlGetProp (node, "type"));
	merge->src = xmlGetProp (node, "src");

	for (child = node->xmlChildrenNode; child != NULL; child = child->next) {

		if (g_strcasecmp (child->name, "Field") == 0) {
			field_def = g_new0 (glMergeFieldDefinition, 1);
			field_def->key = xmlGetProp (child, "key");
			field_def->loc = xmlGetProp (child, "loc");
			merge->field_defs =
			    g_list_append (merge->field_defs,
					   field_def);
		} else if (!xmlNodeIsText (child)) {
			g_warning ("Unexpected Merge_Fields child: \"%s\"",
				   child->name);
		}

	}

	gl_label_set_merge (label, merge);

	gl_merge_free (&merge);

	gl_debug (DEBUG_XML, "END");
}

/****************************************************************************/
/* Save label to xml label file.                                            */
/****************************************************************************/
void
gl_xml_label_save (glLabel *label,
		   const gchar *filename,
		   glXMLLabelStatus *status)
{
	xmlDocPtr doc;
	gint xml_ret;

	gl_debug (DEBUG_XML, "START");

	doc = xml_label_to_doc (label, status);

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
gl_xml_label_save_buffer (glLabel *label,
			  glXMLLabelStatus *status)
{
	xmlDocPtr doc;
	gint size;
	gchar *buffer;

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
xml_label_to_doc (glLabel * label,
		  glXMLLabelStatus *status)
{
	xmlDocPtr doc;
	xmlNsPtr ns;
	glTemplate *template;
	glMerge *merge;

	gl_debug (DEBUG_XML, "START");

	LIBXML_TEST_VERSION;

	doc = xmlNewDoc ("1.0");
	doc->xmlRootNode = xmlNewDocNode (doc, NULL, "Document", NULL);

	ns = xmlNewNs (doc->xmlRootNode, NAME_SPACE, "glabels");
	xmlSetNs (doc->xmlRootNode, ns);

	template = gl_label_get_template (label);
	gl_template_xml_add_sheet (template, doc->xmlRootNode, ns);

	xml_create_objects (doc->xmlRootNode, ns, label);

	merge = gl_label_get_merge (label);
	gl_debug (DEBUG_XML, "merge=%p", merge);
	if (merge->type != GL_MERGE_NONE) {
		xml_create_merge_fields (doc->xmlRootNode, ns, label);
	}
	gl_merge_free (&merge);

	gl_debug (DEBUG_XML, "END");

	*status = XML_LABEL_OK;
	return doc;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label->Objects Node                                    */
/*--------------------------------------------------------------------------*/
static void
xml_create_objects (xmlNodePtr root,
		    xmlNsPtr ns,
		    glLabel * label)
{
	xmlNodePtr node;
	gboolean rotate_flag;
	GList *p;

	gl_debug (DEBUG_XML, "START");

	rotate_flag = gl_label_get_rotate_flag (label);

	node = xmlNewChild (root, ns, "Objects", NULL);
	xmlSetProp (node, "id", "0");
	xmlSetProp (node, "rotate", rotate_flag ? "True" : "False");

	for (p = label->objects; p != NULL; p = p->next) {
		xml_create_object (node, ns, GL_LABEL_OBJECT(p->data));
	}

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML label object Node                                      */
/*--------------------------------------------------------------------------*/
static void
xml_create_object (xmlNodePtr root,
		   xmlNsPtr ns,
		   glLabelObject * object)
{
	xmlNodePtr object_node;
	gdouble x, y;
	gchar *string;

	gl_debug (DEBUG_XML, "START");

	object_node = xmlNewChild (root, ns, "Object", NULL);

	gl_label_object_get_position (object, &x, &y);
	string = g_strdup_printf ("%g", x);
	xmlSetProp (object_node, "x", string);
	g_free (string);
	string = g_strdup_printf ("%g", y);
	xmlSetProp (object_node, "y", string);
	g_free (string);

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
xml_create_text_props (xmlNodePtr object_node,
		       xmlNsPtr ns,
		       glLabelObject * object)
{
	xmlNodePtr line_node, field_node;
	GList *lines;
	gchar *font_family;
	gdouble font_size;
	GnomeFontWeight font_weight;
	gboolean font_italic_flag;
	guint color;
	GtkJustification just;
	gchar *string;
	GList *p_line, *p_node;
	glTextNode *node_text;

	gl_debug (DEBUG_XML, "START");

	xmlSetProp (object_node, "type", "Text");

	lines = gl_label_text_get_lines (GL_LABEL_TEXT(object));
	gl_label_text_get_props (GL_LABEL_TEXT(object),
				 &font_family, &font_size, &font_weight,
				 &font_italic_flag,
				 &color, &just);

	xmlSetProp (object_node, "font_family", font_family);
	string = g_strdup_printf ("%g", font_size);
	xmlSetProp (object_node, "font_size", string);
	g_free (string);
	xmlSetProp (object_node, "font_weight",
		    gl_util_weight_to_string (font_weight));
	xmlSetProp (object_node, "font_italic",
		    font_italic_flag?"True":"False");

	xmlSetProp (object_node, "justify", gl_util_just_to_string (just));

	string = g_strdup_printf ("0x%08x", color);
	xmlSetProp (object_node, "color", string);
	g_free (string);

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
				xmlNodeAddContent (line_node, node_text->data);
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
xml_create_box_props (xmlNodePtr object_node,
		      xmlNsPtr ns,
		      glLabelObject * object)
{
	gchar *string;
	gdouble line_width;
	guint line_color, fill_color;
	gdouble w, h;

	gl_debug (DEBUG_XML, "START");

	xmlSetProp (object_node, "type", "Box");

	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_box_get_line_width (GL_LABEL_BOX(object));
	line_color = gl_label_box_get_line_color (GL_LABEL_BOX(object));
	fill_color = gl_label_box_get_fill_color (GL_LABEL_BOX(object));

	string = g_strdup_printf ("%g", w);
	xmlSetProp (object_node, "w", string);
	g_free (string);
	string = g_strdup_printf ("%g", h);
	xmlSetProp (object_node, "h", string);
	g_free (string);

	string = g_strdup_printf ("%g", line_width);
	xmlSetProp (object_node, "line_width", string);
	g_free (string);

	string = g_strdup_printf ("0x%08x", line_color);
	xmlSetProp (object_node, "line_color", string);
	g_free (string);

	string = g_strdup_printf ("0x%08x", fill_color);
	xmlSetProp (object_node, "fill_color", string);
	g_free (string);

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label->Line Node Properties                            */
/*--------------------------------------------------------------------------*/
static void
xml_create_line_props (xmlNodePtr object_node,
		       xmlNsPtr ns,
		       glLabelObject * object)
{
	gchar *string;
	gdouble line_width;
	guint line_color;
	gdouble w, h;

	gl_debug (DEBUG_XML, "START");

	xmlSetProp (object_node, "type", "Line");

	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_line_get_line_width (GL_LABEL_LINE(object));
	line_color = gl_label_line_get_line_color (GL_LABEL_LINE(object));

	string = g_strdup_printf ("%g", w);
	xmlSetProp (object_node, "dx", string);
	g_free (string);
	string = g_strdup_printf ("%g", h);
	xmlSetProp (object_node, "dy", string);
	g_free (string);

	string = g_strdup_printf ("%g", line_width);
	xmlSetProp (object_node, "line_width", string);
	g_free (string);

	string = g_strdup_printf ("0x%08x", line_color);
	xmlSetProp (object_node, "line_color", string);
	g_free (string);

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label->Ellipse Node Properties                         */
/*--------------------------------------------------------------------------*/
static void
xml_create_ellipse_props (xmlNodePtr object_node,
			  xmlNsPtr ns,
			  glLabelObject * object)
{
	gchar *string;
	gdouble line_width;
	guint line_color, fill_color;
	gdouble w, h;

	gl_debug (DEBUG_XML, "START");

	xmlSetProp (object_node, "type", "Ellipse");

	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_ellipse_get_line_width (GL_LABEL_ELLIPSE(object));
	line_color = gl_label_ellipse_get_line_color (GL_LABEL_ELLIPSE(object));
	fill_color = gl_label_ellipse_get_fill_color (GL_LABEL_ELLIPSE(object));

	string = g_strdup_printf ("%g", w);
	xmlSetProp (object_node, "w", string);
	g_free (string);
	string = g_strdup_printf ("%g", h);
	xmlSetProp (object_node, "h", string);
	g_free (string);

	string = g_strdup_printf ("%g", line_width);
	xmlSetProp (object_node, "line_width", string);
	g_free (string);

	string = g_strdup_printf ("0x%08x", line_color);
	xmlSetProp (object_node, "line_color", string);
	g_free (string);

	string = g_strdup_printf ("0x%08x", fill_color);
	xmlSetProp (object_node, "fill_color", string);
	g_free (string);

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label->Image Node Properties                           */
/*--------------------------------------------------------------------------*/
static void
xml_create_image_props (xmlNodePtr object_node,
			xmlNsPtr ns,
			glLabelObject * object)
{
	gchar *string;
	gdouble w, h;
	gchar *filename;

	gl_debug (DEBUG_XML, "START");

	xmlSetProp (object_node, "type", "Image");

	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	filename = gl_label_image_get_filename (GL_LABEL_IMAGE(object));

	string = g_strdup_printf ("%g", w);
	xmlSetProp (object_node, "w", string);
	g_free (string);
	string = g_strdup_printf ("%g", h);
	xmlSetProp (object_node, "h", string);
	g_free (string);

	xmlNodeSetContent (object_node, filename);

	g_free (filename);

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label->Barcode Node Properties                         */
/*--------------------------------------------------------------------------*/
static void
xml_create_barcode_props (xmlNodePtr object_node,
			  xmlNsPtr ns,
			  glLabelObject * object)
{
	glTextNode          *text_node;
	glBarcodeStyle      style;
	gboolean            text_flag;
	guint               color;
	gdouble             scale;
	xmlNodePtr child;
	gchar *string;

	gl_debug (DEBUG_XML, "START");

	xmlSetProp (object_node, "type", "Barcode");

	text_node = gl_label_barcode_get_data (GL_LABEL_BARCODE(object));
	gl_label_barcode_get_props (GL_LABEL_BARCODE(object),
				    &style, &text_flag, &color, &scale);

	string = g_strdup_printf ("0x%08x", color);
	xmlSetProp (object_node, "color", string);
	g_free (string);

	xmlSetProp (object_node, "style", gl_barcode_style_to_text (style));
	xmlSetProp (object_node, "text", text_flag?"True":"False");
	string = g_strdup_printf ("%g", scale);
	xmlSetProp (object_node, "scale", string);
	g_free (string);

	if (text_node->field_flag) {
		child = xmlNewChild (object_node, ns, "Field", NULL);
		xmlSetProp (child, "name", text_node->data);
	} else {
		xmlNodeSetContent (object_node, text_node->data);
	}

	gl_text_node_free (&text_node);

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label Merge Fields Node                                */
/*--------------------------------------------------------------------------*/
static void
xml_create_merge_fields (xmlNodePtr root,
			     xmlNsPtr ns,
			     glLabel * label)
{
	xmlNodePtr node, child;
	gchar *string;
	GList *p;
	glMerge *merge;
	glMergeFieldDefinition *field_def;

	gl_debug (DEBUG_XML, "START");

	merge = gl_label_get_merge (label);

	node = xmlNewChild (root, ns, "Merge_Fields", NULL);

	string = gl_merge_type_to_text (merge->type);
	xmlSetProp (node, "type", string);
	g_free (string);

	xmlSetProp (node, "src", merge->src);

	for (p = merge->field_defs; p != NULL; p = p->next) {
		field_def = (glMergeFieldDefinition *) p->data;

		child = xmlNewChild (node, ns, "Field", NULL);
		xmlSetProp (child, "key", field_def->key);
		xmlSetProp (child, "loc", field_def->loc);
	}

	gl_merge_free (&merge);

	gl_debug (DEBUG_XML, "END");
}

