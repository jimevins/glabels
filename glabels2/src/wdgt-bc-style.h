/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_bc_style.h:  barcode style selection widget module header file
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

#ifndef __WDGT_BC_STYLE_H__
#define __WDGT_BC_STYLE_H__

#include <gnome.h>
#include "bc.h"
#include "hig.h"

G_BEGIN_DECLS

#define GL_TYPE_WDGT_BC_STYLE (gl_wdgt_bc_style_get_type ())
#define GL_WDGT_BC_STYLE(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_WDGT_BC_STYLE, glWdgtBCStyle ))
#define GL_WDGT_BC_STYLE_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_WDGT_BC_STYLE, glWdgtBCStyleClass))
#define GL_IS_WDGT_BC_STYLE(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_WDGT_BC_STYLE))
#define GL_IS_WDGT_BC_STYLE_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_WDGT_BC_STYLE))

typedef struct _glWdgtBCStyle glWdgtBCStyle;
typedef struct _glWdgtBCStyleClass glWdgtBCStyleClass;

struct _glWdgtBCStyle {
	glHigVBox parent_widget;

	GtkWidget *style_label;
	GtkWidget *style_entry;

	GtkWidget *text_check;
};

struct _glWdgtBCStyleClass {
	glHigVBoxClass parent_class;

	void (*changed) (glWdgtBCStyle * prop, gpointer user_data);
};

guint      gl_wdgt_bc_style_get_type   (void);

GtkWidget *gl_wdgt_bc_style_new        (void);

void       gl_wdgt_bc_style_get_params (glWdgtBCStyle  *bc_style,
					glBarcodeStyle *style,
					gboolean       *text_flag);

void       gl_wdgt_bc_style_set_params (glWdgtBCStyle  *bc_style,
					glBarcodeStyle  style,
					gboolean        text_flag);

void       gl_wdgt_bc_style_set_label_size_group (glWdgtBCStyle *bc_style,
						  GtkSizeGroup  *label_size_group);

G_END_DECLS

#endif
