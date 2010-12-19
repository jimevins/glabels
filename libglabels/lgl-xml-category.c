/*
 *  lgl-xml-category.c
 *  Copyright (C) 2006-2010  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of libglabels.
 *
 *  libglabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libglabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libglabels.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "lgl-xml-category.h"

#include <glib/gi18n.h>
#include <glib.h>
#include <string.h>
#include <libintl.h>

#include "libglabels-private.h"

#include "lgl-xml.h"

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
 * lgl_xml_category_read_categories_from_file:
 * @utf8_filename:       Filename of categories file (name encoded as UTF-8)
 *
 * Read category definitions from a file.
 *
 * Returns: a list of #lglCategory structures.
 *
 */
GList *
lgl_xml_category_read_categories_from_file (gchar *utf8_filename)
{
        gchar      *filename;
        GList      *categories;
        xmlDocPtr   categories_doc;

        LIBXML_TEST_VERSION;

        filename = g_filename_from_utf8 (utf8_filename, -1, NULL, NULL, NULL);
        if (!filename)
        {
                g_message ("Utf8 filename conversion error");
                return NULL;
        }

        categories_doc = xmlParseFile (filename);
        if (!categories_doc)
        {
                g_message ("\"%s\" is not a glabels category file (not XML)",
                           filename);
                return NULL;
        }

        categories = lgl_xml_category_parse_categories_doc (categories_doc);

        g_free (filename);
        xmlFreeDoc (categories_doc);

        return categories;
}


/**
 * lgl_xml_category_parse_categories_doc:
 * @categories_doc:  libxml #xmlDocPtr tree, representing a categories
 * definition file.
 *
 * Read category definitions from a libxml #xmlDocPtr tree.
 *
 * Returns: a list of #lglCategory structures.
 *
 */
GList *
lgl_xml_category_parse_categories_doc (xmlDocPtr  categories_doc)
{
        GList       *categories = NULL;
        xmlNodePtr   root, node;
        lglCategory *category;

        LIBXML_TEST_VERSION;

        root = xmlDocGetRootElement (categories_doc);
        if (!root || !root->name)
        {
                g_message ("\"%s\" is not a glabels category file (no root node)",
                           categories_doc->name);
                xmlFreeDoc (categories_doc);
                return categories;
        }
        if (!lgl_xml_is_node (root, "Glabels-categories"))
        {
                g_message ("\"%s\" is not a glabels category file (wrong root node)",
                           categories_doc->name);
                xmlFreeDoc (categories_doc);
                return categories;
        }

        for (node = root->xmlChildrenNode; node != NULL; node = node->next)
        {

                if (lgl_xml_is_node (node, "Category"))
                {
                        category = lgl_xml_category_parse_category_node (node);
                        categories = g_list_append (categories, category);
                }
                else
                {
                        if ( !xmlNodeIsText(node) )
                        {
                                if (!lgl_xml_is_node (node, "comment"))
                                {
                                        g_message ("bad node =  \"%s\"",node->name);
                                }
                        }
                }
        }

        return categories;
}


/**
 * lgl_xml_category_parse_category_node:
 * @category_node:  libxml #xmlNodePtr category node from a #xmlDocPtr tree.
 *
 * Read a single category definition from a libxml #xmlNodePtr node.
 *
 * Returns: a pointer to a newly created #lglCategory structure.
 *
 */
lglCategory *
lgl_xml_category_parse_category_node (xmlNodePtr category_node)
{
        lglCategory           *category;
        gchar                 *id, *name;

        LIBXML_TEST_VERSION;

        id   = lgl_xml_get_prop_string (category_node, "id", NULL);
        name = lgl_xml_get_prop_i18n_string (category_node, "name", NULL);

        category = lgl_category_new (id, name);

        g_free (id);
        g_free (name);

        return category;
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
