/*
 *  mini-label-preview.h
 *  Copyright (C) 2009  Jim Evins <evins@snaught.com>.
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

#ifndef __MINI_LABEL_PREVIEW_H__
#define __MINI_LABEL_PREVIEW_H__


#include <gtk/gtk.h>


G_BEGIN_DECLS

#define GL_TYPE_MINI_LABEL_PREVIEW (gl_mini_label_preview_get_type ())
#define GL_MINI_LABEL_PREVIEW(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), GL_TYPE_MINI_LABEL_PREVIEW, glMiniLabelPreview ))
#define GL_MINI_LABEL_PREVIEW_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_MINI_LABEL_PREVIEW, glMiniLabelPreviewClass))
#define GL_IS_MINI_LABEL_PREVIEW(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_MINI_LABEL_PREVIEW))
#define GL_IS_MINI_LABEL_PREVIEW_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_MINI_LABEL_PREVIEW))

typedef struct _glMiniLabelPreview        glMiniLabelPreview;
typedef struct _glMiniLabelPreviewPrivate glMiniLabelPreviewPrivate;
typedef struct _glMiniLabelPreviewClass   glMiniLabelPreviewClass;

struct _glMiniLabelPreview {
	GtkDrawingArea               parent_widget;

	glMiniLabelPreviewPrivate   *priv;
};

struct _glMiniLabelPreviewClass {
	GtkDrawingAreaClass          parent_class;
};


GType      gl_mini_label_preview_get_type (void) G_GNUC_CONST;

GtkWidget *gl_mini_label_preview_new         (gint                w,
                                              gint                h);

void       gl_mini_label_preview_set_by_name (glMiniLabelPreview *this,
                                              gchar              *name,
                                              gboolean            rotate_flag);



G_END_DECLS

#endif /* __MINI_LABEL_PREVIEW_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
