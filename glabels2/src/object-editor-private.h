/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  object-editor-private.h:  object properties editor module private header file
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
#ifndef __OBJECT_EDITOR_PRIVATE_H__
#define __OBJECT_EDITOR_PRIVATE_H__

#include <gtk/gtk.h>
#include <glade/glade-xml.h>

G_BEGIN_DECLS

struct _glObjectEditorPrivate {

	GladeXML   *gui;
	GtkWidget  *editor_vbox;

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
	GtkWidget  *fill_color_combo;

	GtkWidget  *line_page_vbox;
	GtkWidget  *line_width_spin;
	GtkWidget  *line_color_combo;

	GtkWidget  *img_page_vbox;
	GtkWidget  *img_file_radio;
	GtkWidget  *img_key_radio;
	GtkWidget  *img_pixmap_entry;
	GtkWidget  *img_key_combo;
	GtkWidget  *img_key_entry;

	GtkWidget  *text_page_vbox;
	GtkWidget  *text_family_entry;
	GtkWidget  *text_family_combo;
	GtkWidget  *text_size_spin;
	GtkWidget  *text_bold_toggle;
	GtkWidget  *text_italic_toggle;
	GtkWidget  *text_color_combo;
	GtkWidget  *text_left_toggle;
	GtkWidget  *text_center_toggle;
	GtkWidget  *text_right_toggle;
	GtkWidget  *text_line_spacing_spin;

	GtkWidget  *edit_page_vbox;
	GtkWidget  *edit_text_view;
	GtkWidget  *edit_key_label;
	GtkWidget  *edit_key_entry;
	GtkWidget  *edit_key_combo;
	GtkWidget  *edit_insert_field_button;

	GtkWidget  *bc_page_vbox;
	GtkWidget  *bc_style_entry;
	GtkWidget  *bc_style_combo;
	GtkWidget  *bc_text_check;
	GtkWidget  *bc_cs_check;
	GtkWidget  *bc_color_combo;

	GtkWidget  *data_page_vbox;
	GtkWidget  *data_literal_radio;
	GtkWidget  *data_key_radio;
	GtkWidget  *data_text_entry;
	GtkWidget  *data_key_entry;
	GtkWidget  *data_key_combo;

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


void gl_object_editor_changed_cb                (glObjectEditor        *editor);
void gl_object_editor_size_changed_cb           (glObjectEditor        *editor);

void lsize_prefs_changed_cb                     (glObjectEditor        *editor);
void size_prefs_changed_cb                      (glObjectEditor        *editor);
void position_prefs_changed_cb                  (glObjectEditor        *editor);

G_END_DECLS

#endif
