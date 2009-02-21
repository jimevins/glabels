/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_merge_menu.h:  Merge menu widget module header file
 *
 *  Copyright (C) 2008  Jim Evins <evins@snaught.com>.
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

#ifndef __WDGT_MERGE_MENU_H__
#define __WDGT_MERGE_MENU_H__

#include <gtk/gtkmenu.h>

G_BEGIN_DECLS

#define GL_TYPE_WDGT_MERGE_MENU (gl_wdgt_merge_menu_get_type ())
#define GL_WDGT_MERGE_MENU(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), GL_TYPE_WDGT_MERGE_MENU, glWdgtMergeMenu ))
#define GL_WDGT_MERGE_MENU_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_WDGT_MERGE_MENU, glWdgtMergeMenuClass))
#define GL_IS_WDGT_MERGE_MENU(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_WDGT_MERGE_MENU))
#define GL_IS_WDGT_MERGE_MENU_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_WDGT_MERGE_MENU))


typedef struct _glWdgtMergeMenu         glWdgtMergeMenu;
typedef struct _glWdgtMergeMenuClass    glWdgtMergeMenuClass;

typedef struct _glWdgtMergeMenuPrivate  glWdgtMergeMenuPrivate;


struct _glWdgtMergeMenu {
        GtkMenu                  parent_widget;

        glWdgtMergeMenuPrivate  *priv;
};


struct _glWdgtMergeMenuClass {
        GtkMenuClass             parent_class;

        void (*field_selected) (glWdgtMergeMenu *merge_menu,
                                gchar           *field,
                                gpointer         user_data);
};


GType      gl_wdgt_merge_menu_get_type      (void) G_GNUC_CONST;

GtkWidget *gl_wdgt_merge_menu_new           (void);

void       gl_wdgt_merge_menu_set_fields    (glWdgtMergeMenu *merge_menu,
                                             GList           *field_list);


G_END_DECLS

#endif
