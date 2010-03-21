/*
 *  media-select.h
 *  Copyright (C) 2001-2009  Jim Evins <evins@snaught.com>.
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

#ifndef __MEDIA_SELECT_H__
#define __MEDIA_SELECT_H__

#include <gtk/gtk.h>


G_BEGIN_DECLS

#define GL_TYPE_MEDIA_SELECT (gl_media_select_get_type ())
#define GL_MEDIA_SELECT(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), GL_TYPE_MEDIA_SELECT, glMediaSelect ))
#define GL_MEDIA_SELECT_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_MEDIA_SELECT, glMediaSelectClass))
#define GL_IS_MEDIA_SELECT(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_MEDIA_SELECT))
#define GL_IS_MEDIA_SELECT_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_MEDIA_SELECT))

typedef struct _glMediaSelect         glMediaSelect;
typedef struct _glMediaSelectClass    glMediaSelectClass;

typedef struct _glMediaSelectPrivate  glMediaSelectPrivate;

struct _glMediaSelect {
        GtkVBox                    parent_widget;

        glMediaSelectPrivate  *priv;
};

struct _glMediaSelectClass {
        GtkVBoxClass               parent_class;

        void (*changed) (glMediaSelect *media_select, gpointer user_data);
};


GType      gl_media_select_get_type      (void) G_GNUC_CONST;

GtkWidget *gl_media_select_new           (void);

gchar     *gl_media_select_get_name      (glMediaSelect *this);

void       gl_media_select_set_name      (glMediaSelect *this,
                                          gchar         *name);

void       gl_media_select_get_filter_parameters (glMediaSelect *this,
                                                  gchar        **page_size_id,
                                                  gchar        **category_id);

void       gl_media_select_set_filter_parameters (glMediaSelect *this,
                                                  const gchar   *page_size_id,
                                                  const gchar   *category_id);

G_END_DECLS

#endif



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
