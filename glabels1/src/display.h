/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  display.h:  GLabels Display module header file
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

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <gtk/gtk.h>
#include "label.h"

typedef enum {
	GL_DISPLAY_STATE_ARROW,
	GL_DISPLAY_STATE_OBJECT_CREATE
} glDisplayState;

#define GL_TYPE_DISPLAY (gl_display_get_type ())
#define GL_DISPLAY(obj) (GTK_CHECK_CAST((obj), GL_TYPE_DISPLAY, glDisplay ))
#define GL_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GL_TYPE_DISPLAY, glDisplayClass))
#define GL_IS_DISPLAY(obj) (GTK_CHECK_TYPE ((obj), GL_TYPE_DISPLAY))
#define GL_IS_DISPLAY_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GL_TYPE_DISPLAY))

typedef struct _glDisplay glDisplay;
typedef struct _glDisplayClass glDisplayClass;

struct _glDisplay {
	GtkVBox parent_widget;

	glLabel *label;

	GtkWidget *canvas;
	gdouble scale;
	gint n_bg_items;
	GList *bg_item_list;

	glDisplayState state;
	glLabelObjectType create_type;

	gboolean modified;

	GList *item_list;
	GList *selected_item_list;

	gint have_selection;
	glLabel *selection_data;
	GtkWidget *invisible;

	GtkWidget *menu;
};

struct _glDisplayClass {
	GtkVBoxClass parent_class;
};

extern guint gl_display_get_type (void);

extern GtkWidget *gl_display_new (glLabel * label);

extern void gl_display_arrow_mode (glDisplay * display);
extern void gl_display_object_create_mode (glDisplay * display,
					   glLabelObjectType type);

extern void gl_display_add_item (glDisplay * display,
				 GnomeCanvasItem * item);

extern void gl_display_select_all (glDisplay * display);
extern void gl_display_select_region (glDisplay * display,
				      gdouble x1,
				      gdouble y1,
				      gdouble x2,
				      gdouble y2);
extern void gl_display_unselect_all (glDisplay * display);

extern void gl_display_select_item (glDisplay * display,
				    GnomeCanvasItem * item);
extern void gl_display_unselect_item (glDisplay * display,
				      GnomeCanvasItem * item);

extern int gl_display_item_event_handler (GnomeCanvasItem * item,
					  GdkEvent * event,
					  gpointer data);

extern GtkWidget *gl_display_new_selection_menu (glDisplay * display);
extern GtkWidget *gl_display_new_item_menu (GnomeCanvasItem * item);

extern gboolean gl_display_modified (glDisplay * display);
extern void gl_display_set_modified (glDisplay * display);
extern void gl_display_clear_modified (glDisplay * display);

extern void gl_display_cut (glDisplay * display);
extern void gl_display_copy (glDisplay * display);
extern void gl_display_paste (glDisplay * display);

extern void gl_display_zoom_in (glDisplay * display);
extern void gl_display_zoom_out (glDisplay * display);
extern void gl_display_set_zoom (glDisplay * display, gdouble scale);
extern gdouble gl_display_get_zoom (glDisplay * display);

#endif
