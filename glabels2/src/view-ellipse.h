/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_ellipse.h:  GLabels canvas item wrapper widget
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

#ifndef __VIEW_ELLIPSE_H__
#define __VIEW_ELLIPSE_H__

#include "view-object.h"
#include "label-ellipse.h"

G_BEGIN_DECLS


#define GL_TYPE_VIEW_ELLIPSE            (gl_view_ellipse_get_type ())
#define GL_VIEW_ELLIPSE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_VIEW_ELLIPSE, glViewEllipse))
#define GL_VIEW_ELLIPSE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_VIEW_ELLIPSE, glViewEllipseClass))
#define GL_IS_VIEW_ELLIPSE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_VIEW_ELLIPSE))
#define GL_IS_VIEW_ELLIPSE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_VIEW_ELLIPSE))

typedef struct _glViewEllipse          glViewEllipse;
typedef struct _glViewEllipseClass     glViewEllipseClass;

typedef struct _glViewEllipsePrivate   glViewEllipsePrivate;

struct _glViewEllipse {
	glViewObject          parent_object;

	glViewEllipsePrivate  *private;
};

struct _glViewEllipseClass {
	glViewObjectClass     parent_class;
};


GType          gl_view_ellipse_get_type (void) G_GNUC_CONST;

glViewObject  *gl_view_ellipse_new      (glLabelEllipse *object,
					 glView         *view);


/* cursor for creating ellipse objects */
GdkCursor *gl_view_ellipse_get_create_cursor (void);

/* event handler for creating ellipse objects */
gint gl_view_ellipse_create_event_handler    (GnomeCanvas *canvas,
					      GdkEvent    *event,
					      glView      *view);

G_END_DECLS

#endif /* __VIEW_ELLIPSE_H__ */
