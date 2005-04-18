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

typedef enum {
	GL_VIEW_HIGHLIGHT_BOX_RESIZABLE,
	GL_VIEW_HIGHLIGHT_ELLIPSE_RESIZABLE,
	GL_VIEW_HIGHLIGHT_LINE_RESIZABLE,
	GL_VIEW_HIGHLIGHT_SIMPLE,
} glViewHighlightStyle;

#include "view.h"
#include "label-object.h"

G_BEGIN_DECLS

#define GL_TYPE_VIEW_HIGHLIGHT            (gl_view_highlight_get_type ())
#define GL_VIEW_HIGHLIGHT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_VIEW_HIGHLIGHT, glViewHighlight))
#define GL_VIEW_HIGHLIGHT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_VIEW_HIGHLIGHT, glViewHighlightClass))
#define GL_IS_VIEW_HIGHLIGHT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_VIEW_HIGHLIGHT))
#define GL_IS_VIEW_HIGHLIGHT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_VIEW_HIGHLIGHT))

typedef struct _glViewHighlight          glViewHighlight;
typedef struct _glViewHighlightClass     glViewHighlightClass;

typedef struct _glViewHighlightPrivate   glViewHighlightPrivate;


struct _glViewHighlight {
	GObject                  parent_object;

	glViewHighlightPrivate  *private;
};

struct _glViewHighlightClass {
	GObjectClass             parent_class;
};



GType    gl_view_highlight_get_type (void) G_GNUC_CONST;

GObject *gl_view_highlight_new      (glViewObject         *view_object,
				     glViewHighlightStyle  style);

void     gl_view_highlight_show     (glViewHighlight      *view_highlight);
void     gl_view_highlight_hide     (glViewHighlight      *view_highlight);

G_END_DECLS

#endif
