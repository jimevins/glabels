/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  object-editor.h:  object properties editor module header file
 *
 *  Copyright (C) 2003  Jim Evins <evins@snaught.com>.
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
#ifndef __OBJECT_EDITOR_H__
#define __OBJECT_EDITOR_H__

#include <gtk/gtkvbox.h>
#include <gtk/gtktextbuffer.h>

#include "text-node.h"
#include "label.h"
#include "merge.h"
#include "bc.h"
#include "color.h"

G_BEGIN_DECLS

typedef enum {
	GL_OBJECT_EDITOR_EMPTY = 1,
	GL_OBJECT_EDITOR_POSITION_PAGE,
	GL_OBJECT_EDITOR_SIZE_PAGE,
	GL_OBJECT_EDITOR_SIZE_IMAGE_PAGE,
	GL_OBJECT_EDITOR_SIZE_LINE_PAGE,
	GL_OBJECT_EDITOR_FILL_PAGE,
	GL_OBJECT_EDITOR_LINE_PAGE,
	GL_OBJECT_EDITOR_IMAGE_PAGE,
	GL_OBJECT_EDITOR_TEXT_PAGE,
	GL_OBJECT_EDITOR_EDIT_PAGE,
	GL_OBJECT_EDITOR_BC_PAGE,
	GL_OBJECT_EDITOR_DATA_PAGE,
	GL_OBJECT_EDITOR_SHADOW_PAGE,
} glObjectEditorOption;

#define GL_TYPE_OBJECT_EDITOR            (gl_object_editor_get_type ())
#define GL_OBJECT_EDITOR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_OBJECT_EDITOR, glObjectEditor))
#define GL_OBJECT_EDITOR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_OBJECT_EDITOR, glObjectEditorClass))
#define GL_IS_OBJECT_EDITOR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_OBJECT_EDITOR))
#define GL_IS_OBJECT_EDITOR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_OBJECT_EDITOR))
#define GL_OBJECT_EDITOR_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_OBJECT_EDITOR, glObjectEditorClass))


typedef struct _glObjectEditor         glObjectEditor;
typedef struct _glObjectEditorClass    glObjectEditorClass;

typedef struct _glObjectEditorPrivate  glObjectEditorPrivate;

struct _glObjectEditor
{
	GtkVBox                parent_instance;

	glObjectEditorPrivate *priv;

};

struct  _glObjectEditorClass
{
	GtkVBoxClass             parent_class;

	void (*changed)      (glObjectEditor *editor, gpointer user_data);
	void (*size_changed) (glObjectEditor *editor, gpointer user_data);
};




GType       gl_object_editor_get_type             (void) G_GNUC_CONST;

GtkWidget  *gl_object_editor_new                  (gchar               *image,
						   gchar               *title,
                                                   glLabel             *label,
						   glObjectEditorOption first_option, ...);

void        gl_object_editor_set_key_names        (glObjectEditor      *editor,
						   glMerge             *merge);


/*
 * Position Page
 */
void        gl_object_editor_set_position         (glObjectEditor      *editor,
						   gdouble              x,
						   gdouble              y);

void        gl_object_editor_set_max_position     (glObjectEditor      *editor,
						   gdouble              x_max,
						   gdouble              y_max);

void        gl_object_editor_get_position         (glObjectEditor      *editor,
						   gdouble             *x,
						   gdouble             *y);

/*
 * Size Page
 */
void        gl_object_editor_set_size             (glObjectEditor      *editor,
						   gdouble              w,
						   gdouble              h);

void        gl_object_editor_set_max_size         (glObjectEditor      *editor,
						   gdouble              w_max,
						   gdouble              h_max);

void        gl_object_editor_set_base_size        (glObjectEditor      *editor,
						   gdouble              w_max,
						   gdouble              h_max);

void        gl_object_editor_get_size             (glObjectEditor      *editor,
						   gdouble             *w,
						   gdouble             *h);


/*
 * Line Size Page
 */
void        gl_object_editor_set_lsize            (glObjectEditor      *editor,
						   gdouble              dx,
						   gdouble              dy);

void        gl_object_editor_set_max_lsize        (glObjectEditor      *editor,
						   gdouble              dx_max,
						   gdouble              dy_max);

void        gl_object_editor_get_lsize            (glObjectEditor      *editor,
						   gdouble             *dx,
						   gdouble             *dy);


/*
 * Fill Page
 */
void        gl_object_editor_set_fill_color       (glObjectEditor      *editor,
						   gboolean             merge_flag,
						   glColorNode         *color_node);

glColorNode* gl_object_editor_get_fill_color      (glObjectEditor      *editor);


/*
 * Line/Outline Page
 */
void        gl_object_editor_set_line_color       (glObjectEditor      *editor,
						   gboolean             merge_flag,
						   glColorNode         *color_node);

glColorNode* gl_object_editor_get_line_color      (glObjectEditor      *editor);

void        gl_object_editor_set_line_width       (glObjectEditor      *editor,
						   gdouble              width);

gdouble     gl_object_editor_get_line_width       (glObjectEditor      *editor);


/*
 * Image Page
 */
void        gl_object_editor_set_image            (glObjectEditor      *editor,
						   gboolean             merge_flag,
						   glTextNode          *text_node);

glTextNode *gl_object_editor_get_image            (glObjectEditor      *editor);


/*
 * Text Page
 */
void        gl_object_editor_set_font_family      (glObjectEditor      *editor,
						   const gchar         *font_family);

gchar      *gl_object_editor_get_font_family      (glObjectEditor      *editor);

void        gl_object_editor_set_font_size        (glObjectEditor      *editor,
						   gdouble              font_size);

gdouble     gl_object_editor_get_font_size        (glObjectEditor      *editor);

void        gl_object_editor_set_font_weight      (glObjectEditor      *editor,
						   PangoWeight          font_weight);

PangoWeight gl_object_editor_get_font_weight      (glObjectEditor      *editor);

void        gl_object_editor_set_font_italic_flag (glObjectEditor      *editor,
						   gboolean             font_italic_flag);

gboolean    gl_object_editor_get_font_italic_flag (glObjectEditor      *editor);

void        gl_object_editor_set_text_alignment   (glObjectEditor      *editor,
						   PangoAlignment       text_alignment);

PangoAlignment gl_object_editor_get_text_alignment (glObjectEditor      *editor);

void        gl_object_editor_set_text_line_spacing (glObjectEditor      *editor,
						   gdouble               text_line_spacing);

gdouble     gl_object_editor_get_text_line_spacing (glObjectEditor      *editor);

void        gl_object_editor_set_text_color       (glObjectEditor       *editor,
						   gboolean              merge_flag,
						   glColorNode          *text_color_node);

glColorNode* gl_object_editor_get_text_color      (glObjectEditor      *editor);

void        gl_object_editor_set_text_auto_shrink (glObjectEditor      *editor,
						   gboolean             auto_shrink);

gboolean    gl_object_editor_get_text_auto_shrink (glObjectEditor      *editor);


/*
 * Edit Text Page
 */
void        gl_object_editor_set_text_buffer      (glObjectEditor      *editor,
						   GtkTextBuffer       *buffer);

/*
 * Barcode Page
 */
void        gl_object_editor_set_bc_style         (glObjectEditor      *editor,
						   gchar               *id,
						   gboolean             text_flag,
						   gboolean             checksum_flag,
						   guint                format_digits);

void        gl_object_editor_get_bc_style         (glObjectEditor      *editor,
						   gchar              **id,
						   gboolean            *text_flag,
						   gboolean            *checksum_flag,
						   guint               *format_digits);

void        gl_object_editor_set_bc_color         (glObjectEditor      *editor,
						   gboolean             merge_flag,
						   glColorNode         *color_node);

glColorNode* gl_object_editor_get_bc_color        (glObjectEditor      *editor);


/*
 * Barcode Data Page
 */
void        gl_object_editor_set_data             (glObjectEditor      *editor,
						   gboolean             merge_flag,
						   glTextNode          *text_node);

glTextNode *gl_object_editor_get_data             (glObjectEditor      *editor);


/*
 * Shadow Page
 */
void        gl_object_editor_set_shadow_state     (glObjectEditor      *editor,
						   gboolean             state);

void        gl_object_editor_set_shadow_offset    (glObjectEditor      *editor,
						   gdouble              x,
						   gdouble              y);

void        gl_object_editor_set_shadow_color     (glObjectEditor      *editor,
						   gboolean             merge_flag,
						   glColorNode         *color_node);

void        gl_object_editor_set_shadow_opacity   (glObjectEditor      *editor,
						   gdouble              alpha);

void        gl_object_editor_set_max_shadow_offset(glObjectEditor      *editor,
						   gdouble              x_max,
						   gdouble              y_max);


gboolean    gl_object_editor_get_shadow_state     (glObjectEditor      *editor);

void        gl_object_editor_get_shadow_offset    (glObjectEditor      *editor,
						   gdouble             *x,
						   gdouble             *y);

glColorNode* gl_object_editor_get_shadow_color    (glObjectEditor      *editor);

gdouble     gl_object_editor_get_shadow_opacity   (glObjectEditor      *editor);



G_END_DECLS

#endif
