/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  alert.h:  a HIG inspired alert dialog
 *
 *  Copyright (C) 2002  Jim Evins <evins@snaught.com>.
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

#ifndef __ALERT_H__
#define __ALERT_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GL_TYPE_ALERT_DIALOG (gl_alert_dialog_get_type ())
#define GL_ALERT_DIALOG(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_ALERT_DIALOG, glAlertDialog ))
#define GL_ALERT_DIALOG_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_ALERT_DIALOG, glAlertDialogClass))
#define GL_IS_ALERT_DIALOG(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_ALERT_DIALOG))
#define GL_IS_ALERT_DIALOG_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_ALERT_DIALOG))

typedef struct _glAlertDialog      glAlertDialog;
typedef struct _glAlertDialogClass glAlertDialogClass;

struct _glAlertDialog {
	GtkDialog         parent_widget;
};

struct _glAlertDialogClass {
	GtkDialogClass    parent_class;
};

guint      gl_alert_dialog_get_type    (void);

GtkWidget *gl_alert_dialog_new         (GtkWindow      *parent,
					GtkDialogFlags  flags,
					GtkMessageType  type,
					GtkButtonsType  buttons,
					const gchar    *primary_text,
					const gchar    *secondary_text);

G_END_DECLS

#endif /* __ALERT_H__ */
