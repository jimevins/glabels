/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  recent.c:  gLabels recent files module
 *
 *  Copyright (C) 2001-2002  Jim Evins <evins@snaught.com>.
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


#include "recent.h"
#include "prefs.h"

#include <string.h>

#include "debug.h"

static EggRecentModel *model;

/**
 * unescape_string:
 * @escaped_string: an escaped URI, path, or other string
 * @illegal_characters: a string containing a sequence of characters
 * considered "illegal", '\0' is automatically in this list.
 *
 * Decodes escaped characters (i.e. PERCENTxx sequences) in @escaped_string.
 * Characters are encoded in PERCENTxy form, where xy is the ASCII hex code 
 * for character 16x+y.
 * 
 * Return value: a newly allocated string with the unescaped equivalents, 
 * or %NULL if @escaped_string contained one of the characters 
 * in @illegal_characters.
 *
 * this code comes from gnome-vfs-utils.c
 **/

#define HEX_ESCAPE '%'

static int
hex_to_int (gchar c)
{
	return  c >= '0' && c <= '9' ? c - '0'
		: c >= 'A' && c <= 'F' ? c - 'A' + 10
		: c >= 'a' && c <= 'f' ? c - 'a' + 10
		: -1;
}

static int
unescape_character (const char *scanner)
{
	int first_digit;
	int second_digit;

	first_digit = hex_to_int (*scanner++);
	if (first_digit < 0) {
		return -1;
	}

	second_digit = hex_to_int (*scanner++);
	if (second_digit < 0) {
		return -1;
	}

	return (first_digit << 4) | second_digit;
}

static char *
unescape_string (const gchar *escaped_string, 
		 const gchar *illegal_characters)
{
	const gchar *in;
	gchar *out, *result;
	gint character;

	if (escaped_string == NULL) {
		return NULL;
	}

	result = g_malloc (strlen (escaped_string) + 1);
	
	out = result;
	for (in = escaped_string; *in != '\0'; in++) {
		character = *in;
		if (*in == HEX_ESCAPE) {
			character = unescape_character (in + 1);

			/* Check for an illegal character. We consider '\0' illegal here. */
			if (character <= 0
			    || (illegal_characters != NULL
				&& strchr (illegal_characters, (char)character) != NULL)) {
				g_free (result);
				return NULL;
			}
			in += 2;
		}
		*out++ = (char)character;
	}
	
	*out = '\0';
	g_assert (out - result <= strlen (escaped_string));
	return result;
	
}

gchar *
gl_recent_get_filename (EggRecentItem *item)
{
	char *uri, *filename;
	char *utf8_filename = NULL;
	int prefix_len = strlen ("file://");
	gboolean result = FALSE;

        uri = egg_recent_item_get_uri (item);

	if (strlen (uri) > prefix_len) { 
		filename = unescape_string (uri + prefix_len, "");
		utf8_filename = g_filename_to_utf8 (filename, -1, NULL, NULL, NULL);
		g_free (filename);
	}
	
        g_free (uri);

        return utf8_filename;
}

void
gl_recent_add_uri (gchar *uri)
{
	EggRecentItem *item;
	
	gl_debug (DEBUG_RECENT, "add_uri = %s", uri);

	item = egg_recent_item_new_from_uri (uri);
	egg_recent_item_add_group (item, "glabels");
	egg_recent_model_add_full (model, item);
	egg_recent_item_unref (item); 
}

EggRecentModel *
gl_recent_get_model (void)
{
	return model;
}

void
gl_recent_init (void)
{
	gl_debug (DEBUG_RECENT, "max_recents = %d", gl_prefs->max_recents);

	model = egg_recent_model_new (EGG_RECENT_MODEL_SORT_MRU);
	egg_recent_model_set_limit (model,
				    gl_prefs->max_recents);
 	egg_recent_model_set_filter_groups (model, "glabels", NULL);
        egg_recent_model_set_filter_uri_schemes (model, "file", NULL);	
}
