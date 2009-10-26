/*
 *  xml.c
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

#include <config.h>

#include "xml.h"

#include <glib/gi18n.h>
#include <glib.h>
#include <string.h>

#include "libglabels-private.h"


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static lglUnits  default_units        = LGL_UNITS_POINT;


/****************************************************************************/

/**
 * lgl_xml_get_prop_string:
 * @node:        the libxml2 #xmlNodePtr of the node
 * @property:    the property name
 * @default_val: a default value to return if property not found
 *
 * Return value of property as a string.
 *
 * Returns: the property as a pointer to a gchar string.  This string should
 *          be freed with g_free().
 *
 */
gchar *
lgl_xml_get_prop_string (xmlNodePtr   node,
			 const gchar *property,
			 const gchar *default_val)
{
	gchar   *val;
	xmlChar *string;

	string = xmlGetProp (node, (xmlChar *)property);
	if ( string != NULL ) {
		val = g_strdup ((gchar *)string);
		xmlFree (string);
		return val;
	}

	if (default_val) {
		return g_strdup (default_val);
	}

	return NULL;
}


/**
 * lgl_xml_get_prop_i18n_string:
 * @node:        the libxml2 #xmlNodePtr of the node
 * @property:    the property name
 * @default_val: a default value to return if property not found
 *
 * Return value of a translatable property as a string.
 *
 * Returns: the property as a pointer to a gchar string.  This string should
 *          be freed with g_free().
 *
 */
gchar *
lgl_xml_get_prop_i18n_string (xmlNodePtr   node,
			      const gchar *property,
			      const gchar *default_val)
{
	gchar   *_property;
	gchar   *val;
	xmlChar *string;

	_property = g_strdup_printf ("_%s", property);
	string = xmlGetProp (node, (xmlChar *)_property);
	g_free (_property);

	if ( string != NULL ) {

		val = g_strdup (gettext ((char *)string));
		xmlFree (string);
		return val;

	}

	string = xmlGetProp (node, (xmlChar *)property);
	if ( string != NULL ) {
		val = g_strdup ((gchar *)string);
		xmlFree (string);
		return val;
	}

	if (default_val) {
		return g_strdup (default_val);
	}

	return NULL;
}


/**
 * lgl_xml_get_prop_double:
 * @node:        the libxml2 #xmlNodePtr of the node
 * @property:    the property name
 * @default_val: a default value to return if property not found
 *
 * Return value of property as a double.
 *
 * Returns: the property as a double.
 *
 */
gdouble
lgl_xml_get_prop_double (xmlNodePtr   node,
			 const gchar *property,
			 gdouble      default_val)
{
	gdouble  val;
	xmlChar *string;

	string = xmlGetProp (node, (xmlChar *)property);
	if ( string != NULL ) {
		val = g_strtod ((gchar *)string, NULL);
		xmlFree (string);
		return val;
	}

	return default_val;
}


/**
 * lgl_xml_get_prop_boolean:
 * @node:        the libxml2 #xmlNodePtr of the node
 * @property:    the property name
 * @default_val: a default value to return if property not found
 *
 * Return value of property as a boolean.
 *
 * Returns: the property as a boolean.
 *
 */
gboolean
lgl_xml_get_prop_boolean (xmlNodePtr   node,
			 const gchar *property,
			 gboolean     default_val)
{
	gboolean  val;
	xmlChar  *string;

	string = xmlGetProp (node, (xmlChar *)property);
	if ( string != NULL ) {
		val = !((xmlStrcasecmp (string, (xmlChar *)"false") == 0) ||
			xmlStrEqual (string, (xmlChar *)"0"));;
		xmlFree (string);
		return val;
	}

	return default_val;
}


/**
 * lgl_xml_get_prop_int:
 * @node:        the libxml2 #xmlNodePtr of the node
 * @property:    the property name
 * @default_val: a default value to return if property not found
 *
 * Return value of property as an integer.
 *
 * Returns: the property as an integer.
 *
 */
gint
lgl_xml_get_prop_int (xmlNodePtr   node,
		      const gchar *property,
		      gint         default_val)
{
	gint     val;
	xmlChar *string;

	string = xmlGetProp (node, (xmlChar *)property);
	if ( string != NULL ) {
		val = strtol ((char *)string, NULL, 0);
		xmlFree (string);
		return val;
	}

	return default_val;
}


/**
 * lgl_xml_get_prop_uint:
 * @node:        the libxml2 #xmlNodePtr of the node
 * @property:    the property name
 * @default_val: a default value to return if property not found
 *
 * Return value of property (usually formatted in hex) as an unsigned integer.
 *
 * Returns: the property as an unsigned integer.
 *
 */
guint
lgl_xml_get_prop_uint (xmlNodePtr   node,
		       const gchar *property,
		       guint        default_val)
{
	guint    val;
	xmlChar *string;

	string = xmlGetProp (node, (xmlChar *)property);
	if ( string != NULL ) {
		val = strtoul ((char *)string, NULL, 0);
		xmlFree (string);
		return val;
	}

	return default_val;
}


/**
 * lgl_xml_get_prop_length:
 * @node:        the libxml2 #xmlNodePtr of the node
 * @property:    the property name
 * @default_val: a default value to return if property not found
 *
 * Return value of a length property as a double, converting to internal
 * units (points).  The property is expected to be formatted as a number
 * followed by a units string.  If there is no units string, the length
 * is assumed to be in points.  Valid units strings are "pt" for points,
 * "in" for inches, "mm" for millimeters, "cm" for centimeters, and
 * "pc" for picas.
 *
 * Returns: the length in points.
 *
 */
gdouble
lgl_xml_get_prop_length (xmlNodePtr   node,
			 const gchar *property,
			 gdouble      default_val)
{
	gdouble  val;
	xmlChar *string;
	xmlChar *unit_id;
        lglUnits units;

	string = xmlGetProp (node, (xmlChar *)property);
	if ( string != NULL ) {

		val = g_strtod ((gchar *)string, (gchar **)&unit_id);

		if (unit_id != string) {
			unit_id = (xmlChar *)g_strchug ((gchar *)unit_id);
                        units = lgl_units_from_id ((gchar *)unit_id);
                        if (units != LGL_UNITS_INVALID)
                        {
                                val *= lgl_units_get_points_per_unit (units);
                        }
                        else
                        {
                                g_message ("Line %ld, Node \"%s\", Property \"%s\": Unknown unit \"%s\", assuming points",
                                           xmlGetLineNo (node), node->name, property, unit_id);
                        }
		}
		else {
			val = 0.0;
		}

		xmlFree (string);
		return val;
	}

	return default_val;
}


/**
 * lgl_xml_set_prop_string:
 * @node:        the libxml2 #xmlNodePtr of the node
 * @property:    the property name
 * @val:         the value to set
 *
 * Set a property from a string.
 *
 */
void
lgl_xml_set_prop_string (xmlNodePtr    node,
			 const gchar  *property,
			 const gchar  *val)
{
	if (val != NULL) {
		xmlSetProp (node, (xmlChar *)property, (xmlChar *)val);
	}
}


/**
 * lgl_xml_set_prop_double:
 * @node:        the libxml2 #xmlNodePtr of the node
 * @property:    the property name
 * @val:         the value to set
 *
 * Set a property from a double.
 *
 */
void
lgl_xml_set_prop_double (xmlNodePtr    node,
			 const gchar  *property,
			 gdouble       val)
{
	gchar  *string, buffer[G_ASCII_DTOSTR_BUF_SIZE];

	/* Guarantee "C" locale by use of g_ascii_formatd */
	string = g_ascii_formatd (buffer, G_ASCII_DTOSTR_BUF_SIZE, "%g", val);

	xmlSetProp (node, (xmlChar *)property, (xmlChar *)string);
}


/**
 * lgl_xml_set_prop_boolean:
 * @node:        the libxml2 #xmlNodePtr of the node
 * @property:    the property name
 * @val:         the value to set
 *
 * Set a property from a boolean.
 *
 */
void
lgl_xml_set_prop_boolean (xmlNodePtr    node,
			  const gchar  *property,
			  gboolean      val)
{
	xmlSetProp (node, (xmlChar *)property, (xmlChar *)(val ? "True" : "False"));
}

/**
 * lgl_xml_set_prop_int:
 * @node:        the libxml2 #xmlNodePtr of the node
 * @property:    the property name
 * @val:         the value to set
 *
 * Set a property from an integer.
 *
 */
void
lgl_xml_set_prop_int (xmlNodePtr    node,
		      const gchar  *property,
		      gint          val)
{
	gchar  *string;

	string = g_strdup_printf ("%d", val);
	xmlSetProp (node, (xmlChar *)property, (xmlChar *)string);
	g_free (string);
}

/**
 * lgl_xml_set_prop_uint_hex:
 * @node:        the libxml2 #xmlNodePtr of the node
 * @property:    the property name
 * @val:         the value to set
 *
 * Set a property from an unsigned integer and format in hex.
 *
 */
void
lgl_xml_set_prop_uint_hex (xmlNodePtr    node,
			   const gchar  *property,
			   guint         val)
{
	gchar  *string;

	string = g_strdup_printf ("0x%08x", val);
	xmlSetProp (node, (xmlChar *)property, (xmlChar *)string);
	g_free (string);
}

/**
 * lgl_xml_set_prop_length:
 * @node:        the libxml2 #xmlNodePtr of the node
 * @property:    the property name
 * @val:         the length to set in internal units (points)
 *
 * Set a property from a length, performing any necessary conversion.
 * Length properties are formatted as a number followed by a units string.
 * The units of the formatted property is determined by the most recent call to
 * lgl_xml_set_default_units().
 *
 */
void
lgl_xml_set_prop_length (xmlNodePtr    node,
			 const gchar  *property,
			 gdouble       val)
{
	gchar  *string, buffer[G_ASCII_DTOSTR_BUF_SIZE];
	gchar  *string_unit;

	/* Convert to default units */
	val *= lgl_units_get_units_per_point (default_units);

	/* Guarantee "C" locale by use of g_ascii_formatd */
	string = g_ascii_formatd (buffer, G_ASCII_DTOSTR_BUF_SIZE, "%g", val);

	string_unit = g_strdup_printf ("%s%s", string, lgl_units_get_id (default_units));
	xmlSetProp (node, (xmlChar *)property, (xmlChar *)string_unit);
        g_free (string_unit);
}

/**
 * lgl_xml_is_node
 * @node:        the libxml2 #xmlNodePtr of the node
 * @name    :    the node name
 *
 * Test if a node name matches given name.
 *
 * Returns: TRUE if the name of the node matches.  Otherwise FALSE.
 *
 */
gboolean
lgl_xml_is_node (xmlNodePtr   node,
		 const gchar *name)
{
	return xmlStrEqual (node->name, (xmlChar *)name);
}


/**
 * lgl_xml_get_node_content
 * @node:        the libxml2 #xmlNodePtr of the node
 *
 * Get the content of a node.
 *
 * Returns: the property as a pointer to a gchar string.  This string should
 *          be freed with g_free().
 */
gchar *
lgl_xml_get_node_content (xmlNodePtr   node)
{
	xmlChar *xml_content;
	gchar   *g_content;

	xml_content = xmlNodeGetContent (node);

	if (xml_content != NULL) {

		g_content = g_strdup ((gchar *)xml_content);
		xmlFree (xml_content);
		return g_content;

	}

	return NULL;
}


/**
 * lgl_xml_set_default_units:
 * @units:       default units selection (#lglUnits)
 *
 * Set the default units when formatting lengths.  See
 * lgl_xml_set_prop_length().
 *
 */
void
lgl_xml_set_default_units (lglUnits   units)
{
	g_return_if_fail ((units >= LGL_UNITS_FIRST) && (units <= LGL_UNITS_LAST));

	default_units = units;
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
