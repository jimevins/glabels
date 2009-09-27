/*
 *  view-object.h
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

#ifndef __VIEW_OBJECT_H__
#define __VIEW_OBJECT_H__

#include <glib-object.h>
#include "label-object.h"

typedef enum {
	GL_VIEW_OBJECT_HANDLES_BOX,
	GL_VIEW_OBJECT_HANDLES_LINE,
} glViewObjectHandlesStyle;

typedef enum {
        GL_VIEW_OBJECT_HANDLE_NONE = 0,
	GL_VIEW_OBJECT_HANDLE_N,
	GL_VIEW_OBJECT_HANDLE_E,
	GL_VIEW_OBJECT_HANDLE_W,
	GL_VIEW_OBJECT_HANDLE_S,
	GL_VIEW_OBJECT_HANDLE_NW,
	GL_VIEW_OBJECT_HANDLE_NE,
	GL_VIEW_OBJECT_HANDLE_SE,
	GL_VIEW_OBJECT_HANDLE_SW,
	GL_VIEW_OBJECT_HANDLE_P1,
	GL_VIEW_OBJECT_HANDLE_P2,
} glViewObjectHandle;


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

#include "cairo.h"

struct _glViewObject {
	GObject               parent_object;

	glViewObjectPrivate  *priv;
};

struct _glViewObjectClass {
	GObjectClass          parent_class;

	/*
	 * Methods
	 */

	GtkWidget * (*construct_editor) (glViewObject *view_object);

        gboolean    (*object_at)        (glViewObject *view_object,
                                         cairo_t      *cr,
                                         gdouble       x_device,
                                         gdouble       y_device);
};




G_BEGIN_DECLS

GType              gl_view_object_get_type          (void) G_GNUC_CONST;

GObject           *gl_view_object_new               (void);


void               gl_view_object_set_view          (glViewObject             *view_object,
                                                     glView                   *view);

void               gl_view_object_set_object        (glViewObject             *view_object,
                                                     glLabelObject            *object,
                                                     glViewObjectHandlesStyle  style);

gboolean           gl_view_object_at                (glViewObject             *view_object,
                                                     cairo_t                  *cr,
                                                     gdouble                   x,
                                                     gdouble                   y);

void               gl_view_object_draw_handles      (glViewObject             *view_object,
                                                     cairo_t                  *cr);

glViewObjectHandle gl_view_object_handle_at         (glViewObject             *view_object,
                                                     cairo_t                  *cr,
                                                     gdouble                   x,
                                                     gdouble                   y);

glView            *gl_view_object_get_view          (glViewObject             *view_object);

glLabelObject     *gl_view_object_get_object        (glViewObject             *view_object);

GtkWidget         *gl_view_object_get_editor        (glViewObject             *view_object);

void               gl_view_object_select            (glViewObject             *view_object);

void               gl_view_object_resize_event      (glViewObject             *view_object,
                                                     glViewObjectHandle        handle,
                                                     gboolean                  honor_aspect,
                                                     cairo_t                  *cr,
                                                     gdouble                   x,
                                                     gdouble                   y);


G_END_DECLS

#endif /* __VIEW_OBJECT_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
