/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_box.h:  GLabels canvas item wrapper widget
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

#ifndef __VIEW_BOX_H__
#define __VIEW_BOX_H__

#include "view-object.h"
#include "label-box.h"

G_BEGIN_DECLS


#define GL_TYPE_VIEW_BOX            (gl_view_box_get_type ())
#define GL_VIEW_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_VIEW_BOX, glViewBox))
#define GL_VIEW_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_VIEW_BOX, glViewBoxClass))
#define GL_IS_VIEW_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_VIEW_BOX))
#define GL_IS_VIEW_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_VIEW_BOX))

typedef struct _glViewBox          glViewBox;
typedef struct _glViewBoxClass     glViewBoxClass;

typedef struct _glViewBoxPrivate   glViewBoxPrivate;

struct _glViewBox {
	glViewObject          parent_object;

	glViewBoxPrivate      *private;
};

struct _glViewBoxClass {
	glViewObjectClass     parent_class;
};


GType          gl_view_box_get_type (void) G_GNUC_CONST;

glViewObject  *gl_view_box_new      (glLabelBox *object,
				     glView     *view);


/* cursor for creating box objects */
GdkCursor *gl_view_box_get_create_cursor (void);

/* event handler for creating box objects */
gint gl_view_box_create_event_handler    (GnomeCanvas *canvas,
					  GdkEvent    *event,
					  glView      *view);

G_END_DECLS

#endif /* __VIEW_BOX_H__ */
