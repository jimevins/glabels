/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prop_bc.h:  barcode properties widget module header file
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

#ifndef __PROP_BC_H__
#define __PROP_BC_H__

#include <gnome.h>
#include "bc.h"

#define GL_TYPE_PROP_BC (gl_prop_bc_get_type ())
#define GL_PROP_BC(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_PROP_BC, glPropBC ))
#define GL_PROP_BC_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_PROP_BC, glPropBCClass))
#define GL_IS_PROP_BC(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_PROP_BC))
#define GL_IS_PROP_BC_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_PROP_BC))

typedef struct _glPropBC glPropBC;
typedef struct _glPropBCClass glPropBCClass;

struct _glPropBC {
	GtkVBox parent_widget;

	GtkWidget *scale_spin;
	GtkWidget *color_picker;
};

struct _glPropBCClass {
	GtkVBoxClass parent_class;

	void (*changed) (glPropBC * prop, gpointer user_data);
};

extern guint gl_prop_bc_get_type (void);

extern GtkWidget *gl_prop_bc_new (gchar * label);

extern void gl_prop_bc_get_params (glPropBC * prop,
				   gdouble * scale,
				   guint * color);

extern void gl_prop_bc_set_params (glPropBC * prop,
				   gdouble scale,
				   guint color);

#endif
