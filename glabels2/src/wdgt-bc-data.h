/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_bc_data.h:  barcode data widget module header file
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

#ifndef __WDGT_BC_DATA_H__
#define __WDGT_BC_DATA_H__

#include <gnome.h>
#include "text-node.h"
#include "hig.h"

G_BEGIN_DECLS

#define GL_TYPE_WDGT_BC_DATA (gl_wdgt_bc_data_get_type ())
#define GL_WDGT_BC_DATA(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_WDGT_BC_DATA, glWdgtBCData ))
#define GL_WDGT_BC_DATA_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_WDGT_BC_DATA, glWdgtBCDataClass))
#define GL_IS_WDGT_BC_DATA(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_WDGT_BC_DATA))
#define GL_IS_WDGT_BC_DATA_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_WDGT_BC_DATA))

typedef struct _glWdgtBCData glWdgtBCData;
typedef struct _glWdgtBCDataClass glWdgtBCDataClass;

struct _glWdgtBCData {
	glHigVBox  parent_widget;

	GtkWidget *literal_radio;
	GtkWidget *literal_entry;

	GtkWidget *key_radio;
	GtkWidget *key_entry;
};

struct _glWdgtBCDataClass {
	glHigVBoxClass parent_class;

	void (*changed) (glWdgtBCData * bc_data, gpointer user_data);
};

guint       gl_wdgt_bc_data_get_type        (void);

GtkWidget  *gl_wdgt_bc_data_new             (GList        *field_defs);

void        gl_wdgt_bc_data_set_field_defs  (glWdgtBCData *bc_data,
					     GList        *field_defs);

glTextNode *gl_wdgt_bc_data_get_data        (glWdgtBCData *bc_data);

void        gl_wdgt_bc_data_set_data        (glWdgtBCData *bc_data,
					     gboolean      merge_flag,
					     glTextNode   *text_node);

void        gl_wdgt_bc_data_set_label_size_group (glWdgtBCData *bc_data,
						  GtkSizeGroup *label_size_group);

G_END_DECLS

#endif
