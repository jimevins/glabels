/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prop_size.h:  size properties widget module header file
 *
 *  Copyright (C) 2000, 2001  Jim Evins <evins@snaught.com>.
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

#ifndef __PROP_SIZE_H__
#define __PROP_SIZE_H__

#include <gnome.h>
#include "label.h"

#define GL_TYPE_PROP_SIZE (gl_prop_size_get_type ())
#define GL_PROP_SIZE(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_PROP_SIZE, glPropSize ))
#define GL_PROP_SIZE_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_PROP_SIZE, glPropSizeClass))
#define GL_IS_PROP_SIZE(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_PROP_SIZE))
#define GL_IS_PROP_SIZE_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_PROP_SIZE))

typedef struct _glPropSize glPropSize;
typedef struct _glPropSizeClass glPropSizeClass;

struct _glPropSize {
	GtkVBox parent_widget;

	gdouble w, h;
	gdouble aspect_ratio;
	gdouble w_max, h_max;
	gdouble w_max_orig, h_max_orig;

	GtkWidget *w_spin;
	GtkWidget *h_spin;
	GtkWidget *units_label;
	GtkWidget *aspect_checkbox;
};

struct _glPropSizeClass {
	GtkVBoxClass parent_class;

	void (*changed) (glPropSize * size, gpointer user_data);
};

extern guint gl_prop_size_get_type (void);

extern GtkWidget *gl_prop_size_new (gchar * label);

extern void gl_prop_size_get_size (glPropSize * size,
				   gdouble * w,	gdouble * h,
				   gboolean * keep_aspect_ratio_flag);

extern void gl_prop_size_set_params (glPropSize * size,
				     gdouble w, gdouble h,
				     gboolean keep_aspect_ratio_flag,
				     gdouble w_max, gdouble h_max);

#endif
