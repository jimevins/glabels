/*
 *  xml-paper.h
 *  Copyright (C) 2003-2009  Jim Evins <evins@snaught.com>.
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

#ifndef __XML_PAPER_H__
#define __XML_PAPER_H__

#include <glib.h>
#include <libxml/tree.h>

#include "paper.h"

G_BEGIN_DECLS

GList       *lgl_xml_paper_read_papers_from_file (gchar        *utf8_filename);

GList       *lgl_xml_paper_parse_papers_doc      (xmlDocPtr     papers_doc);

lglPaper    *lgl_xml_paper_parse_paper_node      (xmlNodePtr    paper_node);


G_END_DECLS

#endif /* __XML_PAPER_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
