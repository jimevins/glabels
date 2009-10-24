/*
 *  color-history-model.h
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

#ifndef __GL_COLOR_HISTORY_MODEL_H__
#define __GL_COLOR_HISTORY_MODEL_H__


#include <glib-object.h>


G_BEGIN_DECLS

/* Utility Macros */
#define TYPE_GL_COLOR_HISTORY_MODEL              (gl_color_history_model_get_type ())
#define GL_COLOR_HISTORY_MODEL(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_GL_COLOR_HISTORY_MODEL, glColorHistoryModel))
#define GL_COLOR_HISTORY_MODEL_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_GL_COLOR_HISTORY_MODEL, glColorHistoryModelClass))
#define IS_GL_COLOR_HISTORY_MODEL(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_GL_COLOR_HISTORY_MODEL))
#define IS_GL_COLOR_HISTORY_MODEL_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_GL_COLOR_HISTORY_MODEL))
#define GL_COLOR_HISTORY_MODEL_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), TYPE_GL_COLOR_HISTORY_MODEL, glColorHistoryModelClass))


/* Type definitions */
typedef struct _glColorHistoryModel          glColorHistoryModel;
typedef struct _glColorHistoryModelPrivate   glColorHistoryModelPrivate;
typedef struct _glColorHistoryModelClass     glColorHistoryModelClass;


/** GL_COLOR_HISTORY_MODEL Object */
struct _glColorHistoryModel {
        GObject                        parent;

        glColorHistoryModelPrivate    *priv;
};

/** GL_COLOR_HISTORY_MODEL Class */
struct _glColorHistoryModelClass {
        GObjectClass                   parent_class;

        /*
         * Signals
         */
        void (*changed)     (glColorHistoryModel *this,
                             gpointer             user_data);

};


GType                gl_color_history_model_get_type  (void) G_GNUC_CONST;

glColorHistoryModel *gl_color_history_model_new       (guint                n);

void                 gl_color_history_model_add_color (glColorHistoryModel *this,
                                                       guint                color);

guint                gl_color_history_model_get_color (glColorHistoryModel *this,
                                                       guint                i);


G_END_DECLS

#endif /* __GL_COLOR_HISTORY_MODEL_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
