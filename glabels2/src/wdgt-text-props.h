/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_text_props.h:  text properties widget module header file
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

#ifndef __WDGT_TEXT_PROPS_H__
#define __WDGT_TEXT_PROPS_H__

#include <gtk/gtk.h>
#include <libgnomeprint/gnome-font.h>
#include "label.h"
#include "hig.h"

G_BEGIN_DECLS

#define GL_TYPE_WDGT_TEXT_PROPS (gl_wdgt_text_props_get_type ())
#define GL_WDGT_TEXT_PROPS(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_WDGT_TEXT_PROPS, glWdgtTextProps ))
#define GL_WDGT_TEXT_PROPS_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_WDGT_TEXT_PROPS, glWdgtTextPropsClass))
#define GL_IS_WDGT_TEXT_PROPS(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_WDGT_TEXT_PROPS))
#define GL_IS_WDGT_TEXT_PROPS_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_WDGT_TEXT_PROPS))

typedef struct _glWdgtTextProps glWdgtTextProps;
typedef struct _glWdgtTextPropsClass glWdgtTextPropsClass;

struct _glWdgtTextProps {
	glHigVBox  parent_widget;

	GtkWidget *font_label;
	GtkWidget *font_family_entry;
	GtkWidget *font_size_spin;
	GtkWidget *font_b_button;
	GtkWidget *font_i_button;

	GtkWidget *color_label;
	GtkWidget *color_picker;

	GtkWidget *alignment_label;
	GtkWidget *left_button, *right_button, *center_button;
};

struct _glWdgtTextPropsClass {
	glHigVBoxClass parent_class;

	void (*changed) (glWdgtTextProps * text, gpointer user_data);
};

guint      gl_wdgt_text_props_get_type   (void);

GtkWidget *gl_wdgt_text_props_new        (void);

void       gl_wdgt_text_props_get_params (glWdgtTextProps  *text,
					  gchar           **font_family,
					  gdouble          *font_size,
					  GnomeFontWeight  *font_weight,
					  gboolean         *font_italic_flag,
					  guint            *color,
					  GtkJustification *just);

void       gl_wdgt_text_props_set_params (glWdgtTextProps  *text,
					  gchar            *font_family,
					  gdouble           font_size,
					  GnomeFontWeight   font_weight,
					  gboolean          font_italic_flag,
					  guint             color,
					  GtkJustification  just);

void       gl_wdgt_text_props_set_label_size_group (glWdgtTextProps *text,
						    GtkSizeGroup    *size_grp);

G_END_DECLS

#endif
