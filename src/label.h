/*
 *  label.h
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

#ifndef __LABEL_H__
#define __LABEL_H__

#include <gtk/gtk.h>
#include <cairo.h>

#include <libglabels/libglabels.h>
#include "merge.h"
#include "color.h"
#include "pixbuf-cache.h"
#include "svg-cache.h"

G_BEGIN_DECLS

typedef struct {
        gdouble x1;
        gdouble y1;
        gdouble x2;
        gdouble y2;
} glLabelRegion;

#define GL_TYPE_LABEL            (gl_label_get_type ())
#define GL_LABEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_LABEL, glLabel))
#define GL_LABEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_LABEL, glLabelClass))
#define GL_IS_LABEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_LABEL))
#define GL_IS_LABEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_LABEL))

typedef struct _glLabel          glLabel;
typedef struct _glLabelClass     glLabelClass;

typedef struct _glLabelPrivate   glLabelPrivate;

#include "label-object.h"
struct _glLabel {
	GObject         object;

	glLabelPrivate *priv;
};

struct _glLabelClass {
	GObjectClass         parent_class;

	void (*selection_changed) (glLabel       *view,
				   gpointer       user_data);

	void (*changed)           (glLabel       *label,
				   gpointer       user_data);

	void (*name_changed)      (glLabel       *label,
				   gpointer       user_data);

	void (*modified_changed)  (glLabel       *label,
				   gpointer       user_data);

	void (*merge_changed)     (glLabel       *label,
				   gpointer       user_data);

	void (*size_changed)      (glLabel       *label,
				   gpointer       user_data);
};


GType         gl_label_get_type                (void) G_GNUC_CONST;

GObject      *gl_label_new                     (void);


void          gl_label_set_filename            (glLabel       *label,
						const gchar   *filename);

gchar        *gl_label_get_filename            (glLabel       *label);

gchar        *gl_label_get_short_name          (glLabel       *label);

gboolean      gl_label_is_untitled             (glLabel       *label);


void          gl_label_set_compression         (glLabel       *label,
						gint           compression);

gint          gl_label_get_compression         (glLabel       *label);


void          gl_label_set_modified            (glLabel       *label);

void          gl_label_clear_modified          (glLabel       *label);

gboolean      gl_label_is_modified             (glLabel       *label);


void          gl_label_set_template            (glLabel            *label,
						const lglTemplate  *template,
                                                gboolean            checkpoint);

const lglTemplate *gl_label_get_template       (glLabel            *label);

void          gl_label_set_rotate_flag         (glLabel       *label,
						gboolean       rotate_flag,
                                                gboolean       checkpoint);

gboolean      gl_label_get_rotate_flag         (glLabel       *label);

void          gl_label_get_size                (glLabel       *label,
						gdouble       *w,
						gdouble       *h);


void          gl_label_set_merge               (glLabel       *label,
						glMerge       *merge,
                                                gboolean       checkpoint);

glMerge      *gl_label_get_merge               (glLabel       *label);

GHashTable   *gl_label_get_pixbuf_cache        (glLabel       *label);


GHashTable   *gl_label_get_svg_cache           (glLabel       *label);


void          gl_label_add_object              (glLabel       *label,
						glLabelObject *object);


void          gl_label_delete_object           (glLabel       *label,
						glLabelObject *object);

const GList  *gl_label_get_object_list         (glLabel       *label);



/*
 * Modify selection methods
 */
void          gl_label_select_object           (glLabel       *label,
                                                glLabelObject *object);

void          gl_label_unselect_object         (glLabel       *label,
                                                glLabelObject *object);

void          gl_label_select_all              (glLabel       *label);

void          gl_label_unselect_all            (glLabel       *label);

void          gl_label_select_region           (glLabel       *label,
                                                glLabelRegion *region);


/*
 * Selection query methods
 */
gboolean       gl_label_is_selection_empty      (glLabel       *label);

gboolean       gl_label_is_selection_atomic     (glLabel       *label);

glLabelObject *gl_label_get_1st_selected_object (glLabel       *label);

GList         *gl_label_get_selection_list      (glLabel       *label);

gboolean       gl_label_can_selection_text      (glLabel       *label);

gboolean       gl_label_can_selection_fill      (glLabel       *label);

gboolean       gl_label_can_selection_line_color(glLabel       *label);

gboolean       gl_label_can_selection_line_width(glLabel       *label);


/*
 * Perform operations on selections
 */
void          gl_label_delete_selection          (glLabel       *label);

void          gl_label_raise_selection_to_top    (glLabel       *label);

void          gl_label_lower_selection_to_bottom (glLabel       *label);

void          gl_label_rotate_selection          (glLabel       *label,
                                                  gdouble        theta_degs);

void          gl_label_rotate_selection_left     (glLabel       *label);

void          gl_label_rotate_selection_right    (glLabel       *label);

void          gl_label_flip_selection_horiz      (glLabel       *label);

void          gl_label_flip_selection_vert       (glLabel       *label);

void          gl_label_align_selection_left      (glLabel       *label);

void          gl_label_align_selection_right     (glLabel       *label);

void          gl_label_align_selection_hcenter   (glLabel       *label);

void          gl_label_align_selection_top       (glLabel       *label);

void          gl_label_align_selection_bottom    (glLabel       *label);

void          gl_label_align_selection_vcenter   (glLabel       *label);

void          gl_label_center_selection_horiz    (glLabel       *label);

void          gl_label_center_selection_vert     (glLabel       *label);

void          gl_label_move_selection            (glLabel       *label,
                                                  gdouble        dx,
                                                  gdouble        dy);

void          gl_label_set_selection_font_family(glLabel        *label,
                                                 const gchar    *font_family);

void          gl_label_set_selection_font_size  (glLabel        *label,
                                                 gdouble         font_size);

void          gl_label_set_selection_font_weight(glLabel        *label,
                                                 PangoWeight     font_weight);

void          gl_label_set_selection_font_italic_flag (glLabel   *label,
                                                       gboolean   font_italic_flag);

void          gl_label_set_selection_text_alignment (glLabel        *label,
                                                     PangoAlignment  text_alignment);

void          gl_label_set_selection_text_line_spacing (glLabel  *label,
                                                        gdouble   text_line_spacing);

void          gl_label_set_selection_text_color (glLabel        *label,
                                                 glColorNode    *text_color_node);

void          gl_label_set_selection_fill_color (glLabel        *label,
                                                 glColorNode    *fill_color_node);

void          gl_label_set_selection_line_color (glLabel        *label,
                                                 glColorNode    *line_color_node);

void          gl_label_set_selection_line_width (glLabel        *label,
                                                 gdouble         line_width);


/*
 * Clipboard operations
 */
void          gl_label_cut_selection             (glLabel       *label);

void          gl_label_copy_selection            (glLabel       *label);

void          gl_label_paste                     (glLabel       *label);

gboolean      gl_label_can_paste                 (glLabel       *label);


/*
 * Set/get current default values.
 */
void       gl_label_set_default_font_family      (glLabel            *label,
                                                  const gchar       *font_family);

void       gl_label_set_default_font_size        (glLabel            *label,
                                                  gdouble            font_size);

void       gl_label_set_default_font_weight      (glLabel            *label,
                                                  PangoWeight        font_weight);

void       gl_label_set_default_font_italic_flag (glLabel            *label,
                                                  gboolean           font_italic_flag);

void       gl_label_set_default_text_color       (glLabel            *label,
                                                  guint              text_color);

void       gl_label_set_default_text_alignment   (glLabel            *label,
                                                  PangoAlignment     text_alignment);

void       gl_label_set_default_text_valignment  (glLabel            *label,
                                                  glValignment       text_valignment);

void       gl_label_set_default_line_width       (glLabel            *label,
                                                  gdouble            line_width);

void       gl_label_set_default_line_color       (glLabel            *label,
                                                  guint              line_color);

void       gl_label_set_default_fill_color       (glLabel            *label,
                                                  guint              fill_color);

void       gl_label_set_default_text_line_spacing(glLabel            *label,
                                                  gdouble            text_line_spacing);

gchar           *gl_label_get_default_font_family      (glLabel            *label);

gdouble          gl_label_get_default_font_size        (glLabel            *label);

PangoWeight      gl_label_get_default_font_weight      (glLabel            *label);

gboolean         gl_label_get_default_font_italic_flag (glLabel            *label);

guint            gl_label_get_default_text_color       (glLabel            *label);

PangoAlignment   gl_label_get_default_text_alignment   (glLabel            *label);

glValignment     gl_label_get_default_text_valignment  (glLabel            *label);

gdouble          gl_label_get_default_text_line_spacing(glLabel            *label);

gdouble          gl_label_get_default_line_width       (glLabel            *label);

guint            gl_label_get_default_line_color       (glLabel            *label);

guint            gl_label_get_default_fill_color       (glLabel            *view);


/*
 * Drawing methods
 */
void           gl_label_draw                   (glLabel       *label,
                                                cairo_t       *cr,
                                                gboolean       screen_flag,
                                                glMergeRecord *record);

glLabelObject *gl_label_object_at              (glLabel       *label,
                                                cairo_t       *cr,
                                                gdouble        x_pixels,
                                                gdouble        y_pixels);

glLabelObject *gl_label_get_handle_at          (glLabel             *label,
                                                cairo_t             *cr,
                                                gdouble              x_pixels,
                                                gdouble              y_pixels,
                                                glLabelObjectHandle *handle);


/*
 * Undo/Redo methods
 */
void          gl_label_checkpoint              (glLabel       *label,
                                                const gchar   *description);

gboolean      gl_label_can_undo                (glLabel       *label);
gboolean      gl_label_can_redo                (glLabel       *label);

gchar        *gl_label_get_undo_description    (glLabel       *label);
gchar        *gl_label_get_redo_description    (glLabel       *label);

void          gl_label_undo                    (glLabel       *label);
void          gl_label_redo                    (glLabel       *label);


G_END_DECLS


#endif /* __LABEL_H__ */




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
