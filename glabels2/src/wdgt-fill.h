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
#include "hig.h"

G_BEGIN_DECLS

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
	glHigVBox  parent_widget;

	GtkWidget *color_label;
	GtkWidget *color_picker;
};

struct _glWdgtFillClass {
	glHigVBoxClass parent_class;

	void (*changed) (glWdgtFill * fill, gpointer user_data);
};

guint      gl_wdgt_fill_get_type             (void);

GtkWidget *gl_wdgt_fill_new                  (void);

void       gl_wdgt_fill_get_params           (glWdgtFill *fill,
					      guint      *color);

void       gl_wdgt_fill_set_params           (glWdgtFill *fill,
					      guint       color);

void       gl_wdgt_fill_set_label_size_group (glWdgtFill   *fill,
					      GtkSizeGroup *label_size_group);

G_END_DECLS

#endif
