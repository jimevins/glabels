/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  rotate_select.h:  label rotate selection widget module header file
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

#ifndef __ROTATE_SELECT_H__
#define __ROTATE_SELECT_H__

#include <gnome.h>
#include "template.h"

#define GL_TYPE_ROTATE_SELECT (gl_rotate_select_get_type ())
#define GL_ROTATE_SELECT(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_ROTATE_SELECT, glRotateSelect ))
#define GL_ROTATE_SELECT_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_ROTATE_SELECT, glRotateSelectClass))
#define GL_IS_ROTATE_SELECT(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_ROTATE_SELECT))
#define GL_IS_ROTATE_SELECT_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_ROTATE_SELECT))

typedef struct _glRotateSelect glRotateSelect;
typedef struct _glRotateSelectClass glRotateSelectClass;

struct _glRotateSelect {
	GtkHBox parent_widget;

	GtkWidget *rotate_check;

	GtkWidget *canvas;

	glTemplate *template;
};

struct _glRotateSelectClass {
	GtkHBoxClass parent_class;

	void (*changed) (glRotateSelect * rotate_select, gpointer user_data);
};

extern guint gl_rotate_select_get_type (void);

extern GtkWidget *gl_rotate_select_new (void);

extern gboolean gl_rotate_select_get_state (glRotateSelect * rotate_select);

extern void gl_rotate_select_set_state (glRotateSelect * rotate_select,
					gboolean state);

extern void gl_rotate_select_set_template_name (glRotateSelect * rotate_select,
						gchar * name);

#endif
