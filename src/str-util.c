/*
 *  str-util.c
 *  Copyright (C) 2001-2009  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of gLabels.
 *
 *  gLabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gLabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gLabels.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "str-util.h"

#include <string.h>
#include <math.h>

#define FRAC_EPSILON 0.00005


/****************************************************************************/
/* Create fractional representation of number, if possible.                 */
/****************************************************************************/
gchar *
gl_str_util_fraction_to_string (gdouble x)
{
	static gdouble denom[] = { 1., 2., 3., 4., 8., 16., 32., 0. };
	gint i;
	gdouble product, remainder;
	gint n, d;

	for ( i=0; denom[i] != 0.0; i++ ) {
		product = x * denom[i];
		remainder = fabs(product - ((gint)(product+0.5)));
		if ( remainder < FRAC_EPSILON ) break;
	}

	if ( denom[i] == 0.0 ) {
		/* None of our denominators work. */
		return g_strdup_printf ("%.5g", x);
	}
	if ( denom[i] == 1.0 ) {
		/* Simple integer. */
		return g_strdup_printf ("%d", (gint)x);
	}
	n = (gint)( x * denom[i] + 0.5 );
	d = (gint)denom[i];
	if ( n > d ) {
		return g_strdup_printf ("%d_%d/%d", (n/d), (n%d), d);
	} else {
		return g_strdup_printf ("%d/%d", (n%d), d);
	}
}


/****************************************************************************/
/* Utilities to deal with PangoAlignment types.                             */
/****************************************************************************/
const gchar *
gl_str_util_align_to_string (PangoAlignment align)
{
	switch (align) {
	case PANGO_ALIGN_LEFT:
		return "Left";
	case PANGO_ALIGN_CENTER:
		return "Center";
	case PANGO_ALIGN_RIGHT:
		return "Right";
	default:
		return "?";
	}
}


PangoAlignment
gl_str_util_string_to_align (const gchar *string)
{

	if (g_strcasecmp (string, "Left") == 0) {
		return PANGO_ALIGN_LEFT;
	} else if (g_strcasecmp (string, "Center") == 0) {
		return PANGO_ALIGN_CENTER;
	} else if (g_strcasecmp (string, "Right") == 0) {
		return PANGO_ALIGN_RIGHT;
	} else {
		return PANGO_ALIGN_LEFT;
	}

}


/****************************************************************************/
/* Utilities to deal with PangoWeight types                                 */
/****************************************************************************/
const gchar *
gl_str_util_weight_to_string (PangoWeight weight)
{
	switch (weight) {
	case PANGO_WEIGHT_NORMAL:
		return "Regular";
	case PANGO_WEIGHT_BOLD:
		return "Bold";
	default:
		return "?";
	}
}


PangoWeight
gl_str_util_string_to_weight (const gchar *string)
{

	if (g_strcasecmp (string, "Regular") == 0) {
		return PANGO_WEIGHT_NORMAL;
	} else if (g_strcasecmp (string, "Bold") == 0) {
		return PANGO_WEIGHT_BOLD;
	} else {
		return PANGO_WEIGHT_NORMAL;
	}

}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
