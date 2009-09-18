/*
 *  view-image.h
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

#ifndef __VIEW_IMAGE_H__
#define __VIEW_IMAGE_H__

#include "view-object.h"
#include "label-image.h"

G_BEGIN_DECLS


#define GL_TYPE_VIEW_IMAGE            (gl_view_image_get_type ())
#define GL_VIEW_IMAGE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_VIEW_IMAGE, glViewImage))
#define GL_VIEW_IMAGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_VIEW_IMAGE, glViewImageClass))
#define GL_IS_VIEW_IMAGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_VIEW_IMAGE))
#define GL_IS_VIEW_IMAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_VIEW_IMAGE))

typedef struct _glViewImage          glViewImage;
typedef struct _glViewImageClass     glViewImageClass;

typedef struct _glViewImagePrivate   glViewImagePrivate;

struct _glViewImage {
	glViewObject        parent_object;

	glViewImagePrivate *priv;
};

struct _glViewImageClass {
	glViewObjectClass   parent_class;
};


GType          gl_view_image_get_type (void) G_GNUC_CONST;

glViewObject  *gl_view_image_new      (glLabelImage *object,
                                       glView     *view);


/* cursor for creating image objects */
GdkCursor *gl_view_image_get_create_cursor (void);

/* Object creation handlers. */
void       gl_view_image_create_button_press_event   (glView *view,
                                                      gdouble x,
                                                      gdouble y);

void       gl_view_image_create_motion_event         (glView *view,
                                                      gdouble x,
                                                      gdouble y);

void       gl_view_image_create_button_release_event (glView *view,
                                                      gdouble x,
                                                      gdouble y);


G_END_DECLS

#endif /* __VIEW_IMAGE_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
