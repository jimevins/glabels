/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  item_image.c:  GLabels Image Object (canvas item) module
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

#include <string.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gnome-canvas-pixbuf.h>

#include "display.h"
#include "item_image.h"

#include "highlight.h"

#include "mdi.h"
#include "propertybox.h"
#include "prop_position.h"
#include "prop_size.h"

#include "pixmaps/cursor_image.xbm"
#include "pixmaps/cursor_image_mask.xbm"

#include "pixmaps/checkerboard.xpm"

#include "debug.h"

#define RED(x)   ( ((x)>>24) & 0xff )
#define GREEN(x) ( ((x)>>16) & 0xff )
#define BLUE(x)  ( ((x)>>8)  & 0xff )
#define ALPHA(x) (  (x)      & 0xff )

/*===========================================*/
/* Private data types                        */
/*===========================================*/

typedef struct {
	GnomeCanvasItem *item;
	gboolean keep_aspect_ratio_flag;

	/* Page 0 widgets */
	GtkWidget *pixmap_entry;

	/* Page 1 widgets */
	GtkWidget *position;
	GtkWidget *size;
} PropertyDialogPassback;

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/* Save state of image file entry */
static gchar *image_path = NULL;

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void item_image_update (GnomeCanvasItem * item);

static void changed_cb (glPropertyBox * propertybox);
static void size_reset_cb (glPropertyBox * propertybox);
static void apply_cb (glPropertyBox * propertybox,
		      gint page, PropertyDialogPassback * data);

/****************************************************************************/
/* Create a image object                                                    */
/****************************************************************************/
GnomeCanvasItem *
gl_item_image_new (glLabelObject * object,
		   glDisplay * display)
{
	GnomeCanvasGroup *group;
	GnomeCanvasItem *item;

	if (object->arg.image.image == NULL) {
		object->arg.image.image =
		    gdk_pixbuf_new_from_xpm_data ((const char **)
						  checkerboard_xpm);
	}

	group = gnome_canvas_root (GNOME_CANVAS (display->canvas));
	item = gnome_canvas_item_new (group, gnome_canvas_pixbuf_get_type (),
				      "x", object->x,
				      "y", object->y,
				      "width_set", TRUE,
				      "height_set", TRUE,
				      "width", object->arg.image.w,
				      "height", object->arg.image.h,
				      "pixbuf", object->arg.image.image, NULL);

	/* Squirrel away pointers to object and display in the canvas item. */
	gtk_object_set_data (GTK_OBJECT (item), "label_object", object);
	gtk_object_set_data (GTK_OBJECT (item), "display", display);
	gtk_object_set_data (GTK_OBJECT (item), "highlight", NULL);

	gtk_signal_connect (GTK_OBJECT (item), "event",
			    GTK_SIGNAL_FUNC (gl_display_item_event_handler),
			    display);

	gl_display_new_item_menu (item);

	gl_display_set_modified (display);

	return item;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE. Update an image object                                          */
/*--------------------------------------------------------------------------*/
static void
item_image_update (GnomeCanvasItem * item)
{
	glLabelObject *object;
	glDisplay *display;
	gdouble affine[6];

	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");

	if (object->arg.image.image == NULL) {
		object->arg.image.image =
		    gdk_pixbuf_new_from_xpm_data ((const char **)
						  checkerboard_xpm);
	}

	art_affine_identity (affine);
	gnome_canvas_item_affine_absolute (item, affine);
	gnome_canvas_item_set (item,
			       "x", object->x,
			       "y", object->y,
			       "width_set", TRUE,
			       "height_set", TRUE,
			       "width", object->arg.image.w,
			       "height", object->arg.image.h,
			       "pixbuf", object->arg.image.image, NULL);

	display = gtk_object_get_data (GTK_OBJECT (item), "display");
	gl_display_set_modified (display);
}

/****************************************************************************/
/* Return a selection canvas item/group for given item                      */
/****************************************************************************/
void
gl_item_image_highlight (GnomeCanvasItem * item)
{
	gl_highlight (item, GL_HIGHLIGHT_BOX_RESIZABLE);
}

/****************************************************************************/
/* Get position and size of object.                                         */
/****************************************************************************/
void
gl_item_image_get_position_size (GnomeCanvasItem * item,
				 gdouble * x,
				 gdouble * y,
				 gdouble * w,
				 gdouble * h)
{
	glLabelObject *object;

	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");

	*x = object->x;
	*y = object->y;

	*w = object->arg.image.w;
	*h = object->arg.image.h;
}

/****************************************************************************/
/* Get position and size of object.                                         */
/****************************************************************************/
void
gl_item_image_set_position_size (GnomeCanvasItem * item,
				 gdouble x,
				 gdouble y,
				 gdouble w,
				 gdouble h)
{
	glLabelObject *object;

	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");

	object->x = x;
	object->y = y;

	object->arg.image.w = w;
	object->arg.image.h = h;

	item_image_update (item);
}

/****************************************************************************/
/* Get image item bounds.                                                   */
/****************************************************************************/
void
gl_item_image_get_bounds (GnomeCanvasItem * item,
			  gdouble * x1,
			  gdouble * y1,
			  gdouble * x2,
			  gdouble * y2)
{
	gnome_canvas_item_get_bounds (item, x1, y1, x2, y2);
}

/****************************************************************************/
/* Create and run an edit dialog on a image object.                         */
/****************************************************************************/
void
gl_item_image_edit_dialog (GnomeCanvasItem * item)
{
	GtkWidget *dialog;
	static PropertyDialogPassback *data = NULL;
	GtkWidget *wbutton;
	glLabelObject *object;
	glDisplay *display;
	GtkWidget *wvbox;
	gdouble image_w, image_h;

	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");
	display = gtk_object_get_data (GTK_OBJECT (item), "display");

	/*-----------------------------------------------------------------*/
	/* Build dialog with notebook.                                     */
	/*-----------------------------------------------------------------*/
	if (data == NULL) {
		data = g_new0 (PropertyDialogPassback, 1);
	}
	data->item = item;

	dialog = gl_property_box_new ();
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (dialog),
				      GTK_WINDOW (gl_mdi_get_active_window ()));
	gtk_window_set_title (GTK_WINDOW (dialog),
			      _("Edit image object properties"));
	gtk_signal_connect (GTK_OBJECT (dialog), "apply",
			    GTK_SIGNAL_FUNC (apply_cb), data);
	gtk_object_set_data (GTK_OBJECT (dialog), "data", data);

	/*---------------------------*/
	/* Image Notebook Tab        */
	/*---------------------------*/
	wvbox = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox), 10);
	gl_property_box_append_page (GL_PROPERTY_BOX (dialog), wvbox,
				     gtk_label_new (_("Image")));

	/* image entry */
	data->pixmap_entry =
	    gnome_pixmap_entry_new ("image", "Load image", TRUE);
	gnome_pixmap_entry_set_preview_size (GNOME_PIXMAP_ENTRY
					     (data->pixmap_entry), 128, 128);

	/* Set default path for image entry */
	if (object->arg.image.filename != NULL) {
		/* Set default path to the directory containing image. */
		if (image_path != NULL)
			g_free (image_path);
		image_path = g_dirname (object->arg.image.filename);
		if (image_path != NULL) {
			image_path = g_strconcat (image_path, "/", NULL);
		}
	} else if (image_path == NULL) {
		/* First time, set it to our CWD. */
		image_path = g_get_current_dir ();
	}
	/* Otherwise, leave it in the last directory that we got an image. */
	gnome_file_entry_set_default_path (GNOME_FILE_ENTRY
					   (GNOME_PIXMAP_ENTRY
					    (data->pixmap_entry)->fentry),
					   image_path);

	if (object->arg.image.filename != NULL) {
		gtk_entry_set_text (GTK_ENTRY
				    (gnome_pixmap_entry_gtk_entry
				     (GNOME_PIXMAP_ENTRY (data->pixmap_entry))),
				    object->arg.image.filename);
	}
	gtk_box_pack_start (GTK_BOX (wvbox), data->pixmap_entry, FALSE, FALSE,
			    0);
	gtk_signal_connect_object (GTK_OBJECT
				   (gnome_pixmap_entry_gtk_entry
				    (GNOME_PIXMAP_ENTRY (data->pixmap_entry))),
				   "changed", GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (dialog));

	/*----------------------------*/
	/* Position/Size Notebook Tab */
	/*----------------------------*/
	wvbox = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox), 10);
	gl_property_box_append_page (GL_PROPERTY_BOX (dialog), wvbox,
				     gtk_label_new (_("Position/Size")));

	/* ------ Position Frame ------ */
	data->position = gl_prop_position_new (_("Position"));
	gl_prop_position_set_params (GL_PROP_POSITION (data->position),
				     object->x, object->y,
				     display->label->width,
				     display->label->height);
	gtk_box_pack_start (GTK_BOX (wvbox), data->position, FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (data->position), "changed",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (dialog));

	/* ------ Size Frame ------ */
	data->size = gl_prop_size_new (_("Size"));
	image_w = gdk_pixbuf_get_width (object->arg.image.image);
	image_h = gdk_pixbuf_get_height (object->arg.image.image);
	gl_prop_size_set_params (GL_PROP_SIZE (data->size),
				 object->arg.image.w, object->arg.image.h,
				 data->keep_aspect_ratio_flag,
				 display->label->width, display->label->height);
	gtk_box_pack_start (GTK_BOX (wvbox), data->size, FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (data->size), "changed",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (dialog));

	/* ------ Size Reset Button ------ */
	wbutton = gtk_button_new_with_label (_("Reset image size"));
	gtk_box_pack_start (GTK_BOX (wvbox), wbutton, FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (wbutton), "clicked",
				   GTK_SIGNAL_FUNC (size_reset_cb),
				   GTK_OBJECT (dialog));

	/*-----------------------------------------------------------------*/
	/* Run dialog, and update state of object if "Applied."            */
	/*-----------------------------------------------------------------*/
	gtk_widget_show_all (GTK_WIDGET (dialog));

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the dialog has changed.        */
/*---------------------------------------------------------------------------*/
static void
changed_cb (glPropertyBox * propertybox)
{
	gl_property_box_changed (propertybox);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Callback to reset size to original image size.                  */
/*---------------------------------------------------------------------------*/
static void
size_reset_cb (glPropertyBox * propertybox)
{
	PropertyDialogPassback *data = NULL;
	glDisplay *display;
	gchar *filename, *ext;
	GdkPixbuf *image;
	gdouble image_w, image_h;

	data = gtk_object_get_data (GTK_OBJECT (propertybox), "data");

	display = gtk_object_get_data (GTK_OBJECT (data->item), "display");

	filename =
	    gnome_pixmap_entry_get_filename (GNOME_PIXMAP_ENTRY
					     (data->pixmap_entry));
	if (filename != NULL) {
		ext = strrchr (filename, '.');
		if (g_strcasecmp (ext, ".tga") != 0) {
			image = gdk_pixbuf_new_from_file (filename);
		} else {
			gnome_error_dialog (_
					    ("Image format not currently supported"));
			return;
		}
	} else {
		return;
	}
	image_w = gdk_pixbuf_get_width (image);
	image_h = gdk_pixbuf_get_height (image);
	gdk_pixbuf_unref (image);

	data->keep_aspect_ratio_flag = TRUE;

	gl_prop_size_set_params (GL_PROP_SIZE (data->size),
				 image_w, image_h,
				 data->keep_aspect_ratio_flag,
				 display->label->width, display->label->height);

	gl_property_box_changed (propertybox);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Apply" callback.                                               */
/*---------------------------------------------------------------------------*/
static void
apply_cb (glPropertyBox * propertybox,
	  gint page,
	  PropertyDialogPassback * data)
{
	glLabelObject *object;
	glDisplay *display;
	gchar *filename, *ext;
	gdouble image_w, image_h;

	object = gtk_object_get_data (GTK_OBJECT (data->item), "label_object");
	display = gtk_object_get_data (GTK_OBJECT (data->item), "display");

	switch (page) {

	case 0:
		/* ------ Update image ------ */
		filename =
		    gnome_pixmap_entry_get_filename (GNOME_PIXMAP_ENTRY
						     (data->pixmap_entry));
		if (filename != NULL) {
			ext = strrchr (filename, '.');
			if (g_strcasecmp (ext, ".tga") != 0) {

				g_free (object->arg.image.filename);
				object->arg.image.filename = filename;
				gdk_pixbuf_unref (object->arg.image.image);
				object->arg.image.image =
				    gdk_pixbuf_new_from_file (object->arg.image.
							      filename);

				/* save state of image entry. */
				if (image_path != NULL)
					g_free (image_path);
				image_path = g_dirname (filename);
				if (image_path != NULL) {
					image_path =
					    g_strconcat (image_path, "/", NULL);
				}

			} else {
				gnome_error_dialog (_
						    ("Image format not currently supported"));
			}
		}
		break;

	case 1:
		/* ------ get updated position ------ */
		gl_prop_position_get_position (GL_PROP_POSITION
					       (data->position), &object->x,
					       &object->y);

		/* ------ get updated size ------ */
		gl_prop_size_get_size (GL_PROP_SIZE (data->size),
				       &object->arg.image.w,
				       &object->arg.image.h,
				       &data->keep_aspect_ratio_flag);
		break;

	default:
		return;
	}

	/* ------ Udate state of object ------ */
	item_image_update (data->item);
	gl_display_select_item (display, data->item);

	/* update size controls */
	image_w = gdk_pixbuf_get_width (object->arg.image.image);
	image_h = gdk_pixbuf_get_height (object->arg.image.image);
	gl_prop_size_set_params (GL_PROP_SIZE (data->size),
				 object->arg.image.w, object->arg.image.h,
				 data->keep_aspect_ratio_flag,
				 display->label->width, display->label->height);
}

/****************************************************************************/
/* Return apropos cursor for create object mode.                            */
/****************************************************************************/
GdkCursor *
gl_item_image_get_create_cursor (void)
{
	static GdkCursor *cursor = NULL;
	GdkPixmap *pixmap_data, *pixmap_mask;
	GdkColor fg = { 0, 0, 0, 0 };
	GdkColor bg = { 0, 65535, 65535, 65535 };

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

	return cursor;
}

/****************************************************************************/
/* Canvas event handler (image mode)                                        */
/****************************************************************************/
gint
gl_item_image_create_event_handler (GnomeCanvas * canvas,
				    GdkEvent * event,
				    gpointer data)
{
	static gdouble x0, y0;
	static gboolean dragging = FALSE;
	static GnomeCanvasItem *item;
	static glLabelObject *object;
	glDisplay *display = GL_DISPLAY (data);
	gdouble x, y;

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			dragging = TRUE;
			gdk_pointer_grab (GTK_WIDGET (display->canvas)->window,
					  FALSE,
					  GDK_POINTER_MOTION_MASK |
					  GDK_BUTTON_RELEASE_MASK |
					  GDK_BUTTON_PRESS_MASK,
					  NULL, NULL, event->button.time);
			gnome_canvas_window_to_world (canvas,
						      event->button.x,
						      event->button.y, &x, &y);
			object =
			    gl_label_object_new (display->label,
						 GL_LABEL_OBJECT_IMAGE);
			object->x = x;
			object->y = y;
			object->arg.image.w = 1.0;
			object->arg.image.h = 1.0;
			object->arg.image.image = NULL;
			object->arg.image.filename = NULL;
			item = gl_item_image_new (object, display);
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
			if ((x0 == x) || (y0 == y)) {
				x = x0 + 24.0;
				y = y0 + 24.0;
			}
			object->x = MIN (x, x0);
			object->y = MIN (y, y0);
			object->arg.image.w = MAX (x, x0) - MIN (x, x0);
			object->arg.image.h = MAX (y, y0) - MIN (y, y0);
			item_image_update (item);
			gl_display_add_item (display, item);
			gl_display_unselect_all (display);
			gl_display_select_item (display, item);
			gl_display_arrow_mode (display);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			gnome_canvas_window_to_world (canvas,
						      event->motion.x,
						      event->motion.y, &x, &y);
			object->x = MIN (x, x0);
			object->y = MIN (y, y0);
			object->arg.image.w = MAX (x, x0) - MIN (x, x0);
			object->arg.image.h = MAX (y, y0) - MIN (y, y0);
			item_image_update (item);
			return TRUE;
		} else {
			return FALSE;
		}

	default:
		return FALSE;
	}

}
