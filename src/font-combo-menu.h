/*
 *  font-combo-menu.h
 *  Copyright (C) 2009  Jim Evins <evins@snaught.com>.
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

#ifndef __FONT_COMBO_MENU_H__
#define __FONT_COMBO_MENU_H__

#include <gtk/gtk.h>


G_BEGIN_DECLS

#define GL_TYPE_FONT_COMBO_MENU (gl_font_combo_menu_get_type ())
#define GL_FONT_COMBO_MENU(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), GL_TYPE_FONT_COMBO_MENU, glFontComboMenu ))
#define GL_FONT_COMBO_MENU_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_FONT_COMBO_MENU, glFontComboMenuClass))
#define GL_IS_FONT_COMBO_MENU(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_FONT_COMBO_MENU))
#define GL_IS_FONT_COMBO_MENU_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_FONT_COMBO_MENU))

typedef struct _glFontComboMenu        glFontComboMenu;
typedef struct _glFontComboMenuPrivate glFontComboMenuPrivate;
typedef struct _glFontComboMenuClass   glFontComboMenuClass;

struct _glFontComboMenu {
	GtkMenu                 parent_widget;

	glFontComboMenuPrivate *priv;
};

struct _glFontComboMenuClass {
	GtkMenuClass            parent_class;

        /*
         * Signals
         */
        void (*font_changed) (glFontComboMenu *object,
                              gpointer         user_data);

};


GType      gl_font_combo_menu_get_type     (void) G_GNUC_CONST;

GtkWidget *gl_font_combo_menu_new          (void);

gchar     *gl_font_combo_menu_get_family   (glFontComboMenu *this);


G_END_DECLS


#endif /* __FONT_COMBO_MENU_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */

