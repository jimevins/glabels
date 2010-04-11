/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_text.c:  GLabels label text object view
 *
 *  Copyright (C) 2001-2007  Jim Evins <evins@snaught.com>.
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

#include "view-text.h"

#include <glib/gi18n.h>
#include <glib/gmem.h>

#include "color.h"
#include "object-editor.h"
#include "stock.h"

#include "pixmaps/cursor_text.xbm"
#include "pixmaps/cursor_text_mask.xbm"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glViewTextPrivate {
        int place_holder; /* Place holder for future private data. */
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void       gl_view_text_finalize             (GObject          *object);

static GtkWidget *construct_properties_editor       (glViewObject     *view_object);

static void       update_object_from_editor_cb      (glObjectEditor   *editor,
						     glLabelObject    *object);

static void       update_object_from_editor_size_cb (glObjectEditor   *editor,
                                                     glLabelObject    *object);

static void       update_editor_from_object_cb      (glLabelObject    *object,
						     glObjectEditor   *editor);

static void       update_editor_from_move_cb        (glLabelObject    *object,
						     gdouble           dx,
						     gdouble           dy,
						     glObjectEditor   *editor);

static gboolean   object_at                         (glViewObject     *view_object,
                                                     cairo_t          *cr,
                                                     gdouble           x,
                                                     gdouble           y);




/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glViewText, gl_view_text, GL_TYPE_VIEW_OBJECT);


static void
gl_view_text_class_init (glViewTextClass *class)
{
	GObjectClass      *object_class      = G_OBJECT_CLASS (class);
	glViewObjectClass *view_object_class = GL_VIEW_OBJECT_CLASS (class);

	gl_debug (DEBUG_VIEW, "START");

	gl_view_text_parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_view_text_finalize;

	view_object_class->construct_editor = construct_properties_editor;
	view_object_class->object_at        = object_at;

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_text_init (glViewText *view_text)
{
	gl_debug (DEBUG_VIEW, "START");

	view_text->priv = g_new0 (glViewTextPrivate, 1);

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_text_finalize (GObject *object)
{
        glViewText *view_text = GL_VIEW_TEXT (object);

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (object && GL_IS_VIEW_TEXT (object));

        g_free (view_text->priv);

	G_OBJECT_CLASS (gl_view_text_parent_class)->finalize (object);

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

	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (object && GL_IS_LABEL_TEXT (object), NULL);
	g_return_val_if_fail (view && GL_IS_VIEW (view), NULL);
	
	view_text = g_object_new (gl_view_text_get_type(), NULL);

	gl_view_object_set_object (GL_VIEW_OBJECT(view_text),
				   GL_LABEL_OBJECT(object),
				   GL_VIEW_OBJECT_HANDLES_BOX);
	gl_view_object_set_view (GL_VIEW_OBJECT(view_text), view);

	gl_debug (DEBUG_VIEW, "END");

	return GL_VIEW_OBJECT (view_text);
}

/*****************************************************************************/
/* Create a properties dialog for a text object.                             */
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
                                       object->parent,
				       GL_OBJECT_EDITOR_SHADOW_PAGE,
				       GL_OBJECT_EDITOR_POSITION_PAGE,
				       GL_OBJECT_EDITOR_SIZE_PAGE,
				       GL_OBJECT_EDITOR_TEXT_PAGE,
				       GL_OBJECT_EDITOR_EDIT_PAGE,
				       0);

	buffer = gl_label_text_get_buffer (GL_LABEL_TEXT(object));
	gl_object_editor_set_text_buffer (GL_OBJECT_EDITOR(editor), buffer);
	
	/* Update */
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

	gl_debug (DEBUG_VIEW, "END");

	return editor;
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
	PangoWeight        font_weight;
	gboolean           font_italic_flag;
	glColorNode       *color_node;
	PangoAlignment     align;
	gdouble            text_line_spacing;
	gboolean           auto_shrink;
	gboolean           shadow_state;
	gdouble            shadow_x, shadow_y;
	glColorNode	  *shadow_color_node;
	gdouble            shadow_opacity;

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
	align = gl_object_editor_get_text_alignment (editor);
	text_line_spacing = gl_object_editor_get_text_line_spacing (editor);
	auto_shrink = gl_object_editor_get_text_auto_shrink (editor);

	gl_label_object_set_position (object, x, y);
	gl_label_object_set_font_family (object, font_family);
	gl_label_object_set_font_size (object, font_size);
	gl_label_object_set_font_weight (object, font_weight);
	gl_label_object_set_font_italic_flag (object, font_italic_flag);
	gl_label_object_set_text_color (object, color_node);
	gl_label_object_set_text_alignment (object, align);
	gl_label_object_set_text_line_spacing (object, text_line_spacing);
	gl_label_text_set_auto_shrink (GL_LABEL_TEXT (object), auto_shrink);

	gl_color_node_free (&color_node);
	g_free (font_family);

	shadow_state = gl_object_editor_get_shadow_state (editor);
	gl_label_object_set_shadow_state (object, shadow_state);

	gl_object_editor_get_shadow_offset (editor, &shadow_x, &shadow_y);
	gl_label_object_set_shadow_offset (object, shadow_x, shadow_y);

	shadow_color_node = gl_object_editor_get_shadow_color (editor);
	gl_label_object_set_shadow_color (object, shadow_color_node);
	gl_color_node_free (&shadow_color_node);

	shadow_opacity = gl_object_editor_get_shadow_opacity (editor);
	gl_label_object_set_shadow_opacity (object, shadow_opacity);

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
	PangoWeight        font_weight;
	gboolean           font_italic_flag;
	glColorNode       *color_node;
	PangoAlignment     align;
	gdouble            text_line_spacing;
	gboolean           auto_shrink;
	gboolean           shadow_state;
	gdouble            shadow_x, shadow_y;
	glColorNode	  *shadow_color_node;
	gdouble            shadow_opacity;
	glMerge		  *merge;

	gl_debug (DEBUG_VIEW, "START");

	gl_label_object_get_size (object, &w, &h);
	gl_object_editor_set_size (editor, w, h);
	merge = gl_label_get_merge (GL_LABEL(object->parent));

	font_family      = gl_label_object_get_font_family (object);
	font_size        = gl_label_object_get_font_size (object);
	font_weight      = gl_label_object_get_font_weight (object);
	font_italic_flag = gl_label_object_get_font_italic_flag (object);
	color_node       = gl_label_object_get_text_color (object);
	align             = gl_label_object_get_text_alignment (object);
	text_line_spacing = gl_label_object_get_text_line_spacing (object);
	auto_shrink      = gl_label_text_get_auto_shrink (GL_LABEL_TEXT (object));

	gl_object_editor_set_font_family (editor, font_family);
	gl_object_editor_set_font_size (editor, font_size);
	gl_object_editor_set_font_weight (editor, font_weight);
	gl_object_editor_set_font_italic_flag (editor, font_italic_flag);
	gl_object_editor_set_text_color (editor, (merge != NULL), color_node);
	gl_object_editor_set_text_alignment (editor, align);
	gl_object_editor_set_text_line_spacing (editor, text_line_spacing);
	gl_object_editor_set_text_auto_shrink (editor, auto_shrink);

	gl_color_node_free (&color_node);
	g_free (font_family);

	shadow_state = gl_label_object_get_shadow_state (object);
	gl_object_editor_set_shadow_state (editor, shadow_state);

	gl_label_object_get_shadow_offset (object, &shadow_x, &shadow_y);
	gl_object_editor_set_shadow_offset (editor, shadow_x, shadow_y);

	shadow_color_node = gl_label_object_get_shadow_color (object);
	gl_object_editor_set_shadow_color (editor, (merge != NULL), shadow_color_node);
	gl_color_node_free (&shadow_color_node);

	shadow_opacity = gl_label_object_get_shadow_opacity (object);
	gl_object_editor_set_shadow_opacity (editor, shadow_opacity);

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


/*****************************************************************************/
/* Is object at (x,y)?                                                       */
/*****************************************************************************/
static gboolean
object_at (glViewObject  *view_object,
           cairo_t       *cr,
           gdouble        x,
           gdouble        y)
{
	glLabelObject    *object;
        gdouble           w, h;

        object = gl_view_object_get_object (view_object);

        gl_label_object_get_size (object, &w, &h);

        cairo_rectangle (cr, 0.0, 0.0, w, h);

        if (cairo_in_fill (cr, x, y))
        {
                return TRUE;
        }

        return FALSE;
}


/*****************************************************************************/
/* Return apropos cursor for create object mode.                             */
/*****************************************************************************/
GdkCursor *
gl_view_text_get_create_cursor (void)
{
	GdkCursor       *cursor = NULL;
	GdkPixmap       *pixmap_data, *pixmap_mask;
	GdkColor         fg = { 0, 0, 0, 0 };
	GdkColor         bg = { 0, 65535, 65535, 65535 };

	gl_debug (DEBUG_VIEW, "START");

        pixmap_data = gdk_bitmap_create_from_data (NULL,
                                                   (gchar *)cursor_text_bits,
                                                   cursor_text_width,
                                                   cursor_text_height);
        pixmap_mask = gdk_bitmap_create_from_data (NULL,
                                                   (gchar *)cursor_text_mask_bits,
                                                   cursor_text_mask_width,
                                                   cursor_text_mask_height);
        cursor = gdk_cursor_new_from_pixmap (pixmap_data, pixmap_mask, &fg,
                                             &bg, cursor_text_x_hot,
                                             cursor_text_y_hot);

	gl_debug (DEBUG_VIEW, "END");

	return cursor;
}

/*****************************************************************************/
/* Object creation handler: button press event.                              */
/*****************************************************************************/
void
gl_view_text_create_button_press_event   (glView *view,
                                          gdouble x,
                                          gdouble y)
{
	GObject             *object;
	GList               *lines;
	gchar               *family;
	glColorNode         *color_node;

        gl_view_unselect_all (view);

        object = gl_label_text_new (view->label);
        color_node = gl_color_node_new_default ();
        gl_label_object_set_position (GL_LABEL_OBJECT(object), x, y);
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
        gl_color_node_free (&color_node);

        view->create_object = GL_LABEL_OBJECT (object);
        view->create_x0 = x;
        view->create_y0 = y;
}

/*****************************************************************************/
/* Object creation handler: motion event.                                    */
/*****************************************************************************/
void
gl_view_text_create_motion_event     (glView *view,
                                      gdouble x,
                                      gdouble y)
{
        gl_label_object_set_position (GL_LABEL_OBJECT(view->create_object), x, y);
}

/*****************************************************************************/
/* Object creation handler: button relesase event.                           */
/*****************************************************************************/
void
gl_view_text_create_button_release_event (glView *view,
                                          gdouble x,
                                          gdouble y)
{
	glColorNode         *color_node;

        color_node = gl_color_node_new_default ();
        gl_label_object_set_position (GL_LABEL_OBJECT(view->create_object), x, y);
        color_node->color = gl_view_get_default_text_color(view);
        gl_label_object_set_text_color (GL_LABEL_OBJECT(view->create_object), color_node);
        gl_color_node_free (&color_node);
}

