/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  edit.h:  Edit module header file
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
#ifndef __EDIT_H__
#define __EDIT_H__

#include <gtk/gtk.h>

extern void gl_edit_add_cb (GtkWidget * widget, gpointer data);

extern void gl_edit_delete_cb (GtkWidget * widget, gpointer data);

extern void gl_edit_title_cb (GtkWidget * widget, gpointer data);

extern void gl_edit_select_all_cb (GtkWidget * widget, gpointer data);

extern void gl_edit_unselect_cb (GtkWidget * widget, gpointer data);

extern void gl_edit_cut_cb (GtkWidget * widget, gpointer data);

extern void gl_edit_copy_cb (GtkWidget * widget, gpointer data);

extern void gl_edit_paste_cb (GtkWidget * widget, gpointer data);

#endif				/* __EDIT_H__ */
