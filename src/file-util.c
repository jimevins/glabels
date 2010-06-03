/*
 *  file-util.c
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

#include "file-util.h"

#include <string.h>


/****************************************************************************/
/* Append ".glabels" extension to filename if needed.                       */
/****************************************************************************/
gchar *
gl_file_util_add_extension (const gchar *orig_filename)
{
	gchar *new_filename, *extension;

	extension = strrchr (orig_filename, '.');
	if (extension == NULL) {
		new_filename = g_strconcat (orig_filename, ".glabels", NULL);
	} else {
		if (g_ascii_strcasecmp (extension, ".glabels") != 0) {
			new_filename =
			    g_strconcat (orig_filename, ".glabels", NULL);
		} else {
			new_filename = g_strdup (orig_filename);
		}
	}

	return new_filename;
}


/****************************************************************************/
/* Remove ".glabels" extension from filename if needed.                     */
/****************************************************************************/
gchar *
gl_file_util_remove_extension (const gchar *orig_filename)
{
	gchar *new_filename, *extension;

	new_filename = g_strdup (orig_filename);

	extension = strrchr (new_filename, '.');
	if (extension != NULL) {
		if (g_ascii_strcasecmp (extension, ".glabels") == 0) {
			*extension = 0; /* truncate string, rm extension */
		}
	}

	return new_filename;
}


/****************************************************************************/
/* Make sure we have an absolute path to filename.                          */
/****************************************************************************/
gchar *
gl_file_util_make_absolute (const gchar *filename)
{
	gchar *pwd, *absolute_filename;

	if (g_path_is_absolute (filename)) {
		absolute_filename = g_strdup (filename);
	} else {
		pwd = g_get_current_dir ();
		absolute_filename = g_build_filename (pwd, filename, NULL);
		g_free (pwd);
	}

	return absolute_filename;
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
