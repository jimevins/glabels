/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_barcode.c:  GLabels label barcode object view
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

#include "view-barcode.h"

#include <glib/gi18n.h>
#include <glib/gmem.h>

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
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void       gl_view_barcode_finalize          (GObject          *object);

static GtkWidget *construct_properties_editor       (glViewObject     *view_object);

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

static gboolean   object_at                         (glViewObject     *view_object,
                                                     cairo_t          *cr,
                                                     gdouble           x,
                                                     gdouble           y);




/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glViewBarcode, gl_view_barcode, GL_TYPE_VIEW_OBJECT);


static void
gl_view_barcode_class_init (glViewBarcodeClass *class)
{
	GObjectClass      *object_class      = G_OBJECT_CLASS (class);
	glViewObjectClass *view_object_class = GL_VIEW_OBJECT_CLASS (class);

	gl_debug (DEBUG_VIEW, "START");

	gl_view_barcode_parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_view_barcode_finalize;

	view_object_class->construct_editor = construct_properties_editor;
	view_object_class->object_at        = object_at;

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_barcode_init (glViewBarcode *view_barcode)
{
	gl_debug (DEBUG_VIEW, "START");

	view_barcode->priv = g_new0 (glViewBarcodePrivate, 1);

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_barcode_finalize (GObject *object)
{
        glViewBarcode *view_barcode = GL_VIEW_BARCODE (object);

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (object && GL_IS_VIEW_BARCODE (object));

        g_free (view_barcode->priv);

	G_OBJECT_CLASS (gl_view_barcode_parent_class)->finalize (object);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* NEW barcode object view.                                                  */
/*****************************************************************************/
glViewObject *
gl_view_barcode_new (glLabelBarcode *object,
                     glView         *view)
{
	glViewBarcode         *view_barcode;

	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (object && GL_IS_LABEL_BARCODE (object), NULL);
	g_return_val_if_fail (view && GL_IS_VIEW (view), NULL);
	
	view_barcode = g_object_new (gl_view_barcode_get_type(), NULL);

	gl_view_object_set_object (GL_VIEW_OBJECT(view_barcode),
				   GL_LABEL_OBJECT(object),
				   GL_VIEW_OBJECT_HANDLES_BOX);
	gl_view_object_set_view (GL_VIEW_OBJECT(view_barcode), view);

	gl_debug (DEBUG_VIEW, "END");

	return GL_VIEW_OBJECT (view_barcode);
}

/*****************************************************************************/
/* Create a properties dialog for a barcode object.                          */
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
	glColorNode       *color_node;
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
	color_node = gl_object_editor_get_bc_color (editor);
	gl_label_barcode_set_props (GL_LABEL_BARCODE(object),
				    id, text_flag, cs_flag, format_digits);
	gl_label_object_set_line_color (object, color_node);
	gl_color_node_free (&color_node);
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
	glColorNode       *color_node;
	glMerge           *merge;
	guint              format_digits;

	gl_debug (DEBUG_VIEW, "START");

	gl_label_object_get_size (object, &w, &h);
	gl_object_editor_set_size (editor, w, h);
	
	merge = gl_label_get_merge (GL_LABEL(object->parent));

	gl_label_barcode_get_props (GL_LABEL_BARCODE(object),
				    &id, &text_flag, &cs_flag, &format_digits);
	color_node = gl_label_object_get_line_color (object);
	gl_object_editor_set_bc_style (editor, id, text_flag, cs_flag, format_digits);
	gl_object_editor_set_bc_color (editor, (merge != NULL), color_node);
	gl_color_node_free (&color_node);
	g_free (id);

	text_node = gl_label_barcode_get_data (GL_LABEL_BARCODE(object));
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
	glMerge		   	   *merge;

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
gl_view_barcode_get_create_cursor (void)
{
	GdkCursor       *cursor = NULL;
	GdkPixmap       *pixmap_data, *pixmap_mask;
	GdkColor         fg = { 0, 0, 0, 0 };
	GdkColor         bg = { 0, 65535, 65535, 65535 };

	gl_debug (DEBUG_VIEW, "START");

        pixmap_data = gdk_bitmap_create_from_data (NULL,
                                                   (gchar *)cursor_barcode_bits,
                                                   cursor_barcode_width,
                                                   cursor_barcode_height);
        pixmap_mask = gdk_bitmap_create_from_data (NULL,
                                                   (gchar *)cursor_barcode_mask_bits,
                                                   cursor_barcode_mask_width,
                                                   cursor_barcode_mask_height);
        cursor = gdk_cursor_new_from_pixmap (pixmap_data, pixmap_mask, &fg,
                                             &bg, cursor_barcode_x_hot,
                                             cursor_barcode_y_hot);

	gl_debug (DEBUG_VIEW, "END");

	return cursor;
}

/*****************************************************************************/
/* Object creation handler: button press event.                              */
/*****************************************************************************/
void
gl_view_barcode_create_button_press_event   (glView *view,
                                             gdouble x,
                                             gdouble y)
{
	GObject             *object;
        glTextNode          *text_node;
	glColorNode         *line_color_node;
        glViewObject        *view_barcode;

        line_color_node = gl_color_node_new_default ();
		
        object = gl_label_barcode_new (view->label);
        gl_label_object_set_position (GL_LABEL_OBJECT(object), x, y);
        text_node = gl_text_node_new_from_text ("123456789");
        gl_label_barcode_set_data (GL_LABEL_BARCODE(object), text_node);
        line_color_node->color = gl_color_set_opacity (gl_view_get_default_line_color(view), 0.5);
        gl_label_object_set_line_color (GL_LABEL_OBJECT(object),
                                        line_color_node);

        gl_color_node_free (&line_color_node);

        gl_view_unselect_all (view);
        view_barcode = gl_view_barcode_new (GL_LABEL_BARCODE(object), view);
        gl_view_object_select (GL_VIEW_OBJECT(view_barcode));
			
        view->create_object = GL_LABEL_OBJECT (object);
        view->create_x0 = x;
        view->create_y0 = y;
}

/*****************************************************************************/
/* Object creation handler: motion event.                                    */
/*****************************************************************************/
void
gl_view_barcode_create_motion_event         (glView *view,
                                             gdouble x,
                                             gdouble y)
{
        gl_label_object_set_position (GL_LABEL_OBJECT(view->create_object), x, y);
}

/*****************************************************************************/
/* Object creation handler: button relesase event.                           */
/*****************************************************************************/
void
gl_view_barcode_create_button_release_event (glView *view,
                                             gdouble x,
                                             gdouble y)
{
	glColorNode         *line_color_node;

        line_color_node = gl_color_node_new_default ();
		
        gl_label_object_set_position (GL_LABEL_OBJECT(view->create_object), x, y);
        line_color_node->color = gl_view_get_default_line_color(view);
        gl_label_object_set_line_color (GL_LABEL_OBJECT(view->create_object), line_color_node);
        gl_color_node_free (&line_color_node);
}

