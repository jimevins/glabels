/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  xml.c:  GLabels xml utilities module
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
#include <config.h>

#include "xml.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/


/****************************************************************************/
/* Return value of property as a double.                                    */
/****************************************************************************/
gdouble
gl_xml_get_prop_double (xmlNodePtr   node,
			const gchar *property,
			gdouble      default_val)
{
	gdouble  val;
	gchar   *string;

	string = xmlGetProp (node, property);
	if ( string != NULL ) {
		val = g_strtod (string, NULL);
		g_free (string);
		return val;
	}

	return default_val;
}

/****************************************************************************/
/* Return value of property as a boolean.                                   */
/****************************************************************************/
gboolean
gl_xml_get_prop_boolean (xmlNodePtr   node,
			 const gchar *property,
			 gboolean     default_val)
{
	gboolean  val;
	gchar    *string;

	string = xmlGetProp (node, property);
	if ( string != NULL ) {
		val = !((xmlStrcasecmp (string, "false") == 0) ||
			xmlStrEqual (string, "0"));;
		g_free (string);
		return val;
	}

	return default_val;
}


/****************************************************************************/
/* Return value of property as an int. .                                    */
/****************************************************************************/
gint
gl_xml_get_prop_int (xmlNodePtr   node,
		     const gchar *property,
		     gint         default_val)
{
	gint     val;
	gchar   *string;

	string = xmlGetProp (node, property);
	if ( string != NULL ) {
		sscanf (string, "%d", &val);
		g_free (string);
		return val;
	}

	return default_val;
}


/****************************************************************************/
/* Return value of hex property as an unsigned int.                         */
/****************************************************************************/
guint
gl_xml_get_prop_uint_hex (xmlNodePtr   node,
			  const gchar *property,
			  guint        default_val)
{
	guint    val;
	gchar   *string;

	string = xmlGetProp (node, property);
	if ( string != NULL ) {
		sscanf (string, "%x", &val);
		g_free (string);
		return val;
	}

	return default_val;
}


/****************************************************************************/
/* Set property from double.                                                */
/****************************************************************************/
void
gl_xml_set_prop_double (xmlNodePtr    node,
			const gchar  *property,
			gdouble       val)
{
	gchar  *string;

	string = g_strdup_printf ("%g", val);
	xmlSetProp (node, property, string);
	g_free (string);
}

/****************************************************************************/
/* Set property from boolean.                                               */
/****************************************************************************/
void
gl_xml_set_prop_boolean (xmlNodePtr    node,
			 const gchar  *property,
			 gboolean      val)
{
	xmlSetProp (node, property, (val ? "True" : "False"));
}

/****************************************************************************/
/* Set property from int.                                                   */
/****************************************************************************/
void
gl_xml_set_prop_int (xmlNodePtr    node,
		     const gchar  *property,
		     gint          val)
{
	gchar  *string;

	string = g_strdup_printf ("%d", val);
	xmlSetProp (node, property, string);
	g_free (string);
}

/****************************************************************************/
/* Set property from uint in hex.                                           */
/****************************************************************************/
void
gl_xml_set_prop_uint_hex (xmlNodePtr    node,
			  const gchar  *property,
			  guint         val)
{
	gchar  *string;

	string = g_strdup_printf ("0x%08x", val);
	xmlSetProp (node, property, string);
	g_free (string);
}

