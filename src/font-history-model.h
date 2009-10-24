/*
 *  font-history-model.h
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

#ifndef __GL_FONT_HISTORY_MODEL_H__
#define __GL_FONT_HISTORY_MODEL_H__


#include <glib-object.h>


G_BEGIN_DECLS

/* Utility Macros */
#define TYPE_GL_FONT_HISTORY_MODEL              (gl_font_history_model_get_type ())
#define GL_FONT_HISTORY_MODEL(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_GL_FONT_HISTORY_MODEL, glFontHistoryModel))
#define GL_FONT_HISTORY_MODEL_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_GL_FONT_HISTORY_MODEL, glFontHistoryModelClass))
#define IS_GL_FONT_HISTORY_MODEL(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_GL_FONT_HISTORY_MODEL))
#define IS_GL_FONT_HISTORY_MODEL_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_GL_FONT_HISTORY_MODEL))
#define GL_FONT_HISTORY_MODEL_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), TYPE_GL_FONT_HISTORY_MODEL, glFontHistoryModelClass))


/* Type definitions */
typedef struct _glFontHistoryModel          glFontHistoryModel;
typedef struct _glFontHistoryModelPrivate   glFontHistoryModelPrivate;
typedef struct _glFontHistoryModelClass     glFontHistoryModelClass;


/** GL_FONT_HISTORY_MODEL Object */
struct _glFontHistoryModel {
        GObject                       parent;

        glFontHistoryModelPrivate    *priv;
};

/** GL_FONT_HISTORY_MODEL Class */
struct _glFontHistoryModelClass {
        GObjectClass                  parent_class;

        /*
         * Signals
         */
        void (*changed)     (glFontHistoryModel     *this,
                             gpointer           user_data);


};


GType                gl_font_history_model_get_type         (void) G_GNUC_CONST;

glFontHistoryModel  *gl_font_history_model_new              (guint               n);

void                 gl_font_history_model_add_family       (glFontHistoryModel *this,
                                                             const gchar        *family);

GList               *gl_font_history_model_get_family_list  (glFontHistoryModel *this);
void                 gl_font_history_model_free_family_list (GList              *list);


G_END_DECLS

#endif /* __GL_FONT_HISTORY_MODEL_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
