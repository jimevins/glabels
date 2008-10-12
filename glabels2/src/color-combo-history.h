/*
 *  color-combo-history.h
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

#ifndef __GL_COLOR_COMBO_HISTORY_H__
#define __GL_COLOR_COMBO_HISTORY_H__


#include <glib-object.h>


G_BEGIN_DECLS

/* Utility Macros */
#define TYPE_GL_COLOR_COMBO_HISTORY              (gl_color_combo_history_get_type ())
#define GL_COLOR_COMBO_HISTORY(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_GL_COLOR_COMBO_HISTORY, glColorComboHistory))
#define GL_COLOR_COMBO_HISTORY_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_GL_COLOR_COMBO_HISTORY, glColorComboHistoryClass))
#define IS_GL_COLOR_COMBO_HISTORY(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_GL_COLOR_COMBO_HISTORY))
#define IS_GL_COLOR_COMBO_HISTORY_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_GL_COLOR_COMBO_HISTORY))
#define GL_COLOR_COMBO_HISTORY_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), TYPE_GL_COLOR_COMBO_HISTORY, glColorComboHistoryClass))


/* Type definitions */
typedef struct _glColorComboHistory          glColorComboHistory;
typedef struct _glColorComboHistoryPrivate   glColorComboHistoryPrivate;
typedef struct _glColorComboHistoryClass     glColorComboHistoryClass;


/** GL_COLOR_COMBO_HISTORY Object */
struct _glColorComboHistory {
        GObject                        parent;

        glColorComboHistoryPrivate    *priv;
};

/** GL_COLOR_COMBO_HISTORY Class */
struct _glColorComboHistoryClass {
        GObjectClass                   parent_class;
};


GType                gl_color_combo_history_get_type  (void) G_GNUC_CONST;

glColorComboHistory *gl_color_combo_history_new       (guint                n);

void                 gl_color_combo_history_add_color (glColorComboHistory *this,
                                                       guint                color);

guint                gl_color_combo_history_get_color (glColorComboHistory *this,
                                                       guint                i);


G_END_DECLS

#endif /* __GL_COLOR_COMBO_HISTORY_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
