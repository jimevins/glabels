/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  item.c:  GLabels canvas item wrapper module module
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

#include "item.h"

#include "display.h"
#include "item_text.h"
#include "item_box.h"
#include "item_line.h"
#include "item_ellipse.h"
#include "item_image.h"
#include "item_barcode.h"

#include "debug.h"

/*===========================================*/
/* Private types                            */
/*===========================================*/

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

/*****************************************************************************/
/* Create new canvas item from object.                                       */
/*****************************************************************************/
GnomeCanvasItem *
gl_item_new (glLabelObject * object,
	     glDisplay * display)
{
	GnomeCanvasItem *item;

	g_return_val_if_fail (object != NULL, NULL);
	g_return_val_if_fail (GL_IS_DISPLAY (display), NULL);

	switch (object->type) {
	case GL_LABEL_OBJECT_TEXT:
		item = gl_item_text_new (object, display);
		break;
	case GL_LABEL_OBJECT_BOX:
		item = gl_item_box_new (object, display);
		break;
	case GL_LABEL_OBJECT_LINE:
		item = gl_item_line_new (object, display);
		break;
	case GL_LABEL_OBJECT_ELLIPSE:
		item = gl_item_ellipse_new (object, display);
		break;
	case GL_LABEL_OBJECT_IMAGE:
		item = gl_item_image_new (object, display);
		break;
	case GL_LABEL_OBJECT_BARCODE:
		item = gl_item_barcode_new (object, display);
		break;
	default:
		item = NULL;
		WARN ("Invalid label object type.");	/* Should not happen! */
		break;
	}

	return item;
}

/*****************************************************************************/
/* Free a canvas item and all our associated data.                           */
/*****************************************************************************/
void
gl_item_free (GnomeCanvasItem ** item)
{
	glLabelObject *object;
	GtkWidget *menu;
	GnomeCanvasItem *highlight;

	g_return_if_fail (GNOME_IS_CANVAS_ITEM (*item));

	menu = gtk_object_get_data (GTK_OBJECT (*item), "object_menu");
	object = gtk_object_get_data (GTK_OBJECT (*item), "label_object");
	highlight = gtk_object_get_data (GTK_OBJECT (*item), "highlight");

	gtk_menu_popdown (GTK_MENU (menu));
	gtk_widget_unref (GTK_WIDGET (menu));

	gl_label_object_free (&object);

	if (highlight != NULL) {
		gtk_object_destroy (GTK_OBJECT (highlight));
	}

	gtk_object_destroy (GTK_OBJECT (*item));
	*item = NULL;
}

/*****************************************************************************/
/* Get position and size of object.                                          */
/*****************************************************************************/
void
gl_item_get_position_size (GnomeCanvasItem * item,
			   gdouble * x,
			   gdouble * y,
			   gdouble * w,
			   gdouble * h)
{
	glLabelObject *object;

	g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));

	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");

	switch (object->type) {
	case GL_LABEL_OBJECT_TEXT:
		gl_item_text_get_position_size (item, x, y, w, h);
		break;
	case GL_LABEL_OBJECT_BOX:
		gl_item_box_get_position_size (item, x, y, w, h);
		break;
	case GL_LABEL_OBJECT_LINE:
		gl_item_line_get_position_size (item, x, y, w, h);
		break;
	case GL_LABEL_OBJECT_ELLIPSE:
		gl_item_ellipse_get_position_size (item, x, y, w, h);
		break;
	case GL_LABEL_OBJECT_IMAGE:
		gl_item_image_get_position_size (item, x, y, w, h);
		break;
	case GL_LABEL_OBJECT_BARCODE:
		gl_item_barcode_get_position_size (item, x, y, w, h);
		break;
	default:
		WARN ("Invalid label object type.");	/* Should not happen! */
		break;
	}

}

/*****************************************************************************/
/* Get position and size of object.                                          */
/*****************************************************************************/
void
gl_item_set_position_size (GnomeCanvasItem * item,
			   gdouble x,
			   gdouble y,
			   gdouble w,
			   gdouble h)
{
	glLabelObject *object;

	g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));

	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");

	switch (object->type) {
	case GL_LABEL_OBJECT_TEXT:
		gl_item_text_set_position_size (item, x, y, w, h);
		break;
	case GL_LABEL_OBJECT_BOX:
		gl_item_box_set_position_size (item, x, y, w, h);
		break;
	case GL_LABEL_OBJECT_LINE:
		gl_item_line_set_position_size (item, x, y, w, h);
		break;
	case GL_LABEL_OBJECT_ELLIPSE:
		gl_item_ellipse_set_position_size (item, x, y, w, h);
		break;
	case GL_LABEL_OBJECT_IMAGE:
		gl_item_image_set_position_size (item, x, y, w, h);
		break;
	case GL_LABEL_OBJECT_BARCODE:
		gl_item_barcode_set_position_size (item, x, y, w, h);
		break;
	default:
		WARN ("Invalid label object type.");	/* Should not happen! */
		break;
	}

}

/*****************************************************************************/
/* Get object associated with canvas item.                                   */
/*****************************************************************************/
glLabelObject *
gl_item_get_object (GnomeCanvasItem * item)
{
	g_return_val_if_fail (GNOME_IS_CANVAS_ITEM (item), NULL);

	return gtk_object_get_data (GTK_OBJECT (item), "label_object");
}

/*****************************************************************************/
/* Get display associated with canvas item.                                  */
/*****************************************************************************/
glDisplay *
gl_item_get_display (GnomeCanvasItem * item)
{
	g_return_val_if_fail (GNOME_IS_CANVAS_ITEM (item), NULL);

	return gtk_object_get_data (GTK_OBJECT (item), "display");
}

/*****************************************************************************/
/* Get menu associated with canvas item.                                     */
/*****************************************************************************/
GtkWidget *
gl_item_get_menu (GnomeCanvasItem * item)
{
	g_return_val_if_fail (GNOME_IS_CANVAS_ITEM (item), NULL);

	return gtk_object_get_data (GTK_OBJECT (item), "object_menu");
}

/*****************************************************************************/
/* Turn on item highlight.                                                   */
/*****************************************************************************/
void
gl_item_highlight (GnomeCanvasItem * item)
{
	glLabelObject *object;

	g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));

	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");

	switch (object->type) {
	case GL_LABEL_OBJECT_TEXT:
		gl_item_text_highlight (item);
		break;
	case GL_LABEL_OBJECT_BOX:
		gl_item_box_highlight (item);
		break;
	case GL_LABEL_OBJECT_LINE:
		gl_item_line_highlight (item);
		break;
	case GL_LABEL_OBJECT_ELLIPSE:
		gl_item_ellipse_highlight (item);
		break;
	case GL_LABEL_OBJECT_IMAGE:
		gl_item_image_highlight (item);
		break;
	case GL_LABEL_OBJECT_BARCODE:
		gl_item_barcode_highlight (item);
		break;
	default:
		WARN ("Invalid label object type.");	/* Should not happen! */
		break;
	}

}

/*****************************************************************************/
/* Turn off item highlight.                                                  */
/*****************************************************************************/
void
gl_item_unhighlight (GnomeCanvasItem * item)
{
	GnomeCanvasItem *sel_box;

	g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));

	if (item != NULL) {

		sel_box = gtk_object_get_data (GTK_OBJECT (item), "highlight");
		gnome_canvas_item_hide (sel_box);

	}
}

/*****************************************************************************/
/* Get item bounds using appropriate custom function.                        */
/*****************************************************************************/
void
gl_item_get_bounds (GnomeCanvasItem * item,
		    gdouble * x1,
		    gdouble * y1,
		    gdouble * x2,
		    gdouble * y2)
{
	glLabelObject *object;

	g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));

	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");

	switch (object->type) {
	case GL_LABEL_OBJECT_TEXT:
		gl_item_text_get_bounds (item, x1, y1, x2, y2);
		break;
	case GL_LABEL_OBJECT_BOX:
		gl_item_box_get_bounds (item, x1, y1, x2, y2);
		break;
	case GL_LABEL_OBJECT_LINE:
		gl_item_line_get_bounds (item, x1, y1, x2, y2);
		break;
	case GL_LABEL_OBJECT_ELLIPSE:
		gl_item_ellipse_get_bounds (item, x1, y1, x2, y2);
		break;
	case GL_LABEL_OBJECT_IMAGE:
		gl_item_image_get_bounds (item, x1, y1, x2, y2);
		break;
	case GL_LABEL_OBJECT_BARCODE:
		gl_item_barcode_get_bounds (item, x1, y1, x2, y2);
		break;
	default:
		WARN ("Invalid label object type.");	/* Should not happen! */
		break;
	}

}

/*****************************************************************************/
/* launch apropriate edit dialog for item/object                             */
/*****************************************************************************/
void
gl_item_edit_dialog (GnomeCanvasItem * item)
{
	glLabelObject *object;
	glDisplay *display;

	g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));

	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");
	display = gtk_object_get_data (GTK_OBJECT (item), "display");

	switch (object->type) {
	case GL_LABEL_OBJECT_TEXT:
		gl_item_text_edit_dialog (item);
		break;
	case GL_LABEL_OBJECT_BOX:
		gl_item_box_edit_dialog (item);
		break;
	case GL_LABEL_OBJECT_LINE:
		gl_item_line_edit_dialog (item);
		break;
	case GL_LABEL_OBJECT_ELLIPSE:
		gl_item_ellipse_edit_dialog (item);
		break;
	case GL_LABEL_OBJECT_IMAGE:
		gl_item_image_edit_dialog (item);
		break;
	case GL_LABEL_OBJECT_BARCODE:
		gl_item_barcode_edit_dialog (item);
		break;
	default:
		WARN ("Invalid label object type.");	/* Should not happen! */
		break;
	}

	/* Update highlight of item */
	gl_item_highlight (item);
}

/*****************************************************************************/
/* Return apropos item create mode cursor for given object type.             */
/*****************************************************************************/
GdkCursor *
gl_item_get_create_cursor (glLabelObjectType type)
{
	switch (type) {

	case GL_LABEL_OBJECT_TEXT:
		return gl_item_text_get_create_cursor ();

	case GL_LABEL_OBJECT_BOX:
		return gl_item_box_get_create_cursor ();

	case GL_LABEL_OBJECT_LINE:
		return gl_item_line_get_create_cursor ();

	case GL_LABEL_OBJECT_ELLIPSE:
		return gl_item_ellipse_get_create_cursor ();

	case GL_LABEL_OBJECT_IMAGE:
		return gl_item_image_get_create_cursor ();

	case GL_LABEL_OBJECT_BARCODE:
		return gl_item_barcode_get_create_cursor ();

	default:
		WARN ("Invalid label object type.");	/* Should not happen! */
		return NULL;

	}

}

/*****************************************************************************/
/* Item/object creation canvas event handler.                                */
/*****************************************************************************/
gint
gl_item_create_event_handler (GnomeCanvas * canvas,
			      GdkEvent * event,
			      glDisplay * display)
{
	switch (display->create_type) {

	case GL_LABEL_OBJECT_TEXT:
		return gl_item_text_create_event_handler (canvas, event,
							  display);

	case GL_LABEL_OBJECT_BOX:
		return gl_item_box_create_event_handler (canvas, event,
							 display);

	case GL_LABEL_OBJECT_LINE:
		return gl_item_line_create_event_handler (canvas, event,
							  display);

	case GL_LABEL_OBJECT_ELLIPSE:
		return gl_item_ellipse_create_event_handler (canvas, event,
							     display);

	case GL_LABEL_OBJECT_IMAGE:
		return gl_item_image_create_event_handler (canvas, event,
							   display);

	case GL_LABEL_OBJECT_BARCODE:
		return gl_item_barcode_create_event_handler (canvas, event,
							     display);

	default:
		WARN ("Invalid label object type.");	/* Should not happen! */
		return FALSE;

	}

}
