/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prop_line.h:  line properties widget module header file
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

#ifndef __PROP_LINE_H__
#define __PROP_LINE_H__

#include <gnome.h>
#include "label.h"

#define GL_TYPE_PROP_LINE (gl_prop_line_get_type ())
#define GL_PROP_LINE(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_PROP_LINE, glPropLine ))
#define GL_PROP_LINE_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_PROP_LINE, glPropLineClass))
#define GL_IS_PROP_LINE(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_PROP_LINE))
#define GL_IS_PROP_LINE_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_PROP_LINE))

typedef struct _glPropLine glPropLine;
typedef struct _glPropLineClass glPropLineClass;

struct _glPropLine {
	GtkVBox parent_widget;

	GtkWidget *width_spin;
	GtkWidget *color_picker;
	GtkWidget *units_label;
};

struct _glPropLineClass {
	GtkVBoxClass parent_class;

	void (*changed) (glPropLine * line, gpointer user_data);
};

extern guint gl_prop_line_get_type (void);

extern GtkWidget *gl_prop_line_new (gchar * label);

extern void gl_prop_line_get_params (glPropLine * line,
				     gdouble * width,
				     guint * color);

extern void gl_prop_line_set_params (glPropLine * line,
				     gdouble width,
				     guint color);

#endif
