/*
 *  label-object.h
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

#ifndef __LABEL_OBJECT_H__
#define __LABEL_OBJECT_H__

#include <glib-object.h>
#include <gtk/gtk.h>
#include <pango/pango.h>
#include <cairo.h>

G_BEGIN_DECLS


typedef enum {
	GL_VALIGN_TOP,
	GL_VALIGN_VCENTER,
	GL_VALIGN_BOTTOM
} glValignment;


typedef enum {
        GL_LABEL_OBJECT_TEXT,
        GL_LABEL_OBJECT_BOX,
        GL_LABEL_OBJECT_LINE,
        GL_LABEL_OBJECT_ELLIPSE,
        GL_LABEL_OBJECT_IMAGE,
        GL_LABEL_OBJECT_BARCODE,
        GL_LABEL_OBJECT_N_TYPES
} glLabelObjectType;


typedef enum {
        GL_LABEL_OBJECT_HANDLE_NONE = 0,
        GL_LABEL_OBJECT_HANDLE_N,
        GL_LABEL_OBJECT_HANDLE_E,
        GL_LABEL_OBJECT_HANDLE_W,
        GL_LABEL_OBJECT_HANDLE_S,
        GL_LABEL_OBJECT_HANDLE_NW,
        GL_LABEL_OBJECT_HANDLE_NE,
        GL_LABEL_OBJECT_HANDLE_SE,
        GL_LABEL_OBJECT_HANDLE_SW,
        GL_LABEL_OBJECT_HANDLE_P1,
        GL_LABEL_OBJECT_HANDLE_P2
} glLabelObjectHandle;


#define GL_TYPE_LABEL_OBJECT              (gl_label_object_get_type ())
#define GL_LABEL_OBJECT(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_LABEL_OBJECT, glLabelObject))
#define GL_LABEL_OBJECT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_LABEL_OBJECT, glLabelObjectClass))
#define GL_IS_LABEL_OBJECT(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_LABEL_OBJECT))
#define GL_IS_LABEL_OBJECT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_LABEL_OBJECT))
#define GL_LABEL_OBJECT_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), GL_TYPE_LABEL_OBJECT, glLabelObjectClass))


typedef struct _glLabelObject          glLabelObject;
typedef struct _glLabelObjectClass     glLabelObjectClass;

typedef struct _glLabelObjectPrivate   glLabelObjectPrivate;

#include "label.h"
#include "color.h"

struct _glLabelObject {
        GObject               object;

        glLabelObjectPrivate *priv;
};

struct _glLabelObjectClass {
        GObjectClass          parent_class;

        /*
         * Set/Get Methods
         */

        void              (*set_size)             (glLabelObject     *object,
                                                   gdouble            w,
                                                   gdouble            h,
                                                   gboolean           checkpoint);

        void              (*set_font_family)      (glLabelObject     *object,
                                                   const gchar       *font_family,
                                                   gboolean           checkpoint);

        void              (*set_font_size)        (glLabelObject     *object,
                                                   gdouble            font_size,
                                                   gboolean           checkpoint);

        void              (*set_font_weight)      (glLabelObject     *object,
                                                   PangoWeight        font_weight,
                                                   gboolean           checkpoint);

        void              (*set_font_italic_flag) (glLabelObject     *object,
                                                   gboolean           font_italic_flag,
                                                   gboolean           checkpoint);

        void              (*set_text_alignment)   (glLabelObject     *object,
                                                   PangoAlignment     text_alignment,
                                                   gboolean           checkpoint);

        void              (*set_text_valignment)  (glLabelObject     *object,
                                                   glValignment       text_valignment,
                                                   gboolean           checkpoint);

        void              (*set_text_line_spacing)(glLabelObject     *object,
                                                   gdouble            text_line_spacing,
                                                   gboolean           checkpoint);

        void              (*set_text_color)       (glLabelObject     *object,
                                                   glColorNode       *text_color_node,
                                                   gboolean           checkpoint);

        void              (*set_fill_color)       (glLabelObject     *object,
                                                   glColorNode       *fill_color_node,
                                                   gboolean           checkpoint);

        void              (*set_line_color)       (glLabelObject     *object,
                                                   glColorNode       *line_color_node,
                                                   gboolean           checkpoint);

        void              (*set_line_width)       (glLabelObject     *object,
                                                   gdouble            line_width,
                                                   gboolean           checkpoint);

        void              (*get_size)             (glLabelObject     *object,
                                                   gdouble           *w,
                                                   gdouble           *h);

        gchar *           (*get_font_family)      (glLabelObject     *object);

        gdouble           (*get_font_size)        (glLabelObject     *object);

        PangoWeight       (*get_font_weight)      (glLabelObject     *object);

        gboolean          (*get_font_italic_flag) (glLabelObject     *object);

        PangoAlignment    (*get_text_alignment)   (glLabelObject     *object);

        glValignment      (*get_text_valignment)  (glLabelObject     *object);

        gdouble           (*get_text_line_spacing) (glLabelObject    *object);

        glColorNode *     (*get_text_color)       (glLabelObject     *object);

        glColorNode *     (*get_fill_color)       (glLabelObject     *object);

        glColorNode *     (*get_line_color)       (glLabelObject     *object);

        gdouble           (*get_line_width)       (glLabelObject     *object);

        void              (*copy)                 (glLabelObject     *dst_object
,
                                                   glLabelObject     *src_object);

        /*
         * Draw methods
         */
        void        (*draw_shadow)      (glLabelObject *object,
                                         cairo_t       *cr,
                                         gboolean       screen_flag,
                                         glMergeRecord *record);

        void        (*draw_object)      (glLabelObject *object,
                                         cairo_t       *cr,
                                         gboolean       screen_flag,
                                         glMergeRecord *record);

        void        (*draw_handles)     (glLabelObject *object,
                                         cairo_t       *cr);

        /*
         * Cairo context query methods
         */
        gboolean            (*object_at) (glLabelObject     *object,
                                          cairo_t           *cr,
                                          gdouble            x_pixels,
                                          gdouble            y_pixels);

        glLabelObjectHandle (*handle_at) (glLabelObject     *object,
                                          cairo_t           *cr,
                                          gdouble            x_pixels,
                                          gdouble            y_pixels);


        /*
         * Signals
         */

        void        (*changed) (glLabelObject *object,
                                gpointer       user_data);

        void        (*moved)   (glLabelObject *object,
                                gpointer       user_data);

};



GType          gl_label_object_get_type              (void) G_GNUC_CONST;

GObject       *gl_label_object_new                   (glLabel           *label);


glLabelObject *gl_label_object_dup                   (glLabelObject     *src_object,
                                                      glLabel           *label);


void           gl_label_object_emit_changed          (glLabelObject     *object);

void           gl_label_object_set_parent            (glLabelObject     *object,
                                                      glLabel           *label);

glLabel       *gl_label_object_get_parent            (glLabelObject     *object);


void           gl_label_object_select                (glLabelObject     *object);
void           gl_label_object_unselect              (glLabelObject     *object);
gboolean       gl_label_object_is_selected           (glLabelObject     *object);

void           gl_label_object_set_name              (glLabelObject     *object,
                                                      gchar             *name);

gchar         *gl_label_object_get_name              (glLabelObject     *object);


void           gl_label_object_set_position          (glLabelObject     *object,
                                                      gdouble            x,
                                                      gdouble            y,
                                                      gboolean           checkpoint);

void           gl_label_object_set_position_relative (glLabelObject     *object,
                                                      gdouble            dx,
                                                      gdouble            dy,
                                                      gboolean           checkpoint);

void           gl_label_object_set_size              (glLabelObject     *object,
                                                      gdouble            w,
                                                      gdouble            h,
                                                      gboolean           checkpoint);

void           gl_label_object_set_raw_size          (glLabelObject     *object,
                                                      gdouble            w,
                                                      gdouble            h,
                                                      gboolean           checkpoint);

void           gl_label_object_set_size_honor_aspect (glLabelObject     *object,
                                                      gdouble            w,
                                                      gdouble            h,
                                                      gboolean           checkpoint);

void           gl_label_object_set_font_family       (glLabelObject     *object,
                                                      const gchar       *font_family,
                                                      gboolean           checkpoint);

void           gl_label_object_set_font_size         (glLabelObject     *object,
                                                      gdouble            font_size,
                                                      gboolean           checkpoint);

void           gl_label_object_set_font_weight       (glLabelObject     *object,
                                                      PangoWeight        font_weight,
                                                      gboolean           checkpoint);

void           gl_label_object_set_font_italic_flag  (glLabelObject     *object,
                                                      gboolean           font_italic_flag,
                                                      gboolean           checkpoint);

void           gl_label_object_set_text_alignment    (glLabelObject     *object,
                                                      PangoAlignment     text_alignment,
                                                      gboolean           checkpoint);

void           gl_label_object_set_text_valignment   (glLabelObject     *object,
                                                      glValignment       text_valignment,
                                                      gboolean           checkpoint);

void           gl_label_object_set_text_color        (glLabelObject     *object,
                                                      glColorNode       *text_color_node,
                                                      gboolean           checkpoint);

void           gl_label_object_set_text_line_spacing (glLabelObject     *object,
                                                      gdouble            text_line_spacing,
                                                      gboolean           checkpoint);

void           gl_label_object_set_fill_color        (glLabelObject     *object,
                                                      glColorNode       *fill_color_node,
                                                      gboolean           checkpoint);

void           gl_label_object_set_line_color        (glLabelObject     *object,
                                                      glColorNode       *line_color_node,
                                                      gboolean           checkpoint);

void           gl_label_object_set_line_width        (glLabelObject     *object,
                                                      gdouble            line_width,
                                                      gboolean           checkpoint);


void           gl_label_object_set_shadow_state      (glLabelObject     *object,
                                                      gboolean           state,
                                                      gboolean           checkpoint);

void           gl_label_object_set_shadow_offset     (glLabelObject     *object,
                                                      gdouble            x,
                                                      gdouble            y,
                                                      gboolean           checkpoint);

void           gl_label_object_set_shadow_color      (glLabelObject     *object,
                                                      glColorNode       *color_node,
                                                      gboolean           checkpoint);

void           gl_label_object_set_shadow_opacity    (glLabelObject     *object,
                                                      gdouble            alpha,
                                                      gboolean           checkpoint);


void           gl_label_object_flip_horiz            (glLabelObject     *object);

void           gl_label_object_flip_vert             (glLabelObject     *object);

void           gl_label_object_rotate                (glLabelObject     *object,
                                                      gdouble            theta_degs);

void           gl_label_object_set_matrix            (glLabelObject     *object,
                                                      cairo_matrix_t    *matrix);


void           gl_label_object_get_position          (glLabelObject     *object,
                                                      gdouble           *x,
                                                      gdouble           *y);

void           gl_label_object_get_size              (glLabelObject     *object,
                                                      gdouble           *w,
                                                      gdouble           *h);

void           gl_label_object_get_raw_size          (glLabelObject     *object,
                                                      gdouble           *w,
                                                      gdouble           *h);

void           gl_label_object_get_extent            (glLabelObject     *object,
                                                      glLabelRegion     *region);

gboolean       gl_label_object_can_text              (glLabelObject     *object);


gchar         *gl_label_object_get_font_family       (glLabelObject     *object);

gdouble        gl_label_object_get_font_size         (glLabelObject     *object);

PangoWeight    gl_label_object_get_font_weight       (glLabelObject     *object);

gboolean       gl_label_object_get_font_italic_flag  (glLabelObject     *object);

PangoAlignment gl_label_object_get_text_alignment    (glLabelObject     *object);

glValignment   gl_label_object_get_text_valignment   (glLabelObject     *object);

gdouble        gl_label_object_get_text_line_spacing (glLabelObject     *object);

glColorNode   *gl_label_object_get_text_color        (glLabelObject     *object);


gboolean       gl_label_object_can_fill              (glLabelObject     *object);

glColorNode*   gl_label_object_get_fill_color        (glLabelObject     *object);


gboolean       gl_label_object_can_line_color        (glLabelObject     *object);

glColorNode   *gl_label_object_get_line_color        (glLabelObject     *object);

gboolean       gl_label_object_can_line_width        (glLabelObject     *object);

gdouble        gl_label_object_get_line_width        (glLabelObject     *object);


void           gl_label_object_get_matrix            (glLabelObject     *object,
                                                      cairo_matrix_t    *matrix);

gboolean       gl_label_object_get_shadow_state      (glLabelObject     *object);

void           gl_label_object_get_shadow_offset     (glLabelObject     *object,
                                                      gdouble           *x,
                                                      gdouble           *y);

glColorNode*   gl_label_object_get_shadow_color      (glLabelObject     *object);

gdouble        gl_label_object_get_shadow_opacity    (glLabelObject     *object);


void           gl_label_object_draw                  (glLabelObject     *object,
                                                      cairo_t           *cr,
                                                      gboolean           screen_flag,
                                                      glMergeRecord     *record);

gboolean       gl_label_object_is_located_at         (glLabelObject     *object,
                                                      cairo_t           *cr,
                                                      gdouble            x_pixels,
                                                      gdouble            y_pixels);

void           gl_label_object_draw_handles          (glLabelObject     *object,
                                                      cairo_t           *cr);

glLabelObjectHandle gl_label_object_handle_at        (glLabelObject     *object,
                                                      cairo_t           *cr,
                                                      gdouble            x_pixels,
                                                      gdouble            y_pixels);


/*
 * Specific handle drawing methods.
 */
void           gl_label_object_draw_handles_box      (glLabelObject     *object,
                                                      cairo_t           *cr);

void           gl_label_object_draw_handles_line     (glLabelObject     *object,
                                                      cairo_t           *cr);


/*
 * Specific handle query methods.
 */
glLabelObjectHandle gl_label_object_box_handle_at    (glLabelObject     *object,
                                                      cairo_t           *cr,
                                                      gdouble            x,
                                                      gdouble            y);

glLabelObjectHandle gl_label_object_line_handle_at   (glLabelObject     *object,
                                                      cairo_t           *cr,
                                                      gdouble            x,
                                                      gdouble            y);



G_END_DECLS

#endif /* __LABEL_OBJECT_H__ */




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
