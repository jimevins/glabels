/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  util.c:  various small utility functions
 *
 *  Copyright (C) 2001  Jim Evins <evins@snaught.com>.
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

#include <string.h>
#include <glib.h>
#include <math.h>
#include <libgnomeprint/gnome-font.h>

#include "util.h"

#define FRAC_DELTA 0.00005


/****************************************************************************/
/* Append ".glabels" extension to filename if needed.                       */
/****************************************************************************/
gchar *
gl_util_add_extension (const gchar * orig_filename)
{
	gchar *new_filename, *extension;

	extension = strrchr (orig_filename, '.');
	if (extension == NULL) {
		new_filename = g_strconcat (orig_filename, ".glabels", NULL);
	} else {
		if (g_strcasecmp (extension, ".glabels") != 0) {
			new_filename =
			    g_strconcat (orig_filename, ".glabels", NULL);
		} else {
			new_filename = g_strdup (orig_filename);
		}
	}

	return new_filename;
}

/****************************************************************************/
/* Make sure we have an absolute path to filename.                          */
/****************************************************************************/
gchar *
gl_util_make_absolute (const gchar * filename)
{
	gchar *pwd, *absolute_filename;

	if (g_path_is_absolute (filename)) {
		absolute_filename = g_strdup (filename);
	} else {
		pwd = g_get_current_dir ();
		absolute_filename =
		    g_strjoin (G_DIR_SEPARATOR_S, pwd, filename, NULL);
		g_free (pwd);
	}

	return absolute_filename;
}

/****************************************************************************/
/* Create fractional representation of number, if possible.                 */
/****************************************************************************/
gchar *
gl_util_fraction( gdouble x )
{
	static gdouble denom[] = { 1., 2., 3., 4., 8., 16., 32., 0. };
	gint i;
	gdouble product, remainder;
	gint n, d;

	for ( i=0; denom[i] != 0.0; i++ ) {
		product = x * denom[i];
		remainder = fabs(product - ((gint)(product+0.5)));
		if ( remainder < FRAC_DELTA ) break;
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
/* Utilities to deal with GTK_JUSTIFICATION types.                          */
/****************************************************************************/
const gchar *
gl_util_just_to_string (GtkJustification just)
{
	switch (just) {
	case GTK_JUSTIFY_LEFT:
		return "Left";
	case GTK_JUSTIFY_CENTER:
		return "Center";
	case GTK_JUSTIFY_RIGHT:
		return "Right";
	default:
		return "?";
	}
}

GtkJustification
gl_util_string_to_just (const gchar * string)
{

	if (g_strcasecmp (string, "Left") == 0) {
		return GTK_JUSTIFY_LEFT;
	} else if (g_strcasecmp (string, "Center") == 0) {
		return GTK_JUSTIFY_CENTER;
	} else if (g_strcasecmp (string, "Right") == 0) {
		return GTK_JUSTIFY_RIGHT;
	} else {
		return GTK_JUSTIFY_LEFT;
	}

}

/****************************************************************************/
/* Utilities to deal with GNOME_FONT_WEIGHT types                           */
/****************************************************************************/
const gchar *
gl_util_weight_to_string (GnomeFontWeight weight)
{
	switch (weight) {
	case GNOME_FONT_BOOK:
		return "Regular";
	case GNOME_FONT_BOLD:
		return "Bold";
	default:
		return "?";
	}
}

GnomeFontWeight
gl_util_string_to_weight (const gchar * string)
{

	if (g_strcasecmp (string, "Regular") == 0) {
		return GNOME_FONT_BOOK;
	} else if (g_strcasecmp (string, "Bold") == 0) {
		return GNOME_FONT_BOLD;
	} else {
		return GNOME_FONT_BOOK;
	}

}

