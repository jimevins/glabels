/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_text.c:  GLabels label text object widget
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

#include <glib.h>

#include <libgnomeprint/gnome-glyphlist.h>

#include "view-barcode.h"
#include "canvas-hacktext.h"
#include "view-highlight.h"

#include "color.h"
#include "object-editor.h"
#include "stock.h"

#include "pixmaps/cursor_barcode.xbm"
#include "pixmaps/cursor_barcode_mask.xbm"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glViewBarcodePrivate {

	GList           *item_list;

};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static glViewObjectClass *parent_class = NULL;


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void       gl_view_barcode_class_init            (glViewBarcodeClass  *klass);
static void       gl_view_barcode_instance_init         (glViewBarcode       *view_bc);
static void       gl_view_barcode_finalize              (GObject             *object);

static GtkWidget *construct_properties_editor           (glViewObject        *view_object);

static void       update_canvas_item_from_object_cb     (glLabelObject       *object,
							 glViewBarcode       *view_bc);

static void       update_object_from_editor_cb          (glObjectEditor      *editor,
							 glLabelObject       *object);

static void       update_editor_from_object_cb          (glLabelObject       *object,
							 glObjectEditor      *editor);

static void       update_editor_from_move_cb            (glLabelObject       *object,
							 gdouble              dx,
							 gdouble              dy,
							 glObjectEditor      *editor);

static void       update_editor_from_label_cb           (glLabel             *label,
							 glObjectEditor      *editor);

static void       draw_barcode                          (glViewBarcode       *view_bc);



/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_view_barcode_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (glViewBarcodeClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_view_barcode_class_init,
			NULL,
			NULL,
			sizeof (glViewBarcode),
			0,
			(GInstanceInitFunc) gl_view_barcode_instance_init,
			NULL
		};

		type = g_type_register_static (GL_TYPE_VIEW_OBJECT,
					       "glViewBarcode", &info, 0);
	}

	return type;
}

static void
gl_view_barcode_class_init (glViewBarcodeClass *klass)
{
	GObjectClass      *object_class      = (GObjectClass *) klass;
	glViewObjectClass *view_object_class = (glViewObjectClass *) klass;

	gl_debug (DEBUG_VIEW, "START");

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gl_view_barcode_finalize;

	view_object_class->construct_editor = construct_properties_editor;

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_barcode_instance_init (glViewBarcode *view_bc)
{
	gl_debug (DEBUG_VIEW, "START");

	view_bc->private = g_new0 (glViewBarcodePrivate, 1);

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_barcode_finalize (GObject *object)
{
	glLabel       *parent;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (object && GL_IS_VIEW_BARCODE (object));

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* NEW barcode object view.                                                  */
/*****************************************************************************/
glViewObject *
gl_view_barcode_new (glLabelBarcode *object,
		     glView         *view)
{
	glViewBarcode      *view_bc;
	GtkMenu            *menu;

	gl_debug (DEBUG_VIEW, "START");
	g_return_if_fail (object && GL_IS_LABEL_BARCODE (object));
	g_return_if_fail (view && GL_IS_VIEW (view));
	
	view_bc = g_object_new (gl_view_barcode_get_type(), NULL);

	gl_view_object_set_view (GL_VIEW_OBJECT(view_bc), view);
	gl_view_object_set_object (GL_VIEW_OBJECT(view_bc),
				   GL_LABEL_OBJECT(object),
				   GL_VIEW_HIGHLIGHT_BOX_RESIZABLE);

	/* Create analogous canvas items. */
	draw_barcode (view_bc);

	g_signal_connect (G_OBJECT (object), "changed",
			  G_CALLBACK (update_canvas_item_from_object_cb), view_bc);

	gl_debug (DEBUG_VIEW, "END");

	return GL_VIEW_OBJECT (view_bc);
}

/*****************************************************************************/
/* Create a properties editor for a barcode object.                          */
/*****************************************************************************/
static GtkWidget *
construct_properties_editor (glViewObject *view_object)
{
	GtkWidget          *editor;
	glViewBarcode      *view_bc = (glViewBarcode *)view_object;
	glLabelObject      *object;

	gl_debug (DEBUG_VIEW, "START");

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_bc));

	/* Build editor. */
	editor = gl_object_editor_new (GL_STOCK_BARCODE, _("Barcode object properties"),
				       GL_OBJECT_EDITOR_POSITION_PAGE,
				       GL_OBJECT_EDITOR_SIZE_PAGE,
				       GL_OBJECT_EDITOR_BC_PAGE,
				       GL_OBJECT_EDITOR_DATA_PAGE,
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
				   glViewBarcode *view_bc)
{
	gl_debug (DEBUG_VIEW, "START");

	/* Adjust appearance of analogous canvas item. */
	draw_barcode (view_bc);

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
	glTextNode        *text_node;
	gchar             *id;
	gboolean           text_flag, cs_flag;
	guint              color;
	guint              format_digits;

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

	text_node = gl_object_editor_get_data (editor);
	gl_label_barcode_set_data (GL_LABEL_BARCODE(object), text_node);
	gl_text_node_free (&text_node);

	gl_object_editor_get_bc_style (editor, &id, &text_flag, &cs_flag, &format_digits);
	color = gl_object_editor_get_bc_color (editor);
	gl_label_barcode_set_props (GL_LABEL_BARCODE(object),
				    id, text_flag, cs_flag, format_digits);
	gl_label_object_set_line_color (object, color);
	g_free (id);

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
	glTextNode        *text_node;
	gchar             *id;
	gboolean           text_flag, cs_flag;
	guint              color;
	glMerge           *merge;
	guint              format_digits;

	gl_debug (DEBUG_VIEW, "START");

	gl_label_object_get_size (object, &w, &h);
	gl_object_editor_set_size (editor, w, h);

	gl_label_barcode_get_props (GL_LABEL_BARCODE(object),
				    &id, &text_flag, &cs_flag, &format_digits);
	color = gl_label_object_get_line_color (object);
	gl_object_editor_set_bc_style (editor, id, text_flag, cs_flag, format_digits);
	gl_object_editor_set_bc_color (editor, color);
	g_free (id);

	text_node = gl_label_barcode_get_data (GL_LABEL_BARCODE(object));
	merge = gl_label_get_merge (GL_LABEL(object->parent));
	gl_object_editor_set_data (editor, (merge != NULL), text_node);
	gl_text_node_free (&text_node);


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
gl_view_barcode_get_create_cursor (void)
{
	static GdkCursor *cursor = NULL;
	GdkPixmap        *pixmap_data, *pixmap_mask;
	GdkColor         fg = { 0, 0, 0, 0 };
	GdkColor         bg = { 0, 65535, 65535, 65535 };

	gl_debug (DEBUG_VIEW, "START");

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

	gl_debug (DEBUG_VIEW, "END");

	return cursor;
}

/*****************************************************************************/
/* Canvas event handler for creating barcode objects.                            */
/*****************************************************************************/
int
gl_view_barcode_create_event_handler (GnomeCanvas *canvas,
				      GdkEvent    *event,
				      glView      *view)
{
	static gdouble      x0, y0;
	static gboolean     dragging = FALSE;
	static glViewObject *view_barcode;
	static GObject      *object;
	gdouble             x, y;
	glTextNode          *text_node;

	gl_debug (DEBUG_VIEW, "");

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		gl_debug (DEBUG_VIEW, "BUTTON_PRESS");
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
			object = gl_label_barcode_new (view->label);
			gl_label_object_set_position (GL_LABEL_OBJECT(object),
						     x, y);
			text_node = gl_text_node_new_from_text ("123456789");
			gl_label_barcode_set_data (GL_LABEL_BARCODE(object),
						   text_node);
			gl_label_barcode_set_props (GL_LABEL_BARCODE(object),
						    "POSTNET",
						    FALSE,
						    TRUE,
						    0);
			gl_label_object_set_line_color (GL_LABEL_OBJECT(object),
						    gl_color_set_opacity (
						      gl_view_get_default_line_color(view),
                                                      0.5));
			view_barcode = gl_view_barcode_new (GL_LABEL_BARCODE(object),
							    view);
			x0 = x;
			y0 = y;
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		gl_debug (DEBUG_VIEW, "BUTTON_RELEASE");
		switch (event->button.button) {
		case 1:
			dragging = FALSE;
			gnome_canvas_item_ungrab (canvas->root, event->button.time);
			gnome_canvas_window_to_world (canvas,
						      event->button.x,
						      event->button.y, &x, &y);
			gl_label_object_set_position (GL_LABEL_OBJECT(object),
						      x, y);
			gl_label_barcode_set_props (GL_LABEL_BARCODE(object),
						    "POSTNET",
						    FALSE,
						    TRUE,
						    0);
			gl_label_object_set_line_color (GL_LABEL_OBJECT(object),
							gl_view_get_default_line_color(view));
			gl_view_unselect_all (view);
			gl_view_object_select (GL_VIEW_OBJECT(view_barcode));
			gl_view_arrow_mode (view);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		gl_debug (DEBUG_VIEW, "MOTION_NOTIFY");
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			gnome_canvas_window_to_world (canvas,
						      event->motion.x,
						      event->motion.y, &x, &y);
			gl_label_object_set_position (GL_LABEL_OBJECT(object),
						      x, y);
			return TRUE;
		} else {
			return FALSE;
		}

	default:
		return FALSE;
	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Draw barcode to item (group).                                  */
/*--------------------------------------------------------------------------*/
static void
draw_barcode (glViewBarcode *view_barcode)
{
	glLabelObject    *object;
	GnomeCanvasItem  *item;
	glTextNode *text_node;
	gchar *id;
	gboolean text_flag;
	gboolean checksum_flag;
	guint color;
	gdouble w, h;
	glBarcodeLine *line;
	glBarcodeChar *bchar;
	glBarcode *gbc;
	GList *li;
	GList *item_list = NULL;
	GnomeCanvasPoints *points;
	gchar *digits, *cstring;
	GnomeFont *font;
	GnomeGlyphList *glyphlist;
	gdouble y_offset;
	guint format_digits;

	gl_debug (DEBUG_VIEW, "START");

	/* Query label object and properties */
	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_barcode));
	gl_label_barcode_get_props (GL_LABEL_BARCODE(object),
				    &id, &text_flag, &checksum_flag, &format_digits);
	color = gl_label_object_get_line_color (object);
	gl_label_object_get_size (object, &w, &h);
	text_node = gl_label_barcode_get_data(GL_LABEL_BARCODE(object));
	if (text_node->field_flag) {
		digits = gl_barcode_default_digits (id, format_digits);
	} else {
		digits = gl_text_node_expand (text_node, NULL);
	}

	/* remove previous items from group. */
	for (li = view_barcode->private->item_list; li!=NULL; li = li->next) {
		gl_debug (DEBUG_VIEW, "in loop");
		gtk_object_destroy (GTK_OBJECT (li->data));
	}
	gl_debug (DEBUG_VIEW, "1");
	g_list_free (view_barcode->private->item_list);
	view_barcode->private->item_list = NULL;
	gl_debug (DEBUG_VIEW, "2");

	/* get Gnome Font */
	font = gnome_font_find_closest_from_weight_slant (GL_BARCODE_FONT_FAMILY,
							  GL_BARCODE_FONT_WEIGHT,
							  FALSE,
							  10.0);

	gbc = gl_barcode_new (id, text_flag, checksum_flag, w, h, digits);
	if (gbc == NULL) {

		cstring = _("Invalid barcode data");
		glyphlist = gnome_glyphlist_from_text_sized_dumb (font,
								  color,
								  0.0, 0.0,
								  cstring,
								  strlen
								  (cstring));
		y_offset = 10.0 - fabs (gnome_font_get_descender (font));
		item = gl_view_object_item_new (GL_VIEW_OBJECT(view_barcode),
						gl_canvas_hacktext_get_type (),
						"x", 0.0,
						"y", y_offset,
						"glyphlist", glyphlist, NULL);

		gnome_glyphlist_unref (glyphlist);

		view_barcode->private->item_list =
			g_list_prepend (view_barcode->private->item_list, item);
	} else {

		points = gnome_canvas_points_new (2);
		for (li = gbc->lines; li != NULL; li = li->next) {
			line = (glBarcodeLine *) li->data;

			points->coords[0] = line->x;
			points->coords[1] = line->y;
			points->coords[2] = line->x;
			points->coords[3] = line->y + line->length;

			item = gl_view_object_item_new (GL_VIEW_OBJECT(view_barcode),
							gnome_canvas_line_get_type (),
							"points", points,
							"width_units", line->width,
							"fill_color_rgba", color,
							NULL);
			view_barcode->private->item_list =
				g_list_prepend (view_barcode->private->item_list, item);
		}
		gnome_canvas_points_free (points);

		for (li = gbc->chars; li != NULL; li = li->next) {
			bchar = (glBarcodeChar *) li->data;

			font = gnome_font_find_closest_from_weight_slant (
				                       GL_BARCODE_FONT_FAMILY,
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
			    bchar->fsize - fabs (gnome_font_get_descender (font));

			item = gl_view_object_item_new (GL_VIEW_OBJECT(view_barcode),
							gl_canvas_hacktext_get_type (),
							"x", bchar->x,
							"y", bchar->y + y_offset,
							"glyphlist", glyphlist,
							NULL);

			gnome_glyphlist_unref (glyphlist);

			view_barcode->private->item_list =
				g_list_prepend (view_barcode->private->item_list, item);

		}

	}

	/* clean up */
	gl_barcode_free (&gbc);
	g_free (digits);
	g_free (id);

	gl_debug (DEBUG_VIEW, "END");
}

