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

#include "xml-label-04.h"
#include "label-text.h"
#include "label-box.h"
#include "label-line.h"
#include "label-ellipse.h"
#include "label-image.h"
#include "label-barcode.h"

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

	if (g_strcasecmp (root->name, "Label") != 0) {
		g_warning (_("Bad root node = \"%s\""), root->name);
		*status = XML_LABEL_ERROR_OPEN_PARSE;
		return NULL;
	}

	label = GL_LABEL (gl_label_new ());

	rotate_flag =
	    !(g_strcasecmp (xmlGetProp (root, "rotate"), "false") == 0);
	gl_label_set_rotate_flag (label, rotate_flag);

	for (node = root->xmlChildrenNode; node != NULL; node = node->next) {

		gl_debug (DEBUG_XML, "node name = \"%s\"", node->name);

		if (!xmlNodeIsText (node)) {
			if (g_strcasecmp (node->name, "Media_Type") == 0) {
				if (!xml04_parse_media_description (node, label)) {
					*status = XML_LABEL_UNKNOWN_MEDIA;
				}
			} else if (g_strcasecmp (node->name, "Text") == 0) {
				object = gl_label_text_new (label);
				xml04_parse_object (node, GL_LABEL_OBJECT(object));
				xml04_parse_text_props (node, GL_LABEL_TEXT(object));
			} else if (g_strcasecmp (node->name, "Box") == 0) {
				object = gl_label_box_new (label);
				xml04_parse_object (node, GL_LABEL_OBJECT(object));
				xml04_parse_box_props (node, GL_LABEL_BOX(object));
			} else if (g_strcasecmp (node->name, "Line") == 0) {
				object = gl_label_line_new (label);
				xml04_parse_object (node, GL_LABEL_OBJECT(object));
				xml04_parse_line_props (node, GL_LABEL_LINE(object));
			} else if (g_strcasecmp (node->name, "Ellipse") == 0) {
				object = gl_label_ellipse_new (label);
				xml04_parse_object (node, GL_LABEL_OBJECT(object));
				xml04_parse_ellipse_props (node,
							   GL_LABEL_ELLIPSE(object));
			} else if (g_strcasecmp (node->name, "Image") == 0) {
				object = gl_label_image_new (label);
				xml04_parse_object (node, GL_LABEL_OBJECT(object));
				xml04_parse_image_props (node, GL_LABEL_IMAGE(object));
			} else if (g_strcasecmp (node->name, "Barcode") == 0) {
				object = gl_label_barcode_new (label);
				xml04_parse_object (node, GL_LABEL_OBJECT(object));
				xml04_parse_barcode_props (node,
							   GL_LABEL_BARCODE(object));
			} else if (g_strcasecmp (node->name, "Merge_Properties") == 0) {
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
	gchar      *template_name;
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

	gl_template_free (&template);
	g_free (template_name);

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

	x = g_strtod (xmlGetProp (object_node, "x"), NULL);
	y = g_strtod (xmlGetProp (object_node, "y"), NULL);

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
	gchar            *font_family;
	gdouble          font_size;
	GnomeFontWeight  font_weight;
	gboolean         font_italic_flag;
	guint            color;
	GtkJustification just;
	xmlNodePtr       line_node, text_node;
	glTextNode       *node_text;
	GList            *nodes, *lines;
	gdouble          w, h, x, y;

	gl_debug (DEBUG_XML, "START");

	font_family = xmlGetProp (object_node, "font_family");
	font_size = g_strtod (xmlGetProp (object_node, "font_size"), NULL);
	font_weight =
		gl_util_string_to_weight (xmlGetProp (object_node, "font_weight"));
	font_italic_flag =
		!(g_strcasecmp (xmlGetProp (object_node, "font_italic"), "false") ==
		  0);

	just = gl_util_string_to_just (xmlGetProp (object_node, "justify"));

	sscanf (xmlGetProp (object_node, "color"), "%x", &color);

	gl_label_text_set_props (object,
				 font_family,
				 font_size,
				 font_weight,
				 font_italic_flag,
				 color,
				 just);

	lines = NULL;
	for (line_node = object_node->xmlChildrenNode; line_node != NULL;
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

	gl_label_text_set_lines (object, lines);

	gl_text_node_lines_free (&lines);
	g_free (font_family);

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
	guint line_color, fill_color;

	gl_debug (DEBUG_XML, "START");

	w = g_strtod (xmlGetProp (node, "w"), NULL);
	h = g_strtod (xmlGetProp (node, "h"), NULL);

	line_width = g_strtod (xmlGetProp (node, "line_width"), NULL);

	sscanf (xmlGetProp (node, "line_color"), "%x", &line_color);
	sscanf (xmlGetProp (node, "fill_color"), "%x", &fill_color);

	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);
	gl_label_box_set_line_width (object, line_width);
	gl_label_box_set_line_color (object, line_color);
	gl_label_box_set_fill_color (object, fill_color);

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
	guint line_color;

	gl_debug (DEBUG_XML, "START");

	w = g_strtod (xmlGetProp (node, "dx"), NULL);
	h = g_strtod (xmlGetProp (node, "dy"), NULL);

	line_width = g_strtod (xmlGetProp (node, "line_width"), NULL);

	sscanf (xmlGetProp (node, "line_color"), "%x", &line_color);

	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);
	gl_label_line_set_line_width (object, line_width);
	gl_label_line_set_line_color (object, line_color);

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
	guint line_color, fill_color;

	gl_debug (DEBUG_XML, "START");

	w = g_strtod (xmlGetProp (node, "w"), NULL);
	h = g_strtod (xmlGetProp (node, "h"), NULL);

	line_width = g_strtod (xmlGetProp (node, "line_width"), NULL);

	sscanf (xmlGetProp (node, "line_color"), "%x", &line_color);
	sscanf (xmlGetProp (node, "fill_color"), "%x", &fill_color);

	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);
	gl_label_ellipse_set_line_width (object, line_width);
	gl_label_ellipse_set_line_color (object, line_color);
	gl_label_ellipse_set_fill_color (object, fill_color);

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

	w = g_strtod (xmlGetProp (node, "w"), NULL);
	h = g_strtod (xmlGetProp (node, "h"), NULL);
	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);

	filename = g_new0 (glTextNode, 1);
	filename->field_flag = FALSE;
	filename->data = xmlGetProp (node, "filename");
	gl_label_image_set_filename (object, filename);
	gl_text_node_free (&filename);

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Barcode Node Properties                       */
/*--------------------------------------------------------------------------*/
static void
xml04_parse_barcode_props (xmlNodePtr    node,
			   glLabelBarcode *object)
{
	glBarcodeStyle style;
	gboolean       text_flag;
	guint          color;
	gdouble        scale;
	xmlNodePtr     child;
	glTextNode     *text_node;

	gl_debug (DEBUG_XML, "START");

	sscanf (xmlGetProp (node, "color"), "%x", &color);
	style = gl_barcode_text_to_style (xmlGetProp (node, "style"));
	text_flag = !(g_strcasecmp (xmlGetProp (node, "text"), "false") == 0);
	scale =	g_strtod (xmlGetProp (node, "scale"), NULL);
	if (scale == 0.0) {
		scale = 0.5; /* Set to a valid value */
	}
	gl_label_barcode_set_props (object, style, text_flag, color, scale);

	child = node->xmlChildrenNode;
	text_node = g_new0 (glTextNode, 1);
	if (g_strcasecmp (child->name, "Field") == 0) {
		text_node->field_flag = TRUE;
		text_node->data       = xmlGetProp (child, "name");
	} else if (xmlNodeIsText (child)) {
		text_node->field_flag = FALSE;
		text_node->data       = xmlNodeGetContent (child);
	} else {
		g_warning ("Unexpected Barcode child: \"%s\"", child->name);
	}
	gl_label_barcode_set_data (object, text_node);

	gl_text_node_free (&text_node);

	gl_debug (DEBUG_XML, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML merge properties tag.                                */
/*--------------------------------------------------------------------------*/
static void
xml04_parse_merge_properties (xmlNodePtr node,
			      glLabel    *label)
{
	glMerge                *merge;
	xmlNodePtr             child;

	gl_debug (DEBUG_XML, "START");

	merge = gl_merge_new (xmlGetProp (node, "type"));
	gl_merge_set_src (merge, xmlGetProp (node, "src"));

	gl_label_set_merge (label, merge);

	g_object_unref (G_OBJECT(merge));

	gl_debug (DEBUG_XML, "END");
}

