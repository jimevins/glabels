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

#include "view-highlight.h"

#include <math.h>

#include "debug.h"

/*===========================================*/
/* Private constants and macros.             */
/*===========================================*/

#define MIN_ITEM_SIZE 1.0
#define FG_COLOR GNOME_CANVAS_COLOR_A (0, 200, 0, 100)
#define BG_COLOR GNOME_CANVAS_COLOR_A (0, 0, 0, 200)
#define HANDLE_PIXELS 3.0 /* 2*HANDLE_PIXELS = handle size */

/*===========================================*/
/* Private data types                        */
/*===========================================*/

struct _glViewHighlightPrivate {
	glViewHighlightStyle     style;
	glLabelObject           *object;
	glView                  *view;

	GnomeCanvasItem         *position_group;
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

static void   flip_rotate_object_cb              (glLabelObject          *object,
						  glViewHighlight        *view_highlight);

static void   object_changed_cb                  (glLabelObject          *object,
						  glViewHighlight        *view_highlight);

static void   view_scale_changed_cb              (glView                 *view,
						  gdouble                 scale,
						  glViewHighlight        *view_highlight);


static void   update_resizable_box               (glViewHighlight        *view_highlight);

static void   update_resizable_line              (glViewHighlight        *view_highlight);

static void   update_simple                      (glViewHighlight        *view_highlight);


static void   get_origin_and_corners             (glViewHighlight        *view_highlight,
						  gdouble                *x0,
						  gdouble                *y0,
						  gdouble                *x1,
						  gdouble                *y1,
						  gdouble                *x2,
						  gdouble                *y2);

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
		static const GTypeInfo info = {
			sizeof (glViewHighlightClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_view_highlight_class_init,
			NULL,
			NULL,
			sizeof (glViewHighlight),
			0,
			(GInstanceInitFunc) gl_view_highlight_instance_init,
			NULL
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
	glLabel         *parent;
	glViewHighlight *view_highlight;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (object && GL_IS_VIEW_HIGHLIGHT (object));

	view_highlight = GL_VIEW_HIGHLIGHT (object);

	/* Free up handler owned by view. */
	g_signal_handlers_disconnect_by_func (G_OBJECT (view_highlight->private->view),
					      G_CALLBACK (view_scale_changed_cb),
					      object);

	gtk_object_destroy (GTK_OBJECT(view_highlight->private->position_group));

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
	glView          *view;
	glLabelObject   *object;
	gdouble          x, y;
	gdouble          affine[6];

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT(view_object));

	view   = gl_view_object_get_view (view_object);
	object = gl_view_object_get_object (view_object);
	gl_label_object_get_position (object, &x, &y);

	view_highlight = g_object_new (gl_view_highlight_get_type (), NULL);

	view_highlight->private->style          = style;
	view_highlight->private->view           = view;
	view_highlight->private->object         = object;
	view_highlight->private->position_group =
		gnome_canvas_item_new (view->highlight_group,
				       gnome_canvas_group_get_type (),
				       "x", x,
				       "y", y,
				       NULL);

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


	gl_label_object_get_affine (view_highlight->private->object, affine);
	gnome_canvas_item_affine_absolute (view_highlight->private->group, affine);

	g_signal_connect (G_OBJECT (view_highlight->private->object), "moved",
			  G_CALLBACK (object_moved_cb), view_highlight);

	g_signal_connect (G_OBJECT (view_highlight->private->object), "changed",
			  G_CALLBACK (object_changed_cb), view_highlight);

	g_signal_connect (G_OBJECT (view_highlight->private->object), "flip_rotate",
			  G_CALLBACK (flip_rotate_object_cb), view_highlight);

	g_signal_connect (G_OBJECT (view_highlight->private->view), "zoom_changed",
			  G_CALLBACK (view_scale_changed_cb), view_highlight);

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
	gdouble           w, h;
	GnomeCanvasItem  *group;
	GnomeCanvasGroup *position_group;
	glView           *view;
	glLabelObject    *object;
	gdouble           scale;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_highlight && GL_IS_VIEW_HIGHLIGHT (view_highlight));
	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));

	view   = view_highlight->private->view;
	scale = view->zoom * view->home_scale;

	object = view_highlight->private->object;

	position_group = GNOME_CANVAS_GROUP(view_highlight->private->position_group);

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	gl_label_object_get_size (object, &w, &h);

	view_highlight->private->group =
		gnome_canvas_item_new (position_group,
				       gnome_canvas_group_get_type (),
				       "x", 0.0, "y", 0.0, NULL);
	gnome_canvas_item_hide (view_highlight->private->group);
	group = view_highlight->private->group;

#ifdef SHOW_OUTLINE
	switch (style) {
	case GL_VIEW_HIGHLIGHT_BOX_RESIZABLE:
		view_highlight->private->outline =
			gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
					       gnome_canvas_rect_get_type (),
					       "x1", -0.5,
					       "y1", -0.5,
					       "x2", w + 0.5,
					       "y2", h + 0.5,
					       "width_pixels", 3,
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
					       "width_pixels", 3,
					       "outline_color_rgba", FG_COLOR,
					       NULL);
		break;
	default:
		view_highlight->private->outline = NULL;
		g_warning ("Invalid resizable highlight style");
	}
#endif

	view_highlight->private->tl =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", -HANDLE_PIXELS/scale,
				       "y1", -HANDLE_PIXELS/scale,
				       "x2", +HANDLE_PIXELS/scale,
				       "y2", +HANDLE_PIXELS/scale,
				       "width_pixels", 1,
				       "fill_color_rgba", FG_COLOR,
				       "outline_color_rgba", BG_COLOR,
				       NULL);

	view_highlight->private->tr =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", w - HANDLE_PIXELS/scale,
				       "y1", -HANDLE_PIXELS/scale,
				       "x2", w + HANDLE_PIXELS/scale,
				       "y2", +HANDLE_PIXELS/scale,
				       "width_pixels", 1,
				       "fill_color_rgba", FG_COLOR,
				       "outline_color_rgba", BG_COLOR,
				       NULL);

	view_highlight->private->bl =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", -HANDLE_PIXELS/scale,
				       "y1", h - HANDLE_PIXELS/scale,
				       "x2", +HANDLE_PIXELS/scale,
				       "y2", h + HANDLE_PIXELS/scale,
				       "width_pixels", 1,
				       "fill_color_rgba", FG_COLOR,
				       "outline_color_rgba", BG_COLOR,
				       NULL);

	view_highlight->private->br =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", w - HANDLE_PIXELS/scale,
				       "y1", h - HANDLE_PIXELS/scale,
				       "x2", w + HANDLE_PIXELS/scale,
				       "y2", h + HANDLE_PIXELS/scale,
				       "width_pixels", 1,
				       "fill_color_rgba", FG_COLOR,
				       "outline_color_rgba", BG_COLOR,
				       NULL);

	view_highlight->private->sl =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", -HANDLE_PIXELS/scale,
				       "y1", h / 2.0 - HANDLE_PIXELS/scale,
				       "x2", +HANDLE_PIXELS/scale,
				       "y2", h / 2.0 + HANDLE_PIXELS/scale,
				       "width_pixels", 1,
				       "fill_color_rgba", FG_COLOR,
				       "outline_color_rgba", BG_COLOR,
				       NULL);

	view_highlight->private->sr =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", w - HANDLE_PIXELS/scale,
				       "y1", h / 2.0 - HANDLE_PIXELS/scale,
				       "x2", w + HANDLE_PIXELS/scale,
				       "y2", h / 2.0 + HANDLE_PIXELS/scale,
				       "width_pixels", 1,
				       "fill_color_rgba", FG_COLOR,
				       "outline_color_rgba", BG_COLOR,
				       NULL);

	view_highlight->private->st =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", w / 2.0 - HANDLE_PIXELS/scale,
				       "y1", -HANDLE_PIXELS/scale,
				       "x2", w / 2.0 + HANDLE_PIXELS/scale,
				       "y2", +HANDLE_PIXELS/scale,
				       "width_pixels", 1,
				       "fill_color_rgba", FG_COLOR,
				       "outline_color_rgba", BG_COLOR,
				       NULL);

	view_highlight->private->sb =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", w / 2.0 - HANDLE_PIXELS/scale,
				       "y1", h - HANDLE_PIXELS/scale,
				       "x2", w / 2.0 + HANDLE_PIXELS/scale,
				       "y2", h + HANDLE_PIXELS/scale,
				       "width_pixels", 1,
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

#ifdef SHOW_OUTLINE
	g_signal_connect (G_OBJECT (view_highlight->private->outline), "event",
			  G_CALLBACK (passthrough_event_handler), view_object);
#endif

	gnome_canvas_item_raise_to_top (group);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create a resizable line highlight.                              */
/*---------------------------------------------------------------------------*/
static void
highlight_resizable_line_construct (glViewHighlight *view_highlight,
				    glViewObject    *view_object)
{
	gdouble            dx, dy;
	GnomeCanvasItem   *group;
	GnomeCanvasGroup  *position_group;
#ifdef SHOW_OUTLINE
	GnomeCanvasPoints *points;
#endif
	glView            *view;
	glLabelObject     *object;
	gdouble            scale;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_highlight && GL_IS_VIEW_HIGHLIGHT (view_highlight));
	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));

	view   = view_highlight->private->view;
	scale = view->zoom * view->home_scale;

	object = view_highlight->private->object;

	position_group = GNOME_CANVAS_GROUP(view_highlight->private->position_group);

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	gl_label_object_get_size (object, &dx, &dy);

#ifdef SHOW_OUTLINE
	points = gnome_canvas_points_new (2);
#endif

	view_highlight->private->group =
		gnome_canvas_item_new (position_group,
				       gnome_canvas_group_get_type (),
				       "x", 0.0, "y", 0.0, NULL);
	gnome_canvas_item_hide (view_highlight->private->group);
	group = view_highlight->private->group;

#ifdef SHOW_OUTLINE
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
#endif

	view_highlight->private->p1 =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", -HANDLE_PIXELS/scale,
				       "y1", -HANDLE_PIXELS/scale,
				       "x2", +HANDLE_PIXELS/scale,
				       "y2", +HANDLE_PIXELS/scale,
				       "width_pixels", 1,
				       "fill_color_rgba", FG_COLOR,
				       "outline_color_rgba", BG_COLOR,
				       NULL);

	view_highlight->private->p2 =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", dx - HANDLE_PIXELS/scale,
				       "y1", dy - HANDLE_PIXELS/scale,
				       "x2", dx + HANDLE_PIXELS/scale,
				       "y2", dy + HANDLE_PIXELS/scale,
				       "width_pixels", 1,
				       "fill_color_rgba", FG_COLOR,
				       "outline_color_rgba", BG_COLOR,
				       NULL);

	g_signal_connect (G_OBJECT (view_highlight->private->p1), "event",
			  G_CALLBACK (p1_resize_event_handler), view_highlight);
	g_signal_connect (G_OBJECT (view_highlight->private->p2), "event",
			  G_CALLBACK (p2_resize_event_handler), view_highlight);

#ifdef SHOW_OUTLINE
	g_signal_connect (G_OBJECT (view_highlight->private->outline), "event",
			  G_CALLBACK (passthrough_event_handler), view_object);
#endif

#ifdef SHOW_OUTLINE
	gnome_canvas_points_free (points);
#endif

	gnome_canvas_item_raise_to_top (group);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create a simple box highlight.                                  */
/*---------------------------------------------------------------------------*/
static void
highlight_simple_construct (glViewHighlight *view_highlight,
			    glViewObject    *view_object)
{
	gdouble           w, h;
	GnomeCanvasItem  *group;
	GnomeCanvasGroup *position_group;
	glView           *view;
	glLabelObject    *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_highlight && GL_IS_VIEW_HIGHLIGHT (view_highlight));
	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));

	view   = view_highlight->private->view;

	object = view_highlight->private->object;

	position_group = GNOME_CANVAS_GROUP(view_highlight->private->position_group);

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

	gl_label_object_get_size (object, &w, &h);


	view_highlight->private->group =
		gnome_canvas_item_new (position_group,
				       gnome_canvas_group_get_type (),
				       "x", 0.0, "y", 0.0, NULL);
	gnome_canvas_item_hide (view_highlight->private->group);
	group = view_highlight->private->group;

	view_highlight->private->outline =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", -0.5,
				       "y1", -0.5,
				       "x2", w + 0.5,
				       "y2", h + 0.5,
				       "width_pixels", 3,
				       "outline_color_rgba", FG_COLOR,
				       NULL);


	g_signal_connect (G_OBJECT (view_highlight->private->outline), "event",
			  G_CALLBACK (passthrough_event_handler), view_object);

	gnome_canvas_item_raise_to_top (group);

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
/* PRIVATE.  label object "moved" callback.                                  */
/*---------------------------------------------------------------------------*/
static void
object_moved_cb (glLabelObject   *object,
		 gdouble          dx,
		 gdouble          dy,
		 glViewHighlight *view_highlight)
{
	gl_debug (DEBUG_VIEW, "START");

	/* Adjust location of outer canvas group. */
	gnome_canvas_item_move (view_highlight->private->position_group, dx, dy);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Flip/rotate object callback.                                    */
/*---------------------------------------------------------------------------*/
static void
flip_rotate_object_cb (glLabelObject    *object,
		       glViewHighlight  *view_highlight)
{
	gdouble          affine[6];

	gl_debug (DEBUG_VIEW, "START");

	/* Adjust affine of inner canvas group. */
	gl_label_object_get_affine (object, affine);
	gnome_canvas_item_affine_absolute (view_highlight->private->group, affine);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "changed" callback.                                 */
/*---------------------------------------------------------------------------*/
static void
object_changed_cb (glLabelObject   *object,
		   glViewHighlight *view_highlight)
{
	gdouble          affine[6];

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
/* PRIVATE. view scale (zoom) changed callback.                              */
/*---------------------------------------------------------------------------*/
static void
view_scale_changed_cb (glView          *view,
		       gdouble          scale,
		       glViewHighlight *view_highlight)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_highlight && GL_IS_VIEW_HIGHLIGHT (view_highlight));
	g_return_if_fail (view && GL_IS_VIEW (view));

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
	gdouble scale;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_highlight && GL_IS_VIEW_HIGHLIGHT (view_highlight));

	scale = view_highlight->private->view->zoom *
		view_highlight->private->view->home_scale;

	gl_label_object_get_size (view_highlight->private->object, &w, &h);


#ifdef SHOW_OUTLINE
	gnome_canvas_item_set (view_highlight->private->outline,
			       "x1", -0.5,
			       "y1", -0.5,
			       "x2", w + 0.5,
			       "y2", h + 0.5,
			       NULL);
#endif

	gnome_canvas_item_set (view_highlight->private->tl,
			       "x1", -HANDLE_PIXELS/scale,
			       "y1", -HANDLE_PIXELS/scale,
			       "x2", +HANDLE_PIXELS/scale,
			       "y2", +HANDLE_PIXELS/scale,
			       NULL);

	gnome_canvas_item_set (view_highlight->private->tr,
			       "x1", w - HANDLE_PIXELS/scale,
			       "y1", -HANDLE_PIXELS/scale,
			       "x2", w + HANDLE_PIXELS/scale,
			       "y2", +HANDLE_PIXELS/scale,
			       NULL);

	gnome_canvas_item_set (view_highlight->private->bl,
			       "x1", -HANDLE_PIXELS/scale,
			       "y1", h - HANDLE_PIXELS/scale,
			       "x2", +HANDLE_PIXELS/scale,
			       "y2", h + HANDLE_PIXELS/scale,
			       NULL);

	gnome_canvas_item_set (view_highlight->private->br,
			       "x1", w - HANDLE_PIXELS/scale,
			       "y1", h - HANDLE_PIXELS/scale,
			       "x2", w + HANDLE_PIXELS/scale,
			       "y2", h + HANDLE_PIXELS/scale,
			       NULL);

	gnome_canvas_item_set (view_highlight->private->sl,
			       "x1", -HANDLE_PIXELS/scale,
			       "y1", h / 2.0 - HANDLE_PIXELS/scale,
			       "x2", +HANDLE_PIXELS/scale,
			       "y2", h / 2.0 + HANDLE_PIXELS/scale,
			       NULL);

	gnome_canvas_item_set (view_highlight->private->sr,
			       "x1", w - HANDLE_PIXELS/scale,
			       "y1", h / 2.0 - HANDLE_PIXELS/scale,
			       "x2", w + HANDLE_PIXELS/scale,
			       "y2", h / 2.0 + HANDLE_PIXELS/scale,
			       NULL);

	gnome_canvas_item_set (view_highlight->private->st,
			       "x1", w / 2.0 - HANDLE_PIXELS/scale,
			       "y1", -HANDLE_PIXELS/scale,
			       "x2", w / 2.0 + HANDLE_PIXELS/scale,
			       "y2", +HANDLE_PIXELS/scale,
			       NULL);

	gnome_canvas_item_set (view_highlight->private->sb,
			       "x1", w / 2.0 - HANDLE_PIXELS/scale,
			       "y1", h - HANDLE_PIXELS/scale,
			       "x2", w / 2.0 + HANDLE_PIXELS/scale,
			       "y2", h + HANDLE_PIXELS/scale,
			       NULL);

	gnome_canvas_item_raise_to_top (view_highlight->private->group);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Update a resizable line highlight.                              */
/*---------------------------------------------------------------------------*/
static void
update_resizable_line (glViewHighlight *view_highlight)
{
	gdouble dx, dy;
#ifdef SHOW_OUTLINE
	GnomeCanvasPoints *points;
#endif
	gdouble scale;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_highlight && GL_IS_VIEW_HIGHLIGHT (view_highlight));

	scale = view_highlight->private->view->zoom *
		view_highlight->private->view->home_scale;

	gl_label_object_get_size (view_highlight->private->object, &dx, &dy);

#ifdef SHOW_OUTLINE
	points = gnome_canvas_points_new (2);

	points->coords[0] = 0.0;
	points->coords[1] = 0.0;
	points->coords[2] = dx;
	points->coords[3] = dy;
	gnome_canvas_item_set (view_highlight->private->outline, "points", points, NULL);
#endif

	gnome_canvas_item_set (view_highlight->private->p1,
			       "x1", -HANDLE_PIXELS/scale,
			       "y1", -HANDLE_PIXELS/scale,
			       "x2", +HANDLE_PIXELS/scale,
			       "y2", +HANDLE_PIXELS/scale,
			       NULL);

	gnome_canvas_item_set (view_highlight->private->p2,
			       "x1", dx - HANDLE_PIXELS/scale,
			       "y1", dy - HANDLE_PIXELS/scale,
			       "x2", dx + HANDLE_PIXELS/scale,
			       "y2", dy + HANDLE_PIXELS/scale,
			       NULL);

#ifdef SHOW_OUTLINE
	gnome_canvas_points_free (points);
#endif

	gnome_canvas_item_raise_to_top (view_highlight->private->group);

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

	gnome_canvas_item_raise_to_top (view_highlight->private->group);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Get origin and corners relative to object.                      */
/*---------------------------------------------------------------------------*/
static void
get_origin_and_corners (glViewHighlight *view_highlight,
			gdouble         *x0,
			gdouble         *y0,
			gdouble         *x1,
			gdouble         *y1,
			gdouble         *x2,
			gdouble         *y2)
{
	glLabelObject *object;

	object = view_highlight->private->object;;

	/* origin, relative to item */
	gl_label_object_get_position (object, x0, y0);
	gnome_canvas_item_w2i (view_highlight->private->group, x0, y0);

	/* Top left corner, relative to item */
	*x1 = 0.0;
	*y1 = 0.0;

	/* Bottom right corner, relative to item */
	gl_label_object_get_size (object, x2, y2);
}
		     

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Top-left" Resize event handler.                                */
/*---------------------------------------------------------------------------*/
static int
tl_resize_event_handler (GnomeCanvasItem *handle_item,
			 GdkEvent        *event,
			 glViewHighlight *view_highlight)
{
	gdouble          x0, y0, w, h;
	gdouble          x1, y1, x2, y2;
	static gboolean  dragging = FALSE;
	static gboolean  is_control_pressed;
	glLabelObject   *object;
	GdkCursor       *cursor;

	if ( view_highlight->private->view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = view_highlight->private->object;;

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			is_control_pressed = event->button.state & GDK_CONTROL_MASK;
			dragging = TRUE;
			gnome_canvas_item_grab (handle_item,
						GDK_POINTER_MOTION_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_BUTTON_PRESS_MASK,
						NULL, event->button.time);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			get_origin_and_corners (view_highlight,
						&x0, &y0, &x1, &y1, &x2, &y2);
			gnome_canvas_item_ungrab (handle_item,
						  event->button.time);
			gnome_canvas_item_w2i (view_highlight->private->group,
					       &event->button.x, &event->button.y);
			w = MAX (x2 - event->button.x, MIN_ITEM_SIZE);
			h = MAX (y2 - event->button.y, MIN_ITEM_SIZE);
			if (is_control_pressed) {
				gl_label_object_set_size_honor_aspect (object, w, h);
			} else {
				gl_label_object_set_size (object, w, h);
			}
			gl_label_object_get_size (object, &w, &h);
			x1 = x2 - w;
			y1 = y2 - h;
			x0 = x0 + x1;
			y0 = y0 + y1;
			gnome_canvas_item_i2w (view_highlight->private->group, &x0, &y0);
			gl_label_object_set_position (object, x0, y0);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			get_origin_and_corners (view_highlight,
						&x0, &y0, &x1, &y1, &x2, &y2);
			gnome_canvas_item_w2i (view_highlight->private->group,
					       &event->button.x, &event->button.y);
			w = MAX (x2 - event->motion.x, MIN_ITEM_SIZE);
			h = MAX (y2 - event->motion.y, MIN_ITEM_SIZE);
			if (is_control_pressed) {
				gl_label_object_set_size_honor_aspect (object, w, h);
			} else {
				gl_label_object_set_size (object, w, h);
			}
			gl_label_object_get_size (object, &w, &h);
			x1 = x2 - w;
			y1 = y2 - h;
			x0 = x0 + x1;
			y0 = y0 + y1;
			gnome_canvas_item_i2w (view_highlight->private->group, &x0, &y0);
			gl_label_object_set_position (object, x0, y0);
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
	gdouble          x0, y0, w, h;
	gdouble          x1, y1, x2, y2;
	static gboolean  dragging = FALSE;
	static gboolean  is_control_pressed;
	glLabelObject   *object;
	GdkCursor       *cursor;

	if ( view_highlight->private->view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = view_highlight->private->object;;

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			is_control_pressed = event->button.state & GDK_CONTROL_MASK;
			dragging = TRUE;
			gnome_canvas_item_grab (handle_item,
						GDK_POINTER_MOTION_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_BUTTON_PRESS_MASK,
						NULL, event->button.time);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			get_origin_and_corners (view_highlight,
						&x0, &y0, &x1, &y1, &x2, &y2);
			gnome_canvas_item_ungrab (handle_item,
						  event->button.time);
			gnome_canvas_item_w2i (view_highlight->private->group,
					       &event->button.x, &event->button.y);
			w = MAX (event->button.x - x1, MIN_ITEM_SIZE);
			h = MAX (y2 - event->button.y, MIN_ITEM_SIZE);
			if (is_control_pressed) {
				gl_label_object_set_size_honor_aspect (object, w, h);
			} else {
				gl_label_object_set_size (object, w, h);
			}
			gl_label_object_get_size (object, &w, &h);
			/* x1 unchanged */
			y1 = y2 - h;
			x0 = x0 + x1;
			y0 = y0 + y1;
			gnome_canvas_item_i2w (view_highlight->private->group, &x0, &y0);
			gl_label_object_set_position (object, x0, y0);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			get_origin_and_corners (view_highlight,
						&x0, &y0, &x1, &y1, &x2, &y2);
			gnome_canvas_item_w2i (view_highlight->private->group,
					       &event->button.x, &event->button.y);
			w = MAX (event->button.x - x1, MIN_ITEM_SIZE);
			h = MAX (y2 - event->button.y, MIN_ITEM_SIZE);
			if (is_control_pressed) {
				gl_label_object_set_size_honor_aspect (object, w, h);
			} else {
				gl_label_object_set_size (object, w, h);
			}
			gl_label_object_get_size (object, &w, &h);
			/* x1 unchanged */
			y1 = y2 - h;
			x0 = x0 + x1;
			y0 = y0 + y1;
			gnome_canvas_item_i2w (view_highlight->private->group, &x0, &y0);
			gl_label_object_set_position (object, x0, y0);
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
	gdouble          x0, y0, w, h;
	gdouble          x1, y1, x2, y2;
	static gboolean  dragging = FALSE;
	static gboolean  is_control_pressed;
	glLabelObject   *object;
	GdkCursor       *cursor;

	if ( view_highlight->private->view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = view_highlight->private->object;;

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			is_control_pressed = event->button.state & GDK_CONTROL_MASK;
			dragging = TRUE;
			gnome_canvas_item_grab (handle_item,
						GDK_POINTER_MOTION_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_BUTTON_PRESS_MASK,
						NULL, event->button.time);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			get_origin_and_corners (view_highlight,
						&x0, &y0, &x1, &y1, &x2, &y2);
			gnome_canvas_item_ungrab (handle_item,
						  event->button.time);
			gnome_canvas_item_w2i (view_highlight->private->group,
					       &event->button.x, &event->button.y);
			w = MAX (x2 - event->button.x, MIN_ITEM_SIZE);
			h = MAX (event->button.y - x1, MIN_ITEM_SIZE);
			if (is_control_pressed) {
				gl_label_object_set_size_honor_aspect (object, w, h);
			} else {
				gl_label_object_set_size (object, w, h);
			}
			gl_label_object_get_size (object, &w, &h);
			x1 = x2 - w;
			/* y1 unchanged */
			x0 = x0 + x1;
			y0 = y0 + y1;
			gnome_canvas_item_i2w (view_highlight->private->group, &x0, &y0);
			gl_label_object_set_position (object, x0, y0);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			get_origin_and_corners (view_highlight,
						&x0, &y0, &x1, &y1, &x2, &y2);
			gnome_canvas_item_w2i (view_highlight->private->group,
					       &event->button.x, &event->button.y);
			w = MAX (x2 - event->button.x, MIN_ITEM_SIZE);
			h = MAX (event->button.y - x1, MIN_ITEM_SIZE);
			if (is_control_pressed) {
				gl_label_object_set_size_honor_aspect (object, w, h);
			} else {
				gl_label_object_set_size (object, w, h);
			}
			gl_label_object_get_size (object, &w, &h);
			x1 = x2 - w;
			/* y1 unchanged */
			x0 = x0 + x1;
			y0 = y0 + y1;
			gnome_canvas_item_i2w (view_highlight->private->group, &x0, &y0);
			gl_label_object_set_position (object, x0, y0);
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
	gdouble          x0, y0, w, h;
	gdouble          x1, y1, x2, y2;
	static gboolean  dragging = FALSE;
	static gboolean  is_control_pressed;
	glLabelObject   *object;
	GdkCursor       *cursor;

	if ( view_highlight->private->view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = view_highlight->private->object;;

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		gl_debug (DEBUG_VIEW, "BUTTON_PRESS");
		switch (event->button.button) {
		case 1:
			is_control_pressed = event->button.state & GDK_CONTROL_MASK;
			dragging = TRUE;
			gnome_canvas_item_grab (handle_item,
						GDK_POINTER_MOTION_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_BUTTON_PRESS_MASK,
						NULL, event->button.time);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		gl_debug (DEBUG_VIEW, "BUTTON_RELEASE");
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			get_origin_and_corners (view_highlight,
						&x0, &y0, &x1, &y1, &x2, &y2);
			gnome_canvas_item_ungrab (handle_item,
						  event->button.time);
			gnome_canvas_item_w2i (view_highlight->private->group,
					       &event->button.x, &event->button.y);
			w = MAX (event->button.x - x1, MIN_ITEM_SIZE);
			h = MAX (event->button.y - x1, MIN_ITEM_SIZE);
			if (is_control_pressed) {
				gl_label_object_set_size_honor_aspect (object, w, h);
			} else {
				gl_label_object_set_size (object, w, h);
			}
			/* x1 unchanged */
			/* y1 unchanged */
			x0 = x0 + x1;
			y0 = y0 + y1;
			gnome_canvas_item_i2w (view_highlight->private->group, &x0, &y0);
			gl_label_object_set_position (object, x0, y0);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		gl_debug (DEBUG_VIEW, "MOTION_NOTIFY");
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			get_origin_and_corners (view_highlight,
						&x0, &y0, &x1, &y1, &x2, &y2);
			gnome_canvas_item_w2i (view_highlight->private->group,
					       &event->button.x, &event->button.y);
			w = MAX (event->button.x - x1, MIN_ITEM_SIZE);
			h = MAX (event->button.y - x1, MIN_ITEM_SIZE);
			if (is_control_pressed) {
				gl_label_object_set_size_honor_aspect (object, w, h);
			} else {
				gl_label_object_set_size (object, w, h);
			}
			x0 = x0 + x1;
			y0 = y0 + y1;
			/* x1 unchanged */
			/* y1 unchanged */
			gnome_canvas_item_i2w (view_highlight->private->group, &x0, &y0);
			gl_label_object_set_position (object, x0, y0);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		gl_debug (DEBUG_VIEW, "ENTER_NOTIFY");
		cursor = gdk_cursor_new (GDK_CROSSHAIR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		gl_debug (DEBUG_VIEW, "LEAVE_NOTIFY");
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view_highlight->private->view->canvas->window,
				       cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	default:
		gl_debug (DEBUG_VIEW, "default");
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
	gdouble x0, y0, w, h;
	gdouble x1, y1, x2, y2;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	GdkCursor *cursor;

	if ( view_highlight->private->view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = view_highlight->private->object;;

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
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			get_origin_and_corners (view_highlight,
						&x0, &y0, &x1, &y1, &x2, &y2);
			gnome_canvas_item_ungrab (handle_item,
						  event->button.time);
			gnome_canvas_item_w2i (view_highlight->private->group,
					       &event->button.x, &event->button.y);
			w = MAX (x2 - event->button.x, MIN_ITEM_SIZE);
			h = y2 - y1;
			gl_label_object_set_size (object, w, h);
			gl_label_object_get_size (object, &w, &h);
			x1 = x2 - w;
			/* y1 unchanged */
			x0 = x0 + x1;
			y0 = y0 + y1;
			gnome_canvas_item_i2w (view_highlight->private->group, &x0, &y0);
			gl_label_object_set_position (object, x0, y0);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			get_origin_and_corners (view_highlight,
						&x0, &y0, &x1, &y1, &x2, &y2);
			gnome_canvas_item_w2i (view_highlight->private->group,
					       &event->button.x, &event->button.y);
			w = MAX (x2 - event->button.x, MIN_ITEM_SIZE);
			h = y2 - y1;
			gl_label_object_set_size (object, w, h);
			gl_label_object_get_size (object, &w, &h);
			x1 = x2 - w;
			/* y1 unchanged */
			x0 = x0 + x1;
			y0 = y0 + y1;
			gnome_canvas_item_i2w (view_highlight->private->group, &x0, &y0);
			gl_label_object_set_position (object, x0, y0);
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
	gdouble x0, y0, w, h;
	gdouble x1, y1, x2, y2;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	GdkCursor *cursor;

	if ( view_highlight->private->view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = view_highlight->private->object;;

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
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			get_origin_and_corners (view_highlight,
						&x0, &y0, &x1, &y1, &x2, &y2);
			gnome_canvas_item_ungrab (handle_item,
						  event->button.time);
			gnome_canvas_item_w2i (view_highlight->private->group,
					       &event->button.x, &event->button.y);
			w = MAX (event->button.x - x1, MIN_ITEM_SIZE);
			h = y2 - y1;
			gl_label_object_set_size (object, w, h);
			gl_label_object_get_size (object, &w, &h);
			/* x1 unchanged */
			/* y1 unchanged */
			x0 = x0 + x1;
			y0 = y0 + y1;
			gnome_canvas_item_i2w (view_highlight->private->group, &x0, &y0);
			gl_label_object_set_position (object, x0, y0);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			get_origin_and_corners (view_highlight,
						&x0, &y0, &x1, &y1, &x2, &y2);
			gnome_canvas_item_w2i (view_highlight->private->group,
					       &event->button.x, &event->button.y);
			w = MAX (event->button.x - x1, MIN_ITEM_SIZE);
			h = y2 - y1;
			gl_label_object_set_size (object, w, h);
			gl_label_object_get_size (object, &w, &h);
			/* x1 unchanged */
			/* y1 unchanged */
			x0 = x0 + x1;
			y0 = y0 + y1;
			gnome_canvas_item_i2w (view_highlight->private->group, &x0, &y0);
			gl_label_object_set_position (object, x0, y0);
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
	gdouble x0, y0, w, h;
	gdouble x1, y1, x2, y2;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	GdkCursor *cursor;

	if ( view_highlight->private->view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = view_highlight->private->object;;

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
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			get_origin_and_corners (view_highlight,
						&x0, &y0, &x1, &y1, &x2, &y2);
			gnome_canvas_item_ungrab (handle_item,
						  event->button.time);
			gnome_canvas_item_w2i (view_highlight->private->group,
					       &event->button.x, &event->button.y);
			w = x2 - x1;
			h = MAX (y2 - event->button.y, MIN_ITEM_SIZE);
			gl_label_object_set_size (object, w, h);
			gl_label_object_get_size (object, &w, &h);
			/* x1 unchanged */
			y1 = y2 - h;
			x0 = x0 + x1;
			y0 = y0 + y1;
			gnome_canvas_item_i2w (view_highlight->private->group, &x0, &y0);
			gl_label_object_set_position (object, x0, y0);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			get_origin_and_corners (view_highlight,
						&x0, &y0, &x1, &y1, &x2, &y2);
			gnome_canvas_item_w2i (view_highlight->private->group,
					       &event->button.x, &event->button.y);
			w = x2 - x1;
			h = MAX (y2 - event->button.y, MIN_ITEM_SIZE);
			gl_label_object_set_size (object, w, h);
			gl_label_object_get_size (object, &w, &h);
			/* x1 unchanged */
			y1 = y2 - h;
			x0 = x0 + x1;
			y0 = y0 + y1;
			gnome_canvas_item_i2w (view_highlight->private->group, &x0, &y0);
			gl_label_object_set_position (object, x0, y0);
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
	gdouble x0, y0, w, h;
	gdouble x1, y1, x2, y2;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	GdkCursor *cursor;

	if ( view_highlight->private->view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = view_highlight->private->object;;

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
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			get_origin_and_corners (view_highlight,
						&x0, &y0, &x1, &y1, &x2, &y2);
			gnome_canvas_item_ungrab (handle_item,
						  event->button.time);
			gnome_canvas_item_w2i (view_highlight->private->group,
					       &event->button.x, &event->button.y);
			w = x2 - x1;
			h = MAX (event->button.y - x1, MIN_ITEM_SIZE);
			gl_label_object_set_size (object, w, h);
			gl_label_object_get_size (object, &w, &h);
			/* x1 unchanged */
			/* y1 unchanged */
			x0 = x0 + x1;
			y0 = y0 + y1;
			gnome_canvas_item_i2w (view_highlight->private->group, &x0, &y0);
			gl_label_object_set_position (object, x0, y0);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			get_origin_and_corners (view_highlight,
						&x0, &y0, &x1, &y1, &x2, &y2);
			gnome_canvas_item_w2i (view_highlight->private->group,
					       &event->button.x, &event->button.y);
			w = x2 - x1;
			h = MAX (event->button.y - x1, MIN_ITEM_SIZE);
			gl_label_object_set_size (object, w, h);
			gl_label_object_get_size (object, &w, &h);
			/* x1 unchanged */
			/* y1 unchanged */
			x0 = x0 + x1;
			y0 = y0 + y1;
			gnome_canvas_item_i2w (view_highlight->private->group, &x0, &y0);
			gl_label_object_set_position (object, x0, y0);
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
	gdouble x0, y0, dx, dy;
	gdouble x1, y1, x2, y2;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	GdkCursor *cursor;

	if ( view_highlight->private->view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = view_highlight->private->object;;

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
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			get_origin_and_corners (view_highlight,
						&x0, &y0, &x1, &y1, &x2, &y2);
			gnome_canvas_item_ungrab (handle_item,
						  event->button.time);
			gnome_canvas_item_w2i (view_highlight->private->group,
					       &event->button.x, &event->button.y);
			x1 = event->button.x;
			y1 = event->button.y;
			dx = (x2 - event->button.x);
			dy = (y2 - event->button.y);
			x0 = x0 + x1;
			y0 = y0 + y1;
			gnome_canvas_item_i2w (view_highlight->private->group, &x0, &y0);
			gl_label_object_set_position (object, x0, y0);
			gl_label_object_set_size (object, dx, dy);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			get_origin_and_corners (view_highlight,
						&x0, &y0, &x1, &y1, &x2, &y2);
			gnome_canvas_item_w2i (view_highlight->private->group,
					       &event->button.x, &event->button.y);
			x1 = event->button.x;
			y1 = event->button.y;
			dx = (x2 - event->button.x);
			dy = (y2 - event->button.y);
			x0 = x0 + x1;
			y0 = y0 + y1;
			gnome_canvas_item_i2w (view_highlight->private->group, &x0, &y0);
			gl_label_object_set_position (object, x0, y0);
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
	gdouble x0, y0, dx, dy;
	gdouble x1, y1, x2, y2;
	static gboolean dragging = FALSE;
	glLabelObject *object;
	GdkCursor *cursor;

	if ( view_highlight->private->view->state != GL_VIEW_STATE_ARROW ) {
		/* don't interfere with object creation modes */
		return FALSE;
	}

	object = view_highlight->private->object;;

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
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			get_origin_and_corners (view_highlight,
						&x0, &y0, &x1, &y1, &x2, &y2);
			gnome_canvas_item_ungrab (handle_item,
						  event->button.time);
			gnome_canvas_item_w2i (view_highlight->private->group,
					       &event->button.x, &event->button.y);
			/* x1 unchanged */
			/* y1 unchanged */
			dx = (event->button.x - x1);
			dy = (event->button.y - x1);
			x0 = x0 + x1;
			y0 = y0 + y1;
			gnome_canvas_item_i2w (view_highlight->private->group, &x0, &y0);
			gl_label_object_set_position (object, x0, y0);
			gl_label_object_set_size (object, dx, dy);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			get_origin_and_corners (view_highlight,
						&x0, &y0, &x1, &y1, &x2, &y2);
			gnome_canvas_item_w2i (view_highlight->private->group,
					       &event->button.x, &event->button.y);
			/* x1 unchanged */
			/* y1 unchanged */
			dx = (event->button.x - x1);
			dy = (event->button.y - x1);
			x0 = x0 + x1;
			y0 = y0 + y1;
			gnome_canvas_item_i2w (view_highlight->private->group, &x0, &y0);
			gl_label_object_set_position (object, x0, y0);
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
	return gl_view_object_item_event_cb (group, event, view_object);
}
