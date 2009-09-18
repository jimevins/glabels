/*
 *  prefs-dialog.h
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

#ifndef __PREFS_DIALOG_H__
#define __PREFS_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GL_TYPE_PREFS_DIALOG            (gl_prefs_dialog_get_type ())
#define GL_PREFS_DIALOG(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_PREFS_DIALOG, glPrefsDialog))
#define GL_PREFS_DIALOG_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_PREFS_DIALOG, glPrefsDialogClass))
#define GL_IS_PREFS_DIALOG(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_PREFS_DIALOG))
#define GL_IS_PREFS_DIALOG_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_PREFS_DIALOG))
#define GL_PREFS_DIALOG_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_PREFS_DIALOG, glPrefsDialogClass))


typedef struct _glPrefsDialog 		glPrefsDialog;
typedef struct _glPrefsDialogClass	glPrefsDialogClass;

typedef struct _glPrefsDialogPrivate	glPrefsDialogPrivate;

struct _glPrefsDialog
{
	GtkDialog             parent_instance;

	glPrefsDialogPrivate *priv;

};

struct  _glPrefsDialogClass
{
	GtkDialogClass        parent_class;
};

GType    	gl_prefs_dialog_get_type 	(void) G_GNUC_CONST;

GtkWidget      *gl_prefs_dialog_new		(GtkWindow *parent);

G_END_DECLS

#endif /* __PREFS_DIALOG_H__ */




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
