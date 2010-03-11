/*
 *  media-combo.h
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

#ifndef __GL_MEDIA_COMBO_H__
#define __GL_MEDIA_COMBO_H__


#include <gtk/gtk.h>


G_BEGIN_DECLS

#define TYPE_GL_MEDIA_COMBO              (gl_media_combo_get_type ())
#define GL_MEDIA_COMBO(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_GL_MEDIA_COMBO, glMediaCombo))
#define GL_MEDIA_COMBO_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_GL_MEDIA_COMBO, glMediaComboClass))
#define IS_GL_MEDIA_COMBO(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_GL_MEDIA_COMBO))
#define IS_GL_MEDIA_COMBO_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_GL_MEDIA_COMBO))
#define GL_MEDIA_COMBO_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), TYPE_GL_MEDIA_COMBO, glMediaComboClass))


typedef struct _glMediaCombo          glMediaCombo;
typedef struct _glMediaComboPrivate   glMediaComboPrivate;
typedef struct _glMediaComboClass     glMediaComboClass;


struct _glMediaCombo {
        GtkToggleButton               parent;

        glMediaComboPrivate          *priv;
};

struct _glMediaComboClass {
        GtkToggleButtonClass          parent_class;

        /*
         * Signals
         */
        void (*changed) (glMediaCombo *object,
                         gpointer     user_data);

};


GType            gl_media_combo_get_type          (void) G_GNUC_CONST;

GtkWidget       *gl_media_combo_new               (void);

void             gl_media_combo_set_name          (glMediaCombo   *this,
                                                   const gchar    *name);

gchar           *gl_media_combo_get_name          (glMediaCombo   *this);


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
