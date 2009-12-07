/*
 *  xml-vendor.h
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

#ifndef __LGL_XML_VENDOR_H__
#define __LGL_XML_VENDOR_H__

#include <glib.h>
#include <libxml/tree.h>

#include "vendor.h"

G_BEGIN_DECLS

GList       *lgl_xml_vendor_read_vendors_from_file (gchar        *utf8_filename);

GList       *lgl_xml_vendor_parse_vendors_doc      (xmlDocPtr     vendors_doc);

lglVendor   *lgl_xml_vendor_parse_vendor_node      (xmlNodePtr    vendor_node);


G_END_DECLS

#endif /* __LGL_XML_VENDOR_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
