/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label_line.h:  GLabels label line object
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

	glLabelLinePrivate *private;
};

struct _glLabelLineClass {
	glLabelObjectClass    parent_class;
};

extern GType       gl_label_line_get_type       (void);

extern GObject    *gl_label_line_new            (glLabel *label);

extern glLabelLine *gl_label_line_dup        (glLabelLine *lline,
						    glLabel *label);

extern void        gl_label_line_set_line_width (glLabelLine *lline,
						    gdouble line_width);

extern void        gl_label_line_set_line_color (glLabelLine *lline,
						    guint line_color);

extern gdouble     gl_label_line_get_line_width (glLabelLine *lline);
extern guint       gl_label_line_get_line_color (glLabelLine *lline);


G_END_DECLS

#endif /* __LABEL_LINE_H__ */
