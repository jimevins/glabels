/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  xml-paper.c:  paper xml module
 *
 *  Copyright (C) 2003  Jim Evins <evins@snaught.com>.
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

#include "xml.h"
#include "xml-paper.h"

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

glPaper *gl_xml_paper_parse_paper (xmlNodePtr paper_node);



/*****************************************************************************/
/* Read papers from paper file.                                        */
/*****************************************************************************/
GList *
gl_xml_paper_read_papers_from_file (GList *papers,
				    gchar *xml_filename)
{
	xmlDocPtr   doc;
	xmlNodePtr  root, node;
	glPaper *paper;

	gl_debug (DEBUG_PAPER, "START");

	LIBXML_TEST_VERSION;

	doc = xmlParseFile (xml_filename);
	if (!doc) {
		g_warning ("\"%s\" is not a glabels paper file (not XML)",
		      xml_filename);
		return papers;
	}

	root = xmlDocGetRootElement (doc);
	if (!root || !root->name) {
		g_warning ("\"%s\" is not a glabels paper file (no root node)",
		      xml_filename);
		xmlFreeDoc (doc);
		return papers;
	}
	if (!xmlStrEqual (root->name, "Glabels-paper-sizes")) {
		g_warning ("\"%s\" is not a glabels paper file (wrong root node)",
		      xml_filename);
		xmlFreeDoc (doc);
		return papers;
	}

	for (node = root->xmlChildrenNode; node != NULL; node = node->next) {

		if (xmlStrEqual (node->name, "Paper-size")) {
			paper = gl_xml_paper_parse_paper (node);
			papers = g_list_append (papers, paper);
		} else {
			if ( !xmlNodeIsText(node) ) {
				if (!xmlStrEqual (node->name,"comment")) {
					g_warning ("bad node =  \"%s\"",node->name);
				}
			}
		}
	}

	xmlFreeDoc (doc);

	gl_debug (DEBUG_PAPER, "END");
	return papers;
}

/*****************************************************************************/
/* Parse XML paper Node.                                                  */
/*****************************************************************************/
glPaper *
gl_xml_paper_parse_paper (xmlNodePtr paper_node)
{
	glPaper               *paper;
	gchar                 *name;

	gl_debug (DEBUG_PAPER, "START");

	paper = g_new0 (glPaper, 1);

	paper->id   = xmlGetProp (paper_node, "id");

	name = xmlGetProp (paper_node, "_name");
	if (name != NULL) {
		paper->name = gettext (name);
	} else {
		paper->name = xmlGetProp (paper_node, "name");
	}

	paper->width  = gl_xml_get_prop_length (paper_node, "width", 0);
	paper->height = gl_xml_get_prop_length (paper_node, "height", 0);

	gl_debug (DEBUG_PAPER, "END");

	return paper;
}

