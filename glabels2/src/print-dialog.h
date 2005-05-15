/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  print-dialog.h:  Print dialog module header file
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
#ifndef __PRINT_DIALOG_H__
#define __PRINT_DIALOG_H__

#include <gtk/gtkwindow.h>
#include "hig.h"
#include "view.h"

G_BEGIN_DECLS

#define GL_TYPE_PRINT_DIALOG            (gl_print_dialog_get_type ())
#define GL_PRINT_DIALOG(obj)            (GTK_CHECK_CAST ((obj), GL_TYPE_PRINT_DIALOG, glPrintDialog))
#define GL_PRINT_DIALOG_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_PRINT_DIALOG, glPrintDialogClass))
#define GL_IS_PRINT_DIALOG(obj)         (GTK_CHECK_TYPE ((obj), GL_TYPE_PRINT_DIALOG))
#define GL_IS_PRINT_DIALOG_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_PRINT_DIALOG))
#define GL_PRINT_DIALOG_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), GL_TYPE_PRINT_DIALOG, glPrintDialogClass))


typedef struct _glPrintDialog         glPrintDialog;
typedef struct _glPrintDialogClass    glPrintDialogClass;

typedef struct _glPrintDialogPrivate  glPrintDialogPrivate;

struct _glPrintDialog
{
	glHigDialog           parent_instance;

	glPrintDialogPrivate *priv;

};

struct  _glPrintDialogClass
{
	glHigDialogClass      parent_class;
};


GType      gl_print_dialog_get_type            (void) G_GNUC_CONST;

GtkWidget *gl_print_dialog_new                 (glLabel       *label,
						GtkWindow     *win);

void       gl_print_dialog_force_outline_flag  (glPrintDialog *dialog);

G_END_DECLS

#endif
