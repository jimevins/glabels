/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  ui.h:  GLabels UI module header file
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
#ifndef __GL_UI_H__
#define __GL_UI_H__

#include <bonobo/bonobo-ui-component.h>
#include <bonobo/bonobo-ui-engine.h>
#include <bonobo/bonobo-window.h>

#include "view.h"

G_BEGIN_DECLS

void gl_ui_init                   (BonoboUIComponent *ui_component,
				   BonoboWindow      *win,
				   GtkWidget         *cursor_info_frame,
				   GtkWidget         *zoom_info_frame);

void gl_ui_unref                  (BonoboUIComponent *ui_component);

void gl_ui_update_all             (BonoboUIComponent *ui_component,
				   glView            *view);

void gl_ui_update_nodoc           (BonoboUIComponent *ui_component);

void gl_ui_update_modified_verbs  (BonoboUIComponent *ui_component,
				   glLabel           *label);

void gl_ui_update_selection_verbs (BonoboUIComponent *ui_component,
				   glView            *view);

void gl_ui_update_zoom_verbs      (BonoboUIComponent *ui_component,
				   glView            *view);

void gl_ui_update_undo_redo_verbs (BonoboUIComponent *ui_component,
				   glLabel           *label);

G_END_DECLS

#endif /* __GL_UI_H__ */
