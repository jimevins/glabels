/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_highlight.h:  GLabels Resizable Highlight module header file
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
#ifndef __VIEW_HIGHLIGHT_H__
#define __VIEW_HIGHLIGHT_H__

#include <gnome.h>

typedef enum {
	GL_VIEW_HIGHLIGHT_BOX_RESIZABLE,
	GL_VIEW_HIGHLIGHT_ELLIPSE_RESIZABLE,
	GL_VIEW_HIGHLIGHT_LINE_RESIZABLE,
	GL_VIEW_HIGHLIGHT_SIMPLE,
} glViewHighlightStyle;

#include "view-object.h"

extern GnomeCanvasItem *gl_view_highlight_new (glViewObject *view_object,
					       glViewHighlightStyle style);

extern void gl_view_highlight_update (GnomeCanvasItem * highlight);

#endif
