/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  util.h:  various small utility functions
 *
 *  Copyright (C) 2001  Jim Evins <evins@snaught.com>.
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

#include <gtk/gtk.h>
#include <libgnomeprint/gnome-font.h>

extern gchar *gl_util_add_extension (const gchar * orig_filename);

extern gchar *gl_util_make_absolute (const gchar * filename);

extern gchar *gl_util_fraction( gdouble x );

extern GtkWidget* gl_util_button_new_with_stock_image (const gchar* text,
						       const gchar* stock_id);

extern GtkWidget* gl_util_dialog_add_button (GtkDialog *dialog, 
					     const gchar* text, 
					     const gchar* stock_id, 
					     gint response_id);

extern const gchar *gl_util_just_to_string (GtkJustification just);
extern GtkJustification gl_util_string_to_just (const gchar * string);

extern const gchar *gl_util_weight_to_string (GnomeFontWeight weight);
extern GnomeFontWeight gl_util_string_to_weight (const gchar * string);

#endif				/* __UTIL_H__ */
