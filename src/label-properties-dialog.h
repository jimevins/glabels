/*
 *  label-properties-dialog.h
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

#ifndef __LABEL_PROPERTIES_DIALOG_H__
#define __LABEL_PROPERTIES_DIALOG_H__


#include <gtk/gtk.h>

#include "label.h"


G_BEGIN_DECLS

#define GL_RESPONSE_SELECT_OTHER 1

#define GL_TYPE_LABEL_PROPERTIES_DIALOG (gl_label_properties_dialog_get_type ())

G_DECLARE_FINAL_TYPE (glLabelPropertiesDialog, gl_label_properties_dialog, GL, LABEL_PROPERTIES_DIALOG, GtkDialog)

GtkWidget *gl_label_properties_dialog_new (glLabel *label, GtkWindow *parent);

G_END_DECLS


#endif /* __LABEL_PROPERTIES_DIALOG_H__ */




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
