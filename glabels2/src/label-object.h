/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label_object.h:  GLabels label object base class
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

#ifndef __LABEL_OBJECT_H__
#define __LABEL_OBJECT_H__

#include <glib-object.h>


G_BEGIN_DECLS

typedef enum {
        GL_LABEL_OBJECT_TEXT,
        GL_LABEL_OBJECT_BOX,
        GL_LABEL_OBJECT_LINE,
        GL_LABEL_OBJECT_ELLIPSE,
        GL_LABEL_OBJECT_IMAGE,
        GL_LABEL_OBJECT_BARCODE,
        GL_LABEL_OBJECT_N_TYPES
} glLabelObjectType;


#define GL_TYPE_LABEL_OBJECT              (gl_label_object_get_type ())
#define GL_LABEL_OBJECT(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_LABEL_OBJECT, glLabelObject))
#define GL_LABEL_OBJECT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_LABEL_OBJECT, glLabelObjectClass))
#define GL_IS_LABEL_OBJECT(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_LABEL_OBJECT))
#define GL_IS_LABEL_OBJECT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_LABEL_OBJECT))
#define GL_LABEL_OBJECT_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), GL_TYPE_LABEL_OBJECT, glLabelObjectClass))


typedef struct _glLabelObject          glLabelObject;
typedef struct _glLabelObjectClass     glLabelObjectClass;

typedef struct _glLabelObjectPrivate   glLabelObjectPrivate;

#include "label.h"

struct _glLabelObject {
	GObject               object;

	glLabel              *parent;

	glLabelObjectPrivate *private;
};

struct _glLabelObjectClass {
	GObjectClass          parent_class;

	/*
	 * Methods
	 */

	void (*set_size)    (glLabelObject     *object,
			     gdouble            w,
			     gdouble            h);

	void (*get_size)    (glLabelObject     *object,
			     gdouble           *w,
			     gdouble           *h);

	/*
	 * Signals
	 */
	void (*changed)     (glLabelObject     *object,
			     gpointer            user_data);

	void (*moved)       (glLabelObject     *object,
			     gdouble            dx,
			     gdouble            dy,
			     gpointer           user_data);

	void (*flip_rotate) (glLabelObject     *object,
			     gpointer           user_data);

	void (*top)         (glLabelObject     *object,
			     gpointer           user_data);

	void (*bottom)      (glLabelObject     *object,
			     gpointer           user_data);
};

GType     gl_label_object_get_type              (void);

GObject  *gl_label_object_new                   (glLabel           *label);


void      gl_label_object_copy_props            (glLabelObject     *dst_object,
						 glLabelObject     *src_object);

void      gl_label_object_emit_changed          (glLabelObject     *object);


void      gl_label_object_set_parent            (glLabelObject     *object,
						 glLabel           *label);

glLabel  *gl_label_object_get_parent            (glLabelObject     *object);


void      gl_label_object_set_name              (glLabelObject     *object,
						 gchar             *name);

gchar    *gl_label_object_get_name              (glLabelObject     *object);


void      gl_label_object_set_position          (glLabelObject     *object,
						 gdouble            x,
						 gdouble            y);

void      gl_label_object_set_position_relative (glLabelObject     *object,
						 gdouble            dx,
						 gdouble            dy);

void      gl_label_object_get_position          (glLabelObject     *object,
						 gdouble           *x,
						 gdouble           *y);

void      gl_label_object_set_size              (glLabelObject     *object,
						 gdouble            w,
						 gdouble            h);

void      gl_label_object_get_size              (glLabelObject     *object,
						 gdouble           *w,
						 gdouble           *h);

void      gl_label_object_get_extent            (glLabelObject     *object,
						 gdouble           *x1,
						 gdouble           *y1,
						 gdouble           *x2,
						 gdouble           *y2);


void      gl_label_object_raise_to_top          (glLabelObject     *object);

void      gl_label_object_lower_to_bottom       (glLabelObject     *object);


void      gl_label_object_flip_horiz            (glLabelObject     *object);

void      gl_label_object_flip_vert             (glLabelObject     *object);

void      gl_label_object_rotate                (glLabelObject     *object,
						 gdouble            theta_degs);

void      gl_label_object_set_affine            (glLabelObject     *object,
						 gdouble           affine[6]);

void      gl_label_object_get_affine            (glLabelObject     *object,
						 gdouble           affine[6]);

void      gl_label_object_get_applied_affine    (glLabelObject     *object,
						 gdouble           affine[6]);

G_END_DECLS

#endif /* __LABEL_OBJECT_H__ */
