/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  hig.h:  HIG inspired dialog and layout tools
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

#ifndef __HIG_H__
#define __HIG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GL_HIG_SPACING 6

/*===========================================================================*/
/* HIG Dialog.                                                               */
/*===========================================================================*/

#define GL_TYPE_HIG_DIALOG (gl_hig_dialog_get_type ())
#define GL_HIG_DIALOG(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_HIG_DIALOG, glHigDialog ))
#define GL_HIG_DIALOG_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_HIG_DIALOG, glHigDialogClass))
#define GL_IS_HIG_DIALOG(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_HIG_DIALOG))
#define GL_IS_HIG_DIALOG_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_HIG_DIALOG))

typedef struct _glHigDialog      glHigDialog;
typedef struct _glHigDialogClass glHigDialogClass;

struct _glHigDialog {
	GtkDialog         parent_widget;

	GtkWidget        *vbox;
};

struct _glHigDialogClass {
	GtkDialogClass    parent_class;
};

guint      gl_hig_dialog_get_type         (void);

GtkWidget *gl_hig_dialog_new              (void);

GtkWidget *gl_hig_dialog_new_with_buttons (const gchar    *title,
					   GtkWindow      *parent,
					   GtkDialogFlags  flags,
					   const gchar    *first_button_text,
					   ...);


/*===========================================================================*/
/* HIG Category.                                                             */
/*===========================================================================*/

#define GL_TYPE_HIG_CATEGORY (gl_hig_category_get_type ())
#define GL_HIG_CATEGORY(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_HIG_CATEGORY, glHigCategory ))
#define GL_HIG_CATEGORY_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_HIG_CATEGORY, glHigCategoryClass))
#define GL_IS_HIG_CATEGORY(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_HIG_CATEGORY))
#define GL_IS_HIG_CATEGORY_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_HIG_CATEGORY))

typedef struct _glHigCategory      glHigCategory;
typedef struct _glHigCategoryClass glHigCategoryClass;

struct _glHigCategory {
	GtkVBox           parent_widget;

	GtkWidget        *label;
	GtkWidget        *vbox;
};

struct _glHigCategoryClass {
	GtkVBoxClass      parent_class;
};

guint      gl_hig_category_get_type         (void);

GtkWidget *gl_hig_category_new              (const gchar *header);

G_END_DECLS

#endif /* __HIG_H__ */
