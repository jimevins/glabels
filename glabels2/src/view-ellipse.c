/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_ellipse.c:  GLabels label ellipse object widget
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
#include <config.h>

#include <glib.h>

#include "view-ellipse.h"

#include "view-highlight.h"

#include "color.h"
#include "object-editor.h"
#include "stock.h"

#include "pixmaps/cursor_ellipse.xbm"
#include "pixmaps/cursor_ellipse_mask.xbm"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

#define DELTA 0.01

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glViewEllipsePrivate {
	GnomeCanvasItem       *item;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static glViewObjectClass *parent_class = NULL;


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void       gl_view_ellipse_class_init        (glViewEllipseClass   *klass);
static void       gl_view_ellipse_instance_init     (glViewEllipse        *view_ellipse);
static void       gl_view_ellipse_finalize          (GObject              *object);

static GtkWidget *construct_properties_editor       (glViewObject         *view_object);

static void       update_canvas_item_from_object_cb (glLabelObject        *object,
						     glViewEllipse        *view_ellipse);

static void       update_object_from_editor_cb      (glObjectEditor       *editor,
						     glLabelObject        *object);

static void       update_editor_from_object_cb      (glLabelObject        *object,
						     glObjectEditor       *editor);

static void       update_editor_from_move_cb        (glLabelObject        *object,
						     gdouble               dx,
						     gdouble               dy,
						     glObjectEditor       *editor);

static void       update_editor_from_label_cb       (glLabel          *label,
						     glObjectEditor   *editor);



/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_view_ellipse_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (glViewEllipseClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_view_ellipse_class_init,
			NULL,
			NULL,
			sizeof (glViewEllipse),
			0,
			(GInstanceInitFunc) gl_view_ellipse_instance_init,
			NULL
		};

		type = g_type_register_static (GL_TYPE_VIEW_OBJECT,
					       "glViewEllipse", &info, 0);
	}

	return type;
}

static void
gl_view_ellipse_class_init (glViewEllipseClass *klass)
{
	GObjectClass      *object_class      = (GObjectClass *) klass;
	glViewObjectClass *view_object_class = (glViewObjectClass *) klass;

	gl_debug (DEBUG_VIEW, "START");

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gl_view_ellipse_finalize;

	view_object_class->construct_editor = construct_properties_editor;

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_ellipse_instance_init (glViewEllipse *view_ellipse)
{
	gl_debug (DEBUG_VIEW, "START");

	view_ellipse->private = g_new0 (glViewEllipsePrivate, 1);

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_ellipse_finalize (GObject *object)
{
	glLabel       *parent;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (object && GL_IS_VIEW_ELLIPSE (object));

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* NEW ellipse object view.                                                  */
/*****************************************************************************/
glViewObject *
gl_view_ellipse_new (glLabelEllipse *object,
		     glView         *view)
{
	glViewEllipse     *view_ellipse;
	gdouble            line_width;
	guint              line_color, fill_color;
	gdouble            w, h;

	gl_debug (DEBUG_VIEW, "START");
	g_return_if_fail (object && GL_IS_LABEL_ELLIPSE (object));
	g_return_if_fail (view && GL_IS_VIEW (view));
	
	view_ellipse = g_object_new (gl_view_ellipse_get_type(), NULL);

	gl_view_object_set_view (GL_VIEW_OBJECT(view_ellipse), view);
	gl_view_object_set_object (GL_VIEW_OBJECT(view_ellipse),
				   GL_LABEL_OBJECT(object),
				   GL_VIEW_HIGHLIGHT_ELLIPSE_RESIZABLE);

	/* Query properties of object. */
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_object_get_line_width(GL_LABEL_OBJECT(object));
	line_color = gl_label_object_get_line_color(GL_LABEL_OBJECT(object));
	fill_color = gl_label_object_get_fill_color(GL_LABEL_OBJECT(object));

	/* Create analogous canvas item. */
	view_ellipse->private->item =
		gl_view_object_item_new (GL_VIEW_OBJECT(view_ellipse),
					 gnome_canvas_ellipse_get_type (),
					 "x1", 0.0,
					 "y1", 0.0,
					 "x2", w + DELTA,
					 "y2", h + DELTA,
					 "width_units", line_width,
					 "outline_color_rgba", line_color,
					 "fill_color_rgba", fill_color,
					 NULL);

	g_signal_connect (G_OBJECT (object), "changed",
			  G_CALLBACK (update_canvas_item_from_object_cb), view_ellipse);

	gl_debug (DEBUG_VIEW, "END");

	return GL_VIEW_OBJECT (view_ellipse);
}

/*****************************************************************************/
/* Create a properties editor for an ellipse object.                         */
/*****************************************************************************/
static GtkWidget *
construct_properties_editor (glViewObject *view_object)
{
	GtkWidget          *editor;
	glViewEllipse      *view_ellipse = (glViewEllipse *)view_object;
	glLabelObject      *object;

	gl_debug (DEBUG_VIEW, "START");

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_ellipse));

	/* Build editor. */
	editor = gl_object_editor_new (GL_STOCK_ELLIPSE, _("Ellipse object properties"),
				       GL_OBJECT_EDITOR_POSITION_PAGE,
				       GL_OBJECT_EDITOR_SIZE_PAGE,
				       GL_OBJECT_EDITOR_FILL_PAGE,
				       GL_OBJECT_EDITOR_LINE_PAGE,
				       0);
	
	/* Update */
	update_editor_from_label_cb (object->parent, GL_OBJECT_EDITOR(editor));
	update_editor_from_object_cb (object, GL_OBJECT_EDITOR(editor));
	update_editor_from_move_cb (object, 0, 0, GL_OBJECT_EDITOR(editor));

	/* Connect signals. */
	g_signal_connect (G_OBJECT (editor), "changed",
			  G_CALLBACK(update_object_from_editor_cb), object);
	g_signal_connect (G_OBJECT (object), "changed",
			  G_CALLBACK (update_editor_from_object_cb), editor);
	g_signal_connect (G_OBJECT (object), "moved",
			  G_CALLBACK (update_editor_from_move_cb), editor);
	g_signal_connect (G_OBJECT (object->parent), "size_changed",
			  G_CALLBACK (update_editor_from_label_cb), editor);

	gl_debug (DEBUG_VIEW, "END");

	return editor;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "changed" callback.                                 */
/*---------------------------------------------------------------------------*/
static void
update_canvas_item_from_object_cb (glLabelObject *object,
				   glViewEllipse *view_ellipse)
{
	gdouble            line_width;
	guint              line_color, fill_color;
	gdouble            w, h;

	gl_debug (DEBUG_VIEW, "START");

	/* Query properties of object. */
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_object_get_line_width(GL_LABEL_OBJECT(object));
	line_color = gl_label_object_get_line_color(GL_LABEL_OBJECT(object));
	fill_color = gl_label_object_get_fill_color(GL_LABEL_OBJECT(object));

	/* Adjust appearance of analogous canvas item. */
	gnome_canvas_item_set (view_ellipse->private->item,
			       "x2", w + DELTA,
			       "y2", h + DELTA,
			       "width_units", line_width,
			       "outline_color_rgba", line_color,
			       "fill_color_rgba", fill_color,
			       NULL);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  editor "changed" callback.                                      */
/*---------------------------------------------------------------------------*/
static void
update_object_from_editor_cb (glObjectEditor *editor,
			      glLabelObject  *object)
{
	gdouble            x, y, w, h;
	guint              line_color, fill_color;
	gdouble            line_width;

	gl_debug (DEBUG_VIEW, "START");

	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_editor_from_object_cb,
					 editor);
	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_editor_from_move_cb,
					 editor);


	gl_object_editor_get_position (editor, &x, &y);
	gl_label_object_set_position (object, x, y);

	gl_object_editor_get_size (editor, &w, &h);
	gl_label_object_set_size (object, w, h);

	fill_color = gl_object_editor_get_fill_color (editor);
	gl_label_object_set_fill_color (object, fill_color);

	line_color = gl_object_editor_get_line_color (editor);
	gl_label_object_set_line_color (object, line_color);

	line_width = gl_object_editor_get_line_width (editor);
	gl_label_object_set_line_width (object, line_width);


	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_editor_from_object_cb,
					   editor);
	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_editor_from_move_cb,
					   editor);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "changed" callback.                                 */
/*---------------------------------------------------------------------------*/
static void
update_editor_from_object_cb (glLabelObject  *object,
			      glObjectEditor *editor)
{
	gdouble            w, h;
	guint              line_color, fill_color;
	gdouble            line_width;

	gl_debug (DEBUG_VIEW, "START");

	gl_label_object_get_size (object, &w, &h);
	gl_object_editor_set_size (editor, w, h);

	fill_color = gl_label_object_get_fill_color (GL_LABEL_OBJECT(object));
	gl_object_editor_set_fill_color (editor, fill_color);

	line_color = gl_label_object_get_line_color (GL_LABEL_OBJECT(object));
	gl_object_editor_set_line_color (editor, line_color);

	line_width = gl_label_object_get_line_width (GL_LABEL_OBJECT(object));
	gl_object_editor_set_line_width (editor, line_width);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "moved" callback.                                   */
/*---------------------------------------------------------------------------*/
static void
update_editor_from_move_cb (glLabelObject    *object,
			    gdouble           dx,
			    gdouble           dy,
			    glObjectEditor   *editor)
{
	gdouble            x, y;

	gl_debug (DEBUG_VIEW, "START");

	gl_label_object_get_position (object, &x, &y);
	gl_object_editor_set_position (editor, x, y);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label "changed" callback.                                        */
/*---------------------------------------------------------------------------*/
static void
update_editor_from_label_cb (glLabel        *label,
			     glObjectEditor *editor)
{
	gdouble            label_width, label_height;

	gl_debug (DEBUG_VIEW, "START");

	gl_label_get_size (label, &label_width, &label_height);
	gl_object_editor_set_max_position (GL_OBJECT_EDITOR (editor),
					   label_width, label_height);
	gl_object_editor_set_max_size (GL_OBJECT_EDITOR (editor),
				       label_width, label_height);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Return apropos cursor for create object mode.                             */
/*****************************************************************************/
GdkCursor *
gl_view_ellipse_get_create_cursor (void)
{
	static GdkCursor *cursor = NULL;
	GdkPixmap        *pixmap_data, *pixmap_mask;
	GdkColor         fg = { 0, 0, 0, 0 };
	GdkColor         bg = { 0, 65535, 65535, 65535 };

	gl_debug (DEBUG_VIEW, "START");

	if (!cursor) {
		pixmap_data = gdk_bitmap_create_from_data (NULL,
							   cursor_ellipse_bits,
							   cursor_ellipse_width,
							   cursor_ellipse_height);
		pixmap_mask = gdk_bitmap_create_from_data (NULL,
							   cursor_ellipse_mask_bits,
							   cursor_ellipse_mask_width,
							   cursor_ellipse_mask_height);
		cursor =
		    gdk_cursor_new_from_pixmap (pixmap_data, pixmap_mask, &fg,
						&bg, cursor_ellipse_x_hot,
						cursor_ellipse_y_hot);
	}

	gl_debug (DEBUG_VIEW, "END");

	return cursor;
}

/*****************************************************************************/
/* Canvas event handler for creating ellipse objects.                        */
/*****************************************************************************/
int
gl_view_ellipse_create_event_handler (GnomeCanvas *canvas,
				      GdkEvent    *event,
				      glView      *view)
{
	static gdouble       x0, y0;
	static gboolean      dragging = FALSE;
	static glViewObject *view_ellipse;
	static GObject      *object;
	guint                line_color, fill_color;
	gdouble              x, y, w, h;

	gl_debug (DEBUG_VIEW, "");

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			dragging = TRUE;
			gnome_canvas_item_grab ( canvas->root,
						 GDK_POINTER_MOTION_MASK |
						 GDK_BUTTON_RELEASE_MASK |
						 GDK_BUTTON_PRESS_MASK,
						 NULL, event->button.time);
			gnome_canvas_window_to_world (canvas,
						      event->button.x,
						      event->button.y, &x, &y);
			object = gl_label_ellipse_new (view->label);
			gl_label_object_set_position (GL_LABEL_OBJECT(object),
						     x, y);
			gl_label_object_set_size (GL_LABEL_OBJECT(object),
						  0.0, 0.0);
			line_color = gl_color_set_opacity (gl_view_get_default_line_color(view), 0.5);
			fill_color = gl_color_set_opacity (gl_view_get_default_fill_color(view), 0.5);
			gl_label_object_set_line_width (GL_LABEL_OBJECT(object),
						     gl_view_get_default_line_width(view));
			gl_label_object_set_line_color (GL_LABEL_OBJECT(object),
							 line_color);
			gl_label_object_set_fill_color (GL_LABEL_OBJECT(object),
							 fill_color);
			view_ellipse = gl_view_ellipse_new (GL_LABEL_ELLIPSE(object),
							    view);
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
			gnome_canvas_item_ungrab (canvas->root, event->button.time);
			gnome_canvas_window_to_world (canvas,
						      event->button.x,
						      event->button.y, &x, &y);
			if ((x0 == x) && (y0 == y)) {
				x = x0 + 36.0;
				y = y0 + 36.0;
			}
			gl_label_object_set_position (GL_LABEL_OBJECT(object),
						     MIN (x, x0), MIN (y, y0));
			w = MAX (x, x0) - MIN (x, x0);
			h = MAX (y, y0) - MIN (y, y0);
			gl_label_object_set_size (GL_LABEL_OBJECT(object),
						  w, h);
			gl_label_object_set_line_color (GL_LABEL_OBJECT(object),
						     gl_view_get_default_line_color(view));
			gl_label_object_set_fill_color (GL_LABEL_OBJECT(object),
						     gl_view_get_default_fill_color(view));
			gl_view_unselect_all (view);
			gl_view_object_select (GL_VIEW_OBJECT(view_ellipse));
			gl_view_arrow_mode (view);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			gnome_canvas_window_to_world (canvas,
						      event->motion.x,
						      event->motion.y, &x, &y);
			gl_label_object_set_position (GL_LABEL_OBJECT(object),
						     MIN (x, x0), MIN (y, y0));
			w = MAX (x, x0) - MIN (x, x0);
			h = MAX (y, y0) - MIN (y, y0);
			gl_label_object_set_size (GL_LABEL_OBJECT(object),
						  w, h);
			return TRUE;
		} else {
			return FALSE;
		}

	default:
		return FALSE;
	}

}
