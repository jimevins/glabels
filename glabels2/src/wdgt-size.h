/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_size.h:  size properties widget module header file
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

#ifndef __WDGT_SIZE_H__
#define __WDGT_SIZE_H__

#include <gnome.h>
#include "label.h"
#include "hig.h"

G_BEGIN_DECLS

#define GL_TYPE_WDGT_SIZE (gl_wdgt_size_get_type ())
#define GL_WDGT_SIZE(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_WDGT_SIZE, glWdgtSize ))
#define GL_WDGT_SIZE_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_WDGT_SIZE, glWdgtSizeClass))
#define GL_IS_WDGT_SIZE(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_WDGT_SIZE))
#define GL_IS_WDGT_SIZE_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_WDGT_SIZE))

typedef struct _glWdgtSize glWdgtSize;
typedef struct _glWdgtSizeClass glWdgtSizeClass;

struct _glWdgtSize {
	glHigVBox  parent_widget;

	gdouble    w, h;
	gdouble    aspect_ratio;
	gdouble    w_max, h_max;
	gdouble    w_max_orig, h_max_orig;

	GtkWidget *w_label;
	GtkWidget *w_spin;

	GtkWidget *h_label;
	GtkWidget *h_spin;
	GtkWidget *units_label;

	GtkWidget *aspect_checkbox;
};

struct _glWdgtSizeClass {
	glHigVBoxClass parent_class;

	void (*changed) (glWdgtSize * size, gpointer user_data);
};

guint      gl_wdgt_size_get_type             (void);

GtkWidget *gl_wdgt_size_new                  (void);

void       gl_wdgt_size_get_size             (glWdgtSize   *size,
					      gdouble      *w,
					      gdouble      *h,
					      gboolean     *keep_aspect_ratio_flag);

void       gl_wdgt_size_set_params           (glWdgtSize   *size,
					      gdouble       w,
					      gdouble       h,
					      gboolean      keep_aspect_ratio_flag,
					      gdouble       w_max,
					      gdouble       h_max);

void       gl_wdgt_size_set_size             (glWdgtSize   *size,
					      gdouble       w,
					      gdouble       h);

void       gl_wdgt_size_set_label_size_group (glWdgtSize   *size,
					      GtkSizeGroup *label_size_group);


G_END_DECLS

#endif
