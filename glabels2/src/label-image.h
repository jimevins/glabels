/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label_image.h:  GLabels label image object
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

#ifndef __LABEL_IMAGE_H__
#define __LABEL_IMAGE_H__

#include "label-object.h"
#include "text-node.h"
#include "merge.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS


#define GL_TYPE_LABEL_IMAGE            (gl_label_image_get_type ())
#define GL_LABEL_IMAGE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_LABEL_IMAGE, glLabelImage))
#define GL_LABEL_IMAGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_LABEL_IMAGE, glLabelImageClass))
#define GL_IS_LABEL_IMAGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_LABEL_IMAGE))
#define GL_IS_LABEL_IMAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_LABEL_IMAGE))

typedef struct _glLabelImage          glLabelImage;
typedef struct _glLabelImageClass     glLabelImageClass;

typedef struct _glLabelImagePrivate   glLabelImagePrivate;

struct _glLabelImage {
	glLabelObject         object;

	glLabelImagePrivate  *priv;
};

struct _glLabelImageClass {
	glLabelObjectClass    parent_class;
};

GType            gl_label_image_get_type     (void) G_GNUC_CONST;

GObject         *gl_label_image_new          (glLabel       *label);

void             gl_label_image_set_filename (glLabelImage  *limage,
					      glTextNode    *filename);

glTextNode      *gl_label_image_get_filename (glLabelImage  *limage);

const GdkPixbuf *gl_label_image_get_pixbuf   (glLabelImage  *limage,
					      glMergeRecord *record);

G_END_DECLS

#endif /* __LABEL_IMAGE_H__ */
