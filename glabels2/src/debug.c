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
#include <glib.h>

#include "debug.h"

/* External debug options, used here and in glabels.c */
gint gl_debug_all = 0;
gint gl_debug_view = 0;
gint gl_debug_item = 0;
gint gl_debug_prefs = 0;
gint gl_debug_print = 0;
gint gl_debug_file = 0;
gint gl_debug_label = 0;
gint gl_debug_template = 0;
gint gl_debug_xml = 0;
gint gl_debug_merge = 0;
gint gl_debug_commands = 0;
gint gl_debug_undo = 0;
gint gl_debug_recent = 0;
gint gl_debug_mdi = 0;
gint gl_debug_media_select = 0;
gint gl_debug_mini_preview = 0;
gint gl_debug_wdgt = 0;

/****************************************************************************/
/* Print debugging information.                                             */
/****************************************************************************/
void
gl_debug (gint   section,
	  gchar *file,
	  gint   line,
	  gchar *function,
	  gchar *format,
	  ...)
{
	va_list  args;
	gchar   *msg;

	g_return_if_fail (format != NULL);

	va_start (args, format);
	msg = g_strdup_vprintf (format, args);
	va_end (args);

	if (gl_debug_all ||
	    (gl_debug_view         && section == GL_DEBUG_VIEW)           ||
	    (gl_debug_item         && section == GL_DEBUG_ITEM)           ||
	    (gl_debug_print        && section == GL_DEBUG_PRINT)          ||
	    (gl_debug_prefs        && section == GL_DEBUG_PREFS)          ||
	    (gl_debug_file         && section == GL_DEBUG_FILE)           ||
	    (gl_debug_label        && section == GL_DEBUG_LABEL)          ||
	    (gl_debug_template     && section == GL_DEBUG_TEMPLATE)       ||
	    (gl_debug_xml          && section == GL_DEBUG_XML)            ||
	    (gl_debug_merge        && section == GL_DEBUG_MERGE)          ||
	    (gl_debug_commands     && section == GL_DEBUG_COMMANDS)       ||
	    (gl_debug_undo         && section == GL_DEBUG_UNDO)           ||
	    (gl_debug_recent       && section == GL_DEBUG_RECENT)         ||
	    (gl_debug_mdi          && section == GL_DEBUG_MDI)            ||
	    (gl_debug_media_select && section == GL_DEBUG_MEDIA_SELECT)   ||
	    (gl_debug_mini_preview && section == GL_DEBUG_MINI_PREVIEW)   ||
	    (gl_debug_wdgt         && section == GL_DEBUG_WDGT) )
		g_print ("%s:%d (%s) %s\n", file, line, function, msg);
	
	g_free (msg);
}
