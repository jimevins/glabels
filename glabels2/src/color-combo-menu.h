/*
 *  color-combo-menu.h
 *  Copyright (C) 2008  Jim Evins <evins@snaught.com>.
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

#ifndef __COLOR_COMBO_MENU_H__
#define __COLOR_COMBO_MENU_H__

#include <gtk/gtkmenu.h>


G_BEGIN_DECLS

#define GL_TYPE_COLOR_COMBO_MENU (gl_color_combo_menu_get_type ())
#define GL_COLOR_COMBO_MENU(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_COLOR_COMBO_MENU, glColorComboMenu ))
#define GL_COLOR_COMBO_MENU_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_COLOR_COMBO_MENU, glColorComboMenuClass))
#define GL_IS_COLOR_COMBO_MENU(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_COLOR_COMBO_MENU))
#define GL_IS_COLOR_COMBO_MENU_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_COLOR_COMBO_MENU))

typedef struct _glColorComboMenu        glColorComboMenu;
typedef struct _glColorComboMenuPrivate glColorComboMenuPrivate;
typedef struct _glColorComboMenuClass   glColorComboMenuClass;

struct _glColorComboMenu {
	GtkMenu                  parent_widget;

	glColorComboMenuPrivate *priv;
};

struct _glColorComboMenuClass {
	GtkMenuClass             parent_class;

        /*
         * Signals
         */
        void (*color_changed) (glColorComboMenu *object,
                               guint             color,
                               gboolean          is_default,
                               gpointer          user_data);

};


GType      gl_color_combo_menu_get_type          (void) G_GNUC_CONST;

GtkWidget *gl_color_combo_menu_new               (const gchar *default_label,
                                                  guint        color);


G_END_DECLS


#endif /* __COLOR_COMBO_MENU_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */

