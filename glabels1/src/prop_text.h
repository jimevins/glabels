/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prop_text.h:  text properties widget module header file
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

#ifndef __PROP_TEXT_H__
#define __PROP_TEXT_H__

#include <gnome.h>
#include "label.h"

#define GL_TYPE_PROP_TEXT (gl_prop_text_get_type ())
#define GL_PROP_TEXT(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_PROP_TEXT, glPropText ))
#define GL_PROP_TEXT_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_PROP_TEXT, glPropTextClass))
#define GL_IS_PROP_TEXT(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_PROP_TEXT))
#define GL_IS_PROP_TEXT_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_PROP_TEXT))

typedef struct _glPropText glPropText;
typedef struct _glPropTextClass glPropTextClass;

struct _glPropText {
	GtkVBox parent_widget;

	GtkWidget *font_family_entry;
	GtkWidget *font_size_spin;
	GtkWidget *font_b_button;
	GtkWidget *font_i_button;

	GtkWidget *color_picker;

	GtkWidget *left_button, *right_button, *center_button;
};

struct _glPropTextClass {
	GtkVBoxClass parent_class;

	void (*changed) (glPropText * text, gpointer user_data);
};

extern guint gl_prop_text_get_type (void);

extern GtkWidget *gl_prop_text_new (gchar * label);

extern void gl_prop_text_get_params (glPropText * text,
				     gchar ** font_family,
				     gdouble * font_size,
				     GnomeFontWeight * font_weight,
				     gboolean * font_italic_flag,
				     guint * color,
				     GtkJustification * just);

extern void gl_prop_text_set_params (glPropText * text,
				     gchar * font_family,
				     gdouble font_size,
				     GnomeFontWeight font_weight,
				     gboolean font_italic_flag,
				     guint color,
				     GtkJustification just);

#endif
