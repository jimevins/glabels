/*
 *  lgl-xml-template.h
 *  Copyright (C) 2001-2010  Jim Evins <evins@snaught.com>.
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

#ifndef __LGL_XML_TEMPLATE_H__
#define __LGL_XML_TEMPLATE_H__

#include <glib.h>
#include <libxml/tree.h>

#include "lgl-template.h"

G_BEGIN_DECLS

void         lgl_xml_template_read_templates_from_file (const gchar       *utf8_filename);

void         lgl_xml_template_parse_templates_doc      (const xmlDocPtr    templates_doc);

lglTemplate *lgl_xml_template_parse_template_node      (const xmlNodePtr   template_node);


gint         lgl_xml_template_write_templates_to_file  (GList             *templates,
							const gchar       *utf8_filename);

gint         lgl_xml_template_write_template_to_file   (const lglTemplate *template,
							const gchar       *utf8_filename);

void         lgl_xml_template_create_template_node     (const lglTemplate *template,
							xmlNodePtr         root,
							const xmlNsPtr     ns);

G_END_DECLS

#endif /* __LGL_XML_TEMPLATE_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
