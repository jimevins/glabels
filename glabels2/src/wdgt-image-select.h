/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_image_select.h:  image selection widget module header file
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

#ifndef __WDGT_IMAGE_SELECT_H__
#define __WDGT_IMAGE_SELECT_H__

#include <gnome.h>
#include "text-node.h"
#include "hig.h"

G_BEGIN_DECLS

#define GL_TYPE_WDGT_IMAGE_SELECT (gl_wdgt_image_select_get_type ())
#define GL_WDGT_IMAGE_SELECT(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_WDGT_IMAGE_SELECT, glWdgtImageSelect ))
#define GL_WDGT_IMAGE_SELECT_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_WDGT_IMAGE_SELECT, glWdgtImageSelectClass))
#define GL_IS_WDGT_IMAGE_SELECT(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_WDGT_IMAGE_SELECT))
#define GL_IS_WDGT_IMAGE_SELECT_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_WDGT_IMAGE_SELECT))

typedef struct _glWdgtImageSelect      glWdgtImageSelect;
typedef struct _glWdgtImageSelectClass glWdgtImageSelectClass;

struct _glWdgtImageSelect {
	glHigVBox  parent_widget;

	GtkWidget *file_radio;
	GtkWidget *file_entry;

	GtkWidget *key_radio;
	GtkWidget *key_combo;
	GtkWidget *key_entry;
};

struct _glWdgtImageSelectClass {
	glHigVBoxClass parent_class;

	void (*changed) (glWdgtImageSelect *image_select,
			 gpointer           user_data);
};

GType       gl_wdgt_image_select_get_type             (void) G_GNUC_CONST;

GtkWidget  *gl_wdgt_image_select_new                  (glMerge           *merge,
						       glTextNode        *text_node);

void        gl_wdgt_image_select_set_field_defs       (glWdgtImageSelect *image_select,
						       glMerge           *merge);

glTextNode *gl_wdgt_image_select_get_data             (glWdgtImageSelect *image_select);

void        gl_wdgt_image_select_set_data             (glWdgtImageSelect *image_select,
						       gboolean           merge_flag,
						       glTextNode        *text_node);

void        gl_wdgt_image_select_set_label_size_group (glWdgtImageSelect *image_select,
						       GtkSizeGroup      *label_size_group);

G_END_DECLS

#endif
