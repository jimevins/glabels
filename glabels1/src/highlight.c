/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  highlight.c:  GLabels Resizable Highlight module
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

#include "highlight.h"
#include "item.h"

#include "math.h"

#include "debug.h"

#define MIN_ITEM_SIZE 1.0

/*===========================================*/
/* Private data types                        */
/*===========================================*/

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void highlight_resizable_box (GnomeCanvasItem * item,
				     glHighlightStyle style);
static void highlight_resizable_line (GnomeCanvasItem * item);
static void highlight_simple (GnomeCanvasItem * item);

static int tl_resize_event_handler (GnomeCanvasItem * handle_item,
				    GdkEvent * event, gpointer data);
static int tr_resize_event_handler (GnomeCanvasItem * handle_item,
				    GdkEvent * event, gpointer data);
static int bl_resize_event_handler (GnomeCanvasItem * handle_item,
				    GdkEvent * event, gpointer data);
static int br_resize_event_handler (GnomeCanvasItem * handle_item,
				    GdkEvent * event, gpointer data);

static int sl_resize_event_handler (GnomeCanvasItem * handle_item,
				    GdkEvent * event, gpointer data);
static int sr_resize_event_handler (GnomeCanvasItem * handle_item,
				    GdkEvent * event, gpointer data);
static int st_resize_event_handler (GnomeCanvasItem * handle_item,
				    GdkEvent * event, gpointer data);
static int sb_resize_event_handler (GnomeCanvasItem * handle_item,
				    GdkEvent * event, gpointer data);

static int p1_resize_event_handler (GnomeCanvasItem * handle_item,
				    GdkEvent * event, gpointer data);
static int p2_resize_event_handler (GnomeCanvasItem * handle_item,
				    GdkEvent * event, gpointer data);

static int passthrough_event_handler (GnomeCanvasItem * handle_item,
				      GdkEvent * event, gpointer data);

/*****************************************************************************/
/* Create/Update & Display a resizable highlight for given item.             */
/*****************************************************************************/
void
gl_highlight (GnomeCanvasItem * item,
	      glHighlightStyle style)
{
	g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));

	switch (style) {

	case GL_HIGHLIGHT_BOX_RESIZABLE:
	case GL_HIGHLIGHT_ELLIPSE_RESIZABLE:
		highlight_resizable_box (item, style);
		break;

	case GL_HIGHLIGHT_LINE_RESIZABLE:
		highlight_resizable_line (item);
		break;

	case GL_HIGHLIGHT_SIMPLE:
		highlight_simple (item);
		break;

	default:
		WARN ("Invalid resizable highlight style");

	}
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create/Update & Display a resizable box highlight.              */
/*---------------------------------------------------------------------------*/
static void
highlight_resizable_box (GnomeCanvasItem * item,
			 glHighlightStyle style)
{
	GnomeCanvasItem *highlight;
	gdouble x, y, w, h;
	GnomeCanvasItem *outline;	/* Outline around item */
	GnomeCanvasItem *tl, *tr, *bl, *br;	/* Handles at four corners */
	GnomeCanvasItem *sl, *sr, *st, *sb;	/* Handles at each side */

	g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));

	highlight = gtk_object_get_data (GTK_OBJECT (item), "highlight");

	gl_item_get_position_size (item, &x, &y, &w, &h);

	if (highlight == NULL) {

		highlight =
		    gnome_canvas_item_new (gnome_canvas_root
					   (GNOME_CANVAS (item->canvas)),
					   gnome_canvas_group_get_type (), "x",
					   x, "y", y, NULL);

		switch (style) {
		case GL_HIGHLIGHT_BOX_RESIZABLE:
			outline =
			    gnome_canvas_item_new (GNOME_CANVAS_GROUP
						   (highlight),
						   gnome_canvas_rect_get_type
						   (),
						   "x1", -0.5,
						   "y1", -0.5,
						   "x2", w + 0.5,
						   "y2", h + 0.5,
						   "width_pixels", 2,
						   "outline_color_rgba",
						   GNOME_CANVAS_COLOR_A (0, 0,
									 255,
									 255),
						   NULL);
			break;
		case GL_HIGHLIGHT_ELLIPSE_RESIZABLE:
			outline =
			    gnome_canvas_item_new (GNOME_CANVAS_GROUP
						   (highlight),
						   gnome_canvas_ellipse_get_type
						   (),
						   "x1", -0.5,
						   "y1", -0.5,
						   "x2", w + 0.5,
						   "y2", h + 0.5,
						   "width_pixels", 2,
						   "outline_color_rgba",
						   GNOME_CANVAS_COLOR_A (0, 0,
									 255,
									 255),
						   NULL);
			break;
		default:
			outline = NULL;
			WARN ("Invalid resizable highlight style");
		}

		tl = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
					    gnome_canvas_rect_get_type (),
					    "x1", -1.0,
					    "y1", -1.0,
					    "x2", +1.0,
					    "y2", +1.0,
					    "fill_color_rgba",
					    GNOME_CANVAS_COLOR_A (0, 0, 255,
								  255),
					    "outline_color_rgba",
					    GNOME_CANVAS_COLOR_A (255, 255, 255,
								  255), NULL);

		tr = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
					    gnome_canvas_rect_get_type (),
					    "x1", w - 1.0,
					    "y1", -1.0,
					    "x2", w + 1.0,
					    "y2", +1.0,
					    "fill_color_rgba",
					    GNOME_CANVAS_COLOR_A (0, 0, 255,
								  255),
					    "outline_color_rgba",
					    GNOME_CANVAS_COLOR_A (255, 255, 255,
								  255), NULL);

		bl = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
					    gnome_canvas_rect_get_type (),
					    "x1", -1.0,
					    "y1", h - 1.0,
					    "x2", +1.0,
					    "y2", h + 1.0,
					    "fill_color_rgba",
					    GNOME_CANVAS_COLOR_A (0, 0, 255,
								  255),
					    "outline_color_rgba",
					    GNOME_CANVAS_COLOR_A (255, 255, 255,
								  255), NULL);

		br = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
					    gnome_canvas_rect_get_type (),
					    "x1", w - 1.0,
					    "y1", h - 1.0,
					    "x2", w + 1.0,
					    "y2", h + 1.0,
					    "fill_color_rgba",
					    GNOME_CANVAS_COLOR_A (0, 0, 255,
								  255),
					    "outline_color_rgba",
					    GNOME_CANVAS_COLOR_A (255, 255, 255,
								  255), NULL);

		sl = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
					    gnome_canvas_rect_get_type (),
					    "x1", -1.0,
					    "y1", h / 2.0 - 1.0,
					    "x2", +1.0,
					    "y2", h / 2.0 + 1.0,
					    "fill_color_rgba",
					    GNOME_CANVAS_COLOR_A (0, 0, 255,
								  255),
					    "outline_color_rgba",
					    GNOME_CANVAS_COLOR_A (255, 255, 255,
								  255), NULL);

		sr = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
					    gnome_canvas_rect_get_type (),
					    "x1", w - 1.0,
					    "y1", h / 2.0 - 1.0,
					    "x2", w + 1.0,
					    "y2", h / 2.0 + 1.0,
					    "fill_color_rgba",
					    GNOME_CANVAS_COLOR_A (0, 0, 255,
								  255),
					    "outline_color_rgba",
					    GNOME_CANVAS_COLOR_A (255, 255, 255,
								  255), NULL);

		st = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
					    gnome_canvas_rect_get_type (),
					    "x1", w / 2.0 - 1.0,
					    "y1", -1.0,
					    "x2", w / 2.0 + 1.0,
					    "y2", +1.0,
					    "fill_color_rgba",
					    GNOME_CANVAS_COLOR_A (0, 0, 255,
								  255),
					    "outline_color_rgba",
					    GNOME_CANVAS_COLOR_A (255, 255, 255,
								  255), NULL);

		sb = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
					    gnome_canvas_rect_get_type (),
					    "x1", w / 2.0 - 1.0,
					    "y1", h - 1.0,
					    "x2", w / 2.0 + 1.0,
					    "y2", h + 1.0,
					    "fill_color_rgba",
					    GNOME_CANVAS_COLOR_A (0, 0, 255,
								  255),
					    "outline_color_rgba",
					    GNOME_CANVAS_COLOR_A (255, 255, 255,
								  255), NULL);

		gtk_object_set_data (GTK_OBJECT (item), "highlight", highlight);

		gtk_object_set_data (GTK_OBJECT (highlight), "outline",
				     outline);

		gtk_object_set_data (GTK_OBJECT (highlight), "tl", tl);
		gtk_object_set_data (GTK_OBJECT (highlight), "tr", tr);
		gtk_object_set_data (GTK_OBJECT (highlight), "bl", bl);
		gtk_object_set_data (GTK_OBJECT (highlight), "br", br);
		gtk_object_set_data (GTK_OBJECT (highlight), "sl", sl);
		gtk_object_set_data (GTK_OBJECT (highlight), "sr", sr);
		gtk_object_set_data (GTK_OBJECT (highlight), "st", st);
		gtk_object_set_data (GTK_OBJECT (highlight), "sb", sb);

		gtk_signal_connect (GTK_OBJECT (tl), "event",
				    GTK_SIGNAL_FUNC (tl_resize_event_handler),
				    item);
		gtk_signal_connect (GTK_OBJECT (tr), "event",
				    GTK_SIGNAL_FUNC (tr_resize_event_handler),
				    item);
		gtk_signal_connect (GTK_OBJECT (bl), "event",
				    GTK_SIGNAL_FUNC (bl_resize_event_handler),
				    item);
		gtk_signal_connect (GTK_OBJECT (br), "event",
				    GTK_SIGNAL_FUNC (br_resize_event_handler),
				    item);
		gtk_signal_connect (GTK_OBJECT (sl), "event",
				    GTK_SIGNAL_FUNC (sl_resize_event_handler),
				    item);
		gtk_signal_connect (GTK_OBJECT (sr), "event",
				    GTK_SIGNAL_FUNC (sr_resize_event_handler),
				    item);
		gtk_signal_connect (GTK_OBJECT (st), "event",
				    GTK_SIGNAL_FUNC (st_resize_event_handler),
				    item);
		gtk_signal_connect (GTK_OBJECT (sb), "event",
				    GTK_SIGNAL_FUNC (sb_resize_event_handler),
				    item);

		gtk_signal_connect (GTK_OBJECT (outline), "event",
				    GTK_SIGNAL_FUNC (passthrough_event_handler),
				    item);

	} else {

		gnome_canvas_item_set (highlight, "x", x, "y", y, NULL);

		outline =
		    gtk_object_get_data (GTK_OBJECT (highlight), "outline");

		tl = gtk_object_get_data (GTK_OBJECT (highlight), "tl");
		tr = gtk_object_get_data (GTK_OBJECT (highlight), "tr");
		bl = gtk_object_get_data (GTK_OBJECT (highlight), "bl");
		br = gtk_object_get_data (GTK_OBJECT (highlight), "br");
		sl = gtk_object_get_data (GTK_OBJECT (highlight), "sl");
		sr = gtk_object_get_data (GTK_OBJECT (highlight), "sr");
		st = gtk_object_get_data (GTK_OBJECT (highlight), "st");
		sb = gtk_object_get_data (GTK_OBJECT (highlight), "sb");

		gnome_canvas_item_set (outline,
				       "x1", -0.5,
				       "y1", -0.5,
				       "x2", w + 0.5, "y2", h + 0.5, NULL);

		gnome_canvas_item_set (tl,
				       "x1", -1.0,
				       "y1", -1.0,
				       "x2", +1.0, "y2", +1.0, NULL);

		gnome_canvas_item_set (tr,
				       "x1", w - 1.0,
				       "y1", -1.0,
				       "x2", w + 1.0, "y2", +1.0, NULL);

		gnome_canvas_item_set (bl,
				       "x1", -1.0,
				       "y1", h - 1.0,
				       "x2", +1.0, "y2", h + 1.0, NULL);

		gnome_canvas_item_set (br,
				       "x1", w - 1.0,
				       "y1", h - 1.0,
				       "x2", w + 1.0, "y2", h + 1.0, NULL);

		gnome_canvas_item_set (sl,
				       "x1", -1.0,
				       "y1", h / 2.0 - 1.0,
				       "x2", +1.0, "y2", h / 2.0 + 1.0, NULL);

		gnome_canvas_item_set (sr,
				       "x1", w - 1.0,
				       "y1", h / 2.0 - 1.0,
				       "x2", w + 1.0,
				       "y2", h / 2.0 + 1.0, NULL);

		gnome_canvas_item_set (st,
				       "x1", w / 2.0 - 1.0,
				       "y1", -1.0,
				       "x2", w / 2.0 + 1.0, "y2", +1.0, NULL);

		gnome_canvas_item_set (sb,
				       "x1", w / 2.0 - 1.0,
				       "y1", h - 1.0,
				       "x2", w / 2.0 + 1.0,
				       "y2", h + 1.0, NULL);

		gnome_canvas_item_show (highlight);

	}

	gnome_canvas_item_raise_to_top (highlight);

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create/Update & Display a resizable line highlight.             */
/*---------------------------------------------------------------------------*/
static void
highlight_resizable_line (GnomeCanvasItem * item)
{
	GnomeCanvasItem *highlight;
	gdouble x, y, dx, dy;
	GnomeCanvasPoints *points;
	GnomeCanvasItem *outline;	/* Outline around item */
	GnomeCanvasItem *p1, *p2;	/* Handles at endpoints */

	g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));

	highlight = gtk_object_get_data (GTK_OBJECT (item), "highlight");

	gl_item_get_position_size (item, &x, &y, &dx, &dy);

	points = gnome_canvas_points_new (2);

	if (highlight == NULL) {

		highlight =
		    gnome_canvas_item_new (gnome_canvas_root
					   (GNOME_CANVAS (item->canvas)),
					   gnome_canvas_group_get_type (), "x",
					   x, "y", y, NULL);

		points->coords[0] = 0.0;
		points->coords[1] = 0.0;
		points->coords[2] = dx;
		points->coords[3] = dy;
		outline =
		    gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
					   gnome_canvas_line_get_type (),
					   "points", points,
					   "width_pixels", 3,
					   "fill_color_rgba",
					   GNOME_CANVAS_COLOR_A (0, 0, 255,
								 255), NULL);

		p1 = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
					    gnome_canvas_rect_get_type (),
					    "x1", -1.0,
					    "y1", -1.0,
					    "x2", +1.0,
					    "y2", +1.0,
					    "fill_color_rgba",
					    GNOME_CANVAS_COLOR_A (0, 0, 255,
								  255),
					    "outline_color_rgba",
					    GNOME_CANVAS_COLOR_A (255, 255, 255,
								  255), NULL);

		p2 = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
					    gnome_canvas_rect_get_type (),
					    "x1", dx - 1.0,
					    "y1", dy - 1.0,
					    "x2", dx + 1.0,
					    "y2", dy + 1.0,
					    "fill_color_rgba",
					    GNOME_CANVAS_COLOR_A (0, 0, 255,
								  255),
					    "outline_color_rgba",
					    GNOME_CANVAS_COLOR_A (255, 255, 255,
								  255), NULL);

		gtk_object_set_data (GTK_OBJECT (item), "highlight", highlight);

		gtk_object_set_data (GTK_OBJECT (highlight), "outline",
				     outline);

		gtk_object_set_data (GTK_OBJECT (highlight), "p1", p1);
		gtk_object_set_data (GTK_OBJECT (highlight), "p2", p2);

		gtk_signal_connect (GTK_OBJECT (outline), "event",
				    GTK_SIGNAL_FUNC (passthrough_event_handler),
				    item);

		gtk_signal_connect (GTK_OBJECT (p1), "event",
				    GTK_SIGNAL_FUNC (p1_resize_event_handler),
				    item);
		gtk_signal_connect (GTK_OBJECT (p2), "event",
				    GTK_SIGNAL_FUNC (p2_resize_event_handler),
				    item);

	} else {

		gnome_canvas_item_set (highlight, "x", x, "y", y, NULL);

		outline =
		    gtk_object_get_data (GTK_OBJECT (highlight), "outline");

		p1 = gtk_object_get_data (GTK_OBJECT (highlight), "p1");
		p2 = gtk_object_get_data (GTK_OBJECT (highlight), "p2");

		points->coords[0] = 0.0;
		points->coords[1] = 0.0;
		points->coords[2] = dx;
		points->coords[3] = dy;
		gnome_canvas_item_set (outline, "points", points, NULL);

		gnome_canvas_item_set (p1,
				       "x1", -1.0,
				       "y1", -1.0,
				       "x2", +1.0, "y2", +1.0, NULL);

		gnome_canvas_item_set (p2,
				       "x1", dx - 1.0,
				       "y1", dy - 1.0,
				       "x2", dx + 1.0, "y2", dy + 1.0, NULL);

		gnome_canvas_item_show (highlight);

	}

	gnome_canvas_points_free (points);

	gnome_canvas_item_raise_to_top (highlight);

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create/Update & Display a simple box highlight.                 */
/*---------------------------------------------------------------------------*/
static void
highlight_simple (GnomeCanvasItem * item)
{
	GnomeCanvasItem *highlight;
	gdouble x1, y1, x2, y2;

	g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));

	highlight = gtk_object_get_data (GTK_OBJECT (item), "highlight");

	gl_item_get_bounds (item, &x1, &y1, &x2, &y2);

	if (highlight == NULL) {

		highlight =
		    gnome_canvas_item_new (gnome_canvas_root
					   (GNOME_CANVAS (item->canvas)),
					   gnome_canvas_rect_get_type (), "x1",
					   x1 - 0.5, "y1", y1 - 0.5, "x2",
					   x2 + 0.5, "y2", y2 + 0.5,
					   "width_pixels", 2,
					   "outline_color_rgba",
					   GNOME_CANVAS_COLOR_A (0, 0, 255,
								 255), NULL);

		gtk_object_set_data (GTK_OBJECT (item), "highlight", highlight);

		gtk_signal_connect (GTK_OBJECT (highlight), "event",
				    GTK_SIGNAL_FUNC (passthrough_event_handler),
				    item);

	} else {

		gnome_canvas_item_set (highlight,
				       "x1", x1 - 0.5,
				       "y1", y1 - 0.5,
				       "x2", x2 + 0.5,
				       "y2", y2 + 0.5,
				       "width_pixels", 2,
				       "outline_color_rgba",
				       GNOME_CANVAS_COLOR_A (0, 0, 255, 255),
				       NULL);

		gnome_canvas_item_show (highlight);

	}

	gnome_canvas_item_raise_to_top (highlight);

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Top-left" Resize event handler.                    */
/*---------------------------------------------------------------------------*/
static int
tl_resize_event_handler (GnomeCanvasItem * handle_item,
			 GdkEvent * event,
			 gpointer data)
{
	gdouble x, y, w, h;
	static gdouble x2, y2;
	static gboolean dragging = FALSE;
	static GnomeCanvasItem *item;
	static glDisplay *display;
	GdkCursor *cursor;

	item = GNOME_CANVAS_ITEM (data);
	gl_item_get_position_size (item, &x, &y, &w, &h);
	display = gl_item_get_display (item);

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			dragging = TRUE;
			gnome_canvas_item_grab (handle_item,
						GDK_POINTER_MOTION_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_BUTTON_PRESS_MASK,
						NULL, event->button.time);
			/* Anchor to bottom-right corner */
			x2 = x + w;
			y2 = y + h;
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			gnome_canvas_item_ungrab (handle_item,
						  event->button.time);
			x = MIN (event->button.x, x2 - MIN_ITEM_SIZE);
			y = MIN (event->button.y, y2 - MIN_ITEM_SIZE);
			w = MAX (x2 - event->button.x, MIN_ITEM_SIZE);
			h = MAX (y2 - event->button.y, MIN_ITEM_SIZE);
			gl_item_set_position_size (item, x, y, w, h);
			gl_display_select_item (display, item);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			x = MIN (event->button.x, x2 - MIN_ITEM_SIZE);
			y = MIN (event->button.y, y2 - MIN_ITEM_SIZE);
			w = MAX (x2 - event->button.x, MIN_ITEM_SIZE);
			h = MAX (y2 - event->button.y, MIN_ITEM_SIZE);
			gl_item_set_position_size (item, x, y, w, h);
			gl_display_select_item (display, item);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	default:
		return FALSE;
	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Top-right" Resize event handler.                               */
/*---------------------------------------------------------------------------*/
static int
tr_resize_event_handler (GnomeCanvasItem * handle_item,
			 GdkEvent * event,
			 gpointer data)
{
	gdouble x, y, w, h;
	static gdouble x1, y2;
	static gboolean dragging = FALSE;
	static GnomeCanvasItem *item;
	static glDisplay *display;
	GdkCursor *cursor;

	item = GNOME_CANVAS_ITEM (data);
	gl_item_get_position_size (item, &x, &y, &w, &h);
	display = gl_item_get_display (item);

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			dragging = TRUE;
			gnome_canvas_item_grab (handle_item,
						GDK_POINTER_MOTION_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_BUTTON_PRESS_MASK,
						NULL, event->button.time);
			/* Anchor to bottom-left corner */
			x1 = x;
			y2 = y + h;
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			gnome_canvas_item_ungrab (handle_item,
						  event->button.time);
			x = x1;
			y = MIN (event->button.y, y2 - MIN_ITEM_SIZE);
			w = MAX (event->button.x - x1, MIN_ITEM_SIZE);
			h = MAX (y2 - event->button.y, MIN_ITEM_SIZE);
			gl_item_set_position_size (item, x, y, w, h);
			gl_display_select_item (display, item);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			x = x1;
			y = MIN (event->button.y, y2 - MIN_ITEM_SIZE);
			w = MAX (event->button.x - x1, MIN_ITEM_SIZE);
			h = MAX (y2 - event->button.y, MIN_ITEM_SIZE);
			gl_item_set_position_size (item, x, y, w, h);
			gl_display_select_item (display, item);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	default:
		return FALSE;
	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Bottom-left" Resize event handler.                             */
/*---------------------------------------------------------------------------*/
static int
bl_resize_event_handler (GnomeCanvasItem * handle_item,
			 GdkEvent * event,
			 gpointer data)
{
	gdouble x, y, w, h;
	static gdouble x2, y1;
	static gboolean dragging = FALSE;
	static GnomeCanvasItem *item;
	static glDisplay *display;
	GdkCursor *cursor;

	item = GNOME_CANVAS_ITEM (data);
	gl_item_get_position_size (item, &x, &y, &w, &h);
	display = gl_item_get_display (item);

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			dragging = TRUE;
			gnome_canvas_item_grab (handle_item,
						GDK_POINTER_MOTION_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_BUTTON_PRESS_MASK,
						NULL, event->button.time);
			/* Anchor to top-right corner */
			x2 = x + w;
			y1 = y;
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			gnome_canvas_item_ungrab (handle_item,
						  event->button.time);
			x = MIN (event->button.x, x2 - MIN_ITEM_SIZE);
			y = y1;
			w = MAX (x2 - event->button.x, MIN_ITEM_SIZE);
			h = MAX (event->button.y - y1, MIN_ITEM_SIZE);
			gl_item_set_position_size (item, x, y, w, h);
			gl_display_select_item (display, item);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			x = MIN (event->button.x, x2 - MIN_ITEM_SIZE);
			y = y1;
			w = MAX (x2 - event->button.x, MIN_ITEM_SIZE);
			h = MAX (event->button.y - y1, MIN_ITEM_SIZE);
			gl_item_set_position_size (item, x, y, w, h);
			gl_display_select_item (display, item);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	default:
		return FALSE;
	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Bottom-right" Resize event handler.                            */
/*---------------------------------------------------------------------------*/
static int
br_resize_event_handler (GnomeCanvasItem * handle_item,
			 GdkEvent * event,
			 gpointer data)
{
	gdouble x, y, w, h;
	static gdouble x1, y1;
	static gboolean dragging = FALSE;
	static GnomeCanvasItem *item;
	static glDisplay *display;
	GdkCursor *cursor;

	item = GNOME_CANVAS_ITEM (data);
	gl_item_get_position_size (item, &x, &y, &w, &h);
	display = gl_item_get_display (item);

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			dragging = TRUE;
			gnome_canvas_item_grab (handle_item,
						GDK_POINTER_MOTION_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_BUTTON_PRESS_MASK,
						NULL, event->button.time);
			/* Anchor to top-left corner */
			x1 = x;
			y1 = y;
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			gnome_canvas_item_ungrab (handle_item,
						  event->button.time);
			x = x1;
			y = y1;
			w = MAX (event->button.x - x1, MIN_ITEM_SIZE);
			h = MAX (event->button.y - y1, MIN_ITEM_SIZE);
			gl_item_set_position_size (item, x, y, w, h);
			gl_display_select_item (display, item);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			x = x1;
			y = y1;
			w = MAX (event->button.x - x1, MIN_ITEM_SIZE);
			h = MAX (event->button.y - y1, MIN_ITEM_SIZE);
			gl_item_set_position_size (item, x, y, w, h);
			gl_display_select_item (display, item);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	default:
		return FALSE;
	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Left-side" Resize event handler.                               */
/*---------------------------------------------------------------------------*/
static int
sl_resize_event_handler (GnomeCanvasItem * handle_item,
			 GdkEvent * event,
			 gpointer data)
{
	gdouble x, y, w, h;
	static gdouble x2;
	static gboolean dragging = FALSE;
	static GnomeCanvasItem *item;
	static glDisplay *display;
	GdkCursor *cursor;

	item = GNOME_CANVAS_ITEM (data);
	gl_item_get_position_size (item, &x, &y, &w, &h);
	display = gl_item_get_display (item);

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			dragging = TRUE;
			gnome_canvas_item_grab (handle_item,
						GDK_POINTER_MOTION_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_BUTTON_PRESS_MASK,
						NULL, event->button.time);
			/* Anchor to right side */
			x2 = x + w;
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			gnome_canvas_item_ungrab (handle_item,
						  event->button.time);
			x = MIN (event->button.x, x2 - MIN_ITEM_SIZE);
			w = MAX (x2 - event->button.x, MIN_ITEM_SIZE);
			gl_item_set_position_size (item, x, y, w, h);
			gl_display_select_item (display, item);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			x = MIN (event->button.x, x2 - MIN_ITEM_SIZE);
			w = MAX (x2 - event->button.x, MIN_ITEM_SIZE);
			gl_item_set_position_size (item, x, y, w, h);
			gl_display_select_item (display, item);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	default:
		return FALSE;
	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Right-side" Resize event handler.                              */
/*---------------------------------------------------------------------------*/
static int
sr_resize_event_handler (GnomeCanvasItem * handle_item,
			 GdkEvent * event,
			 gpointer data)
{
	gdouble x, y, w, h;
	static gdouble x1;
	static gboolean dragging = FALSE;
	static GnomeCanvasItem *item;
	static glDisplay *display;
	GdkCursor *cursor;

	item = GNOME_CANVAS_ITEM (data);
	gl_item_get_position_size (item, &x, &y, &w, &h);
	display = gl_item_get_display (item);

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			dragging = TRUE;
			gnome_canvas_item_grab (handle_item,
						GDK_POINTER_MOTION_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_BUTTON_PRESS_MASK,
						NULL, event->button.time);
			/* Anchor to left side */
			x1 = x;
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			gnome_canvas_item_ungrab (handle_item,
						  event->button.time);
			x = x1;
			w = MAX (event->button.x - x1, MIN_ITEM_SIZE);
			gl_item_set_position_size (item, x, y, w, h);
			gl_display_select_item (display, item);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			x = x1;
			w = MAX (event->button.x - x1, MIN_ITEM_SIZE);
			gl_item_set_position_size (item, x, y, w, h);
			gl_display_select_item (display, item);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	default:
		return FALSE;
	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Top-side" Resize event handler.                                */
/*---------------------------------------------------------------------------*/
static int
st_resize_event_handler (GnomeCanvasItem * handle_item,
			 GdkEvent * event,
			 gpointer data)
{
	gdouble x, y, w, h;
	static gdouble y2;
	static gboolean dragging = FALSE;
	static GnomeCanvasItem *item;
	static glDisplay *display;
	GdkCursor *cursor;

	item = GNOME_CANVAS_ITEM (data);
	gl_item_get_position_size (item, &x, &y, &w, &h);
	display = gl_item_get_display (item);

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			dragging = TRUE;
			gnome_canvas_item_grab (handle_item,
						GDK_POINTER_MOTION_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_BUTTON_PRESS_MASK,
						NULL, event->button.time);
			/* Anchor to bottom-right corner */
			y2 = y + h;
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			gnome_canvas_item_ungrab (handle_item,
						  event->button.time);
			y = MIN (event->button.y, y2 - MIN_ITEM_SIZE);
			h = MAX (y2 - event->button.y, MIN_ITEM_SIZE);
			gl_item_set_position_size (item, x, y, w, h);
			gl_display_select_item (display, item);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			y = MIN (event->button.y, y2 - MIN_ITEM_SIZE);
			h = MAX (y2 - event->button.y, MIN_ITEM_SIZE);
			gl_item_set_position_size (item, x, y, w, h);
			gl_display_select_item (display, item);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	default:
		return FALSE;
	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Bottom-side" Resize event handler.                             */
/*---------------------------------------------------------------------------*/
static int
sb_resize_event_handler (GnomeCanvasItem * handle_item,
			 GdkEvent * event,
			 gpointer data)
{
	gdouble x, y, w, h;
	static gdouble y1;
	static gboolean dragging = FALSE;
	static GnomeCanvasItem *item;
	static glDisplay *display;
	GdkCursor *cursor;

	item = GNOME_CANVAS_ITEM (data);
	gl_item_get_position_size (item, &x, &y, &w, &h);
	display = gl_item_get_display (item);

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			dragging = TRUE;
			gnome_canvas_item_grab (handle_item,
						GDK_POINTER_MOTION_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_BUTTON_PRESS_MASK,
						NULL, event->button.time);
			/* Anchor to top side */
			y1 = y;
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			gnome_canvas_item_ungrab (handle_item,
						  event->button.time);
			y = y1;
			h = MAX (event->button.y - y1, MIN_ITEM_SIZE);
			gl_item_set_position_size (item, x, y, w, h);
			gl_display_select_item (display, item);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			y = y1;
			h = MAX (event->button.y - y1, MIN_ITEM_SIZE);
			gl_item_set_position_size (item, x, y, w, h);
			gl_display_select_item (display, item);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	default:
		return FALSE;
	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Resize "P1" event handler.                                      */
/*---------------------------------------------------------------------------*/
static int
p1_resize_event_handler (GnomeCanvasItem * handle_item,
			 GdkEvent * event,
			 gpointer data)
{
	gdouble x, y, dx, dy;
	static gdouble x0, y0;
	static gboolean dragging = FALSE;
	static GnomeCanvasItem *item;
	static glDisplay *display;
	GdkCursor *cursor;

	item = GNOME_CANVAS_ITEM (data);
	gl_item_get_position_size (item, &x, &y, &dx, &dy);
	display = gl_item_get_display (item);

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			dragging = TRUE;
			gnome_canvas_item_grab (handle_item,
						GDK_POINTER_MOTION_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_BUTTON_PRESS_MASK,
						NULL, event->button.time);
			/* Anchor to P2 */
			x0 = x + dx;
			y0 = y + dy;
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			gnome_canvas_item_ungrab (handle_item,
						  event->button.time);
			x = x0;
			y = y0;
			dx = (event->button.x - x0);
			dy = (event->button.y - y0);
			gl_item_set_position_size (item, x, y, dx, dy);
			gl_display_select_item (display, item);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			x = x0;
			y = y0;
			dx = (event->button.x - x0);
			dy = (event->button.y - y0);
			gl_item_set_position_size (item, x, y, dx, dy);
			gl_display_select_item (display, item);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	default:
		return FALSE;
	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Resize "P2" event handler.                                      */
/*---------------------------------------------------------------------------*/
static int
p2_resize_event_handler (GnomeCanvasItem * handle_item,
			 GdkEvent * event,
			 gpointer data)
{
	gdouble x, y, dx, dy;
	static gdouble x0, y0;
	static gboolean dragging = FALSE;
	static GnomeCanvasItem *item;
	static glDisplay *display;
	GdkCursor *cursor;

	item = GNOME_CANVAS_ITEM (data);
	gl_item_get_position_size (item, &x, &y, &dx, &dy);
	display = gl_item_get_display (item);

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			dragging = TRUE;
			gnome_canvas_item_grab (handle_item,
						GDK_POINTER_MOTION_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_BUTTON_PRESS_MASK,
						NULL, event->button.time);
			/* Anchor to P1 */
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
			gnome_canvas_item_ungrab (handle_item,
						  event->button.time);
			x = x0;
			y = y0;
			dx = (event->button.x - x0);
			dy = (event->button.y - y0);
			gl_item_set_position_size (item, x, y, dx, dy);
			gl_display_select_item (display, item);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			x = x0;
			y = y0;
			dx = (event->button.x - x0);
			dy = (event->button.y - y0);
			gl_item_set_position_size (item, x, y, dx, dy);
			gl_display_select_item (display, item);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	default:
		return FALSE;
	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Passthrough event handler.                                      */
/*---------------------------------------------------------------------------*/
static int
passthrough_event_handler (GnomeCanvasItem * handle_item,
			   GdkEvent * event,
			   gpointer data)
{
	static GnomeCanvasItem *item;
	static glDisplay *display;

	item = GNOME_CANVAS_ITEM (data);
	display = gl_item_get_display (item);
	return gl_display_item_event_handler (item, event, display);

}
