/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  ui-property-bar.h:  Property toolbar header file
 *
 *  Copyright (C) 2003  Jim Evins <evins@snaught.com>.
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

#ifndef __UI_PROPERTY_BAR_H__
#define __UI_PROPERTY_BAR_H__

#include <glib-object.h>
#include <bonobo/bonobo-ui-component.h>
#include <bonobo/bonobo-window.h>

#include "view.h"

G_BEGIN_DECLS

#define GL_TYPE_UI_PROPERTY_BAR (gl_ui_property_bar_get_type ())
#define GL_UI_PROPERTY_BAR(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_UI_PROPERTY_BAR, glUIPropertyBar ))
#define GL_UI_PROPERTY_BAR_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_UI_PROPERTY_BAR, glUIPropertyBarClass))
#define GL_IS_UI_PROPERTY_BAR(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_UI_PROPERTY_BAR))
#define GL_IS_UI_PROPERTY_BAR_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_UI_PROPERTY_BAR))

typedef struct _glUIPropertyBar      glUIPropertyBar;
typedef struct _glUIPropertyBarClass glUIPropertyBarClass;

struct _glUIPropertyBar {
	GObject              parent_widget;

	BonoboUIComponent   *ui_component;

	glView              *view;

	GtkWidget           *font_family_entry;
	GtkWidget           *font_size_spin;
	GtkWidget           *text_color_combo;

	GtkWidget           *fill_color_combo;

	GtkWidget           *line_color_combo;
	GtkWidget           *line_width_spin;
};

struct _glUIPropertyBarClass {
	GObjectClass         parent_class;
};

guint        gl_ui_property_bar_get_type          (void);

GObject     *gl_ui_property_bar_new               (BonoboUIComponent *ui_component);

void         gl_ui_property_bar_set_view          (glUIPropertyBar *property_bar,
						   glView          *view);


G_END_DECLS

#endif /* __UI_PROPERTY_BAR_H__ */
