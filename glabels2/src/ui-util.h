/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  ui-util.h:  GLabels UI utilities module header file
 *
 *  Copyright (C) 2001-2003  Jim Evins <evins@snaught.com>.
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
#ifndef __GL_UI_UTIL_H__
#define __GL_UI_UTIL_H__

#include <bonobo/bonobo-ui-component.h>

G_BEGIN_DECLS

void       gl_ui_util_set_verb_sensitive      (BonoboUIComponent           *ui_component,
					       gchar                       *cname,
					       gboolean                     sensitive);

void       gl_ui_util_set_verb_list_sensitive (BonoboUIComponent           *ui_component,
					       gchar                      **vlist,
					       gboolean                     sensitive);

void       gl_ui_util_set_verb_state          (BonoboUIComponent           *ui_component,
					       gchar                       *cname,
					       gboolean                     state);

void       gl_ui_util_insert_widget           (BonoboUIComponent           *ui_component,
					       GtkWidget                   *widget,
					       const char                  *path);


G_END_DECLS

#endif /* __GL_UI_UTIL_H__ */
