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

extern BonoboUIVerb gl_ui_verbs [];

extern gchar *gl_ui_no_docs_sensible_verbs []; 
extern gchar *gl_ui_not_modified_doc_sensible_verbs []; 
extern gchar *gl_ui_selection_sensible_verbs [];
extern gchar *gl_ui_atomic_selection_sensible_verbs [];

#define gl_ui_all_sensible_verbs gl_ui_no_docs_sensible_verbs

void gl_ui_set_verb_sensitive 		(BonoboUIComponent *ui_component,
					 gchar             *cname,
					 gboolean           sensitive);
void gl_ui_set_verb_list_sensitive 	(BonoboUIComponent *ui_component, 
					 gchar            **vlist,
					 gboolean           sensitive);
void gl_ui_set_verb_state 		(BonoboUIComponent *ui_component, 
					 gchar* cname, 
					 gboolean state);

/* convenience functions for plugins */

void gl_ui_add_menu_item        (BonoboWindow   *window,
				 const gchar    *path,
				 const gchar    *name,
				 const gchar    *label,
				 const gchar    *tooltip,
				 const gchar    *stock_pixmap,
				 BonoboUIVerbFn  cb);

void gl_ui_remove_menu_item     (BonoboWindow   *window,
				 const gchar    *path,
				 const gchar    *name);


void gl_ui_add_menu_item_all    (const gchar    *path,
				 const gchar    *name,
				 const gchar    *label,
				 const gchar    *tooltip,
				 const gchar    *stock_pixmap,
				 BonoboUIVerbFn  cb);

void gl_ui_remove_menu_item_all (const gchar    *path,
				 const gchar    *name);


#endif /* __GL_UI_H__ */
