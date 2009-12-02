/*
 *  window.h
 *  Copyright (C) 2002-2009  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of gLabels.
 *
 *  gLabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gLabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gLabels.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <gtk/gtk.h>

#include "view.h"
#include "label.h"
#include "ui-property-bar.h"
#include "ui-sidebar.h"
#include "print-op.h"
#include "merge-properties-dialog.h"

G_BEGIN_DECLS

#define GL_TYPE_WINDOW (gl_window_get_type ())
#define GL_WINDOW(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), GL_TYPE_WINDOW, glWindow ))
#define GL_WINDOW_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_WINDOW, glWindowClass))
#define GL_IS_WINDOW(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_WINDOW))
#define GL_IS_WINDOW_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_WINDOW))

typedef struct _glWindow      glWindow;
typedef struct _glWindowClass glWindowClass;

struct _glWindow {
	GtkWindow               parent_widget;

	GtkUIManager            *ui;

	GtkWidget               *hbox;

	glLabel                 *label;

	GtkWidget               *view;
	glUIPropertyBar         *property_bar;
	glUISidebar             *sidebar;

	GtkWidget               *status_bar;
	GtkWidget               *cursor_info;
	GtkWidget               *cursor_info_frame;
	GtkWidget               *zoom_info;
	GtkWidget               *zoom_info_frame;

	guint                    menu_tips_context_id;

        glPrintOpSettings       *print_settings;
	glMergePropertiesDialog *merge_dialog;
	GtkMenu                 *context_menu;
	GtkMenu                 *empty_selection_context_menu;
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



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
