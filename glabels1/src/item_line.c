/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  item_line.c:  GLabels Line Object (canvas item) module
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

#include <config.h>

#include <string.h>
#include <math.h>

#include "display.h"
#include "item_line.h"

#include "highlight.h"

#include "mdi.h"
#include "propertybox.h"
#include "prop_line.h"
#include "prop_position.h"
#include "prop_vector.h"

#include "pixmaps/cursor_line.xbm"
#include "pixmaps/cursor_line_mask.xbm"

#include "debug.h"

/*===========================================*/
/* Private data types                        */
/*===========================================*/

typedef struct {
	GnomeCanvasItem *item;

	/* Page 0 widgets */
	GtkWidget *line;

	/* Page 1 widgets */
	GtkWidget *position;
	GtkWidget *vector;
} PropertyDialogPassback;

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void item_line_update (GnomeCanvasItem * item);

static void changed_cb (glPropertyBox * propertybox);
static void apply_cb (glPropertyBox * propertybox,
		      gint page, PropertyDialogPassback * data);

/****************************************************************************/
/* Create a line object                                                     */
/****************************************************************************/
GnomeCanvasItem *
gl_item_line_new (glLabelObject * object,
		  glDisplay * display)
{
	GnomeCanvasGroup *group;
	GnomeCanvasItem *item;
	GnomeCanvasPoints *points;

	points = gnome_canvas_points_new (2);
	points->coords[0] = object->x;
	points->coords[1] = object->y;
	points->coords[2] = object->x + object->arg.line.dx;
	points->coords[3] = object->y + object->arg.line.dy;

	group = gnome_canvas_root (GNOME_CANVAS (display->canvas));
	item = gnome_canvas_item_new (group, gnome_canvas_line_get_type (),
				      "points", points,
				      "width_units",
				      object->arg.line.line_width,
				      "fill_color_rgba",
				      object->arg.line.line_color, NULL);

	gnome_canvas_points_free (points);

	/* Squirrel away pointers to object and display in the canvas item. */
	gtk_object_set_data (GTK_OBJECT (item), "label_object", object);
	gtk_object_set_data (GTK_OBJECT (item), "display", display);
	gtk_object_set_data (GTK_OBJECT (item), "highlight", NULL);

	gtk_signal_connect (GTK_OBJECT (item), "event",
			    GTK_SIGNAL_FUNC (gl_display_item_event_handler),
			    display);

	gl_display_new_item_menu (item);

	gl_display_set_modified (display);

	return item;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE. Update a line object                                            */
/*--------------------------------------------------------------------------*/
static void
item_line_update (GnomeCanvasItem * item)
{
	glLabelObject *object;
	glDisplay *display;
	GnomeCanvasPoints *points;
	gdouble affine[6];

	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");

	points = gnome_canvas_points_new (2);
	points->coords[0] = object->x;
	points->coords[1] = object->y;
	points->coords[2] = object->x + object->arg.line.dx;
	points->coords[3] = object->y + object->arg.line.dy;

	art_affine_identity (affine);
	gnome_canvas_item_affine_absolute (item, affine);
	gnome_canvas_item_set (item,
			       "points", points,
			       "width_units", object->arg.line.line_width,
			       "fill_color_rgba", object->arg.line.line_color,
			       NULL);

	gnome_canvas_points_free (points);

	display = gtk_object_get_data (GTK_OBJECT (item), "display");
	gl_display_set_modified (display);
}

/****************************************************************************/
/* Create a selection canvas item/group for given item                      */
/****************************************************************************/
void
gl_item_line_highlight (GnomeCanvasItem * item)
{
	gl_highlight (item, GL_HIGHLIGHT_LINE_RESIZABLE);
}

/****************************************************************************/
/* Get position and size of object.                                         */
/****************************************************************************/
void
gl_item_line_get_position_size (GnomeCanvasItem * item,
				gdouble * x,
				gdouble * y,
				gdouble * w,
				gdouble * h)
{
	glLabelObject *object;

	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");

	*x = object->x;
	*y = object->y;

	*w = object->arg.line.dx;
	*h = object->arg.line.dy;
}

/****************************************************************************/
/* Get position and size of object.                                         */
/****************************************************************************/
void
gl_item_line_set_position_size (GnomeCanvasItem * item,
				gdouble x,
				gdouble y,
				gdouble w,
				gdouble h)
{
	glLabelObject *object;

	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");

	object->x = x;
	object->y = y;

	object->arg.line.dx = w;
	object->arg.line.dy = h;

	item_line_update (item);
}

/****************************************************************************/
/* Get line item bounds.                                                    */
/****************************************************************************/
void
gl_item_line_get_bounds (GnomeCanvasItem * item,
			 gdouble * x1,
			 gdouble * y1,
			 gdouble * x2,
			 gdouble * y2)
{
	gnome_canvas_item_get_bounds (item, x1, y1, x2, y2);
}

/****************************************************************************/
/* Create and run an edit dialog on a line object.                          */
/****************************************************************************/
void
gl_item_line_edit_dialog (GnomeCanvasItem * item)
{
	GtkWidget *dialog;
	static PropertyDialogPassback *data = NULL;
	glLabelObject *object;
	glDisplay *display;
	GtkWidget *wvbox;

	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");
	display = gtk_object_get_data (GTK_OBJECT (item), "display");

	/*-----------------------------------------------------------------*/
	/* Build dialog with notebook.                                     */
	/*-----------------------------------------------------------------*/
	if (data == NULL) {
		data = g_new0 (PropertyDialogPassback, 1);
	}
	data->item = item;

	dialog = gl_property_box_new ();
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (dialog),
				      GTK_WINDOW (gl_mdi_get_active_window ()));
	gtk_window_set_title (GTK_WINDOW (dialog),
			      _("Edit line object properties"));
	gtk_signal_connect (GTK_OBJECT (dialog), "apply",
			    GTK_SIGNAL_FUNC (apply_cb), data);

	/*---------------------------*/
	/* Appearance Notebook Tab   */
	/*---------------------------*/
	wvbox = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox), 10);
	gl_property_box_append_page (GL_PROPERTY_BOX (dialog), wvbox,
				     gtk_label_new (_("Appearance")));

	/* ------ Begin Line box ------ */
	data->line = gl_prop_line_new (_("Line"));
	gl_prop_line_set_params (GL_PROP_LINE (data->line),
				 object->arg.line.line_width,
				 object->arg.line.line_color);
	gtk_box_pack_start (GTK_BOX (wvbox), data->line, FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (data->line), "changed",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (dialog));

	/*----------------------------*/
	/* Position/Size Notebook Tab */
	/*----------------------------*/
	wvbox = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox), 10);
	gl_property_box_append_page (GL_PROPERTY_BOX (dialog), wvbox,
				     gtk_label_new (_("Position/Size")));

	/* ------ Position Frame ------ */
	data->position = gl_prop_position_new (_("Position"));
	gl_prop_position_set_params (GL_PROP_POSITION (data->position),
				     object->x, object->y,
				     display->label->width,
				     display->label->height);
	gtk_box_pack_start (GTK_BOX (wvbox), data->position, FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (data->position), "changed",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (dialog));

	/* ------ Size Frame ------ */
	data->vector = gl_prop_vector_new (_("Size"));
	gl_prop_vector_set_params (GL_PROP_VECTOR (data->vector),
				   object->arg.line.dx, object->arg.line.dy,
				   display->label->width,
				   display->label->height);
	gtk_box_pack_start (GTK_BOX (wvbox), data->vector, FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (data->vector), "changed",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (dialog));

	/*-----------------------------------------------------------------*/
	/* Run dialog, and update state of object if "Applied."            */
	/*-----------------------------------------------------------------*/
	gtk_widget_show_all (GTK_WIDGET (dialog));

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the dialog has changed.        */
/*---------------------------------------------------------------------------*/
static void
changed_cb (glPropertyBox * propertybox)
{
	gl_property_box_changed (propertybox);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Apply" callback.                                               */
/*---------------------------------------------------------------------------*/
static void
apply_cb (glPropertyBox * propertybox,
	  gint page,
	  PropertyDialogPassback * data)
{
	glLabelObject *object;
	glDisplay *display;

	object = gtk_object_get_data (GTK_OBJECT (data->item), "label_object");
	display = gtk_object_get_data (GTK_OBJECT (data->item), "display");

	switch (page) {

	case 0:
		/* ------- Get updated line width & color ------ */
		gl_prop_line_get_params (GL_PROP_LINE (data->line),
					 &object->arg.line.line_width,
					 &object->arg.line.line_color);
		break;

	case 1:
		/* ------ get updated position ------ */
		gl_prop_position_get_position (GL_PROP_POSITION
					       (data->position), &object->x,
					       &object->y);
		/* ------ get updated size ------ */
		gl_prop_vector_get_params (GL_PROP_VECTOR (data->vector),
					   &object->arg.line.dx,
					   &object->arg.line.dy);
		break;

	default:
		return;
	}

	/* ------ Udate state of object ------ */
	item_line_update (data->item);
	gl_display_select_item (display, data->item);

}

/****************************************************************************/
/* Return apropos cursor for create object mode.                            */
/****************************************************************************/
GdkCursor *
gl_item_line_get_create_cursor (void)
{
	static GdkCursor *cursor = NULL;
	GdkPixmap *pixmap_data, *pixmap_mask;
	GdkColor fg = { 0, 0, 0, 0 };
	GdkColor bg = { 0, 65535, 65535, 65535 };

	if (!cursor) {
		pixmap_data = gdk_bitmap_create_from_data (NULL,
							   cursor_line_bits,
							   cursor_line_width,
							   cursor_line_height);
		pixmap_mask = gdk_bitmap_create_from_data (NULL,
							   cursor_line_mask_bits,
							   cursor_line_mask_width,
							   cursor_line_mask_height);
		cursor =
		    gdk_cursor_new_from_pixmap (pixmap_data, pixmap_mask, &fg,
						&bg, cursor_line_x_hot,
						cursor_line_y_hot);
	}

	return cursor;
}

/****************************************************************************/
/* Canvas event handler (line mode)                                         */
/****************************************************************************/
gint
gl_item_line_create_event_handler (GnomeCanvas * canvas,
				   GdkEvent * event,
				   gpointer data)
{
	static gdouble x0, y0;
	static gboolean dragging = FALSE;
	static GnomeCanvasItem *item;
	static glLabelObject *object;
	glDisplay *display = GL_DISPLAY (data);
	gdouble x, y;

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			dragging = TRUE;
			gdk_pointer_grab (GTK_WIDGET (display->canvas)->window,
					  FALSE,
					  GDK_POINTER_MOTION_MASK |
					  GDK_BUTTON_RELEASE_MASK |
					  GDK_BUTTON_PRESS_MASK,
					  NULL, NULL, event->button.time);
			gnome_canvas_window_to_world (canvas,
						      event->button.x,
						      event->button.y, &x, &y);
			object =
			    gl_label_object_new (display->label,
						 GL_LABEL_OBJECT_LINE);
			object->x = x;
			object->y = y;
			object->arg.line.dx = 0.0;
			object->arg.line.dy = 0.0;
			object->arg.line.line_width = 1.0;
			object->arg.line.line_color =
			    GNOME_CANVAS_COLOR_A (0, 0, 255, 128);
			item = gl_item_line_new (object, display);
			x0 = x;
			y0 = y;
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			gdk_pointer_ungrab (event->button.time);
			gnome_canvas_window_to_world (canvas,
						      event->button.x,
						      event->button.y, &x, &y);
			if ((x0 == x) && (y0 == y)) {
				x = x0 + 36.0;
				y = y0 + 36.0;
			}
			object->x = x0;
			object->y = y0;
			object->arg.line.dx = (x - x0);
			object->arg.line.dy = (y - y0);
			object->arg.line.line_color =
			    GNOME_CANVAS_COLOR (0, 0, 0);
			item_line_update (item);
			gl_display_add_item (display, item);
			gl_display_unselect_all (display);
			gl_display_select_item (display, item);
			gl_display_arrow_mode (display);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			gnome_canvas_window_to_world (canvas,
						      event->motion.x,
						      event->motion.y, &x, &y);
			object->x = x0;
			object->y = y0;
			object->arg.line.dx = (x - x0);
			object->arg.line.dy = (y - y0);
			item_line_update (item);
			return TRUE;
		} else {
			return FALSE;
		}

	default:
		return FALSE;
	}

}
