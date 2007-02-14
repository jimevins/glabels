/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label_text.h:  GLabels label text object
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

#ifndef __LABEL_TEXT_H__
#define __LABEL_TEXT_H__

#include <gtk/gtktextbuffer.h>
#include "text-node.h"
#include "label-object.h"

G_BEGIN_DECLS

#define GL_LABEL_TEXT_MARGIN 3.0


#define GL_TYPE_LABEL_TEXT            (gl_label_text_get_type ())
#define GL_LABEL_TEXT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_LABEL_TEXT, glLabelText))
#define GL_LABEL_TEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_LABEL_TEXT, glLabelTextClass))
#define GL_IS_LABEL_TEXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_LABEL_TEXT))
#define GL_IS_LABEL_TEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_LABEL_TEXT))

typedef struct _glLabelText          glLabelText;
typedef struct _glLabelTextClass     glLabelTextClass;

typedef struct _glLabelTextPrivate   glLabelTextPrivate;

struct _glLabelText {
	glLabelObject         object;

	glLabelTextPrivate   *priv;
};

struct _glLabelTextClass {
	glLabelObjectClass    parent_class;
};

GType          gl_label_text_get_type     (void) G_GNUC_CONST;

GObject       *gl_label_text_new          (glLabel          *label);

void           gl_label_text_set_lines    (glLabelText      *ltext,
					   GList            *lines);
GtkTextBuffer *gl_label_text_get_buffer   (glLabelText      *ltext);
GList         *gl_label_text_get_lines    (glLabelText      *ltext);
void           gl_label_text_get_box      (glLabelText      *ltext,
					   gdouble          *w,
					   gdouble          *h);

void           gl_label_text_set_auto_shrink (glLabelText      *ltext,
					      gboolean          auto_shrink);
gboolean       gl_label_text_get_auto_shrink (glLabelText      *ltext);


G_END_DECLS

#endif /* __LABEL_TEXT_H__ */
