/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_rotate_label.h:  label rotate selection widget module header file
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

#ifndef __WDGT_ROTATE_LABEL_H__
#define __WDGT_ROTATE_LABEL_H__

#include <gnome.h>
#include "hig.h"
#include "template.h"

#define GL_TYPE_WDGT_ROTATE_LABEL (gl_wdgt_rotate_label_get_type ())
#define GL_WDGT_ROTATE_LABEL(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_WDGT_ROTATE_LABEL, glWdgtRotateLabel ))
#define GL_WDGT_ROTATE_LABEL_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_WDGT_ROTATE_LABEL, glWdgtRotateLabelClass))
#define GL_IS_WDGT_ROTATE_LABEL(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_WDGT_ROTATE_LABEL))
#define GL_IS_WDGT_ROTATE_LABEL_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_WDGT_ROTATE_LABEL))

typedef struct _glWdgtRotateLabel      glWdgtRotateLabel;
typedef struct _glWdgtRotateLabelClass glWdgtRotateLabelClass;

struct _glWdgtRotateLabel {
	glHigHBox   parent_widget;

	GtkWidget  *rotate_check;

	GtkWidget  *canvas;

	glTemplate *template;
};

struct _glWdgtRotateLabelClass {
	glHigHBoxClass parent_class;

	void (*changed) (glWdgtRotateLabel * wdgt_rotate_label,
			 gpointer user_data);
};

extern guint     gl_wdgt_rotate_label_get_type (void);

extern GtkWidget *gl_wdgt_rotate_label_new (void);

extern gboolean  gl_wdgt_rotate_label_get_state (glWdgtRotateLabel * wdgt_rotate_label);

extern void      gl_wdgt_rotate_label_set_state (glWdgtRotateLabel * wdgt_rotate_label,
						 gboolean state);

extern void      gl_wdgt_rotate_label_set_template_name (glWdgtRotateLabel * wdgt_rotate_label,
							 gchar * name);

#endif
