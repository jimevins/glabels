/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  debug.c:  GLabels debug module
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

/*
 * This file is based on gedit-debug.c from gedit2:
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi 
 *
 */

#include "debug.h"

#include <glib/gutils.h>
#include <glib/gmessages.h>
#include <glib/gstrfuncs.h>

glDebugSection debug_flags = GLABELS_DEBUG_NONE;

/****************************************************************************/
/* Initialize debug flags, based on environmental variables.                */
/****************************************************************************/
void
gl_debug_init (void)
{
	if (g_getenv ("GLABELS_DEBUG") != NULL)
	{
		/* enable all debugging */
		debug_flags = ~GLABELS_DEBUG_NONE;
		return;
	}

	if (g_getenv ("GLABELS_DEBUG_VIEW") != NULL)
		debug_flags |= GLABELS_DEBUG_VIEW;
	if (g_getenv ("GLABELS_DEBUG_ITEM") != NULL)
		debug_flags |= GLABELS_DEBUG_ITEM;
	if (g_getenv ("GLABELS_DEBUG_PRINT") != NULL)
		debug_flags |= GLABELS_DEBUG_PRINT;
	if (g_getenv ("GLABELS_DEBUG_PREFS") != NULL)
		debug_flags |= GLABELS_DEBUG_PREFS;
	if (g_getenv ("GLABELS_DEBUG_FILE") != NULL)
		debug_flags |= GLABELS_DEBUG_FILE;
	if (g_getenv ("GLABELS_DEBUG_LABEL") != NULL)
		debug_flags |= GLABELS_DEBUG_LABEL;
	if (g_getenv ("GLABELS_DEBUG_TEMPLATE") != NULL)
		debug_flags |= GLABELS_DEBUG_TEMPLATE;
	if (g_getenv ("GLABELS_DEBUG_PAPER") != NULL)
		debug_flags |= GLABELS_DEBUG_PAPER;
	if (g_getenv ("GLABELS_DEBUG_XML") != NULL)
		debug_flags |= GLABELS_DEBUG_XML;
	if (g_getenv ("GLABELS_DEBUG_MERGE") != NULL)
		debug_flags |= GLABELS_DEBUG_MERGE;
	if (g_getenv ("GLABELS_DEBUG_UNDO") != NULL)
		debug_flags |= GLABELS_DEBUG_UNDO;
	if (g_getenv ("GLABELS_DEBUG_RECENT") != NULL)
		debug_flags |= GLABELS_DEBUG_RECENT;
	if (g_getenv ("GLABELS_DEBUG_COMMANDS") != NULL)
		debug_flags |= GLABELS_DEBUG_COMMANDS;
	if (g_getenv ("GLABELS_DEBUG_WINDOW") != NULL)
		debug_flags |= GLABELS_DEBUG_WINDOW;
	if (g_getenv ("GLABELS_DEBUG_UI") != NULL)
		debug_flags |= GLABELS_DEBUG_UI;
	if (g_getenv ("GLABELS_DEBUG_PROPERTY_BAR") != NULL)
		debug_flags |= GLABELS_DEBUG_PROPERTY_BAR;
	if (g_getenv ("GLABELS_DEBUG_MEDIA_SELECT") != NULL)
		debug_flags |= GLABELS_DEBUG_MEDIA_SELECT;
	if (g_getenv ("GLABELS_DEBUG_MINI_PREVIEW") != NULL)
		debug_flags |= GLABELS_DEBUG_MINI_PREVIEW;
	if (g_getenv ("GLABELS_DEBUG_PIXBUF_CACHE") != NULL)
		debug_flags |= GLABELS_DEBUG_PIXBUF_CACHE;
	if (g_getenv ("GLABELS_DEBUG_EDITOR") != NULL)
		debug_flags |= GLABELS_DEBUG_EDITOR;
	if (g_getenv ("GLABELS_DEBUG_WDGT") != NULL)
		debug_flags |= GLABELS_DEBUG_WDGT;
}


/****************************************************************************/
/* Print debugging information.                                             */
/****************************************************************************/
void
gl_debug (glDebugSection  section,
	  const gchar    *file,
	  gint            line,
	  const gchar    *function,
	  const gchar    *format,
	  ...)
{
	if  (debug_flags & section)
	{
		va_list  args;
		gchar   *msg;

		g_return_if_fail (format != NULL);

		va_start (args, format);
		msg = g_strdup_vprintf (format, args);
		va_end (args);

		g_print ("%s:%d (%s) %s\n", file, line, function, msg);

		g_free (msg);
	}
	
}
