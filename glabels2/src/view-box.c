/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_box.c:  GLabels label box object widget
 *
 *  Copyright (C) 2001-2006  Jim Evins <evins@snaught.com>.
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

#include "view-box.h"

#include <glib/gi18n.h>
#include <glib/gmem.h>

#include "view-highlight.h"
#include "color.h"
#include "object-editor.h"
#include "stock.h"

#include "pixmaps/cursor_box.xbm"
#include "pixmaps/cursor_box_mask.xbm"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

#define DELTA 0.01

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glViewBoxPrivate {
	GnomeCanvasItem       *object_item;
	GnomeCanvasItem       *shadow_item;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void       gl_view_box_finalize              (GObject          *object);

static GtkWidget *construct_properties_editor       (glViewObject     *view_object);

static void       update_canvas_item_from_object_cb (glLabelObject    *object,
						     glViewBox        *view_box);

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
G_DEFINE_TYPE (glViewBox, gl_view_box, GL_TYPE_VIEW_OBJECT);


static void
gl_view_box_class_init (glViewBoxClass *class)
{
	GObjectClass      *object_class      = G_OBJECT_CLASS (class);
	glViewObjectClass *view_object_class = GL_VIEW_OBJECT_CLASS (class);

	gl_debug (DEBUG_VIEW, "START");

	gl_view_box_parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_view_box_finalize;

	view_object_class->construct_editor = construct_properties_editor;

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_box_init (glViewBox *view_box)
{
	gl_debug (DEBUG_VIEW, "START");

	view_box->priv = g_new0 (glViewBoxPrivate, 1);

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_box_finalize (GObject *object)
{
        glViewBox *view_box = GL_VIEW_BOX (object);

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (object && GL_IS_VIEW_BOX (object));

        g_free (view_box->priv);

	G_OBJECT_CLASS (gl_view_box_parent_class)->finalize (object);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* NEW box object view.                                                      */
/*****************************************************************************/
glViewObject *
gl_view_box_new (glLabelBox *object,
		 glView     *view)
{
	glViewBox         *view_box;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (object && GL_IS_LABEL_BOX (object));
	g_return_if_fail (view && GL_IS_VIEW (view));
	
	view_box = g_object_new (gl_view_box_get_type(), NULL);

	gl_view_object_set_view (GL_VIEW_OBJECT(view_box), view);
	gl_view_object_set_object (GL_VIEW_OBJECT(view_box),
				   GL_LABEL_OBJECT(object),
				   GL_VIEW_HIGHLIGHT_BOX_RESIZABLE);

	/* Create analogous canvas items. */
	view_box->priv->shadow_item =
		gl_view_object_item_new (GL_VIEW_OBJECT(view_box),
					 gnome_canvas_rect_get_type (),
					 NULL);
	view_box->priv->object_item =
		gl_view_object_item_new (GL_VIEW_OBJECT(view_box),
					 gnome_canvas_rect_get_type (),
					 NULL);
	update_canvas_item_from_object_cb (GL_LABEL_OBJECT(object), view_box);
	
	g_signal_connect (G_OBJECT (object), "changed",
			  G_CALLBACK (update_canvas_item_from_object_cb), view_box);

	gl_debug (DEBUG_VIEW, "END");

	return GL_VIEW_OBJECT (view_box);
}

/*****************************************************************************/
/* Create a properties dialog for a box object.                              */
/*****************************************************************************/
static GtkWidget *
construct_properties_editor (glViewObject *view_object)
{
	GtkWidget          *editor;
	glViewBox          *view_box = (glViewBox *)view_object;
	glLabelObject      *object;

	gl_debug (DEBUG_VIEW, "START");

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_box));

	/* Build editor. */
	editor = gl_object_editor_new (GL_STOCK_BOX, _("Box object properties"),
				       GL_OBJECT_EDITOR_SHADOW_PAGE,
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
				   glViewBox     *view_box)
{
	gdouble            line_width;
	glColorNode       *line_color_node;
	gdouble            w, h;
	glColorNode 	  *fill_color_node;
	gboolean           shadow_state;
	gdouble            shadow_x, shadow_y;
	glColorNode	  *shadow_color_node;
	gdouble            shadow_opacity;
	guint              shadow_line_color;
	guint              shadow_fill_color;

	gl_debug (DEBUG_VIEW, "START");

	/* Query properties of object. */
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_object_get_line_width(GL_LABEL_OBJECT(object));
	line_color_node = gl_label_object_get_line_color(GL_LABEL_OBJECT(object));
	if (line_color_node->field_flag)
	{
		line_color_node->color = GL_COLOR_MERGE_DEFAULT;
	}
	fill_color_node = gl_label_object_get_fill_color(GL_LABEL_OBJECT(object));
	if (fill_color_node->field_flag)
	{
		fill_color_node->color = GL_COLOR_FILL_MERGE_DEFAULT;
	}
	shadow_state = gl_label_object_get_shadow_state (GL_LABEL_OBJECT (object));
	gl_label_object_get_shadow_offset (GL_LABEL_OBJECT (object), &shadow_x, &shadow_y);
	shadow_color_node = gl_label_object_get_shadow_color (GL_LABEL_OBJECT (object));
	if (shadow_color_node->field_flag)
	{
		shadow_color_node->color = GL_COLOR_SHADOW_MERGE_DEFAULT;
	}
	shadow_opacity = gl_label_object_get_shadow_opacity (GL_LABEL_OBJECT (object));
	shadow_line_color = gl_color_shadow (shadow_color_node->color,
					     shadow_opacity,
					     line_color_node->color);
	shadow_fill_color = gl_color_shadow (shadow_color_node->color,
					     shadow_opacity,
					     fill_color_node->color);

	/* Adjust appearance of analogous canvas items. */
	gnome_canvas_item_set (view_box->priv->shadow_item,
			       "x1", shadow_x,
			       "y1", shadow_y,
			       "x2", shadow_x + w + DELTA,
			       "y2", shadow_y + h + DELTA,
			       "width_units", line_width,
			       "outline_color_rgba", shadow_line_color,
			       "fill_color_rgba", shadow_fill_color,
			       NULL);

	if (shadow_state)
	{
		gnome_canvas_item_show (view_box->priv->shadow_item);
	}
	else
	{
		gnome_canvas_item_hide (view_box->priv->shadow_item);
	}

	gnome_canvas_item_set (view_box->priv->object_item,
			       "x2", w + DELTA,
			       "y2", h + DELTA,
			       "width_units", line_width,
			       "outline_color_rgba", line_color_node->color,
			       "fill_color_rgba", fill_color_node->color,
			       NULL);
	
	gl_color_node_free (&line_color_node);
	gl_color_node_free (&fill_color_node);
	gl_color_node_free (&shadow_color_node);

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
	glColorNode	  *fill_color_node;
	gboolean           shadow_state;
	gdouble            shadow_x, shadow_y;
	glColorNode	  *shadow_color_node;
	gdouble            shadow_opacity;
	

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

	fill_color_node = gl_object_editor_get_fill_color (editor);
	gl_label_object_set_fill_color (object, fill_color_node);
	gl_color_node_free (&fill_color_node);

	line_color_node = gl_object_editor_get_line_color (editor);
	gl_label_object_set_line_color (object, line_color_node);
	gl_color_node_free (&line_color_node);

	line_width = gl_object_editor_get_line_width (editor);
	gl_label_object_set_line_width (object, line_width);

	shadow_state = gl_object_editor_get_shadow_state (editor);
	gl_label_object_set_shadow_state (object, shadow_state);

	gl_object_editor_get_shadow_offset (editor, &shadow_x, &shadow_y);
	gl_label_object_set_shadow_offset (object, shadow_x, shadow_y);

	shadow_color_node = gl_object_editor_get_shadow_color (editor);
	gl_label_object_set_shadow_color (object, shadow_color_node);
	gl_color_node_free (&shadow_color_node);

	shadow_opacity = gl_object_editor_get_shadow_opacity (editor);
	gl_label_object_set_shadow_opacity (object, shadow_opacity);

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
	glColorNode       *fill_color_node;
	gboolean           shadow_state;
	gdouble            shadow_x, shadow_y;
	glColorNode	  *shadow_color_node;
	gdouble            shadow_opacity;
	glMerge	          *merge;

	gl_debug (DEBUG_VIEW, "START");

	gl_label_object_get_size (object, &w, &h);
	gl_object_editor_set_size (editor, w, h);
	merge = gl_label_get_merge (GL_LABEL(object->parent));
	
	fill_color_node = gl_label_object_get_fill_color (GL_LABEL_OBJECT(object));
	gl_object_editor_set_fill_color (editor, (merge != NULL), fill_color_node);
	gl_color_node_free (&fill_color_node);

	line_color_node = gl_label_object_get_line_color (GL_LABEL_OBJECT(object));
	gl_object_editor_set_line_color (editor, (merge != NULL), line_color_node);
	gl_color_node_free (&line_color_node);

	line_width = gl_label_object_get_line_width (GL_LABEL_OBJECT(object));
	gl_object_editor_set_line_width (editor, line_width);

	shadow_state = gl_label_object_get_shadow_state (object);
	gl_object_editor_set_shadow_state (editor, shadow_state);

	gl_label_object_get_shadow_offset (object, &shadow_x, &shadow_y);
	gl_object_editor_set_shadow_offset (editor, shadow_x, shadow_y);

	shadow_color_node = gl_label_object_get_shadow_color (object);
	gl_object_editor_set_shadow_color (editor, (merge != NULL), shadow_color_node);
	gl_color_node_free (&shadow_color_node);

	shadow_opacity = gl_label_object_get_shadow_opacity (object);
	gl_object_editor_set_shadow_opacity (editor, shadow_opacity);

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
	glMerge		   	   *merge;

	gl_debug (DEBUG_VIEW, "START");

	gl_label_get_size (label, &label_width, &label_height);
	gl_object_editor_set_max_position (GL_OBJECT_EDITOR (editor),
					   label_width, label_height);
	gl_object_editor_set_max_size (GL_OBJECT_EDITOR (editor),
				       label_width, label_height);
	gl_object_editor_set_max_shadow_offset (GL_OBJECT_EDITOR (editor),
						label_width, label_height);
	
	merge = gl_label_get_merge (label);
	gl_object_editor_set_key_names (editor, merge);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Return apropos cursor for create object mode.                             */
/*****************************************************************************/
GdkCursor *
gl_view_box_get_create_cursor (void)
{
	static GdkCursor *cursor = NULL;
	GdkPixmap        *pixmap_data, *pixmap_mask;
	GdkColor         fg = { 0, 0, 0, 0 };
	GdkColor         bg = { 0, 65535, 65535, 65535 };

	gl_debug (DEBUG_VIEW, "START");

	if (!cursor) {
		pixmap_data = gdk_bitmap_create_from_data (NULL,
							   (gchar *)cursor_box_bits,
							   cursor_box_width,
							   cursor_box_height);
		pixmap_mask = gdk_bitmap_create_from_data (NULL,
							   (gchar *)cursor_box_mask_bits,
							   cursor_box_mask_width,
							   cursor_box_mask_height);
		cursor =
		    gdk_cursor_new_from_pixmap (pixmap_data, pixmap_mask, &fg,
						&bg, cursor_box_x_hot,
						cursor_box_y_hot);
	}

	gl_debug (DEBUG_VIEW, "END");

	return cursor;
}

/*****************************************************************************/
/* Canvas event handler for creating box objects.                            */
/*****************************************************************************/
int
gl_view_box_create_event_handler (GnomeCanvas *canvas,
				  GdkEvent    *event,
				  glView      *view)
{
	static gdouble       x0, y0;
	static gboolean      dragging = FALSE;
	static glViewObject *view_box;
	static GObject      *object;
	glColorNode         *line_color_node;
	gdouble              x, y, w, h;
	glColorNode         *fill_color_node;

	gl_debug (DEBUG_VIEW, "");

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			fill_color_node = gl_color_node_new_default ();
			line_color_node = gl_color_node_new_default ();
		
			dragging = TRUE;
			gnome_canvas_item_grab ( canvas->root,
						 GDK_POINTER_MOTION_MASK |
						 GDK_BUTTON_RELEASE_MASK |
						 GDK_BUTTON_PRESS_MASK,
						 NULL, event->button.time);
			gnome_canvas_window_to_world (canvas,
						      event->button.x,
						      event->button.y, &x, &y);
			object = gl_label_box_new (view->label);
			gl_label_object_set_position (GL_LABEL_OBJECT(object),
						     x, y);
			gl_label_object_set_size (GL_LABEL_OBJECT(object),
						  0.0, 0.0);
			line_color_node->color = gl_color_set_opacity (gl_view_get_default_line_color(view), 0.5);
			fill_color_node->color = gl_color_set_opacity (gl_view_get_default_fill_color(view), 0.5);
			gl_label_object_set_line_width (GL_LABEL_OBJECT(object),
						     gl_view_get_default_line_width(view));
			gl_label_object_set_line_color (GL_LABEL_OBJECT(object),
						     line_color_node);
			gl_label_object_set_fill_color (GL_LABEL_OBJECT(object),
						     fill_color_node);
			view_box = gl_view_box_new (GL_LABEL_BOX(object),
						    view);
							
			gl_color_node_free (&fill_color_node);
			gl_color_node_free (&line_color_node);
			x0 = x;
			y0 = y;
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			fill_color_node = gl_color_node_new_default ();
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
			gl_label_object_set_position (GL_LABEL_OBJECT(object),
						     MIN (x, x0), MIN (y, y0));
			w = MAX (x, x0) - MIN (x, x0);
			h = MAX (y, y0) - MIN (y, y0);
			gl_label_object_set_size (GL_LABEL_OBJECT(object),
						  w, h);
			line_color_node->color = gl_view_get_default_line_color(view);
			gl_label_object_set_line_color (GL_LABEL_OBJECT(object), line_color_node);
			fill_color_node->color = gl_view_get_default_fill_color(view);
			gl_label_object_set_fill_color (GL_LABEL_OBJECT(object), fill_color_node);
			gl_view_unselect_all (view);
			gl_view_object_select (GL_VIEW_OBJECT(view_box));
			gl_view_arrow_mode (view);
			
			gl_color_node_free (&fill_color_node);
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
