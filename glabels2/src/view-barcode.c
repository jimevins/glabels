/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view_barcode.c:  GLabels label barcode object widget
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

#include "view-barcode.h"
#include "canvas-hacktext.h"

#include "view-highlight.h"

#include "wdgt-bc-data.h"
#include "wdgt-bc-props.h"
#include "wdgt-bc-style.h"
#include "wdgt-position.h"
#include "color.h"
#include "prefs.h"

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

	GList     *item_list;

	/* Page 0 widgets */
	GtkWidget *bc_data;

	/* Page 1 widgets */
	GtkWidget *bc_props;
	GtkWidget *bc_style;

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

static void      gl_view_barcode_class_init    (glViewBarcodeClass *klass);
static void      gl_view_barcode_instance_init (glViewBarcode  *view_barcode);
static void      gl_view_barcode_finalize      (GObject        *object);

static void      update_view_barcode_cb        (glLabelObject  *object,
						glViewBarcode  *view_barcode);

static GtkWidget *construct_properties_dialog  (glViewObject   *view_object);

static void      response_cb                   (GtkDialog      *dialog,
						gint            response,
						glViewBarcode  *view_barcode);

static void      bc_data_changed_cb            (glWdgtBCData   *bc_data,
						glViewBarcode  *view_barcode);

static void      bc_props_changed_cb           (glWdgtBCProps  *bc_props,
						glViewBarcode  *view_barcode);

static void      bc_style_changed_cb           (glWdgtBCStyle  *bc_style,
						glViewBarcode  *view_barcode);

static void      position_changed_cb           (glWdgtPosition *position,
						glViewBarcode  *view_barcode);

static void      update_dialog_cb              (glLabelObject  *object,
						glViewBarcode  *view_barcode);

static void      update_dialog_from_move_cb    (glLabelObject  *object,
						gdouble         dx,
						gdouble         dy,
						glViewBarcode  *view_barcode);

static void      draw_barcode                  (glViewBarcode  *view_barcode);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_view_barcode_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (glViewBarcodeClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_view_barcode_class_init,
			NULL,
			NULL,
			sizeof (glViewBarcode),
			0,
			(GInstanceInitFunc) gl_view_barcode_instance_init,
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

	view_object_class->construct_dialog = construct_properties_dialog;

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_barcode_instance_init (glViewBarcode *view_barcode)
{
	gl_debug (DEBUG_VIEW, "START");

	view_barcode->private = g_new0 (glViewBarcodePrivate, 1);

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
	glViewBarcode      *view_barcode;
	GtkMenu            *menu;

	gl_debug (DEBUG_VIEW, "START");
	g_return_if_fail (object && GL_IS_LABEL_BARCODE (object));
	g_return_if_fail (view && GL_IS_VIEW (view));
	
	view_barcode = g_object_new (gl_view_barcode_get_type(), NULL);

	gl_view_object_set_view (GL_VIEW_OBJECT(view_barcode), view);
	gl_view_object_set_object (GL_VIEW_OBJECT(view_barcode),
				   GL_LABEL_OBJECT(object),
				   GL_VIEW_HIGHLIGHT_BOX_RESIZABLE);

	/* Create analogous canvas items. */
	draw_barcode (view_barcode);

	g_signal_connect (G_OBJECT (object), "changed",
			  G_CALLBACK (update_view_barcode_cb), view_barcode);

	gl_debug (DEBUG_VIEW, "END");

	return GL_VIEW_OBJECT (view_barcode);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "changed" callback.                                 */
/*---------------------------------------------------------------------------*/
static void
update_view_barcode_cb (glLabelObject *object,
			glViewBarcode *view_barcode)
{
	glView             *view;
	GnomeCanvasItem    *group;

	gl_debug (DEBUG_VIEW, "START");

	view = gl_view_object_get_view (GL_VIEW_OBJECT(view_barcode));

	/* Adjust appearance of analogous canvas items. */
	draw_barcode (view_barcode);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Create a properties dialog for a barcode object.                          */
/*****************************************************************************/
static GtkWidget *
construct_properties_dialog (glViewObject *view_object)
{
	glViewBarcode      *view_barcode = (glViewBarcode *)view_object;
	GtkWidget          *dialog, *wsection;
	glLabelObject      *object;
	gdouble            x, y, w, h, label_width, label_height;
	glTextNode         *text_node;
	glBarcodeStyle     style;
	gboolean           text_flag;
	gboolean           checksum_flag;
	guint              color;
	glMerge            *merge;
	GtkSizeGroup       *label_size_group;
	GtkWidget          *window;

	gl_debug (DEBUG_VIEW, "START");

	/* retrieve object and query parameters */
	object = gl_view_object_get_object (view_object);
	gl_label_object_get_position (GL_LABEL_OBJECT(object), &x, &y);
	text_node = gl_label_barcode_get_data(GL_LABEL_BARCODE(object));
	gl_label_barcode_get_props (GL_LABEL_BARCODE(object),
				    &style, &text_flag, &checksum_flag, &color);
	gl_label_get_size (GL_LABEL(object->parent),
			   &label_width, &label_height);
	merge = gl_label_get_merge (GL_LABEL(object->parent));

	/*-----------------------------------------------------------------*/
	/* Build dialog.                                                   */
	/*-----------------------------------------------------------------*/
	window = gtk_widget_get_toplevel (
		GTK_WIDGET(gl_view_object_get_view(view_object)));
	dialog = gl_hig_dialog_new_with_buttons ( _("Edit barcode object properties"),
						  GTK_WINDOW (window),
						  GTK_DIALOG_DESTROY_WITH_PARENT,
						  GTK_STOCK_CLOSE,
					                   GTK_RESPONSE_CLOSE,
						  NULL );
        gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK (response_cb), view_object);

	label_size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	/*---------------------------*/
	/* Data section              */
	/*---------------------------*/
	wsection = gl_hig_category_new (_("Data"));
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dialog), wsection);

	/* barcode data */
	view_barcode->private->bc_data = gl_wdgt_bc_data_new (merge);
	gl_wdgt_bc_data_set_label_size_group (GL_WDGT_BC_DATA(view_barcode->private->bc_data),
					      label_size_group);
	gl_wdgt_bc_data_set_data (GL_WDGT_BC_DATA(view_barcode->private->bc_data),
				  (merge != NULL),
				  text_node);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wsection),
				    view_barcode->private->bc_data);
	g_signal_connect ( G_OBJECT(view_barcode->private->bc_data),
			   "changed", G_CALLBACK (bc_data_changed_cb),
			   view_barcode);


	/*---------------------------*/
	/* Appearance section        */
	/*---------------------------*/
	wsection = gl_hig_category_new (_("Properties"));
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dialog), wsection);

	/* Barcode style widget */
	view_barcode->private->bc_style = gl_wdgt_bc_style_new ();
	gl_wdgt_bc_style_set_label_size_group (GL_WDGT_BC_STYLE(view_barcode->private->bc_style),
					       label_size_group);
	gl_wdgt_bc_style_set_params (GL_WDGT_BC_STYLE (view_barcode->private->bc_style),
				     style, text_flag, checksum_flag);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wsection),
				    view_barcode->private->bc_style);
	g_signal_connect (G_OBJECT (view_barcode->private->bc_style),
			  "changed", G_CALLBACK (bc_style_changed_cb),
			  view_barcode);

	/* barcode props entry */
	gl_debug (DEBUG_VIEW, "Creating props entry...");
	view_barcode->private->bc_props = gl_wdgt_bc_props_new ();
	gl_wdgt_bc_props_set_label_size_group (GL_WDGT_BC_PROPS(view_barcode->private->bc_props),
					       label_size_group);
	gl_wdgt_bc_props_set_params (GL_WDGT_BC_PROPS(view_barcode->private->bc_props),
				     color);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wsection),
				    view_barcode->private->bc_props);
	g_signal_connect ( G_OBJECT(view_barcode->private->bc_props),
			   "changed", G_CALLBACK (bc_props_changed_cb),
			   view_barcode);


	/*----------------------------*/
	/* Position section           */
	/*----------------------------*/
	wsection = gl_hig_category_new (_("Position"));
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dialog), wsection);

	/* ------ Position Frame ------ */
	view_barcode->private->position = gl_wdgt_position_new ();
	gl_wdgt_position_set_label_size_group (GL_WDGT_POSITION(view_barcode->private->position),
					       label_size_group);
	gl_wdgt_position_set_params (GL_WDGT_POSITION (view_barcode->private->position),
				     x, y,
				     label_width, label_height);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wsection),
				    view_barcode->private->position);
	g_signal_connect (G_OBJECT (view_barcode->private->position),
			  "changed",
			  G_CALLBACK(position_changed_cb), view_barcode);


	/*----------------------------*/
	/* Track object changes.      */
	/*----------------------------*/
	g_signal_connect (G_OBJECT (object), "changed",
			  G_CALLBACK (update_dialog_cb), view_barcode);
	g_signal_connect (G_OBJECT (object), "moved",
			  G_CALLBACK (update_dialog_from_move_cb),
			  view_barcode);

	gl_debug (DEBUG_VIEW, "END");

	return dialog;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Response" callback.                                            */
/*---------------------------------------------------------------------------*/
static void
response_cb (GtkDialog     *dialog,
	     gint          response,
	     glViewBarcode   *view_barcode)
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
		object = gl_view_object_get_object (GL_VIEW_OBJECT(view_barcode));

		g_signal_handlers_disconnect_by_func (object, update_dialog_cb,
						      view_barcode);
		g_signal_handlers_disconnect_by_func (object, update_dialog_from_move_cb,
						      view_barcode);
		break;
	default:
		g_print ("response = %d", response);
		g_assert_not_reached();
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  barcode data "changed" callback.                                */
/*---------------------------------------------------------------------------*/
static void
bc_data_changed_cb (glWdgtBCData        *bc_data,
		    glViewBarcode       *view_barcode)
{
	glLabelObject    *object;
	glTextNode       *text_node;

	gl_debug (DEBUG_VIEW, "START");

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_barcode));

	text_node = gl_wdgt_bc_data_get_data (bc_data);

	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_dialog_cb, view_barcode);
	gl_label_barcode_set_data (GL_LABEL_BARCODE(object), text_node);
	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_dialog_cb, view_barcode);

	gl_text_node_free (&text_node);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  barcode props "changed" callback.                               */
/*---------------------------------------------------------------------------*/
static void
bc_props_changed_cb (glWdgtBCProps  *text_props,
		     glViewBarcode  *view_barcode)
{
	glLabelObject      *object;
	glBarcodeStyle     style;
	gboolean           text_flag;
	gboolean           checksum_flag;
	guint              color;

	gl_debug (DEBUG_VIEW, "START");

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_barcode));

	gl_label_barcode_get_props (GL_LABEL_BARCODE(object),
				    &style, &text_flag, &checksum_flag, &color);
	gl_wdgt_bc_props_get_params (text_props, &color);

	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_dialog_cb, view_barcode);
	gl_label_barcode_set_props (GL_LABEL_BARCODE(object),
				    style, text_flag, checksum_flag, color);
	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_dialog_cb, view_barcode);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  barcode style "changed" callback.                               */
/*---------------------------------------------------------------------------*/
static void
bc_style_changed_cb (glWdgtBCStyle  *bc_style,
		     glViewBarcode  *view_barcode)
{
	glLabelObject      *object;
	glBarcodeStyle     style;
	gboolean           text_flag;
	gboolean           checksum_flag;
	guint              color;


	gl_debug (DEBUG_VIEW, "START");

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_barcode));

	gl_label_barcode_get_props (GL_LABEL_BARCODE(object),
				    &style, &text_flag, &checksum_flag, &color);
	gl_wdgt_bc_style_get_params (bc_style, &style, &text_flag, &checksum_flag);

	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_dialog_cb, view_barcode);
	gl_label_barcode_set_props (GL_LABEL_BARCODE(object),
				    style, text_flag, checksum_flag, color);
	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_dialog_cb, view_barcode);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  position "changed" callback.                                    */
/*---------------------------------------------------------------------------*/
static void
position_changed_cb (glWdgtPosition     *position,
		     glViewBarcode      *view_barcode)
{
	glLabelObject      *object;
	gdouble            x, y;

	gl_debug (DEBUG_VIEW, "START");

	gl_wdgt_position_get_position (GL_WDGT_POSITION (position), &x, &y);

	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_barcode));

	g_signal_handlers_block_by_func (G_OBJECT(object),
					 update_dialog_cb, view_barcode);
	gl_label_object_set_position (GL_LABEL_OBJECT(object), x, y);
	g_signal_handlers_unblock_by_func (G_OBJECT(object),
					   update_dialog_cb, view_barcode);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "changed" callback.                                 */
/*---------------------------------------------------------------------------*/
static void
update_dialog_cb (glLabelObject  *object,
		  glViewBarcode     *view_barcode)
{
	gdouble            x, y;
	glTextNode         *text_node;
	glBarcodeStyle     style;
	gboolean           text_flag;
	gboolean           checksum_flag;
	guint              color;
	glMerge            *merge;

	gl_debug (DEBUG_VIEW, "START");

	/* Query properties of object. */
	text_node = gl_label_barcode_get_data(GL_LABEL_BARCODE(object));
	gl_label_barcode_get_props (GL_LABEL_BARCODE(object),
				    &style, &text_flag, &checksum_flag, &color);
	gl_label_object_get_position (GL_LABEL_OBJECT(object), &x, &y);
	merge = gl_label_get_merge (GL_LABEL(object->parent));

	/* Block widget handlers to prevent recursion */
	g_signal_handlers_block_by_func (G_OBJECT(view_barcode->private->bc_data),
					 bc_data_changed_cb, view_barcode);
	g_signal_handlers_block_by_func (G_OBJECT(view_barcode->private->bc_props),
					 bc_props_changed_cb, view_barcode);
	g_signal_handlers_block_by_func (G_OBJECT(view_barcode->private->bc_style),
					 bc_style_changed_cb, view_barcode);
	g_signal_handlers_block_by_func (G_OBJECT(view_barcode->private->position),
					 position_changed_cb, view_barcode);

	/* Update widgets in property dialog */

	gl_wdgt_bc_data_set_data (GL_WDGT_BC_DATA(view_barcode->private->bc_data),
				  (merge != NULL),
				  text_node);
	gl_wdgt_bc_data_set_field_defs (GL_WDGT_BC_DATA(view_barcode->private->bc_data),
					merge);
	gl_wdgt_bc_props_set_params (GL_WDGT_BC_PROPS(view_barcode->private->bc_props),
				     color);
	gl_wdgt_bc_style_set_params (GL_WDGT_BC_STYLE(view_barcode->private->bc_style),
				     style, text_flag, checksum_flag);
	gl_wdgt_position_set_position (GL_WDGT_POSITION(view_barcode->private->position),
				       x, y);

	/* Unblock widget handlers */
	g_signal_handlers_unblock_by_func (G_OBJECT(view_barcode->private->bc_data),
					   bc_data_changed_cb, view_barcode);
	g_signal_handlers_unblock_by_func (G_OBJECT(view_barcode->private->bc_props),
					   bc_props_changed_cb, view_barcode);
	g_signal_handlers_unblock_by_func (G_OBJECT(view_barcode->private->bc_style),
					   bc_style_changed_cb, view_barcode);
	g_signal_handlers_unblock_by_func (G_OBJECT(view_barcode->private->position),
					   position_changed_cb, view_barcode);

	gl_text_node_free (&text_node);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. label object "moved" callback.                                   */
/*---------------------------------------------------------------------------*/
static void
update_dialog_from_move_cb (glLabelObject *object,
			    gdouble        dx,
			    gdouble        dy,
			    glViewBarcode *view_barcode)
{
	gdouble            x, y;

	gl_debug (DEBUG_VIEW, "START");

	/* Query properties of object. */
	gl_label_object_get_position (GL_LABEL_OBJECT(object), &x, &y);

	/* Block widget handlers to prevent recursion */
	g_signal_handlers_block_by_func (G_OBJECT(view_barcode->private->position),
					 position_changed_cb, view_barcode);

	/* Update widgets in property dialog */
	gl_wdgt_position_set_position (GL_WDGT_POSITION(view_barcode->private->position),
				       x, y);

	/* Unblock widget handlers */
	g_signal_handlers_unblock_by_func (G_OBJECT(view_barcode->private->position),
					   position_changed_cb, view_barcode);

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
						    GL_BARCODE_STYLE_POSTNET,
						    FALSE,
						    TRUE,
						    gl_color_set_opacity (gl_prefs->default_line_color, 0.5));
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
						    GL_BARCODE_STYLE_POSTNET,
						    FALSE,
						    TRUE,
						    gl_prefs->default_line_color);
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
	glBarcodeStyle style;
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

	gl_debug (DEBUG_VIEW, "START");

	/* Query label object and properties */
	object = gl_view_object_get_object (GL_VIEW_OBJECT(view_barcode));
	gl_label_barcode_get_props (GL_LABEL_BARCODE(object),
				    &style, &text_flag, &checksum_flag, &color);
	gl_label_object_get_size (object, &w, &h);
	text_node = gl_label_barcode_get_data(GL_LABEL_BARCODE(object));
	if (text_node->field_flag) {
		digits = gl_barcode_default_digits (style);
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

	gbc = gl_barcode_new (style, text_flag, checksum_flag, w, h, digits);
	if (gbc == NULL) {

		cstring = _("Invalid barcode");
		glyphlist = gnome_glyphlist_from_text_sized_dumb (font,
								  color,
								  0.0, 0.0,
								  cstring,
								  strlen
								  (cstring));
		y_offset = 10.0 - gnome_font_get_descender (font);
		item = gl_view_object_item_new (GL_VIEW_OBJECT(view_barcode),
						gl_canvas_hacktext_get_type (),
						"x", 0.0,
						"y", y_offset,
						"glyphlist", glyphlist, NULL);

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
			    bchar->fsize - gnome_font_get_descender (font);
			item = gl_view_object_item_new (GL_VIEW_OBJECT(view_barcode),
							gnome_canvas_hacktext_get_type (),
							"x", bchar->x,
							"y", bchar->y + y_offset,
							"glyphlist", glyphlist,
							NULL);

			view_barcode->private->item_list =
				g_list_prepend (view_barcode->private->item_list, item);

		}

	}

	/* clean up */
	gl_barcode_free (&gbc);
	g_free (digits);

	gl_debug (DEBUG_VIEW, "END");
}

