/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  template.c:  template module
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

#include <string.h>
#include <libgnomeprint/gnome-print-paper.h>

#include "prefs.h"
#include "util.h"
#include "template.h"

#include "debug.h"

#define GL_DATA_DIR gnome_program_locate_file (NULL,\
					 GNOME_FILE_DOMAIN_APP_DATADIR,\
					 "glabels",\
					 FALSE, NULL)

#define FULL_PAGE "Full-page"

/*===========================================*/
/* Private types                             */
/*===========================================*/

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GList *templates = NULL;

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/
static glTemplate *template_full_page           (const gchar *page_size);

static GList      *read_templates               (void);

static gchar      *get_home_data_dir            (void);
static GList      *read_template_files_from_dir (GList *templates,
						 const gchar *dirname);
static GList      *read_templates_from_file     (GList *templates,
						 gchar *xml_filename);

static void        xml_parse_label              (xmlNodePtr label_node,
						 glTemplate *template);
static void        xml_parse_layout             (xmlNodePtr layout_node,
						 glTemplate *template);
static void        xml_parse_markup             (xmlNodePtr markup_node,
						 glTemplate *template);
static void        xml_parse_alias              (xmlNodePtr alias_node,
						 glTemplate *template);

static void        xml_add_label                (const glTemplate *template,
						 xmlNodePtr root,
						 xmlNsPtr ns);
static void        xml_add_layout               (glTemplateLayout *layout,
						 xmlNodePtr root,
						 xmlNsPtr ns);
static void        xml_add_markup_margin        (glTemplateMarkupMargin *margin,
						 xmlNodePtr root,
						 xmlNsPtr ns);
static void        xml_add_alias                (gchar *name,
						 xmlNodePtr root,
						 xmlNsPtr ns);

static gint compare_origins (gconstpointer a,
			     gconstpointer b,
			     gpointer user_data);

static glTemplateLayout *layout_new  (gdouble nx,
				      gdouble ny,
				      gdouble x0,
				      gdouble y0,
				      gdouble dx,
				      gdouble dy);
static glTemplateLayout *layout_dup  (glTemplateLayout *orig_layout);
static void              layout_free (glTemplateLayout **layout);

static glTemplateMarkup *markup_margin_new  (gdouble size);
static glTemplateMarkup *markup_dup         (glTemplateMarkup *orig_markup);
static void              markup_free        (glTemplateMarkup **markup);

/*****************************************************************************/
/* Initialize module.                                                        */
/*****************************************************************************/
void
gl_template_init (void)
{
	GList *page_sizes, *p;

	gl_debug (DEBUG_TEMPLATE, "START");

	templates = read_templates ();

	page_sizes = gl_template_get_page_size_list ();
	for ( p=page_sizes; p != NULL; p=p->next ) {
		templates = g_list_append (templates,
					   template_full_page (p->data));
	}
	gl_template_free_page_size_list (&page_sizes);

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*****************************************************************************/
/* Get a list of valid page size names                                       */
/*****************************************************************************/
GList *
gl_template_get_page_size_list (void)
{
	GList *names = NULL;
	GList *p, *paper_list;
	GnomePrintPaper *paper;

	gl_debug (DEBUG_TEMPLATE, "START");

	paper_list = gnome_print_paper_get_list();
	for ( p=paper_list; p != NULL; p=p->next ) {
		paper = (GnomePrintPaper *)p->data;
		if ( g_strcasecmp(paper->name, "custom") != 0 ) {
			names = g_list_append (names, g_strdup (paper->name));
		}
	}

	gl_debug (DEBUG_TEMPLATE, "END");
	return names;
}

/*****************************************************************************/
/* Free a list of page size names.                                           */
/*****************************************************************************/
void
gl_template_free_page_size_list (GList ** names)
{
	GList *p_name;

	gl_debug (DEBUG_TEMPLATE, "START");

	for (p_name = *names; p_name != NULL; p_name = p_name->next) {
		g_free (p_name->data);
		p_name->data = NULL;
	}

	g_list_free (*names);
	*names = NULL;

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*****************************************************************************/
/* Get a list of valid template names for given page size                    */
/*****************************************************************************/
GList *
gl_template_get_name_list (const gchar * page_size)
{
	GList *p_tmplt, *p_name;
	glTemplate *template;
	gchar *str;
	GList *names = NULL;

	gl_debug (DEBUG_TEMPLATE, "START");

	for (p_tmplt = templates; p_tmplt != NULL; p_tmplt = p_tmplt->next) {
		template = (glTemplate *) p_tmplt->data;
		if (g_strcasecmp (page_size, template->page_size) == 0) {
			for (p_name = template->name; p_name != NULL;
			     p_name = p_name->next) {
				str = g_strdup_printf("%s: %s",
						      (gchar *) p_name->data,
						      template->description);
				names = g_list_insert_sorted (names, str,
							     (GCompareFunc)g_strcasecmp);
			}
		}
	}

	gl_debug (DEBUG_TEMPLATE, "templates = %p", templates);
	gl_debug (DEBUG_TEMPLATE, "names = %p", names);

	gl_debug (DEBUG_TEMPLATE, "END");
	return names;
}

/*****************************************************************************/
/* Free a list of template names.                                            */
/*****************************************************************************/
void
gl_template_free_name_list (GList ** names)
{
	GList *p_name;

	gl_debug (DEBUG_TEMPLATE, "START");

	for (p_name = *names; p_name != NULL; p_name = p_name->next) {
		g_free (p_name->data);
		p_name->data = NULL;
	}

	g_list_free (*names);
	*names = NULL;

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*****************************************************************************/
/* Return a template structure from a name.                                  */
/*****************************************************************************/
glTemplate *
gl_template_from_name (const gchar * name)
{
	GList *p_tmplt, *p_name;
	glTemplate *template;
	gchar **split_name;

	gl_debug (DEBUG_TEMPLATE, "START");

	if (name == NULL) {
		/* If no name, return first template as a default */
		return (glTemplate *) templates->data;
	}

	split_name = g_strsplit (name, ":", 2);

	for (p_tmplt = templates; p_tmplt != NULL; p_tmplt = p_tmplt->next) {
		template = (glTemplate *) p_tmplt->data;
		for (p_name = template->name; p_name != NULL;
		     p_name = p_name->next) {
			if (g_strcasecmp (p_name->data, split_name[0]) == 0) {
				g_strfreev (split_name);
				gl_debug (DEBUG_TEMPLATE, "END");
				return gl_template_dup (template);
			}
		}
	}

	g_strfreev (split_name);

	gl_debug (DEBUG_TEMPLATE, "END");
	return NULL;
}

/*****************************************************************************/
/* Copy a template.                                                          */
/*****************************************************************************/
glTemplate *gl_template_dup (const glTemplate *orig_template)
{
	glTemplate       *template;
	GList            *p;
	glTemplateLayout *layout;
	glTemplateMarkup *markup;

	gl_debug (DEBUG_TEMPLATE, "START");

	template = g_new0 (glTemplate,1);

	template->name = NULL;
	for ( p=orig_template->name; p != NULL; p=p->next ) {
		template->name = g_list_append (template->name,
						g_strdup (p->data));
	}
	template->description = g_strdup (orig_template->description);
	template->page_size   = g_strdup (orig_template->page_size);

	template->label       = orig_template->label;

	template->label.any.layouts = NULL;
	for ( p=orig_template->label.any.layouts; p != NULL; p=p->next ) {
		layout = (glTemplateLayout *)p->data;
		template->label.any.layouts =
			g_list_append (template->label.any.layouts,
				       layout_dup (layout));
	}

	template->label.any.markups = NULL;
	for ( p=orig_template->label.any.markups; p != NULL; p=p->next ) {
		markup = (glTemplateMarkup *)p->data;
		template->label.any.markups =
			g_list_append (template->label.any.markups,
				       markup_dup (markup));
	}

	gl_debug (DEBUG_TEMPLATE, "END");
	return template;
}

/*****************************************************************************/
/* Free up a template.                                                       */
/*****************************************************************************/
void gl_template_free (glTemplate **template)
{
	GList *p;

	gl_debug (DEBUG_TEMPLATE, "START");

	if ( *template != NULL ) {

		for ( p=(*template)->name; p != NULL; p=p->next ) {
			g_free (p->data);
			p->data = NULL;
		}
		g_list_free ((*template)->name);
		(*template)->name = NULL;

		g_free ((*template)->description);
		(*template)->description = NULL;

		g_free ((*template)->page_size);
		(*template)->page_size = NULL;

		for ( p=(*template)->label.any.layouts; p != NULL; p=p->next ) {
			layout_free ((glTemplateLayout **)&p->data);
		}
		g_list_free ((*template)->label.any.layouts);
		(*template)->label.any.layouts = NULL;

		for ( p=(*template)->label.any.markups; p != NULL; p=p->next ) {
			markup_free ((glTemplateMarkup **)&p->data);
		}
		g_list_free ((*template)->label.any.markups);
		(*template)->label.any.markups = NULL;

		g_free (*template);
		*template = NULL;

	}

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Make a template for a full page of the given page size.        */
/*--------------------------------------------------------------------------*/
static glTemplate *
template_full_page (const gchar *page_size)
{
	const GnomePrintPaper *paper;
	glTemplate *template;

	paper = gnome_print_paper_get_by_name (page_size);
	if ( paper == NULL ) {
		return NULL;
	}

	template = g_new0 (glTemplate, 1);

	template->name         = g_list_append (template->name,
					 g_strdup_printf(_("Generic %s full page"),
							 page_size));
	template->page_size    = g_strdup(page_size);
	template->description  = g_strdup(FULL_PAGE);

	template->label.style  = GL_TEMPLATE_STYLE_RECT;
	template->label.rect.w = paper->width;
	template->label.rect.h = paper->height;
	template->label.rect.r = 0.0;

	template->label.any.layouts =
		g_list_append (template->label.any.layouts,
			       layout_new (1, 1, 0., 0., 0., 0.));

	template->label.any.markups =
		g_list_append (template->label.any.markups,
			       markup_margin_new (5.0));

	return template;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Read templates from various  files.                            */
/*--------------------------------------------------------------------------*/
static GList *
read_templates (void)
{
	gchar *home_data_dir = get_home_data_dir ();
	GList *templates = NULL;

	gl_debug (DEBUG_TEMPLATE, "START");

	LIBXML_TEST_VERSION;

	templates = read_template_files_from_dir (templates, GL_DATA_DIR);
	templates = read_template_files_from_dir (templates, home_data_dir);

	g_free (home_data_dir);

	if (templates == NULL) {
		g_warning (_("No template files found!"));
	}

	gl_debug (DEBUG_TEMPLATE, "END");
	return templates;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  get '~/.glabels' directory path.                               */
/*--------------------------------------------------------------------------*/
static gchar *
get_home_data_dir (void)
{
	gchar *dir = gnome_util_prepend_user_home (".glabels");

	gl_debug (DEBUG_TEMPLATE, "START");

	/* Try to create ~/.glabels directory.  If it exists, no problem. */
	mkdir (dir, 0775);

	gl_debug (DEBUG_TEMPLATE, "END");
	return dir;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Read all template files from given directory.  Append to list. */
/*--------------------------------------------------------------------------*/
static GList *
read_template_files_from_dir (GList * templates,
			      const gchar * dirname)
{
	GDir *dp;
	const gchar *filename, *extension;
	gchar *full_filename = NULL;
	GError *gerror = NULL;

	gl_debug (DEBUG_TEMPLATE, "START");

	if (dirname == NULL)
		return templates;

	dp = g_dir_open (dirname, 0, &gerror);
	if (gerror != NULL) {
	        g_warning ("cannot open data directory: %s", gerror->message );
		gl_debug (DEBUG_TEMPLATE, "END");
		return templates;
	}

	while ((filename = g_dir_read_name (dp)) != NULL) {

		extension = strrchr (filename, '.');

		if (extension != NULL) {

			if (strcasecmp (extension, ".template") == 0) {

				full_filename =
				    g_build_filename (dirname, filename, NULL);
				templates =
				    read_templates_from_file (templates,
							      full_filename);
				g_free (full_filename);

			}

		}

	}

	g_dir_close (dp);

	gl_debug (DEBUG_TEMPLATE, "END");
	return templates;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Read templates from template file.                             */
/*--------------------------------------------------------------------------*/
static GList *
read_templates_from_file (GList * templates,
			  gchar * xml_filename)
{
	xmlDocPtr doc;
	xmlNodePtr root, node;
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
			template = gl_template_xml_parse_sheet (node);
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
gl_template_xml_parse_sheet (xmlNodePtr sheet_node)
{
	glTemplate *template;
	xmlNodePtr node;

	gl_debug (DEBUG_TEMPLATE, "START");

	template = g_new0 (glTemplate, 1);

	template->name = g_list_append (template->name,
					xmlGetProp (sheet_node, "name"));
	template->page_size = xmlGetProp (sheet_node, "size");
	if ( strcmp (template->page_size,"US-Letter") == 0 ) {
		/* Compatibility with old pre-1.0 template files.*/
		template->page_size = "US Letter";
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
xml_parse_label (xmlNodePtr label_node,
		 glTemplate * template)
{
	xmlNodePtr node;
	gchar *style;

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

	switch (template->label.style) {
	case GL_TEMPLATE_STYLE_RECT:
		template->label.rect.w =
		    g_strtod (xmlGetProp (label_node, "width"), NULL);
		template->label.rect.h =
		    g_strtod (xmlGetProp (label_node, "height"), NULL);
		template->label.rect.r =
		    g_strtod (xmlGetProp (label_node, "round"), NULL);
		break;
	case GL_TEMPLATE_STYLE_ROUND:
		template->label.round.r =
		    g_strtod (xmlGetProp (label_node, "radius"), NULL);
		break;
	case GL_TEMPLATE_STYLE_CD:
		template->label.cd.r1 =
		    g_strtod (xmlGetProp (label_node, "radius"), NULL);
		template->label.cd.r2 =
		    g_strtod (xmlGetProp (label_node, "hole"), NULL);
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
xml_parse_layout (xmlNodePtr layout_node,
		  glTemplate * template)
{
	gint nx, ny;
	gdouble x0, y0, dx, dy;
	xmlNodePtr node;

	gl_debug (DEBUG_TEMPLATE, "START");

	sscanf (xmlGetProp (layout_node, "nx"), "%d", &nx);
	sscanf (xmlGetProp (layout_node, "ny"), "%d", &ny);
	x0 = g_strtod (xmlGetProp (layout_node, "x0"), NULL);
	y0 = g_strtod (xmlGetProp (layout_node, "y0"), NULL);
	dx = g_strtod (xmlGetProp (layout_node, "dx"), NULL);
	dy = g_strtod (xmlGetProp (layout_node, "dy"), NULL);

	for (node = layout_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (g_strcasecmp (node->name, "text") != 0) {
			g_warning ("bad node =  \"%s\"", node->name);
		}
	}

	template->label.any.layouts =
		g_list_append (template->label.any.layouts,
			       layout_new (nx, ny, x0, y0, dx, dy));

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Sheet->Label->Markup Node.                           */
/*--------------------------------------------------------------------------*/
static void
xml_parse_markup (xmlNodePtr markup_node,
		  glTemplate * template)
{
	gchar *type;
	gdouble size;
	xmlNodePtr node;

	gl_debug (DEBUG_TEMPLATE, "START");

	type = xmlGetProp (markup_node, "type");
	if (g_strcasecmp (type, "margin") == 0) {
		size = g_strtod (xmlGetProp (markup_node, "size"), NULL);
		template->label.any.markups =
			g_list_append (template->label.any.markups,
				       markup_margin_new (size));
	}

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
xml_parse_alias (xmlNodePtr alias_node,
		 glTemplate * template)
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
gl_template_xml_add_sheet (const glTemplate * template,
			   xmlNodePtr root,
			   xmlNsPtr ns)
{
	xmlNodePtr node;
	GList *p;

	gl_debug (DEBUG_TEMPLATE, "START");

	node = xmlNewChild (root, ns, "Sheet", NULL);

	xmlSetProp (node, "name", template->name->data);
	xmlSetProp (node, "size", template->page_size);
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
	       xmlNodePtr root,
	       xmlNsPtr ns)
{
	xmlNodePtr node;
	gchar *string;
	GList *p;
	glTemplateMarkup *markup;
	glTemplateLayout *layout;

	gl_debug (DEBUG_TEMPLATE, "START");

	node = xmlNewChild(root, ns, "Label", NULL);

	xmlSetProp (node, "id", "0");

	switch (template->label.style) {
	case GL_TEMPLATE_STYLE_RECT:
		xmlSetProp (node, "style", "rectangle");
		string = g_strdup_printf ("%g", template->label.rect.w);
		xmlSetProp (node, "width", string);
		g_free (string);
		string = g_strdup_printf ("%g", template->label.rect.h);
		xmlSetProp (node, "height", string);
		g_free (string);
		string = g_strdup_printf ("%g", template->label.rect.r);
		xmlSetProp (node, "round", string);
		g_free (string);
		break;
	case GL_TEMPLATE_STYLE_ROUND:
		xmlSetProp (node, "style", "round");
		string = g_strdup_printf ("%g", template->label.round.r);
		xmlSetProp (node, "radius", string);
		g_free (string);
		break;
	case GL_TEMPLATE_STYLE_CD:
		xmlSetProp (node, "style", "cd");
		string = g_strdup_printf ("%g", template->label.cd.r1);
		xmlSetProp (node, "radius", string);
		g_free (string);
		string = g_strdup_printf ("%g", template->label.cd.r2);
		xmlSetProp (node, "hole", string);
		g_free (string);
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
		xmlNodePtr root,
		xmlNsPtr ns)
{
	xmlNodePtr node;
	gchar *string;

	gl_debug (DEBUG_TEMPLATE, "START");

	node = xmlNewChild(root, ns, "Layout", NULL);
	string = g_strdup_printf ("%d", layout->nx);
	xmlSetProp (node, "nx", string);
	g_free (string);
	string = g_strdup_printf ("%d", layout->ny);
	xmlSetProp (node, "ny", string);
	g_free (string);
	string = g_strdup_printf ("%g", layout->x0);
	xmlSetProp (node, "x0", string);
	g_free (string);
	string = g_strdup_printf ("%g", layout->y0);
	xmlSetProp (node, "y0", string);
	g_free (string);
	string = g_strdup_printf ("%g", layout->dx);
	xmlSetProp (node, "dx", string);
	g_free (string);
	string = g_strdup_printf ("%g", layout->dy);
	xmlSetProp (node, "dy", string);
	g_free (string);

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Sheet->Label->Markup Node.                             */
/*--------------------------------------------------------------------------*/
static void
xml_add_markup_margin (glTemplateMarkupMargin *margin,
		       xmlNodePtr root,
		       xmlNsPtr ns)
{
	xmlNodePtr node;
	gchar *string;

	gl_debug (DEBUG_TEMPLATE, "START");

	node = xmlNewChild(root, ns, "Markup", NULL);
	xmlSetProp (node, "type", "margin");

	string = g_strdup_printf ("%g", margin->size);
	xmlSetProp (node, "size", string);
	g_free (string);

	gl_debug (DEBUG_TEMPLATE, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Sheet->Alias Node.                                     */
/*--------------------------------------------------------------------------*/
static void
xml_add_alias (gchar *name,
	       xmlNodePtr root,
	       xmlNsPtr ns)
{
	xmlNodePtr node;

	gl_debug (DEBUG_TEMPLATE, "START");

	node = xmlNewChild (root, ns, "Alias", NULL);
	xmlSetProp (node, "name", name);

	gl_debug (DEBUG_TEMPLATE, "END");
}

/****************************************************************************/
/* Get label size description.                                              */ 
/****************************************************************************/
gchar *
gl_template_get_label_size_desc (const glTemplate *template)
{
	glPrefsUnits units;
	const gchar *units_string;
	gdouble units_per_point;
	gchar *string = NULL;

	units           = gl_prefs_get_units ();
	units_string    = gl_prefs_get_units_string ();
	units_per_point = gl_prefs_get_units_per_point ();

	switch (template->label.style) {
	case GL_TEMPLATE_STYLE_RECT:
		if ( units == GL_PREFS_UNITS_INCHES ) {
			gchar *xstr, *ystr;

			xstr = gl_util_fraction (template->label.rect.w * units_per_point);
			ystr = gl_util_fraction (template->label.rect.h * units_per_point);
			string = g_strdup_printf (_("%s x %s %s"),
						  xstr, ystr, units_string);
			g_free (xstr);
			g_free (ystr);
		} else {
			string = g_strdup_printf (_("%.5g x %.5g %s"),
						  template->label.rect.w * units_per_point,
						  template->label.rect.h * units_per_point,
						  units_string);
		}
		break;
	case GL_TEMPLATE_STYLE_ROUND:
		if ( units == GL_PREFS_UNITS_INCHES ) {
			gchar *dstr;

			dstr = gl_util_fraction (2.0 * template->label.round.r * units_per_point);
			string = g_strdup_printf (_("%s %s diameter"),
						  dstr, units_string);
			g_free (dstr);
		} else {
			string = g_strdup_printf (_("%.5g %s diameter"),
						  2.0 * template->label.round.r * units_per_point,
						  units_string);
		}
		break;
	case GL_TEMPLATE_STYLE_CD:
		if ( units == GL_PREFS_UNITS_INCHES ) {
			gchar *dstr;

			dstr = gl_util_fraction (2.0 * template->label.cd.r1 * units_per_point);
			string = g_strdup_printf (_("%s %s diameter"),
						  dstr, units_string);
			g_free (dstr);
		} else {
			string = g_strdup_printf (_("%.5g %s diameter"),
						  2.0 * template->label.cd.r1 * units_per_point,
						  units_string);
		}
		break;
	default:
		break;
	}

	return string;
}

/****************************************************************************/
/* Get raw label size (width and height).                                   */
/****************************************************************************/
void
gl_template_get_label_size (const glTemplate *template,
			    gdouble          *w,
			    gdouble          *h)
{
	switch (template->label.style) {
	case GL_TEMPLATE_STYLE_RECT:
		*w = template->label.rect.w;
		*h = template->label.rect.h;
		break;
	case GL_TEMPLATE_STYLE_ROUND:
		*w = 2.0 * template->label.round.r;
		*h = 2.0 * template->label.round.r;
		break;
	case GL_TEMPLATE_STYLE_CD:
		*w = 2.0 * template->label.cd.r1;
		*h = 2.0 * template->label.cd.r1;
		break;
	default:
		*w = 0.0;
		*h = 0.0;
		break;
	}
}

/****************************************************************************/
/* Get total number of labels per sheet.                                    */
/****************************************************************************/
gint
gl_template_get_n_labels (const glTemplate *template)
{
	gint n_labels = 0;
	GList *p;
	glTemplateLayout *layout;

	for ( p=template->label.any.layouts; p != NULL; p=p->next ) {
		layout = (glTemplateLayout *)p->data;

		n_labels += layout->nx * layout->ny;
	}

	return n_labels;
}

/****************************************************************************/
/* Get array of origins of individual labels.                               */
/****************************************************************************/
glTemplateOrigin *
gl_template_get_origins (const glTemplate *template)
{
	gint i_label, n_labels, ix, iy;
	glTemplateOrigin *origins;
	GList *p;
	glTemplateLayout *layout;

	n_labels = gl_template_get_n_labels (template);
	origins = g_new0 (glTemplateOrigin, n_labels);

	i_label = 0;
	for ( p=template->label.any.layouts; p != NULL; p=p->next ) {
		layout = (glTemplateLayout *)p->data;

		for (iy = 0; iy < layout->ny; iy++) {
			for (ix = 0; ix < layout->nx; ix++, i_label++) {
				origins[i_label].x = ix*layout->dx + layout->x0;
				origins[i_label].y = iy*layout->dy + layout->y0;
			}
		}
	}

	g_qsort_with_data (origins, n_labels, sizeof(glTemplateOrigin),
			   compare_origins, NULL);

	return origins;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Sort origins comparison function, first by y then by x.        */
/*--------------------------------------------------------------------------*/
static gint
compare_origins (gconstpointer a,
		 gconstpointer b,
		 gpointer user_data)
{
	const glTemplateOrigin *a_origin = a, *b_origin = b;

	if ( a_origin->y < b_origin->y ) {
		return -1;
	} else if ( a_origin->y > b_origin->y ) {
		return +1;
	} else {
		if ( a_origin->x < b_origin->x ) {
			return -1;
		} else if ( a_origin->x > b_origin->x ) {
			return +1;
		} else {
			return 0; /* hopefully 2 labels won't have the same origin */
		}
	}
}

/****************************************************************************/
/* Get a description of the layout and number of labels.                    */
/****************************************************************************/
gchar *
gl_template_get_layout_desc (const glTemplate *template)
{
	gint n_labels;
	glTemplateLayout *layout;
	gchar *string;

	n_labels = gl_template_get_n_labels (template);

	if ( template->label.any.layouts->next == NULL ) {
		layout = (glTemplateLayout *)template->label.any.layouts->data;
		string = g_strdup_printf (_("%d x %d  (%d per sheet)"),
					  layout->nx, layout->ny,
					  n_labels);
	} else {
		string = g_strdup_printf (_("%d per sheet"),
					  n_labels);
	}

	return string;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Create new layout structure.                                   */
/*--------------------------------------------------------------------------*/
static glTemplateLayout *
layout_new (gdouble nx,
	    gdouble ny,
	    gdouble x0,
	    gdouble y0,
	    gdouble dx,
	    gdouble dy)
{
	glTemplateLayout *layout;

	layout = g_new0 (glTemplateLayout, 1);

	layout->nx = nx;
	layout->ny = ny;
	layout->x0 = x0;
	layout->y0 = y0;
	layout->dx = dx;
	layout->dy = dy;

	return layout;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Duplicate layout structure.                                    */
/*--------------------------------------------------------------------------*/
static glTemplateLayout *
layout_dup (glTemplateLayout *orig_layout)
{
	glTemplateLayout *layout;

	layout = g_new0 (glTemplateLayout, 1);

	/* copy contents */
	*layout = *orig_layout;

	return layout;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Free layout structure.                                         */
/*--------------------------------------------------------------------------*/
static void
layout_free (glTemplateLayout **layout)
{
	g_free (*layout);
	*layout = NULL;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Create new margin markup structure.                            */
/*--------------------------------------------------------------------------*/
static glTemplateMarkup *
markup_margin_new (gdouble size)
{
	glTemplateMarkup *markup;

	markup = g_new0 (glTemplateMarkup, 1);

	markup->type        = GL_TEMPLATE_MARKUP_MARGIN;
	markup->margin.size = size;

	return markup;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Duplicate markup structure.                                    */
/*--------------------------------------------------------------------------*/
static glTemplateMarkup *
markup_dup (glTemplateMarkup *orig_markup)
{
	glTemplateMarkup *markup;

	markup = g_new0 (glTemplateMarkup, 1);

	*markup = *orig_markup;

	return markup;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Free markup structure.                                         */
/*--------------------------------------------------------------------------*/
static void
markup_free (glTemplateMarkup **markup)
{
	g_free (*markup);
	*markup = NULL;
}
