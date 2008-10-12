/*
 *  color-combo-color-menu-item.h
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

#ifndef __COLOR_COMBO_COLOR_MENU_ITEM_H__
#define __COLOR_COMBO_COLOR_MENU_ITEM_H__


#include <gtk/gtkmenuitem.h>


G_BEGIN_DECLS

#define GL_TYPE_COLOR_COMBO_COLOR_MENU_ITEM (gl_color_combo_color_menu_item_get_type ())
#define GL_COLOR_COMBO_COLOR_MENU_ITEM(obj) \
        (GTK_CHECK_CAST((obj), GL_TYPE_COLOR_COMBO_COLOR_MENU_ITEM, glColorComboColorMenuItem ))
#define GL_COLOR_COMBO_COLOR_MENU_ITEM_CLASS(klass) \
        (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_COLOR_COMBO_COLOR_MENU_ITEM, glColorComboColorMenuItemClass))
#define GL_IS_COLOR_COMBO_COLOR_MENU_ITEM(obj) \
        (GTK_CHECK_TYPE ((obj), GL_TYPE_COLOR_COMBO_COLOR_MENU_ITEM))
#define GL_IS_COLOR_COMBO_COLOR_MENU_ITEM_CLASS(klass) \
        (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_COLOR_COMBO_COLOR_MENU_ITEM))

typedef struct _glColorComboColorMenuItem        glColorComboColorMenuItem;
typedef struct _glColorComboColorMenuItemPrivate glColorComboColorMenuItemPrivate;
typedef struct _glColorComboColorMenuItemClass   glColorComboColorMenuItemClass;

struct _glColorComboColorMenuItem {
	GtkMenuItem                       parent_widget;

	glColorComboColorMenuItemPrivate *priv;
};

struct _glColorComboColorMenuItemClass {
	GtkMenuItemClass                  parent_class;
};


GType      gl_color_combo_color_menu_item_get_type (void) G_GNUC_CONST;

GtkWidget *gl_color_combo_color_menu_item_new      (gint                       id,
                                                    guint                      color,
                                                    const gchar               *tip);

gint       gl_color_combo_color_menu_item_get_id   (glColorComboColorMenuItem *this);

void       gl_color_combo_color_menu_item_set_color(glColorComboColorMenuItem *this,
                                                    gint                       id,
                                                    guint                      color,
                                                    const gchar               *tip);


G_END_DECLS

#endif /* __COLOR_COMBO_COLOR_MENU_ITEM_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
