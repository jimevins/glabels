/*
 *  xml.h
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

#ifndef __XML_H__
#define __XML_H__

#include <glib.h>
#include <libxml/tree.h>

#include "units.h"

#define LGL_XML_NAME_SPACE "http://snaught.com/glabels/2.2/"

G_BEGIN_DECLS

/*
 * Get property functions
 */
gchar *  lgl_xml_get_prop_string   (xmlNodePtr    node,
				    const gchar  *property,
				    const gchar  *default_val);

gchar *  lgl_xml_get_prop_i18n_string (xmlNodePtr    node,
				       const gchar  *property,
				       const gchar  *default_val);

gdouble  lgl_xml_get_prop_double   (xmlNodePtr    node,
				    const gchar  *property,
				    gdouble       default_val);

gboolean lgl_xml_get_prop_boolean  (xmlNodePtr    node,
				    const gchar  *property,
				    gboolean      default_val);

gint     lgl_xml_get_prop_int      (xmlNodePtr    node,
				    const gchar  *property,
				    gint          default_val);

guint    lgl_xml_get_prop_uint     (xmlNodePtr    node,
				    const gchar  *property,
				    guint         default_val);

gdouble  lgl_xml_get_prop_length   (xmlNodePtr    node,
				    const gchar  *property,
				    gdouble       default_val);


/*
 * Set property functions
 */
void     lgl_xml_set_prop_string   (xmlNodePtr    node,
				    const gchar  *property,
				    const gchar  *val);

void     lgl_xml_set_prop_double   (xmlNodePtr    node,
				    const gchar  *property,
				    gdouble       val);

void     lgl_xml_set_prop_boolean  (xmlNodePtr    node,
				    const gchar  *property,
				    gboolean      val);

void     lgl_xml_set_prop_int      (xmlNodePtr    node,
				    const gchar  *property,
				    gint          val);

void     lgl_xml_set_prop_uint_hex (xmlNodePtr    node,
				    const gchar  *property,
				    guint         val);

void     lgl_xml_set_prop_length   (xmlNodePtr    node,
				    const gchar  *property,
				    gdouble       val);

/*
 * Other node functions
 */
gboolean lgl_xml_is_node           (xmlNodePtr    node,
				    const gchar  *name);

gchar *  lgl_xml_get_node_content  (xmlNodePtr    node);

/*
 * Misc functions
 */
void     lgl_xml_set_default_units (lglUnits      units);

G_END_DECLS


#endif /* __XML_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
