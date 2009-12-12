/*
 *  ui.h
 *  Copyright (C) 2001-2009  Jim Evins <evins@snaught.com>.
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

#ifndef __GL_UI_H__
#define __GL_UI_H__

#include <gtk/gtk.h>

#include "window.h"
#include "view.h"

G_BEGIN_DECLS


GtkUIManager *gl_ui_new                    (glWindow          *window);

void          gl_ui_unref                  (GtkUIManager      *ui);

void          gl_ui_update_all             (GtkUIManager      *ui,
					    glView            *view);

void          gl_ui_update_modified_verbs  (GtkUIManager      *ui,
					    glLabel           *label);

void          gl_ui_update_selection_verbs (GtkUIManager      *ui,
					    glView            *view,
                                            gboolean           has_focus);

void          gl_ui_update_zoom_verbs      (GtkUIManager      *ui,
					    glView            *view);

void          gl_ui_update_paste_verbs     (GtkUIManager      *ui,
                                            gboolean           can_paste);

void          gl_ui_update_undo_redo_verbs (GtkUIManager      *ui,
					    glLabel           *label);

G_END_DECLS

#endif /* __GL_UI_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
