/*
 *  xml-label-04.c
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

#include "xml-label-04.h"

#include <glib/gi18n.h>

#include <libglabels.h>
#include "label-text.h"
#include "label-box.h"
#include "label-line.h"
#include "label-ellipse.h"
#include "label-image.h"
#include "label-barcode.h"
#include "bc-backends.h"
#include "str-util.h"

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
	GObject       *object;
	gboolean      rotate_flag;

	gl_debug (DEBUG_XML, "START");

	*status = XML_LABEL_OK;

	if (!xmlStrEqual (root->name, (xmlChar *)"Label")) {
		g_message (_("Bad root node = \"%s\""), root->name);
		*status = XML_LABEL_ERROR_OPEN_PARSE;
		return NULL;
	}

	label = GL_LABEL (gl_label_new ());

	rotate_flag = lgl_xml_get_prop_boolean (root, "rotate", FALSE);
	gl_label_set_rotate_flag (label, rotate_flag, FALSE);

	for (node = root->xmlChildrenNode; node != NULL; node = node->next) {

		gl_debug (DEBUG_XML, "node name = \"%s\"", node->name);

		if (!xmlNodeIsText (node)) {
			if (xmlStrEqual (node->name, (xmlChar *)"Media_Type")) {
				if (!xml04_parse_media_description (node, label)) {
					*status = XML_LABEL_UNKNOWN_MEDIA;
				}
			} else if (xmlStrEqual (node->name, (xmlChar *)"Text")) {
				object = gl_label_text_new (label, FALSE);
				xml04_parse_object (node, GL_LABEL_OBJECT(object));
				xml04_parse_text_props (node, GL_LABEL_TEXT(object));
			} else if (xmlStrEqual (node->name, (xmlChar *)"Box")) {
				object = gl_label_box_new (label, FALSE);
				xml04_parse_object (node, GL_LABEL_OBJECT(object));
				xml04_parse_box_props (node, GL_LABEL_BOX(object));
			} else if (xmlStrEqual (node->name, (xmlChar *)"Line")) {
				object = gl_label_line_new (label, FALSE);
				xml04_parse_object (node, GL_LABEL_OBJECT(object));
				xml04_parse_line_props (node, GL_LABEL_LINE(object));
			} else if (xmlStrEqual (node->name, (xmlChar *)"Ellipse")) {
				object = gl_label_ellipse_new (label, FALSE);
				xml04_parse_object (node, GL_LABEL_OBJECT(object));
				xml04_parse_ellipse_props (node,
							   GL_LABEL_ELLIPSE(object));
			} else if (xmlStrEqual (node->name, (xmlChar *)"Image")) {
				object = gl_label_image_new (label, FALSE);
				xml04_parse_object (node, GL_LABEL_OBJECT(object));
				xml04_parse_image_props (node, GL_LABEL_IMAGE(object));
			} else if (xmlStrEqual (node->name, (xmlChar *)"Barcode")) {
				object = gl_label_barcode_new (label, FALSE);
				xml04_parse_object (node, GL_LABEL_OBJECT(object));
				xml04_parse_barcode_props (node,
							   GL_LABEL_BARCODE(object));
			} else if (xmlStrEqual (node->name, (xmlChar *)"Merge_Properties")) {
				xml04_parse_merge_properties (node, label);
			} else {
				g_message (_("bad node =  \"%s\""), node->name);
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
	xmlChar     *template_name;
	lglTemplate *template;
	gboolean     ret;

	gl_debug (DEBUG_XML, "START");

	template_name = xmlNodeGetContent (node);

	template = lgl_db_lookup_template_from_name ((gchar *)template_name);
	if (template == NULL) {
		g_message ("Undefined template \"%s\"", template_name);
		/* Get a default */
		template = lgl_db_lookup_template_from_name (NULL);
		ret = FALSE;
	} else {
		ret = TRUE;
	}

	gl_label_set_template (label, template, FALSE);

	lgl_template_free (template);
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

	x = lgl_xml_get_prop_double (object_node, "x", 0);
	y = lgl_xml_get_prop_double (object_node, "y", 0);

	gl_label_object_set_position (object, x, y, FALSE);

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
	PangoWeight      font_weight;
	gboolean         font_italic_flag;
	glColorNode     *color_node;
	PangoAlignment   align;
	xmlNodePtr       line_node, text_node;
	glTextNode      *node_text;
	GList           *nodes, *lines;
	gdouble          w, h, x, y;
	xmlChar         *string;

	gl_debug (DEBUG_XML, "START");

	font_family = xmlGetProp (object_node, (xmlChar *)"font_family");
	font_size = lgl_xml_get_prop_double (object_node, "font_size", 0);
	string = xmlGetProp (object_node, (xmlChar *)"font_weight");
	font_weight = gl_str_util_string_to_weight ((gchar *)string);
	xmlFree (string);
	font_italic_flag = lgl_xml_get_prop_boolean (object_node, "font_italic", FALSE);

	string = xmlGetProp (object_node, (xmlChar *)"justify");
	align = gl_str_util_string_to_align ((gchar *)string);
	xmlFree (string);

	color_node = gl_color_node_new_default ();
	color_node->color = lgl_xml_get_prop_uint (object_node, "color", 0);

	gl_label_object_set_font_family (GL_LABEL_OBJECT(object), (gchar *)font_family, FALSE);
	gl_label_object_set_font_size (GL_LABEL_OBJECT(object), font_size, FALSE);
	gl_label_object_set_font_weight (GL_LABEL_OBJECT(object), font_weight, FALSE);
	gl_label_object_set_font_italic_flag (GL_LABEL_OBJECT(object), font_italic_flag, FALSE);
	gl_label_object_set_text_color (GL_LABEL_OBJECT(object), color_node, FALSE);
	gl_label_object_set_text_alignment (GL_LABEL_OBJECT(object), align, FALSE);
	
	gl_color_node_free (&color_node);

	lines = NULL;
	for (line_node = object_node->xmlChildrenNode; line_node != NULL;
	     line_node = line_node->next) {

		if (xmlStrEqual (line_node->name, (xmlChar *)"Line")) {

			gl_debug (DEBUG_XML, "->Line node");

			nodes = NULL;
			for (text_node = line_node->xmlChildrenNode;
			     text_node != NULL; text_node = text_node->next) {

				if (xmlStrEqual (text_node->name, (xmlChar *)"Field")) {
					gl_debug (DEBUG_XML, "->Line->Field node");
					node_text = g_new0 (glTextNode, 1);
					node_text->field_flag = TRUE;
					node_text->data =
						(gchar *)xmlGetProp (text_node, (xmlChar *)"name");
					nodes =
					    g_list_append (nodes, node_text);
				} else if (xmlNodeIsText (text_node)) {
					gl_debug (DEBUG_XML, "->Line->\"literal\" node");
					node_text = g_new0 (glTextNode, 1);
					node_text->field_flag = FALSE;
					node_text->data =
						(gchar *)xmlNodeGetContent (text_node);
					gl_debug (DEBUG_XML, "text = \"%s\"",
						  node_text->data);
					nodes =
					    g_list_append (nodes, node_text);
				} else {
					g_message ("Unexpected Text Line child: \"%s\"",
						   text_node->name);
				}

			}
			lines = g_list_append (lines, nodes);

		} else if (!xmlNodeIsText (line_node)) {
			g_message ("Unexpected Text child: \"%s\"",
				   line_node->name);
		}

	}

	gl_label_text_set_lines (object, lines, FALSE);

	gl_text_node_lines_free (&lines);
	xmlFree (font_family);

	/* Adjust location.  In 0.4.x, text was anchored at x,y */
	gl_label_object_get_position (GL_LABEL_OBJECT(object), &x, &y);
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	switch (align) {
	case PANGO_ALIGN_LEFT:
		/* nothing */
		break;
	case PANGO_ALIGN_CENTER:
		x -= w/2.0;
		gl_label_object_set_position (GL_LABEL_OBJECT(object), x, y, FALSE);
		break;
	case PANGO_ALIGN_RIGHT:
		x -= w;
		gl_label_object_set_position (GL_LABEL_OBJECT(object), x, y, FALSE);
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

	w = lgl_xml_get_prop_double (node, "w", 0);
	h = lgl_xml_get_prop_double (node, "h", 0);

	line_width = lgl_xml_get_prop_double (node, "line_width", 0);

	line_color_node = gl_color_node_new_default ();
	line_color_node->color = lgl_xml_get_prop_uint (node, "line_color", 0);
	
	fill_color_node = gl_color_node_new_default ();
	fill_color_node->color = lgl_xml_get_prop_uint (node, "fill_color", 0);

	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h, FALSE);
	gl_label_object_set_line_width (GL_LABEL_OBJECT(object), line_width, FALSE);
	gl_label_object_set_line_color (GL_LABEL_OBJECT(object), line_color_node, FALSE);
	gl_label_object_set_fill_color (GL_LABEL_OBJECT(object), fill_color_node, FALSE);

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

	w = lgl_xml_get_prop_double (node, "dx", 0);
	h = lgl_xml_get_prop_double (node, "dy", 0);

	line_width = lgl_xml_get_prop_double (node, "line_width", 0);

	line_color_node = gl_color_node_new_default ();
	line_color_node->color = lgl_xml_get_prop_uint (node, "line_color", 0);

	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h, FALSE);
	gl_label_object_set_line_width (GL_LABEL_OBJECT(object), line_width, FALSE);
	gl_label_object_set_line_color (GL_LABEL_OBJECT(object), line_color_node, FALSE);

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

	w = lgl_xml_get_prop_double (node, "w", 0);
	h = lgl_xml_get_prop_double (node, "h", 0);

	line_width = lgl_xml_get_prop_double (node, "line_width", 0);

	line_color_node = gl_color_node_new_default ();
	line_color_node->color = lgl_xml_get_prop_uint (node, "line_color", 0);
	
	fill_color_node = gl_color_node_new_default ();
	fill_color_node->color = lgl_xml_get_prop_uint (node, "fill_color", 0);

	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h, FALSE);
	gl_label_object_set_line_width (GL_LABEL_OBJECT(object), line_width, FALSE);
	gl_label_object_set_line_color (GL_LABEL_OBJECT(object), line_color_node, FALSE);
	gl_label_object_set_fill_color (GL_LABEL_OBJECT(object), fill_color_node, FALSE);

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
	filename->data = (gchar *)xmlGetProp (node, (xmlChar *)"filename");
	gl_label_image_set_filename (object, filename, FALSE);
	gl_text_node_free (&filename);

	w = lgl_xml_get_prop_double (node, "w", 0);
	h = lgl_xml_get_prop_double (node, "h", 0);
	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h, FALSE);

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
        const gchar   *backend_id;
	gboolean       text_flag;
	glColorNode   *color_node;
	gdouble        scale;
	xmlNodePtr     child;
	glTextNode    *text_node;

	gl_debug (DEBUG_XML, "START");

	color_node = gl_color_node_new_default ();
	color_node->color = lgl_xml_get_prop_uint (node, "color", 0);

	id = xmlGetProp (node, (xmlChar *)"style");
        backend_id = gl_barcode_backends_guess_backend_id (id);

	text_flag = lgl_xml_get_prop_boolean (node, "text", FALSE);
	scale =	lgl_xml_get_prop_double (node, "scale", 1.0);
	if (scale == 0.0) {
		scale = 0.5; /* Set to a valid value */
	}
	gl_label_barcode_set_props (object, (gchar *)id, (gchar *)backend_id, text_flag, TRUE, 0, FALSE);
	gl_label_object_set_line_color (GL_LABEL_OBJECT(object), color_node, FALSE);

	child = node->xmlChildrenNode;
	text_node = g_new0 (glTextNode, 1);
	if (xmlStrEqual (child->name, (xmlChar *)"Field")) {
		text_node->field_flag = TRUE;
		text_node->data       = (gchar *)xmlGetProp (child, (xmlChar *)"name");
	} else if (xmlNodeIsText (child)) {
		text_node->field_flag = FALSE;
		text_node->data       = (gchar *)xmlNodeGetContent (child);
	} else {
		g_message ("Unexpected Barcode child: \"%s\"", child->name);
	}
	gl_label_barcode_set_data (object, text_node, FALSE);

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
	xmlChar               *string;

	gl_debug (DEBUG_XML, "START");

	string = xmlGetProp (node, (xmlChar *)"type");
	merge = gl_merge_new ((gchar *)string);
	xmlFree (string);

	string = xmlGetProp (node, (xmlChar *)"src");
	gl_merge_set_src (merge, (gchar *)string);
	xmlFree (string);

	gl_label_set_merge (label, merge, FALSE);

	g_object_unref (G_OBJECT(merge));

	gl_debug (DEBUG_XML, "END");
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
