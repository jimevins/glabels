/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label.c:  GLabels label module
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

/* LibXML */
#include <tree.h>
#include <parser.h>

#include "label.h"
#include "template.h"

#include "debug.h"

#define NAME_SPACE "http://snaught.com/glabels/0.4/"
#define COMPAT01_NAME_SPACE "http://snaught.com/glabels/0.1/"

static glLabelStatus xml_doc_to_label (xmlDocPtr doc, glLabel ** label);
static gboolean xml_parse_media_description (xmlNodePtr node, glLabel * label);
static void xml_parse_object(xmlNodePtr node, glLabelObject * object,
			     gboolean compat01_flag);
static void xml_parse_text_props (xmlNodePtr node, glLabelObject * object);
static void xml_parse_text_props_compat01( xmlNodePtr node,
					   glLabelObject *object );
static void xml_parse_box_props (xmlNodePtr node, glLabelObject * object);
static void xml_parse_line_props (xmlNodePtr node, glLabelObject * object);
static void xml_parse_ellipse_props (xmlNodePtr node, glLabelObject * object);
static void xml_parse_image_props (xmlNodePtr node, glLabelObject * object);
static void xml_parse_barcode_props (xmlNodePtr node, glLabelObject * object);
static void xml_parse_merge_properties (xmlNodePtr node, glLabel * label);

static glLabelStatus xml_label_to_doc (glLabel * label, xmlDocPtr * doc);
static void xml_create_media_description (xmlNodePtr root, xmlNsPtr ns,
				       glLabel * label);
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
static void xml_create_merge_properties (xmlNodePtr root, xmlNsPtr ns,
				      glLabel * label);

static const gchar *just_to_text (GtkJustification just);
static GtkJustification text_to_just (const gchar * text);

static const gchar *weight_to_text (GnomeFontWeight weight);
static GnomeFontWeight text_to_weight (const gchar * text);

/****************************************************************************/
/* Create a new label.                                                      */
/****************************************************************************/
glLabel *
gl_label_new (void)
{
	glLabel *label;

	label = g_new0 (glLabel, 1);

	return label;
}

/****************************************************************************/
/* Create a new label.                                                      */
/****************************************************************************/
glLabel *
gl_label_new_with_template (const gchar * tmplt_name,
			    gboolean rotate_flag)
{
	glLabel *label;
	glTemplate *template;

	label = gl_label_new ();

	label->template_name = g_strdup (tmplt_name);
	label->template = template = gl_template_from_name (tmplt_name);
	label->rotate_flag = rotate_flag;

	switch (template->style) {

	case GL_TEMPLATE_STYLE_RECT:
		if (!rotate_flag) {
			label->width = template->label_width;
			label->height = template->label_height;
		} else {
			label->width = template->label_height;
			label->height = template->label_width;
		}
		break;

	case GL_TEMPLATE_STYLE_ROUND:
	case GL_TEMPLATE_STYLE_CD:
		label->width = label->height = 2.0 * template->label_radius;
		break;

	default:
		WARN ("Unknown template label style");
		break;
	}

	label->objects = NULL;

	return label;
}

/****************************************************************************/
/* Free a previously allocated label.                                       */
/****************************************************************************/
void
gl_label_free (glLabel ** label)
{
	GList *p, *p_next;
	glLabelObject *object;

	if (*label != NULL) {
		g_free ((*label)->template_name);
		(*label)->template_name = NULL;

		gl_template_free (&(*label)->template);

		for (p = (*label)->objects; p != NULL; p = p_next) {
			p_next = p->next;	/* NOTE: p will be left dangling */
			object = (glLabelObject *) p->data;
			gl_label_object_free (&object);
		}

		g_free (*label);
		*label = NULL;
	}
}

/****************************************************************************/
/* Open and read label from xml file.                                       */
/****************************************************************************/
glLabelStatus
gl_label_open_xml (glLabel ** label,
		   const gchar * filename)
{
	xmlDocPtr doc;
	glLabelStatus status;

	xmlUseNewParser (TRUE);
	doc = xmlParseFile (filename);
	if (!doc) {
		WARN (_("xmlParseFile error"));
		*label = NULL;
		return LABEL_ERROR_OPEN_XML_PARSE;
	}

	status = xml_doc_to_label (doc, label);

	xmlFreeDoc (doc);

	return status;
}

/****************************************************************************/
/* Read label from xml buffer.                                              */
/****************************************************************************/
glLabelStatus
gl_label_open_xml_buffer (glLabel ** label,
			  const gchar * buffer)
{
	xmlDocPtr doc;
	glLabelStatus status;

	xmlUseNewParser (TRUE);
	doc = xmlParseDoc ((xmlChar *) buffer);
	if (!doc) {
		WARN (_("xmlParseFile error"));
		*label = NULL;
		return LABEL_ERROR_OPEN_XML_PARSE;
	}

	status = xml_doc_to_label (doc, label);

	xmlFreeDoc (doc);

	return status;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse xml doc structure and create label.                      */
/*--------------------------------------------------------------------------*/
static glLabelStatus
xml_doc_to_label (xmlDocPtr doc,
		  glLabel ** label)
{
	xmlNodePtr root, node;
	xmlNsPtr ns;
	glLabelObject *object;
	glLabelStatus status = LABEL_OK;
	gboolean compat01_flag = FALSE;

	LIBXML_TEST_VERSION;

	gl_label_free (label);

	root = xmlDocGetRootElement (doc);
	if (!root || !root->name) {
		WARN (_("No document root"));
		return LABEL_ERROR_OPEN_XML_PARSE;
	}
	ns = xmlSearchNsByHref (doc, root, NAME_SPACE);
	if (ns == NULL) {
		/* Try compatability mode */
		ns = xmlSearchNsByHref (doc, root, COMPAT01_NAME_SPACE);
		if (ns != NULL)	compat01_flag = TRUE;
	}
	if (ns == NULL) {
		WARN (_
		      ("document of the wrong type, glabels Namespace not found"));
		return LABEL_ERROR_OPEN_XML_PARSE;
	}

	if (g_strcasecmp (root->name, "Label") != 0) {
		WARN (_("Bad root node = \"%s\""), root->name);
		return LABEL_ERROR_OPEN_XML_PARSE;
	}

	*label = gl_label_new ();

	(*label)->rotate_flag =
	    !(g_strcasecmp (xmlGetProp (root, "rotate"), "false") == 0);
	(*label)->width = g_strtod (xmlGetProp (root, "width"), NULL);
	(*label)->height = g_strtod (xmlGetProp (root, "height"), NULL);

	for (node = root->xmlChildrenNode; node != NULL; node = node->next) {

		if (g_strcasecmp (node->name, "Media_Type") == 0) {
			if (!xml_parse_media_description (node, *label)) {
				status = LABEL_UNKNOWN_MEDIA;
			}
		} else if (g_strcasecmp (node->name, "Text") == 0) {
			object =
			    gl_label_object_new (*label, GL_LABEL_OBJECT_TEXT);
			xml_parse_object (node, object, compat01_flag);
		} else if (g_strcasecmp (node->name, "Box") == 0) {
			object =
			    gl_label_object_new (*label, GL_LABEL_OBJECT_BOX);
			xml_parse_object (node, object, compat01_flag);
		} else if (g_strcasecmp (node->name, "Line") == 0) {
			object =
			    gl_label_object_new (*label, GL_LABEL_OBJECT_LINE);
			xml_parse_object (node, object, compat01_flag);
		} else if (g_strcasecmp (node->name, "Ellipse") == 0) {
			object =
			    gl_label_object_new (*label,
						 GL_LABEL_OBJECT_ELLIPSE);
			xml_parse_object (node, object, compat01_flag);
		} else if (g_strcasecmp (node->name, "Image") == 0) {
			object =
			    gl_label_object_new (*label, GL_LABEL_OBJECT_IMAGE);
			xml_parse_object (node, object, compat01_flag);
		} else if (g_strcasecmp (node->name, "Barcode") == 0) {
			object =
			    gl_label_object_new (*label,
						 GL_LABEL_OBJECT_BARCODE);
			xml_parse_object (node, object, compat01_flag);
		} else if (g_strcasecmp (node->name, "Merge_Properties") == 0) {
			xml_parse_merge_properties (node, *label);
		} else {
			if (!xmlNodeIsText (node)) {
				WARN (_("bad node =  \"%s\""), node->name);
			}
		}
	}

	return status;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse Media Description node.                                  */
/*--------------------------------------------------------------------------*/
static gboolean
xml_parse_media_description (xmlNodePtr node,
			     glLabel * label)
{
	label->template_name = xmlNodeGetContent (node);
	label->template = gl_template_from_name (label->template_name);
	if (label->template == NULL) {
		WARN ("Undefined template \"%s\"", label->template_name);
		/* Get a default */
		label->template = gl_template_from_name (NULL);
		return FALSE;
	}
	return TRUE;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Object Node                                          */
/*--------------------------------------------------------------------------*/
static void
xml_parse_object (xmlNodePtr object_node,
		  glLabelObject * object,
		  gboolean compat01_flag)
{
	object->x = g_strtod (xmlGetProp (object_node, "x"), NULL);
	object->y = g_strtod (xmlGetProp (object_node, "y"), NULL);

	switch (object->type) {
	case GL_LABEL_OBJECT_TEXT:
		if (compat01_flag) {
			xml_parse_text_props_compat01 (object_node, object);
		} else {
			xml_parse_text_props (object_node, object);
		}
		break;
	case GL_LABEL_OBJECT_BOX:
		xml_parse_box_props (object_node, object);
		break;
	case GL_LABEL_OBJECT_LINE:
		xml_parse_line_props (object_node, object);
		break;
	case GL_LABEL_OBJECT_ELLIPSE:
		xml_parse_ellipse_props (object_node, object);
		break;
	case GL_LABEL_OBJECT_IMAGE:
		xml_parse_image_props (object_node, object);
		break;
	case GL_LABEL_OBJECT_BARCODE:
		xml_parse_barcode_props (object_node, object);
		break;
	default:
	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Text Node Properties                          */
/*--------------------------------------------------------------------------*/
static void
xml_parse_text_props (xmlNodePtr object_node,
		       glLabelObject * object)
{
	xmlNodePtr line_node, text_node;
	glTextNode *node_text;
	GList *nodes;

	object->arg.text.font_family = xmlGetProp (object_node, "font_family");
	object->arg.text.font_size =
	    g_strtod (xmlGetProp (object_node, "font_size"), NULL);
	object->arg.text.font_weight =
	    text_to_weight (xmlGetProp (object_node, "font_weight"));
	object->arg.text.font_italic_flag =
	    !(g_strcasecmp (xmlGetProp (object_node, "font_italic"), "false") ==
	      0);

	object->arg.text.just =
	    text_to_just (xmlGetProp (object_node, "justify"));

	sscanf (xmlGetProp (object_node, "color"), "%x",
		&object->arg.text.color);

	object->arg.text.lines = NULL;
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
					WARN ("Unexpected Text Line child: \"%s\"",
					      text_node->name);
				}

			}
			object->arg.text.lines =
			    g_list_append (object->arg.text.lines, nodes);

		} else {
			WARN ("Unexpected Text child: \"%s\"", line_node->name);
		}

	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML label->Text node (Compatability with 0.1 version)    */
/*--------------------------------------------------------------------------*/
static void
xml_parse_text_props_compat01( xmlNodePtr node, glLabelObject *object )
{
	gchar *text;

	text = xmlNodeGetContent( node );
	object->arg.text.lines = gl_text_node_lines_new_from_text( text );
	g_free( text );

	object->arg.text.font_family = xmlGetProp( node, "font_family" );
	object->arg.text.font_size = g_strtod( xmlGetProp( node, "font_size" ),
					       NULL );
	object->arg.text.font_weight =
		text_to_weight(	xmlGetProp( node, "font_weight" ) );
	object->arg.text.font_italic_flag =
		!( g_strcasecmp( xmlGetProp( node, "font_italic" ),
				 "false" ) == 0 );

	object->arg.text.just = text_to_just( xmlGetProp( node, "justify" ) );

	sscanf( xmlGetProp( node, "color" ), "%x", &object->arg.text.color );
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Box Node Properties                           */
/*--------------------------------------------------------------------------*/
static void
xml_parse_box_props (xmlNodePtr node,
		      glLabelObject * object)
{
	object->arg.box.w = g_strtod (xmlGetProp (node, "w"), NULL);
	object->arg.box.h = g_strtod (xmlGetProp (node, "h"), NULL);

	object->arg.box.line_width = g_strtod (xmlGetProp (node, "line_width"),
					       NULL);

	sscanf (xmlGetProp (node, "line_color"), "%x",
		&object->arg.box.line_color);
	sscanf (xmlGetProp (node, "fill_color"), "%x",
		&object->arg.box.fill_color);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Line Node Properties                          */
/*--------------------------------------------------------------------------*/
static void
xml_parse_line_props (xmlNodePtr node,
		       glLabelObject * object)
{
	object->arg.line.dx = g_strtod (xmlGetProp (node, "dx"), NULL);
	object->arg.line.dy = g_strtod (xmlGetProp (node, "dy"), NULL);

	object->arg.line.line_width = g_strtod (xmlGetProp (node, "line_width"),
						NULL);

	sscanf (xmlGetProp (node, "line_color"), "%x",
		&object->arg.line.line_color);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Ellipse Node Properties                       */
/*--------------------------------------------------------------------------*/
static void
xml_parse_ellipse_props (xmlNodePtr node,
			  glLabelObject * object)
{
	object->arg.ellipse.w = g_strtod (xmlGetProp (node, "w"), NULL);
	object->arg.ellipse.h = g_strtod (xmlGetProp (node, "h"), NULL);

	object->arg.ellipse.line_width =
	    g_strtod (xmlGetProp (node, "line_width"), NULL);

	sscanf (xmlGetProp (node, "line_color"), "%x",
		&object->arg.ellipse.line_color);
	sscanf (xmlGetProp (node, "fill_color"), "%x",
		&object->arg.ellipse.fill_color);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Image Node Properties                         */
/*--------------------------------------------------------------------------*/
static void
xml_parse_image_props (xmlNodePtr node,
			glLabelObject * object)
{
	object->arg.image.w = g_strtod (xmlGetProp (node, "w"), NULL);
	object->arg.image.h = g_strtod (xmlGetProp (node, "h"), NULL);

	object->arg.image.filename = xmlGetProp (node, "filename");

	object->arg.image.image =
	    gdk_pixbuf_new_from_file (object->arg.image.filename);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Barcode Node Properties                       */
/*--------------------------------------------------------------------------*/
static void
xml_parse_barcode_props (xmlNodePtr node,
			  glLabelObject * object)
{
	xmlNodePtr child;

	sscanf (xmlGetProp (node, "color"), "%x", &object->arg.barcode.color);

	object->arg.barcode.style =
	    gl_barcode_text_to_style (xmlGetProp (node, "style"));

	object->arg.barcode.text_flag =
	    !(g_strcasecmp (xmlGetProp (node, "text"), "false") == 0);

	object->arg.barcode.scale =
		g_strtod (xmlGetProp (node, "scale"), NULL);
	if (object->arg.barcode.scale == 0.0) {
		object->arg.barcode.scale = 0.5; /* Set to a valid value */
	}

	child = node->xmlChildrenNode;
	object->arg.barcode.text_node = g_new0 (glTextNode, 1);
	if (g_strcasecmp (child->name, "Field") == 0) {
		object->arg.barcode.text_node->field_flag = TRUE;
		object->arg.barcode.text_node->data =
			xmlGetProp (child, "name");
	} else if (xmlNodeIsText (child)) {
		object->arg.barcode.text_node->field_flag = FALSE;
		object->arg.barcode.text_node->data =
			xmlNodeGetContent (child);
	} else {
		WARN ("Unexpected Barcode child: \"%s\"", child->name);
	}
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML merge properties tag.                                */
/*--------------------------------------------------------------------------*/
static void
xml_parse_merge_properties (xmlNodePtr node,
			    glLabel * label)
{
	xmlNodePtr child;
	glMergeFieldDefinition *field_def;

	label->merge_type = gl_merge_text_to_type (xmlGetProp (node, "type"));
	label->merge_src = xmlGetProp (node, "src");

	for (child = node->xmlChildrenNode; child != NULL; child = child->next) {

		if (g_strcasecmp (child->name, "Field") == 0) {
			field_def = g_new0 (glMergeFieldDefinition, 1);
			field_def->key = xmlGetProp (child, "key");
			field_def->loc = xmlGetProp (child, "loc");
			label->merge_fields =
			    g_list_append (label->merge_fields, field_def);
		} else {
			WARN ("Unexpected Merge_Properties child: \"%s\"",
			      child->name);
		}

	}

}

/****************************************************************************/
/* Save label to xml label file.                                            */
/****************************************************************************/
glLabelStatus
gl_label_save_xml (glLabel * label,
		   const gchar * filename)
{
	xmlDocPtr doc;
	glLabelStatus status;
	gint xml_ret;

	status = xml_label_to_doc (label, &doc);

	xml_ret = xmlSaveFile (filename, doc);
	xmlFreeDoc (doc);
	if (xml_ret == -1) {
		WARN (_("Problem saving xml file."));
		return LABEL_ERROR_SAVE_XML_FILE;
	}

	return status;
}

/****************************************************************************/
/* Save label to xml buffer.                                                */
/****************************************************************************/
glLabelStatus
gl_label_save_xml_buffer (glLabel * label,
			  gchar ** buffer)
{
	xmlDocPtr doc;
	glLabelStatus status;
	gint size;

	status = xml_label_to_doc (label, &doc);

	xmlDocDumpMemory (doc, (xmlChar **) buffer, &size);
	xmlFreeDoc (doc);

	return status;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Convert label to xml doc structure.                            */
/*--------------------------------------------------------------------------*/
static glLabelStatus
xml_label_to_doc (glLabel * label,
		  xmlDocPtr * doc)
{
	xmlNsPtr ns;
	gchar *string;
	GList *p;
	glLabelObject *object;

	LIBXML_TEST_VERSION;

	*doc = xmlNewDoc ("1.0");
	(*doc)->xmlRootNode = xmlNewDocNode (*doc, NULL, "Label", NULL);

	ns = xmlNewNs ((*doc)->xmlRootNode, NAME_SPACE, "glabels");
	xmlSetNs ((*doc)->xmlRootNode, ns);

	xmlSetProp ((*doc)->xmlRootNode, "rotate",
		    label->rotate_flag ? "True" : "False");

	string = g_strdup_printf ("%g", label->width);
	xmlSetProp ((*doc)->xmlRootNode, "width", string);
	g_free (string);

	string = g_strdup_printf ("%g", label->height);
	xmlSetProp ((*doc)->xmlRootNode, "height", string);
	g_free (string);

	xml_create_media_description ((*doc)->xmlRootNode, ns, label);

	for (p = label->objects; p != NULL; p = p->next) {
		object = (glLabelObject *) p->data;
		xml_create_object ((*doc)->xmlRootNode, ns, object);
	}

	if (label->merge_type != GL_MERGE_NONE) {
		xml_create_merge_properties ((*doc)->xmlRootNode, ns, label);
	}

	return LABEL_OK;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Label->Text Node                                     */
/*--------------------------------------------------------------------------*/
static void
xml_create_media_description (xmlNodePtr root,
			   xmlNsPtr ns,
			   glLabel * label)
{
	xmlNodePtr node;

	node = xmlNewTextChild (root, ns, "Media_Type", label->template_name);
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
	gchar *string;

	object_node = xmlNewChild (root, ns, "Object", NULL);

	string = g_strdup_printf ("%g", object->x);
	xmlSetProp (object_node, "x", string);
	g_free (string);

	string = g_strdup_printf ("%g", object->y);
	xmlSetProp (object_node, "y", string);
	g_free (string);

	switch (object->type) {
	case GL_LABEL_OBJECT_TEXT:
		xml_create_text_props (object_node, ns, object);
		break;
	case GL_LABEL_OBJECT_BOX:
		xml_create_box_props (object_node, ns, object);
		break;
	case GL_LABEL_OBJECT_LINE:
		xml_create_line_props (object_node, ns, object);
		break;
	case GL_LABEL_OBJECT_ELLIPSE:
		xml_create_ellipse_props (object_node, ns, object);
		break;
	case GL_LABEL_OBJECT_IMAGE:
		xml_create_image_props (object_node, ns, object);
		break;
	case GL_LABEL_OBJECT_BARCODE:
		xml_create_barcode_props (object_node, ns, object);
		break;
	default:
		WARN ("Unknown label object");
	}
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
	gchar *string;
	GList *p_line, *p_node;
	glTextNode *node_text;

	xmlNodeSetName (object_node, "Text");

	xmlSetProp (object_node, "font_family", object->arg.text.font_family);

	string = g_strdup_printf ("%g", object->arg.text.font_size);
	xmlSetProp (object_node, "font_size", string);
	g_free (string);

	xmlSetProp (object_node, "font_weight",
		    weight_to_text (object->arg.text.font_weight));

	if (object->arg.text.font_italic_flag) {
		xmlSetProp (object_node, "font_italic", "True");
	} else {
		xmlSetProp (object_node, "font_italic", "False");
	}

	xmlSetProp (object_node, "justify",
		    just_to_text (object->arg.text.just));

	string = g_strdup_printf ("0x%08x", object->arg.text.color);
	xmlSetProp (object_node, "color", string);
	g_free (string);

	for (p_line = object->arg.text.lines; p_line != NULL;
	     p_line = p_line->next) {
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

	xmlNodeSetName (object_node, "Box");

	string = g_strdup_printf ("%g", object->arg.box.w);
	xmlSetProp (object_node, "w", string);
	g_free (string);

	string = g_strdup_printf ("%g", object->arg.box.h);
	xmlSetProp (object_node, "h", string);
	g_free (string);

	string = g_strdup_printf ("%g", object->arg.box.line_width);
	xmlSetProp (object_node, "line_width", string);
	g_free (string);

	string = g_strdup_printf ("0x%08x", object->arg.box.line_color);
	xmlSetProp (object_node, "line_color", string);
	g_free (string);

	string = g_strdup_printf ("0x%08x", object->arg.box.fill_color);
	xmlSetProp (object_node, "fill_color", string);
	g_free (string);

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

	xmlNodeSetName (object_node, "Line");

	string = g_strdup_printf ("%g", object->arg.line.dx);
	xmlSetProp (object_node, "dx", string);
	g_free (string);

	string = g_strdup_printf ("%g", object->arg.line.dy);
	xmlSetProp (object_node, "dy", string);
	g_free (string);

	string = g_strdup_printf ("%g", object->arg.line.line_width);
	xmlSetProp (object_node, "line_width", string);
	g_free (string);

	string = g_strdup_printf ("0x%08x", object->arg.line.line_color);
	xmlSetProp (object_node, "line_color", string);
	g_free (string);

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

	xmlNodeSetName (object_node, "Ellipse");

	string = g_strdup_printf ("%g", object->arg.ellipse.w);
	xmlSetProp (object_node, "w", string);
	g_free (string);

	string = g_strdup_printf ("%g", object->arg.ellipse.h);
	xmlSetProp (object_node, "h", string);
	g_free (string);

	string = g_strdup_printf ("%g", object->arg.ellipse.line_width);
	xmlSetProp (object_node, "line_width", string);
	g_free (string);

	string = g_strdup_printf ("0x%08x", object->arg.ellipse.line_color);
	xmlSetProp (object_node, "line_color", string);
	g_free (string);

	string = g_strdup_printf ("0x%08x", object->arg.ellipse.fill_color);
	xmlSetProp (object_node, "fill_color", string);
	g_free (string);

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

	xmlNodeSetName (object_node, "Image");

	string = g_strdup_printf ("%g", object->arg.image.w);
	xmlSetProp (object_node, "w", string);
	g_free (string);

	string = g_strdup_printf ("%g", object->arg.image.h);
	xmlSetProp (object_node, "h", string);
	g_free (string);

	xmlSetProp (object_node, "filename", object->arg.image.filename);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label->Barcode Node Properties                         */
/*--------------------------------------------------------------------------*/
static void
xml_create_barcode_props (xmlNodePtr object_node,
			  xmlNsPtr ns,
			  glLabelObject * object)
{
	xmlNodePtr child;
	gchar *string;

	xmlNodeSetName (object_node, "Barcode");

	string = g_strdup_printf ("0x%08x", object->arg.barcode.color);
	xmlSetProp (object_node, "color", string);
	g_free (string);

	xmlSetProp (object_node, "style",
		    gl_barcode_style_to_text (object->arg.barcode.style));

	if (object->arg.barcode.text_flag) {
		xmlSetProp (object_node, "text", "True");
	} else {
		xmlSetProp (object_node, "text", "False");
	}

	string = g_strdup_printf ("%g", object->arg.barcode.scale);
	xmlSetProp (object_node, "scale", string);
	g_free (string);

	if (object->arg.barcode.text_node->field_flag) {
		child = xmlNewChild (object_node, ns, "Field", NULL);
		xmlSetProp (child, "name",
			    object->arg.barcode.text_node->data);
	} else {
		xmlNodeSetContent (object_node,
				   object->arg.barcode.text_node->data);
	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Label Merge Properties Node                            */
/*--------------------------------------------------------------------------*/
static void
xml_create_merge_properties (xmlNodePtr root,
			     xmlNsPtr ns,
			     glLabel * label)
{
	xmlNodePtr node, child;
	gchar *string;
	GList *p;
	glMergeFieldDefinition *field_def;

	node = xmlNewChild (root, ns, "Merge_Properties", NULL);

	string = gl_merge_type_to_text (label->merge_type);
	xmlSetProp (node, "type", string);
	g_free (string);

	xmlSetProp (node, "src", label->merge_src);

	for (p = label->merge_fields; p != NULL; p = p->next) {
		field_def = (glMergeFieldDefinition *) p->data;

		child = xmlNewChild (node, ns, "Field", NULL);
		xmlSetProp (child, "key", field_def->key);
		xmlSetProp (child, "loc", field_def->loc);
	}

}

/****************************************************************************/
/* Create a new label object of the given type.                             */
/****************************************************************************/
glLabelObject *
gl_label_object_new (glLabel * label,
		     glLabelObjectType type)
{
	glLabelObject *object;

	object = g_new0 (glLabelObject, 1);
	object->parent = label;
	object->type = type;

	if (label != NULL) {
		label->objects = g_list_append (label->objects, object);
	}

	return object;
}

/****************************************************************************/
/* Create a new label object from an existing object                        */
/****************************************************************************/
glLabelObject *
gl_label_object_new_from_object (glLabel * label,
				 glLabelObject * src_object)
{
	glLabelObject *object;
	GList *p_line, *nodes, *p_node;
	glTextNode *node, *src_node;

	object = g_new0 (glLabelObject, 1);
	*object = *src_object;

	/* deep copy */
	switch (object->type) {

	case GL_LABEL_OBJECT_TEXT:
		/* deep copy */
		gl_text_node_lines_print (src_object->arg.text.lines);
		object->arg.text.lines = NULL;
		for (p_line = src_object->arg.text.lines; p_line != NULL;
		     p_line = p_line->next) {
			nodes = NULL;
			for (p_node = (GList *) p_line->data; p_node != NULL;
			     p_node = p_node->next) {
				src_node = (glTextNode *) p_node->data;
				node = g_new0 (glTextNode, 1);
				node->field_flag = src_node->field_flag;
				node->data = g_strdup (src_node->data);
				nodes = g_list_append (nodes, node);
			}
			object->arg.text.lines =
			    g_list_append (object->arg.text.lines, nodes);
		}
		object->arg.text.font_family =
		    g_strdup (src_object->arg.text.font_family);
		break;

	case GL_LABEL_OBJECT_IMAGE:
		object->arg.image.filename
		    = g_strdup (src_object->arg.image.filename);
		object->arg.image.image
		    = gdk_pixbuf_copy (src_object->arg.image.image);
		break;

	case GL_LABEL_OBJECT_BARCODE:
		object->arg.barcode.text_node = g_new0 (glTextNode, 1);
		object->arg.barcode.text_node->field_flag =
		    src_object->arg.barcode.text_node->field_flag;
		object->arg.barcode.text_node->data =
		    g_strdup (src_object->arg.barcode.text_node->data);
		break;

	default:
		break;

	}

	/* set parent */
	object->parent = label;
	if (label != NULL) {
		label->objects = g_list_append (label->objects, object);
	}

	return object;
}

/****************************************************************************/
/* Free a previously allocated label object.                                */
/****************************************************************************/
void
gl_label_object_free (glLabelObject ** object)
{
	glLabel *label;

	switch ((*object)->type) {

	case GL_LABEL_OBJECT_TEXT:
		gl_text_node_lines_free (&((*object)->arg.text.lines));
		g_free ((*object)->arg.text.font_family);
		(*object)->arg.text.font_family = NULL;
		break;

	case GL_LABEL_OBJECT_IMAGE:
		g_free ((*object)->arg.image.filename);
		(*object)->arg.image.filename = NULL;
		gdk_pixbuf_unref ((*object)->arg.image.image);
		(*object)->arg.image.image = NULL;
		break;

	case GL_LABEL_OBJECT_BARCODE:
		gl_text_node_free (&((*object)->arg.barcode.text_node));
		break;

	default:
		break;

	}

	label = (*object)->parent;
	if (label != NULL) {
		label->objects = g_list_remove (label->objects, *object);
	}

	g_free (*object);
	*object = NULL;
}

/****************************************************************************/
/* Bring label object to front/top.                                         */
/****************************************************************************/
void
gl_label_object_raise_to_front (glLabelObject * object)
{
	glLabel *label;

	label = object->parent;

	/* Move to end of list, representing front most object */
	label->objects = g_list_remove (label->objects, object);
	label->objects = g_list_append (label->objects, object);
}

/****************************************************************************/
/* Send label object to rear/bottom.                                        */
/****************************************************************************/
void
gl_label_object_lower_to_back (glLabelObject * object)
{
	glLabel *label;

	label = object->parent;

	/* Move to front of list, representing rear most object */
	label->objects = g_list_remove (label->objects, object);
	label->objects = g_list_prepend (label->objects, object);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Utilities to deal with GTK_JUSTIFICATION types                 */
/*--------------------------------------------------------------------------*/
static const gchar *
just_to_text (GtkJustification just)
{
	switch (just) {
	case GTK_JUSTIFY_LEFT:
		return "Left";
	case GTK_JUSTIFY_CENTER:
		return "Center";
	case GTK_JUSTIFY_RIGHT:
		return "Right";
	default:
		return "?";
	}
}

static GtkJustification
text_to_just (const gchar * text)
{

	if (g_strcasecmp (text, "Left") == 0) {
		return GTK_JUSTIFY_LEFT;
	} else if (g_strcasecmp (text, "Center") == 0) {
		return GTK_JUSTIFY_CENTER;
	} else if (g_strcasecmp (text, "Right") == 0) {
		return GTK_JUSTIFY_RIGHT;
	} else {
		return GTK_JUSTIFY_LEFT;
	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Utilities to deal with GNOME_FONT_WEIGHT types                 */
/*--------------------------------------------------------------------------*/
static const gchar *
weight_to_text (GnomeFontWeight weight)
{
	switch (weight) {
	case GNOME_FONT_BOOK:
		return "Regular";
	case GNOME_FONT_BOLD:
		return "Bold";
	default:
		return "?";
	}
}

static GnomeFontWeight
text_to_weight (const gchar * text)
{

	if (g_strcasecmp (text, "Regular") == 0) {
		return GNOME_FONT_BOOK;
	} else if (g_strcasecmp (text, "Bold") == 0) {
		return GNOME_FONT_BOLD;
	} else {
		return GNOME_FONT_BOOK;
	}

}
