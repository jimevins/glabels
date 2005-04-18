/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_line.c:  GLabels label line object widget
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

#include "view-line.h"

#include <glib/gi18n.h>
#include <glib/gmem.h>

#include "view-highlight.h"
#include "color.h"
#include "object-editor.h"
#include "stock.h"

#include "pixmaps/cursor_line.xbm"
#include "pixmaps/cursor_line_mask.xbm"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glViewLinePrivate {
	GnomeCanvasItem       *item;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static glViewObjectClass *parent_class = NULL;


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void       gl_view_line_class_init           (glViewLineClass  *klass);
static void       gl_view_line_instance_init        (glViewLine       *view_line);
static void       gl_view_line_finalize             (GObject          *object);

static GtkWidget *construct_properties_editor       (glViewObject     *view_object);

static void       update_canvas_item_from_object_cb (glLabelObject    *object,
						     glViewLine        *view_line);

static void       update_object_from_editor_cb      (glObjectEditor   *editor,
						     glLabelObject    *object);

static void       update_editor_from_object_cb      (glLabelObject    *object,
						     glObjectEditor   *editor);

static void       update_editor_from_move_cb        (glLabelObject    *object,
						     gdouble           dx,
						     gdouble           dy,
						     glObjectEditor   *editor);

static void       update_editor_from_label_cb       (glLabel          *label,
						     glObjectEditor   *editor);



/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_view_line_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (glViewLineClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_view_line_class_init,
			NULL,
			NULL,
			sizeof (glViewLine),
			0,
			(GInstanceInitFunc) gl_view_line_instance_init,
			NULL
		};

		type = g_type_register_static (GL_TYPE_VIEW_OBJECT,
					       "glViewLine", &info, 0);
	}

	return type;
}

static void
gl_view_line_class_init (glViewLineClass *klass)
{
	GObjectClass      *object_class      = (GObjectClass *) klass;
	glViewObjectClass *view_object_class = (glViewObjectClass *) klass;

	gl_debug (DEBUG_VIEW, "START");

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gl_view_line_finalize;

	view_object_class->construct_editor = construct_properties_editor;

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_line_instance_init (glViewLine *view_line)
{
	gl_debug (DEBUG_VIEW, "START");

	view_line->private = g_new0 (glViewLinePrivate, 1);

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_line_finalize (GObject *object)
{
	glLabel       *parent;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (object && GL_IS_VIEW_LINE (object));

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* NEW line object view.                                                     */
/*****************************************************************************/
glViewObject *
gl_view_line_new (glLabelLine *object,
		  glView      *view)
{
	glViewLine        *view_line;
	gdouble            line_width;
	glColorNode       *line_color_node;
	gdouble            w, h;
	GnomeCanvasPoints *points;

	gl_debug (DEBUG_VIEW, "START");
	g_return_if_fail (object && GL_IS_LABEL_LINE (object));
	g_return_if_fail (view && GL_IS_VIEW (view));
	
	view_line = g_object_new (gl_view_line_get_type(), NULL);

	gl_view_object_set_view (GL_VIEW_OBJECT(view_line), view);
	gl_view_object_set_object (GL_VIEW_OBJECT(view_line),
				   GL_LABEL_OBJECT(object),
				   GL_VIEW_HIGHLIGHT_LINE_RESIZABLE);

	/* Query properties of object. */
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_object_get_line_width(GL_LABEL_OBJECT(object));
	line_color_node = gl_label_object_get_line_color(GL_LABEL_OBJECT(object));
	if (line_color_node->field_flag)
	{
		line_color_node->color = GL_COLOR_MERGE_DEFAULT;
	}

	/* Create analogous canvas item. */
	points = gnome_canvas_points_new (2);
	points->coords[0] = 0.0;
	points->coords[1] = 0.0;
	points->coords[2] = w;
	points->coords[3] = h;
	view_line->private->item =
		gl_view_object_item_new (GL_VIEW_OBJECT(view_line),
					 gnome_canvas_line_get_type (),
					 "points", points,
					 "width_units", line_width,
					 "fill_color_rgba", line_color_node->color,
					 NULL);
	gl_color_node_free (&line_color_node);
	gnome_canvas_points_free (points);

	g_signal_connect (G_OBJECT (object), "changed",
			  G_CALLBACK (update_canvas_item_from_object_cb), view_line);

	gl_debug (DEBUG_VIEW, "END");

	return GL_VIEW_OBJECT (view_line);
}

/*****************************************************************************/
/* Create a properties editor for a line object.                             */
/*****************************************************************************/
static GtkWidget *
construct_properties_editor (glViewObject *view_object)
{
	GtkWidget          *editor;
	glViewLine         *view_line = (glViewLine *)view_object;
	glLabelObject      *object;

	gl_debug (DEBUG_VIEW, "START");

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_line));

	/* Build editor. */
	editor = gl_object_editor_new (GL_STOCK_LINE, _("Line object properties"),
				       GL_OBJECT_EDITOR_POSITION_PAGE,
				       GL_OBJECT_EDITOR_SIZE_LINE_PAGE,
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
	g_signal_connect (G_OBJECT (object->parent), "merge_changed",
			  G_CALLBACK (update_editor_from_label_cb), editor);

	gl_debug (DEBUG_VIEW, "END");

	return editor;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "changed" callback.                                 */
/*---------------------------------------------------------------------------*/
static void
update_canvas_item_from_object_cb (glLabelObject *object,
				   glViewLine     *view_line)
{
	gdouble            line_width;
	glColorNode       *line_color_node;
	gdouble            w, h;
	GnomeCanvasPoints *points;

	gl_debug (DEBUG_VIEW, "START");

	/* Query properties of object. */
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_object_get_line_width(GL_LABEL_OBJECT(object));
	line_color_node = gl_label_object_get_line_color(GL_LABEL_OBJECT(object));
	if (line_color_node->field_flag)
	{
		line_color_node->color = GL_COLOR_MERGE_DEFAULT;
	}

	/* Adjust appearance of analogous canvas item. */
	points = gnome_canvas_points_new (2);
	points->coords[0] = 0.0;
	points->coords[1] = 0.0;
	points->coords[2] = w;
	points->coords[3] = h;
	gnome_canvas_item_set (view_line->private->item,
			       "points", points,
			       "width_units", line_width,
			       "fill_color_rgba", line_color_node->color,
			       NULL);
	gl_color_node_free (&line_color_node);
	gnome_canvas_points_free (points);

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
	glColorNode       *line_color_node;
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

	gl_object_editor_get_lsize (editor, &w, &h);
	gl_label_object_set_size (object, w, h);
	
	line_color_node = gl_object_editor_get_line_color (editor);
	gl_label_object_set_line_color (object, line_color_node);
	gl_color_node_free (&line_color_node);

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
	glColorNode       *line_color_node;
	gdouble            line_width;
	glMerge	          *merge;

	gl_debug (DEBUG_VIEW, "START");

	gl_label_object_get_size (object, &w, &h);
	gl_object_editor_set_lsize (editor, w, h);
	
	merge = gl_label_get_merge (GL_LABEL(object->parent));

	line_color_node = gl_label_object_get_line_color (GL_LABEL_OBJECT(object));
	gl_object_editor_set_line_color (editor, (merge != NULL), line_color_node);
	gl_color_node_free (&line_color_node);

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
	glMerge     	  *merge;

	gl_debug (DEBUG_VIEW, "START");

	gl_label_get_size (label, &label_width, &label_height);
	gl_object_editor_set_max_position (GL_OBJECT_EDITOR (editor),
					   label_width, label_height);
	gl_object_editor_set_max_lsize (GL_OBJECT_EDITOR (editor),
					label_width, label_height);
	
	merge = gl_label_get_merge (label);
	gl_object_editor_set_key_names (editor, merge);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Return apropos cursor for create object mode.                             */
/*****************************************************************************/
GdkCursor *
gl_view_line_get_create_cursor (void)
{
	static GdkCursor *cursor = NULL;
	GdkPixmap        *pixmap_data, *pixmap_mask;
	GdkColor         fg = { 0, 0, 0, 0 };
	GdkColor         bg = { 0, 65535, 65535, 65535 };

	gl_debug (DEBUG_VIEW, "START");

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

	gl_debug (DEBUG_VIEW, "END");

	return cursor;
}

/*****************************************************************************/
/* Canvas event handler for creating line objects.                            */
/*****************************************************************************/
int
gl_view_line_create_event_handler (GnomeCanvas *canvas,
				   GdkEvent    *event,
				   glView      *view)
{
	static gdouble       x0, y0;
	static gboolean      dragging = FALSE;
	static glViewObject *view_line;
	static GObject      *object;
	gdouble              x, y, w, h;
	glColorNode         *line_color_node;

	gl_debug (DEBUG_VIEW, "");

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			line_color_node = gl_color_node_new_default ();
			dragging = TRUE;
			gnome_canvas_item_grab (canvas->root,
						GDK_POINTER_MOTION_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_BUTTON_PRESS_MASK,
						NULL, event->button.time);
			gnome_canvas_window_to_world (canvas,
						      event->button.x,
						      event->button.y, &x, &y);
			object = gl_label_line_new (view->label);
			gl_label_object_set_position (GL_LABEL_OBJECT(object),
						      x, y);
			gl_label_object_set_size (GL_LABEL_OBJECT(object),
						  0.0, 0.0);
			gl_label_object_set_line_width (GL_LABEL_OBJECT(object),
						      gl_view_get_default_line_width(view));
			line_color_node->color = gl_color_set_opacity (gl_view_get_default_line_color(view), 0.5);
			gl_label_object_set_line_color (GL_LABEL_OBJECT(object),
						     line_color_node);
			view_line = gl_view_line_new (GL_LABEL_LINE(object),
						      view);
			x0 = x;
			y0 = y;
			gl_color_node_free (&line_color_node);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			line_color_node = gl_color_node_new_default ();
			dragging = FALSE;
			gnome_canvas_item_ungrab (canvas->root, event->button.time);
			gnome_canvas_window_to_world (canvas,
						      event->button.x,
						      event->button.y, &x, &y);
			if ((x0 == x) && (y0 == y)) {
				x = x0 + 36.0;
				y = y0 + 36.0;
			}
			w = x - x0;
			h = y - y0;
			gl_label_object_set_size (GL_LABEL_OBJECT(object),
						  w, h);
			line_color_node->color = gl_view_get_default_line_color(view);
			gl_label_object_set_line_color (GL_LABEL_OBJECT(object),
						     line_color_node);
			gl_view_unselect_all (view);
			gl_view_object_select (GL_VIEW_OBJECT(view_line));
			gl_view_arrow_mode (view);
			gl_color_node_free (&line_color_node);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			gnome_canvas_window_to_world (canvas,
						      event->motion.x,
						      event->motion.y, &x, &y);
			w = x - x0;
			h = y - y0;
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
