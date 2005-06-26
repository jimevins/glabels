/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_image.c:  GLabels label image object widget
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

#include "view-image.h"

#include "view-highlight.h"

#include "color.h"
#include "object-editor.h"
#include "stock.h"

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
	GnomeCanvasItem       *item;
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

static void       gl_view_image_class_init          (glViewImageClass *klass);
static void       gl_view_image_instance_init       (glViewImage      *view_image);
static void       gl_view_image_finalize            (GObject          *object);

static GtkWidget *construct_properties_editor       (glViewObject     *view_object);

static void       update_canvas_item_from_object_cb (glLabelObject    *object,
						     glViewImage      *view_image);

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
gl_view_image_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (glViewImageClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_view_image_class_init,
			NULL,
			NULL,
			sizeof (glViewImage),
			0,
			(GInstanceInitFunc) gl_view_image_instance_init,
			NULL
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

	view_object_class->construct_editor = construct_properties_editor;

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
/* NEW image object view.                                                    */
/*****************************************************************************/
glViewObject *
gl_view_image_new (glLabelImage *object,
		   glView       *view)
{
	glViewImage        *view_image;
	const GdkPixbuf    *pixbuf;
	gdouble            w, h;

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
			  G_CALLBACK (update_canvas_item_from_object_cb), view_image);

	gl_debug (DEBUG_VIEW, "END");

	return GL_VIEW_OBJECT (view_image);
}

/*****************************************************************************/
/* Create a properties editor for an image object.                           */
/*****************************************************************************/
static GtkWidget *
construct_properties_editor (glViewObject *view_object)
{
	GtkWidget          *editor;
	glViewImage        *view_image = (glViewImage *)view_object;
	glLabelObject      *object;

	gl_debug (DEBUG_VIEW, "START");

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_image));

	/* Build editor. */
	editor = gl_object_editor_new (GL_STOCK_IMAGE, _("Image object properties"),
				       GL_OBJECT_EDITOR_POSITION_PAGE,
				       GL_OBJECT_EDITOR_SIZE_IMAGE_PAGE,
				       GL_OBJECT_EDITOR_IMAGE_PAGE,
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
				   glViewImage     *view_image)
{
	const GdkPixbuf    *pixbuf;
	gdouble             w, h;

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

/*---------------------------------------------------------------------------*/
/* PRIVATE.  editor "changed" callback.                                      */
/*---------------------------------------------------------------------------*/
static void
update_object_from_editor_cb (glObjectEditor *editor,
			      glLabelObject  *object)
{
	gdouble            x, y, w, h;
	glTextNode        *filename;
	const GdkPixbuf   *pixbuf;
	gdouble            image_w, image_h;

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

	filename = gl_object_editor_get_image (editor);
	gl_label_image_set_filename (GL_LABEL_IMAGE(object), filename);

	/* Setting filename may have modified the size. */
	gl_label_object_get_size (object, &w, &h);
	gl_object_editor_set_size (editor, w, h);

	/* It may also have a new base size. */
        pixbuf = gl_label_image_get_pixbuf (GL_LABEL_IMAGE(object), NULL);
        image_w = gdk_pixbuf_get_width (pixbuf);
        image_h = gdk_pixbuf_get_height (pixbuf);
	gl_object_editor_set_base_size (editor, image_w, image_h);

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
	const GdkPixbuf   *pixbuf;
	gdouble            image_w, image_h;
	glTextNode        *filename;
	glMerge           *merge;

	gl_debug (DEBUG_VIEW, "START");

	gl_label_object_get_size (object, &w, &h);
	gl_object_editor_set_size (editor, w, h);

        pixbuf = gl_label_image_get_pixbuf (GL_LABEL_IMAGE(object), NULL);
        image_w = gdk_pixbuf_get_width (pixbuf);
        image_h = gdk_pixbuf_get_height (pixbuf);
	gl_object_editor_set_base_size (editor, image_w, image_h);

	filename = gl_label_image_get_filename (GL_LABEL_IMAGE(object));
	merge = gl_label_get_merge (GL_LABEL(object->parent));
	if ( filename != NULL ) {
		gl_object_editor_set_image (editor, (merge != NULL), filename);
	}

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
	glMerge           *merge;

	gl_debug (DEBUG_VIEW, "START");

	gl_label_get_size (label, &label_width, &label_height);
	gl_object_editor_set_max_position (GL_OBJECT_EDITOR (editor),
					   label_width, label_height);
	gl_object_editor_set_max_size (GL_OBJECT_EDITOR (editor),
				       label_width, label_height);

	merge = gl_label_get_merge (label);
	gl_object_editor_set_key_names (editor, merge);

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
						  1.0, 1.0);
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
