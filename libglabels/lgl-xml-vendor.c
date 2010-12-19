/*
 *  lgl-xml-vendor.c
 *  Copyright (C) 2003-2010  Jim Evins <evins@snaught.com>.
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

#include "lgl-xml-vendor.h"

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
 * lgl_xml_vendor_read_vendors_from_file:
 * @utf8_filename:       Filename of vendors file (name encoded as UTF-8)
 *
 * Read vendor definitions from a file.
 *
 * Returns: a list of #lglVendor structures.
 *
 */
GList *
lgl_xml_vendor_read_vendors_from_file (gchar *utf8_filename)
{
	gchar      *filename;
	GList      *vendors;
	xmlDocPtr   vendors_doc;

	LIBXML_TEST_VERSION;

	filename = g_filename_from_utf8 (utf8_filename, -1, NULL, NULL, NULL);
	if (!filename) {
		g_message ("Utf8 filename conversion error");
		return NULL;
	}

	vendors_doc = xmlParseFile (filename);
	if (!vendors_doc) {
		g_message ("\"%s\" is not a glabels vendor file (not XML)",
			   filename);
		return NULL;
	}

	vendors = lgl_xml_vendor_parse_vendors_doc (vendors_doc);

	g_free (filename);
	xmlFreeDoc (vendors_doc);

	return vendors;
}


/**
 * lgl_xml_vendor_parse_vendors_doc:
 * @vendors_doc:  libxml #xmlDocPtr tree, representing a vendors definition file.
 *
 * Read vendor definitions from a libxml #xmlDocPtr tree.
 *
 * Returns: a list of #lglVendor structures.
 *
 */
GList *
lgl_xml_vendor_parse_vendors_doc (xmlDocPtr  vendors_doc)
{
	GList       *vendors = NULL;
	xmlNodePtr   root, node;
	lglVendor   *vendor;

	LIBXML_TEST_VERSION;

	root = xmlDocGetRootElement (vendors_doc);
	if (!root || !root->name) {
		g_message ("\"%s\" is not a glabels vendor file (no root node)",
			   vendors_doc->name);
		xmlFreeDoc (vendors_doc);
		return vendors;
	}
	if (!lgl_xml_is_node (root, "Glabels-vendors")) {
		g_message ("\"%s\" is not a glabels vendor file (wrong root node)",
			   vendors_doc->name);
		xmlFreeDoc (vendors_doc);
		return vendors;
	}

	for (node = root->xmlChildrenNode; node != NULL; node = node->next) {

		if (lgl_xml_is_node (node, "Vendor")) {
			vendor = lgl_xml_vendor_parse_vendor_node (node);
			vendors = g_list_append (vendors, vendor);
		} else {
			if ( !xmlNodeIsText(node) ) {
				if (!lgl_xml_is_node (node, "comment")) {
					g_message ("bad node =  \"%s\"",node->name);
				}
			}
		}
	}

	return vendors;
}


/**
 * lgl_xml_vendor_parse_vendor_node:
 * @vendor_node:  libxml #xmlNodePtr vendor node from a #xmlDocPtr tree.
 *
 * Read a single vendor definition from a libxml #xmlNodePtr node.
 *
 * Returns: a pointer to a newly created #lglVendor structure.
 *
 */
lglVendor *
lgl_xml_vendor_parse_vendor_node (xmlNodePtr vendor_node)
{
	lglVendor             *vendor;
	gchar                 *name;

	LIBXML_TEST_VERSION;

	name = lgl_xml_get_prop_i18n_string (vendor_node, "name", NULL);

	vendor = lgl_vendor_new (name);

        vendor->url = lgl_xml_get_prop_i18n_string (vendor_node, "url", NULL);

	g_free (name);

	return vendor;
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
