/*
 *  lgl-xml-paper.c
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

#include "lgl-xml-paper.h"

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
 * lgl_xml_paper_read_papers_from_file:
 * @utf8_filename:       Filename of papers file (name encoded as UTF-8)
 *
 * Read paper definitions from a file.
 *
 * Returns: a list of #lglPaper structures.
 *
 */
GList *
lgl_xml_paper_read_papers_from_file (gchar *utf8_filename)
{
	gchar      *filename;
	GList      *papers;
	xmlDocPtr   papers_doc;

	LIBXML_TEST_VERSION;

	filename = g_filename_from_utf8 (utf8_filename, -1, NULL, NULL, NULL);
	if (!filename) {
		g_message ("Utf8 filename conversion error");
		return NULL;
	}

	papers_doc = xmlParseFile (filename);
	if (!papers_doc) {
		g_message ("\"%s\" is not a glabels paper file (not XML)",
			   filename);
		return NULL;
	}

	papers = lgl_xml_paper_parse_papers_doc (papers_doc);

	g_free (filename);
	xmlFreeDoc (papers_doc);

	return papers;
}


/**
 * lgl_xml_paper_parse_papers_doc:
 * @papers_doc:  libxml #xmlDocPtr tree, representing a papers definition file.
 *
 * Read paper definitions from a libxml #xmlDocPtr tree.
 *
 * Returns: a list of #lglPaper structures.
 *
 */
GList *
lgl_xml_paper_parse_papers_doc (xmlDocPtr  papers_doc)
{
	GList      *papers = NULL;
	xmlNodePtr  root, node;
	lglPaper   *paper;

	LIBXML_TEST_VERSION;

	root = xmlDocGetRootElement (papers_doc);
	if (!root || !root->name) {
		g_message ("\"%s\" is not a glabels paper file (no root node)",
			   papers_doc->name);
		xmlFreeDoc (papers_doc);
		return papers;
	}
	if (!lgl_xml_is_node (root, "Glabels-paper-sizes")) {
		g_message ("\"%s\" is not a glabels paper file (wrong root node)",
			   papers_doc->name);
		xmlFreeDoc (papers_doc);
		return papers;
	}

	for (node = root->xmlChildrenNode; node != NULL; node = node->next) {

		if (lgl_xml_is_node (node, "Paper-size")) {
			paper = lgl_xml_paper_parse_paper_node (node);
			papers = g_list_append (papers, paper);
		} else {
			if ( !xmlNodeIsText(node) ) {
				if (!lgl_xml_is_node (node, "comment")) {
					g_message ("bad node =  \"%s\"",node->name);
				}
			}
		}
	}

	return papers;
}


/**
 * lgl_xml_paper_parse_paper_node:
 * @paper_node:  libxml #xmlNodePtr paper node from a #xmlDocPtr tree.
 *
 * Read a single paper definition from a libxml #xmlNodePtr node.
 *
 * Returns: a pointer to a newly created #lglPaper structure.
 *
 */
lglPaper *
lgl_xml_paper_parse_paper_node (xmlNodePtr paper_node)
{
	lglPaper              *paper;
	gchar                 *id, *name, *pwg_size;
	gdouble                width, height;

	LIBXML_TEST_VERSION;

	id   = lgl_xml_get_prop_string (paper_node, "id", NULL);

	name = lgl_xml_get_prop_i18n_string (paper_node, "name", NULL);

	width  = lgl_xml_get_prop_length (paper_node, "width", 0);
	height = lgl_xml_get_prop_length (paper_node, "height", 0);

	pwg_size = lgl_xml_get_prop_string (paper_node, "pwg_size", NULL);

	paper = lgl_paper_new (id, name, width, height, pwg_size);

	g_free (id);
	g_free (name);
	g_free (pwg_size);

	return paper;
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
