/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prop_bc_data.h:  barcode data widget module header file
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

#ifndef __PROP_BC_DATA_H__
#define __PROP_BC_DATA_H__

#include <gnome.h>

#define GL_TYPE_PROP_BC_DATA (gl_prop_bc_data_get_type ())
#define GL_PROP_BC_DATA(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_PROP_BC_DATA, glPropBCData ))
#define GL_PROP_BC_DATA_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_PROP_BC_DATA, glPropBCDataClass))
#define GL_IS_PROP_BC_DATA(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_PROP_BC_DATA))
#define GL_IS_PROP_BC_DATA_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_PROP_BC_DATA))

typedef struct _glPropBCData glPropBCData;
typedef struct _glPropBCDataClass glPropBCDataClass;

struct _glPropBCData {
	GtkVBox parent_widget;

	GtkWidget *literal_radio;
	GtkWidget *literal_entry;

	GtkWidget *key_radio;
	GtkWidget *key_entry;
};

struct _glPropBCDataClass {
	GtkVBoxClass parent_class;

	void (*changed) (glPropBCData * bc_data, gpointer user_data);
};

extern guint gl_prop_bc_data_get_type (void);

extern GtkWidget *gl_prop_bc_data_new (gchar * label, GList * field_defs);

extern void gl_prop_bc_data_get_data (glPropBCData * bc_data,
				      gboolean * field_flag,
				      gchar ** data,
				      gchar ** key);

extern void gl_prop_bc_data_set_data (glPropBCData * bc_data,
				      gboolean merge_flag,
				      gboolean field_flag,
				      gchar * data,
				      gchar * key);

#endif
