/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  mini_preview.h:  mini-preview widget module header file
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

#ifndef __MINI_PREVIEW_H__
#define __MINI_PREVIEW_H__

#include <gnome.h>
#include "label.h"

#define GL_TYPE_MINI_PREVIEW (gl_mini_preview_get_type ())
#define GL_MINI_PREVIEW(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_MINI_PREVIEW, glMiniPreview ))
#define GL_MINI_PREVIEW_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_MINI_PREVIEW, glMiniPreviewClass))
#define GL_IS_MINI_PREVIEW(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_MINI_PREVIEW))
#define GL_IS_MINI_PREVIEW_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_MINI_PREVIEW))

typedef struct _glMiniPreview glMiniPreview;
typedef struct _glMiniPreviewClass glMiniPreviewClass;

struct _glMiniPreview {
	GtkHBox parent_widget;

	gint height;
	gint width;

	GtkWidget *canvas;
	GnomeCanvasItem *paper_item;

	gint labels_per_sheet;
	GList *label_items;
};

struct _glMiniPreviewClass {
	GtkHBoxClass parent_class;

	void (*clicked) (glMiniPreview *preview,
			 gint index, gpointer user_data);
	void (*pressed) (glMiniPreview *preview,
			 gint index1, gint index2,
			 gpointer user_data);
};

extern guint gl_mini_preview_get_type (void);

extern GtkWidget *gl_mini_preview_new (gint height, gint width);

extern void gl_mini_preview_set_label (glMiniPreview * preview,
				       gchar * name);

extern void gl_mini_preview_highlight_range (glMiniPreview * preview,
					     gint first_label,
					     gint last_label);

#endif
