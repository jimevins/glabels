/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  xml-template.c:  template xml module
 *
 *  Copyright (C) 2001-2003  Jim Evins <evins@snaught.com>.
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

#include <string.h>
#include <libintl.h>

#include "util.h"
#include "paper.h"
#include "xml.h"
#include "xml-template.h"

#include "debug.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/
static void        xml_parse_label_rectangle    (xmlNodePtr              layout_node,
						 glTemplate             *template);
static void        xml_parse_label_round        (xmlNodePtr              layout_node,
						 glTemplate             *template);
static void        xml_parse_label_cd           (xmlNodePtr              layout_node,
						 glTemplate             *template);
static void        xml_parse_layout             (xmlNodePtr              label_node,
						 glTemplate             *template);
static void        xml_parse_markup_margin      (xmlNodePtr              markup_node,
						 glTemplate             *template);
static void        xml_parse_markup_line        (xmlNodePtr              markup_node,
						 glTemplate             *template);
static void        xml_parse_markup_circle      (xmlNodePtr              markup_node,
						 glTemplate             *template);
static void        xml_parse_alias              (xmlNodePtr              alias_node,
						 glTemplate             *template);

static void        xml_add_label                (const glTemplate       *template,
						 xmlNodePtr              root,
						 xmlNsPtr                ns);
static void        xml_add_layout               (glTemplateLayout       *layout,
						 xmlNodePtr              root,
						 xmlNsPtr                ns);
static void        xml_add_markup_margin        (glTemplateMarkupMargin *margin,
						 xmlNodePtr              root,
						 xmlNsPtr                ns);
static void        xml_add_markup_line          (glTemplateMarkupLine   *line,
						 xmlNodePtr              root,
						 xmlNsPtr                ns);
static void        xml_add_markup_circle        (glTemplateMarkupCircle *circle,
						 xmlNodePtr              root,
						 xmlNsPtr                ns);
static void        xml_add_alias                (gchar                  *name,
						 xmlNodePtr              root,
						 xmlNsPtr                ns);


/*****************************************************************************/
/* Read templates from template file.                                        */
/*****************************************************************************/
GList *
gl_xml_template_read_templates_from_file (GList *templates,
					  gchar *xml_filename)
{
	xmlDocPtr   doc;
	xmlNodePtr  root, node;
	glTemplate *template;

	gl_debug (DEBUG_TEMPLATE, "START");

	LIBXML_TEST_VERSION;

	doc = xmlParseFile (xml_filename);
	if (!doc) {
		g_warning ("\"%s\" is not a glabels template file (not XML)",
		      xml_filename);
		return templates;
	}

	root = xmlDocGetRootElement (doc);
	if (!root || !root->name) {
		g_warning ("\"%s\" is not a glabels template file (no root node)",
		      xml_filename);
		xmlFreeDoc (doc);
		return templates;
	}
	if (!xmlStrEqual (root->name, "Glabels-templates")) {
		g_warning ("\"%s\" is not a glabels template file (wrong root node)",
		      xml_filename);
		xmlFreeDoc (doc);
		return templates;
	}

	for (node = root->xmlChildrenNode; node != NULL; node = node->next) {

		if (xmlStrEqual (node->name, "Template")) {
			template = gl_xml_template_parse_template (node);
			templates = g_list_append (templates, template);
		} else {
			if ( !xmlNodeIsText(node) ) {
				if (!xmlStrEqual (node->name,"comment")) {
					g_warning ("bad node =  \"%s\"",node->name);
				}
			}
		}
	}

	xmlFreeDoc (doc);

	gl_debug (DEBUG_TEMPLATE, "END");
	return templates;
}

/*****************************************************************************/
/* Parse XML template Node.                                                  */
/*****************************************************************************/
glTemplate *
gl_xml_template_parse_template (xmlNodePtr template_node)
{
	glTemplate            *template;
	xmlNodePtr             node;
	gchar                 *description;
	glPaper               *paper;

	gl_debug (DEBUG_TEMPLATE, "START");

	template = g_new0 (glTemplate, 1);

	template->name  = xmlGetProp (template_node, "name");
	template->alias = g_list_append (template->alias, g_strdup (template->name));
	gl_debug (DEBUG_TEMPLATE, "Template = %s", template->name);

	template->page_size = xmlGetProp (template_node, "size");
	if (xmlStrEqual (template->page_size, "Other")) {

		template->page_width = gl_xml_get_prop_length (template_node, "width", 0);
		template->page_height = gl_xml_get_prop_length (template_node, "height", 0);

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
			template->page_width  = paper->width;
			template->page_height = paper->height;
		} else {
			g_warning (_("Unknown page size id or name \"%s\""),
				   template->page_size);
		}
		gl_paper_free (&paper);
	}

	description = xmlGetProp (template_node, "_description");
	if (description != NULL) {
		template->description = gettext (description);
	} else {
		template->description = xmlGetProp (template_node, "description");
	}

	for (node = template_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (xmlStrEqual (node->name, "Label-rectangle")) {
			xml_parse_label_rectangle (node, template);
		} else if (xmlStrEqual (node->name, "Label-round")) {
			xml_parse_label_round (node, template);
		} else if (xmlStrEqual (node->name, "Label-cd")) {
			xml_parse_label_cd (node, template);
		} else if (xmlStrEqual (node->name, "Alias")) {
			xml_parse_alias (node, template);
		} else {
			if (!xmlNodeIsText (node)) {
				g_warning ("bad node =  \"%s\"", node->name);
			}
		}
	}

	gl_debug (DEBUG_TEMPLATE, "END");

	return template;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label-rectangle Node.                      */
/*--------------------------------------------------------------------------*/
static void
xml_parse_label_rectangle (xmlNodePtr  label_node,
			   glTemplate *template)
{
	xmlNodePtr  node;

	gl_debug (DEBUG_TEMPLATE, "START");

	template->label.style = GL_TEMPLATE_STYLE_RECT;

	template->label.rect.w = gl_xml_get_prop_length (label_node, "width", 0);
	template->label.rect.h = gl_xml_get_prop_length (label_node, "height", 0);
	template->label.rect.r = gl_xml_get_prop_length (label_node, "round", 0);

	template->label.rect.waste = gl_xml_get_prop_length (label_node, "waste", 0);

	for (node = label_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (xmlStrEqual (node->name, "Layout")) {
			xml_parse_layout (node, template);
		} else if (xmlStrEqual (node->name, "Markup-margin")) {
			xml_parse_markup_margin (node, template);
		} else if (xmlStrEqual (node->name, "Markup-line")) {
			xml_parse_markup_line (node, template);
		} else if (xmlStrEqual (node->name, "Markup-circle")) {
			xml_parse_markup_circle (node, template);
		} else if (!xmlNodeIsText (node)) {
			g_warning ("bad node =  \"%s\"", node->name);
		}
	}

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label-round Node.                          */
/*--------------------------------------------------------------------------*/
static void
xml_parse_label_round (xmlNodePtr  label_node,
		       glTemplate *template)
{
	xmlNodePtr  node;

	gl_debug (DEBUG_TEMPLATE, "START");

	template->label.style = GL_TEMPLATE_STYLE_ROUND;

	template->label.round.r = gl_xml_get_prop_length (label_node, "radius", 0);

	template->label.round.waste = gl_xml_get_prop_length (label_node, "waste", 0);

	for (node = label_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (xmlStrEqual (node->name, "Layout")) {
			xml_parse_layout (node, template);
		} else if (xmlStrEqual (node->name, "Markup-margin")) {
			xml_parse_markup_margin (node, template);
		} else if (xmlStrEqual (node->name, "Markup-line")) {
			xml_parse_markup_line (node, template);
		} else if (!xmlNodeIsText (node)) {
			g_warning ("bad node =  \"%s\"", node->name);
		}
	}

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label-cd Node.                             */
/*--------------------------------------------------------------------------*/
static void
xml_parse_label_cd (xmlNodePtr  label_node,
		    glTemplate *template)
{
	xmlNodePtr  node;

	gl_debug (DEBUG_TEMPLATE, "START");

	template->label.style = GL_TEMPLATE_STYLE_CD;

	template->label.cd.r1 = gl_xml_get_prop_length (label_node, "radius", 0);
	template->label.cd.r2 = gl_xml_get_prop_length (label_node, "hole", 0);
	template->label.cd.w  = gl_xml_get_prop_length (label_node, "width", 0);
	template->label.cd.h  = gl_xml_get_prop_length (label_node, "height", 0);

	template->label.cd.waste = gl_xml_get_prop_length (label_node, "waste", 0);

	for (node = label_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (xmlStrEqual (node->name, "Layout")) {
			xml_parse_layout (node, template);
		} else if (xmlStrEqual (node->name, "Markup-margin")) {
			xml_parse_markup_margin (node, template);
		} else if (xmlStrEqual (node->name, "Markup-line")) {
			xml_parse_markup_line (node, template);
		} else if (!xmlNodeIsText (node)) {
			g_warning ("bad node =  \"%s\"", node->name);
		}
	}

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label->Layout Node.`                        */
/*--------------------------------------------------------------------------*/
static void
xml_parse_layout (xmlNodePtr  layout_node,
		  glTemplate *template)
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

	template->label.any.layouts =
		g_list_append (template->label.any.layouts,
			       gl_template_layout_new (nx, ny, x0, y0, dx, dy));

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label->Markup-margin Node.                 */
/*--------------------------------------------------------------------------*/
static void
xml_parse_markup_margin (xmlNodePtr  markup_node,
			 glTemplate *template)
{
	gdouble     size;
	xmlNodePtr  node;

	gl_debug (DEBUG_TEMPLATE, "START");

	size = gl_xml_get_prop_length (markup_node, "size", 0);

	template->label.any.markups =
		g_list_append (template->label.any.markups,
			       gl_template_markup_margin_new (size));

	for (node = markup_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (!xmlNodeIsText (node)) {
			g_warning ("bad node =  \"%s\"", node->name);
		}
	}

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label->Markup-line Node.                   */
/*--------------------------------------------------------------------------*/
static void
xml_parse_markup_line (xmlNodePtr  markup_node,
		       glTemplate *template)
{
	gdouble     x1, y1, x2, y2;
	xmlNodePtr  node;

	gl_debug (DEBUG_TEMPLATE, "START");

	x1 = gl_xml_get_prop_length (markup_node, "x1", 0);
	y1 = gl_xml_get_prop_length (markup_node, "y1", 0);
	x2 = gl_xml_get_prop_length (markup_node, "x2", 0);
	y2 = gl_xml_get_prop_length (markup_node, "y2", 0);

	template->label.any.markups =
		g_list_append (template->label.any.markups,
			       gl_template_markup_line_new (x1, y1, x2, y2));

	for (node = markup_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (!xmlNodeIsText (node)) {
			g_warning ("bad node =  \"%s\"", node->name);
		}
	}

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Label->Markup-circle Node.                 */
/*--------------------------------------------------------------------------*/
static void
xml_parse_markup_circle (xmlNodePtr  markup_node,
		       glTemplate *template)
{
	gdouble     x0, y0, r;
	xmlNodePtr  node;

	gl_debug (DEBUG_TEMPLATE, "START");

	x0 = gl_xml_get_prop_length (markup_node, "x0", 0);
	y0 = gl_xml_get_prop_length (markup_node, "y0", 0);
	r  = gl_xml_get_prop_length (markup_node, "radius", 0);

	template->label.any.markups =
		g_list_append (template->label.any.markups,
			       gl_template_markup_circle_new (x0, y0, r));

	for (node = markup_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (!xmlNodeIsText (node)) {
			g_warning ("bad node =  \"%s\"", node->name);
		}
	}

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Template->Alias Node.                                */
/*--------------------------------------------------------------------------*/
static void
xml_parse_alias (xmlNodePtr  alias_node,
		 glTemplate *template)
{
	gl_debug (DEBUG_TEMPLATE, "START");

	template->alias = g_list_append (template->alias,
					xmlGetProp (alias_node, "name"));

	gl_debug (DEBUG_TEMPLATE, "END");
}

/****************************************************************************/
/* Add XML Template Node                                                    */
/****************************************************************************/
void
gl_xml_template_add_template (const glTemplate *template,
			      xmlNodePtr        root,
			      xmlNsPtr          ns)
{
	xmlNodePtr  node;
	GList      *p;

	gl_debug (DEBUG_TEMPLATE, "START");

	node = xmlNewChild (root, ns, "Template", NULL);

	xmlSetProp (node, "name", template->name);

	xmlSetProp (node, "size", template->page_size);
	if (xmlStrEqual (template->page_size, "Other")) {

		gl_xml_set_prop_length (node, "width", template->page_width);
		gl_xml_set_prop_length (node, "height", template->page_height);

	}

	xmlSetProp (node, "description", template->description);

	xml_add_label (template, node, ns);

	for ( p=template->alias; p != NULL; p=p->next ) {
		if (!xmlStrEqual (template->name, p->data)) {
			xml_add_alias( p->data, node, ns );
		}
	}

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Template->Label Node.                                     */
/*--------------------------------------------------------------------------*/
static void
xml_add_label (const glTemplate *template,
	       xmlNodePtr        root,
	       xmlNsPtr          ns)
{
	xmlNodePtr        node;
	GList            *p;
	glTemplateMarkup *markup;
	glTemplateLayout *layout;

	gl_debug (DEBUG_TEMPLATE, "START");

	switch (template->label.style) {

	case GL_TEMPLATE_STYLE_RECT:
		node = xmlNewChild(root, ns, "Label-rectangle", NULL);
		xmlSetProp (node, "id", "0");
		gl_xml_set_prop_length (node, "width",  template->label.rect.w);
		gl_xml_set_prop_length (node, "height", template->label.rect.h);
		gl_xml_set_prop_length (node, "round",  template->label.rect.r);
		gl_xml_set_prop_length (node, "waste",  template->label.rect.waste);
		break;

	case GL_TEMPLATE_STYLE_ROUND:
		node = xmlNewChild(root, ns, "Label-round", NULL);
		xmlSetProp (node, "id", "0");
		gl_xml_set_prop_length (node, "radius",  template->label.round.r);
		gl_xml_set_prop_length (node, "waste",   template->label.round.waste);
		break;

	case GL_TEMPLATE_STYLE_CD:
		node = xmlNewChild(root, ns, "Label-cd", NULL);
		xmlSetProp (node, "id", "0");
		gl_xml_set_prop_length (node, "radius",  template->label.cd.r1);
		gl_xml_set_prop_length (node, "hole",    template->label.cd.r2);
		if (template->label.cd.w != 0.0) {
			gl_xml_set_prop_length (node, "width",  template->label.cd.w);
		}
		if (template->label.cd.h != 0.0) {
			gl_xml_set_prop_length (node, "height", template->label.cd.h);
		}
		gl_xml_set_prop_length (node, "waste",  template->label.cd.waste);
		break;

	default:
		g_warning ("Unknown label style");
		return;
		break;

	}

	for ( p=template->label.any.markups; p != NULL; p=p->next ) {
		markup = (glTemplateMarkup *)p->data;
		switch (markup->type) {
		case GL_TEMPLATE_MARKUP_MARGIN:
			xml_add_markup_margin ((glTemplateMarkupMargin *)markup,
					       node, ns);
			break;
		case GL_TEMPLATE_MARKUP_LINE:
			xml_add_markup_line ((glTemplateMarkupLine *)markup,
					     node, ns);
			break;
		case GL_TEMPLATE_MARKUP_CIRCLE:
			xml_add_markup_circle ((glTemplateMarkupCircle *)markup,
					       node, ns);
			break;
		default:
			g_warning ("Unknown markup type");
			break;
		}
	}

	for ( p=template->label.any.layouts; p != NULL; p=p->next ) {
		layout = (glTemplateLayout *)p->data;
		xml_add_layout (layout, node, ns);
	}

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Template->Label->Layout Node.                             */
/*--------------------------------------------------------------------------*/
static void
xml_add_layout (glTemplateLayout *layout,
		xmlNodePtr        root,
		xmlNsPtr          ns)
{
	xmlNodePtr  node;

	gl_debug (DEBUG_TEMPLATE, "START");

	node = xmlNewChild(root, ns, "Layout", NULL);
	gl_xml_set_prop_int (node, "nx", layout->nx);
	gl_xml_set_prop_int (node, "ny", layout->ny);
	gl_xml_set_prop_length (node, "x0", layout->x0);
	gl_xml_set_prop_length (node, "y0", layout->y0);
	gl_xml_set_prop_length (node, "dx", layout->dx);
	gl_xml_set_prop_length (node, "dy", layout->dy);

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Template->Label->Markup-margin Node.                   */
/*--------------------------------------------------------------------------*/
static void
xml_add_markup_margin (glTemplateMarkupMargin *margin,
		       xmlNodePtr              root,
		       xmlNsPtr                ns)
{
	xmlNodePtr  node;

	gl_debug (DEBUG_TEMPLATE, "START");

	node = xmlNewChild(root, ns, "Markup-margin", NULL);

	gl_xml_set_prop_length (node, "size", margin->size);

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Template->Label->Markup-line Node.                     */
/*--------------------------------------------------------------------------*/
static void
xml_add_markup_line (glTemplateMarkupLine *line,
		     xmlNodePtr            root,
		     xmlNsPtr              ns)
{
	xmlNodePtr  node;

	gl_debug (DEBUG_TEMPLATE, "START");

	node = xmlNewChild(root, ns, "Markup-line", NULL);

	gl_xml_set_prop_length (node, "x1", line->x1);
	gl_xml_set_prop_length (node, "y1", line->y1);
	gl_xml_set_prop_length (node, "x2", line->x2);
	gl_xml_set_prop_length (node, "y2", line->y2);

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Template->Label->Markup-circle Node.                   */
/*--------------------------------------------------------------------------*/
static void
xml_add_markup_circle (glTemplateMarkupCircle *circle,
		       xmlNodePtr            root,
		       xmlNsPtr              ns)
{
	xmlNodePtr  node;

	gl_debug (DEBUG_TEMPLATE, "START");

	node = xmlNewChild(root, ns, "Markup-circle", NULL);

	gl_xml_set_prop_length (node, "x0", circle->x0);
	gl_xml_set_prop_length (node, "y0", circle->y0);
	gl_xml_set_prop_length (node, "radius", circle->r);

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Template->Alias Node.                                     */
/*--------------------------------------------------------------------------*/
static void
xml_add_alias (gchar      *name,
	       xmlNodePtr  root,
	       xmlNsPtr    ns)
{
	xmlNodePtr node;

	gl_debug (DEBUG_TEMPLATE, "START");

	node = xmlNewChild (root, ns, "Alias", NULL);
	xmlSetProp (node, "name", name);

	gl_debug (DEBUG_TEMPLATE, "END");
}

