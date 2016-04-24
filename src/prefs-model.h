/*
 *  prefs-model.h
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

#ifndef __PREFS_MODEL_H__
#define __PREFS_MODEL_H__

#include <glib-object.h>
#include <pango/pango.h>
#include <libglabels/libglabels.h>

G_BEGIN_DECLS


#define GL_TYPE_PREFS_MODEL              (gl_prefs_model_get_type ())
#define GL_PREFS_MODEL(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_PREFS_MODEL, glPrefsModel))
#define GL_PREFS_MODEL_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_PREFS_MODEL, glPrefsModelClass))
#define GL_IS_PREFS_MODEL(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_PREFS_MODEL))
#define GL_IS_PREFS_MODEL_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_PREFS_MODEL))
#define GL_PREFS_MODEL_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), GL_TYPE_PREFS_MODEL, glPrefsModelClass))


typedef struct _glPrefsModel          glPrefsModel;
typedef struct _glPrefsModelClass     glPrefsModelClass;
typedef struct _glPrefsModelPrivate   glPrefsModelPrivate;


struct _glPrefsModel {
	GObject              parent;

        glPrefsModelPrivate *priv;
};

struct _glPrefsModelClass {
	GObjectClass         parent_class;

        void (*changed) (glPrefsModel *prefs_model,
                         gpointer      user_data);
};


GType          gl_prefs_model_get_type                      (void) G_GNUC_CONST;

glPrefsModel  *gl_prefs_model_new                           (void);

glPrefsModel  *gl_prefs_model_new_null                      (void);

void           gl_prefs_model_set_units                     (glPrefsModel     *this,
                                                             lglUnits          units);

lglUnits       gl_prefs_model_get_units                     (glPrefsModel     *this);


void           gl_prefs_model_set_default_page_size         (glPrefsModel     *this,
                                                             const gchar      *page_size);

gchar         *gl_prefs_model_get_default_page_size         (glPrefsModel     *this);


void           gl_prefs_model_set_default_font_family       (glPrefsModel     *this,
                                                             const gchar      *family);

gchar         *gl_prefs_model_get_default_font_family       (glPrefsModel     *this);


void           gl_prefs_model_set_default_font_size         (glPrefsModel     *this,
                                                             gdouble           size);

gdouble        gl_prefs_model_get_default_font_size         (glPrefsModel     *this);


void           gl_prefs_model_set_default_font_weight       (glPrefsModel     *this,
                                                             PangoWeight       weight);

PangoWeight    gl_prefs_model_get_default_font_weight       (glPrefsModel     *this);


void           gl_prefs_model_set_default_font_italic_flag  (glPrefsModel     *this,
                                                             gboolean          italic_flag);

gboolean       gl_prefs_model_get_default_font_italic_flag  (glPrefsModel     *this);


void           gl_prefs_model_set_default_text_color        (glPrefsModel     *this,
                                                             guint             color);

guint          gl_prefs_model_get_default_text_color        (glPrefsModel     *this);


void           gl_prefs_model_set_default_text_alignment    (glPrefsModel     *this,
                                                             PangoAlignment    alignment);

PangoAlignment gl_prefs_model_get_default_text_alignment    (glPrefsModel     *this);


void           gl_prefs_model_set_default_text_line_spacing (glPrefsModel     *this,
                                                             gdouble           spacing);

gdouble        gl_prefs_model_get_default_text_line_spacing (glPrefsModel     *this);


void           gl_prefs_model_set_default_line_width        (glPrefsModel     *this,
                                                             gdouble           width);

gdouble        gl_prefs_model_get_default_line_width        (glPrefsModel     *this);


void           gl_prefs_model_set_default_line_color        (glPrefsModel     *this,
                                                             guint             color);

guint          gl_prefs_model_get_default_line_color        (glPrefsModel     *this);


void           gl_prefs_model_set_default_fill_color        (glPrefsModel     *this,
                                                             guint             color);

guint          gl_prefs_model_get_default_fill_color        (glPrefsModel     *this);


void           gl_prefs_model_set_main_toolbar_visible      (glPrefsModel     *this,
                                                             gboolean          visible);

gboolean       gl_prefs_model_get_main_toolbar_visible      (glPrefsModel     *this);


void           gl_prefs_model_set_drawing_toolbar_visible   (glPrefsModel     *this,
                                                             gboolean          visible);

gboolean       gl_prefs_model_get_drawing_toolbar_visible   (glPrefsModel     *this);


void           gl_prefs_model_set_property_toolbar_visible  (glPrefsModel     *this,
                                                             gboolean          visible);

gboolean       gl_prefs_model_get_property_toolbar_visible  (glPrefsModel     *this);


void           gl_prefs_model_set_grid_visible              (glPrefsModel     *this,
                                                             gboolean          visible);

gboolean       gl_prefs_model_get_grid_visible              (glPrefsModel     *this);


void           gl_prefs_model_set_markup_visible            (glPrefsModel     *this,
                                                             gboolean          visible);

gboolean       gl_prefs_model_get_markup_visible            (glPrefsModel     *this);


void           gl_prefs_model_set_max_recents               (glPrefsModel     *this,
                                                             gint              max_recents);

gint           gl_prefs_model_get_max_recents               (glPrefsModel     *this);


G_END_DECLS

#endif /* __PREFS_MODEL_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
