/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  xml.h:  GLabels xml utilities header file
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
#ifndef __XML_H__
#define __XML_H__

#include <glib.h>
#include <libxml/tree.h>

G_BEGIN_DECLS

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

G_END_DECLS


#endif /* __XML_H__ */
