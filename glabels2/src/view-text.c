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
#include <config.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <libgnomeprint/gnome-glyphlist.h>

#include "view-text.h"
#include "canvas-hacktext.h"
#include "view-highlight.h"

#include "color.h"
#include "object-editor.h"
#include "stock.h"

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

};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static glViewObjectClass *parent_class = NULL;


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void       gl_view_text_class_init            (glViewTextClass  *klass);
static void       gl_view_text_instance_init         (glViewText       *view_text);
static void       gl_view_text_finalize              (GObject          *object);

static GtkWidget *construct_properties_editor        (glViewObject     *view_object);

static void       update_canvas_item_from_object_cb  (glLabelObject    *object,
						      glViewText       *view_text);

static void       update_object_from_editor_cb       (glObjectEditor   *editor,
						      glLabelObject    *object);

static void       update_object_from_editor_size_cb  (glObjectEditor   *editor,
						      glLabelObject    *object);

static void       update_editor_from_object_cb       (glLabelObject    *object,
						      glObjectEditor   *editor);

static void       update_editor_from_move_cb         (glLabelObject    *object,
						      gdouble           dx,
						      gdouble           dy,
						      glObjectEditor   *editor);

static void       update_editor_from_label_cb        (glLabel          *label,
						      glObjectEditor   *editor);

static void       draw_hacktext                      (glViewText       *view_text);

static void       draw_cursor                        (glViewText       *view_text);

static void       mark_set_cb                        (GtkTextBuffer    *textbuffer,
						      GtkTextIter      *iter,
						      GtkTextMark      *mark,
						      glViewText       *view_text);

static void       blink_start                        (glViewText       *view_text);
static void       blink_stop                         (glViewText       *view_text);
static gboolean   blink_cb                           (glViewText       *view_text);

static gint       item_event_cb                      (GnomeCanvasItem  *item,
						      GdkEvent         *event,
						      glViewObject     *view_object);



/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_view_text_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (glViewTextClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_view_text_class_init,
			NULL,
			NULL,
			sizeof (glViewText),
			0,
			(GInstanceInitFunc) gl_view_text_instance_init,
			NULL
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

	view_object_class->construct_editor = construct_properties_editor;

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
/* NEW text object view.                                                     */
/*****************************************************************************/
glViewObject *
gl_view_text_new (glLabelText *object,
		  glView      *view)
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
			  G_CALLBACK (update_canvas_item_from_object_cb), view_text);

	g_signal_connect (G_OBJECT (group), "event",
			  G_CALLBACK (item_event_cb), view_text);

	buffer = gl_label_text_get_buffer (object);
	g_signal_connect (G_OBJECT (buffer), "mark-set",
			  G_CALLBACK (mark_set_cb), view_text);

	gl_debug (DEBUG_VIEW, "END");

	return GL_VIEW_OBJECT (view_text);
}

/*****************************************************************************/
/* Create a properties editor for a text object.                             */
/*****************************************************************************/
static GtkWidget *
construct_properties_editor (glViewObject *view_object)
{
	GtkWidget          *editor;
	glViewText          *view_text = (glViewText *)view_object;
	glLabelObject      *object;
	GtkTextBuffer      *buffer;

	gl_debug (DEBUG_VIEW, "START");

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_text));

	/* Build editor. */
	editor = gl_object_editor_new (GL_STOCK_TEXT, _("Text object properties"),
				       GL_OBJECT_EDITOR_POSITION_PAGE,
				       GL_OBJECT_EDITOR_SIZE_PAGE,
				       GL_OBJECT_EDITOR_TEXT_PAGE,
				       GL_OBJECT_EDITOR_EDIT_PAGE,
				       0);

	buffer = gl_label_text_get_buffer (GL_LABEL_TEXT(object));
	gl_object_editor_set_text_buffer (GL_OBJECT_EDITOR(editor), buffer);
	
	/* Update */
	update_editor_from_label_cb (object->parent, GL_OBJECT_EDITOR(editor));
	update_editor_from_object_cb (object, GL_OBJECT_EDITOR(editor));
	update_editor_from_move_cb (object, 0, 0, GL_OBJECT_EDITOR(editor));

	/* Connect signals. */
	g_signal_connect (G_OBJECT (editor), "changed",
			  G_CALLBACK(update_object_from_editor_cb), object);
	g_signal_connect (G_OBJECT (editor), "size_changed",
			  G_CALLBACK(update_object_from_editor_size_cb), object);
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
				   glViewText    *view_text)
{
	gl_debug (DEBUG_VIEW, "START");

	/* Adjust appearance of analogous canvas item. */
	draw_hacktext (view_text);
	draw_cursor (view_text);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  editor "changed" callback.                                      */
/*---------------------------------------------------------------------------*/
static void
update_object_from_editor_cb (glObjectEditor *editor,
			      glLabelObject  *object)
{
	gdouble            x, y;
	gchar             *font_family;
	gdouble            font_size;
	GnomeFontWeight    font_weight;
	gboolean           font_italic_flag;
	glColorNode       *color_node;
	GtkJustification   just;
	gdouble            text_line_spacing;
	gboolean           auto_shrink;

	gl_debug (DEBUG_VIEW, "START");

	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_editor_from_object_cb,
					 editor);
	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_editor_from_move_cb,
					 editor);

	gl_object_editor_get_position (editor, &x, &y);
	font_family = gl_object_editor_get_font_family (editor);
	font_size = gl_object_editor_get_font_size (editor);
	font_weight = gl_object_editor_get_font_weight (editor);
	font_italic_flag = gl_object_editor_get_font_italic_flag (editor);
	color_node = gl_object_editor_get_text_color (editor);
	just = gl_object_editor_get_text_alignment (editor);
	text_line_spacing = gl_object_editor_get_text_line_spacing (editor);
	auto_shrink = gl_object_editor_get_text_auto_shrink (editor);

	gl_label_object_set_position (object, x, y);
	gl_label_object_set_font_family (object, font_family);
	gl_label_object_set_font_size (object, font_size);
	gl_label_object_set_font_weight (object, font_weight);
	gl_label_object_set_font_italic_flag (object, font_italic_flag);
	gl_label_object_set_text_color (object, color_node);
	gl_label_object_set_text_alignment (object, just);
	gl_label_object_set_text_line_spacing (object, text_line_spacing);
	gl_label_text_set_auto_shrink (GL_LABEL_TEXT (object), auto_shrink);

	gl_color_node_free (&color_node);
	g_free (font_family);

	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_editor_from_object_cb,
					   editor);
	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_editor_from_move_cb,
					   editor);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  editor "changed" callback.                                      */
/*---------------------------------------------------------------------------*/
static void
update_object_from_editor_size_cb (glObjectEditor *editor,
				   glLabelObject  *object)
{
	gdouble            w, h;

	gl_debug (DEBUG_VIEW, "START");

	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_editor_from_object_cb,
					 editor);
	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_editor_from_move_cb,
					 editor);

	gl_object_editor_get_size (editor, &w, &h);

	gl_label_object_set_size (object, w, h);

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
	gchar             *font_family;
	gdouble            font_size;
	GnomeFontWeight    font_weight;
	gboolean           font_italic_flag;
	glColorNode       *color_node;
	GtkJustification   just;
	gdouble            text_line_spacing;
	gboolean           auto_shrink;
	glMerge			   *merge;

	gl_debug (DEBUG_VIEW, "START");

	gl_label_object_get_size (object, &w, &h);
	gl_object_editor_set_size (editor, w, h);
	merge = gl_label_get_merge (GL_LABEL(object->parent));

	font_family      = gl_label_object_get_font_family (object);
	font_size        = gl_label_object_get_font_size (object);
	font_weight      = gl_label_object_get_font_weight (object);
	font_italic_flag = gl_label_object_get_font_italic_flag (object);
	color_node       = gl_label_object_get_text_color (object);
	just             = gl_label_object_get_text_alignment (object);
	text_line_spacing = gl_label_object_get_text_line_spacing (object);
	auto_shrink      = gl_label_text_get_auto_shrink (GL_LABEL_TEXT (object));

	gl_object_editor_set_font_family (editor, font_family);
	gl_object_editor_set_font_size (editor, font_size);
	gl_object_editor_set_font_weight (editor, font_weight);
	gl_object_editor_set_font_italic_flag (editor, font_italic_flag);
	gl_object_editor_set_text_color (editor, (merge != NULL), color_node);
	gl_object_editor_set_text_alignment (editor, just);
	gl_object_editor_set_text_line_spacing (editor, text_line_spacing);
	gl_object_editor_set_text_auto_shrink (editor, auto_shrink);

	gl_color_node_free (&color_node);
	g_free (font_family);

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
	static gdouble       x0, y0;
	static gboolean      dragging = FALSE;
	static glViewObject *view_text;
	static GObject      *object;
	gdouble              x, y;
	GList               *lines;
	gchar               *family;
	glColorNode         *color_node;

	gl_debug (DEBUG_VIEW, "");

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		gl_debug (DEBUG_VIEW, "BUTTON_PRESS");
		switch (event->button.button) {
		case 1:
			color_node = gl_color_node_new_default ();
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
			gl_label_object_set_font_family (GL_LABEL_OBJECT(object), family);
			gl_label_object_set_font_size (GL_LABEL_OBJECT(object),
						       gl_view_get_default_font_size (view));
			gl_label_object_set_font_weight (GL_LABEL_OBJECT(object),
							 gl_view_get_default_font_weight (view));
			gl_label_object_set_font_italic_flag (GL_LABEL_OBJECT(object),
							      gl_view_get_default_font_italic_flag (view));
								  
			color_node->color = gl_color_set_opacity (gl_view_get_default_text_color (view), 0.5);
			gl_label_object_set_text_color (GL_LABEL_OBJECT(object),
							color_node);
			gl_label_object_set_text_alignment (GL_LABEL_OBJECT(object),
							    gl_view_get_default_text_alignment (view));
			gl_label_object_set_text_line_spacing (GL_LABEL_OBJECT(object), gl_view_get_default_text_line_spacing (view));
						       
			g_free (family);
			lines = gl_text_node_lines_new_from_text (_("Text"));
			gl_label_text_set_lines (GL_LABEL_TEXT(object), lines);
			view_text = gl_view_text_new (GL_LABEL_TEXT(object),
						      view);
			x0 = x;
			y0 = y;
			gl_color_node_free (&color_node);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		gl_debug (DEBUG_VIEW, "BUTTON_RELEASE");
		switch (event->button.button) {
		case 1:
			color_node = gl_color_node_new_default ();
			dragging = FALSE;
			gnome_canvas_item_ungrab (canvas->root, event->button.time);
			gnome_canvas_window_to_world (canvas,
						      event->button.x,
						      event->button.y, &x, &y);
			gl_label_object_set_position (GL_LABEL_OBJECT(object),
						      x, y);
			family = gl_view_get_default_font_family (view);
			gl_label_object_set_font_family (GL_LABEL_OBJECT(object), family);
			gl_label_object_set_font_size (GL_LABEL_OBJECT(object),
						       gl_view_get_default_font_size (view));
			gl_label_object_set_font_weight (GL_LABEL_OBJECT(object),
							 gl_view_get_default_font_weight (view));
			gl_label_object_set_font_italic_flag (GL_LABEL_OBJECT(object),
							      gl_view_get_default_font_italic_flag (view));
			
			color_node->color = gl_view_get_default_text_color (view);
			gl_label_object_set_text_color (GL_LABEL_OBJECT(object),
							color_node);
			gl_label_object_set_text_alignment (GL_LABEL_OBJECT(object),
							    gl_view_get_default_text_alignment (view));
			gl_label_object_set_text_line_spacing (GL_LABEL_OBJECT(object), gl_view_get_default_text_line_spacing (view));

			g_free (family);
			gl_view_unselect_all (view);
			gl_view_object_select (GL_VIEW_OBJECT(view_text));
			gl_view_arrow_mode (view);
			gl_color_node_free (&color_node);
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
	gdouble            text_line_spacing;
	glColorNode       *color_node;
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
	font_family = gl_label_object_get_font_family (object);
	font_size = gl_label_object_get_font_size (object);
	text_line_spacing = gl_label_object_get_text_line_spacing (object);
	font_weight = gl_label_object_get_font_weight (object);
	font_italic_flag = gl_label_object_get_font_italic_flag (object);
	color_node = gl_label_object_get_text_color (object);
	if (color_node->field_flag)
	{
		color_node->color = GL_COLOR_MERGE_DEFAULT;
	}
	just = gl_label_object_get_text_alignment (object);
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

		glyphlist = gnome_glyphlist_from_text_dumb (font, color_node->color,
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

		/* Work out the y position to the BOTTOM of the first line */
		y_offset = GL_LABEL_TEXT_MARGIN +
			   + gnome_font_get_descender (font)
	       		   + (i + 1) * font_size * text_line_spacing;

		/* Remove any text line spacing from the first row. */
		y_offset -= font_size * (text_line_spacing - 1);

		item = gl_view_object_item_new (GL_VIEW_OBJECT(view_text),
						gl_canvas_hacktext_get_type (),
						"x", x_offset,
						"y", y_offset,
						"glyphlist", glyphlist, NULL);

		gnome_glyphlist_unref (glyphlist);

		view_text->private->item_list =
			g_list_prepend (view_text->private->item_list, item);

	}

	/* clean up */
	gl_color_node_free (&color_node);
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
	glColorNode       *color_node;
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
	font_family = gl_label_object_get_font_family (object);
	font_size = gl_label_object_get_font_size (object);
	font_weight = gl_label_object_get_font_weight (object);
	font_italic_flag = gl_label_object_get_font_italic_flag (object);
	color_node = gl_label_object_get_text_color (object);
	just = gl_label_object_get_text_alignment (object);
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
			
			glyphlist = gnome_glyphlist_from_text_dumb (font, color_node->color,
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

			glyphlist = gnome_glyphlist_from_text_sized_dumb (font, color_node->color,
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
	gl_color_node_free (&color_node);
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
