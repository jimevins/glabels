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
struct _glViewHighlightPrivate {
	glViewHighlightStyle     style;
	glLabelObject           *object;
	glView                  *view;

	GnomeCanvasItem         *group;
	GnomeCanvasItem         *outline;
	GnomeCanvasItem         *tl, *tr, *bl, *br; /* Corner handles */
	GnomeCanvasItem         *sl, *sr, *st, *sb; /* Side handles */
	GnomeCanvasItem         *p1, *p2;           /* Line endpoint handles */
};

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GObjectClass *parent_class = NULL;

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void   gl_view_highlight_class_init       (glViewHighlightClass   *klass);
static void   gl_view_highlight_instance_init    (glViewHighlight        *view_highlight);
static void   gl_view_highlight_finalize         (GObject                *object);


static void   highlight_resizable_box_construct  (glViewHighlight        *view_highlight,
						  glViewObject           *view_object,
						  glViewHighlightStyle    style);

static void   highlight_resizable_line_construct (glViewHighlight        *view_highlight,
						  glViewObject           *view_object);

static void   highlight_simple_construct         (glViewHighlight        *view_highlight,
						  glViewObject           *view_object);

static void   object_moved_cb                    (glLabelObject          *object,
						  gdouble                 x,
						  gdouble                 y,
						  glViewHighlight        *view_highlight);

static void   object_changed_cb                  (glLabelObject          *object,
						  glViewHighlight        *view_highlight);


static void   update_resizable_box               (glViewHighlight        *view_highlight);

static void   update_resizable_line              (glViewHighlight        *view_highlight);

static void   update_simple                      (glViewHighlight        *view_highlight);


static int tl_resize_event_handler (GnomeCanvasItem *handle_item,
				    GdkEvent        *event,
				    glViewHighlight *view_highlight);
static int tr_resize_event_handler (GnomeCanvasItem *handle_item,
				    GdkEvent        *event,
				    glViewHighlight *view_highlight);
static int bl_resize_event_handler (GnomeCanvasItem *handle_item,
				    GdkEvent        *event,
				    glViewHighlight *view_highlight);
static int br_resize_event_handler (GnomeCanvasItem *handle_item,
				    GdkEvent        *event,
				    glViewHighlight *view_highlight);

static int sl_resize_event_handler (GnomeCanvasItem *handle_item,
				    GdkEvent        *event,
				    glViewHighlight *view_highlight);
static int sr_resize_event_handler (GnomeCanvasItem *handle_item,
				    GdkEvent        *event,
				    glViewHighlight *view_highlight);
static int st_resize_event_handler (GnomeCanvasItem *handle_item,
				    GdkEvent        *event,
				    glViewHighlight *view_highlight);
static int sb_resize_event_handler (GnomeCanvasItem *handle_item,
				    GdkEvent        *event,
				    glViewHighlight *view_highlight);

static int p1_resize_event_handler (GnomeCanvasItem *handle_item,
				    GdkEvent        *event,
				    glViewHighlight *view_highlight);
static int p2_resize_event_handler (GnomeCanvasItem *handle_item,
				    GdkEvent        *event,
				    glViewHighlight *view_highlight);

static int passthrough_event_handler (GnomeCanvasItem *handle_item,
				      GdkEvent        *event,
				      glViewObject    *view_object);

/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_view_highlight_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (glViewHighlightClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_view_highlight_class_init,
			NULL,
			NULL,
			sizeof (glViewHighlight),
			0,
			(GInstanceInitFunc) gl_view_highlight_instance_init,
		};

		type = g_type_register_static (G_TYPE_OBJECT,
					       "glViewHighlight", &info, 0);
	}

	return type;
}

static void
gl_view_highlight_class_init (glViewHighlightClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	gl_debug (DEBUG_VIEW, "START");

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gl_view_highlight_finalize;

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_highlight_instance_init (glViewHighlight *view_highlight)
{
	gl_debug (DEBUG_VIEW, "START");

	view_highlight->private = g_new0 (glViewHighlightPrivate, 1);

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_highlight_finalize (GObject *object)
{
	glLabel       *parent;
	glView        *view;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (object && GL_IS_VIEW_HIGHLIGHT (object));

	g_object_unref (GL_VIEW_HIGHLIGHT(object)->private->object);

	gtk_object_destroy (GTK_OBJECT(GL_VIEW_HIGHLIGHT(object)->private->group));

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Create a resizable highlight for given object.                            */
/*****************************************************************************/
GObject *
gl_view_highlight_new (glViewObject         *view_object,
		       glViewHighlightStyle  style)
{
	glViewHighlight *view_highlight;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT(view_object));

	view_highlight = g_object_new (gl_view_highlight_get_type (), NULL);

	view_highlight->private->style = style;

	switch (style) {

	case GL_VIEW_HIGHLIGHT_BOX_RESIZABLE:
	case GL_VIEW_HIGHLIGHT_ELLIPSE_RESIZABLE:
		highlight_resizable_box_construct (view_highlight, view_object, style);
		break;

	case GL_VIEW_HIGHLIGHT_LINE_RESIZABLE:
		highlight_resizable_line_construct (view_highlight, view_object);
		break;

	case GL_VIEW_HIGHLIGHT_SIMPLE:
		highlight_simple_construct (view_highlight, view_object);
		break;

	default:
		g_warning ("Invalid resizable highlight style");

	}

	g_signal_connect (G_OBJECT (view_highlight->private->object), "moved",
			  G_CALLBACK (object_moved_cb), view_highlight);

	g_signal_connect (G_OBJECT (view_highlight->private->object), "changed",
			  G_CALLBACK (object_changed_cb), view_highlight);

	gl_debug (DEBUG_VIEW, "END");

	return G_OBJECT (view_highlight);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create a resizable box highlight.                               */
/*---------------------------------------------------------------------------*/
static void
highlight_resizable_box_construct (glViewHighlight        *view_highlight,
				   glViewObject           *view_object,
				   glViewHighlightStyle    style)
{
	gdouble x, y, w, h;
	GnomeCanvasItem *group;
	glView *view;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_highlight && GL_IS_VIEW_HIGHLIGHT (view_highlight));
	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));

	view   = gl_view_object_get_view (view_object);
	view_highlight->private->view = view;

	object = gl_view_object_get_object (view_object);
	view_highlight->private->object = object;

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	gl_label_object_get_position (object, &x, &y);
	gl_label_object_get_size (object, &w, &h);

	view_highlight->private->group =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (view->canvas)),
				       gnome_canvas_group_get_type (),
				       "x", x, "y", y, NULL);
	gnome_canvas_item_hide (view_highlight->private->group);
	group = view_highlight->private->group;

	switch (style) {
	case GL_VIEW_HIGHLIGHT_BOX_RESIZABLE:
		view_highlight->private->outline =
			gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
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
		view_highlight->private->outline =
			gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
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
		view_highlight->private->outline = NULL;
		g_warning ("Invalid resizable highlight style");
	}

	view_highlight->private->tl =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", -1.0,
				       "y1", -1.0,
				       "x2", +1.0,
				       "y2", +1.0,
				       "fill_color_rgba", FG_COLOR,
				       "outline_color_rgba", BG_COLOR,
				       NULL);

	view_highlight->private->tr =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", w - 1.0,
				       "y1", -1.0,
				       "x2", w + 1.0,
				       "y2", +1.0,
				       "fill_color_rgba", FG_COLOR,
				       "outline_color_rgba", BG_COLOR,
				       NULL);

	view_highlight->private->bl =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", -1.0,
				       "y1", h - 1.0,
				       "x2", +1.0,
				       "y2", h + 1.0,
				       "fill_color_rgba", FG_COLOR,
				       "outline_color_rgba", BG_COLOR,
				       NULL);

	view_highlight->private->br =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", w - 1.0,
				       "y1", h - 1.0,
				       "x2", w + 1.0,
				       "y2", h + 1.0,
				       "fill_color_rgba", FG_COLOR,
				       "outline_color_rgba", BG_COLOR,
				       NULL);

	view_highlight->private->sl =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", -1.0,
				       "y1", h / 2.0 - 1.0,
				       "x2", +1.0,
				       "y2", h / 2.0 + 1.0,
				       "fill_color_rgba", FG_COLOR,
				       "outline_color_rgba", BG_COLOR,
				       NULL);

	view_highlight->private->sr =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", w - 1.0,
				       "y1", h / 2.0 - 1.0,
				       "x2", w + 1.0,
				       "y2", h / 2.0 + 1.0,
				       "fill_color_rgba", FG_COLOR,
				       "outline_color_rgba", BG_COLOR,
				       NULL);

	view_highlight->private->st =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", w / 2.0 - 1.0,
				       "y1", -1.0,
				       "x2", w / 2.0 + 1.0,
				       "y2", +1.0,
				       "fill_color_rgba", FG_COLOR,
				       "outline_color_rgba", BG_COLOR,
				       NULL);

	view_highlight->private->sb =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", w / 2.0 - 1.0,
				       "y1", h - 1.0,
				       "x2", w / 2.0 + 1.0,
				       "y2", h + 1.0,
				       "fill_color_rgba", FG_COLOR,
				       "outline_color_rgba", BG_COLOR,
				       NULL);

	g_signal_connect (G_OBJECT (view_highlight->private->tl), "event",
			  G_CALLBACK (tl_resize_event_handler), view_highlight);
	g_signal_connect (G_OBJECT (view_highlight->private->tr), "event",
			  G_CALLBACK (tr_resize_event_handler), view_highlight);
	g_signal_connect (G_OBJECT (view_highlight->private->bl), "event",
			  G_CALLBACK (bl_resize_event_handler), view_highlight);
	g_signal_connect (G_OBJECT (view_highlight->private->br), "event",
			  G_CALLBACK (br_resize_event_handler), view_highlight);
	g_signal_connect (G_OBJECT (view_highlight->private->sl), "event",
			  G_CALLBACK (sl_resize_event_handler), view_highlight);
	g_signal_connect (G_OBJECT (view_highlight->private->sr), "event",
			  G_CALLBACK (sr_resize_event_handler), view_highlight);
	g_signal_connect (G_OBJECT (view_highlight->private->st), "event",
			  G_CALLBACK (st_resize_event_handler), view_highlight);
	g_signal_connect (G_OBJECT (view_highlight->private->sb), "event",
			  G_CALLBACK (sb_resize_event_handler), view_highlight);

	g_signal_connect (G_OBJECT (view_highlight->private->outline), "event",
			  G_CALLBACK (passthrough_event_handler), view_object);

	/* send to top, then lower below all items that form the foregound */
	gnome_canvas_item_raise_to_top (group);
	gl_view_raise_fg (view);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create a resizable line highlight.                              */
/*---------------------------------------------------------------------------*/
static void
highlight_resizable_line_construct (glViewHighlight        *view_highlight,
				    glViewObject           *view_object)
{
	gdouble x, y, dx, dy;
	GnomeCanvasItem *group;
	GnomeCanvasPoints *points;
	glView *view;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_highlight && GL_IS_VIEW_HIGHLIGHT (view_highlight));
	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));

	view   = gl_view_object_get_view (view_object);
	view_highlight->private->view = view;

	object = gl_view_object_get_object (view_object);
	view_highlight->private->object = object;

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	gl_label_object_get_position (object, &x, &y);
	gl_label_object_get_size (object, &dx, &dy);

	points = gnome_canvas_points_new (2);

	view_highlight->private->group =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (view->canvas)),
				       gnome_canvas_group_get_type (),
				       "x", x, "y", y, NULL);
	gnome_canvas_item_hide (view_highlight->private->group);
	group = view_highlight->private->group;

	points->coords[0] = 0.0;
	points->coords[1] = 0.0;
	points->coords[2] = dx;
	points->coords[3] = dy;
	view_highlight->private->outline =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_line_get_type (),
				       "points", points,
				       "width_pixels", 3,
				       "fill_color_rgba", FG_COLOR,
				       NULL);

	view_highlight->private->p1 =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", -1.0,
				       "y1", -1.0,
				       "x2", +1.0,
				       "y2", +1.0,
				       "fill_color_rgba", FG_COLOR,
				       "outline_color_rgba", BG_COLOR,
				       NULL);

	view_highlight->private->p2 =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", dx - 1.0,
				       "y1", dy - 1.0,
				       "x2", dx + 1.0,
				       "y2", dy + 1.0,
				       "fill_color_rgba", FG_COLOR,
				       "outline_color_rgba", BG_COLOR,
				       NULL);

	g_signal_connect (G_OBJECT (view_highlight->private->p1), "event",
			  G_CALLBACK (p1_resize_event_handler), view_highlight);
	g_signal_connect (G_OBJECT (view_highlight->private->p2), "event",
			  G_CALLBACK (p2_resize_event_handler), view_highlight);

	g_signal_connect (G_OBJECT (view_highlight->private->outline), "event",
			  G_CALLBACK (passthrough_event_handler), view_object);

	gnome_canvas_points_free (points);

	/* send to top, then lower below all items that form the foregound */
	gnome_canvas_item_raise_to_top (group);
	gl_view_raise_fg (view);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create a simple box highlight.                                  */
/*---------------------------------------------------------------------------*/
static void
highlight_simple_construct (glViewHighlight        *view_highlight,
			    glViewObject           *view_object)
{
	gdouble x, y, w, h;
	GnomeCanvasItem *group;
	glView *view;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_highlight && GL_IS_VIEW_HIGHLIGHT (view_highlight));
	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));

	view   = gl_view_object_get_view (view_object);
	view_highlight->private->view = view;

	object = gl_view_object_get_object (view_object);
	view_highlight->private->object = object;

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	gl_label_object_get_position (object, &x, &y);
	gl_label_object_get_size (object, &w, &h);


	view_highlight->private->group =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (view->canvas)),
				       gnome_canvas_group_get_type (),
				       "x", x, "y", y, NULL);
	gnome_canvas_item_hide (view_highlight->private->group);
	group = view_highlight->private->group;

	view_highlight->private->outline =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", -0.5,
				       "y1", -0.5,
				       "x2", w + 0.5,
				       "y2", h + 0.5,
				       "width_pixels", 2,
				       "outline_color_rgba", FG_COLOR,
				       NULL);


	g_signal_connect (G_OBJECT (view_highlight->private->outline), "event",
			  G_CALLBACK (passthrough_event_handler), view_object);

	/* send to top, then lower below all items that form the foregound */
	gnome_canvas_item_raise_to_top (group);
	gl_view_raise_fg (view);

	gl_debug (DEBUG_VIEW, "END");
}

/****************************************************************************/
/* Show highlight.                                                          */
/****************************************************************************/
void
gl_view_highlight_show (glViewHighlight *view_highlight)
{
	gnome_canvas_item_show (view_highlight->private->group);

	gnome_canvas_item_raise_to_top (view_highlight->private->group);
	gl_view_raise_fg (view_highlight->private->view);
}

/****************************************************************************/
/* Hide highlight.                                                          */
/****************************************************************************/
void
gl_view_highlight_hide (glViewHighlight *view_highlight)
{
	gnome_canvas_item_hide (view_highlight->private->group);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Object moved callback.                                          */
/*---------------------------------------------------------------------------*/
static void
object_moved_cb (glLabelObject    *object,
		 gdouble           dx,
		 gdouble           dy,
		 glViewHighlight  *view_highlight)
{
	GnomeCanvasItem    *item, *highlight;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_highlight && GL_IS_VIEW_HIGHLIGHT (view_highlight));
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	/* Adjust location of highlight group */
	gnome_canvas_item_move (view_highlight->private->group, dx, dy);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "changed" callback.                                 */
/*---------------------------------------------------------------------------*/
static void
object_changed_cb (glLabelObject   *object,
		   glViewHighlight *view_highlight)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_highlight && GL_IS_VIEW_HIGHLIGHT (view_highlight));
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	switch (view_highlight->private->style) {

	case GL_VIEW_HIGHLIGHT_BOX_RESIZABLE:
	case GL_VIEW_HIGHLIGHT_ELLIPSE_RESIZABLE:
		update_resizable_box (view_highlight);
		break;

	case GL_VIEW_HIGHLIGHT_LINE_RESIZABLE:
		update_resizable_line (view_highlight);
		break;

	case GL_VIEW_HIGHLIGHT_SIMPLE:
		update_simple (view_highlight);
		break;

	default:
		g_warning ("Invalid resizable highlight style");

	}

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Update a resizable box highlight.                               */
/*---------------------------------------------------------------------------*/
static void
update_resizable_box  (glViewHighlight *view_highlight)
{
	gdouble w, h;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_highlight && GL_IS_VIEW_HIGHLIGHT (view_highlight));

	gl_label_object_get_size (view_highlight->private->object, &w, &h);


	gnome_canvas_item_set (view_highlight->private->outline,
			       "x1", -0.5,
			       "y1", -0.5,
			       "x2", w + 0.5,
			       "y2", h + 0.5,
			       NULL);

	gnome_canvas_item_set (view_highlight->private->tl,
			       "x1", -1.0,
			       "y1", -1.0,
			       "x2", +1.0,
			       "y2", +1.0,
			       NULL);

	gnome_canvas_item_set (view_highlight->private->tr,
			       "x1", w - 1.0,
			       "y1", -1.0,
			       "x2", w + 1.0,
			       "y2", +1.0,
			       NULL);

	gnome_canvas_item_set (view_highlight->private->bl,
			       "x1", -1.0,
			       "y1", h - 1.0,
			       "x2", +1.0,
			       "y2", h + 1.0,
			       NULL);

	gnome_canvas_item_set (view_highlight->private->br,
			       "x1", w - 1.0,
			       "y1", h - 1.0,
			       "x2", w + 1.0,
			       "y2", h + 1.0,
			       NULL);

	gnome_canvas_item_set (view_highlight->private->sl,
			       "x1", -1.0,
			       "y1", h / 2.0 - 1.0,
			       "x2", +1.0,
			       "y2", h / 2.0 + 1.0,
			       NULL);

	gnome_canvas_item_set (view_highlight->private->sr,
			       "x1", w - 1.0,
			       "y1", h / 2.0 - 1.0,
			       "x2", w + 1.0,
			       "y2", h / 2.0 + 1.0,
			       NULL);

	gnome_canvas_item_set (view_highlight->private->st,
			       "x1", w / 2.0 - 1.0,
			       "y1", -1.0,
			       "x2", w / 2.0 + 1.0,
			       "y2", +1.0,
			       NULL);

	gnome_canvas_item_set (view_highlight->private->sb,
			       "x1", w / 2.0 - 1.0,
			       "y1", h - 1.0,
			       "x2", w / 2.0 + 1.0,
			       "y2", h + 1.0,
			       NULL);

	/* send to top, then lower below all items that form the foregound */
	gnome_canvas_item_raise_to_top (view_highlight->private->group);
	gl_view_raise_fg (view_highlight->private->view);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Update a resizable line highlight.                              */
/*---------------------------------------------------------------------------*/
static void
update_resizable_line (glViewHighlight *view_highlight)
{
	gdouble dx, dy;
	GnomeCanvasPoints *points;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_highlight && GL_IS_VIEW_HIGHLIGHT (view_highlight));

	gl_label_object_get_size (view_highlight->private->object, &dx, &dy);

	points = gnome_canvas_points_new (2);

	points->coords[0] = 0.0;
	points->coords[1] = 0.0;
	points->coords[2] = dx;
	points->coords[3] = dy;
	gnome_canvas_item_set (view_highlight->private->outline, "points", points, NULL);

	gnome_canvas_item_set (view_highlight->private->p1,
			       "x1", -1.0,
			       "y1", -1.0,
			       "x2", +1.0,
			       "y2", +1.0,
			       NULL);

	gnome_canvas_item_set (view_highlight->private->p2,
			       "x1", dx - 1.0,
			       "y1", dy - 1.0,
			       "x2", dx + 1.0,
			       "y2", dy + 1.0,
			       NULL);

	gnome_canvas_points_free (points);

	/* send to top, then lower below all items that form the foregound */
	gnome_canvas_item_raise_to_top (view_highlight->private->group);
	gl_view_raise_fg (view_highlight->private->view);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Update a simple box highlight.                                  */
/*---------------------------------------------------------------------------*/
static void
update_simple (glViewHighlight *view_highlight)
{
	gdouble w, h;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_highlight && GL_IS_VIEW_HIGHLIGHT (view_highlight));

	gl_label_object_get_size (view_highlight->private->object, &w, &h);

	gnome_canvas_item_set (view_highlight->private->outline,
			       "x2", w + 0.5,
			       "y2", h + 0.5,
			       NULL);

	/* send to top, then lower below all items that form the foregound */
	gnome_canvas_item_raise_to_top (view_highlight->private->group);
	gl_view_raise_fg (view_highlight->private->view);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Top-left" Resize event handler.                    */
/*---------------------------------------------------------------------------*/
static int
tl_resize_event_handler (GnomeCanvasItem *handle_item,
			 GdkEvent        *event,
			 glViewHighlight *view_highlight)
{
	gdouble x, y, w, h;
	static gdouble x2, y2;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	GdkCursor *cursor;

	if ( view_highlight->private->view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = view_highlight->private->object;;
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
			x = MIN (event->motion.x, x2 - MIN_ITEM_SIZE);
			y = MIN (event->motion.y, y2 - MIN_ITEM_SIZE);
			w = MAX (x2 - event->motion.x, MIN_ITEM_SIZE);
			h = MAX (y2 - event->motion.y, MIN_ITEM_SIZE);
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
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
tr_resize_event_handler (GnomeCanvasItem *handle_item,
			 GdkEvent        *event,
			 glViewHighlight *view_highlight)
{
	gdouble x, y, w, h;
	static gdouble x1, y2;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	GdkCursor *cursor;

	if ( view_highlight->private->view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = view_highlight->private->object;;
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
			y = MIN (event->motion.y, y2 - MIN_ITEM_SIZE);
			w = MAX (event->motion.x - x1, MIN_ITEM_SIZE);
			h = MAX (y2 - event->motion.y, MIN_ITEM_SIZE);
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
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
bl_resize_event_handler (GnomeCanvasItem *handle_item,
			 GdkEvent        *event,
			 glViewHighlight *view_highlight)
{
	gdouble x, y, w, h;
	static gdouble x2, y1;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	GdkCursor *cursor;

	if ( view_highlight->private->view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = view_highlight->private->object;;
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
			x = MIN (event->motion.x, x2 - MIN_ITEM_SIZE);
			y = y1;
			w = MAX (x2 - event->motion.x, MIN_ITEM_SIZE);
			h = MAX (event->motion.y - y1, MIN_ITEM_SIZE);
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
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
br_resize_event_handler (GnomeCanvasItem *handle_item,
			 GdkEvent        *event,
			 glViewHighlight *view_highlight)
{
	gdouble x, y, w, h;
	static gdouble x1, y1;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	GdkCursor *cursor;

	if ( view_highlight->private->view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = view_highlight->private->object;;
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
			w = MAX (event->motion.x - x1, MIN_ITEM_SIZE);
			h = MAX (event->motion.y - y1, MIN_ITEM_SIZE);
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
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
sl_resize_event_handler (GnomeCanvasItem *handle_item,
			 GdkEvent        *event,
			 glViewHighlight *view_highlight)
{
	gdouble x, y, w, h;
	static gdouble x2;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	GdkCursor *cursor;

	if ( view_highlight->private->view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = view_highlight->private->object;;
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
			x = MIN (event->motion.x, x2 - MIN_ITEM_SIZE);
			w = MAX (x2 - event->motion.x, MIN_ITEM_SIZE);
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
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
sr_resize_event_handler (GnomeCanvasItem *handle_item,
			 GdkEvent        *event,
			 glViewHighlight *view_highlight)
{
	gdouble x, y, w, h;
	static gdouble x1;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	GdkCursor *cursor;

	if ( view_highlight->private->view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = view_highlight->private->object;;
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
			w = MAX (event->motion.x - x1, MIN_ITEM_SIZE);
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
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
st_resize_event_handler (GnomeCanvasItem *handle_item,
			 GdkEvent        *event,
			 glViewHighlight *view_highlight)
{
	gdouble x, y, w, h;
	static gdouble y2;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	GdkCursor *cursor;

	if ( view_highlight->private->view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = view_highlight->private->object;;
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
			y = MIN (event->motion.y, y2 - MIN_ITEM_SIZE);
			h = MAX (y2 - event->motion.y, MIN_ITEM_SIZE);
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
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
sb_resize_event_handler (GnomeCanvasItem *handle_item,
			 GdkEvent        *event,
			 glViewHighlight *view_highlight)
{
	gdouble x, y, w, h;
	static gdouble y1;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	GdkCursor *cursor;

	if ( view_highlight->private->view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = view_highlight->private->object;;
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
			h = MAX (event->motion.y - y1, MIN_ITEM_SIZE);
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, w, h);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
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
p1_resize_event_handler (GnomeCanvasItem *handle_item,
			 GdkEvent        *event,
			 glViewHighlight *view_highlight)
{
	gdouble x, y, dx, dy;
	static gdouble x0, y0;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	GdkCursor *cursor;

	if ( view_highlight->private->view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = view_highlight->private->object;;
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
			dx = (event->motion.x - x0);
			dy = (event->motion.y - y0);
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, dx, dy);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
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
p2_resize_event_handler (GnomeCanvasItem *handle_item,
			 GdkEvent        *event,
			 glViewHighlight *view_highlight)
{
	gdouble x, y, dx, dy;
	static gdouble x0, y0;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	GdkCursor *cursor;

	if ( view_highlight->private->view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = view_highlight->private->object;;
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
			dx = (event->motion.x - x0);
			dy = (event->motion.y - y0);
			gl_label_object_set_position (object, x, y);
			gl_label_object_set_size (object, dx, dy);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
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
passthrough_event_handler (GnomeCanvasItem *handle_item,
			   GdkEvent        *event,
			   glViewObject    *view_object)
{
	GnomeCanvasItem *group;

	group = gl_view_object_get_group (view_object);
	return gl_view_item_event_handler (group, event, view_object);
}
