/*
 *  label-text.h
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

#ifndef __LABEL_TEXT_H__
#define __LABEL_TEXT_H__

#include <gtk/gtk.h>
#include "text-node.h"
#include "label-object.h"

G_BEGIN_DECLS

#define GL_LABEL_TEXT_MARGIN 3.0


#define GL_TYPE_LABEL_TEXT              (gl_label_text_get_type ())
#define GL_LABEL_TEXT(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_LABEL_TEXT, glLabelText))
#define GL_LABEL_TEXT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_LABEL_TEXT, glLabelTextClass))
#define GL_IS_LABEL_TEXT(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_LABEL_TEXT))
#define GL_IS_LABEL_TEXT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_LABEL_TEXT))
#define GL_LABEL_TEXT_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), GL_TYPE_LABEL_TEXT, glLabelTextClass))


typedef struct _glLabelText          glLabelText;
typedef struct _glLabelTextPrivate   glLabelTextPrivate;
typedef struct _glLabelTextClass     glLabelTextClass;


struct _glLabelText {
	glLabelObject         object;

	glLabelTextPrivate   *priv;
};

struct _glLabelTextClass {
	glLabelObjectClass    parent_class;
};


GType          gl_label_text_get_type        (void) G_GNUC_CONST;

GObject       *gl_label_text_new             (glLabel          *label,
                                              gboolean          checkpoint);

void           gl_label_text_set_lines       (glLabelText      *ltext,
                                              GList            *lines,
                                              gboolean          checkpoint);

void           gl_label_text_set_text        (glLabelText      *ltext,
                                              const gchar      *text,
                                              gboolean          checkpoint);

GtkTextBuffer *gl_label_text_get_buffer      (glLabelText      *ltext);

gchar         *gl_label_text_get_text        (glLabelText      *ltext);

GList         *gl_label_text_get_lines       (glLabelText      *ltext);

void           gl_label_text_set_auto_shrink (glLabelText      *ltext,
					      gboolean          auto_shrink,
                                              gboolean          checkpoint);

gboolean       gl_label_text_get_auto_shrink (glLabelText      *ltext);

void           gl_label_text_set_merge_nonl (glLabelText      *ltext,
					      gboolean          merge_nonl,
                                              gboolean          checkpoint);

gboolean       gl_label_text_get_merge_nonl (glLabelText      *ltext);


G_END_DECLS

#endif /* __LABEL_TEXT_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
