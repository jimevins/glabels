/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_text.h:  GLabels canvas item wrapper widget
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

#ifndef __VIEW_TEXT_H__
#define __VIEW_TEXT_H__

#include "view-object.h"
#include "label-text.h"

G_BEGIN_DECLS


#define GL_TYPE_VIEW_TEXT            (gl_view_text_get_type ())
#define GL_VIEW_TEXT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_VIEW_TEXT, glViewText))
#define GL_VIEW_TEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_VIEW_TEXT, glViewTextClass))
#define GL_IS_VIEW_TEXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_VIEW_TEXT))
#define GL_IS_VIEW_TEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_VIEW_TEXT))

typedef struct _glViewText          glViewText;
typedef struct _glViewTextClass     glViewTextClass;

typedef struct _glViewTextPrivate   glViewTextPrivate;

struct _glViewText {
	glViewObject        parent_object;

	glViewTextPrivate  *priv;
};

struct _glViewTextClass {
	glViewObjectClass   parent_class;
};


GType          gl_view_text_get_type (void) G_GNUC_CONST;

glViewObject  *gl_view_text_new      (glLabelText *object,
				      glView      *view);


/* cursor for creating text objects */
GdkCursor *gl_view_text_get_create_cursor (void);

/* event handler for creating text objects */
gint gl_view_text_create_event_handler    (GnomeCanvas *canvas,
					   GdkEvent    *event,
					   glView      *view);

G_END_DECLS

#endif /* __VIEW_TEXT_H__ */
