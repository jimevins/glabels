/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_text.c:  GLabels label text object widget
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

#include <libgnomeprint/gnome-glyphlist.h>

#include "view-text.h"
#include "canvas-hacktext.h"
#include "view-highlight.h"
#include "wdgt-text-entry.h"
#include "wdgt-text-props.h"
#include "wdgt-position.h"
#include "color.h"
#include "hig.h"

#include "pixmaps/cursor_text.xbm"
#include "pixmaps/cursor_text_mask.xbm"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

#define CURSOR_COLOR    GL_COLOR_A (0, 0, 255, 128)

#define CURSOR_ON_TIME  800
#define CURSOR_OFF_TIME 400

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glViewTextPrivate {

	GList           *item_list;

	GnomeCanvasItem *cursor;
	gboolean         cursor_visible;
	gboolean         cursor_state;
	guint            cursor_timeout;

	/* Page 0 widgets */
	GtkWidget *text_entry;

	/* Page 1 widgets */
	GtkWidget *text_props;

	/* Page 2 widgets */
	GtkWidget *position;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static glViewObjectClass *parent_class = NULL;


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void      gl_view_text_class_init       (glViewTextClass *klass);
static void      gl_view_text_instance_init    (glViewText      *view_text);
static void      gl_view_text_finalize         (GObject         *object);

static void      update_view_text_cb           (glLabelObject   *object,
						glViewText      *view_text);

static GtkWidget *construct_properties_dialog  (glViewObject    *view_object);

static void      response_cb                   (GtkDialog       *dialog,
						gint             response,
						glViewText      *view_text);

static void      text_props_changed_cb         (glWdgtTextProps *text_props,
						glViewText      *view_text);

static void      position_changed_cb           (glWdgtPosition  *position,
						glViewText      *view_text);

static void      update_dialog_cb              (glLabelObject   *object,
						glViewText      *view_text);

static void      update_dialog_from_move_cb    (glLabelObject   *object,
						gdouble          dx,
						gdouble          dy,
						glViewText      *view_text);

static void      draw_hacktext                 (glViewText      *view_text);

static void      draw_cursor                   (glViewText      *view_text);

static void      mark_set_cb                   (GtkTextBuffer   *textbuffer,
						GtkTextIter     *iter,
						GtkTextMark     *mark,
						glViewText      *view_text);

static void      blink_start                   (glViewText      *view_text);
static void      blink_stop                    (glViewText      *view_text);
static gboolean  blink_cb                      (glViewText      *view_text);

static gint      item_event_cb                 (GnomeCanvasItem *item,
						GdkEvent        *event,
						glViewObject    *view_object);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_view_text_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (glViewTextClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_view_text_class_init,
			NULL,
			NULL,
			sizeof (glViewText),
			0,
			(GInstanceInitFunc) gl_view_text_instance_init,
		};

		type = g_type_register_static (GL_TYPE_VIEW_OBJECT,
					       "glViewText", &info, 0);
	}

	return type;
}

static void
gl_view_text_class_init (glViewTextClass *klass)
{
	GObjectClass      *object_class      = (GObjectClass *) klass;
	glViewObjectClass *view_object_class = (glViewObjectClass *) klass;

	gl_debug (DEBUG_VIEW, "START");

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gl_view_text_finalize;

	view_object_class->construct_dialog = construct_properties_dialog;

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_text_instance_init (glViewText *view_text)
{
	gl_debug (DEBUG_VIEW, "START");

	view_text->private = g_new0 (glViewTextPrivate, 1);

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_text_finalize (GObject *object)
{
	glLabel       *parent;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (object && GL_IS_VIEW_TEXT (object));

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* NEW text object view.                                                  */
/*****************************************************************************/
glViewObject *
gl_view_text_new (glLabelText *object,
		  glView     *view)
{
	glViewText         *view_text;
	GtkMenu            *menu;
	GtkTextBuffer      *buffer;
	GnomeCanvasItem    *group;

	gl_debug (DEBUG_VIEW, "START");
	g_return_if_fail (object && GL_IS_LABEL_TEXT (object));
	g_return_if_fail (view && GL_IS_VIEW (view));
	
	view_text = g_object_new (gl_view_text_get_type(), NULL);

	gl_view_object_set_view (GL_VIEW_OBJECT(view_text), view);
	gl_view_object_set_object (GL_VIEW_OBJECT(view_text),
				   GL_LABEL_OBJECT(object),
				   GL_VIEW_HIGHLIGHT_BOX_RESIZABLE);

	group = gl_view_object_get_group (GL_VIEW_OBJECT(view_text));

	/* Create analogous canvas item. */
	draw_hacktext (view_text);
	draw_cursor (view_text);

	g_signal_connect (G_OBJECT (object), "changed",
			  G_CALLBACK (update_view_text_cb), view_text);

	g_signal_connect (G_OBJECT (group), "event",
			  G_CALLBACK (item_event_cb), view_text);

	buffer = gl_label_text_get_buffer (object);
	g_signal_connect (G_OBJECT (buffer), "mark-set",
			  G_CALLBACK (mark_set_cb), view_text);

	gl_debug (DEBUG_VIEW, "END");

	return GL_VIEW_OBJECT (view_text);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "changed" callback.                                 */
/*---------------------------------------------------------------------------*/
static void
update_view_text_cb (glLabelObject *object,
		     glViewText    *view_text)
{
	gl_debug (DEBUG_VIEW, "START");

	/* Adjust appearance of analogous canvas item. */
	draw_hacktext (view_text);
	draw_cursor (view_text);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Create a properties dialog for a text object.                          */
/*****************************************************************************/
static GtkWidget *
construct_properties_dialog (glViewObject *view_object)
{
	glViewText        *view_text = (glViewText *)view_object;
	GtkWidget         *dialog, *wsection, *wbutton;
	glLabelObject     *object;
	gdouble            x, y, w, h, label_width, label_height;
	GtkTextBuffer     *buffer;
	gchar             *font_family;
	gdouble            font_size;
	GnomeFontWeight    font_weight;
	gboolean           font_italic_flag;
	guint              color;
	GtkJustification   just;
	glMerge           *merge;
	GtkSizeGroup      *label_size_group;
	GtkWidget         *window;

	gl_debug (DEBUG_VIEW, "START");

	/* retrieve object and query parameters */
	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_text));
	gl_label_object_get_position (GL_LABEL_OBJECT(object), &x, &y);
	buffer = gl_label_text_get_buffer(GL_LABEL_TEXT(object));
	gl_label_text_get_props (GL_LABEL_TEXT(object),
				 &font_family, &font_size,
				 &font_weight, &font_italic_flag,
				 &color, &just);
	gl_label_get_size (GL_LABEL(object->parent),
			   &label_width, &label_height);
	merge = gl_label_get_merge (GL_LABEL(object->parent));

	/*-----------------------------------------------------------------*/
	/* Build dialog.                                                   */
	/*-----------------------------------------------------------------*/
	window = gtk_widget_get_toplevel (
		GTK_WIDGET(gl_view_object_get_view(GL_VIEW_OBJECT(view_text))));
	dialog = gl_hig_dialog_new_with_buttons ( _("Edit text object properties"),
						  GTK_WINDOW (window),
						  GTK_DIALOG_DESTROY_WITH_PARENT,
						  GTK_STOCK_CLOSE,
					                   GTK_RESPONSE_CLOSE,
						  NULL );
        gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK (response_cb), view_text);

	label_size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	/*---------------------------*/
	/* Text Section              */
	/*---------------------------*/
	wsection = gl_hig_category_new (_("Text"));
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dialog), wsection);
	view_text->private->text_entry =
		gl_wdgt_text_entry_new (merge);
	gl_wdgt_text_entry_set_label_size_group (GL_WDGT_TEXT_ENTRY(view_text->private->text_entry),
						 label_size_group);
	gl_wdgt_text_entry_set_buffer (GL_WDGT_TEXT_ENTRY(view_text->private->text_entry),
				       buffer);
	gl_wdgt_text_entry_set_field_defs (GL_WDGT_TEXT_ENTRY(view_text->private->text_entry),
					     merge);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wsection),
				    view_text->private->text_entry);


	/*---------------------------*/
	/* Text Properties section   */
	/*---------------------------*/
	wsection = gl_hig_category_new (_("Properties"));
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dialog), wsection);
	view_text->private->text_props = gl_wdgt_text_props_new ();
	gl_wdgt_text_props_set_label_size_group (GL_WDGT_TEXT_PROPS(view_text->private->text_props),
						 label_size_group);
	gl_wdgt_text_props_set_params (GL_WDGT_TEXT_PROPS(view_text->private->text_props),
				       font_family, font_size, font_weight,
				       font_italic_flag, color, just);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wsection),
				    view_text->private->text_props);
	g_signal_connect ( G_OBJECT(view_text->private->text_props),
			   "changed", G_CALLBACK (text_props_changed_cb),
			   view_text);


	/*----------------------------*/
	/* Position section           */
	/*----------------------------*/
	wsection = gl_hig_category_new (_("Position"));
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dialog), wsection);
	view_text->private->position = gl_wdgt_position_new ();
	gl_wdgt_position_set_label_size_group (GL_WDGT_POSITION(view_text->private->position),
					       label_size_group);
	gl_wdgt_position_set_params (GL_WDGT_POSITION (view_text->private->position),
				     x, y,
				     label_width, label_height);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wsection),
				    view_text->private->position);
	g_signal_connect (G_OBJECT (view_text->private->position),
			  "changed",
			  G_CALLBACK(position_changed_cb), view_text);


	/*----------------------------*/
	/* Track object changes.      */
	/*----------------------------*/
	g_signal_connect (G_OBJECT (object), "changed",
			  G_CALLBACK (update_dialog_cb), view_text);
	g_signal_connect (G_OBJECT (object), "moved",
			  G_CALLBACK (update_dialog_from_move_cb), view_text);

	gl_debug (DEBUG_VIEW, "END");

	return dialog;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Response" callback.                                            */
/*---------------------------------------------------------------------------*/
static void
response_cb (GtkDialog     *dialog,
	     gint           response,
	     glViewText    *view_text)
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
		object = gl_view_object_get_object (GL_VIEW_OBJECT(view_text));

		g_signal_handlers_disconnect_by_func (object, update_dialog_cb,
						      view_text);
		g_signal_handlers_disconnect_by_func (object, update_dialog_from_move_cb,
						      view_text);
		break;
	default:
		g_print ("response = %d", response);
		g_assert_not_reached();
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  text_props "changed" callback.                                  */
/*---------------------------------------------------------------------------*/
static void
text_props_changed_cb (glWdgtTextProps  *text_props,
		       glViewText       *view_text)
{
	glLabelObject      *object;
	gchar              *font_family;
	gdouble            font_size;
	GnomeFontWeight    font_weight;
	gboolean           font_italic_flag;
	guint              color;
	GtkJustification   just;


	gl_debug (DEBUG_VIEW, "START");

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_text));

	gl_wdgt_text_props_get_params (text_props,
				       &font_family, &font_size, &font_weight,
				       &font_italic_flag,
				       &color, &just);

	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_dialog_cb, view_text);
	gl_label_text_set_props (GL_LABEL_TEXT(object),
				 font_family, font_size, font_weight,
				 font_italic_flag,
				 color, just);
	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_dialog_cb, view_text);

	g_free (font_family);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  position "changed" callback.                                    */
/*---------------------------------------------------------------------------*/
static void
position_changed_cb (glWdgtPosition     *position,
		     glViewText         *view_text)
{
	glLabelObject      *object;
	gdouble            x, y;

	gl_debug (DEBUG_VIEW, "START");

	gl_wdgt_position_get_position (GL_WDGT_POSITION (position), &x, &y);

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_text));

	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_dialog_cb, view_text);
	gl_label_object_set_position (GL_LABEL_OBJECT(object), x, y);
	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_dialog_cb, view_text);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "changed" callback.                                 */
/*---------------------------------------------------------------------------*/
static void
update_dialog_cb (glLabelObject  *object,
		  glViewText     *view_text)
{
	gdouble            x, y;
	glMerge            *merge;
	gchar              *font_family;
	gdouble            font_size;
	GnomeFontWeight    font_weight;
	gboolean           font_italic_flag;
	guint              color;
	GtkJustification   just;

	gl_debug (DEBUG_VIEW, "START");

	/* Query properties of object. */
	gl_label_text_get_props (GL_LABEL_TEXT(object),
				 &font_family, &font_size,
				 &font_weight, &font_italic_flag,
				 &color, &just);
	gl_label_object_get_position (GL_LABEL_OBJECT(object), &x, &y);
	merge = gl_label_get_merge (GL_LABEL(object->parent));

	/* Block widget handlers to prevent recursion */
	g_signal_handlers_block_by_func (G_OBJECT(view_text->private->text_props),
					 text_props_changed_cb, view_text);
	g_signal_handlers_block_by_func (G_OBJECT(view_text->private->position),
					 position_changed_cb, view_text);

	/* Update widgets in property dialog */

	gl_wdgt_text_entry_set_field_defs (GL_WDGT_TEXT_ENTRY(view_text->private->text_entry),
					   merge);
	gl_wdgt_text_props_set_params (GL_WDGT_TEXT_PROPS(view_text->private->text_props),
				       font_family, font_size, font_weight,
				       font_italic_flag, color, just);
	gl_wdgt_position_set_position (GL_WDGT_POSITION(view_text->private->position),
				       x, y);

	/* Unblock widget handlers */
	g_signal_handlers_unblock_by_func (G_OBJECT(view_text->private->text_props),
					   text_props_changed_cb, view_text);
	g_signal_handlers_unblock_by_func (G_OBJECT(view_text->private->position),
					   position_changed_cb, view_text);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "moved" callback.                                   */
/*---------------------------------------------------------------------------*/
static void
update_dialog_from_move_cb (glLabelObject *object,
			    gdouble        dx,
			    gdouble        dy,
			    glViewText    *view_text)
{
	gdouble            x, y;

	gl_debug (DEBUG_VIEW, "START");

	/* Query properties of object. */
	gl_label_object_get_position (GL_LABEL_OBJECT(object), &x, &y);

	/* Block widget handlers to prevent recursion */
	g_signal_handlers_block_by_func (G_OBJECT(view_text->private->position),
					 position_changed_cb, view_text);

	/* Update widgets in property dialog */
	gl_wdgt_position_set_position (GL_WDGT_POSITION(view_text->private->position),
				       x, y);

	/* Unblock widget handlers */
	g_signal_handlers_unblock_by_func (G_OBJECT(view_text->private->position),
					   position_changed_cb, view_text);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Return apropos cursor for create object mode.                             */
/*****************************************************************************/
GdkCursor *
gl_view_text_get_create_cursor (void)
{
	static GdkCursor *cursor = NULL;
	GdkPixmap        *pixmap_data, *pixmap_mask;
	GdkColor         fg = { 0, 0, 0, 0 };
	GdkColor         bg = { 0, 65535, 65535, 65535 };

	gl_debug (DEBUG_VIEW, "START");

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

	gl_debug (DEBUG_VIEW, "END");

	return cursor;
}

/*****************************************************************************/
/* Canvas event handler for creating text objects.                           */
/*****************************************************************************/
int
gl_view_text_create_event_handler (GnomeCanvas *canvas,
				      GdkEvent    *event,
				      glView      *view)
{
	static gdouble      x0, y0;
	static gboolean     dragging = FALSE;
	static glViewObject *view_text;
	static GObject      *object;
	gdouble             x, y;
	GList               *lines;
	gchar               *family;

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
			object = gl_label_text_new (view->label);
			gl_label_object_set_position (GL_LABEL_OBJECT(object),
						     x, y);
			family = gl_view_get_default_font_family (view);
			gl_label_text_set_props (GL_LABEL_TEXT(object),
						 family,
						 gl_view_get_default_font_size (view),
						 gl_view_get_default_font_weight (view),
						 gl_view_get_default_font_italic_flag (view),
						 gl_color_set_opacity (gl_view_get_default_text_color (view), 0.5),
						 gl_view_get_default_text_alignment (view));
			g_free (family);
			lines = gl_text_node_lines_new_from_text (_("Text"));
			gl_label_text_set_lines (GL_LABEL_TEXT(object), lines);
			view_text = gl_view_text_new (GL_LABEL_TEXT(object),
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
			family = gl_view_get_default_font_family (view);
			gl_label_text_set_props (GL_LABEL_TEXT(object),
						 family,
						 gl_view_get_default_font_size (view),
						 gl_view_get_default_font_weight (view),
						 gl_view_get_default_font_italic_flag (view),
						 gl_view_get_default_text_color (view),
						 gl_view_get_default_text_alignment (view));
			g_free (family);
			gl_view_unselect_all (view);
			gl_view_object_select (GL_VIEW_OBJECT(view_text));
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
/* PRIVATE.  Draw hacktext to item (group).                                 */
/*--------------------------------------------------------------------------*/
static void
draw_hacktext (glViewText *view_text)
{
	glLabelObject     *object;
	GnomeCanvasItem   *item;
	GtkTextBuffer     *buffer;
	GtkTextIter        start, end;
	gchar             *text;
	gchar             *font_family;
	GnomeFontWeight    font_weight;
	gboolean           font_italic_flag;
	gdouble            font_size;
	guint              color;
	GtkJustification   just;
	GnomeFont         *font;
	GnomeGlyphList    *glyphlist;
	ArtDRect           bbox;
	gdouble            affine[6];
	gdouble            x_offset, y_offset, w, object_w, object_h;
	gint               i;
	gchar            **line;
	GList             *li;

	gl_debug (DEBUG_VIEW, "START");

	/* Query label object and properties */
	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_text));
	gl_label_object_get_size (object, &object_w, &object_h);
	gl_label_text_get_props (GL_LABEL_TEXT(object),
				 &font_family, &font_size,
				 &font_weight, &font_italic_flag,
				 &color, &just);
	buffer = gl_label_text_get_buffer(GL_LABEL_TEXT(object));
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
	line = g_strsplit (text, "\n", -1);

	/* remove previous items from group. */
	for (li = view_text->private->item_list; li != NULL; li = li->next) {
		gl_debug (DEBUG_VIEW, "in loop");
		gtk_object_destroy (GTK_OBJECT (li->data));
	}
	gl_debug (DEBUG_VIEW, "1");
	g_list_free (view_text->private->item_list);
	view_text->private->item_list = NULL;
	gl_debug (DEBUG_VIEW, "2");

	/* get Gnome Font */
	font = gnome_font_find_closest_from_weight_slant (font_family,
							  font_weight,
							  font_italic_flag,
							  font_size);
	art_affine_identity (affine);

	/* render to group, one item per line. */
	for (i = 0; line[i] != NULL; i++) {

		glyphlist = gnome_glyphlist_from_text_dumb (font, color,
							    0.0, 0.0,
							    line[i]);

		gnome_glyphlist_bbox (glyphlist, affine, 0, &bbox);
		w = bbox.x1;

		switch (just) {
		case GTK_JUSTIFY_LEFT:
			x_offset = GL_LABEL_TEXT_MARGIN;
			break;
		case GTK_JUSTIFY_CENTER:
			x_offset = (object_w - GL_LABEL_TEXT_MARGIN - w) / 2.0;
			break;
		case GTK_JUSTIFY_RIGHT:
			x_offset = object_w - GL_LABEL_TEXT_MARGIN - w;
			break;
		default:
			x_offset = 0.0;
			break;	/* shouldn't happen */
		}

		y_offset = GL_LABEL_TEXT_MARGIN +
			(i + 1) * font_size + gnome_font_get_descender (font);

		item = gl_view_object_item_new (GL_VIEW_OBJECT(view_text),
						gl_canvas_hacktext_get_type (),
						"x", x_offset,
						"y", y_offset,
						"glyphlist", glyphlist, NULL);
		view_text->private->item_list =
			g_list_prepend (view_text->private->item_list, item);

	}

	/* clean up */
	g_strfreev (line);
	g_free (text);

	gl_debug (DEBUG_VIEW, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Draw cursor to item (group).                                   */
/*--------------------------------------------------------------------------*/
static void
draw_cursor (glViewText *view_text)
{
	glLabelObject     *object;
	GnomeCanvasItem   *item;
	GtkTextBuffer     *buffer;
	GtkTextIter        start, end;
	gchar             *text;
	gchar             *font_family;
	GnomeFontWeight    font_weight;
	gboolean           font_italic_flag;
	gdouble            font_size;
	guint              color;
	GtkJustification   just;
	GnomeFont         *font;
	GnomeGlyphList    *glyphlist;
	ArtDRect           bbox;
	gdouble            affine[6];
	gdouble            x_offset, w, object_w, object_h;
	gint               i;
	gchar            **line;
	GList             *li;
	GtkTextMark       *cursor_mark, *bound_mark;
	GtkTextIter        cursor_iter, bound_iter;
	gint               cursor_line, cursor_char, bound_line, bound_char;
	gboolean           selection_flag;

	gl_debug (DEBUG_VIEW, "START");

	/* Query label object and properties */
	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_text));
	gl_label_object_get_size (object, &object_w, &object_h);
	gl_label_text_get_props (GL_LABEL_TEXT(object),
				 &font_family, &font_size,
				 &font_weight, &font_italic_flag,
				 &color, &just);
	buffer = gl_label_text_get_buffer(GL_LABEL_TEXT(object));
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
	line = g_strsplit (text, "\n", -1);

	/* Get cursor and selection information. */
	cursor_mark = gtk_text_buffer_get_insert (buffer);
	bound_mark  = gtk_text_buffer_get_selection_bound (buffer);
	gtk_text_buffer_get_iter_at_mark (buffer, &cursor_iter, cursor_mark);
	gtk_text_buffer_get_iter_at_mark (buffer, &bound_iter, bound_mark);
	cursor_line = gtk_text_iter_get_line (&cursor_iter);
	cursor_char = gtk_text_iter_get_visible_line_index (&cursor_iter);
	bound_line = gtk_text_iter_get_line (&bound_iter);
	bound_char = gtk_text_iter_get_visible_line_index (&bound_iter);
	selection_flag = !gtk_text_iter_equal (&cursor_iter, &bound_iter);

	/* get Gnome Font */
	font = gnome_font_find_closest_from_weight_slant (font_family,
							  font_weight,
							  font_italic_flag,
							  font_size);
	art_affine_identity (affine);

	for (i = 0; line[i] != NULL; i++) {

		if ( i == cursor_line ) {
			GnomeCanvasPoints *points;
			
			glyphlist = gnome_glyphlist_from_text_dumb (font, color,
								    0.0, 0.0,
								    line[i]);

			gnome_glyphlist_bbox (glyphlist, affine, 0, &bbox);
			gnome_glyphlist_unref (glyphlist);
			w = bbox.x1;

			switch (just) {
			case GTK_JUSTIFY_LEFT:
				x_offset = GL_LABEL_TEXT_MARGIN;
				break;
			case GTK_JUSTIFY_CENTER:
				x_offset = (object_w - GL_LABEL_TEXT_MARGIN - w) / 2.0;
				break;
			case GTK_JUSTIFY_RIGHT:
				x_offset = object_w - GL_LABEL_TEXT_MARGIN - w;
				break;
			default:
				x_offset = 0.0;
				break;	/* shouldn't happen */
			}

			glyphlist = gnome_glyphlist_from_text_sized_dumb (font, color,
									  0.0, 0.0,
									  line[i],
									  cursor_char);
			gnome_glyphlist_bbox (glyphlist, affine, 0, &bbox);
			gnome_glyphlist_unref (glyphlist);
			x_offset += bbox.x1;

			points = gnome_canvas_points_new (2);
			points->coords[0] = x_offset;
			points->coords[1] = GL_LABEL_TEXT_MARGIN + i*font_size;
			points->coords[2] = x_offset;
			points->coords[3] = GL_LABEL_TEXT_MARGIN + (i+1)*font_size;
			
			if (view_text->private->cursor) {
				gtk_object_destroy (GTK_OBJECT (view_text->private->cursor));
			}
			view_text->private->cursor =
				gl_view_object_item_new (GL_VIEW_OBJECT(view_text),
							 gnome_canvas_line_get_type (),
							 "points", points,
							 "fill_color_rgba", CURSOR_COLOR,
							 "width_pixels", 2,
							 NULL);
			gnome_canvas_points_free (points);

			if ( !view_text->private->cursor_visible ) {
				gnome_canvas_item_hide (view_text->private->cursor);
			}

		}

	}

	/* clean up */
	g_strfreev (line);
	g_free (text);

	gl_debug (DEBUG_VIEW, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Text buffer "mark-set" callback.  Tracks cursor position.      */
/*--------------------------------------------------------------------------*/
static void
mark_set_cb (GtkTextBuffer   *textbuffer,
	     GtkTextIter     *iter,
	     GtkTextMark     *mark,
	     glViewText      *view_text)
{
	const gchar *mark_name;

	mark_name = gtk_text_mark_get_name (mark);

	if ( mark_name && !strcmp (mark_name, "insert") ) {
		draw_cursor (view_text);
	}
}



/*--------------------------------------------------------------------------*/
/* PRIVATE.  Start the cursor blinking.                                     */
/*--------------------------------------------------------------------------*/
static void
blink_start (glViewText *view_text)
{
	if ( !view_text->private->cursor_visible ) return;

	view_text->private->cursor_state = TRUE;
	gnome_canvas_item_show (view_text->private->cursor);
	view_text->private->cursor_timeout =
		gtk_timeout_add (CURSOR_ON_TIME, (GtkFunction)blink_cb, view_text);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Stop the cursor blinking.                                      */
/*--------------------------------------------------------------------------*/
static void
blink_stop (glViewText *view_text)
{
	if ( view_text->private->cursor_timeout ) {
		gtk_timeout_remove (view_text->private->cursor_timeout);
		view_text->private->cursor_timeout = 0;
	}

	gnome_canvas_item_hide (view_text->private->cursor);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Blink cursor timeout callback.                                 */
/*--------------------------------------------------------------------------*/
static gboolean
blink_cb (glViewText *view_text)
{
	if ( view_text->private->cursor_visible ) {

		view_text->private->cursor_state =
			!view_text->private->cursor_state;


		if ( view_text->private->cursor_state ) {
			gnome_canvas_item_show (view_text->private->cursor);
			view_text->private->cursor_timeout =
				gtk_timeout_add (CURSOR_ON_TIME,
						 (GtkFunction)blink_cb, view_text);
		} else {
			gnome_canvas_item_hide (view_text->private->cursor);
			view_text->private->cursor_timeout =
				gtk_timeout_add (CURSOR_OFF_TIME,
						 (GtkFunction)blink_cb, view_text);
		}

	}

	return FALSE;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Item event callback.                                           */
/*--------------------------------------------------------------------------*/
static gint
item_event_cb (GnomeCanvasItem *item,
	       GdkEvent        *event,
	       glViewObject    *view_object)
{
	gl_debug (DEBUG_VIEW, "");
}

