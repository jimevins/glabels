/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_ellipse.c:  GLabels label ellipse object widget
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

#include "view-ellipse.h"

#include "view-highlight.h"

#include "wdgt-line.h"
#include "wdgt-fill.h"
#include "wdgt-size.h"
#include "wdgt-position.h"
#include "color.h"
#include "prefs.h"

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
	GnomeCanvasItem *item;

	/* Property dialog Page 0 widgets */
	GtkWidget             *line;
	GtkWidget             *fill;

	/* Property dialog Page 1 widgets */
	GtkWidget             *position;
	GtkWidget             *size;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static glViewObjectClass *parent_class = NULL;


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void      gl_view_ellipse_class_init    (glViewEllipseClass *klass);
static void      gl_view_ellipse_instance_init (glViewEllipse  *view_ellipse);
static void      gl_view_ellipse_finalize      (GObject        *object);

static void      update_view_ellipse_cb        (glLabelObject  *object,
						glViewEllipse  *view_ellipse);

static GtkWidget *construct_properties_dialog  (glViewObject   *view_object);

static void      response_cb                   (GtkDialog      *dialog,
						gint            response,
						glViewEllipse  *view_ellipse);

static void      line_changed_cb               (glWdgtLine     *line,
						glViewEllipse  *view_ellipse);

static void      fill_changed_cb               (glWdgtFill     *fill,
						glViewEllipse  *view_ellipse);

static void      position_changed_cb           (glWdgtPosition *position,
						glViewEllipse  *view_ellipse);

static void      size_changed_cb               (glWdgtSize     *size,
						glViewEllipse  *view_ellipse);

static void      update_dialog_cb              (glLabelObject  *object,
						glViewEllipse  *view_ellipse);

static void      update_dialog_from_move_cb    (glLabelObject  *object,
						gdouble         dx,
						gdouble         dy,
						glViewEllipse  *view_ellipse);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_view_ellipse_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (glViewEllipseClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_view_ellipse_class_init,
			NULL,
			NULL,
			sizeof (glViewEllipse),
			0,
			(GInstanceInitFunc) gl_view_ellipse_instance_init,
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

	view_object_class->construct_dialog = construct_properties_dialog;

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
		     glView     *view)
{
	glViewEllipse      *view_ellipse;
	gdouble            line_width;
	guint              line_color, fill_color;
	gdouble            w, h;
	GtkMenu            *menu;

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
	line_width = gl_label_ellipse_get_line_width(object);
	line_color = gl_label_ellipse_get_line_color(object);
	fill_color = gl_label_ellipse_get_fill_color(object);

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
			  G_CALLBACK (update_view_ellipse_cb), view_ellipse);

	gl_debug (DEBUG_VIEW, "END");

	return GL_VIEW_OBJECT (view_ellipse);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "changed" callback.                                 */
/*---------------------------------------------------------------------------*/
static void
update_view_ellipse_cb (glLabelObject *object,
			glViewEllipse     *view_ellipse)
{
	gdouble            line_width;
	guint              line_color, fill_color;
	gdouble            w, h;

	gl_debug (DEBUG_VIEW, "START");

	/* Query properties of object. */
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_ellipse_get_line_width(GL_LABEL_ELLIPSE(object));
	line_color = gl_label_ellipse_get_line_color(GL_LABEL_ELLIPSE(object));
	fill_color = gl_label_ellipse_get_fill_color(GL_LABEL_ELLIPSE(object));

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

/*****************************************************************************/
/* Create a properties dialog for a ellipse object.                          */
/*****************************************************************************/
static GtkWidget *
construct_properties_dialog (glViewObject *view_object)
{
	glViewEllipse      *view_ellipse = (glViewEllipse *)view_object;
	GtkWidget          *dialog, *wsection;
	glLabelObject      *object;
	gdouble            line_width;
	guint              line_color, fill_color;
	gdouble            x, y, w, h, label_width, label_height;
	GtkSizeGroup       *label_size_group;
	GtkWidget          *window;

	gl_debug (DEBUG_VIEW, "START");

	/* retrieve object and query parameters */
	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_ellipse));
	gl_label_object_get_position (GL_LABEL_OBJECT(object), &x, &y);
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_ellipse_get_line_width(GL_LABEL_ELLIPSE(object));
	line_color = gl_label_ellipse_get_line_color(GL_LABEL_ELLIPSE(object));
	fill_color = gl_label_ellipse_get_fill_color(GL_LABEL_ELLIPSE(object));
	gl_label_get_size (GL_LABEL(object->parent),
			   &label_width, &label_height);

	/*-----------------------------------------------------------------*/
	/* Build dialog with notebook.                                     */
	/*-----------------------------------------------------------------*/
	window = gtk_widget_get_toplevel (
		GTK_WIDGET(gl_view_object_get_view(GL_VIEW_OBJECT(view_ellipse))));
	dialog = gl_hig_dialog_new_with_buttons ( _("Edit ellipse object properties"),
						  GTK_WINDOW (window),
						  GTK_DIALOG_DESTROY_WITH_PARENT,
						  GTK_STOCK_CLOSE,
					                   GTK_RESPONSE_CLOSE,
						  NULL );
        gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK (response_cb), view_ellipse);

	label_size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);


	/*---------------------------*/
	/* Outline section           */
	/*---------------------------*/
	wsection = gl_hig_category_new (_("Outline"));
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dialog), wsection);
	view_ellipse->private->line = gl_wdgt_line_new ();
	gl_wdgt_line_set_label_size_group (GL_WDGT_LINE(view_ellipse->private->line),
					   label_size_group);
	gl_wdgt_line_set_params (GL_WDGT_LINE (view_ellipse->private->line),
				 line_width,
				 line_color);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wsection),
				    view_ellipse->private->line);
	g_signal_connect (G_OBJECT (view_ellipse->private->line), "changed",
			  G_CALLBACK(line_changed_cb), view_ellipse);

	/*---------------------------*/
	/* Fill section              */
	/*---------------------------*/
	wsection = gl_hig_category_new (_("Fill"));
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dialog), wsection);
	view_ellipse->private->fill = gl_wdgt_fill_new ();
	gl_wdgt_fill_set_label_size_group (GL_WDGT_FILL(view_ellipse->private->fill),
					   label_size_group);
	gl_wdgt_fill_set_params (GL_WDGT_FILL (view_ellipse->private->fill),
				 fill_color);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wsection),
				    view_ellipse->private->fill);
	g_signal_connect (G_OBJECT (view_ellipse->private->fill), "changed",
			  G_CALLBACK(fill_changed_cb), view_ellipse);


	/*---------------------------*/
	/* Position section          */
	/*---------------------------*/
	wsection = gl_hig_category_new (_("Position"));
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dialog), wsection);
	view_ellipse->private->position = gl_wdgt_position_new ();
	gl_wdgt_position_set_label_size_group (GL_WDGT_POSITION(view_ellipse->private->position),
					       label_size_group);
	gl_wdgt_position_set_params (GL_WDGT_POSITION (view_ellipse->private->position),
				     x, y, label_width, label_height);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wsection),
				    view_ellipse->private->position);
	g_signal_connect (G_OBJECT (view_ellipse->private->position), "changed",
			  G_CALLBACK(position_changed_cb), view_ellipse);

	/*---------------------------*/
	/* Size section              */
	/*---------------------------*/
	wsection = gl_hig_category_new (_("Size"));
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dialog), wsection);
	view_ellipse->private->size = gl_wdgt_size_new ();
	gl_wdgt_size_set_label_size_group (GL_WDGT_SIZE(view_ellipse->private->size),
					   label_size_group);
	gl_wdgt_size_set_params (GL_WDGT_SIZE (view_ellipse->private->size),
				 w, h, TRUE, label_width, label_height);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wsection),
				    view_ellipse->private->size);
	g_signal_connect (G_OBJECT (view_ellipse->private->size), "changed",
			  G_CALLBACK(size_changed_cb), view_ellipse);


	/*----------------------------*/
	/* Track object changes.      */
	/*----------------------------*/
	g_signal_connect (G_OBJECT (object), "changed",
			  G_CALLBACK (update_dialog_cb), view_ellipse);
	g_signal_connect (G_OBJECT (object), "moved",
			  G_CALLBACK (update_dialog_from_move_cb),
			  view_ellipse);

	gl_debug (DEBUG_VIEW, "END");

	return dialog;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Response" callback.                                            */
/*---------------------------------------------------------------------------*/
static void
response_cb (GtkDialog     *dialog,
	     gint          response,
	     glViewEllipse *view_ellipse)
{
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail(dialog != NULL);
	g_return_if_fail(GTK_IS_DIALOG(dialog));

	switch(response) {
	case GTK_RESPONSE_CLOSE:
		gtk_widget_hide (GTK_WIDGET(dialog));
		break;
	case GTK_RESPONSE_DELETE_EVENT:
		/* Dialog destroyed, remove callbacks that reference it. */
		object = gl_view_object_get_object (GL_VIEW_OBJECT(view_ellipse));

		g_signal_handlers_disconnect_by_func (object, update_dialog_cb,
						      view_ellipse);
		g_signal_handlers_disconnect_by_func (object, update_dialog_from_move_cb,
						      view_ellipse);
		break;
	default:
		g_print ("response = %d", response);
		g_assert_not_reached();
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  line properties "changed" callback.                             */
/*---------------------------------------------------------------------------*/
static void
line_changed_cb (glWdgtLine     *line,
		 glViewEllipse  *view_ellipse)
{
	glLabelObject      *object;
	gdouble            line_width;
	guint              line_color;

	gl_debug (DEBUG_VIEW, "START");

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_ellipse));

	gl_wdgt_line_get_params (GL_WDGT_LINE (line),
				 &line_width,
				 &line_color);

	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_dialog_cb, view_ellipse);
	gl_label_ellipse_set_line_width(GL_LABEL_ELLIPSE(object), line_width);
	gl_label_ellipse_set_line_color(GL_LABEL_ELLIPSE(object), line_color);
	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_dialog_cb, view_ellipse);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  fill properties "changed" callback.                             */
/*---------------------------------------------------------------------------*/
static void
fill_changed_cb (glWdgtFill     *fill,
		 glViewEllipse  *view_ellipse)
{
	glLabelObject    *object;
	guint            fill_color;

	gl_debug (DEBUG_VIEW, "START");

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_ellipse));

	gl_wdgt_fill_get_params (GL_WDGT_FILL (fill),
				 &fill_color);

	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_dialog_cb, view_ellipse);
	gl_label_ellipse_set_fill_color(GL_LABEL_ELLIPSE(object), fill_color);
	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_dialog_cb, view_ellipse);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  position "changed" callback.                                    */
/*---------------------------------------------------------------------------*/
static void
position_changed_cb (glWdgtPosition     *position,
		     glViewEllipse      *view_ellipse)
{
	glLabelObject      *object;
	gdouble            x, y;

	gl_debug (DEBUG_VIEW, "START");

	gl_wdgt_position_get_position (GL_WDGT_POSITION (position), &x, &y);

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_ellipse));

	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_dialog_cb, view_ellipse);
	gl_label_object_set_position (GL_LABEL_OBJECT(object), x, y);
	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_dialog_cb, view_ellipse);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  size "changed" callback.                                        */
/*---------------------------------------------------------------------------*/
static void
size_changed_cb (glWdgtSize     *size,
		 glViewEllipse  *view_ellipse)
{
	glLabelObject *object;
	gdouble       w, h;
	gboolean      keep_aspect_ratio_flag;

	gl_debug (DEBUG_VIEW, "START");

	gl_wdgt_size_get_size (GL_WDGT_SIZE (size),
			       &w, &h, &keep_aspect_ratio_flag);

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_ellipse));

	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_dialog_cb, view_ellipse);
	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);
	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_dialog_cb, view_ellipse);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "changed" callback.                                 */
/*---------------------------------------------------------------------------*/
static void
update_dialog_cb (glLabelObject     *object,
		  glViewEllipse     *view_ellipse)
{
	gdouble            line_width;
	guint              line_color, fill_color;
	gdouble            x, y, w, h;

	gl_debug (DEBUG_VIEW, "START");

	/* Query properties of object. */
	gl_label_object_get_position (GL_LABEL_OBJECT(object), &x, &y);
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_ellipse_get_line_width(GL_LABEL_ELLIPSE(object));
	line_color = gl_label_ellipse_get_line_color(GL_LABEL_ELLIPSE(object));
	fill_color = gl_label_ellipse_get_fill_color(GL_LABEL_ELLIPSE(object));

	/* Block widget handlers to prevent recursion */
	g_signal_handlers_block_by_func (G_OBJECT(view_ellipse->private->line),
					 line_changed_cb, view_ellipse);
	g_signal_handlers_block_by_func (G_OBJECT(view_ellipse->private->fill),
					 fill_changed_cb, view_ellipse);
	g_signal_handlers_block_by_func (G_OBJECT(view_ellipse->private->position),
					 position_changed_cb, view_ellipse);
	g_signal_handlers_block_by_func (G_OBJECT(view_ellipse->private->size),
					 size_changed_cb, view_ellipse);

	/* Update widgets in property dialog */
	gl_wdgt_line_set_params (GL_WDGT_LINE (view_ellipse->private->line),
				 line_width,
				 line_color);
	gl_wdgt_fill_set_params (GL_WDGT_FILL (view_ellipse->private->fill),
				 fill_color);
	gl_wdgt_position_set_position (GL_WDGT_POSITION(view_ellipse->private->position),
				       x, y);
	gl_wdgt_size_set_size (GL_WDGT_SIZE(view_ellipse->private->size), w, h);

	/* Unblock widget handlers */
	g_signal_handlers_unblock_by_func (G_OBJECT(view_ellipse->private->line),
					   line_changed_cb, view_ellipse);
	g_signal_handlers_unblock_by_func (G_OBJECT(view_ellipse->private->fill),
					   fill_changed_cb, view_ellipse);
	g_signal_handlers_unblock_by_func (G_OBJECT(view_ellipse->private->position),
					   position_changed_cb, view_ellipse);
	g_signal_handlers_unblock_by_func (G_OBJECT(view_ellipse->private->size),
					   size_changed_cb, view_ellipse);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "moved" callback.                                   */
/*---------------------------------------------------------------------------*/
static void
update_dialog_from_move_cb (glLabelObject *object,
			    gdouble        dx,
			    gdouble        dy,
			    glViewEllipse *view_ellipse)
{
	gdouble            x, y;

	gl_debug (DEBUG_VIEW, "START");

	/* Query properties of object. */
	gl_label_object_get_position (GL_LABEL_OBJECT(object), &x, &y);

	/* Block widget handlers to prevent recursion */
	g_signal_handlers_block_by_func (G_OBJECT(view_ellipse->private->position),
					 position_changed_cb, view_ellipse);

	/* Update widgets in property dialog */
	gl_wdgt_position_set_position (GL_WDGT_POSITION(view_ellipse->private->position),
				       x, y);

	/* Unblock widget handlers */
	g_signal_handlers_unblock_by_func (G_OBJECT(view_ellipse->private->position),
					   position_changed_cb, view_ellipse);

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
	static gdouble      x0, y0;
	static gboolean     dragging = FALSE;
	static glViewObject *view_ellipse;
	static GObject      *object;
	guint               line_color, fill_color;
	gdouble             x, y, w, h;

	gl_debug (DEBUG_VIEW, "");

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			dragging = TRUE;
			gnome_canvas_item_grab (canvas->root,
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
			line_color = gl_color_set_opacity (gl_prefs->default_line_color, 0.5);
			fill_color = gl_color_set_opacity (gl_prefs->default_fill_color, 0.5);
			gl_label_ellipse_set_line_width (GL_LABEL_ELLIPSE(object),
						     gl_prefs->default_line_width);
			gl_label_ellipse_set_line_color (GL_LABEL_ELLIPSE(object),
						     line_color);
			gl_label_ellipse_set_fill_color (GL_LABEL_ELLIPSE(object),
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
			gl_label_ellipse_set_line_color (GL_LABEL_ELLIPSE(object),
						     gl_prefs->default_line_color);
			gl_label_ellipse_set_fill_color (GL_LABEL_ELLIPSE(object),
						     gl_prefs->default_fill_color);
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
