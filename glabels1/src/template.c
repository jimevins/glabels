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

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>


#include "template.h"

#include "debug.h"

#ifdef PACKAGE_DATA_DIR
#define GL_DATA_DIR (PACKAGE_DATA_DIR G_DIR_SEPARATOR_S "glabels")
#else
#define GL_DATA_DIR gnome_datadir_file("glabels")
#endif

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
static GList *read_templates (void);

static gchar *get_home_data_dir (void);
static GList *read_template_files_from_dir (GList * templates,
					    const gchar * dirname);
static GList *read_templates_from_file (GList * templates,
					gchar * xml_filename);

static void xml_parse_label (xmlNodePtr label_node, glTemplate * template);
static void xml_parse_layout (xmlNodePtr layout_node, glTemplate * template);
static void xml_parse_alias (xmlNodePtr alias_node, glTemplate * template);

static void xml_add_label (glTemplate *template, xmlNodePtr root, xmlNsPtr ns);
static void xml_add_layout (glTemplate *template, xmlNodePtr root, xmlNsPtr ns);
static void xml_add_alias (gchar *name, xmlNodePtr root, xmlNsPtr ns);

/*****************************************************************************/
/* Initialize module.                                                        */
/*****************************************************************************/
void
gl_template_init (void)
{
	templates = read_templates ();
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

	return names;
}

/*****************************************************************************/
/* Free a list of template names.                                            */
/*****************************************************************************/
void
gl_template_free_name_list (GList ** names)
{
	GList *p_name;

	for (p_name = *names; p_name != NULL; p_name = p_name->next) {
		g_free (p_name->data);
		p_name->data = NULL;
	}

	g_list_free (*names);
	*names = NULL;
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
				return gl_template_copy(template);
			}
		}
	}

	g_strfreev (split_name);

	return NULL;
}

/*****************************************************************************/
/* Copy a template.                                                          */
/*****************************************************************************/
glTemplate *gl_template_copy (const glTemplate *orig_template)
{
	glTemplate *template;
	GList *p;

	template = g_new0 (glTemplate,1);

	/* Shallow copy first */
	*template = *orig_template;

	/* Now the deep stuff */
	template->name = NULL;
	for ( p=orig_template->name; p != NULL; p=p->next ) {
		template->name = g_list_append (template->name,
						g_strdup (p->data));
	}
	template->description = g_strdup (orig_template->description);
	template->page_size = g_strdup (orig_template->page_size);

	return template;
}

/*****************************************************************************/
/* Free up a template.                                                       */
/*****************************************************************************/
void gl_template_free (glTemplate **template)
{
	GList *p;

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

	g_free (*template);
	*template = NULL;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Read templates from various  files.                            */
/*--------------------------------------------------------------------------*/
static GList *
read_templates (void)
{
	gchar *home_data_dir = get_home_data_dir ();
	GList *templates = NULL;

	LIBXML_TEST_VERSION;

	templates = read_template_files_from_dir (templates, GL_DATA_DIR);
	templates = read_template_files_from_dir (templates, home_data_dir);

	g_free (home_data_dir);

	if (templates == NULL) {
		WARN (_("No template files found!"));
	}

	return templates;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  get '~/.glabels' directory path.                               */
/*--------------------------------------------------------------------------*/
static gchar *
get_home_data_dir (void)
{
	gchar *dir = gnome_util_prepend_user_home (".glabels");

	/* Try to create ~/.glabels directory.  If it exists, no problem. */
	mkdir (dir, 0775);

	return dir;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Read all template files from given directory.  Append to list. */
/*--------------------------------------------------------------------------*/
static GList *
read_template_files_from_dir (GList * templates,
			      const gchar * dirname)
{
	DIR *dp;
	struct dirent *d_entry;
	gchar *filename, *extension;
	gchar *full_filename = NULL;

	if (dirname == NULL)
		return templates;

	dp = opendir (dirname);
	if (dp == NULL)
		return templates;

	while ((d_entry = readdir (dp)) != NULL) {

		filename = d_entry->d_name;
		extension = strrchr (filename, '.');

		if (extension != NULL) {

			if (strcasecmp (extension, ".template") == 0) {

				full_filename =
				    g_concat_dir_and_file (dirname, filename);
				templates =
				    read_templates_from_file (templates,
							      full_filename);
				g_free (full_filename);

			}

		}

	}

	closedir (dp);

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

	doc = xmlParseFile (xml_filename);
	if (!doc) {
		WARN ("\"%s\" is not a glabels template file (not XML)",
		      xml_filename);
		return templates;
	}

	root = xmlDocGetRootElement (doc);
	if (!root || !root->name) {
		WARN ("\"%s\" is not a glabels template file (no root node)",
		      xml_filename);
		xmlFreeDoc (doc);
		return templates;
	}
	if (g_strcasecmp (root->name, "glabels-templates") != 0) {
		WARN ("\"%s\" is not a glabels template file (wrong root node)",
		      xml_filename);
		xmlFreeDoc (doc);
		return templates;
	}

	for (node = root->xmlChildrenNode; node != NULL; node = node->next) {

		if (g_strcasecmp (node->name, "Sheet") == 0) {
			template = g_new0 (glTemplate, 1);
			gl_template_xml_parse_sheet (template, node);
			templates = g_list_append (templates, template);
		} else {
			if (g_strcasecmp (node->name, "text") != 0) {
				WARN ("bad node =  \"%s\"", node->name);
			}
		}
	}

	xmlFreeDoc (doc);

	return templates;
}

/*****************************************************************************/
/* Parse XML template Node.                                                  */
/*****************************************************************************/
void
gl_template_xml_parse_sheet (glTemplate * template,
			     xmlNodePtr sheet_node)
{
	xmlNodePtr node;

	template->name = g_list_append (template->name,
					xmlGetProp (sheet_node, "name"));
	template->page_size = xmlGetProp (sheet_node, "size");
	template->description = xmlGetProp (sheet_node, "description");

	for (node = sheet_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (g_strcasecmp (node->name, "Label") == 0) {
			xml_parse_label (node, template);
		} else if (g_strcasecmp (node->name, "Alias") == 0) {
			xml_parse_alias (node, template);
		} else {
			if (g_strcasecmp (node->name, "text") != 0) {
				WARN ("bad node =  \"%s\"", node->name);
			}
		}
	}

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

	style = xmlGetProp (label_node, "style");
	if (g_strcasecmp (style, "rectangle") == 0) {
		template->style = GL_TEMPLATE_STYLE_RECT;
	} else if (g_strcasecmp (style, "round") == 0) {
		template->style = GL_TEMPLATE_STYLE_ROUND;
	} else if (g_strcasecmp (style, "cd") == 0) {
		template->style = GL_TEMPLATE_STYLE_CD;
	} else {
		WARN ("Unknown label style in template");
	}

	if (template->style == GL_TEMPLATE_STYLE_RECT) {
		template->label_width =
		    g_strtod (xmlGetProp (label_node, "width"), NULL);
		template->label_height =
		    g_strtod (xmlGetProp (label_node, "height"), NULL);
		template->label_round =
		    g_strtod (xmlGetProp (label_node, "round"), NULL);
	} else if (template->style == GL_TEMPLATE_STYLE_ROUND) {
		template->label_radius =
		    g_strtod (xmlGetProp (label_node, "radius"), NULL);
		template->label_width = 2.0 * template->label_radius;
		template->label_height = 2.0 * template->label_radius;
	} else if (template->style == GL_TEMPLATE_STYLE_CD) {
		template->label_radius =
		    g_strtod (xmlGetProp (label_node, "radius"), NULL);
		template->label_hole =
		    g_strtod (xmlGetProp (label_node, "hole"), NULL);
		template->label_width = 2.0 * template->label_radius;
		template->label_height = 2.0 * template->label_radius;
	}

	template->label_margin =
	    g_strtod (xmlGetProp (label_node, "margin"), NULL);

	for (node = label_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (g_strcasecmp (node->name, "Layout") == 0) {
			xml_parse_layout (node, template);
		} else {
			if (g_strcasecmp (node->name, "text") != 0) {
				WARN ("bad node =  \"%s\"", node->name);
			}
		}
	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Sheet->Label->Layout Node.                           */
/*--------------------------------------------------------------------------*/
static void
xml_parse_layout (xmlNodePtr layout_node,
		  glTemplate * template)
{
	xmlNodePtr node;

	sscanf (xmlGetProp (layout_node, "nx"), "%d", &(template->nx));
	sscanf (xmlGetProp (layout_node, "ny"), "%d", &(template->ny));
	template->x0 = g_strtod (xmlGetProp (layout_node, "x0"), NULL);
	template->y0 = g_strtod (xmlGetProp (layout_node, "y0"), NULL);
	template->dx = g_strtod (xmlGetProp (layout_node, "dx"), NULL);
	template->dy = g_strtod (xmlGetProp (layout_node, "dy"), NULL);

	for (node = layout_node->xmlChildrenNode; node != NULL;
	     node = node->next) {
		if (g_strcasecmp (node->name, "text") != 0) {
			WARN ("bad node =  \"%s\"", node->name);
		}
	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Parse XML Sheet->Alias Node.                                   */
/*--------------------------------------------------------------------------*/
static void
xml_parse_alias (xmlNodePtr alias_node,
		 glTemplate * template)
{
	template->name = g_list_append (template->name,
					xmlGetProp (alias_node, "name"));
}

/****************************************************************************/
/* Add XML Template Node                                                    */
/****************************************************************************/
void
gl_template_xml_add_sheet (glTemplate * template,
			   xmlNodePtr root,
			   xmlNsPtr ns)
{
	xmlNodePtr node;
	GList *p;

	node = xmlNewChild (root, ns, "Sheet", NULL);

	xmlSetProp (node, "name", template->name->data);
	xmlSetProp (node, "size", template->page_size);
	xmlSetProp (node, "description", template->description);

	xml_add_label (template, node, ns);

	for ( p=template->name->next; p != NULL; p=p->next ) {
		xml_add_alias( p->data, node, ns );
	}
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Sheet->Label Node.                                     */
/*--------------------------------------------------------------------------*/
static void
xml_add_label (glTemplate *template,
	       xmlNodePtr root,
	       xmlNsPtr ns)
{
	xmlNodePtr node;
	gchar *string;

	node = xmlNewChild(root, ns, "Label", NULL);
	string = g_strdup_printf ("%g", template->label_margin);
	xmlSetProp (node, "margin", string);
	g_free (string);
	switch (template->style) {
	case GL_TEMPLATE_STYLE_RECT:
		xmlSetProp (node, "style", "rectangle");
		string = g_strdup_printf ("%g", template->label_width);
		xmlSetProp (node, "width", string);
		g_free (string);
		string = g_strdup_printf ("%g", template->label_height);
		xmlSetProp (node, "height", string);
		g_free (string);
		string = g_strdup_printf ("%g", template->label_round);
		xmlSetProp (node, "round", string);
		g_free (string);
		break;
	case GL_TEMPLATE_STYLE_ROUND:
		xmlSetProp (node, "style", "round");
		string = g_strdup_printf ("%g", template->label_radius);
		xmlSetProp (node, "radius", string);
		g_free (string);
		break;
	case GL_TEMPLATE_STYLE_CD:
		xmlSetProp (node, "style", "cd");
		string = g_strdup_printf ("%g", template->label_radius);
		xmlSetProp (node, "radius", string);
		g_free (string);
		string = g_strdup_printf ("%g", template->label_hole);
		xmlSetProp (node, "hole", string);
		g_free (string);
		break;
	default:
		WARN ("Unknown label style");
		break;
	}

	xml_add_layout (template, node, ns);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Add XML Sheet->Label->Layout Node.                             */
/*--------------------------------------------------------------------------*/
static void
xml_add_layout (glTemplate *template,
		xmlNodePtr root,
		xmlNsPtr ns)
{
	xmlNodePtr node;
	gchar *string;

	node = xmlNewChild(root, ns, "Layout", NULL);
	string = g_strdup_printf ("%d", template->nx);
	xmlSetProp (node, "nx", string);
	g_free (string);
	string = g_strdup_printf ("%d", template->ny);
	xmlSetProp (node, "ny", string);
	g_free (string);
	string = g_strdup_printf ("%g", template->x0);
	xmlSetProp (node, "x0", string);
	g_free (string);
	string = g_strdup_printf ("%g", template->y0);
	xmlSetProp (node, "y0", string);
	g_free (string);
	string = g_strdup_printf ("%g", template->dx);
	xmlSetProp (node, "dx", string);
	g_free (string);
	string = g_strdup_printf ("%g", template->dy);
	xmlSetProp (node, "dy", string);
	g_free (string);
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

	node = xmlNewChild (root, ns, "Alias", NULL);
	xmlSetProp (node, "name", name);
}

