/*
 *  debug.h
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

/*
 * This file is based on gedit-debug.h from gedit2:
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi 
 *
 */
#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <glib.h>

G_BEGIN_DECLS

/*
 * Set an environmental var of the same name to turn on
 * debugging output. Setting GLABELS_DEBUG will turn on all
 * sections.
 */

typedef enum {
	GLABELS_DEBUG_NONE         = 0,
	GLABELS_DEBUG_VIEW         = 1 << 0,
	GLABELS_DEBUG_ITEM         = 1 << 1,
	GLABELS_DEBUG_PRINT        = 1 << 2,
	GLABELS_DEBUG_PREFS        = 1 << 3,
	GLABELS_DEBUG_FILE         = 1 << 4,
	GLABELS_DEBUG_LABEL        = 1 << 5,
	GLABELS_DEBUG_TEMPLATE     = 1 << 6,
	GLABELS_DEBUG_PAPER        = 1 << 7,
	GLABELS_DEBUG_XML          = 1 << 8,
	GLABELS_DEBUG_MERGE        = 1 << 9,
	GLABELS_DEBUG_UNDO         = 1 << 10,
	GLABELS_DEBUG_RECENT       = 1 << 11,
	GLABELS_DEBUG_COMMANDS     = 1 << 12,
	GLABELS_DEBUG_WINDOW       = 1 << 13,
	GLABELS_DEBUG_UI           = 1 << 14,
	GLABELS_DEBUG_PROPERTY_BAR = 1 << 15,
	GLABELS_DEBUG_MEDIA_SELECT = 1 << 16,
	GLABELS_DEBUG_MINI_PREVIEW = 1 << 17,
	GLABELS_DEBUG_PIXBUF_CACHE = 1 << 18,
	GLABELS_DEBUG_EDITOR       = 1 << 19,
	GLABELS_DEBUG_WDGT         = 1 << 20,
        GLABELS_DEBUG_PATH         = 1 << 21,
	GLABELS_DEBUG_FIELD_BUTTON = 1 << 22,
        GLABELS_DEBUG_BARCODE      = 1 << 23,
} glDebugSection;


#ifndef __GNUC__
#define __FUNCTION__   ""
#endif

#define	DEBUG_VIEW	GLABELS_DEBUG_VIEW,    __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_ITEM	GLABELS_DEBUG_ITEM,    __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_PRINT	GLABELS_DEBUG_PRINT,   __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_PREFS	GLABELS_DEBUG_PREFS,   __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_FILE	GLABELS_DEBUG_FILE,    __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_LABEL	GLABELS_DEBUG_LABEL,   __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_TEMPLATE	GLABELS_DEBUG_TEMPLATE,__FILE__, __LINE__, __FUNCTION__
#define	DEBUG_PAPER	GLABELS_DEBUG_PAPER,   __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_XML	GLABELS_DEBUG_XML,     __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_MERGE	GLABELS_DEBUG_MERGE,   __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_UNDO	GLABELS_DEBUG_UNDO,    __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_RECENT	GLABELS_DEBUG_RECENT,  __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_COMMANDS	GLABELS_DEBUG_COMMANDS,__FILE__, __LINE__, __FUNCTION__
#define	DEBUG_WINDOW    GLABELS_DEBUG_WINDOW,  __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_UI        GLABELS_DEBUG_UI,      __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_PROPERTY_BAR      GLABELS_DEBUG_PROPERTY_BAR,     __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_MEDIA_SELECT	GLABELS_DEBUG_MEDIA_SELECT,     __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_MINI_PREVIEW	GLABELS_DEBUG_MINI_PREVIEW,     __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_PIXBUF_CACHE	GLABELS_DEBUG_PIXBUF_CACHE,     __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_EDITOR	GLABELS_DEBUG_EDITOR, __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_WDGT	GLABELS_DEBUG_WDGT,   __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_PATH      GLABELS_DEBUG_PATH,   __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_FIELD_BUTTON      GLABELS_DEBUG_FIELD_BUTTON,   __FILE__, __LINE__, __FUNCTION__
#define	DEBUG_BARCODE   GLABELS_DEBUG_BARCODE,__FILE__, __LINE__, __FUNCTION__

void gl_debug_init (void);

void gl_debug      (glDebugSection  section,
		    const gchar    *file,
		    gint            line,
		    const gchar    *function,
		    const gchar    *format,
		    ...);

G_END_DECLS

#endif /* __DEBUG_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
