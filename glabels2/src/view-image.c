/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_image.c:  GLabels label image object widget
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

#include "view-image.h"

#include "view-highlight.h"

#include "wdgt-image-select.h"
#include "wdgt-line.h"
#include "wdgt-fill.h"
#include "wdgt-size.h"
#include "wdgt-position.h"

#include "pixmaps/cursor_image.xbm"
#include "pixmaps/cursor_image_mask.xbm"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

#define DELTA 0.01

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glViewImagePrivate {
	GnomeCanvasItem *item;

	/* Page 0 widgets */
	GtkWidget       *pixmap_entry;

	/* Page 1 widgets */
	GtkWidget       *position;
	GtkWidget       *size;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static glViewObjectClass *parent_class = NULL;

/* Save state of image file entry */
static gchar *image_path = NULL;


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void      gl_view_image_class_init      (glViewImageClass *klass);
static void      gl_view_image_instance_init   (glViewImage    *view_image);
static void      gl_view_image_finalize        (GObject        *object);

static void      update_view_image_cb          (glLabelObject  *object,
						glViewImage    *view_image);

static GtkWidget *construct_properties_dialog  (glViewObject   *view_object);

static void      response_cb                   (GtkDialog      *dialog,
						gint            response,
						glViewImage    *view_image);

static void      file_changed_cb               (GtkEntry       *pixmap_entry,
						glViewImage    *view_image);

static void      position_changed_cb           (glWdgtPosition *position,
						glViewImage    *view_image);

static void      size_changed_cb               (glWdgtSize     *size,
						glViewImage    *view_image);

static void      size_reset_cb                 (GtkButton      *button,
						glViewImage    *view_image);

static void      update_dialog_cb              (glLabelObject  *object,
						glViewImage    *view_image);

static void      update_dialog_from_move_cb    (glLabelObject  *object,
						gdouble         dx,
						gdouble         dy,
						glViewImage    *view_image);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_view_image_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (glViewImageClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_view_image_class_init,
			NULL,
			NULL,
			sizeof (glViewImage),
			0,
			(GInstanceInitFunc) gl_view_image_instance_init,
		};

		type = g_type_register_static (GL_TYPE_VIEW_OBJECT,
					       "glViewImage", &info, 0);
	}

	return type;
}

static void
gl_view_image_class_init (glViewImageClass *klass)
{
	GObjectClass      *object_class      = (GObjectClass *) klass;
	glViewObjectClass *view_object_class = (glViewObjectClass *) klass;

	gl_debug (DEBUG_VIEW, "START");

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gl_view_image_finalize;

	view_object_class->construct_dialog = construct_properties_dialog;

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_image_instance_init (glViewImage *view_image)
{
	gl_debug (DEBUG_VIEW, "START");

	view_image->private = g_new0 (glViewImagePrivate, 1);

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_image_finalize (GObject *object)
{
	glLabel       *parent;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (object && GL_IS_VIEW_IMAGE (object));

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* NEW image object view.                                                  */
/*****************************************************************************/
glViewObject *
gl_view_image_new (glLabelImage *object,
		   glView       *view)
{
	glViewImage        *view_image;
	const GdkPixbuf    *pixbuf;
	gdouble            w, h;
	GtkMenu            *menu;

	gl_debug (DEBUG_VIEW, "START");
	g_return_if_fail (object && GL_IS_LABEL_IMAGE (object));
	g_return_if_fail (view && GL_IS_VIEW (view));
	
	view_image = g_object_new (gl_view_image_get_type(), NULL);

	gl_view_object_set_view (GL_VIEW_OBJECT(view_image), view);
	gl_view_object_set_object (GL_VIEW_OBJECT(view_image),
				   GL_LABEL_OBJECT(object),
				   GL_VIEW_HIGHLIGHT_BOX_RESIZABLE);

	/* Query properties of object. */
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	pixbuf = gl_label_image_get_pixbuf(object, NULL);

	/* Create analogous canvas item. */
	view_image->private->item =
		gl_view_object_item_new (GL_VIEW_OBJECT(view_image),
					 gnome_canvas_pixbuf_get_type (),
					 "x", 0.0,
					 "y", 0.0,
					 "width_set", TRUE,
					 "height_set", TRUE,
					 "width", w,
					 "height", h,
					 "pixbuf", pixbuf,
					 NULL);

	g_signal_connect (G_OBJECT (object), "changed",
			  G_CALLBACK (update_view_image_cb), view_image);

	gl_debug (DEBUG_VIEW, "END");

	return GL_VIEW_OBJECT (view_image);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "changed" callback.                                 */
/*---------------------------------------------------------------------------*/
static void
update_view_image_cb (glLabelObject *object,
		      glViewImage   *view_image)
{
	const GdkPixbuf    *pixbuf;
	gdouble            w, h;

	gl_debug (DEBUG_VIEW, "START");

	/* Query properties of object. */
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	pixbuf = gl_label_image_get_pixbuf (GL_LABEL_IMAGE(object), NULL);

	/* Adjust appearance of analogous canvas item. */
	gnome_canvas_item_set (view_image->private->item,
			       "width_set", TRUE,
			       "height_set", TRUE,
			       "width", w,
			       "height", h,
			       "pixbuf", pixbuf,
			       NULL);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Create a properties dialog for a image object.                          */
/*****************************************************************************/
static GtkWidget *
construct_properties_dialog (glViewObject *view_object)
{
	glViewImage        *view_image = (glViewImage *)view_object;
	GtkWidget          *dialog, *wsection, *wbutton;
	glLabelObject      *object;
	gdouble            x, y, w, h, label_width, label_height;
	glTextNode         *filename;
	glMerge            *merge;
	GtkSizeGroup       *label_size_group;
	GtkWidget          *window;

	gl_debug (DEBUG_VIEW, "START");

	/* retrieve object and query parameters */
	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_image));
	gl_label_object_get_position (GL_LABEL_OBJECT(object), &x, &y);
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	filename = gl_label_image_get_filename (GL_LABEL_IMAGE(object));
	gl_label_get_size (GL_LABEL(object->parent),
			   &label_width, &label_height);
	merge = gl_label_get_merge (GL_LABEL(object->parent));

	/*-----------------------------------------------------------------*/
	/* Build dialog.                                                   */
	/*-----------------------------------------------------------------*/
	window = gtk_widget_get_toplevel (
		GTK_WIDGET(gl_view_object_get_view(GL_VIEW_OBJECT(view_image))));
	dialog = gl_hig_dialog_new_with_buttons ( _("Edit image object properties"),
						  GTK_WINDOW (window),
						  GTK_DIALOG_DESTROY_WITH_PARENT,
						  GTK_STOCK_CLOSE,
					                   GTK_RESPONSE_CLOSE,
						  NULL );
        gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK (response_cb), view_image);

	label_size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	/*---------------------------*/
	/* Image section             */
	/*---------------------------*/
	wsection = gl_hig_category_new (_("Image"));
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dialog), wsection);

	view_image->private->pixmap_entry =
		gl_wdgt_image_select_new (merge, filename);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wsection),
				    view_image->private->pixmap_entry);
	g_signal_connect ( G_OBJECT(view_image->private->pixmap_entry), "changed",
			   G_CALLBACK (file_changed_cb), view_image);


	/*----------------------------*/
	/* Position section           */
	/*----------------------------*/
	wsection = gl_hig_category_new (_("Position"));
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dialog), wsection);
	view_image->private->position = gl_wdgt_position_new ();
	gl_wdgt_position_set_label_size_group (GL_WDGT_POSITION(view_image->private->position),
					       label_size_group);
	gl_wdgt_position_set_params (GL_WDGT_POSITION (view_image->private->position),
				     x, y, label_width, label_height);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wsection),
				    view_image->private->position);
	g_signal_connect (G_OBJECT (view_image->private->position),
			  "changed",
			  G_CALLBACK(position_changed_cb), view_image);


	/*----------------------------*/
	/* Size section               */
	/*----------------------------*/
	wsection = gl_hig_category_new (_("Size"));
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dialog), wsection);
	view_image->private->size = gl_wdgt_size_new ();
	gl_wdgt_size_set_label_size_group (GL_WDGT_SIZE(view_image->private->size),
					       label_size_group);
	gl_wdgt_size_set_params (GL_WDGT_SIZE (view_image->private->size),
				 w, h, TRUE, label_width, label_height);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wsection),
				    view_image->private->size);
	g_signal_connect (G_OBJECT (view_image->private->size), "changed",
			  G_CALLBACK(size_changed_cb), view_image);

	/* ------ Size Reset Button ------ */
	wbutton = gtk_button_new_with_label (_("Reset image size"));
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wsection), wbutton);
	g_signal_connect (G_OBJECT (wbutton), "clicked",
			  G_CALLBACK (size_reset_cb), view_image);


	/*----------------------------*/
	/* Track object changes.      */
	/*----------------------------*/
	g_signal_connect (G_OBJECT (object), "changed",
			  G_CALLBACK (update_dialog_cb), view_image);
	g_signal_connect (G_OBJECT (object), "moved",
			  G_CALLBACK (update_dialog_from_move_cb), view_image);

	gl_debug (DEBUG_VIEW, "END");

	return dialog;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Response" callback.                                            */
/*---------------------------------------------------------------------------*/
static void
response_cb (GtkDialog     *dialog,
	     gint           response,
	     glViewImage   *view_image)
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
		object = gl_view_object_get_object (GL_VIEW_OBJECT(view_image));

		g_signal_handlers_disconnect_by_func (object, update_dialog_cb,
						      view_image);
		g_signal_handlers_disconnect_by_func (object, update_dialog_from_move_cb,
						      view_image);
		break;
	default:
		g_print ("response = %d", response);
		g_assert_not_reached();
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  file "changed" callback.                                        */
/*---------------------------------------------------------------------------*/
static void
file_changed_cb (GtkEntry          *pixmap_entry,
		 glViewImage       *view_image)
{
	glLabelObject    *object;
	glTextNode       *filename;

	gl_debug (DEBUG_VIEW, "START");

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_image));

	filename = gl_wdgt_image_select_get_data (GL_WDGT_IMAGE_SELECT(view_image->private->pixmap_entry));
	gl_debug (DEBUG_VIEW, "filename = %s", filename->data);

	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_dialog_cb, view_image);
	gl_label_image_set_filename(GL_LABEL_IMAGE(object), filename);
	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_dialog_cb, view_image);

	gl_text_node_free (&filename);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  position "changed" callback.                                    */
/*---------------------------------------------------------------------------*/
static void
position_changed_cb (glWdgtPosition   *position,
		     glViewImage      *view_image)
{
	glLabelObject      *object;
	gdouble            x, y;

	gl_debug (DEBUG_VIEW, "START");

	gl_wdgt_position_get_position (GL_WDGT_POSITION (position), &x, &y);

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_image));

	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_dialog_cb, view_image);
	gl_label_object_set_position (GL_LABEL_OBJECT(object), x, y);
	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_dialog_cb, view_image);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  size "changed" callback.                                        */
/*---------------------------------------------------------------------------*/
static void
size_changed_cb (glWdgtSize   *size,
		 glViewImage  *view_image)
{
	glLabelObject *object;
	gdouble       w, h;
	gboolean      keep_aspect_ratio_flag;

	gl_debug (DEBUG_VIEW, "START");

	gl_wdgt_size_get_size (GL_WDGT_SIZE (size),
			       &w, &h, &keep_aspect_ratio_flag);

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_image));

	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_dialog_cb, view_image);
	gl_label_object_set_size (GL_LABEL_OBJECT(object), w, h);
	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_dialog_cb, view_image);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  size "changed" callback.                                        */
/*---------------------------------------------------------------------------*/
static void
size_reset_cb (GtkButton    *button,
	       glViewImage  *view_image)
{
	glLabelObject   *object;
	gdouble         image_w, image_h;
	const GdkPixbuf *pixbuf;

	gl_debug (DEBUG_VIEW, "START");

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_image));
	pixbuf = gl_label_image_get_pixbuf (GL_LABEL_IMAGE(object), NULL);

	image_w = gdk_pixbuf_get_width (pixbuf);
	image_h = gdk_pixbuf_get_height (pixbuf);

	gl_label_object_set_size (GL_LABEL_OBJECT(object), image_w, image_h);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "changed" callback.                                 */
/*---------------------------------------------------------------------------*/
static void
update_dialog_cb (glLabelObject   *object,
		  glViewImage     *view_image)
{
	glTextNode        *filename;
	gdouble            x, y, w, h;
	glMerge           *merge;

	gl_debug (DEBUG_VIEW, "START");

	/* Query properties of object. */
	gl_label_object_get_position (GL_LABEL_OBJECT(object), &x, &y);
	gl_label_object_get_size (GL_LABEL_OBJECT(object), &w, &h);
	filename = gl_label_image_get_filename (GL_LABEL_IMAGE(object));
	merge = gl_label_get_merge (GL_LABEL(object->parent));

	/* Block widget handlers to prevent recursion */
	g_signal_handlers_block_by_func (G_OBJECT(view_image->private->pixmap_entry),
					 file_changed_cb, view_image);
	g_signal_handlers_block_by_func (G_OBJECT(view_image->private->position),
					 position_changed_cb, view_image);
	g_signal_handlers_block_by_func (G_OBJECT(view_image->private->size),
					 size_changed_cb, view_image);

	/* Update widgets in property dialog */

	if ( filename != NULL ) {
		gl_wdgt_image_select_set_data (GL_WDGT_IMAGE_SELECT(view_image->private->pixmap_entry),
					       (merge != NULL),
					       filename);
	}
	gl_wdgt_image_select_set_field_defs (GL_WDGT_IMAGE_SELECT(view_image->private->pixmap_entry),
					     merge);
	gl_wdgt_position_set_position (GL_WDGT_POSITION(view_image->private->position),
				       x, y);
	gl_wdgt_size_set_size (GL_WDGT_SIZE(view_image->private->size), w, h);

	/* Unblock widget handlers */
	g_signal_handlers_unblock_by_func (G_OBJECT(view_image->private->pixmap_entry),
					   file_changed_cb, view_image);
	g_signal_handlers_unblock_by_func (G_OBJECT(view_image->private->position),
					   position_changed_cb, view_image);
	g_signal_handlers_unblock_by_func (G_OBJECT(view_image->private->size),
					   size_changed_cb, view_image);

	g_free (filename);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "moved" callback.                                   */
/*---------------------------------------------------------------------------*/
static void
update_dialog_from_move_cb (glLabelObject *object,
			    gdouble        dx,
			    gdouble        dy,
			    glViewImage   *view_image)
{
	gdouble            x, y;

	gl_debug (DEBUG_VIEW, "START");

	/* Query properties of object. */
	gl_label_object_get_position (GL_LABEL_OBJECT(object), &x, &y);

	/* Block widget handlers to prevent recursion */
	g_signal_handlers_block_by_func (G_OBJECT(view_image->private->position),
					 position_changed_cb, view_image);

	/* Update widgets in property dialog */
	gl_wdgt_position_set_position (GL_WDGT_POSITION(view_image->private->position),
				       x, y);

	/* Unblock widget handlers */
	g_signal_handlers_unblock_by_func (G_OBJECT(view_image->private->position),
					   position_changed_cb, view_image);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Return apropos cursor for create object mode.                             */
/*****************************************************************************/
GdkCursor *
gl_view_image_get_create_cursor (void)
{
	static GdkCursor *cursor = NULL;
	GdkPixmap        *pixmap_data, *pixmap_mask;
	GdkColor         fg = { 0, 0, 0, 0 };
	GdkColor         bg = { 0, 65535, 65535, 65535 };

	gl_debug (DEBUG_VIEW, "START");

	if (!cursor) {
		pixmap_data = gdk_bitmap_create_from_data (NULL,
							   cursor_image_bits,
							   cursor_image_width,
							   cursor_image_height);
		pixmap_mask = gdk_bitmap_create_from_data (NULL,
							   cursor_image_mask_bits,
							   cursor_image_mask_width,
							   cursor_image_mask_height);
		cursor =
		    gdk_cursor_new_from_pixmap (pixmap_data, pixmap_mask, &fg,
						&bg, cursor_image_x_hot,
						cursor_image_y_hot);
	}

	gl_debug (DEBUG_VIEW, "END");

	return cursor;
}

/*****************************************************************************/
/* Canvas event handler for creating image objects.                          */
/*****************************************************************************/
int
gl_view_image_create_event_handler (GnomeCanvas *canvas,
				      GdkEvent    *event,
				      glView      *view)
{
	static gdouble      x0, y0;
	static gboolean     dragging = FALSE;
	static glViewObject *view_image;
	static GObject      *object;
	gdouble             line_width;
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
			object = gl_label_image_new (view->label);
			gl_label_object_set_position (GL_LABEL_OBJECT(object),
						     x, y);
			gl_label_object_set_size (GL_LABEL_OBJECT(object),
						  0.0, 0.0);
			view_image = gl_view_image_new (GL_LABEL_IMAGE(object),
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
			gl_view_unselect_all (view);
			gl_view_object_select (GL_VIEW_OBJECT(view_image));
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
