/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_object.h:  GLabels canvas item wrapper widget
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

#ifndef __VIEW_OBJECT_H__
#define __VIEW_OBJECT_H__

#include <glib-object.h>
#include <gnome.h>
#include "label-object.h"

#define GL_TYPE_VIEW_OBJECT              (gl_view_object_get_type ())
#define GL_VIEW_OBJECT(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_VIEW_OBJECT, glViewObject))
#define GL_VIEW_OBJECT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_VIEW_OBJECT, glViewObjectClass))
#define GL_IS_VIEW_OBJECT(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_VIEW_OBJECT))
#define GL_IS_VIEW_OBJECT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_VIEW_OBJECT))
#define GL_VIEW_OBJECT_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), GL_TYPE_VIEW_OBJECT, glViewObjectClass))

typedef struct _glViewObject          glViewObject;
typedef struct _glViewObjectClass     glViewObjectClass;

typedef struct _glViewObjectPrivate   glViewObjectPrivate;


#include "view.h"
#include "view-highlight.h"

struct _glViewObject {
	GObject              parent_object;

	glViewObjectPrivate  *private;
};

struct _glViewObjectClass {
	GObjectClass         parent_class;

	/*
	 * Methods
	 */

	GtkWidget * (*construct_dialog) (glViewObject *view_object);
};


G_BEGIN_DECLS

GType            gl_view_object_get_type          (void);

GObject         *gl_view_object_new               (void);


void             gl_view_object_set_view          (glViewObject         *view_object,
						   glView               *view);

void             gl_view_object_set_object        (glViewObject         *view_object,
						   glLabelObject        *object,
						   glViewHighlightStyle  style);

glView          *gl_view_object_get_view          (glViewObject         *view_object);

glLabelObject   *gl_view_object_get_object        (glViewObject         *view_object);

GnomeCanvasItem *gl_view_object_get_group         (glViewObject         *view_object);

GnomeCanvasItem *gl_view_object_item_new          (glViewObject         *view_object,
						   GType                 type,
						   const gchar          *first_arg_name,
						   ...);

GtkMenu         *gl_view_object_get_menu          (glViewObject         *view_object);

void             gl_view_object_show_highlight    (glViewObject         *view_object);

void             gl_view_object_hide_highlight    (glViewObject         *view_object);

void             gl_view_object_show_dialog       (glViewObject         *view_object);

void             gl_view_object_select            (glViewObject         *view_object);

G_END_DECLS

#endif /* __VIEW_OBJECT_H__ */
