/*
 *  (LIBGLABELS) Template library for GLABELS
 *
 *  xml.h:  GLabels xml utilities header file
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

#ifndef __XML_H__
#define __XML_H__

#include <glib/gtypes.h>
#include <libxml/tree.h>

#include "enums.h"

#define GL_XML_NAME_SPACE "http://snaught.com/glabels/2.0/"

G_BEGIN_DECLS

/*
 * Get property functions
 */
gchar *  gl_xml_get_prop_string   (xmlNodePtr    node,
				   const gchar  *property,
				   const gchar  *default_val);

gchar *  gl_xml_get_prop_i18n_string (xmlNodePtr    node,
				   const gchar  *property,
				   const gchar  *default_val);

gdouble  gl_xml_get_prop_double   (xmlNodePtr    node,
				   const gchar  *property,
				   gdouble       default_val);

gboolean gl_xml_get_prop_boolean  (xmlNodePtr    node,
				   const gchar  *property,
				   gboolean      default_val);

gint     gl_xml_get_prop_int      (xmlNodePtr    node,
				   const gchar  *property,
				   gint          default_val);

guint    gl_xml_get_prop_uint     (xmlNodePtr    node,
				   const gchar  *property,
				   guint         default_val);

gdouble  gl_xml_get_prop_length   (xmlNodePtr    node,
				   const gchar  *property,
				   gdouble       default_val);


/*
 * Set property functions
 */
void     gl_xml_set_prop_string   (xmlNodePtr    node,
				   const gchar  *property,
				   const gchar  *val);

void     gl_xml_set_prop_double   (xmlNodePtr    node,
				   const gchar  *property,
				   gdouble       val);

void     gl_xml_set_prop_boolean  (xmlNodePtr    node,
				   const gchar  *property,
				   gboolean      val);

void     gl_xml_set_prop_int      (xmlNodePtr    node,
				   const gchar  *property,
				   gint          val);

void     gl_xml_set_prop_uint_hex (xmlNodePtr    node,
				   const gchar  *property,
				   guint         val);

void     gl_xml_set_prop_length   (xmlNodePtr    node,
				   const gchar  *property,
				   gdouble       val);

/*
 * Other node functions
 */
gboolean gl_xml_is_node           (xmlNodePtr    node,
				   const gchar  *name);

gchar *  gl_xml_get_node_content  (xmlNodePtr    node);

/*
 * Misc functions
 */
void     gl_xml_set_default_units (glUnitsType   units);

G_END_DECLS


#endif /* __XML_H__ */
