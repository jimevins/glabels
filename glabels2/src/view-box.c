/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_box.c:  GLabels label box object widget
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

#include "view-box.h"

#include "view-highlight.h"

#include "glabels.h"
#include "wdgt-line.h"
#include "wdgt-fill.h"
#include "wdgt-size.h"
#include "wdgt-position.h"
#include "color.h"
#include "prefs.h"

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
	GnomeCanvasItem       *item;

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

static void      gl_view_box_class_init       (glViewBoxClass *klass);
static void      gl_view_box_instance_init    (glViewBox      *view_box);
static void      gl_view_box_finalize         (GObject        *object);

static void      update_view_box_cb           (glLabelObject  *object,
					       glViewBox      *view_box);

static GtkWidget *construct_properties_dialog (glViewBox      *view_box);

static void      response_cb                  (GtkDialog      *dialog,
					       gint            response,
					       glViewBox      *view_box);

static void      line_changed_cb              (glWdgtLine     *line,
					       glViewBox      *view_box);

static void      fill_changed_cb              (glWdgtFill     *fill,
					       glViewBox      *view_box);

static void      position_changed_cb          (glWdgtPosition *position,
					       glViewBox      *view_box);

static void      size_changed_cb              (glWdgtSize     *size,
					       glViewBox      *view_box);

static void      update_dialog_cb             (glLabelObject  *object,
					       glViewBox      *view_box);

static void      update_dialog_from_move_cb   (glLabelObject  *object,
					       gdouble         dx,
					       gdouble         dy,
					       glViewBox      *view_box);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_view_box_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (glViewBoxClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_view_box_class_init,
			NULL,
			NULL,
			sizeof (glViewBox),
			0,
			(GInstanceInitFunc) gl_view_box_instance_init,
		};

		type = g_type_register_static (GL_TYPE_VIEW_OBJECT,
					       "glViewBox", &info, 0);
	}

	return type;
}

static void
gl_view_box_class_init (glViewBoxClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	gl_debug (DEBUG_VIEW, "START");

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gl_view_box_finalize;

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_box_instance_init (glViewBox *view_box)
{
	gl_debug (DEBUG_VIEW, "START");

	view_box->private = g_new0 (glViewBoxPrivate, 1);

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_box_finalize (GObject *object)
{
	glLabel       *parent;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (object && GL_IS_VIEW_BOX (object));

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* NEW box object view.                                                      */
/*****************************************************************************/
glViewObject *
gl_view_box_new (glLabelBox *object,
		 glView     *view)
{
	glViewBox          *view_box;
	GnomeCanvasItem    *group;
	gdouble            line_width;
	guint              line_color, fill_color;
	gdouble            w, h;
	GtkMenu            *menu;
	GtkWidget          *dialog;

	gl_debug (DEBUG_VIEW, "START");
	g_return_if_fail (object && GL_IS_LABEL_BOX (object));
	g_return_if_fail (view && GL_IS_VIEW (view));
	
	view_box = g_object_new (gl_view_box_get_type(), NULL);

	gl_view_object_set_view (GL_VIEW_OBJECT(view_box), view);
	gl_view_object_set_object (GL_VIEW_OBJECT(view_box),
				   GL_LABEL_OBJECT(object),
				   GL_VIEW_HIGHLIGHT_BOX_RESIZABLE);

	/* Query properties of object. */
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_box_get_line_width(object);
	line_color = gl_label_box_get_line_color(object);
	fill_color = gl_label_box_get_fill_color(object);

	/* Create analogous canvas item. */
	group = gl_view_object_get_group (GL_VIEW_OBJECT(view_box));
	view_box->private->item =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP(group),
				       gnome_canvas_rect_get_type (),
				       "x1", 0.0,
				       "y1", 0.0,
				       "x2", w + DELTA,
				       "y2", h + DELTA,
				       "width_units", line_width,
				       "outline_color_rgba", line_color,
				       "fill_color_rgba", fill_color,
				       NULL);

	g_signal_connect (G_OBJECT (object), "changed",
			  G_CALLBACK (update_view_box_cb), view_box);

	/* Create a dialog for controlling/viewing object properties. */
	dialog = construct_properties_dialog (view_box);
	gl_view_object_set_dialog     (GL_VIEW_OBJECT(view_box), dialog);

	gl_debug (DEBUG_VIEW, "END");

	return GL_VIEW_OBJECT (view_box);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "changed" callback.                                 */
/*---------------------------------------------------------------------------*/
static void
update_view_box_cb (glLabelObject *object,
		    glViewBox     *view_box)
{
	gdouble            line_width;
	guint              line_color, fill_color;
	gdouble            w, h;

	gl_debug (DEBUG_VIEW, "START");

	/* Query properties of object. */
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_box_get_line_width(GL_LABEL_BOX(object));
	line_color = gl_label_box_get_line_color(GL_LABEL_BOX(object));
	fill_color = gl_label_box_get_fill_color(GL_LABEL_BOX(object));

	/* Adjust appearance of analogous canvas item. */
	gnome_canvas_item_set (view_box->private->item,
			       "x2", w + DELTA,
			       "y2", h + DELTA,
			       "width_units", line_width,
			       "outline_color_rgba", line_color,
			       "fill_color_rgba", fill_color,
			       NULL);

	/* Adjust highlight */
	gl_view_object_update_highlight (GL_VIEW_OBJECT(view_box));

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Create a properties dialog for a box object.                              */
/*****************************************************************************/
static GtkWidget *
construct_properties_dialog (glViewBox *view_box)
{
	GtkWidget          *dialog, *wsection;
	BonoboWindow       *win = glabels_get_active_window ();
	glLabelObject      *object;
	gdouble            line_width;
	guint              line_color, fill_color;
	gdouble            x, y, w, h, label_width, label_height;
	GtkSizeGroup       *label_size_group;

	gl_debug (DEBUG_VIEW, "START");

	/* retrieve object and query parameters */
	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_box));
	gl_label_object_get_position (GL_LABEL_OBJECT(object), &x, &y);
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_box_get_line_width(GL_LABEL_BOX(object));
	line_color = gl_label_box_get_line_color(GL_LABEL_BOX(object));
	fill_color = gl_label_box_get_fill_color(GL_LABEL_BOX(object));
	gl_label_get_size (GL_LABEL(object->parent),
			   &label_width, &label_height);

	/*-----------------------------------------------------------------*/
	/* Build dialog with notebook.                                     */
	/*-----------------------------------------------------------------*/
	dialog = gl_hig_dialog_new_with_buttons ( _("Edit box object properties"),
						  GTK_WINDOW (win),
						  GTK_DIALOG_DESTROY_WITH_PARENT,
						  GTK_STOCK_CLOSE,
					                   GTK_RESPONSE_CLOSE,
						  NULL );
        gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK (response_cb), view_box);

	label_size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);


	/*---------------------------*/
	/* Outline section           */
	/*---------------------------*/
	wsection = gl_hig_category_new (_("Outline"));
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dialog), wsection);
	view_box->private->line = gl_wdgt_line_new ();
	gl_wdgt_line_set_label_size_group (GL_WDGT_LINE(view_box->private->line),
					   label_size_group);
	gl_wdgt_line_set_params (GL_WDGT_LINE (view_box->private->line),
				 line_width,
				 line_color);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wsection),
				    view_box->private->line);
	g_signal_connect (G_OBJECT (view_box->private->line), "changed",
			  G_CALLBACK(line_changed_cb), view_box);

	/*---------------------------*/
	/* Fill section              */
	/*---------------------------*/
	wsection = gl_hig_category_new (_("Fill"));
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dialog), wsection);
	view_box->private->fill = gl_wdgt_fill_new ();
	gl_wdgt_fill_set_label_size_group (GL_WDGT_FILL(view_box->private->fill),
					   label_size_group);
	gl_wdgt_fill_set_params (GL_WDGT_FILL (view_box->private->fill),
				 fill_color);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wsection),
				    view_box->private->fill);
	g_signal_connect (G_OBJECT (view_box->private->fill), "changed",
			  G_CALLBACK(fill_changed_cb), view_box);


	/*---------------------------*/
	/* Position section          */
	/*---------------------------*/
	wsection = gl_hig_category_new (_("Position"));
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dialog), wsection);
	view_box->private->position = gl_wdgt_position_new ();
	gl_wdgt_position_set_label_size_group (GL_WDGT_POSITION(view_box->private->position),
					       label_size_group);
	gl_wdgt_position_set_params (GL_WDGT_POSITION (view_box->private->position),
				     x, y, label_width, label_height);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wsection),
				    view_box->private->position);
	g_signal_connect (G_OBJECT (view_box->private->position), "changed",
			  G_CALLBACK(position_changed_cb), view_box);

	/*---------------------------*/
	/* Size section              */
	/*---------------------------*/
	wsection = gl_hig_category_new (_("Size"));
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dialog), wsection);
	view_box->private->size = gl_wdgt_size_new ();
	gl_wdgt_size_set_label_size_group (GL_WDGT_SIZE(view_box->private->size),
					   label_size_group);
	gl_wdgt_size_set_params (GL_WDGT_SIZE (view_box->private->size),
				 w, h, TRUE, label_width, label_height);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wsection),
				    view_box->private->size);
	g_signal_connect (G_OBJECT (view_box->private->size), "changed",
			  G_CALLBACK(size_changed_cb), view_box);


	/*----------------------------*/
	/* Track object changes.      */
	/*----------------------------*/
	g_signal_connect (G_OBJECT (object), "changed",
			  G_CALLBACK (update_dialog_cb), view_box);
	g_signal_connect (G_OBJECT (object), "moved",
			  G_CALLBACK (update_dialog_from_move_cb), view_box);

	gl_debug (DEBUG_VIEW, "END");

	return dialog;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Response" callback.                                            */
/*---------------------------------------------------------------------------*/
static void
response_cb (GtkDialog *dialog,
	     gint      response,
	     glViewBox *view_box)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail(dialog != NULL);
	g_return_if_fail(GTK_IS_DIALOG(dialog));

	switch(response) {
	case GTK_RESPONSE_CLOSE:
		gtk_widget_hide (GTK_WIDGET(dialog));
		break;
	default:
		g_assert_not_reached();
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  line properties "changed" callback.                             */
/*---------------------------------------------------------------------------*/
static void
line_changed_cb (glWdgtLine *line,
		 glViewBox  *view_box)
{
	glLabelObject      *object;
	gdouble            line_width;
	guint              line_color;

	gl_debug (DEBUG_VIEW, "START");

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_box));

	gl_wdgt_line_get_params (GL_WDGT_LINE (line),
				 &line_width,
				 &line_color);

	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_dialog_cb, view_box);
	gl_label_box_set_line_width(GL_LABEL_BOX(object), line_width);
	gl_label_box_set_line_color(GL_LABEL_BOX(object), line_color);
	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_dialog_cb, view_box);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  fill properties "changed" callback.                             */
/*---------------------------------------------------------------------------*/
static void
fill_changed_cb (glWdgtFill *fill,
		 glViewBox  *view_box)
{
	glLabelObject    *object;
	guint            fill_color;

	gl_debug (DEBUG_VIEW, "START");

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_box));

	gl_wdgt_fill_get_params (GL_WDGT_FILL (fill),
				 &fill_color);

	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_dialog_cb, view_box);
	gl_label_box_set_fill_color(GL_LABEL_BOX(object), fill_color);
	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_dialog_cb, view_box);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  position "changed" callback.                                    */
/*---------------------------------------------------------------------------*/
static void
position_changed_cb (glWdgtPosition *position,
		     glViewBox      *view_box)
{
	glLabelObject      *object;
	gdouble            x, y;

	gl_debug (DEBUG_VIEW, "START");

	gl_wdgt_position_get_position (GL_WDGT_POSITION (position), &x, &y);

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_box));

	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_dialog_cb, view_box);
	gl_label_object_set_position (GL_LABEL_OBJECT(object), x, y);
	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_dialog_cb, view_box);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  size "changed" callback.                                        */
/*---------------------------------------------------------------------------*/
static void
size_changed_cb (glWdgtSize *size,
		 glViewBox  *view_box)
{
	glLabelObject *object;
	gdouble       w, h;
	gboolean      keep_aspect_ratio_flag;

	gl_debug (DEBUG_VIEW, "START");

	gl_wdgt_size_get_size (GL_WDGT_SIZE (size),
			       &w, &h, &keep_aspect_ratio_flag);

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_box));

	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_dialog_cb, view_box);
	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);
	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_dialog_cb, view_box);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "changed" callback.                                 */
/*---------------------------------------------------------------------------*/
static void
update_dialog_cb (glLabelObject *object,
		  glViewBox     *view_box)
{
	gdouble            line_width;
	guint              line_color, fill_color;
	gdouble            x, y, w, h;

	gl_debug (DEBUG_VIEW, "START");

	/* Query properties of object. */
	gl_label_object_get_position (GL_LABEL_OBJECT(object), &x, &y);
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	line_width = gl_label_box_get_line_width(GL_LABEL_BOX(object));
	line_color = gl_label_box_get_line_color(GL_LABEL_BOX(object));
	fill_color = gl_label_box_get_fill_color(GL_LABEL_BOX(object));

	/* Block widget handlers to prevent recursion */
	g_signal_handlers_block_by_func (G_OBJECT(view_box->private->line),
					 line_changed_cb, view_box);
	g_signal_handlers_block_by_func (G_OBJECT(view_box->private->fill),
					 fill_changed_cb, view_box);
	g_signal_handlers_block_by_func (G_OBJECT(view_box->private->position),
					 position_changed_cb, view_box);
	g_signal_handlers_block_by_func (G_OBJECT(view_box->private->size),
					 size_changed_cb, view_box);

	/* Update widgets in property dialog */
	gl_wdgt_line_set_params (GL_WDGT_LINE (view_box->private->line),
				 line_width,
				 line_color);
	gl_wdgt_fill_set_params (GL_WDGT_FILL (view_box->private->fill),
				 fill_color);
	gl_wdgt_position_set_position (GL_WDGT_POSITION(view_box->private->position),
				       x, y);
	gl_wdgt_size_set_size (GL_WDGT_SIZE(view_box->private->size), w, h);

	/* Unblock widget handlers */
	g_signal_handlers_unblock_by_func (G_OBJECT(view_box->private->line),
					   line_changed_cb, view_box);
	g_signal_handlers_unblock_by_func (G_OBJECT(view_box->private->fill),
					   fill_changed_cb, view_box);
	g_signal_handlers_unblock_by_func (G_OBJECT(view_box->private->position),
					   position_changed_cb, view_box);
	g_signal_handlers_unblock_by_func (G_OBJECT(view_box->private->size),
					   size_changed_cb, view_box);


	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "moved" callback.                                   */
/*---------------------------------------------------------------------------*/
static void
update_dialog_from_move_cb (glLabelObject *object,
			    gdouble        dx,
			    gdouble        dy,
			    glViewBox     *view_box)
{
	gdouble            x, y;

	gl_debug (DEBUG_VIEW, "START");

	/* Query properties of object. */
	gl_label_object_get_position (GL_LABEL_OBJECT(object), &x, &y);

	/* Block widget handlers to prevent recursion */
	g_signal_handlers_block_by_func (G_OBJECT(view_box->private->position),
					 position_changed_cb, view_box);

	/* Update widgets in property dialog */
	gl_wdgt_position_set_position (GL_WDGT_POSITION(view_box->private->position),
				       x, y);

	/* Unblock widget handlers */
	g_signal_handlers_unblock_by_func (G_OBJECT(view_box->private->position),
					   position_changed_cb, view_box);

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
							   cursor_box_bits,
							   cursor_box_width,
							   cursor_box_height);
		pixmap_mask = gdk_bitmap_create_from_data (NULL,
							   cursor_box_mask_bits,
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
	static gdouble      x0, y0;
	static gboolean     dragging = FALSE;
	static glViewObject *view_box;
	static GObject      *object;
	guint               line_color, fill_color;
	gdouble             x, y, w, h;

	gl_debug (DEBUG_VIEW, "");

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			dragging = TRUE;
			gdk_pointer_grab (GTK_WIDGET (view->canvas)->window,
					  FALSE,
					  GDK_POINTER_MOTION_MASK |
					  GDK_BUTTON_RELEASE_MASK |
					  GDK_BUTTON_PRESS_MASK,
					  NULL, NULL, event->button.time);
			gnome_canvas_window_to_world (canvas,
						      event->button.x,
						      event->button.y, &x, &y);
			object = gl_label_box_new (view->label);
			gl_label_object_set_position (GL_LABEL_OBJECT(object),
						     x, y);
			gl_label_object_set_size (GL_LABEL_OBJECT(object),
						  0.0, 0.0);
			line_color = gl_color_set_opacity (gl_prefs->default_line_color, 0.5);
			fill_color = gl_color_set_opacity (gl_prefs->default_fill_color, 0.5);
			gl_label_box_set_line_width (GL_LABEL_BOX(object),
						     gl_prefs->default_line_width);
			gl_label_box_set_line_color (GL_LABEL_BOX(object),
						     line_color);
			gl_label_box_set_fill_color (GL_LABEL_BOX(object),
						     fill_color);
			view_box = gl_view_box_new (GL_LABEL_BOX(object),
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
			gdk_pointer_ungrab (event->button.time);
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
			gl_label_box_set_line_color (GL_LABEL_BOX(object),
						     gl_prefs->default_line_color);
			gl_label_box_set_fill_color (GL_LABEL_BOX(object),
						     gl_prefs->default_fill_color);
			gl_view_unselect_all (view);
			gl_view_object_select (GL_VIEW_OBJECT(view_box));
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
