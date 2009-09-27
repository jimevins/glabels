/*
 *  (LIBGLABELS) Template library for GLABELS
 *
 *  xml-category.h:  category xml module header file
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

#ifndef __XML_CATEGORY_H__
#define __XML_CATEGORY_H__

#include <glib/gtypes.h>
#include <glib/glist.h>
#include <libxml/tree.h>

#include "category.h"

G_BEGIN_DECLS

GList       *lgl_xml_category_read_categories_from_file (gchar        *utf8_filename);

GList       *lgl_xml_category_parse_categories_doc      (xmlDocPtr     categories_doc);

lglCategory *lgl_xml_category_parse_category_node       (xmlNodePtr    category_node);


G_END_DECLS

#endif /* __XML_CATEGORY_H__ */
