/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  tools.h:  toolbar module header file
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
#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <gnome.h>

extern void gl_tools_arrow_cb (GtkWidget * widget, gpointer data);
extern void gl_tools_text_cb (GtkWidget * widget, gpointer data);
extern void gl_tools_box_cb (GtkWidget * widget, gpointer data);
extern void gl_tools_line_cb (GtkWidget * widget, gpointer data);
extern void gl_tools_ellipse_cb (GtkWidget * widget, gpointer data);
extern void gl_tools_image_cb (GtkWidget * widget, gpointer data);
extern void gl_tools_barcode_cb (GtkWidget * widget, gpointer data);
extern void gl_tools_zoomin_cb (GtkWidget * widget, gpointer data);
extern void gl_tools_zoomout_cb (GtkWidget * widget, gpointer data);
extern void gl_tools_zoom1to1_cb (GtkWidget * widget, gpointer data);
extern void gl_tools_merge_properties_cb (GtkWidget * widget, gpointer data);

#endif
