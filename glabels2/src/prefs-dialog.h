/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prefs-dialog.h:  Preferences dialog module header file
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

#ifndef __PREFS_DIALOG_H__
#define __PREFS_DIALOG_H__

#include <gtk/gtkwindow.h>
#include <gtk/gtkdialog.h>

G_BEGIN_DECLS

#define GL_TYPE_PREFS_DIALOG            (gl_prefs_dialog_get_type ())
#define GL_PREFS_DIALOG(obj)            (GTK_CHECK_CAST ((obj), GL_TYPE_PREFS_DIALOG, glPrefsDialog))
#define GL_PREFS_DIALOG_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_PREFS_DIALOG, glPrefsDialogClass))
#define GL_IS_PREFS_DIALOG(obj)         (GTK_CHECK_TYPE ((obj), GL_TYPE_PREFS_DIALOG))
#define GL_IS_PREFS_DIALOG_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_PREFS_DIALOG))
#define GL_PREFS_DIALOG_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), GL_TYPE_PREFS_DIALOG, glPrefsDialogClass))


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
