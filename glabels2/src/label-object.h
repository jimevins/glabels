/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label_object.h:  GLabels label object base class
 *
 *  Copyright (C) 2001-2002  Jim Evins <evins@snaught.com>.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#ifndef __LABEL_OBJECT_H__
#define __LABEL_OBJECT_H__

#include <glib-object.h>
#include <gtk/gtkenums.h>
#include <libgnomeprint/gnome-font.h>

G_BEGIN_DECLS

typedef enum {
        GL_LABEL_OBJECT_TEXT,
        GL_LABEL_OBJECT_BOX,
        GL_LABEL_OBJECT_LINE,
        GL_LABEL_OBJECT_ELLIPSE,
        GL_LABEL_OBJECT_IMAGE,
        GL_LABEL_OBJECT_BARCODE,
        GL_LABEL_OBJECT_N_TYPES
} glLabelObjectType;


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

struct _glLabelObject {
	GObject               object;

	glLabel              *parent;

	glLabelObjectPrivate *private;
};

struct _glLabelObjectClass {
	GObjectClass          parent_class;

	/*
	 * Methods
	 */

	void              (*set_size)             (glLabelObject     *object,
						   gdouble            w,
						   gdouble            h);

	void              (*get_size)             (glLabelObject     *object,
						   gdouble           *w,
						   gdouble           *h);

	void              (*set_font_family)      (glLabelObject     *object,
						   const gchar       *font_family);

	void              (*set_font_size)        (glLabelObject     *object,
						   gdouble            font_size);

	void              (*set_font_weight)      (glLabelObject     *object,
						   GnomeFontWeight    font_weight);

	void              (*set_font_italic_flag) (glLabelObject     *object,
						   gboolean           font_italic_flag);

	void              (*set_text_alignment)   (glLabelObject     *object,
						   GtkJustification   text_alignment);

	void              (*set_text_line_spacing) (glLabelObject    *object,
						    gdouble           text_line_spacing);

	void              (*set_text_color)       (glLabelObject     *object,
						   guint              text_color);

	void              (*set_fill_color)       (glLabelObject     *object,
						   guint              fill_color);

	void              (*set_line_color)       (glLabelObject     *object,
						   guint              line_color);

	void              (*set_line_width)       (glLabelObject     *object,
						   gdouble            line_width);

	gchar            *(*get_font_family)      (glLabelObject     *object);

	gdouble           (*get_font_size)        (glLabelObject     *object);

	GnomeFontWeight   (*get_font_weight)      (glLabelObject     *object);

	gboolean          (*get_font_italic_flag) (glLabelObject     *object);

	GtkJustification  (*get_text_alignment)   (glLabelObject     *object);

	gdouble           (*get_text_line_spacing) (glLabelObject    *object);

	guint             (*get_text_color)       (glLabelObject     *object);

	guint             (*get_fill_color)       (glLabelObject     *object);

	guint             (*get_line_color)       (glLabelObject     *object);

	gdouble           (*get_line_width)       (glLabelObject     *object);

	void              (*copy)                 (glLabelObject     *dst_object,
						   glLabelObject     *src_object);

	/*
	 * Signals
	 */
	void (*changed)     (glLabelObject     *object,
			     gpointer            user_data);

	void (*moved)       (glLabelObject     *object,
			     gdouble            dx,
			     gdouble            dy,
			     gpointer           user_data);

	void (*flip_rotate) (glLabelObject     *object,
			     gpointer           user_data);

	void (*top)         (glLabelObject     *object,
			     gpointer           user_data);

	void (*bottom)      (glLabelObject     *object,
			     gpointer           user_data);
};

GType          gl_label_object_get_type              (void) G_GNUC_CONST;

GObject       *gl_label_object_new                   (glLabel           *label);


glLabelObject *gl_label_object_dup                   (glLabelObject     *src_object,
						      glLabel           *label);

void           gl_label_object_emit_changed          (glLabelObject     *object);


void           gl_label_object_set_parent            (glLabelObject     *object,
						      glLabel           *label);

glLabel       *gl_label_object_get_parent            (glLabelObject     *object);


void           gl_label_object_set_name              (glLabelObject     *object,
						      gchar             *name);

gchar         *gl_label_object_get_name              (glLabelObject     *object);


void           gl_label_object_set_position          (glLabelObject     *object,
						      gdouble            x,
						      gdouble            y);

void           gl_label_object_set_position_relative (glLabelObject     *object,
						      gdouble            dx,
						      gdouble            dy);

void           gl_label_object_get_position          (glLabelObject     *object,
						      gdouble           *x,
						      gdouble           *y);

void           gl_label_object_set_size              (glLabelObject     *object,
						      gdouble            w,
						      gdouble            h);

void           gl_label_object_set_size_honor_aspect (glLabelObject     *object,
						      gdouble            w,
						      gdouble            h);

void           gl_label_object_get_size              (glLabelObject     *object,
						      gdouble           *w,
						      gdouble           *h);

void           gl_label_object_get_extent            (glLabelObject     *object,
						      gdouble           *x1,
						      gdouble           *y1,
						      gdouble           *x2,
						      gdouble           *y2);


gboolean       gl_label_object_can_text              (glLabelObject     *object);

void           gl_label_object_set_font_family       (glLabelObject     *object,
						      const gchar       *font_family);

void           gl_label_object_set_font_size         (glLabelObject     *object,
						      gdouble            font_size);

void           gl_label_object_set_font_weight       (glLabelObject     *object,
						      GnomeFontWeight    font_weight);

void           gl_label_object_set_font_italic_flag  (glLabelObject     *object,
						      gboolean           font_italic_flag);

void           gl_label_object_set_text_alignment    (glLabelObject     *object,
						      GtkJustification   text_alignment);

void           gl_label_object_set_text_color        (glLabelObject     *object,
						      guint              text_color);

void           gl_label_object_set_text_line_spacing (glLabelObject     *object,
						      gdouble            text_line_spacing);


gchar           *gl_label_object_get_font_family       (glLabelObject     *object);

gdouble          gl_label_object_get_font_size         (glLabelObject     *object);

GnomeFontWeight  gl_label_object_get_font_weight       (glLabelObject     *object);

gboolean         gl_label_object_get_font_italic_flag  (glLabelObject     *object);

GtkJustification gl_label_object_get_text_alignment    (glLabelObject     *object);

gdouble          gl_label_object_get_text_line_spacing (glLabelObject     *object);

guint            gl_label_object_get_text_color        (glLabelObject     *object);


gboolean       gl_label_object_can_fill              (glLabelObject     *object);

void           gl_label_object_set_fill_color        (glLabelObject     *object,
						      guint              fill_color);

guint          gl_label_object_get_fill_color        (glLabelObject     *object);


gboolean       gl_label_object_can_line_color        (glLabelObject     *object);

void           gl_label_object_set_line_color        (glLabelObject     *object,
						      guint              line_color);

guint          gl_label_object_get_line_color        (glLabelObject     *object);

gboolean       gl_label_object_can_line_width        (glLabelObject     *object);

void           gl_label_object_set_line_width        (glLabelObject     *object,
						      gdouble            line_width);

gdouble        gl_label_object_get_line_width        (glLabelObject     *object);


void           gl_label_object_raise_to_top          (glLabelObject     *object);

void           gl_label_object_lower_to_bottom       (glLabelObject     *object);


void           gl_label_object_flip_horiz            (glLabelObject     *object);

void           gl_label_object_flip_vert             (glLabelObject     *object);

void           gl_label_object_rotate                (glLabelObject     *object,
						      gdouble            theta_degs);

void           gl_label_object_set_affine            (glLabelObject     *object,
						      gdouble           affine[6]);

void           gl_label_object_get_affine            (glLabelObject     *object,
						      gdouble           affine[6]);

void           gl_label_object_get_i2w_affine        (glLabelObject     *object,
						      gdouble           affine[6]);

void           gl_label_object_get_w2i_affine        (glLabelObject     *object,
						      gdouble           affine[6]);

G_END_DECLS

#endif /* __LABEL_OBJECT_H__ */
