/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/**
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  window.h:  a gLabels app window
 *
 *  Copyright (C) 2002  Jim Evins <evins@snaught.com>.
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

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <gtk/gtkwindow.h>
#include <gtk/gtkuimanager.h>

#include "view.h"
#include "label.h"

#include "ui-property-bar.h"
#include "ui-sidebar.h"

G_BEGIN_DECLS

#define GL_TYPE_WINDOW (gl_window_get_type ())
#define GL_WINDOW(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_WINDOW, glWindow ))
#define GL_WINDOW_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_WINDOW, glWindowClass))
#define GL_IS_WINDOW(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_WINDOW))
#define GL_IS_WINDOW_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_WINDOW))

typedef struct _glWindow      glWindow;
typedef struct _glWindowClass glWindowClass;

struct _glWindow {
	GtkWindow               parent_widget;

	GtkUIManager           *ui;

	GtkWidget              *view;

	GtkWidget              *hbox;

	glUIPropertyBar        *property_bar;
	glUISidebar            *sidebar;

	GtkWidget              *status_bar;
	GtkWidget              *cursor_info;
	GtkWidget              *cursor_info_frame;
	GtkWidget              *zoom_info;
	GtkWidget              *zoom_info_frame;

	guint                   menu_tips_context_id;
};

struct _glWindowClass {
	GtkWindowClass          parent_class;
};

GType        gl_window_get_type          (void) G_GNUC_CONST;

GtkWidget   *gl_window_new               (void);

GtkWidget   *gl_window_new_from_file     (const gchar *filename);

GtkWidget   *gl_window_new_from_label    (glLabel     *label);

gboolean     gl_window_is_empty          (glWindow    *window);

void         gl_window_set_label         (glWindow    *window,
					  glLabel     *label);

const GList *gl_window_get_window_list   (void);

G_END_DECLS

#endif /* __WINDOW_H__ */
