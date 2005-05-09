/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/**
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  ui-commands.h:  GLabels UI commands module header file
 *
 *  Copyright (C) 2001-2005  Jim Evins <evins@snaught.com>.
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

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include <gtk/gtkaction.h>
#include <gtk/gtktoggleaction.h>
#include "window.h"

G_BEGIN_DECLS

void gl_ui_cmd_file_new                 (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_file_properties          (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_file_template_designer   (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_file_open                (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_file_open_recent         (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_file_save                (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_file_save_as             (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_file_print               (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_file_print_preview       (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_file_close               (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_file_quit                (GtkAction   *action,
					 glWindow    *window);



void gl_ui_cmd_edit_undo                (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_edit_redo                (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_edit_cut                 (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_edit_copy                (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_edit_paste               (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_edit_delete              (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_edit_select_all          (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_edit_unselect_all        (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_edit_preferences         (GtkAction   *action,
					 glWindow    *window);



void gl_ui_cmd_view_property_bar_toggle      (GtkToggleAction *action,
					      glWindow        *window);

void gl_ui_cmd_view_property_bar_tips_toggle (GtkToggleAction *action,
					      glWindow        *window);

void gl_ui_cmd_view_grid_toggle              (GtkToggleAction *action,
					      glWindow        *window);

void gl_ui_cmd_view_markup_toggle            (GtkToggleAction *action,
					      glWindow        *window);



void gl_ui_cmd_view_zoomin              (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_view_zoomout             (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_view_zoom1to1            (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_view_zoom_to_fit         (GtkAction   *action,
					 glWindow    *window);



void gl_ui_cmd_objects_arrow_mode       (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_create_text      (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_create_box       (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_create_line      (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_create_ellipse   (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_create_image     (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_create_barcode   (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_raise            (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_lower            (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_rotate_left      (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_rotate_right     (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_flip_horiz       (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_flip_vert        (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_align_left       (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_align_right      (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_align_hcenter    (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_align_top        (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_align_bottom     (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_align_vcenter    (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_center_horiz     (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_center_vert      (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_objects_merge_properties (GtkAction   *action,
					 glWindow    *window);



void gl_ui_cmd_help_contents            (GtkAction   *action,
					 glWindow    *window);

void gl_ui_cmd_help_about 	        (GtkAction   *action,
					 glWindow    *window);

G_END_DECLS

#endif /* __COMMANDS_H__ */ 
