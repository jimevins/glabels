/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  xml-template.h:  template xml module header file
 *
 *  Copyright (C) 2001-2003  Jim Evins <evins@snaught.com>.
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

#ifndef __XML_TEMPLATE_H__
#define __XML_TEMPLATE_H__

#include <libgnome/libgnome.h>
#include <libxml/tree.h>
#include <libxml/parser.h>

#include "template.h"

G_BEGIN_DECLS

GList       *gl_xml_template_read_templates_from_file (GList             *templates,
						       gchar             *xml_filename);

glTemplate  *gl_xml_template_parse_sheet              (xmlNodePtr         sheet_node);

void         gl_xml_template_add_sheet                (const glTemplate  *template,
						       xmlNodePtr         root,
						       xmlNsPtr           ns);

G_END_DECLS

#endif /* __XML_TEMPLATE_H__ */
