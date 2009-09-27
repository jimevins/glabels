/*
 *  label-line.h
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

#ifndef __LABEL_LINE_H__
#define __LABEL_LINE_H__

#include "label-object.h"


G_BEGIN_DECLS


#define GL_TYPE_LABEL_LINE            (gl_label_line_get_type ())
#define GL_LABEL_LINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_LABEL_LINE, glLabelLine))
#define GL_LABEL_LINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_LABEL_LINE, glLabelLineClass))
#define GL_IS_LABEL_LINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_LABEL_LINE))
#define GL_IS_LABEL_LINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_LABEL_LINE))

typedef struct _glLabelLine          glLabelLine;
typedef struct _glLabelLineClass     glLabelLineClass;

typedef struct _glLabelLinePrivate   glLabelLinePrivate;

struct _glLabelLine {
	glLabelObject         object;

	glLabelLinePrivate   *priv;
};

struct _glLabelLineClass {
	glLabelObjectClass    parent_class;
};

GType        gl_label_line_get_type       (void) G_GNUC_CONST;

GObject     *gl_label_line_new            (glLabel     *label);


G_END_DECLS

#endif /* __LABEL_LINE_H__ */




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
