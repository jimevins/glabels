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

/*
 * This file is based on gedit-menus.h from gedit2:
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi 
 *
 */
#ifndef __GL_UI_H__
#define __GL_UI_H__

#include <bonobo/bonobo-ui-component.h>
#include <bonobo/bonobo-ui-engine.h>
#include <bonobo/bonobo-window.h>

#include "view.h"

extern BonoboUIVerb gl_ui_verbs [];


extern void gl_ui_init                   (BonoboUIComponent *ui_component,
					  BonoboWindow      *win);
extern void gl_ui_update_all             (BonoboUIComponent *ui_component,
					  glView            *view);
extern void gl_ui_update_nodoc           (BonoboUIComponent *ui_component);
extern void gl_ui_update_undo_redo_verbs (BonoboUIComponent *ui_component,
					  glLabel           *label);
extern void gl_ui_update_selection_verbs (BonoboUIComponent *ui_component,
					  glView            *view);

#endif /* __GL_UI_H__ */
