/*
 *  color-combo-button.h
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

#ifndef __GL_COLOR_COMBO_BUTTON_H__
#define __GL_COLOR_COMBO_BUTTON_H__


#include <gtk/gtkhbox.h>


G_BEGIN_DECLS

#define TYPE_GL_COLOR_COMBO_BUTTON              (gl_color_combo_button_get_type ())
#define GL_COLOR_COMBO_BUTTON(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_GL_COLOR_COMBO_BUTTON, glColorComboButton))
#define GL_COLOR_COMBO_BUTTON_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_GL_COLOR_COMBO_BUTTON, glColorComboButtonClass))
#define IS_GL_COLOR_COMBO_BUTTON(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_GL_COLOR_COMBO_BUTTON))
#define IS_GL_COLOR_COMBO_BUTTON_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_GL_COLOR_COMBO_BUTTON))
#define GL_COLOR_COMBO_BUTTON_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), TYPE_GL_COLOR_COMBO_BUTTON, glColorComboButtonClass))


typedef struct _glColorComboButton          glColorComboButton;
typedef struct _glColorComboButtonPrivate   glColorComboButtonPrivate;
typedef struct _glColorComboButtonClass     glColorComboButtonClass;


struct _glColorComboButton {
        GtkHBox                        parent;

        glColorComboButtonPrivate     *priv;
};

struct _glColorComboButtonClass {
        GtkHBoxClass                   parent_class;

        /*
         * Signals
         */
        void (*color_changed) (glColorComboButton *object,
                               guint               color,
                               gboolean            is_default,
                               gpointer            user_data);

};


GType            gl_color_combo_button_get_type       (void) G_GNUC_CONST;

GtkWidget       *gl_color_combo_button_new            (GdkPixbuf          *icon,
                                                       const gchar        *default_label,
                                                       guint               default_color,
                                                       guint               color);

void             gl_color_combo_button_set_color      (glColorComboButton *this,
                                                       guint               color);

void             gl_color_combo_button_set_to_default (glColorComboButton *this);

guint            gl_color_combo_button_get_color      (glColorComboButton *this,
                                                       gboolean           *is_default);

void             gl_color_combo_button_set_relief     (glColorComboButton *this,
                                                       GtkReliefStyle      relief);


G_END_DECLS

#endif /* __GL_COLOR_COMBO_BUTTON_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
