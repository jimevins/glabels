/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label_box.h:  GLabels label box object
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

#ifndef __LABEL_BOX_H__
#define __LABEL_BOX_H__

#include "label-object.h"


G_BEGIN_DECLS


#define GL_TYPE_LABEL_BOX            (gl_label_box_get_type ())
#define GL_LABEL_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_LABEL_BOX, glLabelBox))
#define GL_LABEL_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_LABEL_BOX, glLabelBoxClass))
#define GL_IS_LABEL_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_LABEL_BOX))
#define GL_IS_LABEL_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_LABEL_BOX))

typedef struct _glLabelBox          glLabelBox;
typedef struct _glLabelBoxClass     glLabelBoxClass;

typedef struct _glLabelBoxPrivate   glLabelBoxPrivate;

struct _glLabelBox {
	glLabelObject         object;

	glLabelBoxPrivate    *private;
};

struct _glLabelBoxClass {
	glLabelObjectClass    parent_class;
};

GType         gl_label_box_get_type       (void);

GObject      *gl_label_box_new            (glLabel    *label);

void          gl_label_box_set_line_width (glLabelBox *lbox,
					   gdouble     line_width);

void          gl_label_box_set_line_color (glLabelBox *lbox,
					   guint       line_color);

void          gl_label_box_set_fill_color (glLabelBox *lbox,
					   guint       fill_color);

gdouble       gl_label_box_get_line_width (glLabelBox *lbox);
guint         gl_label_box_get_line_color (glLabelBox *lbox);
guint         gl_label_box_get_fill_color (glLabelBox *lbox);


G_END_DECLS

#endif /* __LABEL_BOX_H__ */
