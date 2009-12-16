/*
 *  field-button.h
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

#ifndef __GL_FIELD_BUTTON_H__
#define __GL_FIELD_BUTTON_H__


#include <gtk/gtk.h>


G_BEGIN_DECLS

#define TYPE_GL_FIELD_BUTTON              (gl_field_button_get_type ())
#define GL_FIELD_BUTTON(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_GL_FIELD_BUTTON, glFieldButton))
#define GL_FIELD_BUTTON_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_GL_FIELD_BUTTON, glFieldButtonClass))
#define IS_GL_FIELD_BUTTON(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_GL_FIELD_BUTTON))
#define IS_GL_FIELD_BUTTON_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_GL_FIELD_BUTTON))
#define GL_FIELD_BUTTON_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), TYPE_GL_FIELD_BUTTON, glFieldButtonClass))


typedef struct _glFieldButton          glFieldButton;
typedef struct _glFieldButtonPrivate   glFieldButtonPrivate;
typedef struct _glFieldButtonClass     glFieldButtonClass;


struct _glFieldButton {
        GtkToggleButton               parent;

        glFieldButtonPrivate         *priv;
};

struct _glFieldButtonClass {
        GtkToggleButtonClass          parent_class;

        /*
         * Signals
         */
        void (*key_selected) (glFieldButton *object,
                              gchar         *key,
                              gpointer       user_data);

        void (*changed)      (glFieldButton *object,
                              gpointer       user_data);

};


GType            gl_field_button_get_type          (void) G_GNUC_CONST;

GtkWidget       *gl_field_button_new               (const gchar     *name);

void             gl_field_button_set_keys          (glFieldButton   *this,
                                                    GList           *key_list);

void             gl_field_button_set_key           (glFieldButton   *this,
                                                    const gchar     *key);

gchar           *gl_field_button_get_key           (glFieldButton   *this);


G_END_DECLS

#endif /* __GL_COLOR_COMBO_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
