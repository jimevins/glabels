/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  debug.h:  GLabels debug module
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
 * This file is based on gedit-debug.h from gedit2:
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi 
 *
 */
#ifndef __DEBUG_H__
#define __DEBUG_H__

G_BEGIN_DECLS

typedef enum {
	GL_DEBUG_VIEW,
	GL_DEBUG_ITEM,
	GL_DEBUG_PRINT,
	GL_DEBUG_PREFS,
	GL_DEBUG_FILE,
	GL_DEBUG_LABEL,
	GL_DEBUG_TEMPLATE,
	GL_DEBUG_PAPER,
	GL_DEBUG_XML,
	GL_DEBUG_MERGE,
	GL_DEBUG_UNDO,
	GL_DEBUG_RECENT,
	GL_DEBUG_COMMANDS,
	GL_DEBUG_WINDOW,
	GL_DEBUG_UI,
	GL_DEBUG_PROPERTY_BAR,
	GL_DEBUG_MEDIA_SELECT,
	GL_DEBUG_MINI_PREVIEW,
	GL_DEBUG_PIXBUF_CACHE,
	GL_DEBUG_WDGT,
} glDebugSection;

extern gint gl_debug_all;
extern gint gl_debug_view;
extern gint gl_debug_item;
extern gint gl_debug_print;
extern gint gl_debug_prefs;
extern gint gl_debug_file;
extern gint gl_debug_label;
extern gint gl_debug_template;
extern gint gl_debug_paper;
extern gint gl_debug_xml;
extern gint gl_debug_merge;
extern gint gl_debug_commands;
extern gint gl_debug_undo;
extern gint gl_debug_recent;
extern gint gl_debug_window;
extern gint gl_debug_ui;
extern gint gl_debug_property_bar;
extern gint gl_debug_media_select;
extern gint gl_debug_mini_preview;
extern gint gl_debug_pixbuf_cache;
extern gint gl_debug_wdgt;

#ifndef __GNUC__
#define __FUNCTION__   ""
#endif

#define	DEBUG_VIEW	GL_DEBUG_VIEW,    __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_ITEM	GL_DEBUG_ITEM,    __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_PRINT	GL_DEBUG_PRINT,   __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_PREFS	GL_DEBUG_PREFS,   __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_FILE	GL_DEBUG_FILE,    __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_LABEL	GL_DEBUG_LABEL,   __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_TEMPLATE	GL_DEBUG_TEMPLATE,__FILE__, __LINE__, __FUNCTION__
#define	DEBUG_PAPER	GL_DEBUG_PAPER,   __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_XML	GL_DEBUG_XML,     __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_MERGE	GL_DEBUG_MERGE,   __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_UNDO	GL_DEBUG_UNDO,    __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_RECENT	GL_DEBUG_RECENT,  __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_COMMANDS	GL_DEBUG_COMMANDS,__FILE__, __LINE__, __FUNCTION__
#define	DEBUG_WINDOW    GL_DEBUG_WINDOW,  __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_UI        GL_DEBUG_UI,      __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_PROPERTY_BAR      GL_DEBUG_PROPERTY_BAR,     __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_MEDIA_SELECT	GL_DEBUG_MEDIA_SELECT,     __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_MINI_PREVIEW	GL_DEBUG_MINI_PREVIEW,     __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_PIXBUF_CACHE	GL_DEBUG_PIXBUF_CACHE,     __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_WDGT	GL_DEBUG_WDGT,    __FILE__, __LINE__, __FUNCTION__

void gl_debug (gint section, gchar *file,
	       gint line, gchar* function, gchar* format, ...);

G_END_DECLS

#endif /* __DEBUG_H__ */
