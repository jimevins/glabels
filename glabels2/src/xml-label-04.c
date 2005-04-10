/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  xml-label-04.c:  GLabels xml label compat module
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

#include <glib/gi18n.h>

#include "xml-label-04.h"
#include "label-text.h"
#include "label-box.h"
#include "label-line.h"
#include "label-ellipse.h"
#include "label-image.h"
#include "label-barcode.h"
#include <libglabels/xml.h>

#include "debug.h"

static gboolean xml04_parse_media_description (xmlNodePtr node,
					       glLabel *label);
static void     xml04_parse_object            (xmlNodePtr node,
					       glLabelObject *object);
static void     xml04_parse_text_props        (xmlNodePtr node,
					       glLabelText *object);
static void     xml04_parse_box_props         (xmlNodePtr node,
					       glLabelBox *object);
static void     xml04_parse_line_props        (xmlNodePtr node,
					       glLabelLine *object);
static void     xml04_parse_ellipse_props     (xmlNodePtr node,
					       glLabelEllipse *object);
static void     xml04_parse_image_props       (xmlNodePtr node,
					       glLabelImage *object);
static void     xml04_parse_barcode_props     (xmlNodePtr node,
					       glLabelBarcode *object);
static void     xml04_parse_merge_properties  (xmlNodePtr node,
					       glLabel *label);


/****************************************************************************/
/* PRIVATE.  Parse xml doc structure and create label.                      */
/****************************************************************************/
glLabel      *gl_xml_label_04_parse      (xmlNodePtr       root,
					  glXMLLabelStatus *status)
{
	glLabel       *label;
	xmlNodePtr    node;
	xmlNsPtr      ns;
	GObject       *object;
	gboolean      rotate_flag;

	gl_debug (DEBUG_XML, "START");

	*status = XML_LABEL_OK;

	if (!xmlStrEqual (root->name, "Label")) {
		g_warning (_("Bad root node = \"%s\""), root->name);
		*status = XML_LABEL_ERROR_OPEN_PARSE;
		return NULL;
	}

	label = GL_LABEL (gl_label_new ());

	rotate_flag = gl_xml_get_prop_boolean (root, "rotate", FALSE);
	gl_label_set_rotate_flag (label, rotate_flag);

	for (node = root->xmlChildrenNode; node != NULL; node = node->next) {

		gl_debug (DEBUG_XML, "node name = \"%s\"", node->name);

		if (!xmlNodeIsText (node)) {
			if (xmlStrEqual (node->name, "Media_Type")) {
				if (!xml04_parse_media_description (node, label)) {
					*status = XML_LABEL_UNKNOWN_MEDIA;
				}
			} else if (xmlStrEqual (node->name, "Text")) {
				object = gl_label_text_new (label);
				xml04_parse_object (node, GL_LABEL_OBJECT(object));
				xml04_parse_text_props (node, GL_LABEL_TEXT(object));
			} else if (xmlStrEqual (node->name, "Box")) {
				object = gl_label_box_new (label);
				xml04_parse_object (node, GL_LABEL_OBJECT(object));
				xml04_parse_box_props (node, GL_LABEL_BOX(object));
			} else if (xmlStrEqual (node->name, "Line")) {
				object = gl_label_line_new (label);
				xml04_parse_object (node, GL_LABEL_OBJECT(object));
				xml04_parse_line_props (node, GL_LABEL_LINE(object));
			} else if (xmlStrEqual (node->name, "Ellipse")) {
				object = gl_label_ellipse_new (label);
				xml04_parse_object (node, GL_LABEL_OBJECT(object));
				xml04_parse_ellipse_props (node,
							   GL_LABEL_ELLIPSE(object));
			} else if (xmlStrEqual (node->name, "Image")) {
				object = gl_label_image_new (label);
				xml04_parse_object (node, GL_LABEL_OBJECT(object));
				xml04_parse_image_props (node, GL_LABEL_IMAGE(object));
			} else if (xmlStrEqual (node->name, "Barcode")) {
				object = gl_label_barcode_new (label);
				xml04_parse_object (node, GL_LABEL_OBJECT(object));
				xml04_parse_barcode_props (node,
							   GL_LABEL_BARCODE(object));
			} else if (xmlStrEqual (node->name, "Merge_Properties")) {
				xml04_parse_merge_properties (node, label);
			} else {
				g_warning (_("bad node =  \"%s\""), node->name);
			}
		}
	}

	gl_debug (DEBUG_XML, "END");

	return label;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse Media Description node.                                  */
/*--------------------------------------------------------------------------*/
static gboolean
xml04_parse_media_description (xmlNodePtr node,
			       glLabel    *label)
{
	xmlChar    *template_name;
	glTemplate *template;
	gboolean   ret;

	gl_debug (DEBUG_XML, "START");

	template_name = xmlNodeGetContent (node);

	template = gl_template_from_name (template_name);
	if (template == NULL) {
		g_warning ("Undefined template \"%s\"", template_name);
		/* Get a default */
		template = gl_template_from_name (NULL);
		ret = FALSE;
	} else {
		ret = TRUE;
	}

	gl_label_set_template (label, template);

	gl_template_free (template);
	xmlFree (template_name);

	gl_debug (DEBUG_XML, "END");

	return ret;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Object Node                                          */
/*--------------------------------------------------------------------------*/
static void
xml04_parse_object (xmlNodePtr    object_node,
		    glLabelObject *object)
{
	gdouble x, y;

	gl_debug (DEBUG_XML, "START");

	x = gl_xml_get_prop_double (object_node, "x", 0);
	y = gl_xml_get_prop_double (object_node, "y", 0);

	gl_label_object_set_position (object, x, y);

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Text Node Properties                          */
/*--------------------------------------------------------------------------*/
static void
xml04_parse_text_props (xmlNodePtr    object_node,
			glLabelText   *object)
{
	xmlChar         *font_family;
	gdouble          font_size;
	GnomeFontWeight  font_weight;
	gboolean         font_italic_flag;
	glColorNode     *color_node;
	GtkJustification just;
	xmlNodePtr       line_node, text_node;
	glTextNode      *node_text;
	GList           *nodes, *lines;
	gdouble          w, h, x, y;
	xmlChar         *string;

	gl_debug (DEBUG_XML, "START");

	font_family = xmlGetProp (object_node, "font_family");
	font_size = gl_xml_get_prop_double (object_node, "font_size", 0);
	string = xmlGetProp (object_node, "font_weight");
	font_weight = gl_util_string_to_weight (string);
	xmlFree (string);
	font_italic_flag = gl_xml_get_prop_boolean (object_node, "font_italic", FALSE);

	string = xmlGetProp (object_node, "justify");
	just = gl_util_string_to_just (string);
	xmlFree (string);

	color_node = gl_color_node_new_default ();
	color_node->color = gl_xml_get_prop_uint (object_node, "color", 0);

	gl_label_object_set_font_family (GL_LABEL_OBJECT(object), font_family);
	gl_label_object_set_font_size (GL_LABEL_OBJECT(object), font_size);
	gl_label_object_set_font_weight (GL_LABEL_OBJECT(object), font_weight);
	gl_label_object_set_font_italic_flag (GL_LABEL_OBJECT(object), font_italic_flag);
	gl_label_object_set_text_color (GL_LABEL_OBJECT(object), color_node);
	gl_label_object_set_text_alignment (GL_LABEL_OBJECT(object), just);
	
	gl_color_node_free (&color_node);

	lines = NULL;
	for (line_node = object_node->xmlChildrenNode; line_node != NULL;
	     line_node = line_node->next) {

		if (xmlStrEqual (line_node->name, "Line")) {

			gl_debug (DEBUG_XML, "->Line node");

			nodes = NULL;
			for (text_node = line_node->xmlChildrenNode;
			     text_node != NULL; text_node = text_node->next) {

				if (xmlStrEqual (text_node->name, "Field")) {
					gl_debug (DEBUG_XML, "->Line->Field node");
					node_text = g_new0 (glTextNode, 1);
					node_text->field_flag = TRUE;
					node_text->data =
					    xmlGetProp (text_node, "name");
					nodes =
					    g_list_append (nodes, node_text);
				} else if (xmlNodeIsText (text_node)) {
					gl_debug (DEBUG_XML, "->Line->\"literal\" node");
					node_text = g_new0 (glTextNode, 1);
					node_text->field_flag = FALSE;
					node_text->data =
					    xmlNodeGetContent (text_node);
					gl_debug (DEBUG_XML, "text = \"%s\"",
						  node_text->data);
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

	gl_label_text_set_lines (object, lines);

	gl_text_node_lines_free (&lines);
	xmlFree (font_family);

	/* Adjust location.  In 0.4.x, text was anchored at x,y */
	gl_label_object_get_position (GL_LABEL_OBJECT(object), &x, &y);
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	switch (just) {
	case GTK_JUSTIFY_LEFT:
		/* nothing */
		break;
	case GTK_JUSTIFY_CENTER:
		x -= w/2.0;
		gl_label_object_set_position (GL_LABEL_OBJECT(object), x, y);
		break;
	case GTK_JUSTIFY_RIGHT:
		x -= w;
		gl_label_object_set_position (GL_LABEL_OBJECT(object), x, y);
		break;
	default:
		/* should not happen */
		break;
	}

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Box Node Properties                           */
/*--------------------------------------------------------------------------*/
static void
xml04_parse_box_props (xmlNodePtr    node,
		       glLabelBox    *object)
{
	gdouble w, h, line_width;
	glColorNode *line_color_node;
	glColorNode *fill_color_node;

	gl_debug (DEBUG_XML, "START");

	w = gl_xml_get_prop_double (node, "w", 0);
	h = gl_xml_get_prop_double (node, "h", 0);

	line_width = gl_xml_get_prop_double (node, "line_width", 0);

	line_color_node = gl_color_node_new_default ();
	line_color_node->color = gl_xml_get_prop_uint (node, "line_color", 0);
	
	fill_color_node = gl_color_node_new_default ();
	fill_color_node->color = gl_xml_get_prop_uint (node, "fill_color", 0);

	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);
	gl_label_object_set_line_width (GL_LABEL_OBJECT(object), line_width);
	gl_label_object_set_line_color (GL_LABEL_OBJECT(object), line_color_node);
	gl_label_object_set_fill_color (GL_LABEL_OBJECT(object), fill_color_node);

	gl_color_node_free (&line_color_node);
	gl_color_node_free (&fill_color_node);
	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Line Node Properties                          */
/*--------------------------------------------------------------------------*/
static void
xml04_parse_line_props (xmlNodePtr    node,
			glLabelLine *object)
{
	gdouble w, h, line_width;
	glColorNode *line_color_node;

	gl_debug (DEBUG_XML, "START");

	w = gl_xml_get_prop_double (node, "dx", 0);
	h = gl_xml_get_prop_double (node, "dy", 0);

	line_width = gl_xml_get_prop_double (node, "line_width", 0);

	line_color_node = gl_color_node_new_default ();
	line_color_node->color = gl_xml_get_prop_uint (node, "line_color", 0);

	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);
	gl_label_object_set_line_width (GL_LABEL_OBJECT(object), line_width);
	gl_label_object_set_line_color (GL_LABEL_OBJECT(object), line_color_node);

	gl_color_node_free (&line_color_node);
	
	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Ellipse Node Properties                       */
/*--------------------------------------------------------------------------*/
static void
xml04_parse_ellipse_props (xmlNodePtr     node,
			   glLabelEllipse *object)
{
	gdouble w, h, line_width;
	glColorNode *line_color_node;
	glColorNode *fill_color_node;

	gl_debug (DEBUG_XML, "START");

	w = gl_xml_get_prop_double (node, "w", 0);
	h = gl_xml_get_prop_double (node, "h", 0);

	line_width = gl_xml_get_prop_double (node, "line_width", 0);

	line_color_node = gl_color_node_new_default ();
	line_color_node->color = gl_xml_get_prop_uint (node, "line_color", 0);
	
	fill_color_node = gl_color_node_new_default ();
	fill_color_node->color = gl_xml_get_prop_uint (node, "fill_color", 0);

	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);
	gl_label_object_set_line_width (GL_LABEL_OBJECT(object), line_width);
	gl_label_object_set_line_color (GL_LABEL_OBJECT(object), line_color_node);
	gl_label_object_set_fill_color (GL_LABEL_OBJECT(object), fill_color_node);

	gl_color_node_free (&line_color_node);
	gl_color_node_free (&fill_color_node);
	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Image Node Properties                         */
/*--------------------------------------------------------------------------*/
static void
xml04_parse_image_props (xmlNodePtr    node,
			 glLabelImage  *object)
{
	gdouble w, h;
	glTextNode *filename;

	gl_debug (DEBUG_XML, "START");

	filename = g_new0 (glTextNode, 1);
	filename->field_flag = FALSE;
	filename->data = xmlGetProp (node, "filename");
	gl_label_image_set_filename (object, filename);
	gl_text_node_free (&filename);

	w = gl_xml_get_prop_double (node, "w", 0);
	h = gl_xml_get_prop_double (node, "h", 0);
	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Barcode Node Properties                       */
/*--------------------------------------------------------------------------*/
static void
xml04_parse_barcode_props (xmlNodePtr    node,
			   glLabelBarcode *object)
{
	xmlChar       *id;
	gboolean       text_flag;
	glColorNode   *color_node;
	gdouble        scale;
	xmlNodePtr     child;
	glTextNode    *text_node;

	gl_debug (DEBUG_XML, "START");

	color_node = gl_color_node_new_default ();
	color_node->color = gl_xml_get_prop_uint (node, "color", 0);

	id = xmlGetProp (node, "style");

	text_flag = gl_xml_get_prop_boolean (node, "text", FALSE);
	scale =	gl_xml_get_prop_double (node, "scale", 1.0);
	if (scale == 0.0) {
		scale = 0.5; /* Set to a valid value */
	}
	gl_label_barcode_set_props (object, id, text_flag, TRUE, 0);
	gl_label_object_set_line_color (GL_LABEL_OBJECT(object), color_node);

	child = node->xmlChildrenNode;
	text_node = g_new0 (glTextNode, 1);
	if (xmlStrEqual (child->name, "Field")) {
		text_node->field_flag = TRUE;
		text_node->data       = xmlGetProp (child, "name");
	} else if (xmlNodeIsText (child)) {
		text_node->field_flag = FALSE;
		text_node->data       = xmlNodeGetContent (child);
	} else {
		g_warning ("Unexpected Barcode child: \"%s\"", child->name);
	}
	gl_label_barcode_set_data (object, text_node);

	gl_color_node_free (&color_node);
	gl_text_node_free (&text_node);
	xmlFree (id);

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML merge properties tag.                                */
/*--------------------------------------------------------------------------*/
static void
xml04_parse_merge_properties (xmlNodePtr node,
			      glLabel    *label)
{
	glMerge               *merge;
	xmlNodePtr             child;
	xmlChar               *string;

	gl_debug (DEBUG_XML, "START");

	string = xmlGetProp (node, "type");
	merge = gl_merge_new (string);
	xmlFree (string);

	string = xmlGetProp (node, "src");
	gl_merge_set_src (merge, string);
	xmlFree (string);

	gl_label_set_merge (label, merge);

	g_object_unref (G_OBJECT(merge));

	gl_debug (DEBUG_XML, "END");
}
