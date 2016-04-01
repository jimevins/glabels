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
#define GL_LABEL_PROPERTIES_DIALOG(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_LABEL_PROPERTIES_DIALOG, glLabelPropertiesDialog))
#define GL_LABEL_PROPERTIES_DIALOG_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_LABEL_PROPERTIES_DIALOG, glLabelPropertiesDialogClass))
#define GL_IS_LABEL_PROPERTIES_DIALOG(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_LABEL_PROPERTIES_DIALOG))
#define GL_IS_LABEL_PROPERTIES_DIALOG_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_LABEL_PROPERTIES_DIALOG))
#define GL_LABEL_PROPERTIES_DIALOG_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_LABEL_PROPERTIES_DIALOG, glLabelPropertiesDialogClass))

typedef struct _glLabelPropertiesDialog         glLabelPropertiesDialog;
typedef struct _glLabelPropertiesDialogClass    glLabelPropertiesDialogClass;

typedef struct _glLabelPropertiesDialogPrivate  glLabelPropertiesDialogPrivate;

struct _glLabelPropertiesDialog
{
	GtkDialog                       parent_instance;

	glLabelPropertiesDialogPrivate *priv;
};

struct  _glLabelPropertiesDialogClass
{
	GtkDialogClass                  parent_class;
};


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
