/*
 *  (LIBGLABELS) Template library for GLABELS
 *
 *  xml-paper.h:  paper xml module header file
 *
 *  Copyright (C) 2003, 2004  Jim Evins <evins@snaught.com>.
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

#ifndef __XML_PAPER_H__
#define __XML_PAPER_H__

#include <glib.h>
#include <libxml/tree.h>

#include "paper.h"

G_BEGIN_DECLS

GList       *gl_xml_paper_read_papers_from_file (gchar        *utf8_filename);

GList       *gl_xml_paper_parse_papers_doc      (xmlDocPtr     papers_doc);

glPaper     *gl_xml_paper_parse_paper_node      (xmlNodePtr    paper_node);


G_END_DECLS

#endif /* __XML_PAPER_H__ */
