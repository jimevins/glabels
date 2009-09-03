/*
 *  mini-preview.h
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

#ifndef __MINI_PREVIEW_H__
#define __MINI_PREVIEW_H__

#include <gtk/gtk.h>
#include "label.h"


G_BEGIN_DECLS

#define GL_TYPE_MINI_PREVIEW (gl_mini_preview_get_type ())
#define GL_MINI_PREVIEW(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), GL_TYPE_MINI_PREVIEW, glMiniPreview ))
#define GL_MINI_PREVIEW_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_MINI_PREVIEW, glMiniPreviewClass))
#define GL_IS_MINI_PREVIEW(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_MINI_PREVIEW))
#define GL_IS_MINI_PREVIEW_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_MINI_PREVIEW))


typedef struct _glMiniPreview        glMiniPreview;
typedef struct _glMiniPreviewPrivate glMiniPreviewPrivate;
typedef struct _glMiniPreviewClass   glMiniPreviewClass;

struct _glMiniPreview {
	GtkEventBox           parent;

	glMiniPreviewPrivate *priv;
};

struct _glMiniPreviewClass {
	GtkEventBoxClass      parent_class;

	void (*clicked) (glMiniPreview *this,
			 gint           index,
			 gpointer       user_data);

	void (*pressed) (glMiniPreview *this,
			 gint           index1,
			 gint           index2,
			 gpointer       user_data);
};


GType      gl_mini_preview_get_type          (void) G_GNUC_CONST;

GtkWidget *gl_mini_preview_new               (gint               height,
                                              gint               width);

void       gl_mini_preview_set_label_by_name (glMiniPreview     *this,
                                              const gchar       *name);

void       gl_mini_preview_set_template      (glMiniPreview     *this,
                                              const lglTemplate *template);

void       gl_mini_preview_highlight_range   (glMiniPreview     *this,
                                              gint               first_label,
                                              gint               last_label);

G_END_DECLS

#endif



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
