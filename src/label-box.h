/*
 *  label-box.h
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

	glLabelBoxPrivate    *priv;
};

struct _glLabelBoxClass {
	glLabelObjectClass    parent_class;
};

GType         gl_label_box_get_type       (void) G_GNUC_CONST;

GObject      *gl_label_box_new            (glLabel    *label,
                                           gboolean    checkpoint);

G_END_DECLS

#endif /* __LABEL_BOX_H__ */




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
