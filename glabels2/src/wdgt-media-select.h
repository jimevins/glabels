/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_media_select.h:  media selection widget module header file
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

#ifndef __WDGT_MEDIA_SELECT_H__
#define __WDGT_MEDIA_SELECT_H__

#include <gnome.h>
#include "hig.h"

G_BEGIN_DECLS

#define GL_TYPE_WDGT_MEDIA_SELECT (gl_wdgt_media_select_get_type ())
#define GL_WDGT_MEDIA_SELECT(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_WDGT_MEDIA_SELECT, glWdgtMediaSelect ))
#define GL_WDGT_MEDIA_SELECT_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_WDGT_MEDIA_SELECT, glWdgtMediaSelectClass))
#define GL_IS_WDGT_MEDIA_SELECT(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_WDGT_MEDIA_SELECT))
#define GL_IS_WDGT_MEDIA_SELECT_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_WDGT_MEDIA_SELECT))

typedef struct _glWdgtMediaSelect      glWdgtMediaSelect;
typedef struct _glWdgtMediaSelectClass glWdgtMediaSelectClass;

struct _glWdgtMediaSelect {
	glHigVBox  parent_widget;

	GtkWidget *page_size_entry;
	GtkWidget *template_combo;
	GtkWidget *template_entry;

	GtkWidget *mini_preview;

	GtkWidget *desc_label;
	GtkWidget *sheet_size_label;
	GtkWidget *number_label;
	GtkWidget *label_size_label;
};

struct _glWdgtMediaSelectClass {
	glHigVBoxClass parent_class;

	void (*changed) (glWdgtMediaSelect * media_select, gpointer user_data);
};

GType      gl_wdgt_media_select_get_type      (void) G_GNUC_CONST;

GtkWidget *gl_wdgt_media_select_new           (void);

gchar     *gl_wdgt_media_select_get_name      (glWdgtMediaSelect *media_select);

void       gl_wdgt_media_select_set_name      (glWdgtMediaSelect *media_select,
					       gchar             *name);

gchar     *gl_wdgt_media_select_get_page_size (glWdgtMediaSelect *media_select);

void       gl_wdgt_media_select_set_page_size (glWdgtMediaSelect *media_select,
					       gchar             *page_size_id);

G_END_DECLS

#endif
