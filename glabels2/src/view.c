/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view.c:  GLabels View module
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

#include "view.h"

#include <glib/gi18n.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtklayout.h>
#include <gtk/gtkselection.h>
#include <gtk/gtkinvisible.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <math.h>

#include "label.h"
#include "cairo-label-path.h"
#include "cairo-markup-path.h"
#include "view-object.h"
#include "view-box.h"
#include "view-ellipse.h"
#include "view-line.h"
#include "view-image.h"
#include "view-text.h"
#include "view-barcode.h"
#include "xml-label.h"
#include "color.h"
#include "prefs.h"
#include "marshal.h"

#include "debug.h"

/*==========================================================================*/
/* Private macros and constants.                                            */
/*==========================================================================*/

#define BG_COLOR        GL_COLOR (192, 192, 192)

#define PAPER_RGB_ARGS          1.0,   1.0,   1.0
#define GRID_RGB_ARGS           0.753, 0.753, 0.753
#define MARKUP_RGB_ARGS         0.94,  0.39,  0.39
#define OUTLINE_RGB_ARGS        0.68,  0.85,  0.90
#define SELECT_LINE_RGBA_ARGS   0.0,   0.0,   1.0,   0.5
#define SELECT_FILL_RGBA_ARGS   0.75,  0.75,  1.0,   0.5

#define GRID_LINE_WIDTH_PIXELS    1.0
#define MARKUP_LINE_WIDTH_PIXELS  1.0
#define OUTLINE_WIDTH_PIXELS      3.0
#define SELECT_LINE_WIDTH_PIXELS  3.0

#define ZOOMTOFIT_PAD   16

#define POINTS_PER_MM    2.83464566929

/*==========================================================================*/
/* Private types.                                                           */
/*==========================================================================*/

enum {
	SELECTION_CHANGED,
	CONTEXT_MENU_ACTIVATE,
	ZOOM_CHANGED,
	POINTER_MOVED,
	POINTER_EXIT,
	MODE_CHANGED,
	LAST_SIGNAL
};


/*==========================================================================*/
/* Private globals                                                          */
/*==========================================================================*/

static guint signals[LAST_SIGNAL] = {0};

/* "CLIPBOARD" selection */
static GdkAtom clipboard_atom = GDK_NONE;

static gdouble zooms[] = {
	8.00,
	6.00,
	4.00,
	3.00,
	2.00,
	1.50,
	1.00,
	0.75,
	0.67,
	0.50,
	0.33,
	0.25,
	0.20,
	0.15,
	0.10,
};
#define N_ZOOMS G_N_ELEMENTS(zooms)


/*==========================================================================*/
/* Local function prototypes                                                */
/*==========================================================================*/

static void       gl_view_finalize                (GObject        *object);

static void       gl_view_construct               (glView         *view,
                                                   glLabel        *label);

static gdouble    get_home_scale                  (glView         *view);

static gboolean   expose_cb                       (glView         *view,
                                                   GdkEventExpose *event);

static void       realize_cb                      (glView         *view);

static void       size_allocate_cb                (glView         *view,
                                                   GtkAllocation  *allocation);

static void       screen_changed_cb               (glView         *view);

static void       label_changed_cb                (glView         *view);

static void       label_resized_cb                (glView         *view);

static void       label_object_added_cb           (glView         *view,
                                                   glLabelObject  *object);

static void       draw_layers                     (glView         *view,
                                                   cairo_t        *cr);

static void       draw_bg_layer                   (glView         *view,
                                                   cairo_t        *cr);
static void       draw_grid_layer                 (glView         *view,
                                                   cairo_t        *cr);
static void       draw_markup_layer               (glView         *view,
                                                   cairo_t        *cr);
static void       draw_objects_layer              (glView         *view,
                                                   cairo_t        *cr);
static void       draw_fg_layer                   (glView         *view,
                                                   cairo_t        *cr);
static void       draw_highlight_layer            (glView         *view,
                                                   cairo_t        *cr);
static void       draw_select_region_layer        (glView         *view,
                                                   cairo_t        *cr);

static void       select_object_real              (glView         *view,
						   glViewObject   *view_object);
static void       unselect_object_real            (glView         *view,
						   glViewObject   *view_object);

static glViewObject *view_view_object_at          (glView         *view,
                                                   cairo_t        *cr,
						   gdouble         x,
                                                   gdouble         y);

static void       set_zoom_real                   (glView         *view,
						   gdouble         zoom,
						   gboolean        scale_to_fit_flag);

static void       selection_clear_cb              (GtkWidget         *widget,
                                                   GdkEventSelection *event,
                                                   glView            *view);

static void       selection_get_cb                (GtkWidget         *widget,
                                                   GtkSelectionData  *selection_data,
                                                   guint              info,
                                                   guint              time,
                                                   glView            *view);

static void       selection_received_cb           (GtkWidget         *widget,
                                                   GtkSelectionData  *selection_data,
                                                   guint              time,
                                                   glView            *view);

static gboolean   focus_in_event_cb               (glView            *view,
                                                   GdkEventFocus     *event);

static gboolean   focus_out_event_cb              (glView            *view,
                                                   GdkEventFocus     *event);

static gboolean   enter_notify_event_cb           (glView            *view,
                                                   GdkEventCrossing  *event);

static gboolean   leave_notify_event_cb           (glView            *view,
                                                   GdkEventCrossing  *event);

static gboolean   motion_notify_event_cb          (glView            *view,
                                                   GdkEventMotion    *event);

static gboolean   button_press_event_cb           (glView            *view,
                                                   GdkEventButton    *event);

static gboolean   button_release_event_cb         (glView            *view,
                                                   GdkEventButton    *event);

static gboolean   key_press_event_cb              (glView            *view,
                                                   GdkEventKey       *event);


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glView, gl_view, GTK_TYPE_VBOX);

static void
gl_view_class_init (glViewClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	gl_debug (DEBUG_VIEW, "START");

	gl_view_parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_view_finalize;

	signals[SELECTION_CHANGED] =
		g_signal_new ("selection_changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (glViewClass, selection_changed),
			      NULL, NULL,
			      gl_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

	signals[CONTEXT_MENU_ACTIVATE] =
		g_signal_new ("context_menu_activate",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (glViewClass, context_menu_activate),
			      NULL, NULL,
			      gl_marshal_VOID__INT_UINT,
			      G_TYPE_NONE,
			      2, G_TYPE_INT, G_TYPE_UINT);

	signals[ZOOM_CHANGED] =
		g_signal_new ("zoom_changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (glViewClass, zoom_changed),
			      NULL, NULL,
			      gl_marshal_VOID__DOUBLE,
			      G_TYPE_NONE,
			      1, G_TYPE_DOUBLE);

	signals[POINTER_MOVED] =
		g_signal_new ("pointer_moved",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (glViewClass, pointer_moved),
			      NULL, NULL,
			      gl_marshal_VOID__DOUBLE_DOUBLE,
			      G_TYPE_NONE,
			      2, G_TYPE_DOUBLE, G_TYPE_DOUBLE);

	signals[POINTER_EXIT] =
		g_signal_new ("pointer_exit",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (glViewClass, pointer_exit),
			      NULL, NULL,
			      gl_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

	signals[MODE_CHANGED] =
		g_signal_new ("mode_changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (glViewClass, mode_changed),
			      NULL, NULL,
			      gl_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_init (glView *view)
{
	GtkWidget *wscroll;
	GdkColor  *bg_color;

	gl_debug (DEBUG_VIEW, "START");

	view->label                = NULL;
	view->grid_visible         = TRUE;
	view->grid_spacing         = 9;
	view->markup_visible       = TRUE;
	view->default_font_family  = NULL;
	view->mode                 = GL_VIEW_MODE_ARROW;
	view->object_list          = NULL;
	view->selected_object_list = NULL;
	view->zoom                 = 1.0;
	view->home_scale           = get_home_scale (view);

        /*
         * Canvas
         */
        view->canvas = gtk_layout_new (NULL, NULL);
        wscroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (wscroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (view), wscroll, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (wscroll), view->canvas);

	bg_color = gl_color_to_gdk_color (BG_COLOR);
	gtk_widget_modify_bg (GTK_WIDGET (view->canvas), GTK_STATE_NORMAL, bg_color);
	g_free (bg_color);

        GTK_WIDGET_SET_FLAGS (GTK_WIDGET (view->canvas), GTK_CAN_FOCUS);

        gtk_widget_add_events (GTK_WIDGET (view->canvas),
                               (GDK_FOCUS_CHANGE_MASK   |
                                GDK_ENTER_NOTIFY_MASK   |
                                GDK_LEAVE_NOTIFY_MASK   |
                                GDK_POINTER_MOTION_MASK |
                                GDK_BUTTON_PRESS_MASK   |
                                GDK_BUTTON_RELEASE_MASK |
                                GDK_KEY_PRESS_MASK));

	g_signal_connect_swapped (G_OBJECT (view->canvas), "expose-event",
				  G_CALLBACK (expose_cb), view);
	g_signal_connect_swapped (G_OBJECT (view->canvas), "realize",
				  G_CALLBACK (realize_cb), view);
	g_signal_connect_swapped (G_OBJECT (view->canvas), "size-allocate",
				  G_CALLBACK (size_allocate_cb), view);
	g_signal_connect_swapped (G_OBJECT (view->canvas), "screen-changed",
				  G_CALLBACK (screen_changed_cb), view);
	g_signal_connect_swapped (G_OBJECT (view->canvas), "focus-in-event",
				  G_CALLBACK (focus_in_event_cb), view);
	g_signal_connect_swapped (G_OBJECT (view->canvas), "focus-out-event",
				  G_CALLBACK (focus_out_event_cb), view);
	g_signal_connect_swapped (G_OBJECT (view->canvas), "enter-notify-event",
				  G_CALLBACK (enter_notify_event_cb), view);
	g_signal_connect_swapped (G_OBJECT (view->canvas), "leave-notify-event",
				  G_CALLBACK (leave_notify_event_cb), view);
	g_signal_connect_swapped (G_OBJECT (view->canvas), "motion-notify-event",
				  G_CALLBACK (motion_notify_event_cb), view);
	g_signal_connect_swapped (G_OBJECT (view->canvas), "button-press-event",
				  G_CALLBACK (button_press_event_cb), view);
	g_signal_connect_swapped (G_OBJECT (view->canvas), "button-release-event",
				  G_CALLBACK (button_release_event_cb), view);
	g_signal_connect_swapped (G_OBJECT (view->canvas), "key-press-event",
				  G_CALLBACK (key_press_event_cb), view);

        /*
         * Clipboard
         */
	view->have_selection       = FALSE;
	view->selection_data       = NULL;
	view->invisible            = gtk_invisible_new ();
	if (!clipboard_atom) {
		clipboard_atom = gdk_atom_intern ("GLABELS_CLIPBOARD", FALSE);
	}
	gtk_selection_add_target (view->invisible,
				  clipboard_atom, GDK_SELECTION_TYPE_STRING, 1);
	g_signal_connect (G_OBJECT (view->invisible),
			  "selection_clear_event",
			  G_CALLBACK (selection_clear_cb), view);
	g_signal_connect (G_OBJECT (view->invisible), "selection_get",
			  G_CALLBACK (selection_get_cb), view);
	g_signal_connect (G_OBJECT (view->invisible),
			  "selection_received",
			  G_CALLBACK (selection_received_cb), view);

        /*
         * Defaults from preferences
         */
	gl_view_set_default_font_family       (view, gl_prefs->default_font_family);
	gl_view_set_default_font_size         (view, gl_prefs->default_font_size);
	gl_view_set_default_font_weight       (view, gl_prefs->default_font_weight);
	gl_view_set_default_font_italic_flag  (view, gl_prefs->default_font_italic_flag);
	gl_view_set_default_text_color        (view, gl_prefs->default_text_color);
	gl_view_set_default_text_alignment    (view, gl_prefs->default_text_alignment);
	gl_view_set_default_text_line_spacing (view, gl_prefs->default_text_line_spacing);
	gl_view_set_default_line_width        (view, gl_prefs->default_line_width);
	gl_view_set_default_line_color        (view, gl_prefs->default_line_color);
	gl_view_set_default_fill_color        (view, gl_prefs->default_fill_color);

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_finalize (GObject *object)
{
	glView *view = GL_VIEW (object);

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_VIEW (object));

	if (view->default_font_family) {
		g_free (view->default_font_family);
	}

	G_OBJECT_CLASS (gl_view_parent_class)->finalize (object);

	gl_debug (DEBUG_VIEW, "END");
}

/****************************************************************************/
/* NEW view object.                                                         */
/****************************************************************************/
GtkWidget *
gl_view_new (glLabel *label)
{
	glView *view;

	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (label && GL_IS_LABEL (label), NULL);

	view = g_object_new (GL_TYPE_VIEW, NULL);

	gl_view_construct (view, label);

	gl_debug (DEBUG_VIEW, "END");

	return GTK_WIDGET (view);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                     */
/*---------------------------------------------------------------------------*/
static void
gl_view_construct (glView  *view,
                   glLabel *label)
{
        GList            *p_obj;
        glLabelObject    *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

	view->label = label;

        for (p_obj = label->objects; p_obj != NULL; p_obj = p_obj->next)
        {
                object = GL_LABEL_OBJECT (p_obj->data);

                if (GL_IS_LABEL_BOX (object)) {
                        gl_view_box_new (GL_LABEL_BOX(object), view);
                } else if (GL_IS_LABEL_ELLIPSE (object)) {
                        gl_view_ellipse_new (GL_LABEL_ELLIPSE(object), view);
                } else if (GL_IS_LABEL_LINE (object)) {
                        gl_view_line_new (GL_LABEL_LINE(object), view);
                } else if (GL_IS_LABEL_IMAGE (object)) {
                        gl_view_image_new (GL_LABEL_IMAGE(object), view);
                } else if (GL_IS_LABEL_TEXT (object)) {
                        gl_view_text_new (GL_LABEL_TEXT(object), view);
                } else if (GL_IS_LABEL_BARCODE (object)) {
                        gl_view_barcode_new (GL_LABEL_BARCODE(object), view);
                } else {
                        /* Should not happen! */
                        g_message ("Invalid label object type.");
                }
        }

	g_signal_connect_swapped (G_OBJECT (view->label), "changed",
                                  G_CALLBACK (label_changed_cb), view);
	g_signal_connect_swapped (G_OBJECT (view->label), "size_changed",
                                  G_CALLBACK (label_resized_cb), view);
	g_signal_connect_swapped (G_OBJECT (view->label), "object_added",
                                  G_CALLBACK (label_object_added_cb), view);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIAVTE.  Calculate 1:1 scale for screen.                                 */
/*---------------------------------------------------------------------------*/
static gdouble
get_home_scale (glView *view)
{
	GdkScreen *screen;
	gdouble    screen_width_pixels;
	gdouble    screen_width_mm;
	gdouble    screen_height_pixels;
	gdouble    screen_height_mm;
	gdouble    x_pixels_per_mm;
	gdouble    y_pixels_per_mm;
	gdouble    scale;

	if (view->canvas == NULL) return 1.0;

	if (!gtk_widget_has_screen (GTK_WIDGET (view->canvas))) return 1.0;

	screen = gtk_widget_get_screen (GTK_WIDGET (view->canvas));

	gl_debug (DEBUG_VIEW, "Screen = %p", screen);

	screen_width_pixels  = gdk_screen_get_width (screen);
	screen_width_mm      = gdk_screen_get_width_mm (screen);
	screen_height_pixels = gdk_screen_get_height (screen);
	screen_height_mm     = gdk_screen_get_height_mm (screen);

	x_pixels_per_mm      = screen_width_pixels / screen_width_mm;
	y_pixels_per_mm      = screen_height_pixels / screen_height_mm;

	gl_debug (DEBUG_VIEW, "Horizontal dot pitch: %g pixels/mm (%g dpi)",
		  x_pixels_per_mm, x_pixels_per_mm * 25.4);
	gl_debug (DEBUG_VIEW, "Vertical dot pitch: %g pixels/mm (%g dpi)",
		  y_pixels_per_mm, y_pixels_per_mm * 25.4);

	scale = (x_pixels_per_mm + y_pixels_per_mm) / 2.0;

	gl_debug (DEBUG_VIEW, "Average dot pitch: %g pixels/mm (%g dpi)",
		  scale, scale * 25.4);

	scale /= POINTS_PER_MM;

	gl_debug (DEBUG_VIEW, "Scale = %g pixels/point", scale);

	/* Make sure scale is somewhat sane. */
	if ( (scale < 0.25) || (scale > 4.0) ) return 1.0;

	return scale;
}

/*---------------------------------------------------------------------------*/
/* Schedule canvas update.                                                   */
/*---------------------------------------------------------------------------*/
void
gl_view_update (glView  *view)
{
 	GtkWidget *widget;
	GdkRegion *region;
	
	gl_debug (DEBUG_VIEW, "START");

	widget = GTK_WIDGET (view->canvas);

	if (!widget->window) return;

        if ( !view->update_scheduled_flag )
        {
                view->update_scheduled_flag = TRUE;

                region = gdk_drawable_get_clip_region (widget->window);
                /* redraw the cairo canvas completely by exposing it */
                gdk_window_invalidate_region (widget->window, region, TRUE);
                gdk_region_destroy (region);
        }

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* Schedule canvas region update.                                            */
/*---------------------------------------------------------------------------*/
void
gl_view_update_region (glView        *view,
                       cairo_t       *cr,
                       glLabelRegion *region)
{
 	GtkWidget    *widget;
	GdkRectangle  rect;
        gdouble       x, y, w, h;

	gl_debug (DEBUG_VIEW, "START");

	widget = GTK_WIDGET (view->canvas);

	if (!widget->window) return;

        x = MIN (region->x1, region->x2);
        y = MIN (region->y1, region->y2);
        w = fabs (region->x2 - region->x1);
        h = fabs (region->y2 - region->y1);

        cairo_user_to_device (cr, &x, &y);
        cairo_user_to_device_distance (cr, &w, &h);

        rect.x      = x - 3;
        rect.y      = y - 3;
        rect.width  = w + 6;
        rect.height = h + 6;

        gdk_window_invalidate_rect (widget->window, &rect, TRUE);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Expose handler.                                                 */
/*---------------------------------------------------------------------------*/
static gboolean
expose_cb (glView         *view,
           GdkEventExpose *event)
{
	cairo_t *cr;

	gl_debug (DEBUG_VIEW, "START");

        view->update_scheduled_flag = FALSE;

	/* get a cairo_t */
	cr = gdk_cairo_create (GTK_LAYOUT (view->canvas)->bin_window);

	cairo_rectangle (cr,
			event->area.x, event->area.y,
			event->area.width, event->area.height);
	cairo_clip (cr);
	
	draw_layers (view, cr);

	cairo_destroy (cr);

	gl_debug (DEBUG_VIEW, "END");

	return FALSE;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Realize handler.                                                */
/*---------------------------------------------------------------------------*/
static void
realize_cb (glView  *view)
{
	g_return_if_fail (view && GL_IS_VIEW (view));

	gl_debug (DEBUG_VIEW, "START");

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. Size allocation changed callback.                                */
/*---------------------------------------------------------------------------*/
static void
size_allocate_cb (glView         *view,
                  GtkAllocation  *allocation)
{
	gl_debug (DEBUG_VIEW, "START");

        GTK_LAYOUT (view->canvas)->hadjustment->page_size = allocation->width;
        GTK_LAYOUT (view->canvas)->hadjustment->page_increment = allocation->width / 2;
 
        GTK_LAYOUT (view->canvas)->vadjustment->page_size = allocation->height;
        GTK_LAYOUT (view->canvas)->vadjustment->page_increment = allocation->height / 2;

        g_signal_emit_by_name (GTK_LAYOUT (view->canvas)->hadjustment, "changed");
        g_signal_emit_by_name (GTK_LAYOUT (view->canvas)->vadjustment, "changed");

	if (view->zoom_to_fit_flag) {
		/* Maintain best fit zoom */
		gl_view_zoom_to_fit (view);
	}

	gl_debug (DEBUG_VIEW, "END");
}



/*---------------------------------------------------------------------------*/
/* PRIVATE. Screen changed callback.                                         */
/*---------------------------------------------------------------------------*/
static void
screen_changed_cb (glView *view)
{
	gl_debug (DEBUG_VIEW, "START");

	if (gtk_widget_has_screen (GTK_WIDGET (view->canvas))) {

		view->home_scale = get_home_scale (view);

		if (view->zoom_to_fit_flag) {
			/* Maintain best fit zoom */
			gl_view_zoom_to_fit (view);
		}
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Handle label changed event.                                     */
/*---------------------------------------------------------------------------*/
static void
label_changed_cb (glView  *view)
{
	g_return_if_fail (view && GL_IS_VIEW (view));

	gl_debug (DEBUG_VIEW, "START");

        gl_view_update (view);

	gl_debug (DEBUG_VIEW, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Handle label resize event.                                      */
/*---------------------------------------------------------------------------*/
static void
label_resized_cb (glView  *view)
{
	g_return_if_fail (view && GL_IS_VIEW (view));

	gl_debug (DEBUG_VIEW, "START");

        g_signal_emit_by_name (GTK_LAYOUT (view->canvas)->hadjustment, "changed");
        g_signal_emit_by_name (GTK_LAYOUT (view->canvas)->vadjustment, "changed");

        gl_view_update (view);

	gl_debug (DEBUG_VIEW, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Handle new label object.                                        */
/*---------------------------------------------------------------------------*/
static void
label_object_added_cb (glView         *view,
                       glLabelObject  *object)
{
        glViewObject *view_object;

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (object && GL_IS_LABEL_OBJECT (object));

        if (GL_IS_LABEL_BOX (object)) {
                view_object = gl_view_box_new (GL_LABEL_BOX(object), view);
        } else if (GL_IS_LABEL_ELLIPSE (object)) {
                view_object = gl_view_ellipse_new (GL_LABEL_ELLIPSE(object), view);
        } else if (GL_IS_LABEL_LINE (object)) {
                view_object = gl_view_line_new (GL_LABEL_LINE(object), view);
        } else if (GL_IS_LABEL_IMAGE (object)) {
                view_object = gl_view_image_new (GL_LABEL_IMAGE(object), view);
        } else if (GL_IS_LABEL_TEXT (object)) {
                view_object = gl_view_text_new (GL_LABEL_TEXT(object), view);
        } else if (GL_IS_LABEL_BARCODE (object)) {
                view_object = gl_view_barcode_new (GL_LABEL_BARCODE(object), view);
        } else {
                /* Should not happen! */
                view_object = NULL;
                g_message ("Invalid label object type.");
        }

        gl_view_select_object (view, view_object);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create, draw and order layers.                                  */
/*---------------------------------------------------------------------------*/
static void
draw_layers (glView  *view,
             cairo_t *cr)
{
	gdouble                    scale;
	gdouble                    w, h;
        gint                       canvas_w, canvas_h;

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (view->label && GL_IS_LABEL (view->label));

	gl_debug (DEBUG_VIEW, "START");

        scale = view->zoom * view->home_scale;

        gl_label_get_size (view->label, &w, &h);

        scale = view->home_scale * view->zoom;
        gtk_layout_set_size (GTK_LAYOUT (view->canvas), w*scale+8, h*scale+8);

        gdk_drawable_get_size (GTK_LAYOUT (view->canvas)->bin_window, &canvas_w, &canvas_h);

        view->x0 = (canvas_w/scale - w) / 2.0;
        view->y0 = (canvas_h/scale - h) / 2.0;
        view->w  = w;
        view->h  = h;

        cairo_save (cr);

        cairo_scale (cr, scale, scale);
        cairo_translate (cr, view->x0, view->y0);

	draw_bg_layer (view, cr);
	draw_grid_layer (view, cr);
	draw_markup_layer (view, cr);
	draw_objects_layer (view, cr);
	draw_fg_layer (view, cr);
	draw_highlight_layer (view, cr);
        draw_select_region_layer (view, cr);

        cairo_restore (cr);

	gl_debug (DEBUG_VIEW, "END");

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw background                                                 */
/*---------------------------------------------------------------------------*/
static void
draw_bg_layer (glView  *view,
               cairo_t *cr)
{
	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (view->label && GL_IS_LABEL (view->label));

        gl_cairo_label_path (cr, view->label->template, view->label->rotate_flag, FALSE);

        cairo_set_source_rgb (cr, PAPER_RGB_ARGS);
        cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
        cairo_fill (cr);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw grid lines.                                                */
/*---------------------------------------------------------------------------*/
static void
draw_grid_layer (glView  *view,
                 cairo_t *cr)
{
	gdouble                    w, h;
	gdouble                    x, y;
	gdouble                    x0, y0;
	const lglTemplateFrame    *frame;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (view->label && GL_IS_LABEL(view->label));

        if (view->grid_visible)
        {

                frame = (lglTemplateFrame *)view->label->template->frames->data;

                gl_label_get_size (view->label, &w, &h);
	
                if (frame->shape == LGL_TEMPLATE_FRAME_SHAPE_RECT) {
                        x0 = 0.0;
                        y0 = 0.0;
                } else {
                        /* round labels, adjust grid to line up with center of label. */
                        x0 = fmod (w/2.0, view->grid_spacing);
                        y0 = fmod (h/2.0, view->grid_spacing);
                }


                cairo_save (cr);

                cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
                cairo_set_line_width (cr, GRID_LINE_WIDTH_PIXELS/(view->home_scale * view->zoom));
                cairo_set_source_rgb (cr, GRID_RGB_ARGS);

                for ( x=x0+view->grid_spacing; x < w; x += view->grid_spacing )
                {
                        cairo_move_to (cr, x, 0);
                        cairo_line_to (cr, x, h);
                        cairo_stroke (cr);
                }

                for ( y=y0+view->grid_spacing; y < h; y += view->grid_spacing )
                {
                        cairo_move_to (cr, 0, y);
                        cairo_line_to (cr, w, y);
                        cairo_stroke (cr);
                }

                cairo_restore (cr);

        }

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw markup layer.                                              */
/*---------------------------------------------------------------------------*/
static void
draw_markup_layer (glView  *view,
                   cairo_t *cr)
{
	glLabel                   *label;
	const lglTemplateFrame    *frame;
	GList                     *p;
	lglTemplateMarkup         *markup;
        gdouble                    width, height;

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (view->label && GL_IS_LABEL (view->label));

        if (view->markup_visible)
        {

                label      = view->label;
                frame = (lglTemplateFrame *)view->label->template->frames->data;

                cairo_save (cr);

                if (label->rotate_flag)
                {
                        lgl_template_frame_get_size (frame, &width, &height);
                        cairo_rotate (cr, -M_PI/2.0);
                        cairo_translate (cr, -width, 0.0);
                }

                cairo_set_line_width (cr, MARKUP_LINE_WIDTH_PIXELS/(view->home_scale * view->zoom));
                cairo_set_source_rgb (cr, MARKUP_RGB_ARGS);

                for ( p=frame->all.markups; p != NULL; p=p->next )
                {
                        markup = (lglTemplateMarkup *)p->data;

                        gl_cairo_markup_path (cr, markup, label);

                        cairo_stroke (cr);
                }

                cairo_restore (cr);
        }

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw objects layer.                                             */
/*---------------------------------------------------------------------------*/
static void
draw_objects_layer (glView  *view,
                    cairo_t *cr)
{
        gl_label_draw (view->label, cr, TRUE, NULL);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw foreground                                                 */
/*---------------------------------------------------------------------------*/
static void
draw_fg_layer (glView  *view,
               cairo_t *cr)
{
	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (view->label && GL_IS_LABEL (view->label));

        gl_cairo_label_path (cr, view->label->template, view->label->rotate_flag, FALSE);

        cairo_set_line_width (cr, OUTLINE_WIDTH_PIXELS/(view->home_scale * view->zoom));
        cairo_set_source_rgb (cr, OUTLINE_RGB_ARGS);
        cairo_stroke (cr);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create highlight layer.                                         */
/*---------------------------------------------------------------------------*/
static void
draw_highlight_layer (glView  *view,
                      cairo_t *cr)
{
	GList            *p_obj;
	glViewObject     *view_object;

	g_return_if_fail (view && GL_IS_VIEW (view));

        cairo_save (cr);

        cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

	for (p_obj = view->selected_object_list; p_obj != NULL; p_obj = p_obj->next)
        {
		view_object = GL_VIEW_OBJECT (p_obj->data);

                gl_view_object_draw_handles (view_object, cr);
	}

        cairo_restore (cr);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw select region layer.                                       */
/*---------------------------------------------------------------------------*/
static void
draw_select_region_layer (glView  *view,
                          cairo_t *cr)
{
        gdouble x1, y1;
        gdouble w, h;

	g_return_if_fail (view && GL_IS_VIEW (view));

        if (view->select_region_visible)
        {
                x1 = MIN (view->select_region.x1, view->select_region.x2);
                y1 = MIN (view->select_region.y1, view->select_region.y2);
                w  = fabs (view->select_region.x2 - view->select_region.x1);
                h  = fabs (view->select_region.y2 - view->select_region.y1);

                cairo_save (cr);

                cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

                cairo_rectangle (cr, x1, y1, w, h);

                cairo_set_source_rgba (cr, SELECT_FILL_RGBA_ARGS);
                cairo_fill_preserve (cr);

                cairo_set_line_width (cr, SELECT_LINE_WIDTH_PIXELS/(view->home_scale * view->zoom));
                cairo_set_source_rgba (cr, SELECT_LINE_RGBA_ARGS);
                cairo_stroke (cr);

                cairo_restore (cr);
        }
}

/*****************************************************************************/
/* Show grid.                                                                */
/*****************************************************************************/
void
gl_view_show_grid (glView *view)
{
	g_return_if_fail (view && GL_IS_VIEW (view));

        view->grid_visible = TRUE;
        gl_view_update (view);
}

/*****************************************************************************/
/* Hide grid.                                                                */
/*****************************************************************************/
void
gl_view_hide_grid (glView *view)
{
	g_return_if_fail (view && GL_IS_VIEW (view));

        view->grid_visible = FALSE;
        gl_view_update (view);
}

/*****************************************************************************/
/* Set grid spacing.                                                         */
/*****************************************************************************/
void
gl_view_set_grid_spacing (glView  *view,
			  gdouble  spacing)
{
	g_return_if_fail (view && GL_IS_VIEW (view));

	view->grid_spacing = spacing;
        gl_view_update (view);
}

/*****************************************************************************/
/* Show markup.                                                              */
/*****************************************************************************/
void
gl_view_show_markup (glView *view)
{
	g_return_if_fail (view && GL_IS_VIEW (view));

        view->markup_visible = TRUE;
        gl_view_update (view);
}

/*****************************************************************************/
/* Hide markup.                                                              */
/*****************************************************************************/
void
gl_view_hide_markup (glView *view)
{
	g_return_if_fail (view && GL_IS_VIEW (view));

        view->markup_visible = FALSE;
        gl_view_update (view);
}

/*****************************************************************************/
/* Set arrow mode.                                                           */
/*****************************************************************************/
void
gl_view_arrow_mode (glView *view)
{
	GdkCursor *cursor;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

        cursor = gdk_cursor_new (GDK_LEFT_PTR);
	gdk_window_set_cursor (view->canvas->window, cursor);
        gdk_cursor_unref (cursor);

	view->mode = GL_VIEW_MODE_ARROW;
        view->state = GL_VIEW_IDLE;

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Set create text object mode.                                              */
/*****************************************************************************/
void
gl_view_object_create_mode (glView            *view,
			    glLabelObjectType  type)
{
	GdkCursor *cursor = NULL;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	switch (type)
        {
	case GL_LABEL_OBJECT_BOX:
		cursor = gl_view_box_get_create_cursor ();
		break;
	case GL_LABEL_OBJECT_ELLIPSE:
		cursor = gl_view_ellipse_get_create_cursor ();
		break;
	case GL_LABEL_OBJECT_LINE:
		cursor = gl_view_line_get_create_cursor ();
		break;
	case GL_LABEL_OBJECT_IMAGE:
		cursor = gl_view_image_get_create_cursor ();
		break;
	case GL_LABEL_OBJECT_TEXT:
		cursor = gl_view_text_get_create_cursor ();
		break;
	case GL_LABEL_OBJECT_BARCODE:
		cursor = gl_view_barcode_get_create_cursor ();
		break;
	default:
		g_message ("Invalid label object type.");/*Should not happen!*/
		break;
	}

	gdk_window_set_cursor (view->canvas->window, cursor);
        gdk_cursor_unref (cursor);

	view->mode = GL_VIEW_MODE_OBJECT_CREATE;
        view->state = GL_VIEW_IDLE;
	view->create_type = type;

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Select given object (adding to current selection).                        */
/*****************************************************************************/
void
gl_view_select_object (glView       *view,
		       glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	select_object_real (view, view_object);

	g_signal_emit (G_OBJECT(view), signals[SELECTION_CHANGED], 0);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Unselect given object (removing from current selection).                  */
/*****************************************************************************/
void
gl_view_unselect_object (glView       *view,
			 glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	unselect_object_real (view, view_object);

	g_signal_emit (G_OBJECT(view), signals[SELECTION_CHANGED], 0);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Select all items.                                                         */
/*****************************************************************************/
void
gl_view_select_all (glView *view)
{
	GList *p, *p_next;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	/* 1st unselect anything already selected. */
	for (p = view->selected_object_list; p != NULL; p = p_next) {
		p_next = p->next;
		unselect_object_real (view, GL_VIEW_OBJECT (p->data));
	}

	/* Finally select all objects. */
	for (p = view->object_list; p != NULL; p = p->next) {
		select_object_real (view, GL_VIEW_OBJECT (p->data));
	}

	g_signal_emit (G_OBJECT(view), signals[SELECTION_CHANGED], 0);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Remove all selections                                                     */
/*****************************************************************************/
void
gl_view_unselect_all (glView *view)
{
	GList *p;
	GList *p_next;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p_next) {
		p_next = p->next;
		unselect_object_real (view, GL_VIEW_OBJECT (p->data));
	}

	g_signal_emit (G_OBJECT(view), signals[SELECTION_CHANGED], 0);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Select all objects within given rectangular region (adding to selection). */
/*****************************************************************************/
void
gl_view_select_region (glView        *view,
                       glLabelRegion *region)
{
	GList         *p;
	glViewObject  *view_object;
	glLabelObject *object;
        gdouble        r_x1, r_y1;
        gdouble        r_x2, r_y2;
        glLabelRegion  obj_extent;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

        r_x1 = MIN (region->x1, region->x2);
        r_y1 = MIN (region->y1, region->y2);
        r_x2 = MAX (region->x1, region->x2);
        r_y2 = MAX (region->y1, region->y2);

	for (p = view->object_list; p != NULL; p = p->next)
        {
		view_object = GL_VIEW_OBJECT(p->data);
		if (!gl_view_is_object_selected (view, view_object))
                {

			object = gl_view_object_get_object (view_object);

			gl_label_object_get_extent (object, &obj_extent);
			if ((obj_extent.x1 >= r_x1) &&
                            (obj_extent.x2 <= r_x2) &&
                            (obj_extent.y1 >= r_y1) &&
                            (obj_extent.y2 <= r_y2))
                        {
				select_object_real (view, view_object);
			}

		}
	}

	g_signal_emit (G_OBJECT(view), signals[SELECTION_CHANGED], 0);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. Select an object.                                                */
/*---------------------------------------------------------------------------*/
static void
select_object_real (glView       *view,
		    glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (GL_IS_VIEW_OBJECT (view_object));

	if (!gl_view_is_object_selected (view, view_object)) {
		view->selected_object_list =
		    g_list_append (view->selected_object_list, view_object);
	}
	gtk_widget_grab_focus (GTK_WIDGET (view->canvas));

        gl_view_update (view);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Un-select object.                                               */
/*---------------------------------------------------------------------------*/
static void
unselect_object_real (glView       *view,
		      glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (GL_IS_VIEW_OBJECT (view_object));

	view->selected_object_list =
	    g_list_remove (view->selected_object_list, view_object);

        gl_view_update (view);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. Return object at (x,y).                                          */
/*---------------------------------------------------------------------------*/
static glViewObject *
view_view_object_at (glView  *view,
                     cairo_t *cr,
                     gdouble  x,
                     gdouble  y)
{
	GList            *p_obj;
	glViewObject     *view_object;

	g_return_val_if_fail (view && GL_IS_VIEW (view), NULL);

	for (p_obj = g_list_last (view->object_list); p_obj != NULL; p_obj = p_obj->prev)
        {

		view_object = GL_VIEW_OBJECT (p_obj->data);

                if (gl_view_object_at (view_object, cr, x, y))
                {
                        return view_object;
                }

	}

        return NULL;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. Return object handle at (x,y).                                   */
/*---------------------------------------------------------------------------*/
static glViewObject *
view_handle_at (glView             *view,
                cairo_t            *cr,
                gdouble             x,
                gdouble             y,
                glViewObjectHandle *handle)
{
	GList            *p_obj;
	glViewObject     *view_object;

	g_return_val_if_fail (view && GL_IS_VIEW (view), NULL);

	for (p_obj = g_list_last (view->selected_object_list); p_obj != NULL; p_obj = p_obj->prev)
        {

		view_object = GL_VIEW_OBJECT (p_obj->data);

                if ((*handle = gl_view_object_handle_at (view_object, cr, x, y)))
                {
                        return view_object;
                }

	}

        return NULL;
}

/*****************************************************************************/
/* Is the object in our current selection?                                   */
/*****************************************************************************/
gboolean
gl_view_is_object_selected (glView       *view,
			    glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "");

	g_return_val_if_fail (view && GL_IS_VIEW (view), FALSE);
	g_return_val_if_fail (GL_IS_VIEW_OBJECT (view_object), FALSE);

	if (g_list_find (view->selected_object_list, view_object) == NULL) {
		return FALSE;
	}
	return TRUE;
}

/*****************************************************************************/
/* Is our current selection empty?                                           */
/*****************************************************************************/
gboolean
gl_view_is_selection_empty (glView *view)
{
	gl_debug (DEBUG_VIEW, "");

	g_return_val_if_fail (view && GL_IS_VIEW (view), FALSE);

	if (view->selected_object_list == NULL) {
		return TRUE;
	} else {
		return FALSE;
	}
}

/*****************************************************************************/
/* Is our current selection atomic?  I.e. only one item selected.            */
/*****************************************************************************/
gboolean
gl_view_is_selection_atomic (glView *view)
{
	gl_debug (DEBUG_VIEW, "");

	g_return_val_if_fail (view && GL_IS_VIEW (view), FALSE);

	if (view->selected_object_list == NULL)
		return FALSE;
	if (view->selected_object_list->next == NULL)
		return TRUE;
	return FALSE;
}

/*****************************************************************************/
/* Delete selected objects. (Bypass clipboard)                               */
/*****************************************************************************/
void
gl_view_delete_selection (glView *view)
{
	GList         *object_list;
	GList         *p;
	GList         *p_next;
	glViewObject  *view_object;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	object_list = view->selected_object_list;
	view->selected_object_list = NULL;
	g_signal_emit (G_OBJECT(view), signals[SELECTION_CHANGED], 0);

	for (p = object_list; p != NULL; p = p_next) {
		p_next = p->next;
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
                gl_label_object_remove (object);
	}

        g_list_free (object_list);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Get object property editor of first selected object.                      */
/*****************************************************************************/
GtkWidget *
gl_view_get_editor (glView *view)
{
	glViewObject *view_object;
	GtkWidget    *editor = NULL;

	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view && GL_IS_VIEW (view), NULL);

	if (!gl_view_is_selection_empty (view)) {

		view_object = GL_VIEW_OBJECT(view->selected_object_list->data);
		editor = gl_view_object_get_editor (view_object);

	}

	gl_debug (DEBUG_VIEW, "END");

	return editor;
}

/*****************************************************************************/
/* Raise selected items to top.                                              */
/*****************************************************************************/
void
gl_view_raise_selection (glView *view)
{
	GList         *p;
	glViewObject  *view_object;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_raise_to_top (object);
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Lower selected items to bottom.                                           */
/*****************************************************************************/
void
gl_view_lower_selection (glView *view)
{
	GList         *p;
	glViewObject  *view_object;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_lower_to_bottom (object);
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Rotate selected objects by given angle.                                   */
/*****************************************************************************/
void
gl_view_rotate_selection (glView *view,
			  gdouble theta_degs)
{
	GList         *p;
	glViewObject  *view_object;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_rotate (object, theta_degs);
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Rotate selected objects 90 degrees left.                                  */
/*****************************************************************************/
void
gl_view_rotate_selection_left (glView *view)
{
	GList         *p;
	glViewObject  *view_object;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_rotate (object, -90.0);
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Rotate selected objects 90 degrees right.                                 */
/*****************************************************************************/
void
gl_view_rotate_selection_right (glView *view)
{
	GList         *p;
	glViewObject  *view_object;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_rotate (object, 90.0);
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Flip selected objects horizontally.                                       */
/*****************************************************************************/
void
gl_view_flip_selection_horiz (glView *view)
{
	GList         *p;
	glViewObject  *view_object;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_flip_horiz (object);
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Flip selected objects vertically.                                         */
/*****************************************************************************/
void
gl_view_flip_selection_vert (glView *view)
{
	GList         *p;
	glViewObject  *view_object;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_flip_vert (object);
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Align selected objects to left most edge.                                 */
/*****************************************************************************/
void
gl_view_align_selection_left (glView *view)
{
	GList         *p;
	glViewObject  *view_object;
	glLabelObject *object;
	gdouble        dx, x1_min;
        glLabelRegion  obj_extent;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	g_return_if_fail (!gl_view_is_selection_empty (view) &&
			  !gl_view_is_selection_atomic (view));

	/* find left most edge */
	p = view->selected_object_list;
	view_object = GL_VIEW_OBJECT (p->data);
	object = gl_view_object_get_object (view_object);
	gl_label_object_get_extent (object, &obj_extent);
        x1_min = obj_extent.x1;
	for (p = p->next; p != NULL; p = p->next)
        {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_get_extent (object, &obj_extent);
		if ( obj_extent.x1 < x1_min ) x1_min = obj_extent.x1;
	}

	/* now adjust the object positions to line up the left edges */
	for (p = view->selected_object_list; p != NULL; p = p->next)
        {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_get_extent (object, &obj_extent);
		dx = x1_min - obj_extent.x1;
		gl_label_object_set_position_relative (object, dx, 0.0);
	}

	gl_debug (DEBUG_VIEW, "END");
}


/*****************************************************************************/
/* Align selected objects to right most edge.                                */
/*****************************************************************************/
void
gl_view_align_selection_right (glView *view)
{
	GList         *p;
	glViewObject  *view_object;
	glLabelObject *object;
	gdouble        dx, x2_max;
        glLabelRegion  obj_extent;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	g_return_if_fail (!gl_view_is_selection_empty (view) &&
			  !gl_view_is_selection_atomic (view));

	/* find right most edge */
	p = view->selected_object_list;
	view_object = GL_VIEW_OBJECT (p->data);
	object = gl_view_object_get_object (view_object);
	gl_label_object_get_extent (object, &obj_extent);
        x2_max = obj_extent.x2;
	for (p = p->next; p != NULL; p = p->next)
        {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_get_extent (object, &obj_extent);
		if ( obj_extent.x2 > x2_max ) x2_max = obj_extent.x2;
	}

	/* now adjust the object positions to line up the right edges */
	for (p = view->selected_object_list; p != NULL; p = p->next)
        {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_get_extent (object, &obj_extent);
		dx = x2_max - obj_extent.x2;
		gl_label_object_set_position_relative (object, dx, 0.0);
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Align selected objects to horizontal center of objects.                   */
/*****************************************************************************/
void
gl_view_align_selection_hcenter (glView *view)
{
	GList         *p;
	glViewObject  *view_object;
	glLabelObject *object;
	gdouble        dx;
	gdouble        dxmin;
	gdouble        xsum, xavg;
        glLabelRegion  obj_extent;
	gdouble        xcenter;
	gint           n;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	g_return_if_fail (!gl_view_is_selection_empty (view) &&
			  !gl_view_is_selection_atomic (view));

	/* find average center of objects */
	xsum = 0.0;
	n = 0;
	for (p = view->selected_object_list; p != NULL; p = p->next)
        {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_get_extent (object, &obj_extent);
		xsum += (obj_extent.x1 + obj_extent.x2) / 2.0;
		n++;
	}
	xavg = xsum / n;

	/* find center of object closest to average center */
	p = view->selected_object_list;
	view_object = GL_VIEW_OBJECT (p->data);
	object = gl_view_object_get_object (view_object);
	gl_label_object_get_extent (object, &obj_extent);
	dxmin = fabs (xavg - (obj_extent.x1 + obj_extent.x2)/2.0);
	xcenter = (obj_extent.x1 + obj_extent.x2)/2.0;
	for (p = p->next; p != NULL; p = p->next)
        {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_get_extent (object, &obj_extent);
		dx = fabs (xavg - (obj_extent.x1 + obj_extent.x2)/2.0);
		if ( dx < dxmin )
                {
			dxmin = dx;
			xcenter = (obj_extent.x1 + obj_extent.x2)/2.0;
		}
	}

	/* now adjust the object positions to line up this center */
	for (p = view->selected_object_list; p != NULL; p = p->next) {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_get_extent (object, &obj_extent);
		dx = xcenter - (obj_extent.x1 + obj_extent.x2)/2.0;
		gl_label_object_set_position_relative (object, dx, 0.0);
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Align selected objects to top most edge.                                  */
/*****************************************************************************/
void
gl_view_align_selection_top (glView *view)
{
	GList         *p;
	glViewObject  *view_object;
	glLabelObject *object;
	gdouble        dy, y1_min;
        glLabelRegion  obj_extent;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	g_return_if_fail (!gl_view_is_selection_empty (view) &&
			  !gl_view_is_selection_atomic (view));

	/* find top most edge */
	p = view->selected_object_list;
	view_object = GL_VIEW_OBJECT (p->data);
	object = gl_view_object_get_object (view_object);
	gl_label_object_get_extent (object, &obj_extent);
        y1_min = obj_extent.y1;
	for (p = p->next; p != NULL; p = p->next)
        {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_get_extent (object, &obj_extent);
		if ( obj_extent.y1 < y1_min ) y1_min = obj_extent.y1;
	}

	/* now adjust the object positions to line up the top edges */
	for (p = view->selected_object_list; p != NULL; p = p->next)
        {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_get_extent (object, &obj_extent);
		dy = y1_min - obj_extent.y1;
		gl_label_object_set_position_relative (object, 0.0, dy);
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Align selected objects to bottom most edge.                               */
/*****************************************************************************/
void
gl_view_align_selection_bottom (glView *view)
{
	GList         *p;
	glViewObject  *view_object;
	glLabelObject *object;
	gdouble        dy, y2_max;
        glLabelRegion  obj_extent;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	g_return_if_fail (!gl_view_is_selection_empty (view) &&
			  !gl_view_is_selection_atomic (view));

	/* find bottom most edge */
	p = view->selected_object_list;
	view_object = GL_VIEW_OBJECT (p->data);
	object = gl_view_object_get_object (view_object);
	gl_label_object_get_extent (object, &obj_extent);
        y2_max = obj_extent.y2;
	for (p = p->next; p != NULL; p = p->next)
        {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_get_extent (object, &obj_extent);
		if ( obj_extent.y2 > y2_max ) y2_max = obj_extent.y2;
	}

	/* now adjust the object positions to line up the bottom edges */
	for (p = view->selected_object_list; p != NULL; p = p->next)
        {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_get_extent (object, &obj_extent);
		dy = y2_max - obj_extent.y2;
		gl_label_object_set_position_relative (object, 0.0, dy);
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Align selected objects to viertical center of objects.                    */
/*****************************************************************************/
void
gl_view_align_selection_vcenter (glView *view)
{
	GList         *p;
	glViewObject  *view_object;
	glLabelObject *object;
	gdouble        dy;
	gdouble        dymin;
	gdouble        ysum, yavg;
        glLabelRegion  obj_extent;
	gdouble        ycenter;
	gint           n;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	g_return_if_fail (!gl_view_is_selection_empty (view) &&
			  !gl_view_is_selection_atomic (view));

	/* find average center of objects */
	ysum = 0.0;
	n = 0;
	for (p = view->selected_object_list; p != NULL; p = p->next)
        {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_get_extent (object, &obj_extent);
		ysum += (obj_extent.y1 + obj_extent.y2) / 2.0;
		n++;
	}
	yavg = ysum / n;

	/* find center of object closest to average center */
	p = view->selected_object_list;
	view_object = GL_VIEW_OBJECT (p->data);
	object = gl_view_object_get_object (view_object);
	gl_label_object_get_extent (object, &obj_extent);
	dymin = fabs (yavg - (obj_extent.y1 + obj_extent.y2)/2.0);
	ycenter = (obj_extent.y1 + obj_extent.y2)/2.0;
	for (p = p->next; p != NULL; p = p->next)
        {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_get_extent (object, &obj_extent);
		dy = fabs (yavg - (obj_extent.y1 + obj_extent.y2)/2.0);
		if ( dy < dymin )
                {
			dymin = dy;
			ycenter = (obj_extent.y1 + obj_extent.y2)/2.0;
		}
	}

	/* now adjust the object positions to line up this center */
	for (p = view->selected_object_list; p != NULL; p = p->next)
        {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_get_extent (object, &obj_extent);
		dy = ycenter - (obj_extent.y1 + obj_extent.y2)/2.0;
		gl_label_object_set_position_relative (object, 0.0, dy);
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Center selected objects to in center of label.                            */
/*****************************************************************************/
void
gl_view_center_selection_horiz (glView *view)
{
	GList         *p;
	glViewObject  *view_object;
	glLabelObject *object;
	gdouble        dx;
	gdouble        x_label_center;
	gdouble        x_obj_center;
	glLabelRegion  obj_extent;
	gdouble        w, h;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	g_return_if_fail (!gl_view_is_selection_empty (view));

	gl_label_get_size (view->label, &w, &h);
	x_label_center = w / 2.0;

	/* adjust the object positions */
	for (p = view->selected_object_list; p != NULL; p = p->next)
        {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_get_extent (object, &obj_extent);
		x_obj_center = (obj_extent.x1 + obj_extent.x2) / 2.0;
		dx = x_label_center - x_obj_center;
		gl_label_object_set_position_relative (object, dx, 0.0);
	}

	gl_debug (DEBUG_VIEW, "END");
}


/*****************************************************************************/
/* Center selected objects to in center of label.                            */
/*****************************************************************************/
void
gl_view_center_selection_vert (glView *view)
{
	GList         *p;
	glViewObject  *view_object;
	glLabelObject *object;
	gdouble        dy;
	gdouble        y_label_center;
	gdouble        y_obj_center;
	glLabelRegion  obj_extent;
	gdouble        w, h;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	g_return_if_fail (!gl_view_is_selection_empty (view));

	gl_label_get_size (view->label, &w, &h);
	y_label_center = h / 2.0;

	/* adjust the object positions */
	for (p = view->selected_object_list; p != NULL; p = p->next)
        {
		view_object = GL_VIEW_OBJECT (p->data);
		object = gl_view_object_get_object (view_object);
		gl_label_object_get_extent (object, &obj_extent);
		y_obj_center = (obj_extent.y1 + obj_extent.y2) / 2.0;
		dy = y_label_center - y_obj_center;
		gl_label_object_set_position_relative (object, 0.0, dy);
	}

	gl_debug (DEBUG_VIEW, "END");
}


/*****************************************************************************/
/* Move selected objects                                                     */
/*****************************************************************************/
void
gl_view_move_selection (glView  *view,
			gdouble  dx,
			gdouble  dy)
{
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next)
        {

		object = gl_view_object_get_object(GL_VIEW_OBJECT (p->data));
		gl_label_object_set_position_relative (object, dx, dy);

	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Can text properties be set for selection?                                 */
/*****************************************************************************/
gboolean
gl_view_can_selection_text (glView *view)
{
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "");

	g_return_val_if_fail (view && GL_IS_VIEW (view), FALSE);

	for (p = view->selected_object_list; p != NULL; p = p->next)
        {

		object = gl_view_object_get_object(GL_VIEW_OBJECT (p->data));
		if (gl_label_object_can_text (object))
                {
			return TRUE;
		}

	}

	return FALSE;
}

/*****************************************************************************/
/* Set font family for all text contained in selected objects.               */
/*****************************************************************************/
void
gl_view_set_selection_font_family (glView      *view,
				   const gchar *font_family)
{
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {

		object = gl_view_object_get_object(GL_VIEW_OBJECT (p->data));
		gl_label_object_set_font_family (object, font_family);

	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Set font size for all text contained in selected objects.                 */
/*****************************************************************************/
void
gl_view_set_selection_font_size (glView  *view,
				 gdouble  font_size)
{
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {

		object = gl_view_object_get_object(GL_VIEW_OBJECT (p->data));
		gl_label_object_set_font_size (object, font_size);

	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Set font weight for all text contained in selected objects.               */
/*****************************************************************************/
void
gl_view_set_selection_font_weight (glView      *view,
				   PangoWeight  font_weight)
{
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {

		object = gl_view_object_get_object(GL_VIEW_OBJECT (p->data));
		gl_label_object_set_font_weight (object, font_weight);

	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Set font italic flag for all text contained in selected objects.          */
/*****************************************************************************/
void
gl_view_set_selection_font_italic_flag (glView   *view,
					gboolean  font_italic_flag)
{
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {

		object = gl_view_object_get_object(GL_VIEW_OBJECT (p->data));
		gl_label_object_set_font_italic_flag (object, font_italic_flag);

	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Set text alignment for all text contained in selected objects.            */
/*****************************************************************************/
void
gl_view_set_selection_text_alignment (glView            *view,
				      PangoAlignment     text_alignment)
{
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {

		object = gl_view_object_get_object(GL_VIEW_OBJECT (p->data));
		gl_label_object_set_text_alignment (object, text_alignment);

	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Set text line spacing for all text contained in selected objects.         */
/*****************************************************************************/
void
gl_view_set_selection_text_line_spacing (glView  *view,
				         gdouble  text_line_spacing)
{
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {

		object = gl_view_object_get_object(GL_VIEW_OBJECT (p->data));
		gl_label_object_set_text_line_spacing (object, text_line_spacing);

	}

	gl_debug (DEBUG_VIEW, "END");
}
/*****************************************************************************/
/* Set text color for all text contained in selected objects.                */
/*****************************************************************************/
void
gl_view_set_selection_text_color (glView      *view,
				  glColorNode *text_color_node)
{
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {

		object = gl_view_object_get_object(GL_VIEW_OBJECT (p->data));
		gl_label_object_set_text_color (object, text_color_node);

	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Can fill properties be set for selection?                                 */
/*****************************************************************************/
gboolean
gl_view_can_selection_fill (glView *view)
{
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "");

	g_return_val_if_fail (view && GL_IS_VIEW (view), FALSE);

	for (p = view->selected_object_list; p != NULL; p = p->next) {

		object = gl_view_object_get_object(GL_VIEW_OBJECT (p->data));
		if (gl_label_object_can_fill (object)) {
			return TRUE;
		}

	}

	return FALSE;
}

/*****************************************************************************/
/* Set fill color for all selected objects.                                  */
/*****************************************************************************/
void
gl_view_set_selection_fill_color (glView      *view,
				  glColorNode *fill_color_node)
{
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {

		object = gl_view_object_get_object(GL_VIEW_OBJECT (p->data));
		gl_label_object_set_fill_color (object, fill_color_node);

	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Can line color properties be set for selection?                           */
/*****************************************************************************/
gboolean
gl_view_can_selection_line_color (glView *view)
{
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "");

	g_return_val_if_fail (view && GL_IS_VIEW (view), FALSE);

	for (p = view->selected_object_list; p != NULL; p = p->next) {

		object = gl_view_object_get_object(GL_VIEW_OBJECT (p->data));
		if (gl_label_object_can_line_color (object)) {
			return TRUE;
		}

	}

	return FALSE;
}

/*****************************************************************************/
/* Set line color for all selected objects.                                  */
/*****************************************************************************/
void
gl_view_set_selection_line_color (glView      *view,
				  glColorNode *line_color_node)
{
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {

		object = gl_view_object_get_object(GL_VIEW_OBJECT (p->data));
		gl_label_object_set_line_color (object, line_color_node);

	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Can line width properties be set for selection?                           */
/*****************************************************************************/
gboolean
gl_view_can_selection_line_width (glView *view)
{
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "");

	g_return_val_if_fail (view && GL_IS_VIEW (view), FALSE);

	for (p = view->selected_object_list; p != NULL; p = p->next) {

		object = gl_view_object_get_object(GL_VIEW_OBJECT (p->data));
		if (gl_label_object_can_line_width (object)) {
			return TRUE;
		}

	}

	return FALSE;
}

/*****************************************************************************/
/* Set line width for all selected objects.                                  */
/*****************************************************************************/
void
gl_view_set_selection_line_width (glView  *view,
				  gdouble  line_width)
{
	GList         *p;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {

		object = gl_view_object_get_object(GL_VIEW_OBJECT (p->data));
		gl_label_object_set_line_width (object, line_width);

	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* "Cut" selected items and place in clipboard selections.                   */
/*****************************************************************************/
void
gl_view_cut (glView *view)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	gl_view_copy (view);
	gl_view_delete_selection (view);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* "Copy" selected items to clipboard selections.                            */
/*****************************************************************************/
void
gl_view_copy (glView *view)
{
	GList         *p;
	glViewObject  *view_object;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	if (view->selected_object_list) {

		if ( view->selection_data ) {
			g_object_unref (view->selection_data);
		}
		view->selection_data = GL_LABEL(gl_label_new ());
		gl_label_set_template (view->selection_data, view->label->template);
		gl_label_set_rotate_flag (view->selection_data, view->label->rotate_flag);

		for (p = view->selected_object_list; p != NULL; p = p->next) {

			view_object = GL_VIEW_OBJECT (p->data);
			object = gl_view_object_get_object (view_object);

			gl_label_object_dup (object, view->selection_data);

		}

		gtk_selection_owner_set (view->invisible,
					 clipboard_atom, GDK_CURRENT_TIME);
		view->have_selection = TRUE;

	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* "Paste" from private clipboard selection.                                 */
/*****************************************************************************/
void
gl_view_paste (glView *view)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	gtk_selection_convert (GTK_WIDGET (view->invisible),
			       clipboard_atom, GDK_SELECTION_TYPE_STRING,
			       GDK_CURRENT_TIME);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Zoom in one "notch"                                                       */
/*****************************************************************************/
void
gl_view_zoom_in (glView *view)
{
	gint    i, i_min;
	gdouble dist, dist_min;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	/* Find index of current scale (or best match) */
	i_min = 1;		/* start with 2nd largest scale */
	dist_min = fabs (zooms[1] - view->zoom);
	for (i = 2; i < N_ZOOMS; i++) {
		dist = fabs (zooms[i] - view->zoom);
		if (dist < dist_min) {
			i_min = i;
			dist_min = dist;
		}
	}

	/* zoom in one "notch" */
	i = MAX (0, i_min - 1);
	gl_debug (DEBUG_VIEW, "zoom[%d] = %g", i, zooms[i]);
	set_zoom_real (view, zooms[i], FALSE);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Zoom out one "notch"                                                      */
/*****************************************************************************/
void
gl_view_zoom_out (glView *view)
{
	gint    i, i_min;
	gdouble dist, dist_min;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	/* Find index of current scale (or best match) */
	i_min = 0;		/* start with largest scale */
	dist_min = fabs (zooms[0] - view->zoom);
	for (i = 1; i < N_ZOOMS; i++) {
		dist = fabs (zooms[i] - view->zoom);
		if (dist < dist_min) {
			i_min = i;
			dist_min = dist;
		}
	}

	/* zoom out one "notch" */
	if (i_min >= N_ZOOMS)
		return;
	i = i_min + 1;
	if (i >= N_ZOOMS)
		return;
	set_zoom_real (view, zooms[i], FALSE);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Set zoom to best fit.                                                     */
/*****************************************************************************/
void
gl_view_zoom_to_fit (glView *view)
{
	gint    w_view, h_view;
	gdouble w_label, h_label;
	gdouble x_scale, y_scale, scale;

	gl_debug (DEBUG_VIEW, "");

	if ( ! GTK_WIDGET_VISIBLE(view)) {
		set_zoom_real (view, 1.0, TRUE);
		return;
	}

	w_view = GTK_WIDGET(view)->allocation.width;
	h_view = GTK_WIDGET(view)->allocation.height;

	gl_label_get_size (GL_LABEL(view->label), &w_label, &h_label);

	gl_debug (DEBUG_VIEW, "View size: %d, %d", w_view, h_view);
	gl_debug (DEBUG_VIEW, "Label size: %g, %g", w_label, h_label);

	/* Calculate best scale */
	x_scale = (double)(w_view - ZOOMTOFIT_PAD) / w_label;
	y_scale = (double)(h_view - ZOOMTOFIT_PAD) / h_label;
	scale = MIN (x_scale, y_scale);
	gl_debug (DEBUG_VIEW, "Candidate zooms: %g, %g => %g", x_scale, y_scale, scale);

	/* Limit */
	gl_debug (DEBUG_VIEW, "Scale: %g", scale);
	scale = MIN (scale, zooms[0]*view->home_scale);
	scale = MAX (scale, zooms[N_ZOOMS-1]*view->home_scale);
	gl_debug (DEBUG_VIEW, "Limitted scale: %g", scale);

	set_zoom_real (view, scale/view->home_scale, TRUE);
}

/*****************************************************************************/
/* Set current zoom factor to explicit value.                                */
/*****************************************************************************/
void
gl_view_set_zoom (glView  *view,
		  gdouble  zoom)
{
	gl_debug (DEBUG_VIEW, "START");

	set_zoom_real (view, zoom, FALSE);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set canvas scale.                                               */
/*---------------------------------------------------------------------------*/
static void
set_zoom_real (glView   *view,
	       gdouble   zoom,
	       gboolean  zoom_to_fit_flag)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (zoom > 0.0);

	/* Limit, if needed */
	gl_debug (DEBUG_VIEW, "Zoom requested: %g", zoom);
	zoom = MIN (zoom, zooms[0]);
	zoom = MAX (zoom, zooms[N_ZOOMS-1]);
	gl_debug (DEBUG_VIEW, "Limitted zoom: %g", zoom);

	if ( zoom != view->zoom ) {

		view->zoom = zoom;
		view->zoom_to_fit_flag = zoom_to_fit_flag;

                gl_view_update (view);

		g_signal_emit (G_OBJECT(view), signals[ZOOM_CHANGED], 0, zoom);

	}

	gl_debug (DEBUG_VIEW, "END");

}


/*****************************************************************************/
/* Get current zoom factor.                                                  */
/*****************************************************************************/
gdouble
gl_view_get_zoom (glView *view)
{
	gl_debug (DEBUG_VIEW, "");

	g_return_val_if_fail (view && GL_IS_VIEW (view), 1.0);

	return view->zoom;
}

/*****************************************************************************/
/* Is this the maximum zoom level.                                           */
/*****************************************************************************/
gboolean
gl_view_is_zoom_max (glView *view)
{
	gl_debug (DEBUG_VIEW, "");

	g_return_val_if_fail (GL_IS_VIEW (view), FALSE);

	return view->zoom >= zooms[0];
}

/*****************************************************************************/
/* Is this the minimum zoom level.                                           */
/*****************************************************************************/
gboolean
gl_view_is_zoom_min (glView *view)
{
	gl_debug (DEBUG_VIEW, "");

	g_return_val_if_fail (view && GL_IS_VIEW (view), FALSE);

	return view->zoom <= zooms[N_ZOOMS-1];
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Handle "selection-clear" signal.                                */
/*---------------------------------------------------------------------------*/
static void
selection_clear_cb (GtkWidget         *widget,
		    GdkEventSelection *event,
		    glView            *view)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	view->have_selection = FALSE;
	g_object_unref (view->selection_data);
	view->selection_data = NULL;

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Handle "selection-get" signal.                                  */
/*---------------------------------------------------------------------------*/
static void
selection_get_cb (GtkWidget        *widget,
		  GtkSelectionData *selection_data,
		  guint             info,
		  guint             time,
		  glView           *view)
{
	gchar            *buffer;
	glXMLLabelStatus  status;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	if (view->have_selection) {

		buffer = gl_xml_label_save_buffer (view->selection_data,
						   &status);
		gtk_selection_data_set (selection_data,
					GDK_SELECTION_TYPE_STRING, 8,
					(guchar *)buffer, strlen (buffer));
		g_free (buffer);
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Handle "selection-received" signal.  (Result of Paste)          */
/*---------------------------------------------------------------------------*/
static void
selection_received_cb (GtkWidget        *widget,
		       GtkSelectionData *selection_data,
		       guint             time,
		       glView           *view)
{
	glLabel          *label = NULL;
	glXMLLabelStatus  status;
	GList            *p, *p_next;
	glLabelObject    *object, *newobject;
	glViewObject     *view_object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	if (selection_data->length < 0) {
		return;
	}
	if (selection_data->type != GDK_SELECTION_TYPE_STRING) {
		return;
	}

	gl_view_unselect_all (view);

	label = gl_xml_label_open_buffer ((gchar *)selection_data->data, &status);
	for (p = label->objects; p != NULL; p = p_next) {
		p_next = p->next;

		object = (glLabelObject *) p->data;
		newobject = gl_label_object_dup (object, view->label);

		gl_debug (DEBUG_VIEW, "object pasted");

		if (GL_IS_LABEL_BOX (newobject)) {
			view_object = gl_view_box_new (GL_LABEL_BOX(newobject),
						       view);
		} else if (GL_IS_LABEL_ELLIPSE (newobject)) {
			view_object = gl_view_ellipse_new (GL_LABEL_ELLIPSE(newobject),
							   view);
		} else if (GL_IS_LABEL_LINE (newobject)) {
			view_object = gl_view_line_new (GL_LABEL_LINE(newobject),
							view);
		} else if (GL_IS_LABEL_IMAGE (newobject)) {
			view_object = gl_view_image_new (GL_LABEL_IMAGE(newobject),
							 view);
		} else if (GL_IS_LABEL_TEXT (newobject)) {
			view_object = gl_view_text_new (GL_LABEL_TEXT(newobject),
							view);
		} else if (GL_IS_LABEL_BARCODE (newobject)) {
			view_object = gl_view_barcode_new (GL_LABEL_BARCODE(newobject),
							   view);
		} else {
			/* Should not happen! */
			view_object = NULL;
			g_message ("Invalid label object type.");
		}
		gl_view_select_object (view, view_object);
	}
	g_object_unref (label);

	gl_debug (DEBUG_VIEW, "END");
}

/****************************************************************************/
/* Set default font family.                                                 */
/****************************************************************************/
void
gl_view_set_default_font_family (glView      *view,
				 const gchar *font_family)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	if (view->default_font_family) {
		g_free (view->default_font_family);
	}
	view->default_font_family = g_strdup (font_family);

	gl_debug (DEBUG_VIEW, "END");
}


/****************************************************************************/
/* Set default font size.                                                   */
/****************************************************************************/
void
gl_view_set_default_font_size (glView  *view,
			       gdouble  font_size)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	view->default_font_size = font_size;

	gl_debug (DEBUG_VIEW, "END");
}


/****************************************************************************/
/* Set default font weight.                                                 */
/****************************************************************************/
void
gl_view_set_default_font_weight (glView      *view,
				 PangoWeight  font_weight)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	view->default_font_weight = font_weight;

	gl_debug (DEBUG_VIEW, "END");
}


/****************************************************************************/
/* Set default font italic flag.                                            */
/****************************************************************************/
void
gl_view_set_default_font_italic_flag (glView   *view,
				      gboolean  font_italic_flag)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	view->default_font_italic_flag = font_italic_flag;

	gl_debug (DEBUG_VIEW, "END");
}


/****************************************************************************/
/* Set default text color.                                                  */
/****************************************************************************/
void
gl_view_set_default_text_color (glView *view,
				guint   text_color)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	view->default_text_color = text_color;

	gl_debug (DEBUG_VIEW, "END");
}


/****************************************************************************/
/* Set default text alignment.                                              */
/****************************************************************************/
void
gl_view_set_default_text_alignment (glView           *view,
				    PangoAlignment    text_alignment)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	view->default_text_alignment = text_alignment;
	gl_debug (DEBUG_VIEW, "END");
}

/****************************************************************************/
/* Set default text line spacing.                                           */
/****************************************************************************/
void
gl_view_set_default_text_line_spacing (glView  *view,
			               gdouble  text_line_spacing)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	view->default_text_line_spacing = text_line_spacing;

	gl_debug (DEBUG_VIEW, "END");
}


/****************************************************************************/
/* Set default line width.                                                  */
/****************************************************************************/
void
gl_view_set_default_line_width (glView  *view,
				gdouble  line_width)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	view->default_line_width = line_width;

	gl_debug (DEBUG_VIEW, "END");
}


/****************************************************************************/
/* Set default line color.                                                  */
/****************************************************************************/
void
gl_view_set_default_line_color (glView *view,
				guint   line_color)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	view->default_line_color = line_color;

	gl_debug (DEBUG_VIEW, "END");
}


/****************************************************************************/
/* Set default fill color.                                                  */
/****************************************************************************/
void
gl_view_set_default_fill_color (glView *view,
				guint   fill_color)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

	view->default_fill_color = fill_color;

	gl_debug (DEBUG_VIEW, "END");
}



/****************************************************************************/
/* Get default font family.                                                 */
/****************************************************************************/
gchar *
gl_view_get_default_font_family (glView *view)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view && GL_IS_VIEW (view), NULL);

	gl_debug (DEBUG_VIEW, "END");

	return g_strdup (view->default_font_family);
}


/****************************************************************************/
/* Get default font size.                                                   */
/****************************************************************************/
gdouble
gl_view_get_default_font_size (glView *view)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view && GL_IS_VIEW (view), 12.0);

	gl_debug (DEBUG_VIEW, "END");

	return view->default_font_size;
}


/****************************************************************************/
/* Get default font weight.                                                 */
/****************************************************************************/
PangoWeight
gl_view_get_default_font_weight (glView *view)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view && GL_IS_VIEW (view), PANGO_WEIGHT_NORMAL);

	gl_debug (DEBUG_VIEW, "END");

	return view->default_font_weight;
}


/****************************************************************************/
/* Get default font italic flag.                                            */
/****************************************************************************/
gboolean
gl_view_get_default_font_italic_flag (glView *view)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view && GL_IS_VIEW (view), FALSE);

	gl_debug (DEBUG_VIEW, "END");

	return view->default_font_italic_flag;
}


/****************************************************************************/
/* Get default text color.                                                  */
/****************************************************************************/
guint
gl_view_get_default_text_color (glView *view)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view && GL_IS_VIEW (view), 0);

	gl_debug (DEBUG_VIEW, "END");

	return view->default_text_color;
}


/****************************************************************************/
/* Get default text alignment.                                              */
/****************************************************************************/
PangoAlignment
gl_view_get_default_text_alignment (glView *view)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view && GL_IS_VIEW (view), PANGO_ALIGN_LEFT);

	gl_debug (DEBUG_VIEW, "END");

	return view->default_text_alignment;
}

/****************************************************************************/
/* Get default text line spacing.                                           */
/****************************************************************************/
gdouble
gl_view_get_default_text_line_spacing (glView *view)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view && GL_IS_VIEW (view), 1.0);

	gl_debug (DEBUG_VIEW, "END");

	return view->default_text_line_spacing;
}



/****************************************************************************/
/* Get default line width.                                                  */
/****************************************************************************/
gdouble
gl_view_get_default_line_width (glView *view)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view && GL_IS_VIEW (view), 1.0);

	gl_debug (DEBUG_VIEW, "END");

	return view->default_line_width;
}


/****************************************************************************/
/* Get default line color.                                                  */
/****************************************************************************/
guint
gl_view_get_default_line_color (glView *view)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view && GL_IS_VIEW (view), 0);

	gl_debug (DEBUG_VIEW, "END");

	return view->default_line_color;
}


/****************************************************************************/
/* Get default fill color.                                                  */
/****************************************************************************/
guint
gl_view_get_default_fill_color (glView *view)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (view && GL_IS_VIEW (view), 0);

	gl_debug (DEBUG_VIEW, "END");

	return view->default_fill_color;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Focus in event handler.                                         */
/*---------------------------------------------------------------------------*/
static gboolean
focus_in_event_cb (glView            *view,
                   GdkEventFocus     *event)
{
        GTK_WIDGET_SET_FLAGS (GTK_WIDGET (view->canvas), GTK_HAS_FOCUS);

        return FALSE;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Focus out event handler.                                        */
/*---------------------------------------------------------------------------*/
static gboolean
focus_out_event_cb (glView            *view,
                    GdkEventFocus     *event)
{
        GTK_WIDGET_UNSET_FLAGS (GTK_WIDGET (view->canvas), GTK_HAS_FOCUS);

        return FALSE;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Enter notify event handler.                                     */
/*---------------------------------------------------------------------------*/
static gboolean
enter_notify_event_cb (glView            *view,
                       GdkEventCrossing  *event)
{
        gtk_widget_grab_focus(GTK_WIDGET (view->canvas));

        return FALSE;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Leave notify event handler.                                     */
/*---------------------------------------------------------------------------*/
static gboolean
leave_notify_event_cb (glView            *view,
                       GdkEventCrossing  *event)
{

        g_signal_emit (G_OBJECT(view), signals[POINTER_EXIT], 0);

        return FALSE;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Motion notify event handler.                                    */
/*---------------------------------------------------------------------------*/
static gboolean
motion_notify_event_cb (glView            *view,
                        GdkEventMotion    *event)
{
        gboolean            return_value = FALSE;
	cairo_t            *cr;
        gdouble             scale;
        gdouble             x, y;
        GdkCursor          *cursor;
        glViewObjectHandle  handle;

	cr = gdk_cairo_create (GTK_LAYOUT (view->canvas)->bin_window);

        /*
         * Translate to label coordinates
         */
        scale = view->zoom * view->home_scale;
        cairo_scale (cr, scale, scale);
        cairo_translate (cr, view->x0, view->y0);

        x = event->x;
        y = event->y;
        cairo_device_to_user (cr, &x, &y);

        /*
         * Emit signal regardless of mode
         */
        g_signal_emit (G_OBJECT(view), signals[POINTER_MOVED], 0, x, y);

        /*
         * Handle event as appropriate for mode
         */
        switch (view->mode)
        {

        case GL_VIEW_MODE_ARROW:
                switch (view->state)
                {

                case GL_VIEW_IDLE:
                        if (view_handle_at (view, cr, event->x, event->y, &handle))
                        {
                                cursor = gdk_cursor_new (GDK_CROSSHAIR);
                        }
                        else if (view_view_object_at (view, cr, event->x, event->y))
                        {
                                cursor = gdk_cursor_new (GDK_FLEUR);
                        }
                        else
                        {
                                cursor = gdk_cursor_new (GDK_LEFT_PTR);
                        }
                        gdk_window_set_cursor (view->canvas->window, cursor);
                        gdk_cursor_unref (cursor);
                        break;

                case GL_VIEW_ARROW_SELECT_REGION:
#ifdef CLIP_UPDATES                                
                        gl_view_update_region (view, cr, &view->select_region);
#endif
                        view->select_region.x2 = x;
                        view->select_region.y2 = y;
#ifdef CLIP_UPDATES                                
                        gl_view_update_region (view, cr, &view->select_region);
#else
                        gl_view_update (view);
#endif
                        break;

                case GL_VIEW_ARROW_MOVE:
                        gl_view_move_selection (view,
                                                (x - view->move_last_x),
                                                (y - view->move_last_y));
                        view->move_last_x = x;
                        view->move_last_y = y;
                        break;

                case GL_VIEW_ARROW_RESIZE:
                        gl_view_object_resize_event (view->resize_object,
                                                     view->resize_handle,
                                                     view->resize_honor_aspect,
                                                     cr,
                                                     event->x,
                                                     event->y);
                        break;

                default:
                        g_message ("Invalid arrow state.");      /*Should not happen!*/
                }
                return_value = TRUE;
                break;


        case GL_VIEW_MODE_OBJECT_CREATE:
                if (view->state != GL_VIEW_IDLE)
                {
                        switch (view->create_type)
                        {
                        case GL_LABEL_OBJECT_BOX:
                                gl_view_box_create_motion_event (view, x, y);
                                break;
                        case GL_LABEL_OBJECT_ELLIPSE:
                                gl_view_ellipse_create_motion_event (view, x, y);
                                break;
                        case GL_LABEL_OBJECT_LINE: 
                                gl_view_line_create_motion_event (view, x, y);
                                break;
                        case GL_LABEL_OBJECT_IMAGE:
                                gl_view_image_create_motion_event (view, x, y);
                                break;
                        case GL_LABEL_OBJECT_TEXT:
                                gl_view_text_create_motion_event (view, x, y);
                                break;
                        case GL_LABEL_OBJECT_BARCODE:
                                gl_view_barcode_create_motion_event (view, x, y);
                                break;
                        default:
                                g_message ("Invalid create type.");   /*Should not happen!*/
                        }
                }
                break;


        default:
                g_message ("Invalid view mode.");      /*Should not happen!*/

        }

	cairo_destroy (cr);

        /*
         * FIXME: we re-establish grabs here if the grab has been lost.  We seem to be
         *        losing grabs when we emit signals that lead to the manipulation of
         *        the GtkUIManager.  Needs more investigation
         */
        if (view->grabbed_flag && !gdk_pointer_is_grabbed ())
        {
                gdk_pointer_grab (GTK_LAYOUT (view->canvas)->bin_window,
                                  FALSE,
                                  (GDK_BUTTON1_MOTION_MASK | GDK_BUTTON_RELEASE_MASK),
                                  NULL,
                                  NULL,
                                  event->time);
        }

        return return_value;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Button press event handler.                                     */
/*---------------------------------------------------------------------------*/
static gboolean
button_press_event_cb (glView            *view,
                       GdkEventButton    *event)
{
        gboolean            return_value = FALSE;
	cairo_t            *cr;
        gdouble             scale;
        gdouble             x, y;
        glViewObject       *view_object;
        glViewObjectHandle  handle;

	cr = gdk_cairo_create (GTK_LAYOUT (view->canvas)->bin_window);

        /*
         * Translate to label coordinates
         */
        scale = view->zoom * view->home_scale;
        cairo_scale (cr, scale, scale);
        cairo_translate (cr, view->x0, view->y0);

        x = event->x;
        y = event->y;
        cairo_device_to_user (cr, &x, &y);

        switch (event->button)
        {

        case 1:
                /*
                 * Handle event as appropriate for mode
                 */
                switch (view->mode)
                {
                case GL_VIEW_MODE_ARROW:
                        if ((view_object = view_handle_at (view, cr, event->x, event->y, &handle)))
                        {
                                view->resize_object = view_object;
                                view->resize_handle = handle;
                                view->resize_honor_aspect = event->state & GDK_CONTROL_MASK;

                                view->state = GL_VIEW_ARROW_RESIZE;
                        }
                        else if ((view_object = view_view_object_at (view, cr, event->x, event->y)))
                        {
                                if (event->state & GDK_CONTROL_MASK)
                                {
                                        if (gl_view_is_object_selected (view, view_object))
                                        {
                                                /* Un-selecting a selected item */
                                                gl_view_unselect_object (view, view_object);
                                        } else {
                                                /* Add to current selection */
                                                gl_view_select_object (view, view_object);
                                        }
                                }
                                else
                                {
                                        if (!gl_view_is_object_selected (view, view_object))
                                        {
                                                /* remove any selections before adding */
                                                gl_view_unselect_all (view);
                                                /* Add to current selection */
                                                gl_view_select_object (view, view_object);
                                        }
                                }
                                view->move_last_x = x;
                                view->move_last_y = y;

                                view->state = GL_VIEW_ARROW_MOVE;
                        }
                        else
                        {
                                if (!(event->state & GDK_CONTROL_MASK))
                                {
                                        gl_view_unselect_all (view);
                                }

                                view->select_region_visible = TRUE;
                                view->select_region.x1 = x;
                                view->select_region.y1 = y;
                                view->select_region.x2 = x;
                                view->select_region.y2 = y;

                                view->state = GL_VIEW_ARROW_SELECT_REGION;
                        }


                        return_value = TRUE;
                        break;

                case GL_VIEW_MODE_OBJECT_CREATE:
                        switch (view->create_type)
                        {
                        case GL_LABEL_OBJECT_BOX:
                                gl_view_box_create_button_press_event (view, x, y);
                                break;
                        case GL_LABEL_OBJECT_ELLIPSE:
                                gl_view_ellipse_create_button_press_event (view, x, y);
                                break;
                        case GL_LABEL_OBJECT_LINE:
                                gl_view_line_create_button_press_event (view, x, y);
                                break;
                        case GL_LABEL_OBJECT_IMAGE:
                                gl_view_image_create_button_press_event (view, x, y);
                                break;
                        case GL_LABEL_OBJECT_TEXT:
                                gl_view_text_create_button_press_event (view, x, y);
                                break;
                        case GL_LABEL_OBJECT_BARCODE:
                                gl_view_barcode_create_button_press_event (view, x, y);
                                break;
                        default:
                                g_message ("Invalid create type.");   /*Should not happen!*/
                        }
                        view->state = GL_VIEW_CREATE_DRAG;
                        return_value = TRUE;
                        break;

                default:
                        g_message ("Invalid view mode.");      /*Should not happen!*/
                }

                view->grabbed_flag = TRUE;
                gdk_pointer_grab (GTK_LAYOUT (view->canvas)->bin_window,
                                  FALSE,
                                  (GDK_BUTTON1_MOTION_MASK | GDK_BUTTON_RELEASE_MASK),
                                  NULL,
                                  NULL,
                                  event->time);
                break;

        case 3:
                g_signal_emit (G_OBJECT (view),
                               signals[CONTEXT_MENU_ACTIVATE], 0,
                               event->button, event->time);
                return_value = TRUE;
                break;

        }

	cairo_destroy (cr);

        return return_value;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Button release event handler.                                   */
/*---------------------------------------------------------------------------*/
static gboolean
button_release_event_cb (glView            *view,
                         GdkEventButton    *event)
{
        gboolean     return_value = FALSE;
	cairo_t     *cr;
        gdouble      scale;
        gdouble      x, y;
        GdkCursor   *cursor;

	cr = gdk_cairo_create (GTK_LAYOUT (view->canvas)->bin_window);

        /*
         * Translate to label coordinates
         */
        scale = view->zoom * view->home_scale;
        cairo_scale (cr, scale, scale);
        cairo_translate (cr, view->x0, view->y0);

        x = event->x;
        y = event->y;
        cairo_device_to_user (cr, &x, &y);

        switch (event->button)
        {

        case 1:
                view->grabbed_flag = FALSE;
                gdk_pointer_ungrab (event->time);
                /*
                 * Handle event as appropriate for mode
                 */
                switch (view->mode)
                {
                case GL_VIEW_MODE_ARROW:
                        switch (view->state)
                        {
                        case GL_VIEW_ARROW_RESIZE:
                                view->resize_object = NULL;

                                view->state = GL_VIEW_IDLE;
                                break;

                        case GL_VIEW_ARROW_SELECT_REGION:
#ifdef CLIP_UPDATES                                
                                gl_view_update_region (view, cr, &view->select_region);
#else
                                gl_view_update (view);
#endif

                                view->select_region_visible = FALSE;
                                view->select_region.x2 = x;
                                view->select_region.y2 = y;

                                gl_view_select_region (view, &view->select_region);

                                view->state = GL_VIEW_IDLE;
                                break;

                        default:
                                view->state = GL_VIEW_IDLE;
                                break;
                                
                        }

                        return_value = TRUE;
                        break;


                case GL_VIEW_MODE_OBJECT_CREATE:
                        switch (view->create_type)
                        {
                        case GL_LABEL_OBJECT_BOX:
                                gl_view_box_create_button_release_event (view, x, y);
                                break;
                        case GL_LABEL_OBJECT_ELLIPSE:
                                gl_view_ellipse_create_button_release_event (view, x, y);
                                break;
                        case GL_LABEL_OBJECT_LINE:
                                gl_view_line_create_button_release_event (view, x, y);
                                break;
                        case GL_LABEL_OBJECT_IMAGE:
                                gl_view_image_create_button_release_event (view, x, y);
                                break;
                        case GL_LABEL_OBJECT_TEXT:
                                gl_view_text_create_button_release_event (view, x, y);
                                break;
                        case GL_LABEL_OBJECT_BARCODE:
                                gl_view_barcode_create_button_release_event (view, x, y);
                                break;
                        default:
                                g_message ("Invalid create type.");   /*Should not happen!*/
                        }
                        view->mode = GL_VIEW_MODE_ARROW;
                        view->state = GL_VIEW_IDLE;
                        cursor = gdk_cursor_new (GDK_LEFT_PTR);
                        gdk_window_set_cursor (view->canvas->window, cursor);
                        gdk_cursor_unref (cursor);
                        break;


                default:
                        g_message ("Invalid view mode.");      /*Should not happen!*/
                }

        }

	cairo_destroy (cr);

        return return_value;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Key press event handler.                                        */
/*---------------------------------------------------------------------------*/
static gboolean
key_press_event_cb (glView            *view,
                    GdkEventKey       *event)
{
        GdkCursor *cursor;

        gl_debug (DEBUG_VIEW, "");

        if ( (view->mode == GL_VIEW_MODE_ARROW) &&
             (view->state == GL_VIEW_IDLE) )
        {
                switch (event->keyval) {

                case GDK_Left:
                case GDK_KP_Left:
                        gl_view_move_selection (view, -1.0 / (view->zoom), 0.0);
                        break;
                case GDK_Up:
                case GDK_KP_Up:
                        gl_view_move_selection (view, 0.0, -1.0 / (view->zoom));
                        break;
                case GDK_Right:
                case GDK_KP_Right:
                        gl_view_move_selection (view, 1.0 / (view->zoom), 0.0);
                        break;
                case GDK_Down:
                case GDK_KP_Down:
                        gl_view_move_selection (view, 0.0, 1.0 / (view->zoom));
                        break;
                case GDK_Delete:
                case GDK_KP_Delete:
                        gl_view_delete_selection (view);
                        cursor = gdk_cursor_new (GDK_LEFT_PTR);
                        gdk_window_set_cursor (GTK_WIDGET (view->canvas)->window
, cursor);
                        gdk_cursor_unref (cursor);
                        break;
                default:
                        return FALSE;
 
               }
        }
        return TRUE;    /* We handled this or we were dragging. */
}

