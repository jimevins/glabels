/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  util.h:  various small utility functions
 *
 *  Copyright (C) 2001-2003  Jim Evins <evins@snaught.com>.
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

#ifndef __UTIL_H__
#define __UTIL_H__

#include <glib/glist.h>
#include <gtk/gtkcombobox.h>
#include <pango/pango.h>

G_BEGIN_DECLS

gchar              *gl_util_add_extension         (const gchar       *orig_filename);
gchar              *gl_util_remove_extension      (const gchar       *orig_filename);

gchar              *gl_util_make_absolute         (const gchar       *filename);

gchar              *gl_util_fraction              (gdouble            x);

const gchar        *gl_util_align_to_string       (PangoAlignment     align);
PangoAlignment      gl_util_string_to_align       (const gchar       *string);

const gchar        *gl_util_weight_to_string      (PangoWeight        weight);
PangoWeight         gl_util_string_to_weight      (const gchar       *string);

void gl_util_combo_box_set_strings     (GtkComboBox       *combo,
				        GList             *list);
void gl_util_combo_box_set_active_text (GtkComboBox       *combo,
					const gchar       *text);
void gl_util_combo_box_add_text_model  (GtkComboBox       *combo);

GList  *gl_util_get_font_family_list (void);
void    gl_util_font_family_list_free (GList *list);

G_END_DECLS

#endif /* __UTIL_H__ */
