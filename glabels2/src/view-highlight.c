/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_highlight.c:  GLabels Resizable Highlight module
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

#include <math.h>

#include "view-highlight.h"

#include "debug.h"

#define MIN_ITEM_SIZE 1.0
#define FG_COLOR GNOME_CANVAS_COLOR_A (0, 0, 255, 255)
#define BG_COLOR GNOME_CANVAS_COLOR_A (255, 255, 255, 255)

/*===========================================*/
/* Private data types                        */
/*===========================================*/

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static GnomeCanvasItem *highlight_resizable_box  (glViewObject *view_object,
						  glViewHighlightStyle style);
static GnomeCanvasItem *highlight_resizable_line (glViewObject *view_object);
static GnomeCanvasItem *highlight_simple         (glViewObject *view_object);

static void update_resizable_box  (GnomeCanvasItem *highlight,
				   glViewHighlightStyle style);
static void update_resizable_line (GnomeCanvasItem *highlight);
static void update_simple         (GnomeCanvasItem *highlight);

static int tl_resize_event_handler (GnomeCanvasItem * handle_item,
				    GdkEvent * event,
				    glViewObject *view_object);
static int tr_resize_event_handler (GnomeCanvasItem * handle_item,
				    GdkEvent * event,
				    glViewObject *view_object);
static int bl_resize_event_handler (GnomeCanvasItem * handle_item,
				    GdkEvent * event,
				    glViewObject *view_object);
static int br_resize_event_handler (GnomeCanvasItem * handle_item,
				    GdkEvent * event,
				    glViewObject *view_object);

static int sl_resize_event_handler (GnomeCanvasItem * handle_item,
				    GdkEvent * event,
				    glViewObject *view_object);
static int sr_resize_event_handler (GnomeCanvasItem * handle_item,
				    GdkEvent * event,
				    glViewObject *view_object);
static int st_resize_event_handler (GnomeCanvasItem * handle_item,
				    GdkEvent * event,
				    glViewObject *view_object);
static int sb_resize_event_handler (GnomeCanvasItem * handle_item,
				    GdkEvent * event,
				    glViewObject *view_object);

static int p1_resize_event_handler (GnomeCanvasItem * handle_item,
				    GdkEvent * event,
				    glViewObject *view_object);
static int p2_resize_event_handler (GnomeCanvasItem * handle_item,
				    GdkEvent * event,
				    glViewObject *view_object);

static int passthrough_event_handler (GnomeCanvasItem * handle_item,
				      GdkEvent * event,
				      glViewObject *view_object);

/*****************************************************************************/
/* Create a resizable highlight for given object.                            */
/*****************************************************************************/
GnomeCanvasItem *
gl_view_highlight_new (glViewObject *view_object,
		       glViewHighlightStyle style)
{
	GnomeCanvasItem *highlight;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT(view_object));

	switch (style) {

	case GL_VIEW_HIGHLIGHT_BOX_RESIZABLE:
	case GL_VIEW_HIGHLIGHT_ELLIPSE_RESIZABLE:
		highlight = highlight_resizable_box (view_object, style);
		break;

	case GL_VIEW_HIGHLIGHT_LINE_RESIZABLE:
		highlight = highlight_resizable_line (view_object);
		break;

	case GL_VIEW_HIGHLIGHT_SIMPLE:
		highlight = highlight_simple (view_object);
		break;

	default:
		g_warning ("Invalid resizable highlight style");

	}

	g_object_set_data (G_OBJECT(highlight), "style",
			   GINT_TO_POINTER(style));

	gl_debug (DEBUG_VIEW, "END");

	return highlight;
}

/*****************************************************************************/
/* Update a resizable highlight for given item.                              */
/*****************************************************************************/
void
gl_view_highlight_update (GnomeCanvasItem * highlight)
{
	glViewHighlightStyle style;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GNOME_IS_CANVAS_ITEM (highlight));

	style =	GPOINTER_TO_INT(g_object_get_data (G_OBJECT(highlight),
						   "style"));

	switch (style) {

	case GL_VIEW_HIGHLIGHT_BOX_RESIZABLE:
	case GL_VIEW_HIGHLIGHT_ELLIPSE_RESIZABLE:
		update_resizable_box (highlight, style);
		break;

	case GL_VIEW_HIGHLIGHT_LINE_RESIZABLE:
		update_resizable_line (highlight);
		break;

	case GL_VIEW_HIGHLIGHT_SIMPLE:
		update_simple (highlight);
		break;

	default:
		g_warning ("Invalid resizable highlight style");

	}

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create a resizable box highlight.                               */
/*---------------------------------------------------------------------------*/
static GnomeCanvasItem *
highlight_resizable_box  (glViewObject *view_object,
			  glViewHighlightStyle style)
{
	GnomeCanvasItem *highlight;
	glLabelObject *object;
	glView *view;
	gdouble x, y, w, h;
	GnomeCanvasItem *outline;	        /* Outline around item */
	GnomeCanvasItem *tl, *tr, *bl, *br;	/* Handles at four corners */
	GnomeCanvasItem *sl, *sr, *st, *sb;	/* Handles at each side */

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW_OBJECT (view_object));

	object = gl_view_object_get_object (view_object);
	gl_label_object_get_position (object, &x, &y);
	gl_label_object_get_size (object, &w, &h);

	view = gl_view_object_get_view (view_object);

	highlight =
		gnome_canvas_item_new (gnome_canvas_root
				       (GNOME_CANVAS (view->canvas)),
				       gnome_canvas_group_get_type (),
				       "x", x, "y", y, NULL);

	gnome_canvas_item_hide (highlight);

	switch (style) {
	case GL_VIEW_HIGHLIGHT_BOX_RESIZABLE:
		outline =
			gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
					       gnome_canvas_rect_get_type (),
					       "x1", -0.5,
					       "y1", -0.5,
					       "x2", w + 0.5,
					       "y2", h + 0.5,
					       "width_pixels", 2,
					       "outline_color_rgba", FG_COLOR,
					       NULL);
		break;
	case GL_VIEW_HIGHLIGHT_ELLIPSE_RESIZABLE:
		outline =
			gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
					       gnome_canvas_ellipse_get_type (),
					       "x1", -0.5,
					       "y1", -0.5,
					       "x2", w + 0.5,
					       "y2", h + 0.5,
					       "width_pixels", 2,
					       "outline_color_rgba", FG_COLOR,
					       NULL);
		break;
	default:
		outline = NULL;
		g_warning ("Invalid resizable highlight style");
	}

	tl = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
				    gnome_canvas_rect_get_type (),
				    "x1", -1.0,
				    "y1", -1.0,
				    "x2", +1.0,
				    "y2", +1.0,
				    "fill_color_rgba", FG_COLOR,
				    "outline_color_rgba", BG_COLOR,
				    NULL);

	tr = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
				    gnome_canvas_rect_get_type (),
				    "x1", w - 1.0,
				    "y1", -1.0,
				    "x2", w + 1.0,
				    "y2", +1.0,
				    "fill_color_rgba", FG_COLOR,
				    "outline_color_rgba", BG_COLOR,
				    NULL);

	bl = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
				    gnome_canvas_rect_get_type (),
				    "x1", -1.0,
				    "y1", h - 1.0,
				    "x2", +1.0,
				    "y2", h + 1.0,
				    "fill_color_rgba", FG_COLOR,
				    "outline_color_rgba", BG_COLOR,
				    NULL);

	br = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
				    gnome_canvas_rect_get_type (),
				    "x1", w - 1.0,
				    "y1", h - 1.0,
				    "x2", w + 1.0,
				    "y2", h + 1.0,
				    "fill_color_rgba", FG_COLOR,
				    "outline_color_rgba", BG_COLOR,
				    NULL);

	sl = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
				    gnome_canvas_rect_get_type (),
				    "x1", -1.0,
				    "y1", h / 2.0 - 1.0,
				    "x2", +1.0,
				    "y2", h / 2.0 + 1.0,
				    "fill_color_rgba", FG_COLOR,
				    "outline_color_rgba", BG_COLOR,
				    NULL);

	sr = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
				    gnome_canvas_rect_get_type (),
				    "x1", w - 1.0,
				    "y1", h / 2.0 - 1.0,
				    "x2", w + 1.0,
				    "y2", h / 2.0 + 1.0,
				    "fill_color_rgba", FG_COLOR,
				    "outline_color_rgba", BG_COLOR,
				    NULL);

	st = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
				    gnome_canvas_rect_get_type (),
				    "x1", w / 2.0 - 1.0,
				    "y1", -1.0,
				    "x2", w / 2.0 + 1.0,
				    "y2", +1.0,
				    "fill_color_rgba", FG_COLOR,
				    "outline_color_rgba", BG_COLOR,
				    NULL);

	sb = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
				    gnome_canvas_rect_get_type (),
				    "x1", w / 2.0 - 1.0,
				    "y1", h - 1.0,
				    "x2", w / 2.0 + 1.0,
				    "y2", h + 1.0,
				    "fill_color_rgba", FG_COLOR,
				    "outline_color_rgba", BG_COLOR,
				    NULL);

	g_object_set_data (G_OBJECT (highlight), "object", object);

	g_object_set_data (G_OBJECT (highlight), "outline", outline);

	g_object_set_data (G_OBJECT (highlight), "tl", tl);
	g_object_set_data (G_OBJECT (highlight), "tr", tr);
	g_object_set_data (G_OBJECT (highlight), "bl", bl);
	g_object_set_data (G_OBJECT (highlight), "br", br);
	g_object_set_data (G_OBJECT (highlight), "sl", sl);
	g_object_set_data (G_OBJECT (highlight), "sr", sr);
	g_object_set_data (G_OBJECT (highlight), "st", st);
	g_object_set_data (G_OBJECT (highlight), "sb", sb);

	g_signal_connect (G_OBJECT (tl), "event",
			  G_CALLBACK (tl_resize_event_handler), view_object);
	g_signal_connect (G_OBJECT (tr), "event",
			  G_CALLBACK (tr_resize_event_handler), view_object);
	g_signal_connect (G_OBJECT (bl), "event",
			  G_CALLBACK (bl_resize_event_handler), view_object);
	g_signal_connect (G_OBJECT (br), "event",
			  G_CALLBACK (br_resize_event_handler), view_object);
	g_signal_connect (G_OBJECT (sl), "event",
			  G_CALLBACK (sl_resize_event_handler), view_object);
	g_signal_connect (G_OBJECT (sr), "event",
			  G_CALLBACK (sr_resize_event_handler), view_object);
	g_signal_connect (G_OBJECT (st), "event",
			  G_CALLBACK (st_resize_event_handler), view_object);
	g_signal_connect (G_OBJECT (sb), "event",
			  G_CALLBACK (sb_resize_event_handler), view_object);

	g_signal_connect (G_OBJECT (outline), "event",
			  G_CALLBACK (passthrough_event_handler), view_object);

	gnome_canvas_item_raise_to_top (highlight);

	gl_debug (DEBUG_VIEW, "END");

	return highlight;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create a resizable line highlight.                              */
/*---------------------------------------------------------------------------*/
static GnomeCanvasItem *
highlight_resizable_line (glViewObject *view_object)
{
	GnomeCanvasItem *highlight;
	glLabelObject *object;
	glView *view;
	gdouble x, y, dx, dy;
	GnomeCanvasPoints *points;
	GnomeCanvasItem *outline;	/* Outline around item */
	GnomeCanvasItem *p1, *p2;	/* Handles at endpoints */

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW_OBJECT (view_object));

	object = gl_view_object_get_object (view_object);
	gl_label_object_get_position (object, &x, &y);
	gl_label_object_get_size (object, &dx, &dy);

	view = gl_view_object_get_view (view_object);

	points = gnome_canvas_points_new (2);


	highlight =
		gnome_canvas_item_new (gnome_canvas_root
				       (GNOME_CANVAS (view->canvas)),
				       gnome_canvas_group_get_type (),
				       "x", x, "y", y, NULL);

	gnome_canvas_item_hide (highlight);

	points->coords[0] = 0.0;
	points->coords[1] = 0.0;
	points->coords[2] = dx;
	points->coords[3] = dy;
	outline = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
					 gnome_canvas_line_get_type (),
					 "points", points,
					 "width_pixels", 3,
					 "fill_color_rgba", FG_COLOR,
					 NULL);

	p1 = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
				    gnome_canvas_rect_get_type (),
				    "x1", -1.0,
				    "y1", -1.0,
				    "x2", +1.0,
				    "y2", +1.0,
				    "fill_color_rgba", FG_COLOR,
				    "outline_color_rgba", BG_COLOR,
				    NULL);

	p2 = gnome_canvas_item_new (GNOME_CANVAS_GROUP (highlight),
				    gnome_canvas_rect_get_type (),
				    "x1", dx - 1.0,
				    "y1", dy - 1.0,
				    "x2", dx + 1.0,
				    "y2", dy + 1.0,
				    "fill_color_rgba", FG_COLOR,
				    "outline_color_rgba", BG_COLOR,
				    NULL);

	g_object_set_data (G_OBJECT (highlight), "object", object);

	g_object_set_data (G_OBJECT (highlight), "outline", outline);

	g_object_set_data (G_OBJECT (highlight), "p1", p1);
	g_object_set_data (G_OBJECT (highlight), "p2", p2);

	g_signal_connect (G_OBJECT (p1), "event",
			  G_CALLBACK (p1_resize_event_handler), view_object);
	g_signal_connect (G_OBJECT (p2), "event",
			  G_CALLBACK (p2_resize_event_handler), view_object);

	g_signal_connect (G_OBJECT (outline), "event",
			  G_CALLBACK (passthrough_event_handler), view_object);

	gnome_canvas_points_free (points);

	gnome_canvas_item_raise_to_top (highlight);

	gl_debug (DEBUG_VIEW, "END");

	return highlight;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create a simple box highlight.                                  */
/*---------------------------------------------------------------------------*/
static GnomeCanvasItem *
highlight_simple         (glViewObject *view_object)
{
	GnomeCanvasItem *highlight, *outline;
	glLabelObject *object;
	glView *view;
	gdouble x, y, w, h;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW_OBJECT (view_object));

	object = gl_view_object_get_object (view_object);
	gl_label_object_get_position (object, &x, &y);
	gl_label_object_get_size (object, &w, &h);

	view = gl_view_object_get_view (view_object);

	highlight =
		gnome_canvas_item_new (gnome_canvas_root
				       (GNOME_CANVAS (view->canvas)),
				       gnome_canvas_group_get_type (),
				       "x", x, "y", y, NULL);

	gnome_canvas_item_hide (highlight);

	outline =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(highlight),
				       gnome_canvas_rect_get_type (),
				       "x1", -0.5,
				       "y1", -0.5,
				       "x2", w + 0.5,
				       "y2", h + 0.5,
				       "width_pixels", 2,
				       "outline_color_rgba", FG_COLOR,
				       NULL);


	g_object_set_data (G_OBJECT (highlight), "outline", outline);

	g_object_set_data (G_OBJECT (highlight), "object", object);

	g_signal_connect (G_OBJECT (highlight), "event",
			  G_CALLBACK (passthrough_event_handler), view_object);

	gnome_canvas_item_raise_to_top (highlight);

	gl_debug (DEBUG_VIEW, "END");

	return highlight;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Update a resizable box highlight.                               */
/*---------------------------------------------------------------------------*/
static void
update_resizable_box  (GnomeCanvasItem *highlight,
		       glViewHighlightStyle style)
{
	glLabelObject *object;
	gdouble w, h;
	GnomeCanvasItem *outline;               /* Outline around item */
	GnomeCanvasItem *tl, *tr, *bl, *br;	/* Handles at four corners */
	GnomeCanvasItem *sl, *sr, *st, *sb;	/* Handles at each side */

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GNOME_IS_CANVAS_ITEM (highlight));

	object = g_object_get_data (G_OBJECT (highlight), "object");
	gl_label_object_get_size (object, &w, &h);

	outline = g_object_get_data (G_OBJECT (highlight), "outline");

	tl = g_object_get_data (G_OBJECT (highlight), "tl");
	tr = g_object_get_data (G_OBJECT (highlight), "tr");
	bl = g_object_get_data (G_OBJECT (highlight), "bl");
	br = g_object_get_data (G_OBJECT (highlight), "br");
	sl = g_object_get_data (G_OBJECT (highlight), "sl");
	sr = g_object_get_data (G_OBJECT (highlight), "sr");
	st = g_object_get_data (G_OBJECT (highlight), "st");
	sb = g_object_get_data (G_OBJECT (highlight), "sb");

	gnome_canvas_item_set (outline,
			       "x1", -0.5,
			       "y1", -0.5,
			       "x2", w + 0.5,
			       "y2", h + 0.5,
			       NULL);

	gnome_canvas_item_set (tl,
			       "x1", -1.0,
			       "y1", -1.0,
			       "x2", +1.0,
			       "y2", +1.0,
			       NULL);

	gnome_canvas_item_set (tr,
			       "x1", w - 1.0,
			       "y1", -1.0,
			       "x2", w + 1.0,
			       "y2", +1.0,
			       NULL);

	gnome_canvas_item_set (bl,
			       "x1", -1.0,
			       "y1", h - 1.0,
			       "x2", +1.0,
			       "y2", h + 1.0,
			       NULL);

	gnome_canvas_item_set (br,
			       "x1", w - 1.0,
			       "y1", h - 1.0,
			       "x2", w + 1.0,
			       "y2", h + 1.0,
			       NULL);

	gnome_canvas_item_set (sl,
			       "x1", -1.0,
			       "y1", h / 2.0 - 1.0,
			       "x2", +1.0,
			       "y2", h / 2.0 + 1.0,
			       NULL);

	gnome_canvas_item_set (sr,
			       "x1", w - 1.0,
			       "y1", h / 2.0 - 1.0,
			       "x2", w + 1.0,
			       "y2", h / 2.0 + 1.0,
			       NULL);

	gnome_canvas_item_set (st,
			       "x1", w / 2.0 - 1.0,
			       "y1", -1.0,
			       "x2", w / 2.0 + 1.0,
			       "y2", +1.0,
			       NULL);

	gnome_canvas_item_set (sb,
			       "x1", w / 2.0 - 1.0,
			       "y1", h - 1.0,
			       "x2", w / 2.0 + 1.0,
			       "y2", h + 1.0,
			       NULL);

	gnome_canvas_item_raise_to_top (highlight);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Update a resizable line highlight.                              */
/*---------------------------------------------------------------------------*/
static void
update_resizable_line (GnomeCanvasItem *highlight)
{
	glLabelObject *object;
	gdouble dx, dy;
	GnomeCanvasPoints *points;
	GnomeCanvasItem *outline;	/* Outline around item */
	GnomeCanvasItem *p1, *p2;	/* Handles at endpoints */

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GNOME_IS_CANVAS_ITEM (highlight));

	object = g_object_get_data (G_OBJECT (highlight), "object");
	gl_label_object_get_size (object, &dx, &dy);

	points = gnome_canvas_points_new (2);

	outline = g_object_get_data (G_OBJECT (highlight), "outline");

	p1 = g_object_get_data (G_OBJECT (highlight), "p1");
	p2 = g_object_get_data (G_OBJECT (highlight), "p2");

	points->coords[0] = 0.0;
	points->coords[1] = 0.0;
	points->coords[2] = dx;
	points->coords[3] = dy;
	gnome_canvas_item_set (outline, "points", points, NULL);

	gnome_canvas_item_set (p1,
			       "x1", -1.0,
			       "y1", -1.0,
			       "x2", +1.0,
			       "y2", +1.0,
			       NULL);

	gnome_canvas_item_set (p2,
			       "x1", dx - 1.0,
			       "y1", dy - 1.0,
			       "x2", dx + 1.0,
			       "y2", dy + 1.0,
			       NULL);

	gnome_canvas_points_free (points);

	gnome_canvas_item_raise_to_top (highlight);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Update a simple box highlight.                                  */
/*---------------------------------------------------------------------------*/
static void
update_simple (GnomeCanvasItem *highlight)
{
	glLabelObject *object;
	gdouble w, h;
	GnomeCanvasItem *outline;               /* Outline around item */

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GNOME_IS_CANVAS_ITEM (highlight));

	object = g_object_get_data (G_OBJECT (highlight), "object");
	gl_label_object_get_size (object, &w, &h);

	outline = g_object_get_data (G_OBJECT (highlight), "outline");

	gnome_canvas_item_set (outline,
			       "x2", w + 0.5,
			       "y2", h + 0.5,
			       NULL);

	gnome_canvas_item_raise_to_top (highlight);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Top-left" Resize event handler.                    */
/*---------------------------------------------------------------------------*/
static int
tl_resize_event_handler (GnomeCanvasItem * handle_item,
			 GdkEvent * event,
			 glViewObject *view_object)
{
	gdouble x, y, w, h;
	static gdouble x2, y2;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	glView *view;
	GdkCursor *cursor;

	gl_debug (DEBUG_VIEW, "");

	view = gl_view_object_get_view (view_object);
	if ( view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = gl_view_object_get_object (view_object);
	gl_label_object_get_position (object, &x, &y);
	gl_label_object_get_size (object, &w, &h);

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
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
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
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
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
			 glViewObject *view_object)
{
	gdouble x, y, w, h;
	static gdouble x1, y2;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	glView *view;
	GdkCursor *cursor;

	gl_debug (DEBUG_VIEW, "");

	view = gl_view_object_get_view (view_object);
	if ( view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = gl_view_object_get_object (view_object);
	gl_label_object_get_position (object, &x, &y);
	gl_label_object_get_size (object, &w, &h);

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
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
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
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
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
			 glViewObject *view_object)
{
	gdouble x, y, w, h;
	static gdouble x2, y1;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	glView *view;
	GdkCursor *cursor;

	gl_debug (DEBUG_VIEW, "");

	view = gl_view_object_get_view (view_object);
	if ( view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = gl_view_object_get_object (view_object);
	gl_label_object_get_position (object, &x, &y);
	gl_label_object_get_size (object, &w, &h);

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
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
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
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
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
			 glViewObject *view_object)
{
	gdouble x, y, w, h;
	static gdouble x1, y1;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	glView *view;
	GdkCursor *cursor;

	view = gl_view_object_get_view (view_object);
	if ( view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	gl_debug (DEBUG_VIEW, "");

	object = gl_view_object_get_object (view_object);
	gl_label_object_get_position (object, &x, &y);
	gl_label_object_get_size (object, &w, &h);

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
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
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
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
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
			 glViewObject *view_object)
{
	gdouble x, y, w, h;
	static gdouble x2;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	glView *view;
	GdkCursor *cursor;

	gl_debug (DEBUG_VIEW, "");

	view = gl_view_object_get_view (view_object);
	if ( view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = gl_view_object_get_object (view_object);
	gl_label_object_get_position (object, &x, &y);
	gl_label_object_get_size (object, &w, &h);

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
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			x = MIN (event->button.x, x2 - MIN_ITEM_SIZE);
			w = MAX (x2 - event->button.x, MIN_ITEM_SIZE);
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
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
			 glViewObject *view_object)
{
	gdouble x, y, w, h;
	static gdouble x1;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	glView *view;
	GdkCursor *cursor;

	gl_debug (DEBUG_VIEW, "");

	view = gl_view_object_get_view (view_object);
	if ( view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = gl_view_object_get_object (view_object);
	gl_label_object_get_position (object, &x, &y);
	gl_label_object_get_size (object, &w, &h);

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
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			x = x1;
			w = MAX (event->button.x - x1, MIN_ITEM_SIZE);
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
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
			 glViewObject *view_object)
{
	gdouble x, y, w, h;
	static gdouble y2;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	glView *view;
	GdkCursor *cursor;

	gl_debug (DEBUG_VIEW, "");

	view = gl_view_object_get_view (view_object);
	if ( view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = gl_view_object_get_object (view_object);
	gl_label_object_get_position (object, &x, &y);
	gl_label_object_get_size (object, &w, &h);

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
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			y = MIN (event->button.y, y2 - MIN_ITEM_SIZE);
			h = MAX (y2 - event->button.y, MIN_ITEM_SIZE);
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
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
			 glViewObject *view_object)
{
	gdouble x, y, w, h;
	static gdouble y1;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	glView *view;
	GdkCursor *cursor;

	gl_debug (DEBUG_VIEW, "");

	view = gl_view_object_get_view (view_object);
	if ( view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = gl_view_object_get_object (view_object);
	gl_label_object_get_position (object, &x, &y);
	gl_label_object_get_size (object, &w, &h);

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
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			y = y1;
			h = MAX (event->button.y - y1, MIN_ITEM_SIZE);
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
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
			 glViewObject *view_object)
{
	gdouble x, y, dx, dy;
	static gdouble x0, y0;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	glView *view;
	GdkCursor *cursor;

	gl_debug (DEBUG_VIEW, "");

	view = gl_view_object_get_view (view_object);
	if ( view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = gl_view_object_get_object (view_object);
	gl_label_object_get_position (object, &x, &y);
	gl_label_object_get_size (object, &dx, &dy);

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
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, dx, dy);
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
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, dx, dy);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
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
			 glViewObject *view_object)
{
	gdouble x, y, dx, dy;
	static gdouble x0, y0;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	glView *view;
	GdkCursor *cursor;

	gl_debug (DEBUG_VIEW, "");

	view = gl_view_object_get_view (view_object);
	if ( view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = gl_view_object_get_object (view_object);
	gl_label_object_get_position (object, &x, &y);
	gl_label_object_get_size (object, &dx, &dy);

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
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, dx, dy);
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
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, dx, dy);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
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
			   glViewObject *view_object)
{
	GnomeCanvasItem *group;

	gl_debug (DEBUG_VIEW, "");

	group = gl_view_object_get_group (view_object);
	return gl_view_item_event_handler (group, event, view_object);

}
