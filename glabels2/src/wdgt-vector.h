/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_vector.h:  vector properties widget module header file
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

#ifndef __WDGT_VECTOR_H__
#define __WDGT_VECTOR_H__

#include <gnome.h>
#include "label.h"

#define GL_TYPE_WDGT_VECTOR (gl_wdgt_vector_get_type ())
#define GL_WDGT_VECTOR(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_WDGT_VECTOR, glWdgtVector ))
#define GL_WDGT_VECTOR_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_WDGT_VECTOR, glWdgtVectorClass))
#define GL_IS_WDGT_VECTOR(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_WDGT_VECTOR))
#define GL_IS_WDGT_VECTOR_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_WDGT_VECTOR))

typedef struct _glWdgtVector      glWdgtVector;
typedef struct _glWdgtVectorClass glWdgtVectorClass;

struct _glWdgtVector {
	GtkVBox parent_widget;

	GtkWidget *len_spin;
	GtkWidget *len_units_label;
	GtkWidget *angle_spin;
	GtkWidget *angle_units_label;
};

struct _glWdgtVectorClass {
	GtkVBoxClass parent_class;

	void (*changed) (glWdgtVector * vector, gpointer user_data);
};

extern guint gl_wdgt_vector_get_type (void);

extern GtkWidget *gl_wdgt_vector_new (gchar * label);

extern void gl_wdgt_vector_get_params (glWdgtVector * vector,
				       gdouble * dx, gdouble * dy);

extern void gl_wdgt_vector_set_params (glWdgtVector * vector,
				       gdouble dx, gdouble dy,
				       gdouble x_max, gdouble y_max);

#endif
