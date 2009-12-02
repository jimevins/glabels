/*
 *  ui-sidebar.h
 *  Copyright (C) 2003-2009  Jim Evins <evins@snaught.com>.
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

#ifndef __UI_SIDEBAR_H__
#define __UI_SIDEBAR_H__

#include <gtk/gtk.h>

#include "view.h"

G_BEGIN_DECLS

#define GL_TYPE_UI_SIDEBAR (gl_ui_sidebar_get_type ())
#define GL_UI_SIDEBAR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), GL_TYPE_UI_SIDEBAR, glUISidebar ))
#define GL_UI_SIDEBAR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_UI_SIDEBAR, glUISidebarClass))
#define GL_IS_UI_SIDEBAR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_UI_SIDEBAR))
#define GL_IS_UI_SIDEBAR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_UI_SIDEBAR))

typedef struct _glUISidebar        glUISidebar;
typedef struct _glUISidebarClass   glUISidebarClass;

typedef struct _glUISidebarPrivate glUISidebarPrivate;

struct _glUISidebar {
	GtkVBox              parent_widget;

	glUISidebarPrivate  *priv;
};

struct _glUISidebarClass {
	GtkVBoxClass         parent_class;
};

GType        gl_ui_sidebar_get_type          (void) G_GNUC_CONST;

GtkWidget   *gl_ui_sidebar_new               (void);

void         gl_ui_sidebar_set_label         (glUISidebar       *sidebar,
					      glLabel           *label);


G_END_DECLS

#endif /* __UI_SIDEBAR_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
