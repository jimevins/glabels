/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_bc_props.h:  barcode properties widget module header file
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

#ifndef __WDGT_BC_PROPS_H__
#define __WDGT_BC_PROPS_H__

#include <gnome.h>
#include "bc.h"
#include "hig.h"

G_BEGIN_DECLS

#define GL_TYPE_WDGT_BC_PROPS (gl_wdgt_bc_props_get_type ())
#define GL_WDGT_BC_PROPS(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_WDGT_BC_PROPS, glWdgtBCProps ))
#define GL_WDGT_BC_PROPS_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_WDGT_BC_PROPS, glWdgtBCPropsClass))
#define GL_IS_WDGT_BC_PROPS(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_WDGT_BC_PROPS))
#define GL_IS_WDGT_BC_PROPS_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_WDGT_BC_PROPS))

typedef struct _glWdgtBCProps      glWdgtBCProps;
typedef struct _glWdgtBCPropsClass glWdgtBCPropsClass;

struct _glWdgtBCProps {
	glHigVBox  parent_widget;

	GtkWidget *scale_label;
	GtkWidget *scale_spin;

	GtkWidget *color_label;
	GtkWidget *color_picker;
};

struct _glWdgtBCPropsClass {
	glHigVBoxClass parent_class;

	void (*changed) (glWdgtBCProps * prop, gpointer user_data);
};

guint      gl_wdgt_bc_props_get_type       (void);

GtkWidget *gl_wdgt_bc_props_new            (void);

void       gl_wdgt_bc_props_get_params     (glWdgtBCProps *prop,
					    gdouble       *scale,
					    guint         *color);

void       gl_wdgt_bc_props_set_params     (glWdgtBCProps *prop,
					    gdouble        scale,
					    guint          color);

void       gl_wdgt_bc_props_set_label_size_group (glWdgtBCProps *prop,
						  GtkSizeGroup  *label_size_group);

G_END_DECLS

#endif
