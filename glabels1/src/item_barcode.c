/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  item_barcode.c:  GLabels Barcode Object (canvas item) module
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

#include "display.h"
#include "item_barcode.h"
#include "bc.h"

#include "highlight.h"

#include "mdi.h"
#include "propertybox.h"
#include "prop_bc_data.h"
#include "prop_bc.h"
#include "prop_bc_style.h"
#include "prop_position.h"

#include "pixmaps/cursor_barcode.xbm"
#include "pixmaps/cursor_barcode_mask.xbm"

#ifdef HACKTEXT
#include "gnome-canvas-hacktext.h"
#include <libgnomeprint/gnome-glyphlist.h>
#endif

#include "debug.h"

#define Y_FONT_ADJUST (-1.0)	/* Adjust font vertical position */

/*===========================================*/
/* Private data types                        */
/*===========================================*/

typedef struct {
	GnomeCanvasItem *item;

	/* Page 0 widgets */
	GtkWidget *data_entry;

	/* Page 1 widgets */
	GtkWidget *bcprop;
	GtkWidget *style;

	/* Page 2 widgets */
	GtkWidget *position;
} PropertyDialogPassback;

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void item_barcode_update (GnomeCanvasItem * item);

static void changed_cb (glPropertyBox * propertybox);
static void apply_cb (glPropertyBox * propertybox, gint page,
		      PropertyDialogPassback * data);

static void draw_barcode (GnomeCanvasItem * item,
			  glBarcodeStyle style,
			  guint color,
			  gboolean text_flag,
			  gdouble scale,
			  gchar * digits,
			  gdouble canvas_scale);

/*****************************************************************************/
/* Create a barcode object                                                   */
/*****************************************************************************/
GnomeCanvasItem *
gl_item_barcode_new (glLabelObject * object,
		     glDisplay * display)
{
	GnomeCanvasGroup *group;
	GnomeCanvasItem *item;
	gchar *text;

	group = gnome_canvas_root (GNOME_CANVAS (display->canvas));
	item = gnome_canvas_item_new (group, gnome_canvas_group_get_type (),
				      "x", object->x, "y", object->y, NULL);

	if (object->arg.barcode.text_node->field_flag) {
		text = gl_barcode_default_digits (object->arg.barcode.style);
	} else {
		text =
		    gl_text_node_expand (object->arg.barcode.text_node, NULL);
	}
	draw_barcode (item,
		      object->arg.barcode.style,
		      object->arg.barcode.color,
		      object->arg.barcode.text_flag,
		      object->arg.barcode.scale, text, display->scale);
	g_free (text);

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

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Update a barcode object                                         */
/*---------------------------------------------------------------------------*/
static void
item_barcode_update (GnomeCanvasItem * item)
{
	glLabelObject *object;
	glDisplay *display;
	GList *item_list, *li;
	gdouble affine[6];
	gchar *text;

	display = gtk_object_get_data (GTK_OBJECT (item), "display");
	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");

	item_list = gtk_object_get_data (GTK_OBJECT (item), "item_list");
	for (li = item_list; li != NULL; li = li->next) {
		gtk_object_destroy (GTK_OBJECT (li->data));
	}
	g_list_free (item_list);
	gtk_object_set_data (GTK_OBJECT (item), "item_list", NULL);

	if (object->arg.barcode.text_node->field_flag) {
		text = gl_barcode_default_digits (object->arg.barcode.style);
	} else {
		text =
		    gl_text_node_expand (object->arg.barcode.text_node, NULL);
	}
	draw_barcode (item,
		      object->arg.barcode.style,
		      object->arg.barcode.color,
		      object->arg.barcode.text_flag,
		      object->arg.barcode.scale, text, display->scale);
	g_free (text);

	art_affine_identity (affine);
	gnome_canvas_item_affine_absolute (item, affine);
	gnome_canvas_item_set (item, "x", object->x, "y", object->y, NULL);

	gl_display_set_modified (display);
}

/*****************************************************************************/
/* Highlight given canvas item.                                              */
/*****************************************************************************/
void
gl_item_barcode_highlight (GnomeCanvasItem * item)
{
	gl_highlight (item, GL_HIGHLIGHT_SIMPLE);
}

/*****************************************************************************/
/* Get position and size of object.                                          */
/*****************************************************************************/
void
gl_item_barcode_get_position_size (GnomeCanvasItem * item,
				   gdouble * x,
				   gdouble * y,
				   gdouble * w,
				   gdouble * h)
{
	glLabelObject *object;
	glBarcode *gbc;

	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");

	*x = object->x;
	*y = object->y;

	gbc = gtk_object_get_data (GTK_OBJECT (item), "bc");
	*w = gbc->width;
	*h = gbc->height;

}

/*****************************************************************************/
/* Get position and size of object.                                          */
/*****************************************************************************/
void
gl_item_barcode_set_position_size (GnomeCanvasItem * item,
				   gdouble x,
				   gdouble y,
				   gdouble w,
				   gdouble h)
{
	glLabelObject *object;

	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");

	object->x = x;
	object->y = y;

	/* Currently ignore w, h */

	item_barcode_update (item);
}

/*****************************************************************************/
/* Get barcode item bounds.                                                  */
/*****************************************************************************/
void
gl_item_barcode_get_bounds (GnomeCanvasItem * item,
			    gdouble * x1,
			    gdouble * y1,
			    gdouble * x2,
			    gdouble * y2)
{
	glBarcode *gbc;
	glLabelObject *object;

	gbc = gtk_object_get_data (GTK_OBJECT (item), "bc");
	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");

	if (gbc == NULL) {
		gnome_canvas_item_get_bounds (item, x1, y1, x2, y2);
	} else {
		*x1 = object->x;
		*y1 = object->y;
		*x2 = *x1 + gbc->width;
		*y2 = *y1 + gbc->height;
	}
}

/*****************************************************************************/
/* Create and run an edit dialog on a barcode object.                        */
/*****************************************************************************/
void
gl_item_barcode_edit_dialog (GnomeCanvasItem * item)
{
	GtkWidget *dialog;
	static PropertyDialogPassback *data = NULL;
	glLabelObject *object;
	glDisplay *display;
	GtkWidget *wvbox;

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
			      _("Edit barcode object properties"));
	gtk_signal_connect (GTK_OBJECT (dialog), "apply",
			    GTK_SIGNAL_FUNC (apply_cb), data);

	/*---------------------------*/
	/* Data Notebook Tab         */
	/*---------------------------*/
	wvbox = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox), 10);
	gl_property_box_append_page (GL_PROPERTY_BOX (dialog),
				     wvbox, gtk_label_new (_("Data")));

	data->data_entry = gl_prop_bc_data_new (_("Barcode data"),
						object->parent->merge_fields);
	gl_prop_bc_data_set_data (GL_PROP_BC_DATA (data->data_entry),
				  (object->parent->merge_type != GL_MERGE_NONE),
				  object->arg.barcode.text_node->field_flag,
				  object->arg.barcode.text_node->data,
				  object->arg.barcode.text_node->data);
	gtk_box_pack_start (GTK_BOX (wvbox), data->data_entry, FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (data->data_entry), "changed",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (dialog));

	/*---------------------------*/
	/* Appearance Notebook Tab   */
	/*---------------------------*/
	wvbox = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox), 10);
	gl_property_box_append_page (GL_PROPERTY_BOX (dialog),
				     wvbox, gtk_label_new (_("Appearance")));

	/* ------ Begin Barcode Properties Frame ------ */
	data->bcprop = gl_prop_bc_new (_("General"));
	gl_prop_bc_set_params (GL_PROP_BC (data->bcprop),
			       object->arg.barcode.scale,
			       object->arg.barcode.color);
	gtk_box_pack_start (GTK_BOX (wvbox), data->bcprop, FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (data->bcprop), "changed",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (dialog));

	/* ------ Barcode Style Frame ------ */
	data->style = gl_prop_bc_style_new (_("Style"));
	gl_prop_bc_style_set_params (GL_PROP_BC_STYLE (data->style),
				     object->arg.barcode.style,
				     object->arg.barcode.text_flag);
	gtk_box_pack_start (GTK_BOX (wvbox), data->style, FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (data->style), "changed",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (dialog));

	/*----------------------------*/
	/* Position Notebook Tab      */
	/*----------------------------*/
	wvbox = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox), 10);
	gl_property_box_append_page (GL_PROPERTY_BOX (dialog),
				     wvbox, gtk_label_new (_("Position")));

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
/* PRIVATE.  "Apply" callback.                                               */
/*---------------------------------------------------------------------------*/
static void
apply_cb (glPropertyBox * propertybox,
	  gint page,
	  PropertyDialogPassback * data)
{
	glLabelObject *object;
	glDisplay *display;
	gboolean flag;
	gchar *tmp1, *tmp2;

	object = gtk_object_get_data (GTK_OBJECT (data->item), "label_object");
	display = gtk_object_get_data (GTK_OBJECT (data->item), "display");

	switch (page) {

	case 0:
		/* ------- Get updated digit data ------ */
		gl_prop_bc_data_get_data (GL_PROP_BC_DATA (data->data_entry),
					  &flag, &tmp1, &tmp2);
		object->arg.barcode.text_node->field_flag = flag;
		if (!flag) {
			object->arg.barcode.text_node->data = tmp1;
			g_free (tmp2);
		} else {
			object->arg.barcode.text_node->data = tmp2;
			g_free (tmp1);
		}
		break;

	case 1:
		/* ------- Get updated scale/color ------ */
		gl_prop_bc_get_params (GL_PROP_BC (data->bcprop),
				       &object->arg.barcode.scale,
				       &object->arg.barcode.color);
		/* ------ Get updated style ------ */
		gl_prop_bc_style_get_params (GL_PROP_BC_STYLE (data->style),
					     &object->arg.barcode.style,
					     &object->arg.barcode.text_flag);
		break;

	case 2:
		/* ------ get updated position ------ */
		gl_prop_position_get_position (GL_PROP_POSITION
					       (data->position), &object->x,
					       &object->y);
		break;

	default:
		return;
	}

	/* ------ Udate state of object ------ */
	item_barcode_update (data->item);
	gl_display_select_item (display, data->item);

}

/*****************************************************************************/
/* Return apropos cursor for create object mode.                             */
/*****************************************************************************/
GdkCursor *
gl_item_barcode_get_create_cursor (void)
{
	static GdkCursor *cursor = NULL;
	GdkPixmap *pixmap_data, *pixmap_mask;
	GdkColor fg = { 0, 0, 0, 0 };
	GdkColor bg = { 0, 65535, 65535, 65535 };

	if (!cursor) {
		pixmap_data = gdk_bitmap_create_from_data (NULL,
							   cursor_barcode_bits,
							   cursor_barcode_width,
							   cursor_barcode_height);
		pixmap_mask = gdk_bitmap_create_from_data (NULL,
							   cursor_barcode_mask_bits,
							   cursor_barcode_mask_width,
							   cursor_barcode_mask_height);
		cursor =
		    gdk_cursor_new_from_pixmap (pixmap_data, pixmap_mask, &fg,
						&bg, cursor_barcode_x_hot,
						cursor_barcode_y_hot);
	}

	return cursor;
}

/*****************************************************************************/
/* Canvas event handler (barcode mode)                                       */
/*****************************************************************************/
int
gl_item_barcode_create_event_handler (GnomeCanvas * canvas,
				      GdkEvent * event,
				      gpointer data)
{
	glDisplay *display = GL_DISPLAY (data);
	gdouble x, y;
	glLabelObject *object;
	GnomeCanvasItem *item;

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			gnome_canvas_window_to_world (canvas,
						      event->button.x,
						      event->button.y, &x, &y);
			object =
			    gl_label_object_new (display->label,
						 GL_LABEL_OBJECT_BARCODE);
			object->x = x;
			object->y = y;
			object->arg.barcode.text_node =
			    gl_text_node_new_from_text ("123456789");
			object->arg.barcode.style = GL_BARCODE_STYLE_POSTNET;
			object->arg.barcode.color =
			    GNOME_CANVAS_COLOR_A (0, 0, 0, 255);
			object->arg.barcode.text_flag = TRUE;
			object->arg.barcode.scale = 1.0;
			item = gl_item_barcode_new (object, display);
			gl_display_add_item (display, item);
			gl_display_unselect_all (display);
			gl_display_select_item (display, item);
			gl_display_arrow_mode (display);
			return TRUE;

		default:
			return FALSE;
		}

	default:
		return FALSE;
	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw barcode to canvas item (group).                            */
/*---------------------------------------------------------------------------*/
static void
draw_barcode (GnomeCanvasItem * group,
	      glBarcodeStyle style,
	      guint color,
	      gboolean text_flag,
	      gdouble scale,
	      gchar * digits,
	      gdouble canvas_scale)
{
	glBarcodeLine *line;
	glBarcodeChar *bchar;
	glBarcode *gbc;
	GList *li;
	GnomeCanvasItem *item;
	GList *item_list = NULL;
	GnomeCanvasPoints *points;
	gchar *cstring;
#ifdef HACKTEXT
	GnomeFont *font;
	GnomeGlyphList *glyphlist;
	gdouble y_offset;
#else
	GdkFont *gdk_font;
	GnomeDisplayFont *display_font;
#endif

	gbc = gtk_object_get_data (GTK_OBJECT (group), "bc");
	gl_barcode_free (&gbc);
	gbc = gl_barcode (style, text_flag, scale, digits);
	if (gbc == NULL) {
#ifdef HACKTEXT
		font = gnome_font_new_closest (GL_BARCODE_FONT_FAMILY,
					       GL_BARCODE_FONT_WEIGHT,
					       FALSE, 10.0);
		cstring = _("Invalid barcode");
		glyphlist = gnome_glyphlist_from_text_sized_dumb (font,
								  color,
								  0.0, 0.0,
								  cstring,
								  strlen
								  (cstring));
		y_offset = 10.0 - gnome_font_get_descender (font);
		item = gnome_canvas_item_new (GNOME_CANVAS_GROUP (group),
					      gnome_canvas_hacktext_get_type (),
					      "x", 0.0,
					      "y", y_offset,
					      "glyphlist", glyphlist, NULL);
#else
		display_font = gnome_get_display_font (GL_BARCODE_FONT_FAMILY,
						       GL_BARCODE_FONT_WEIGHT,
						       FALSE,
						       10.0, canvas_scale);
		gdk_font = gnome_display_font_get_gdk_font (display_font);
		item = gnome_canvas_item_new (GNOME_CANVAS_GROUP (group),
					      gnome_canvas_text_get_type (),
					      "x", 0.0,
					      "y", 0.0,
					      "text", _("Invalid barcode"),
					      "font_gdk", gdk_font,
					      "anchor", GTK_ANCHOR_NW,
					      "fill_color_rgba", color, NULL);
#endif
		item_list = g_list_prepend (item_list, item);
	} else {

		points = gnome_canvas_points_new (2);
		for (li = gbc->lines; li != NULL; li = li->next) {
			line = (glBarcodeLine *) li->data;

			points->coords[0] = line->x;
			points->coords[1] = line->y;
			points->coords[2] = line->x;
			points->coords[3] = line->y + line->length;

			item =
			    gnome_canvas_item_new (GNOME_CANVAS_GROUP (group),
						   gnome_canvas_line_get_type
						   (), "points", points,
						   "width_units", line->width,
						   "fill_color_rgba", color,
						   NULL);
			item_list = g_list_prepend (item_list, item);
		}
		gnome_canvas_points_free (points);

		for (li = gbc->chars; li != NULL; li = li->next) {
			bchar = (glBarcodeChar *) li->data;

#ifdef HACKTEXT
			font = gnome_font_new_closest (GL_BARCODE_FONT_FAMILY,
						       GL_BARCODE_FONT_WEIGHT,
						       FALSE, bchar->fsize);
			glyphlist = gnome_glyphlist_from_text_sized_dumb (font,
									  color,
									  0.0,
									  0.0,
									  &
									  (bchar->
									   c),
									  1);
			y_offset =
			    bchar->fsize - gnome_font_get_descender (font);
			item =
			    gnome_canvas_item_new (GNOME_CANVAS_GROUP (group),
						   gnome_canvas_hacktext_get_type
						   (), "x", bchar->x, "y",
						   bchar->y + y_offset,
						   "glyphlist", glyphlist,
						   NULL);
#else
			display_font =
			    gnome_get_display_font (GL_BARCODE_FONT_FAMILY,
						    GL_BARCODE_FONT_WEIGHT,
						    FALSE, bchar->fsize,
						    canvas_scale);
			gdk_font =
			    gnome_display_font_get_gdk_font (display_font);
			cstring = g_strdup_printf ("%c", bchar->c);
			item =
			    gnome_canvas_item_new (GNOME_CANVAS_GROUP (group),
						   gnome_canvas_text_get_type
						   (), "x", bchar->x, "y",
						   bchar->y + Y_FONT_ADJUST,
						   "text", cstring, "font_gdk",
						   gdk_font, "anchor",
						   GTK_ANCHOR_NW,
						   "fill_color_rgba", color,
						   NULL);
			g_free (cstring);
#endif
			item_list = g_list_prepend (item_list, item);

		}

	}

	/* attach item list to group, so we can get at these again */
	gtk_object_set_data (GTK_OBJECT (group), "item_list", item_list);

	gtk_object_set_data (GTK_OBJECT (group), "bc", gbc);

}
