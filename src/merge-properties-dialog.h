/*
 *  merge-properties-dialog.h
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

#ifndef __MERGE_PROPERTIES_DIALOG_H__
#define __MERGE_PROPERTIES_DIALOG_H__

#include <gtk/gtk.h>
#include "label.h"

G_BEGIN_DECLS

#define GL_TYPE_MERGE_PROPERTIES_DIALOG            (gl_merge_properties_dialog_get_type ())
#define GL_MERGE_PROPERTIES_DIALOG(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_MERGE_PROPERTIES_DIALOG, glMergePropertiesDialog))
#define GL_MERGE_PROPERTIES_DIALOG_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_MERGE_PROPERTIES_DIALOG, glMergePropertiesDialogClass))
#define GL_IS_MERGE_PROPERTIES_DIALOG(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_MERGE_PROPERTIES_DIALOG))
#define GL_IS_MERGE_PROPERTIES_DIALOG_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_MERGE_PROPERTIES_DIALOG))
#define GL_MERGE_PROPERTIES_DIALOG_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_MERGE_PROPERTIES_DIALOG, glMergePropertiesDialogClass))


typedef struct _glMergePropertiesDialog         glMergePropertiesDialog;
typedef struct _glMergePropertiesDialogClass    glMergePropertiesDialogClass;

typedef struct _glMergePropertiesDialogPrivate  glMergePropertiesDialogPrivate;

struct _glMergePropertiesDialog
{
	GtkDialog                       parent_instance;

	glMergePropertiesDialogPrivate *priv;

};

struct  _glMergePropertiesDialogClass
{
	GtkDialogClass                  parent_class;
};

GType      gl_merge_properties_dialog_get_type    (void) G_GNUC_CONST;

GtkWidget *gl_merge_properties_dialog_new         (glLabel   *label,
	                                           GtkWindow *window);

G_END_DECLS

#endif



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
