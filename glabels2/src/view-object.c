/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_object.c:  GLabels label object base class
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

#include <glib.h>

#include "view-object.h"

#include "libart_lgpl/libart.h"

#include "debug.h"

/*========================================================*/
/* Private constants and macros.                          */
/*========================================================*/

/* Used for a workaround for a bug with images when flipped or rotated by 90 degrees. */
#define DELTA_DEG 0.001

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glViewObjectPrivate {

	glView                     *view;
	glLabelObject              *object;

	GnomeCanvasItem            *group;
	glViewHighlight            *highlight;

	GtkWidget                  *property_editor;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static GObjectClass *parent_class = NULL;


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void     gl_view_object_class_init    (glViewObjectClass   *klass);
static void     gl_view_object_instance_init (glViewObject        *view_object);
static void     gl_view_object_finalize      (GObject             *object);

static void     object_moved_cb              (glLabelObject       *object,
					      gdouble              x,
					      gdouble              y,
					      glViewObject        *view_object);

static void     raise_object_cb              (glLabelObject       *object,
					      glViewObject        *view_object);

static void     lower_object_cb              (glLabelObject       *object,
					      glViewObject        *view_object);

static void     flip_rotate_object_cb        (glLabelObject       *object,
					      glViewObject        *view_object);

static gint     item_event_arrow_mode        (GnomeCanvasItem     *item,
					      GdkEvent            *event,
					      glViewObject        *view_object);





/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_view_object_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (glViewObjectClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_view_object_class_init,
			NULL,
			NULL,
			sizeof (glViewObject),
			0,
			(GInstanceInitFunc) gl_view_object_instance_init,
			NULL
		};

		type = g_type_register_static (G_TYPE_OBJECT,
					       "glViewObject", &info, 0);
	}

	return type;
}

static void
gl_view_object_class_init (glViewObjectClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	gl_debug (DEBUG_VIEW, "START");

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gl_view_object_finalize;

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_object_instance_init (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	view_object->private = g_new0 (glViewObjectPrivate, 1);

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_object_finalize (GObject *object)
{
	glLabel       *parent;
	glView        *view;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (object && GL_IS_VIEW_OBJECT (object));

	view = GL_VIEW_OBJECT(object)->private->view;
	view->object_list = g_list_remove (view->object_list, object);
	view->selected_object_list =
		g_list_remove (view->selected_object_list, object);

	g_object_unref (GL_VIEW_OBJECT(object)->private->object);
	g_object_unref (G_OBJECT(GL_VIEW_OBJECT(object)->private->highlight));
	gtk_object_destroy (GTK_OBJECT(GL_VIEW_OBJECT(object)->private->group));
	if (GL_VIEW_OBJECT(object)->private->property_editor) {
		gtk_object_destroy (GTK_OBJECT(GL_VIEW_OBJECT(object)->private->property_editor));
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* NEW object view.                                                          */
/*****************************************************************************/
GObject *
gl_view_object_new (void)
{
	glViewObject *view_object;

	gl_debug (DEBUG_VIEW, "START");

	view_object = g_object_new (gl_view_object_get_type(), NULL);

	gl_debug (DEBUG_VIEW, "END");

	return G_OBJECT (view_object);
}

/*****************************************************************************/
/* Set parent view to which this object view belongs.                        */
/*****************************************************************************/
void
gl_view_object_set_view       (glViewObject *view_object,
			       glView *view)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));
	g_return_if_fail (view && GL_IS_VIEW (view));
	
	view_object->private->view = view;

	view->object_list = g_list_prepend (view->object_list, view_object);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Set Label object to follow.                                               */
/*****************************************************************************/
void
gl_view_object_set_object     (glViewObject         *view_object,
			       glLabelObject        *object,
			       glViewHighlightStyle style)
{
	GnomeCanvas        *canvas;
	GnomeCanvasGroup   *root;
	gdouble            x, y, w, h;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));
	
	view_object->private->object = object;

	gl_label_object_get_position (GL_LABEL_OBJECT(object), &x, &y);
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);

	/* create canvas group to contain items representing object */
	view_object->private->group =
		gnome_canvas_item_new (view_object->private->view->label_group,
				       gnome_canvas_group_get_type (),
				       "x", x,
				       "y", y,
				       NULL);

	/* Create appropriate selection highlight canvas item. */
	view_object->private->highlight =
		GL_VIEW_HIGHLIGHT (gl_view_highlight_new (view_object, style));

	g_signal_connect (G_OBJECT (object), "moved",
			  G_CALLBACK (object_moved_cb),
			  view_object);

	g_signal_connect (G_OBJECT (object), "top",
			  G_CALLBACK (raise_object_cb),
			  view_object);

	g_signal_connect (G_OBJECT (object), "bottom",
			  G_CALLBACK (lower_object_cb),
			  view_object);

	g_signal_connect (G_OBJECT (object), "flip_rotate",
			  G_CALLBACK (flip_rotate_object_cb),
			  view_object);

	g_signal_connect (G_OBJECT (view_object->private->group), "event",
			  G_CALLBACK (gl_view_object_item_event_cb),
			  view_object);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Return parent view associated with this view.                             */
/*****************************************************************************/
glView *
gl_view_object_get_view   (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));

	gl_debug (DEBUG_VIEW, "END");

	return view_object->private->view;
}

/*****************************************************************************/
/* Return label object that we are following.                                */
/*****************************************************************************/
glLabelObject *
gl_view_object_get_object (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));
	
	gl_debug (DEBUG_VIEW, "END");

	return view_object->private->object;
}

/*****************************************************************************/
/* Return canvas item representing our object in this view.                  */
/*****************************************************************************/
GnomeCanvasItem *
gl_view_object_get_group   (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));
	
	gl_debug (DEBUG_VIEW, "END");

	return view_object->private->group;
}

/*****************************************************************************/
/* Create canvas item for this object.                                       */
/*****************************************************************************/
GnomeCanvasItem *
gl_view_object_item_new (glViewObject *view_object,
			 GType         type,
			 const gchar  *first_arg_name,
			 ...)
{
	GnomeCanvasItem *item;
	va_list          args;
	gdouble          affine[6], delta_affine[6];

	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object), NULL);

	item = gnome_canvas_item_new (GNOME_CANVAS_GROUP(view_object->private->group),
				      type, NULL);
	
        va_start (args, first_arg_name);
        gnome_canvas_item_set_valist (item, first_arg_name, args);
        va_end (args);

	gl_label_object_get_affine (view_object->private->object, affine);

	/* Apply a very small rotation, fixes problems with flipped or rotated images */
	art_affine_rotate (delta_affine, DELTA_DEG);
	art_affine_multiply (affine, affine, delta_affine);
	
	gnome_canvas_item_affine_absolute (item, affine);

	gl_debug (DEBUG_VIEW, "END");

	return item;
}

/*****************************************************************************/
/* Highlight view of object.                                                 */
/*****************************************************************************/
void
gl_view_object_show_highlight     (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));
	g_return_if_fail (view_object->private->highlight);
	
	gl_view_highlight_show (view_object->private->highlight);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Remove highlight from view of object.                                     */
/*****************************************************************************/
void
gl_view_object_hide_highlight   (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object));
	g_return_if_fail (view_object->private->highlight);
	
	gl_view_highlight_hide (view_object->private->highlight);

	gl_debug (DEBUG_VIEW, "END");
}


/*****************************************************************************/
/* Get property editor.                                                      */
/*****************************************************************************/
GtkWidget *
gl_view_object_get_editor (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view_object && GL_IS_VIEW_OBJECT (view_object), NULL);

	if ( GL_VIEW_OBJECT_GET_CLASS(view_object)->construct_editor != NULL ) {

		if (view_object->private->property_editor == NULL) {
			view_object->private->property_editor =
				GL_VIEW_OBJECT_GET_CLASS(view_object)->construct_editor (view_object);
		}
		g_signal_connect (G_OBJECT (view_object->private->property_editor),
				  "destroy",
				  G_CALLBACK (gtk_widget_destroyed),
				  &view_object->private->property_editor);
	
		gtk_widget_show (view_object->private->property_editor);

	}


	gl_debug (DEBUG_VIEW, "END");

	return view_object->private->property_editor;
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Object moved callback.                                          */
/*---------------------------------------------------------------------------*/
static void
object_moved_cb (glLabelObject *object,
		 gdouble        dx,
		 gdouble        dy,
		 glViewObject  *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	/* Adjust location of analogous canvas group. */
	gnome_canvas_item_move (view_object->private->group, dx, dy);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  raise item to front callback.                                   */
/*---------------------------------------------------------------------------*/
static void
raise_object_cb (glLabelObject *object,
		 glViewObject  *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	/* send to top */
	gnome_canvas_item_raise_to_top (view_object->private->group);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  lower item to back callback.                                    */
/*---------------------------------------------------------------------------*/
static void
lower_object_cb (glLabelObject *object,
		 glViewObject  *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	/* Send to bottom */
	gnome_canvas_item_lower_to_bottom (view_object->private->group);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Flip/rotate object callback.                                    */
/*---------------------------------------------------------------------------*/
static void
flip_rotate_object_cb (glLabelObject *object,
		       glViewObject  *view_object)
{
	gdouble          affine[6], delta_affine[6];
	GList           *p, *item_list;
	GnomeCanvasItem *item;

	gl_debug (DEBUG_VIEW, "START");

	gl_label_object_get_affine (object, affine);

	/* Apply a very small rotation, fixes problems with flipped or rotated images */
	art_affine_rotate (delta_affine, DELTA_DEG);
	art_affine_multiply (affine, affine, delta_affine);
	
	item_list = GNOME_CANVAS_GROUP(view_object->private->group)->item_list;
	for ( p=item_list; p != NULL; p=p->next ) {
		item = GNOME_CANVAS_ITEM(p->data);
		gnome_canvas_item_affine_absolute (item, affine);
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Select object.                                                            */
/*****************************************************************************/
void
gl_view_object_select (glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	gl_view_select_object(view_object->private->view, view_object);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Item event handler.                                                       */
/*****************************************************************************/
gint
gl_view_object_item_event_cb (GnomeCanvasItem *item,
			      GdkEvent        *event,
			      glViewObject    *view_object)
{
	glView *view;

	gl_debug (DEBUG_VIEW, "");

	view = gl_view_object_get_view(view_object);
	switch (view->state) {

	case GL_VIEW_STATE_ARROW:
		return item_event_arrow_mode (item, event, view_object);

	default:
		return FALSE;

	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Item event handler (arrow mode)                                 */
/*---------------------------------------------------------------------------*/
static gint
item_event_arrow_mode (GnomeCanvasItem *item,
		       GdkEvent        *event,
		       glViewObject    *view_object)
{
	static gdouble x, y;
	static gboolean dragging = FALSE;
	glView *view;
	GdkCursor *cursor;
	gdouble item_x, item_y;
	gdouble new_x, new_y;
	gboolean control_key_pressed;

	gl_debug (DEBUG_VIEW, "");

	item_x = event->button.x;
	item_y = event->button.y;
	gnome_canvas_item_w2i (item->parent, &item_x, &item_y);

	view = gl_view_object_get_view(view_object);

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		gl_debug (DEBUG_VIEW, "BUTTON_PRESS");
		control_key_pressed = event->button.state & GDK_CONTROL_MASK;
		switch (event->button.button) {
		case 1:
			if (control_key_pressed) {
				if (gl_view_is_object_selected (view, view_object)) {
					/* Un-selecting a selected item */
					gl_view_unselect_object (view,
								 view_object);
					return TRUE;
				} else {
					/* Add to current selection */
					gl_view_select_object (view,
							       view_object);
				}
			} else {
				if (!gl_view_is_object_selected (view, view_object)) {
					/* No control, key so remove any selections before adding */
					gl_view_unselect_all (view);
					/* Add to current selection */
					gl_view_select_object (view,
							       view_object);
				}
			}
			/* Go into dragging mode while button remains pressed. */
			x = item_x;
			y = item_y;
			cursor = gdk_cursor_new (GDK_FLEUR);
			gnome_canvas_item_grab (item,
						GDK_POINTER_MOTION_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_BUTTON_PRESS_MASK,
						cursor, event->button.time);
			gdk_cursor_unref (cursor);
			dragging = TRUE;
			return TRUE;

		case 3:
			if (!gl_view_is_object_selected (view, view_object)) {
				if (!control_key_pressed) {
					/* No control, key so remove any selections before adding */
					gl_view_unselect_all (view);
				}
			}
			/* Add to current selection */
			gl_view_select_object (view, view_object);
			/* bring up apropriate menu for selection. */
			gl_view_popup_menu (view, event);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		gl_debug (DEBUG_VIEW, "BUTTON_RELEASE");
		switch (event->button.button) {
		case 1:
			/* Exit dragging mode */
			gnome_canvas_item_ungrab (item, event->button.time);
			dragging = FALSE;
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		gl_debug (DEBUG_VIEW, "MOTION_NOTIFY");
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			/* Dragging mode, move selection */
			new_x = item_x;
			new_y = item_y;
			gl_view_move_selection (view, (new_x - x), (new_y - y));
			x = new_x;
			y = new_y;
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_2BUTTON_PRESS:
		gl_debug (DEBUG_VIEW, "2BUTTON_PRESS");
		switch (event->button.button) {
		case 1:
			/* Also exit dragging mode w/ double-click */
			gnome_canvas_item_ungrab (item, event->button.time);
			dragging = FALSE;
			gl_view_select_object (view, view_object);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		gl_debug (DEBUG_VIEW, "ENTER_NOTIFY");
		cursor = gdk_cursor_new (GDK_FLEUR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		gl_debug (DEBUG_VIEW, "LEAVE_NOTIFY");
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	default:
		gl_debug (DEBUG_VIEW, "default");
		return FALSE;
	}

}

