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
#include <libgnomeprint/gnome-print-paper.h>

#include "util.h"
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
static void        xml_parse_label              (xmlNodePtr              label_node,
						 glTemplate             *template);
static void        xml_parse_layout             (xmlNodePtr              layout_node,
						 glTemplate             *template);
static void        xml_parse_markup             (xmlNodePtr              markup_node,
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
	if (g_strcasecmp (root->name, "glabels-templates") != 0) {
		g_warning ("\"%s\" is not a glabels template file (wrong root node)",
		      xml_filename);
		xmlFreeDoc (doc);
		return templates;
	}

	for (node = root->xmlChildrenNode; node != NULL; node = node->next) {

		if (g_strcasecmp (node->name, "Sheet") == 0) {
			template = gl_xml_template_parse_sheet (node);
			templates = g_list_append (templates, template);
		} else {
			if ( !xmlNodeIsText(node) ) {
				if (g_strcasecmp (node->name,"comment") != 0) {
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
gl_xml_template_parse_sheet (xmlNodePtr sheet_node)
{
	glTemplate            *template;
	xmlNodePtr             node;
	const GnomePrintPaper *paper;

	gl_debug (DEBUG_TEMPLATE, "START");

	template = g_new0 (glTemplate, 1);

	template->name = g_list_append (template->name,
					xmlGetProp (sheet_node, "name"));
	gl_debug (DEBUG_TEMPLATE, "Sheet = %s", template->name->data);

	template->page_size = xmlGetProp (sheet_node, "size");
	if ( strcmp (template->page_size,"US-Letter") == 0 ) {
		template->page_size = "US Letter";
	}
	if (g_strcasecmp (template->page_size, "Other") == 0) {

		template->page_width = gl_xml_get_prop_double (sheet_node, "width", 0);
		template->page_height = gl_xml_get_prop_double (sheet_node, "height", 0);

	} else {
		paper = gnome_print_paper_get_by_name (template->page_size);
		if (paper != NULL) {
			template->page_width  = paper->width;
			template->page_height = paper->height;
		} else {
			g_warning (_("Unknown page size \"%s\""), template->page_size);
		}
	}

	template->description = xmlGetProp (sheet_node, "description");

	for (node = sheet_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (g_strcasecmp (node->name, "Label") == 0) {
			xml_parse_label (node, template);
		} else if (g_strcasecmp (node->name, "Alias") == 0) {
			xml_parse_alias (node, template);
		} else {
			if (g_strcasecmp (node->name, "text") != 0) {
				g_warning ("bad node =  \"%s\"", node->name);
			}
		}
	}

	gl_debug (DEBUG_TEMPLATE, "END");

	return template;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Sheet->Label Node.                                   */
/*--------------------------------------------------------------------------*/
static void
xml_parse_label (xmlNodePtr  label_node,
		 glTemplate *template)
{
	xmlNodePtr  node;
	gchar      *style;

	gl_debug (DEBUG_TEMPLATE, "START");

	style = xmlGetProp (label_node, "style");
	if (g_strcasecmp (style, "rectangle") == 0) {
		template->label.style = GL_TEMPLATE_STYLE_RECT;
	} else if (g_strcasecmp (style, "round") == 0) {
		template->label.style = GL_TEMPLATE_STYLE_ROUND;
	} else if (g_strcasecmp (style, "cd") == 0) {
		template->label.style = GL_TEMPLATE_STYLE_CD;
	} else {
		template->label.style = GL_TEMPLATE_STYLE_RECT;
		g_warning ("Unknown label style in template");
	}
	g_free (style);

	switch (template->label.style) {

	case GL_TEMPLATE_STYLE_RECT:
		template->label.rect.w = gl_xml_get_prop_double (label_node, "width", 0);
		template->label.rect.h = gl_xml_get_prop_double (label_node, "height", 0);
		template->label.rect.r = gl_xml_get_prop_double (label_node, "round", 0);
		break;

	case GL_TEMPLATE_STYLE_ROUND:
		template->label.round.r = gl_xml_get_prop_double (label_node, "radius", 0);
		break;

	case GL_TEMPLATE_STYLE_CD:
		template->label.cd.r1 = gl_xml_get_prop_double (label_node, "radius", 0);
		template->label.cd.r2 = gl_xml_get_prop_double (label_node, "hole", 0);
		template->label.cd.w  = gl_xml_get_prop_double (label_node, "width", 0);
		template->label.cd.h  = gl_xml_get_prop_double (label_node, "height", 0);
		break;

	default:
		break;

	}

	for (node = label_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (g_strcasecmp (node->name, "Layout") == 0) {
			xml_parse_layout (node, template);
		} else if (g_strcasecmp (node->name, "Markup") == 0) {
			xml_parse_markup (node, template);
		} else if (g_strcasecmp (node->name, "text") != 0) {
			g_warning ("bad node =  \"%s\"", node->name);
		}
	}

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Sheet->Label->Layout Node.                           */
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

	x0 = gl_xml_get_prop_double (layout_node, "x0", 0);
	y0 = gl_xml_get_prop_double (layout_node, "y0", 0);

	dx = gl_xml_get_prop_double (layout_node, "dx", 0);
	dy = gl_xml_get_prop_double (layout_node, "dy", 0);

	for (node = layout_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (g_strcasecmp (node->name, "text") != 0) {
			g_warning ("bad node =  \"%s\"", node->name);
		}
	}

	template->label.any.layouts =
		g_list_append (template->label.any.layouts,
			       gl_template_layout_new (nx, ny, x0, y0, dx, dy));

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Sheet->Label->Markup Node.                           */
/*--------------------------------------------------------------------------*/
static void
xml_parse_markup (xmlNodePtr  markup_node,
		  glTemplate *template)
{
	gchar      *type;
	gdouble     size;
	gdouble     x1, y1, x2, y2;
	xmlNodePtr  node;

	gl_debug (DEBUG_TEMPLATE, "START");

	type = xmlGetProp (markup_node, "type");
	if (g_strcasecmp (type, "margin") == 0) {

		size = gl_xml_get_prop_double (markup_node, "size", 0);

		template->label.any.markups =
			g_list_append (template->label.any.markups,
				       gl_template_markup_margin_new (size));

	} else if (g_strcasecmp (type, "line") == 0) {

		x1 = gl_xml_get_prop_double (markup_node, "x1", 0);
		y1 = gl_xml_get_prop_double (markup_node, "y1", 0);
		x2 = gl_xml_get_prop_double (markup_node, "x2", 0);
		y2 = gl_xml_get_prop_double (markup_node, "y2", 0);

		template->label.any.markups =
			g_list_append (template->label.any.markups,
				       gl_template_markup_line_new (x1, y1, x2, y2));
	}
	g_free (type);

	for (node = markup_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (g_strcasecmp (node->name, "text") != 0) {
			g_warning ("bad node =  \"%s\"", node->name);
		}
	}

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Sheet->Alias Node.                                   */
/*--------------------------------------------------------------------------*/
static void
xml_parse_alias (xmlNodePtr  alias_node,
		 glTemplate *template)
{
	gl_debug (DEBUG_TEMPLATE, "START");

	template->name = g_list_append (template->name,
					xmlGetProp (alias_node, "name"));

	gl_debug (DEBUG_TEMPLATE, "END");
}

/****************************************************************************/
/* Add XML Template Node                                                    */
/****************************************************************************/
void
gl_xml_template_add_sheet (const glTemplate *template,
			   xmlNodePtr        root,
			   xmlNsPtr          ns)
{
	xmlNodePtr  node;
	GList      *p;

	gl_debug (DEBUG_TEMPLATE, "START");

	node = xmlNewChild (root, ns, "Sheet", NULL);

	xmlSetProp (node, "name", template->name->data);

	xmlSetProp (node, "size", template->page_size);
	if (g_strcasecmp (template->page_size, "Other") == 0) {

		gl_xml_set_prop_double (node, "width", template->page_width);
		gl_xml_set_prop_double (node, "height", template->page_height);

	}

	xmlSetProp (node, "description", template->description);

	xml_add_label (template, node, ns);

	for ( p=template->name->next; p != NULL; p=p->next ) {
		xml_add_alias( p->data, node, ns );
	}

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Sheet->Label Node.                                     */
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

	node = xmlNewChild(root, ns, "Label", NULL);

	xmlSetProp (node, "id", "0");

	switch (template->label.style) {

	case GL_TEMPLATE_STYLE_RECT:
		xmlSetProp (node, "style", "rectangle");
		gl_xml_set_prop_double (node, "width",  template->label.rect.w);
		gl_xml_set_prop_double (node, "height", template->label.rect.h);
		gl_xml_set_prop_double (node, "round",  template->label.rect.r);
		break;

	case GL_TEMPLATE_STYLE_ROUND:
		xmlSetProp (node, "style", "round");
		gl_xml_set_prop_double (node, "radius",  template->label.round.r);
		break;

	case GL_TEMPLATE_STYLE_CD:
		xmlSetProp (node, "style", "cd");
		gl_xml_set_prop_double (node, "radius",  template->label.cd.r1);
		gl_xml_set_prop_double (node, "hole",    template->label.cd.r2);
		if (template->label.cd.w != 0.0) {
			gl_xml_set_prop_double (node, "width",  template->label.cd.w);
		}
		if (template->label.cd.h != 0.0) {
			gl_xml_set_prop_double (node, "height", template->label.cd.h);
		}
		break;

	default:
		g_warning ("Unknown label style");
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
/* PRIVATE.  Add XML Sheet->Label->Layout Node.                             */
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
	gl_xml_set_prop_double (node, "x0", layout->x0);
	gl_xml_set_prop_double (node, "y0", layout->y0);
	gl_xml_set_prop_double (node, "dx", layout->dx);
	gl_xml_set_prop_double (node, "dy", layout->dy);

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Sheet->Label->Markup (margin) Node.                    */
/*--------------------------------------------------------------------------*/
static void
xml_add_markup_margin (glTemplateMarkupMargin *margin,
		       xmlNodePtr              root,
		       xmlNsPtr                ns)
{
	xmlNodePtr  node;

	gl_debug (DEBUG_TEMPLATE, "START");

	node = xmlNewChild(root, ns, "Markup", NULL);
	xmlSetProp (node, "type", "margin");

	gl_xml_set_prop_double (node, "size", margin->size);

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Sheet->Label->Markup (line) Node.                      */
/*--------------------------------------------------------------------------*/
static void
xml_add_markup_line (glTemplateMarkupLine *line,
		     xmlNodePtr            root,
		     xmlNsPtr              ns)
{
	xmlNodePtr  node;

	gl_debug (DEBUG_TEMPLATE, "START");

	node = xmlNewChild(root, ns, "Markup", NULL);
	xmlSetProp (node, "type", "line");

	gl_xml_set_prop_double (node, "x1", line->x1);
	gl_xml_set_prop_double (node, "y1", line->y1);
	gl_xml_set_prop_double (node, "x2", line->x2);
	gl_xml_set_prop_double (node, "y2", line->y2);

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Sheet->Alias Node.                                     */
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

