/*
 *  (LIBGLABELS) Template library for GLABELS
 *
 *  xml-template.h:  template xml module header file
 *
 *  Copyright (C) 2001-2004  Jim Evins <evins@snaught.com>.
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

#ifndef __XML_TEMPLATE_H__
#define __XML_TEMPLATE_H__

#include <glib.h>
#include <libxml/tree.h>

#include "template.h"

G_BEGIN_DECLS

GList       *gl_xml_template_read_templates_from_file (const gchar       *utf8_filename);

GList       *gl_xml_template_parse_templates_doc      (const xmlDocPtr    templates_doc);

glTemplate  *gl_xml_template_parse_template_node      (const xmlNodePtr   template_node);


void         gl_xml_template_write_templates_to_file  (GList             *templates,
						       const gchar       *utf8_filename);

void         gl_xml_template_write_template_to_file   (const glTemplate  *template,
						       const gchar       *utf8_filename);

xmlDocPtr    gl_xml_template_create_templates_doc     (GList             *templates);

void         gl_xml_template_create_template_node     (const glTemplate  *template,
						       xmlNodePtr         root,
						       const xmlNsPtr     ns);

G_END_DECLS

#endif /* __XML_TEMPLATE_H__ */
