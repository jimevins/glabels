/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prop_fill.h:  fill properties widget module header file
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

#ifndef __PROP_FILL_H__
#define __PROP_FILL_H__

#include <gnome.h>
#include "label.h"

#define GL_TYPE_PROP_FILL (gl_prop_fill_get_type ())
#define GL_PROP_FILL(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_PROP_FILL, glPropFill ))
#define GL_PROP_FILL_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_PROP_FILL, glPropFillClass))
#define GL_IS_PROP_FILL(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_PROP_FILL))
#define GL_IS_PROP_FILL_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_PROP_FILL))

typedef struct _glPropFill glPropFill;
typedef struct _glPropFillClass glPropFillClass;

struct _glPropFill {
	GtkVBox parent_widget;

	GtkWidget *color_picker;
};

struct _glPropFillClass {
	GtkVBoxClass parent_class;

	void (*changed) (glPropFill * fill, gpointer user_data);
};

extern guint gl_prop_fill_get_type (void);

extern GtkWidget *gl_prop_fill_new (gchar * label);

extern void gl_prop_fill_get_params (glPropFill * fill, guint * color);

extern void gl_prop_fill_set_params (glPropFill * fill, guint color);

#endif
