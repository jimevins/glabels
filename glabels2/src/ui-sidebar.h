/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/**
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  ui-sidebar.h:  Object property sidebar header file
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

#ifndef __UI_SIDEBAR_H__
#define __UI_SIDEBAR_H__

#include <gtk/gtkvbox.h>

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

void         gl_ui_sidebar_set_view          (glUISidebar       *sidebar,
					      glView            *view);


G_END_DECLS

#endif /* __UI_SIDEBAR_H__ */
