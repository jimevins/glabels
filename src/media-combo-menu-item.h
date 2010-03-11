/*
 *  media-combo-menu-item.h
 *  Copyright (C) 2010  Jim Evins <evins@snaught.com>.
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

#ifndef __MEDIA_COMBO_MENU_ITEM_H__
#define __MEDIA_COMBO_MENU_ITEM_H__


#include <gtk/gtk.h>


G_BEGIN_DECLS

#define GL_TYPE_MEDIA_COMBO_MENU_ITEM (gl_media_combo_menu_item_get_type ())
#define GL_MEDIA_COMBO_MENU_ITEM(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), GL_TYPE_MEDIA_COMBO_MENU_ITEM, glMediaComboMenuItem ))
#define GL_MEDIA_COMBO_MENU_ITEM_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_MEDIA_COMBO_MENU_ITEM, glMediaComboMenuItemClass))
#define GL_IS_MEDIA_COMBO_MENU_ITEM(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_MEDIA_COMBO_MENU_ITEM))
#define GL_IS_MEDIA_COMBO_MENU_ITEM_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_MEDIA_COMBO_MENU_ITEM))

typedef struct _glMediaComboMenuItem        glMediaComboMenuItem;
typedef struct _glMediaComboMenuItemPrivate glMediaComboMenuItemPrivate;
typedef struct _glMediaComboMenuItemClass   glMediaComboMenuItemClass;

struct _glMediaComboMenuItem {
	GtkMenuItem            parent_widget;

	glMediaComboMenuItemPrivate *priv;
};

struct _glMediaComboMenuItemClass {
	GtkMenuItemClass       parent_class;
};


GType      gl_media_combo_menu_item_get_type   (void) G_GNUC_CONST;

GtkWidget *gl_media_combo_menu_item_new        (gchar                *name);

gchar     *gl_media_combo_menu_item_get_name   (glMediaComboMenuItem *this);


G_END_DECLS

#endif /* __MEDIA_COMBO_MENU_ITEM_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
