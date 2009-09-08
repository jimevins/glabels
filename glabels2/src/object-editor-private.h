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

#include <gtk/gtkwidget.h>
#include <gtk/gtkbuilder.h>

G_BEGIN_DECLS

struct _glObjectEditorPrivate {

	GtkBuilder *builder;
	GtkWidget  *editor_vbox;

        glLabel    *label;

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
	GtkWidget  *edit_insert_field_button;
        GtkWidget  *edit_insert_field_menu;

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

void gl_object_editor_prepare_size_page         (glObjectEditor        *editor,
						 glObjectEditorOption   option);

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
