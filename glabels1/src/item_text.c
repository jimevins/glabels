/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  item_text.c:  GLabels Text Object (canvas item) module
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
#include "item_text.h"

#include "highlight.h"

#include "mdi.h"
#include "propertybox.h"
#include "prop_text_entry.h"
#include "prop_text.h"
#include "prop_position.h"

#include "pixmaps/cursor_text.xbm"
#include "pixmaps/cursor_text_mask.xbm"

#ifdef HACKTEXT
#include "gnome-canvas-hacktext.h"
#include <libgnomeprint/gnome-glyphlist.h>
#include "hack.h"
#endif

#include "debug.h"

#define DEFAULT_FONT_FAMILY      "Helvetica"
#define DEFAULT_FONT_SIZE        14.0
#define DEFAULT_FONT_WEIGHT      GNOME_FONT_BOOK
#define DEFAULT_FONT_ITALIC_FLAG FALSE

/*===========================================*/
/* Private data types                        */
/*===========================================*/

typedef struct {
	GnomeCanvasItem *item;

	/* Page 0 widgets */
	GtkWidget *text_entry;

	/* Page 1 widgets */
	GtkWidget *text;

	/* Page 2 widgets */
	GtkWidget *position;
} PropertyDialogPassback;

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void item_text_update (GnomeCanvasItem * item);

static void changed_cb (glPropertyBox * propertybox);
static void apply_cb (glPropertyBox * propertybox,
		      gint page, PropertyDialogPassback * data);

#ifdef HACKTEXT
static void draw_hacktext (GnomeCanvasItem * group,
			   gchar * text,
			   gchar * font_family,
			   GnomeFontWeight font_weight,
			   gboolean font_italic_flag,
			   gdouble font_size,
			   guint color,
			   GtkJustification just,
			   gdouble scale);

static void hacktext_get_size (gchar * text,
			       gchar * font_family,
			       GnomeFontWeight font_weight,
			       gboolean font_italic_flag,
			       gdouble font_size,
			       gdouble * w,
			       gdouble * h);
#endif

/****************************************************************************/
/* Create a text object                                                     */
/****************************************************************************/
GnomeCanvasItem *
gl_item_text_new (glLabelObject * object,
		  glDisplay * display)
{
	GnomeCanvasGroup *group;
	GnomeCanvasItem *item;
#ifndef HACKTEXT
	GdkFont *gdk_font;
	GnomeDisplayFont *display_font;
	GtkAnchorType anchor;
#endif
	gchar *text;

	text = gl_text_node_lines_expand (object->arg.text.lines, NULL);

#ifdef HACKTEXT

	group = gnome_canvas_root (GNOME_CANVAS (display->canvas));
	item = gnome_canvas_item_new (group, gnome_canvas_group_get_type (),
				      "x", object->x, "y", object->y, NULL);
	draw_hacktext (item,
		       text,
		       object->arg.text.font_family,
		       object->arg.text.font_weight,
		       object->arg.text.font_italic_flag,
		       object->arg.text.font_size,
		       object->arg.text.color,
		       object->arg.text.just, display->scale);

#else

	display_font = gnome_get_display_font (object->arg.text.font_family,
					       object->arg.text.font_weight,
					       object->arg.text.
					       font_italic_flag,
					       object->arg.text.font_size,
					       display->scale);
	gdk_font = gnome_display_font_get_gdk_font (display_font);

	switch (object->arg.text.just) {
	case GTK_JUSTIFY_LEFT:
		anchor = GTK_ANCHOR_NW;
		break;
	case GTK_JUSTIFY_CENTER:
		anchor = GTK_ANCHOR_N;
		break;
	case GTK_JUSTIFY_RIGHT:
		anchor = GTK_ANCHOR_NE;
		break;
	default:
		anchor = GTK_ANCHOR_NW;
		break;
	}

	group = gnome_canvas_root (GNOME_CANVAS (display->canvas));
	item = gnome_canvas_item_new (group, gnome_canvas_text_get_type (),
				      "x", object->x,
				      "y", object->y,
				      "text", g_strdup (text),
				      "font_gdk", gdk_font,
				      "justification", object->arg.text.just,
				      "anchor", anchor,
				      "fill_color_rgba", object->arg.text.color,
				      NULL);

#endif

	/* Squirrel away pointers to object and display in the canvas item. */
	gtk_object_set_data (GTK_OBJECT (item), "label_object", object);
	gtk_object_set_data (GTK_OBJECT (item), "display", display);
	gtk_object_set_data (GTK_OBJECT (item), "highlight", NULL);

	gtk_signal_connect (GTK_OBJECT (item), "event",
			    GTK_SIGNAL_FUNC (gl_display_item_event_handler),
			    display);

	gl_display_new_item_menu (item);

	gl_display_set_modified (display);

	g_free (text);
	return item;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE. Update a text object                                            */
/*--------------------------------------------------------------------------*/
static void
item_text_update (GnomeCanvasItem * item)
{
	glLabelObject *object;
	glDisplay *display;
	gdouble affine[6];
#ifdef HACKTEXT
	GList *item_list, *li;
#else
	GdkFont *gdk_font;
	GnomeDisplayFont *display_font;
	GtkAnchorType anchor;
#endif
	gchar *text;

	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");
	display = gtk_object_get_data (GTK_OBJECT (item), "display");

	text = gl_text_node_lines_expand (object->arg.text.lines, NULL);

#ifdef HACKTEXT

	item_list = gtk_object_get_data (GTK_OBJECT (item), "item_list");
	for (li = item_list; li != NULL; li = li->next) {
		gtk_object_destroy (GTK_OBJECT (li->data));
	}
	g_list_free (item_list);
	gtk_object_set_data (GTK_OBJECT (item), "item_list", NULL);

	draw_hacktext (item,
		       text,
		       object->arg.text.font_family,
		       object->arg.text.font_weight,
		       object->arg.text.font_italic_flag,
		       object->arg.text.font_size,
		       object->arg.text.color,
		       object->arg.text.just, display->scale);

	art_affine_identity (affine);
	gnome_canvas_item_affine_absolute (item, affine);
	gnome_canvas_item_set (item, "x", object->x, "y", object->y, NULL);

#else

	display_font = gnome_get_display_font (object->arg.text.font_family,
					       object->arg.text.font_weight,
					       object->arg.text.
					       font_italic_flag,
					       object->arg.text.font_size,
					       display->scale);
	gdk_font = gnome_display_font_get_gdk_font (display_font);

	switch (object->arg.text.just) {
	case GTK_JUSTIFY_LEFT:
		anchor = GTK_ANCHOR_NW;
		break;
	case GTK_JUSTIFY_CENTER:
		anchor = GTK_ANCHOR_N;
		break;
	case GTK_JUSTIFY_RIGHT:
		anchor = GTK_ANCHOR_NE;
		break;
	default:
		anchor = GTK_ANCHOR_NW;
		break;
	}

	art_affine_identity (affine);
	gnome_canvas_item_affine_absolute (item, affine);
	gnome_canvas_item_set (item,
			       "x", object->x,
			       "y", object->y,
			       "text", g_strdup (text),
			       "font_gdk", gdk_font,
			       "justification", object->arg.text.just,
			       "anchor", anchor,
			       "fill_color_rgba", object->arg.text.color, NULL);

#endif

	g_free (text);

	gl_display_set_modified (display);
}

/****************************************************************************/
/* Highlight item.                                                          */
/****************************************************************************/
void
gl_item_text_highlight (GnomeCanvasItem * item)
{
	gl_highlight (item, GL_HIGHLIGHT_SIMPLE);
}

/****************************************************************************/
/* Get position and size of object.                                         */
/****************************************************************************/
void
gl_item_text_get_position_size (GnomeCanvasItem * item,
				gdouble * x,
				gdouble * y,
				gdouble * w,
				gdouble * h)
{
	glLabelObject *object;
#ifndef HACKTEXT
	GtkArg arg_info;
	gdouble x1, y1, x2, y2;
#else
	gchar *text;
#endif

	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");

	*x = object->x;
	*y = object->y;

#ifdef HACKTEXT
	text = gl_text_node_lines_expand (object->arg.text.lines, NULL);

	hacktext_get_size (text,
			   object->arg.text.font_family,
			   object->arg.text.font_weight,
			   object->arg.text.font_italic_flag,
			   object->arg.text.font_size, w, h);

	g_free (text);
#else

	gnome_canvas_item_get_bounds (item, &x1, &y1, &x2, &y2);
	*h = y2 - y1;

	/* workaround: the above bounding box is too wide, correct the width. */
	arg_info.name = "text_width";
	gtk_object_getv (GTK_OBJECT (item), 1, &arg_info);
	*w = GTK_VALUE_DOUBLE (arg_info);

#endif
}

/****************************************************************************/
/* Get position and size of object.                                         */
/****************************************************************************/
void
gl_item_text_set_position_size (GnomeCanvasItem * item,
				gdouble x,
				gdouble y,
				gdouble w,
				gdouble h)
{
	glLabelObject *object;

	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");

	object->x = x;
	object->y = y;

	/* Ignore w,h for now */

	item_text_update (item);
}

/****************************************************************************/
/* Get text item bounds.                                                    */
/****************************************************************************/
void
gl_item_text_get_bounds (GnomeCanvasItem * item,
			 gdouble * x1,
			 gdouble * y1,
			 gdouble * x2,
			 gdouble * y2)
{
#ifdef HACKTEXT

	glLabelObject *object;
	gdouble w, h, x_offset, y_offset;
	gchar *text;

	object = gtk_object_get_data (GTK_OBJECT (item), "label_object");

	text = gl_text_node_lines_expand (object->arg.text.lines, NULL);

	hacktext_get_size (text,
			   object->arg.text.font_family,
			   object->arg.text.font_weight,
			   object->arg.text.font_italic_flag,
			   object->arg.text.font_size, &w, &h);

	g_free (text);

	switch (object->arg.text.just) {
	case GTK_JUSTIFY_LEFT:
		x_offset = 0.0;
		break;
	case GTK_JUSTIFY_CENTER:
		x_offset = -w / 2.0;
		break;
	case GTK_JUSTIFY_RIGHT:
		x_offset = -w;
		break;
	default:
		x_offset = 0.0;
		break;		/* shouldn't happen */
	}

	y_offset = 0.0;

	*x1 = object->x + x_offset;
	*y1 = object->y + y_offset;
	*x2 = *x1 + w;
	*y2 = *y1 + h;

#else

	GtkArg arg_info;

	gnome_canvas_item_get_bounds (item, x1, y1, x2, y2);

	/* workaround: the above bounding box is too wide, correct the width. */
	arg_info.name = "text_width";
	gtk_object_getv (GTK_OBJECT (item), 1, &arg_info);
	*x2 = *x1 + GTK_VALUE_DOUBLE (arg_info);

#endif

}

/****************************************************************************/
/* Create and run an edit dialog on a text object.                          */
/****************************************************************************/
void
gl_item_text_edit_dialog (GnomeCanvasItem * item)
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
			      _("Edit text object properties"));
	gtk_signal_connect (GTK_OBJECT (dialog), "apply",
			    GTK_SIGNAL_FUNC (apply_cb), data);

	/*---------------------------*/
	/* Text Notebook Tab         */
	/*---------------------------*/
	wvbox = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox), 10);
	gl_property_box_append_page (GL_PROPERTY_BOX (dialog), wvbox,
				     gtk_label_new (_("Text")));

	/* Text entry widget */
	data->text_entry =
	    gl_prop_text_entry_new (_("Text"), object->parent->merge_fields);
	gl_prop_text_entry_set_text (GL_PROP_TEXT_ENTRY (data->text_entry),
				     (object->parent->merge_type !=
				      GL_MERGE_NONE), object->arg.text.lines);
	gtk_box_pack_start (GTK_BOX (wvbox), data->text_entry, FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (data->text_entry), "changed",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (dialog));

	/*---------------------------*/
	/* Appearance Notebook Tab   */
	/*---------------------------*/
	wvbox = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox), 10);
	gl_property_box_append_page (GL_PROPERTY_BOX (dialog), wvbox,
				     gtk_label_new (_("Appearance")));

	/* ------ Begin Text Properties box ------ */
	data->text = gl_prop_text_new (_("Text Properties"));
	gl_prop_text_set_params (GL_PROP_TEXT (data->text),
				 object->arg.text.font_family,
				 object->arg.text.font_size,
				 object->arg.text.font_weight,
				 object->arg.text.font_italic_flag,
				 object->arg.text.color, object->arg.text.just);
	gtk_box_pack_start (GTK_BOX (wvbox), data->text, FALSE, FALSE, 0);
	gtk_signal_connect_object (GTK_OBJECT (data->text), "changed",
				   GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (dialog));

	/*---------------------------*/
	/* Position Notebook Tab     */
	/*---------------------------*/
	wvbox = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox), 10);
	gl_property_box_append_page (GL_PROPERTY_BOX (dialog), wvbox,
				     gtk_label_new (_("Position")));

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

	object = gtk_object_get_data (GTK_OBJECT (data->item), "label_object");
	display = gtk_object_get_data (GTK_OBJECT (data->item), "display");

	switch (page) {

	case 0:
		/* ------- Get updated text ------ */
		gl_text_node_lines_free (&object->arg.text.lines);
		object->arg.text.lines =
		    gl_prop_text_entry_get_text (GL_PROP_TEXT_ENTRY
						 (data->text_entry));
		break;

	case 1:
		/* ------ Get updated text parameter information ------ */
		gl_prop_text_get_params (GL_PROP_TEXT (data->text),
					 &object->arg.text.font_family,
					 &object->arg.text.font_size,
					 &object->arg.text.font_weight,
					 &object->arg.text.font_italic_flag,
					 &object->arg.text.color,
					 &object->arg.text.just);
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
	item_text_update (data->item);
	gl_display_select_item (display, data->item);

}

/****************************************************************************/
/* Return apropos cursor for create object mode.                            */
/****************************************************************************/
GdkCursor *
gl_item_text_get_create_cursor (void)
{
	static GdkCursor *cursor = NULL;
	GdkPixmap *pixmap_data, *pixmap_mask;
	GdkColor fg = { 0, 0, 0, 0 };
	GdkColor bg = { 0, 65535, 65535, 65535 };

	if (!cursor) {
		pixmap_data = gdk_bitmap_create_from_data (NULL,
							   cursor_text_bits,
							   cursor_text_width,
							   cursor_text_height);
		pixmap_mask = gdk_bitmap_create_from_data (NULL,
							   cursor_text_mask_bits,
							   cursor_text_mask_width,
							   cursor_text_mask_height);
		cursor =
		    gdk_cursor_new_from_pixmap (pixmap_data, pixmap_mask, &fg,
						&bg, cursor_text_x_hot,
						cursor_text_y_hot);
	}

	return cursor;
}

/****************************************************************************/
/* Canvas event handler (text mode)                                         */
/****************************************************************************/
gint
gl_item_text_create_event_handler (GnomeCanvas * canvas,
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
						 GL_LABEL_OBJECT_TEXT);
			object->x = x;
			object->y = y;
			object->arg.text.lines =
			    gl_text_node_lines_new_from_text (_("Text"));
			object->arg.text.font_family =
			    g_strdup (DEFAULT_FONT_FAMILY);
			object->arg.text.font_size = DEFAULT_FONT_SIZE;
			object->arg.text.font_weight = DEFAULT_FONT_WEIGHT;
			object->arg.text.font_italic_flag =
			    DEFAULT_FONT_ITALIC_FLAG;
			object->arg.text.just = GTK_JUSTIFY_LEFT;
			object->arg.text.color = GNOME_CANVAS_COLOR (0, 0, 0);
			item = gl_item_text_new (object, display);
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

#ifdef HACKTEXT
/*--------------------------------------------------------------------------*/
/* PRIVATE.  Draw hacktext to item (group).                                 */
/*--------------------------------------------------------------------------*/
static void
draw_hacktext (GnomeCanvasItem * group,
	       gchar * text,
	       gchar * font_family,
	       GnomeFontWeight font_weight,
	       gboolean font_italic_flag,
	       gdouble font_size,
	       guint color,
	       GtkJustification just,
	       gdouble scale)
{
	GnomeFont *font;
	GnomeGlyphList *glyphlist;
	gdouble x_offset, y_offset;
	gint i;
	gdouble w;
	gchar **line, *utf8_text;
	GnomeCanvasItem *item;
	GList *item_list = NULL;

	font = gnome_font_new_closest (font_family,
				       font_weight,
				       font_italic_flag, font_size);

	line = g_strsplit (text, "\n", -1);

	for (i = 0; line[i] != NULL; i++) {

		utf8_text = gl_hack_text_to_utf8 (line[i]);

		w = gl_hack_get_width_string (font, line[i]);

		glyphlist = gnome_glyphlist_from_text_sized_dumb (font,
								  color,
								  0.0, 0.0,
								  utf8_text,
								  strlen
								  (utf8_text));
		switch (just) {
		case GTK_JUSTIFY_LEFT:
			x_offset = 0.0;
			break;
		case GTK_JUSTIFY_CENTER:
			x_offset = -w / 2.0;
			break;
		case GTK_JUSTIFY_RIGHT:
			x_offset = -w;
			break;
		default:
			x_offset = 0.0;
			break;	/* shouldn't happen */
		}

		y_offset =
		    (i + 1) * font_size - gnome_font_get_descender (font);

		item = gnome_canvas_item_new (GNOME_CANVAS_GROUP (group),
					      gnome_canvas_hacktext_get_type (),
					      "x", x_offset,
					      "y", y_offset,
					      "glyphlist", glyphlist, NULL);
		item_list = g_list_prepend (item_list, item);

		g_free (utf8_text);
	}

	g_strfreev (line);

	/* attach item list to group, so we can get at these again */
	gtk_object_set_data (GTK_OBJECT (group), "item_list", item_list);
}
#endif

#ifdef HACKTEXT
/*--------------------------------------------------------------------------*/
/* PRIVATE.  get size of hacktext item.                                     */
/*--------------------------------------------------------------------------*/
static void
hacktext_get_size (gchar * text,
		   gchar * font_family,
		   GnomeFontWeight font_weight,
		   gboolean font_italic_flag,
		   gdouble font_size,
		   gdouble * w,
		   gdouble * h)
{
	GnomeFont *font;
	gchar **line;
	gdouble wline;
	gint i;

	*w = 0.0;
	*h = 0.0;

	font = gnome_font_new_closest (font_family,
				       font_weight,
				       font_italic_flag, font_size);

	line = g_strsplit (text, "\n", -1);

	for (i = 0; line[i] != NULL; i++) {

		wline = gl_hack_get_width_string (font, line[i]);

		if (wline > *w)
			*w = wline;
		*h += font_size;

	}

	g_strfreev (line);

}
#endif
