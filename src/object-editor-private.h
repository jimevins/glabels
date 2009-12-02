/*
 *  object-editor-private.h
 *  Copyright (C) 2003-2009  Jim Evins <evins@snaught.com>.
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

#ifndef __OBJECT_EDITOR_PRIVATE_H__
#define __OBJECT_EDITOR_PRIVATE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

struct _glObjectEditorPrivate {

	GtkBuilder *builder;
	GtkWidget  *editor_vbox;

        glLabel        *label;
        glLabelObject  *object;

	gdouble     units_per_point;

	GtkWidget  *title_image;
	GtkWidget  *title_label;
	GtkWidget  *notebook;

	GtkWidget  *pos_page_vbox;
	GtkWidget  *pos_x_spin;
	GtkWidget  *pos_y_spin;
	GtkWidget  *pos_x_units_label;
	GtkWidget  *pos_y_units_label;
	gdouble     x;
	gdouble     y;
	gdouble     x_max;
	gdouble     y_max;

	GtkWidget  *size_page_vbox;
	GtkWidget  *size_w_spin;
	GtkWidget  *size_h_spin;
	GtkWidget  *size_w_units_label;
	GtkWidget  *size_h_units_label;
	GtkWidget  *size_aspect_vbox;
	GtkWidget  *size_aspect_checkbutton;
	GtkWidget  *size_reset_image_button;
	gdouble     size_aspect_ratio;
	gdouble     w;
	gdouble     h;
	gdouble     w_max;
	gdouble     h_max;
	gdouble     w_base;
	gdouble     h_base;

	GtkWidget  *lsize_page_vbox;
	GtkWidget  *lsize_r_spin;
	GtkWidget  *lsize_theta_spin;
	GtkWidget  *lsize_r_units_label;
	gdouble     dx;
	gdouble     dy;
	gdouble     dx_max;
	gdouble     dy_max;

	GtkWidget  *fill_page_vbox;
	GtkWidget  *fill_color_hbox;
	GtkWidget  *fill_color_combo;
	GtkWidget  *fill_key_combo;
	GtkWidget  *fill_key_radio;
	GtkWidget  *fill_color_radio;

	GtkWidget  *line_page_vbox;
	GtkWidget  *line_width_spin;
	GtkWidget  *line_color_radio;
	GtkWidget  *line_color_hbox;
	GtkWidget  *line_color_combo;
	GtkWidget  *line_key_radio;
	GtkWidget  *line_key_combo;

	GtkWidget  *img_page_vbox;
	GtkWidget  *img_file_radio;
	GtkWidget  *img_key_radio;
	GtkWidget  *img_file_button;
	GtkWidget  *img_key_combo;

	GtkWidget  *text_page_vbox;
	GtkWidget  *text_family_hbox;
	GtkWidget  *text_family_combo;
	GtkWidget  *text_size_spin;
	GtkWidget  *text_bold_toggle;
	GtkWidget  *text_italic_toggle;
	GtkWidget  *text_color_radio;
	GtkWidget  *text_color_hbox;
	GtkWidget  *text_color_combo;
	GtkWidget  *text_color_key_radio;
	GtkWidget  *text_color_key_combo;
	GtkWidget  *text_left_toggle;
	GtkWidget  *text_center_toggle;
	GtkWidget  *text_right_toggle;
	GtkWidget  *text_line_spacing_spin;
	GtkWidget  *text_auto_shrink_check;

	GtkWidget  *edit_page_vbox;
	GtkWidget  *edit_text_view;
	GtkWidget  *edit_insert_field_vbox;
	GtkWidget  *edit_insert_field_button;

	GtkWidget  *bc_page_vbox;
	GtkWidget  *bc_style_combo;
	GtkWidget  *bc_text_check;
	GtkWidget  *bc_cs_check;
	GtkWidget  *bc_color_radio;
	GtkWidget  *bc_color_hbox;
	GtkWidget  *bc_color_combo;
	GtkWidget  *bc_key_radio;
	GtkWidget  *bc_key_combo;

	GtkWidget  *data_page_vbox;
	GtkWidget  *data_literal_radio;
	GtkWidget  *data_key_radio;
	GtkWidget  *data_text_entry;
	GtkWidget  *data_key_combo;
	GtkWidget  *data_format_label;
	GtkWidget  *data_ex_label;
	GtkWidget  *data_digits_label;
	GtkWidget  *data_digits_spin;
	gboolean    data_format_fixed_flag;

	GtkWidget  *shadow_page_vbox;
	GtkWidget  *shadow_enable_check;
	GtkWidget  *shadow_controls_table;
	GtkWidget  *shadow_x_spin;
	GtkWidget  *shadow_y_spin;
	GtkWidget  *shadow_x_units_label;
	GtkWidget  *shadow_y_units_label;
	GtkWidget  *shadow_color_radio;
	GtkWidget  *shadow_key_radio;
	GtkWidget  *shadow_color_hbox;
	GtkWidget  *shadow_color_combo;
	GtkWidget  *shadow_key_combo;
	GtkWidget  *shadow_opacity_spin;
	gdouble     shadow_x;
	gdouble     shadow_y;
	gdouble     shadow_x_max;
	gdouble     shadow_y_max;

        /* Prevent recursion */
	gboolean    stop_signals;
};

enum {
	CHANGED,
	SIZE_CHANGED,
	LAST_SIGNAL
};

extern gint gl_object_editor_signals[LAST_SIGNAL];



void gl_object_editor_prepare_position_page     (glObjectEditor        *editor);

void gl_object_editor_prepare_size_page         (glObjectEditor        *editor);

void gl_object_editor_prepare_lsize_page        (glObjectEditor        *editor);

void gl_object_editor_prepare_fill_page         (glObjectEditor        *editor);

void gl_object_editor_prepare_line_page         (glObjectEditor        *editor);

void gl_object_editor_prepare_image_page        (glObjectEditor        *editor);

void gl_object_editor_prepare_text_page         (glObjectEditor        *editor);

void gl_object_editor_prepare_edit_page         (glObjectEditor        *editor);

void gl_object_editor_prepare_bc_page           (glObjectEditor        *editor);

void gl_object_editor_prepare_data_page         (glObjectEditor        *editor);

void gl_object_editor_prepare_shadow_page       (glObjectEditor        *editor);


void gl_object_editor_changed_cb                (glObjectEditor        *editor);
void gl_object_editor_size_changed_cb           (glObjectEditor        *editor);

void lsize_prefs_changed_cb                     (glObjectEditor        *editor);
void size_prefs_changed_cb                      (glObjectEditor        *editor);
void position_prefs_changed_cb                  (glObjectEditor        *editor);
void shadow_prefs_changed_cb                    (glObjectEditor        *editor);


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



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
