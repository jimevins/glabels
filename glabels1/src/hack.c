/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  hack.c:  various small hacks to deal with I18N
 *
 *  Copyright (C) 2001  Jim Evins <evins@snaught.com>.
 *
 * 
 *  The g_unichar_to_utf8 has been borrowed from gnumeric, which was borrowed
 *  from glib-1.3:
 *     g_unichar_to_utf8: Copyright Red Hat, Inc
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

#include <gdk/gdk.h>
#include <string.h>

#include "hack.h"

static int g_unichar_to_utf8 (gint c, gchar * outbuf);

/*****************************************************************************/
/* Convert text to UTF8.                                                     */
/*****************************************************************************/
gchar *
gl_hack_text_to_utf8 (gchar * text)
{
	gint len;
	GdkWChar *wide_text;
	gint enc_status;
	gchar *p, *utf8_text;
	gint i;

	len = strlen (text);
	wide_text = g_new (GdkWChar, len + 1);
	enc_status = gdk_mbstowcs (wide_text, text, len);
	if (enc_status < 0) {
		g_free (wide_text);
		return NULL;
	}

	p = utf8_text = g_new (gchar, enc_status * 6 + 1);
	for (i = 0; i < enc_status; i++) {
		p += g_unichar_to_utf8 ((gint) wide_text[i], p);
	}
	*p = 0;			/* terminate */

	g_free (wide_text);

	return utf8_text;
}

/*****************************************************************************/
/* Calculate width of text with given font.                                  */
/*****************************************************************************/
double
gl_hack_get_width_string (GnomeFont * font,
			  gchar * text)
{
	gint len;
	GdkWChar *wide_text;
	gint enc_status;
	gint i;
	gdouble w;

	len = strlen (text);
	wide_text = g_new (GdkWChar, len + 1);
	enc_status = gdk_mbstowcs (wide_text, text, len);
	if (enc_status < 0) {
		g_free (wide_text);
		return 0.0;
	}

	w = 0;
	for (i = 0; i < enc_status; i++) {
		w += gnome_font_get_glyph_width (font,
						 gnome_font_lookup_default
						 (font, wide_text[i]));
	}

	g_free (wide_text);

	return w;
}

/*-----------------------------------------------------------------------------
 * This is cut & pasted from glib 1.3
 *
 * We need it only for iso-8859-1 converter and it will be
 * abandoned, if glib 2.0 or any other unicode library will
 * be introduced.
 *---------------------------------------------------------------------------*/
static int
g_unichar_to_utf8 (gint c,
		   gchar * outbuf)
{
	size_t len = 0;
	int first;
	int i;

	if (c < 0x80) {
		first = 0;
		len = 1;
	} else if (c < 0x800) {
		first = 0xc0;
		len = 2;
	} else if (c < 0x10000) {
		first = 0xe0;
		len = 3;
	} else if (c < 0x200000) {
		first = 0xf0;
		len = 4;
	} else if (c < 0x4000000) {
		first = 0xf8;
		len = 5;
	} else {
		first = 0xfc;
		len = 6;
	}

	if (outbuf) {
		for (i = len - 1; i > 0; --i) {
			outbuf[i] = (c & 0x3f) | 0x80;
			c >>= 6;
		}
		outbuf[0] = c | first;
	}

	return len;
}
