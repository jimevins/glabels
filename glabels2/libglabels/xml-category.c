/*
 *  (LIBGLABELS) Template library for GLABELS
 *
 *  xml-category.c:  category xml module
 *
 *  Copyright (C) 2006  Jim Evins <evins@snaught.com>.
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

#include "xml-category.h"

#include <glib/gi18n.h>
#include <glib/gmessages.h>
#include <string.h>
#include <libintl.h>

#include "libglabels-private.h"

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


/**
 * gl_xml_category_read_categories_from_file:
 * @utf8_filename:       Filename of categories file (name encoded as UTF-8)
 *
 * Read category definitions from a file.
 *
 * Returns: a list of #glCategory structures.
 *
 */
GList *
gl_xml_category_read_categories_from_file (gchar *utf8_filename)
{
	gchar      *filename;
	GList      *categories;
	xmlDocPtr   categories_doc;

	LIBXML_TEST_VERSION;

	filename = g_filename_from_utf8 (utf8_filename, -1, NULL, NULL, NULL);
	if (!filename) {
		g_message ("Utf8 filename conversion error");
		return NULL;
	}

	categories_doc = xmlParseFile (filename);
	if (!categories_doc) {
		g_message ("\"%s\" is not a glabels category file (not XML)",
			   filename);
		return NULL;
	}

	categories = gl_xml_category_parse_categories_doc (categories_doc);

	g_free (filename);
	xmlFreeDoc (categories_doc);

	return categories;
}


/**
 * gl_xml_category_parse_categories_doc:
 * @categories_doc:  libxml #xmlDocPtr tree, representing a categories
 * definition file.
 *
 * Read category definitions from a libxml #xmlDocPtr tree.
 *
 * Returns: a list of #glCategory structures.
 *
 */
GList *
gl_xml_category_parse_categories_doc (xmlDocPtr  categories_doc)
{
	GList      *categories = NULL;
	xmlNodePtr  root, node;
	glCategory *category;

	LIBXML_TEST_VERSION;

	root = xmlDocGetRootElement (categories_doc);
	if (!root || !root->name) {
		g_message ("\"%s\" is not a glabels category file (no root node)",
			   categories_doc->name);
		xmlFreeDoc (categories_doc);
		return categories;
	}
	if (!gl_xml_is_node (root, "Glabels-categories")) {
		g_message ("\"%s\" is not a glabels category file (wrong root node)",
			   categories_doc->name);
		xmlFreeDoc (categories_doc);
		return categories;
	}

	for (node = root->xmlChildrenNode; node != NULL; node = node->next) {

		if (gl_xml_is_node (node, "Category")) {
			category = gl_xml_category_parse_category_node (node);
			categories = g_list_append (categories, category);
		} else {
			if ( !xmlNodeIsText(node) ) {
				if (!gl_xml_is_node (node, "comment")) {
					g_message ("bad node =  \"%s\"",node->name);
				}
			}
		}
	}

	return categories;
}


/**
 * gl_xml_category_parse_category_node:
 * @category_node:  libxml #xmlNodePtr category node from a #xmlDocPtr tree.
 *
 * Read a single category definition from a libxml #xmlNodePtr node.
 *
 * Returns: a pointer to a newly created #glCategory structure.
 *
 */
glCategory *
gl_xml_category_parse_category_node (xmlNodePtr category_node)
{
	glCategory            *category;
	gchar                 *id, *name;

	LIBXML_TEST_VERSION;

	id   = gl_xml_get_prop_string (category_node, "id", NULL);
	name = gl_xml_get_prop_i18n_string (category_node, "name", NULL);

	category = gl_category_new (id, name);

	g_free (id);
	g_free (name);

	return category;
}

