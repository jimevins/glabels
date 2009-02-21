/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_rotate_label.h:  label rotate selection widget module header file
 *
 *  Copyright (C) 2001-2006  Jim Evins <evins@snaught.com>.
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

#ifndef __WDGT_ROTATE_LABEL_H__
#define __WDGT_ROTATE_LABEL_H__

#include <gtk/gtkhbox.h>

G_BEGIN_DECLS

#define GL_TYPE_WDGT_ROTATE_LABEL (gl_wdgt_rotate_label_get_type ())
#define GL_WDGT_ROTATE_LABEL(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), GL_TYPE_WDGT_ROTATE_LABEL, glWdgtRotateLabel ))
#define GL_WDGT_ROTATE_LABEL_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_WDGT_ROTATE_LABEL, glWdgtRotateLabelClass))
#define GL_IS_WDGT_ROTATE_LABEL(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_WDGT_ROTATE_LABEL))
#define GL_IS_WDGT_ROTATE_LABEL_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_WDGT_ROTATE_LABEL))

typedef struct _glWdgtRotateLabel         glWdgtRotateLabel;
typedef struct _glWdgtRotateLabelClass    glWdgtRotateLabelClass;

typedef struct _glWdgtRotateLabelPrivate  glWdgtRotateLabelPrivate;

struct _glWdgtRotateLabel {
	GtkHBox                    parent_widget;

	glWdgtRotateLabelPrivate  *priv;
};

struct _glWdgtRotateLabelClass {
	GtkHBoxClass parent_class;

	void (*changed) (glWdgtRotateLabel  *wdgt_rotate_label,
			 gpointer            user_data);
};

GType      gl_wdgt_rotate_label_get_type          (void) G_GNUC_CONST;

GtkWidget *gl_wdgt_rotate_label_new               (void);

gboolean   gl_wdgt_rotate_label_get_state         (glWdgtRotateLabel *rotate_label);

void       gl_wdgt_rotate_label_set_state         (glWdgtRotateLabel *rotate_label,
						   gboolean           state);

void       gl_wdgt_rotate_label_set_template_name (glWdgtRotateLabel *rotate_label,
						   gchar             *name);

G_END_DECLS

#endif
