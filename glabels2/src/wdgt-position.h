/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_position.h:  position properties widget module header file
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

#ifndef __WDGT_POSITION_H__
#define __WDGT_POSITION_H__

#include <gnome.h>
#include "label.h"

#define GL_TYPE_WDGT_POSITION (gl_wdgt_position_get_type ())
#define GL_WDGT_POSITION(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_WDGT_POSITION, glWdgtPosition ))
#define GL_WDGT_POSITION_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_WDGT_POSITION, glWdgtPositionClass))
#define GL_IS_WDGT_POSITION(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_WDGT_POSITION))
#define GL_IS_WDGT_POSITION_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_WDGT_POSITION))

typedef struct _glWdgtPosition glWdgtPosition;
typedef struct _glWdgtPositionClass glWdgtPositionClass;

struct _glWdgtPosition {
	GtkVBox parent_widget;

	GtkWidget *x_spin;
	GtkWidget *y_spin;
	GtkWidget *units_label;
};

struct _glWdgtPositionClass {
	GtkVBoxClass parent_class;

	void (*changed) (glWdgtPosition * prop, gpointer user_data);
};

extern guint gl_wdgt_position_get_type (void);

extern GtkWidget *gl_wdgt_position_new (gchar * label);

extern void gl_wdgt_position_get_position (glWdgtPosition * position,
					   gdouble * x, gdouble * y);

extern void gl_wdgt_position_set_params (glWdgtPosition * position,
					 gdouble x, gdouble y,
					 gdouble x_max, gdouble y_max);

extern void gl_wdgt_position_set_position (glWdgtPosition * position,
					   gdouble x, gdouble y);

#endif
