/*
 *  (LIBGLABELS) Template library for GLABELS
 *
 *  xml.c:  GLabels xml utilities module
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

#include "libglabels-private.h"

#include "xml.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

#define POINTS_PER_POINT    1.0 /* internal units are points. */
#define POINTS_PER_INCH    72.0
#define POINTS_PER_MM       2.83464566929
#define POINTS_PER_CM       (10.0*POINTS_PER_MM)
#define POINTS_PER_PICA     (1.0/12.0)

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

typedef struct {
	gchar       *name;
	gdouble      points_per_unit;
} UnitTableEntry;

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static UnitTableEntry unit_table[] = {

	/* These names are identical to the absolute length units supported in
	   the CSS2 Specification (Section 4.3.2) */

	/* This table must be sorted exactly as the enumerations in glUnitsType */

	/* [GL_UNITS_POINT] */   {"pt",      POINTS_PER_POINT},
	/* [GL_UNITS_INCH]  */   {"in",      POINTS_PER_INCH},
	/* [GL_UNITS_MM]    */   {"mm",      POINTS_PER_MM},
	/* [GL_UNITS_CM]    */   {"cm",      POINTS_PER_CM},
	/* [GL_UNITS_PICA]  */   {"pc",      POINTS_PER_PICA},

};

static glUnitsType  default_units        = GL_UNITS_POINT;

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
		val = strtol (string, NULL, 0);
		g_free (string);
		return val;
	}

	return default_val;
}


/****************************************************************************/
/* Return value of hex property as an unsigned int.                         */
/****************************************************************************/
guint
gl_xml_get_prop_uint (xmlNodePtr   node,
		      const gchar *property,
		      guint        default_val)
{
	guint    val;
	gchar   *string;

	string = xmlGetProp (node, property);
	if ( string != NULL ) {
		val = strtoul (string, NULL, 0);
		g_free (string);
		return val;
	}

	return default_val;
}


/****************************************************************************/
/* Return value of length property as a double, converting to internal units*/
/****************************************************************************/
gdouble
gl_xml_get_prop_length (xmlNodePtr   node,
			const gchar *property,
			gdouble      default_val)
{
	gdouble  val;
	gchar   *string, *unit;
	gint     i;

	string = xmlGetProp (node, property);
	if ( string != NULL ) {

		val = g_strtod (string, &unit);

		if (unit != string) {
			unit = g_strchug (unit);
			if (strlen (unit) > 0 ) {
				for (i=GL_UNITS_FIRST; i<=GL_UNITS_LAST; i++) {
					if (xmlStrcasecmp (unit, unit_table[i].name) == 0) {
						val *= unit_table[i].points_per_unit;
						break;
					}
				}
				if (i>GL_UNITS_LAST) {
					g_warning ("Line %d, Node \"%s\", Property \"%s\": Unknown unit \"%s\", assuming points",
						   xmlGetLineNo (node), node->name, property,
						   unit);
				}
			}
		}
		else {
			val = 0.0;
		}

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
	gchar  *string, buffer[G_ASCII_DTOSTR_BUF_SIZE];

	/* Guarantee "C" locale by use of g_ascii_formatd */
	string = g_ascii_formatd (buffer, G_ASCII_DTOSTR_BUF_SIZE, "%g", val);

	xmlSetProp (node, property, string);
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

/****************************************************************************/
/* Set property from length.                                                */
/****************************************************************************/
void
gl_xml_set_prop_length (xmlNodePtr    node,
			const gchar  *property,
			gdouble       val)
{
	gchar  *string, buffer[G_ASCII_DTOSTR_BUF_SIZE];
	gchar  *string_unit;

	/* Convert to default units */
	val /= unit_table[default_units].points_per_unit;

	/* Guarantee "C" locale by use of g_ascii_formatd */
	string = g_ascii_formatd (buffer, G_ASCII_DTOSTR_BUF_SIZE, "%g", val);

	string_unit = g_strdup_printf ("%s%s", string, unit_table[default_units].name);
	xmlSetProp (node, property, string_unit);
        g_free (string_unit);
}

/****************************************************************************/
/* Set default length units.                                                */
/****************************************************************************/
void
gl_xml_set_default_units (glUnitsType   units)
{
	g_return_if_fail ((units >= GL_UNITS_FIRST) && (units <= GL_UNITS_LAST));

	default_units = units;
}


