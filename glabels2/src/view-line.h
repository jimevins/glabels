/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_line.h:  GLabels canvas item wrapper widget
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

#ifndef __VIEW_LINE_H__
#define __VIEW_LINE_H__

#include "view-object.h"
#include "label-line.h"

G_BEGIN_DECLS


#define GL_TYPE_VIEW_LINE            (gl_view_line_get_type ())
#define GL_VIEW_LINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_VIEW_LINE, glViewLine))
#define GL_VIEW_LINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_VIEW_LINE, glViewLineClass))
#define GL_IS_VIEW_LINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_VIEW_LINE))
#define GL_IS_VIEW_LINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_VIEW_LINE))

typedef struct _glViewLine          glViewLine;
typedef struct _glViewLineClass     glViewLineClass;

typedef struct _glViewLinePrivate   glViewLinePrivate;

struct _glViewLine {
	glViewObject          parent_object;

	glViewLinePrivate  *private;
};

struct _glViewLineClass {
	glViewObjectClass     parent_class;
};


GType          gl_view_line_get_type (void) G_GNUC_CONST;

glViewObject  *gl_view_line_new      (glLabelLine *object,
				      glView      *view);


/* cursor for creating line objects */
GdkCursor *gl_view_line_get_create_cursor (void);

/* event handler for creating line objects */
gint gl_view_line_create_event_handler    (GnomeCanvas *canvas,
					   GdkEvent    *event,
					   glView      *view);

G_END_DECLS

#endif /* __VIEW_LINE_H__ */
