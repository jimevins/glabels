/*
 *  font-combo-menu-item.h
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

#ifndef __FONT_COMBO_MENU_ITEM_H__
#define __FONT_COMBO_MENU_ITEM_H__


#include <gtk/gtk.h>


G_BEGIN_DECLS

#define GL_TYPE_FONT_COMBO_MENU_ITEM (gl_font_combo_menu_item_get_type ())
#define GL_FONT_COMBO_MENU_ITEM(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), GL_TYPE_FONT_COMBO_MENU_ITEM, glFontComboMenuItem ))
#define GL_FONT_COMBO_MENU_ITEM_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_FONT_COMBO_MENU_ITEM, glFontComboMenuItemClass))
#define GL_IS_FONT_COMBO_MENU_ITEM(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_FONT_COMBO_MENU_ITEM))
#define GL_IS_FONT_COMBO_MENU_ITEM_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_FONT_COMBO_MENU_ITEM))

typedef struct _glFontComboMenuItem        glFontComboMenuItem;
typedef struct _glFontComboMenuItemPrivate glFontComboMenuItemPrivate;
typedef struct _glFontComboMenuItemClass   glFontComboMenuItemClass;

struct _glFontComboMenuItem {
	GtkMenuItem            parent_widget;

	glFontComboMenuItemPrivate *priv;
};

struct _glFontComboMenuItemClass {
	GtkMenuItemClass       parent_class;
};


GType      gl_font_combo_menu_item_get_type   (void) G_GNUC_CONST;

GtkWidget *gl_font_combo_menu_item_new        (gchar                *font_family);

gchar     *gl_font_combo_menu_item_get_family (glFontComboMenuItem  *this);


G_END_DECLS

#endif /* __FONT_COMBO_MENU_ITEM_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
