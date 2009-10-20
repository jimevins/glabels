/*
 *  field-button-menu.h
 *  Copyright (C) 2008-2009  Jim Evins <evins@snaught.com>.
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

#ifndef __FIELD_BUTTON_MENU_H__
#define __FIELD_BUTTON_MENU_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GL_TYPE_FIELD_BUTTON_MENU (gl_field_button_menu_get_type ())
#define GL_FIELD_BUTTON_MENU(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), GL_TYPE_FIELD_BUTTON_MENU, glFieldButtonMenu ))
#define GL_FIELD_BUTTON_MENU_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_FIELD_BUTTON_MENU, glFieldButtonMenuClass))
#define GL_IS_FIELD_BUTTON_MENU(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_FIELD_BUTTON_MENU))
#define GL_IS_FIELD_BUTTON_MENU_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_FIELD_BUTTON_MENU))


typedef struct _glFieldButtonMenu         glFieldButtonMenu;
typedef struct _glFieldButtonMenuClass    glFieldButtonMenuClass;

typedef struct _glFieldButtonMenuPrivate  glFieldButtonMenuPrivate;


struct _glFieldButtonMenu {
        GtkMenu                    parent_widget;

        glFieldButtonMenuPrivate  *priv;
};


struct _glFieldButtonMenuClass {
        GtkMenuClass               parent_class;

        void (*key_selected) (glFieldButtonMenu *merge_menu,
                              gchar             *key,
                              gpointer           user_data);
};


GType      gl_field_button_menu_get_type    (void) G_GNUC_CONST;

GtkWidget *gl_field_button_menu_new         (void);

void       gl_field_button_menu_set_keys    (glFieldButtonMenu *merge_menu,
                                             GList             *key_list);


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
