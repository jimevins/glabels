/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_fill.h:  fill properties widget module header file
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

#ifndef __WDGT_FILL_H__
#define __WDGT_FILL_H__

#include <gnome.h>
#include "label.h"

#define GL_TYPE_WDGT_FILL (gl_wdgt_fill_get_type ())
#define GL_WDGT_FILL(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_WDGT_FILL, glWdgtFill ))
#define GL_WDGT_FILL_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_WDGT_FILL, glWdgtFillClass))
#define GL_IS_WDGT_FILL(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_WDGT_FILL))
#define GL_IS_WDGT_FILL_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_WDGT_FILL))

typedef struct _glWdgtFill glWdgtFill;
typedef struct _glWdgtFillClass glWdgtFillClass;

struct _glWdgtFill {
	GtkVBox parent_widget;

	GtkWidget *color_picker;
};

struct _glWdgtFillClass {
	GtkVBoxClass parent_class;

	void (*changed) (glWdgtFill * fill, gpointer user_data);
};

extern guint gl_wdgt_fill_get_type (void);

extern GtkWidget *gl_wdgt_fill_new (gchar * label);

extern void gl_wdgt_fill_get_params (glWdgtFill * fill, guint * color);

extern void gl_wdgt_fill_set_params (glWdgtFill * fill, guint color);

#endif
