/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label.h:  GLabels label module header file
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
#ifndef __LABEL_H__
#define __LABEL_H__

#include <glib-object.h>

#include "merge.h"
#include "template.h"
#include "pixbuf-cache.h"

G_BEGIN_DECLS

#define GL_TYPE_LABEL            (gl_label_get_type ())
#define GL_LABEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_LABEL, glLabel))
#define GL_LABEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_LABEL, glLabelClass))
#define GL_IS_LABEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_LABEL))
#define GL_IS_LABEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_LABEL))

typedef struct _glLabel          glLabel;
typedef struct _glLabelClass     glLabelClass;

typedef struct _glLabelPrivate   glLabelPrivate;

#include "label-object.h"
struct _glLabel {
	GObject         object;

	GList          *objects;

	glLabelPrivate *private;
};

struct _glLabelClass {
	GObjectClass         parent_class;

	void (*changed)          (glLabel *label, gpointer user_data);

	void (*name_changed)     (glLabel *label, gpointer user_data);

	void (*modified_changed) (glLabel *label, gpointer user_data);

	void (*merge_changed)    (glLabel *label, gpointer user_data);

	void (*size_changed)     (glLabel *label, gpointer user_data);

};


GType         gl_label_get_type                (void);

GObject      *gl_label_new                     (void);


void          gl_label_add_object              (glLabel       *label,
						glLabelObject *object);

void          gl_label_remove_object           (glLabel       *label,
						glLabelObject *object);

void          gl_label_raise_object_to_top     (glLabel       *label,
						glLabelObject *object);

void          gl_label_lower_object_to_bottom  (glLabel       *label,
						glLabelObject *object);


void          gl_label_set_template            (glLabel       *label,
						glTemplate    *template);

void          gl_label_set_rotate_flag         (glLabel       *label,
						gboolean       rotate_flag);


glTemplate   *gl_label_get_template            (glLabel       *label);

gboolean      gl_label_get_rotate_flag         (glLabel       *label);

void          gl_label_get_size                (glLabel       *label,
						gdouble       *w,
						gdouble       *h);


void          gl_label_set_merge               (glLabel       *label,
						glMerge       *merge);

glMerge      *gl_label_get_merge               (glLabel       *label);


gchar        *gl_label_get_filename            (glLabel       *label);

gchar        *gl_label_get_short_name          (glLabel       *label);

GHashTable   *gl_label_get_pixbuf_cache        (glLabel       *label);

gboolean      gl_label_is_modified             (glLabel       *label);

gboolean      gl_label_is_untitled             (glLabel       *label);

gboolean      gl_label_can_undo                (glLabel       *label);

gboolean      gl_label_can_redo                (glLabel       *label);


void          gl_label_set_filename            (glLabel       *label,
						const gchar   *filename);

void          gl_label_clear_modified          (glLabel       *label);

G_END_DECLS


#endif /* __LABEL_H__ */
