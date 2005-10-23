/*
 *  (LIBGLABELS) Template library for GLABELS
 *
 *  xml-template.c:  template xml module
 *
 *  Copyright (C) 2001-2004  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of the LIBGLABELS library.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 *  MA 02111-1307, USA
 */
#include <config.h>

#include "xml-template.h"

#include <glib/gi18n.h>
#include <glib/gmessages.h>
#include <string.h>
#include <libintl.h>

#include "libglabels-private.h"

#include "paper.h"
#include "xml.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/
static void  xml_parse_label_rectangle_node (xmlNodePtr              label_node,
					     glTemplate             *template);
static void  xml_parse_label_round_node     (xmlNodePtr              label_node,
					     glTemplate             *template);
static void  xml_parse_label_cd_node        (xmlNodePtr              label_node,
					     glTemplate             *template);
static void  xml_parse_layout_node          (xmlNodePtr              layout_node,
					     glTemplateLabelType    *label_type);
static void  xml_parse_markup_margin_node   (xmlNodePtr              markup_node,
					     glTemplateLabelType    *label_type);
static void  xml_parse_markup_line_node     (xmlNodePtr              markup_node,
					     glTemplateLabelType    *label_type);
static void  xml_parse_markup_circle_node   (xmlNodePtr              markup_node,
					     glTemplateLabelType    *label_type);
static void  xml_parse_alias_node           (xmlNodePtr              alias_node,
					     glTemplate             *template);

static void  xml_create_label_node          (const glTemplateLabelType    *label_type,
					     xmlNodePtr                    root,
					     const xmlNsPtr                ns);
static void  xml_create_layout_node         (const glTemplateLayout       *layout,
					     xmlNodePtr                    root,
					     const xmlNsPtr                ns);
static void  xml_create_markup_margin_node  (const glTemplateMarkup       *margin,
					     xmlNodePtr                    root,
					     const xmlNsPtr                ns);
static void  xml_create_markup_line_node    (const glTemplateMarkup       *line,
					     xmlNodePtr                    root,
					     const xmlNsPtr                ns);
static void  xml_create_markup_circle_node  (const glTemplateMarkup       *circle,
					     xmlNodePtr                    root,
					     const xmlNsPtr                ns);
static void  xml_create_alias_node          (const gchar                  *name,
					     xmlNodePtr                    root,
					     const xmlNsPtr                ns);


/**
 * gl_xml_template_read_templates_from_file:
 * @utf8_filename:       Filename of papers file (name encoded as UTF-8)
 *
 * Read glabels templates from template file.
 *
 * Returns: a list of #glTemplate structures.
 *
 */
GList *
gl_xml_template_read_templates_from_file (const gchar *utf8_filename)
{
	gchar      *filename;
	xmlDocPtr   templates_doc;
	GList      *templates = NULL;

	LIBXML_TEST_VERSION;

	filename = g_filename_from_utf8 (utf8_filename, -1, NULL, NULL, NULL);
	if (!filename) {
		g_warning ("Utf8 filename conversion error");
		return NULL;
	}

	templates_doc = xmlParseFile (filename);
	if (!templates_doc) {
		g_warning ("\"%s\" is not a glabels template file (not XML)",
		      filename);
		return templates;
	}

	templates = gl_xml_template_parse_templates_doc (templates_doc);

	g_free (filename);
	xmlFreeDoc (templates_doc);

	return templates;
}


/**
 * gl_xml_template_parse_templates_doc:
 * @templates_doc:  libxml #xmlDocPtr tree, representing template file.
 *
 * Read glabels templates from a libxml #xmlDocPtr tree.
 *
 * Returns: a list of #glTemplate structures.
 *
 */
GList *
gl_xml_template_parse_templates_doc (const xmlDocPtr templates_doc)
{
	
	GList      *templates = NULL;
	xmlNodePtr  root, node;
	glTemplate *template;

	LIBXML_TEST_VERSION;

	root = xmlDocGetRootElement (templates_doc);
	if (!root || !root->name) {
		g_warning ("\"%s\" is not a glabels template file (no root node)",
			   templates_doc->URL);
		return templates;
	}
	if (!gl_xml_is_node (root, "Glabels-templates")) {
		g_warning ("\"%s\" is not a glabels template file (wrong root node)",
		      templates_doc->URL);
		return templates;
	}

	for (node = root->xmlChildrenNode; node != NULL; node = node->next) {

		if (gl_xml_is_node (node, "Template")) {
			template = gl_xml_template_parse_template_node (node);
			templates = g_list_append (templates, template);
		} else {
			if ( !xmlNodeIsText(node) ) {
				if (!gl_xml_is_node (node,"comment")) {
					g_warning ("bad node =  \"%s\"",node->name);
				}
			}
		}
	}

	return templates;
}


/**
 * gl_xml_template_parse_template_node:
 * @template_node:  libxml #xmlNodePtr template node from a #xmlDocPtr tree.
 *
 * Read a single glabels template from a libxml #xmlNodePtr node.
 *
 * Returns: a pointer to a newly created #glTemplate structure.
 *
 */
glTemplate *
gl_xml_template_parse_template_node (const xmlNodePtr template_node)
{
	gchar                 *name;
	gchar                 *description;
	gchar                 *page_size;
	gdouble                page_width, page_height;
	glPaper               *paper = NULL;
	glTemplate            *template;
	xmlNodePtr             node;

	name = gl_xml_get_prop_string (template_node, "name", NULL);
	description = gl_xml_get_prop_i18n_string (template_node, "description", NULL);
	page_size = gl_xml_get_prop_string (template_node, "size", NULL);

	if (gl_paper_is_id_other (page_size)) {

		page_width = gl_xml_get_prop_length (template_node, "width", 0);
		page_height = gl_xml_get_prop_length (template_node, "height", 0);

	} else {
		paper = gl_paper_from_id (page_size);
		if (paper == NULL) {
			/* This should always be an id, but just in case a name
			   slips by! */
			g_warning (_("Unknown page size id \"%s\", trying as name"),
				   page_size);
			paper = gl_paper_from_name (page_size);
			g_free (page_size);
			page_size = g_strdup (paper->id);
		}
		if (paper != NULL) {
			page_width  = paper->width;
			page_height = paper->height;
		} else {
			g_warning (_("Unknown page size id or name \"%s\""),
				   page_size);
		}
		gl_paper_free (paper);
		paper = NULL;
	}

	template = gl_template_new (name,
				    description,
				    page_size,
				    page_width, page_height);

	for (node = template_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (gl_xml_is_node (node, "Label-rectangle")) {
			xml_parse_label_rectangle_node (node, template);
		} else if (gl_xml_is_node (node, "Label-round")) {
			xml_parse_label_round_node (node, template);
		} else if (gl_xml_is_node (node, "Label-cd")) {
			xml_parse_label_cd_node (node, template);
		} else if (gl_xml_is_node (node, "Alias")) {
			xml_parse_alias_node (node, template);
		} else {
			if (!xmlNodeIsText (node)) {
				if (!gl_xml_is_node (node,"comment")) {
					g_warning ("bad node =  \"%s\"",node->name);
				}
			}
		}
	}

	g_free (name);
	g_free (description);
	g_free (page_size);

	return template;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label-rectangle Node.                      */
/*--------------------------------------------------------------------------*/
static void
xml_parse_label_rectangle_node (xmlNodePtr  label_node,
				glTemplate *template)
{
	gchar               *id;
	gchar               *tmp;
	gdouble              x_waste, y_waste;
	gdouble              w, h, r;
	glTemplateLabelType *label_type;
	xmlNodePtr           node;

	id      = gl_xml_get_prop_string (label_node, "id", NULL);

	if (tmp = gl_xml_get_prop_string (label_node, "waste", NULL)) {
		/* Handle single "waste" property. */
		x_waste = y_waste = gl_xml_get_prop_length (label_node, "waste", 0);
		g_free (tmp);
	} else {
		x_waste = gl_xml_get_prop_length (label_node, "x_waste", 0);
		y_waste = gl_xml_get_prop_length (label_node, "y_waste", 0);
	}

	w       = gl_xml_get_prop_length (label_node, "width", 0);
	h       = gl_xml_get_prop_length (label_node, "height", 0);
	r       = gl_xml_get_prop_length (label_node, "round", 0);

	label_type = gl_template_rect_label_type_new ((gchar *)id, w, h, r, x_waste, y_waste);
	gl_template_add_label_type (template, label_type);

	for (node = label_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (gl_xml_is_node (node, "Layout")) {
			xml_parse_layout_node (node, label_type);
		} else if (gl_xml_is_node (node, "Markup-margin")) {
			xml_parse_markup_margin_node (node, label_type);
		} else if (gl_xml_is_node (node, "Markup-line")) {
			xml_parse_markup_line_node (node, label_type);
		} else if (gl_xml_is_node (node, "Markup-circle")) {
			xml_parse_markup_circle_node (node, label_type);
		} else if (!xmlNodeIsText (node)) {
			if (!gl_xml_is_node (node, "comment")) {
				g_warning ("bad node =  \"%s\"",node->name);
			}
		}
	}

	g_free (id);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label-round Node.                          */
/*--------------------------------------------------------------------------*/
static void
xml_parse_label_round_node (xmlNodePtr  label_node,
			    glTemplate *template)
{
	gchar               *id;
	gdouble              waste;
	gdouble              r;
	glTemplateLabelType *label_type;
	xmlNodePtr           node;

	id    = gl_xml_get_prop_string (label_node, "id", NULL);
	waste = gl_xml_get_prop_length (label_node, "waste", 0);
	r     = gl_xml_get_prop_length (label_node, "radius", 0);

	label_type = gl_template_round_label_type_new ((gchar *)id, r, waste);
	gl_template_add_label_type (template, label_type);

	for (node = label_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (gl_xml_is_node (node, "Layout")) {
			xml_parse_layout_node (node, label_type);
		} else if (gl_xml_is_node (node, "Markup-margin")) {
			xml_parse_markup_margin_node (node, label_type);
		} else if (gl_xml_is_node (node, "Markup-line")) {
			xml_parse_markup_line_node (node, label_type);
		} else if (gl_xml_is_node (node, "Markup-circle")) {
			xml_parse_markup_circle_node (node, label_type);
		} else if (!xmlNodeIsText (node)) {
			if (!gl_xml_is_node (node, "comment")) {
				g_warning ("bad node =  \"%s\"",node->name);
			}
		}
	}

	g_free (id);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label-cd Node.                             */
/*--------------------------------------------------------------------------*/
static void
xml_parse_label_cd_node (xmlNodePtr  label_node,
			 glTemplate *template)
{
	gchar               *id;
	gdouble              waste;
	gdouble              r1, r2, w, h;
	glTemplateLabelType *label_type;
	xmlNodePtr           node;

	id    = gl_xml_get_prop_string (label_node, "id", NULL);
	waste = gl_xml_get_prop_length (label_node, "waste", 0);
	r1    = gl_xml_get_prop_length (label_node, "radius", 0);
	r2    = gl_xml_get_prop_length (label_node, "hole", 0);
	w     = gl_xml_get_prop_length (label_node, "width", 0);
	h     = gl_xml_get_prop_length (label_node, "height", 0);

	label_type = gl_template_cd_label_type_new ((gchar *)id, r1, r2, w, h, waste);
	gl_template_add_label_type (template, label_type);

	for (node = label_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (gl_xml_is_node (node, "Layout")) {
			xml_parse_layout_node (node, label_type);
		} else if (gl_xml_is_node (node, "Markup-margin")) {
			xml_parse_markup_margin_node (node, label_type);
		} else if (gl_xml_is_node (node, "Markup-line")) {
			xml_parse_markup_line_node (node, label_type);
		} else if (gl_xml_is_node (node, "Markup-circle")) {
			xml_parse_markup_circle_node (node, label_type);
		} else if (!xmlNodeIsText (node)) {
			if (!gl_xml_is_node (node, "comment")) {
				g_warning ("bad node =  \"%s\"",node->name);
			}
		}
	}

	g_free (id);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label->Layout Node.                        */
/*--------------------------------------------------------------------------*/
static void
xml_parse_layout_node (xmlNodePtr              layout_node,
		       glTemplateLabelType    *label_type)
{
	gint        nx, ny;
	gdouble     x0, y0, dx, dy;
	xmlNodePtr  node;

	nx = gl_xml_get_prop_int (layout_node, "nx", 1);
	ny = gl_xml_get_prop_int (layout_node, "ny", 1);

	x0 = gl_xml_get_prop_length (layout_node, "x0", 0);
	y0 = gl_xml_get_prop_length (layout_node, "y0", 0);

	dx = gl_xml_get_prop_length (layout_node, "dx", 0);
	dy = gl_xml_get_prop_length (layout_node, "dy", 0);

	gl_template_add_layout (label_type,
				gl_template_layout_new (nx, ny, x0, y0, dx, dy));

	for (node = layout_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (!xmlNodeIsText (node)) {
			if (!gl_xml_is_node (node, "comment")) {
				g_warning ("bad node =  \"%s\"",node->name);
			}
		}
	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label->Markup-margin Node.                 */
/*--------------------------------------------------------------------------*/
static void
xml_parse_markup_margin_node (xmlNodePtr              markup_node,
			      glTemplateLabelType    *label_type)
{
	gdouble     size;
	xmlNodePtr  node;

	size = gl_xml_get_prop_length (markup_node, "size", 0);

	gl_template_add_markup (label_type,
				gl_template_markup_margin_new (size));

	for (node = markup_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (!xmlNodeIsText (node)) {
			if (!gl_xml_is_node (node, "comment")) {
				g_warning ("bad node =  \"%s\"",node->name);
			}
		}
	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label->Markup-line Node.                   */
/*--------------------------------------------------------------------------*/
static void
xml_parse_markup_line_node (xmlNodePtr              markup_node,
			    glTemplateLabelType    *label_type)
{
	gdouble     x1, y1, x2, y2;
	xmlNodePtr  node;

	x1 = gl_xml_get_prop_length (markup_node, "x1", 0);
	y1 = gl_xml_get_prop_length (markup_node, "y1", 0);
	x2 = gl_xml_get_prop_length (markup_node, "x2", 0);
	y2 = gl_xml_get_prop_length (markup_node, "y2", 0);

	gl_template_add_markup (label_type,
				gl_template_markup_line_new (x1, y1, x2, y2));

	for (node = markup_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (!xmlNodeIsText (node)) {
			if (!gl_xml_is_node (node, "comment")) {
				g_warning ("bad node =  \"%s\"",node->name);
			}
		}
	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label->Markup-circle Node.                 */
/*--------------------------------------------------------------------------*/
static void
xml_parse_markup_circle_node (xmlNodePtr              markup_node,
			      glTemplateLabelType    *label_type)
{
	gdouble     x0, y0, r;
	xmlNodePtr  node;

	x0 = gl_xml_get_prop_length (markup_node, "x0", 0);
	y0 = gl_xml_get_prop_length (markup_node, "y0", 0);
	r  = gl_xml_get_prop_length (markup_node, "radius", 0);

	gl_template_add_markup (label_type,
				gl_template_markup_circle_new (x0, y0, r));

	for (node = markup_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (!xmlNodeIsText (node)) {
			if (!gl_xml_is_node (node, "comment")) {
				g_warning ("bad node =  \"%s\"",node->name);
			}
		}
	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Alias Node.                                */
/*--------------------------------------------------------------------------*/
static void
xml_parse_alias_node (xmlNodePtr  alias_node,
		      glTemplate *template)
{
	gchar       *name;

	name = gl_xml_get_prop_string (alias_node, "name", NULL);

	gl_template_add_alias (template, (gchar *)name);

	g_free (name);
}

/**
 * gl_xml_template_write_templates_to_file:
 * @templates:      List of #glTemplate structures
 * @utf8_filename:  Filename of templates file (name encoded as UTF-8)
 *
 * Write a list of #glTemplate structures to a glabels XML template file.
 *
 */
void
gl_xml_template_write_templates_to_file (GList       *templates,
					 const gchar *utf8_filename)
{
	xmlDocPtr    doc;
	xmlNsPtr     ns;
	gint         xml_ret;
	GList       *p;
	glTemplate  *template;
	gchar       *filename;

	doc = xmlNewDoc ((xmlChar *)"1.0");
	doc->xmlRootNode = xmlNewDocNode (doc, NULL, (xmlChar *)"Glabels-templates", NULL);

	ns = xmlNewNs (doc->xmlRootNode, (xmlChar *)GL_XML_NAME_SPACE, NULL);
	xmlSetNs (doc->xmlRootNode, ns);

	for (p=templates; p!=NULL; p=p->next) {
		template = (glTemplate *)p->data;
		gl_xml_template_create_template_node (template, doc->xmlRootNode, ns);
	}

	filename = g_filename_from_utf8 (utf8_filename, -1, NULL, NULL, NULL);
	if (!filename)
		g_warning (_("Utf8 conversion error."));
	else {
		xmlSetDocCompressMode (doc, 0);
		xml_ret = xmlSaveFormatFile (filename, doc, TRUE);
		xmlFreeDoc (doc);
		if (xml_ret == -1) {

			g_warning (_("Problem saving xml file."));

		}
		g_free (filename);
	}

}


/**
 * gl_xml_template_write_template_to_file:
 * @template:       #glTemplate structure to be written
 * @utf8_filename:  Filename of templates file (name encoded as UTF-8)
 *
 * Write a single #glTemplate structures to a glabels XML template file.
 *
 */
void
gl_xml_template_write_template_to_file (const glTemplate  *template,
					const gchar       *utf8_filename)
{
	GList     *templates = NULL;

	templates = g_list_append (templates, (gpointer)template);

	gl_xml_template_write_templates_to_file (templates, utf8_filename);

	g_list_free (templates);
}


/**
 * gl_xml_template_create_template_node:
 * @template:       #glTemplate structure to be written
 * @root:           parent node to receive new child node
 * @ns:             a libxml #xmlNsPtr
 *
 * Add a single #glTemplate child node to given #xmlNodePtr.
 *
 */
void
gl_xml_template_create_template_node (const glTemplate *template,
				      xmlNodePtr        root,
				      const xmlNsPtr    ns)
{
	xmlNodePtr              node;
	GList                  *p;
	glTemplateLabelType    *label_type;

	node = xmlNewChild (root, ns, (xmlChar *)"Template", NULL);

	gl_xml_set_prop_string (node, "name", template->name);

	gl_xml_set_prop_string (node, "size", template->page_size);
	if (xmlStrEqual ((xmlChar *)template->page_size, (xmlChar *)"Other")) {

		gl_xml_set_prop_length (node, "width", template->page_width);
		gl_xml_set_prop_length (node, "height", template->page_height);

	}

	gl_xml_set_prop_string (node, "description", template->description);

	for ( p=template->label_types; p != NULL; p=p->next ) {
		label_type = (glTemplateLabelType *)p->data;
		xml_create_label_node (label_type, node, ns);
	}

	for ( p=template->aliases; p != NULL; p=p->next ) {
		if (!xmlStrEqual ((xmlChar *)template->name, (xmlChar *)p->data)) {
			xml_create_alias_node ( p->data, node, ns );
		}
	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Template->Label Node.                                  */
/*--------------------------------------------------------------------------*/
static void
xml_create_label_node (const glTemplateLabelType  *label_type,
		       xmlNodePtr                  root,
		       const xmlNsPtr              ns)
{
	xmlNodePtr        node;
	GList            *p;
	glTemplateMarkup *markup;
	glTemplateLayout *layout;

	switch (label_type->shape) {

	case GL_TEMPLATE_SHAPE_RECT:
		node = xmlNewChild(root, ns, (xmlChar *)"Label-rectangle", NULL);
		gl_xml_set_prop_string (node, "id",      label_type->id);
		gl_xml_set_prop_length (node, "width",   label_type->size.rect.w);
		gl_xml_set_prop_length (node, "height",  label_type->size.rect.h);
		gl_xml_set_prop_length (node, "round",   label_type->size.rect.r);
		gl_xml_set_prop_length (node, "x_waste", label_type->size.rect.x_waste);
		gl_xml_set_prop_length (node, "y_waste", label_type->size.rect.y_waste);
		break;

	case GL_TEMPLATE_SHAPE_ROUND:
		node = xmlNewChild(root, ns, (xmlChar *)"Label-round", NULL);
		gl_xml_set_prop_string (node, "id",      label_type->id);
		gl_xml_set_prop_length (node, "radius",  label_type->size.round.r);
		gl_xml_set_prop_length (node, "waste",   label_type->size.round.waste);
		break;

	case GL_TEMPLATE_SHAPE_CD:
		node = xmlNewChild(root, ns, (xmlChar *)"Label-cd", NULL);
		gl_xml_set_prop_string (node, "id",     label_type->id);
		gl_xml_set_prop_length (node, "radius", label_type->size.cd.r1);
		gl_xml_set_prop_length (node, "hole",   label_type->size.cd.r2);
		if (label_type->size.cd.w != 0.0) {
			gl_xml_set_prop_length (node, "width",  label_type->size.cd.w);
		}
		if (label_type->size.cd.h != 0.0) {
			gl_xml_set_prop_length (node, "height", label_type->size.cd.h);
		}
		gl_xml_set_prop_length (node, "waste",  label_type->size.cd.waste);
		break;

	default:
		g_warning ("Unknown label style");
		return;
		break;

	}

	for ( p=label_type->markups; p != NULL; p=p->next ) {
		markup = (glTemplateMarkup *)p->data;
		switch (markup->type) {
		case GL_TEMPLATE_MARKUP_MARGIN:
			xml_create_markup_margin_node (markup, node, ns);
			break;
		case GL_TEMPLATE_MARKUP_LINE:
			xml_create_markup_line_node (markup, node, ns);
			break;
		case GL_TEMPLATE_MARKUP_CIRCLE:
			xml_create_markup_circle_node (markup, node, ns);
			break;
		default:
			g_warning ("Unknown markup type");
			break;
		}
	}

	for ( p=label_type->layouts; p != NULL; p=p->next ) {
		layout = (glTemplateLayout *)p->data;
		xml_create_layout_node (layout, node, ns);
	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Template->Label->Layout Node.                          */
/*--------------------------------------------------------------------------*/
static void
xml_create_layout_node (const glTemplateLayout *layout,
			xmlNodePtr              root,
			const xmlNsPtr          ns)
{
	xmlNodePtr  node;

	node = xmlNewChild(root, ns, (xmlChar *)"Layout", NULL);
	gl_xml_set_prop_int (node, "nx", layout->nx);
	gl_xml_set_prop_int (node, "ny", layout->ny);
	gl_xml_set_prop_length (node, "x0", layout->x0);
	gl_xml_set_prop_length (node, "y0", layout->y0);
	gl_xml_set_prop_length (node, "dx", layout->dx);
	gl_xml_set_prop_length (node, "dy", layout->dy);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Template->Label->Markup-margin Node.                   */
/*--------------------------------------------------------------------------*/
static void
xml_create_markup_margin_node (const glTemplateMarkup  *markup,
			       xmlNodePtr               root,
			       const xmlNsPtr           ns)
{
	xmlNodePtr  node;

	node = xmlNewChild(root, ns, (xmlChar *)"Markup-margin", NULL);

	gl_xml_set_prop_length (node, "size", markup->data.margin.size);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Template->Label->Markup-line Node.                     */
/*--------------------------------------------------------------------------*/
static void
xml_create_markup_line_node (const glTemplateMarkup *markup,
			     xmlNodePtr              root,
			     const xmlNsPtr          ns)
{
	xmlNodePtr  node;

	node = xmlNewChild(root, ns, (xmlChar *)"Markup-line", NULL);

	gl_xml_set_prop_length (node, "x1", markup->data.line.x1);
	gl_xml_set_prop_length (node, "y1", markup->data.line.y1);
	gl_xml_set_prop_length (node, "x2", markup->data.line.x2);
	gl_xml_set_prop_length (node, "y2", markup->data.line.y2);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Template->Label->Markup-circle Node.                   */
/*--------------------------------------------------------------------------*/
static void
xml_create_markup_circle_node (const glTemplateMarkup *markup,
			       xmlNodePtr              root,
			       const xmlNsPtr          ns)
{
	xmlNodePtr  node;

	node = xmlNewChild(root, ns, (xmlChar *)"Markup-circle", NULL);

	gl_xml_set_prop_length (node, "x0",     markup->data.circle.x0);
	gl_xml_set_prop_length (node, "y0",     markup->data.circle.y0);
	gl_xml_set_prop_length (node, "radius", markup->data.circle.r);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Template->Alias Node.                                  */
/*--------------------------------------------------------------------------*/
static void
xml_create_alias_node (const gchar      *name,
		       xmlNodePtr        root,
		       const xmlNsPtr    ns)
{
	xmlNodePtr node;

	node = xmlNewChild (root, ns, (xmlChar *)"Alias", NULL);
	gl_xml_set_prop_string (node, "name", name);

}

