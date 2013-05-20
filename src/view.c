/*
 *  view.c
 *  Copyright (C) 2001-2009  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of gLabels.
 *
 *  gLabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gLabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gLabels.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "view.h"

#include "view-box.h"
#include "view-ellipse.h"
#include "view-line.h"
#include "view-image.h"
#include "view-text.h"
#include "view-barcode.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <math.h>

#include "label.h"
#include "cairo-label-path.h"
#include "cairo-markup-path.h"
#include "color.h"
#include "prefs.h"
#include "units-util.h"
#include "marshal.h"

#include "debug.h"


/*==========================================================================*/
/* Private macros and constants.                                            */
/*==========================================================================*/

#define BG_COLOR        GL_COLOR (204, 204, 204)

#define PAPER_RGB_ARGS          1.0,   1.0,   1.0
#define SHADOW_RGB_ARGS         0.2,   0.2,   0.2
#define GRID_RGB_ARGS           0.753, 0.753, 0.753
#define MARKUP_RGB_ARGS         0.94,  0.39,  0.39
#define OUTLINE_RGB_ARGS        0.0,   0.0,   0.0
#define SELECT_LINE_RGBA_ARGS   0.0,   0.0,   1.0,   0.5
#define SELECT_FILL_RGBA_ARGS   0.75,  0.75,  1.0,   0.5

#define GRID_LINE_WIDTH_PIXELS    1.0
#define MARKUP_LINE_WIDTH_PIXELS  1.0
#define OUTLINE_WIDTH_PIXELS      1.0
#define SELECT_LINE_WIDTH_PIXELS  3.0

#define ZOOMTOFIT_PAD   16

#define SHADOW_OFFSET_PIXELS (ZOOMTOFIT_PAD/4)

#define POINTS_PER_MM    2.83464566929


/*==========================================================================*/
/* Private types.                                                           */
/*==========================================================================*/

enum {
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

static void       prefs_changed_cb                (glView         *view);

static gboolean   draw_cb                         (glView         *view,
                                                   cairo_t        *cr);

static void       realize_cb                      (glView         *view);

static void       size_allocate_cb                (glView         *view,
                                                   GtkAllocation  *allocation);

static void       screen_changed_cb               (glView         *view);

static void       label_changed_cb                (glView         *view);

static void       label_resized_cb                (glView         *view);

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

static void       set_zoom_real                   (glView         *view,
						   gdouble         zoom,
						   gboolean        scale_to_fit_flag);

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

static void       resize_event                    (glView            *view,
                                                   cairo_t           *cr,
                                                   gdouble            x,
                                                   gdouble            y);

/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glView, gl_view, GTK_TYPE_VBOX)


static void
gl_view_class_init (glViewClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	gl_debug (DEBUG_VIEW, "START");

	gl_view_parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_view_finalize;

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
        lglUnits   units;
	GtkWidget *wscroll;
	GdkColor  *bg_color;

	gl_debug (DEBUG_VIEW, "START");

        units = gl_prefs_model_get_units (gl_prefs);

	view->label                = NULL;
	view->grid_visible         = TRUE;
	view->grid_spacing         = gl_units_util_get_grid_size (units);
	view->markup_visible       = TRUE;
	view->mode                 = GL_VIEW_MODE_ARROW;
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

        gtk_widget_set_can_focus (GTK_WIDGET (view->canvas), TRUE);

        gtk_widget_add_events (GTK_WIDGET (view->canvas),
                               (GDK_FOCUS_CHANGE_MASK   |
                                GDK_ENTER_NOTIFY_MASK   |
                                GDK_LEAVE_NOTIFY_MASK   |
                                GDK_POINTER_MOTION_MASK |
                                GDK_BUTTON_PRESS_MASK   |
                                GDK_BUTTON_RELEASE_MASK |
                                GDK_KEY_PRESS_MASK));

        g_signal_connect_swapped (G_OBJECT (gl_prefs), "changed",
                                  G_CALLBACK (prefs_changed_cb), view);
	g_signal_connect_swapped (G_OBJECT (view->canvas), "draw",
				  G_CALLBACK (draw_cb), view);
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

	gl_debug (DEBUG_VIEW, "END");
}


static void
gl_view_finalize (GObject *object)
{
	glView *view = GL_VIEW (object);

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_VIEW (view));

        g_signal_handlers_disconnect_by_func (G_OBJECT (gl_prefs),
                                              G_CALLBACK (prefs_changed_cb), view);

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
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

	view->label = label;

	g_signal_connect_swapped (G_OBJECT (view->label), "selection_changed",
                                  G_CALLBACK (label_changed_cb), view);
	g_signal_connect_swapped (G_OBJECT (view->label), "changed",
                                  G_CALLBACK (label_changed_cb), view);
	g_signal_connect_swapped (G_OBJECT (view->label), "size_changed",
                                  G_CALLBACK (label_resized_cb), view);

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
/* Prefs "changed" callback.                                                 */
/*---------------------------------------------------------------------------*/
static void
prefs_changed_cb (glView         *view)
{
        lglUnits   units;

        units = gl_prefs_model_get_units (gl_prefs);
	view->grid_spacing = gl_units_util_get_grid_size (units);

        gl_view_update (view);
}


/*---------------------------------------------------------------------------*/
/* Schedule canvas update.                                                   */
/*---------------------------------------------------------------------------*/
void
gl_view_update (glView  *view)
{
        GdkWindow     *window;
        GtkAllocation  allocation;

	gl_debug (DEBUG_VIEW, "START");

        window = gtk_widget_get_window (GTK_WIDGET (view->canvas));
        
	if (window)
        {

                if ( !view->update_scheduled_flag )
                {
                        view->update_scheduled_flag = TRUE;

                        allocation.x      = 0;
                        allocation.y      = 0;
                        allocation.width  = gtk_widget_get_allocated_width (view->canvas);
                        allocation.height = gtk_widget_get_allocated_height (view->canvas);
                        gdk_window_invalidate_rect (window, &allocation, TRUE);
                }

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
        GdkWindow    *window;
	GdkRectangle  rect;
        gdouble       x, y, w, h;

	gl_debug (DEBUG_VIEW, "START");

	window = gtk_widget_get_window (GTK_WIDGET (view->canvas));

	if (!window) return;

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

        gdk_window_invalidate_rect (window, &rect, TRUE);

	gl_debug (DEBUG_VIEW, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Expose handler.                                                 */
/*---------------------------------------------------------------------------*/
static gboolean
draw_cb (glView         *view,
         cairo_t        *cr)
{
        GdkWindow *bin_window;
        cairo_t   *bin_cr;

	gl_debug (DEBUG_VIEW, "START");

        view->update_scheduled_flag = FALSE;

        bin_window = gtk_layout_get_bin_window (GTK_LAYOUT (view->canvas));
        bin_cr = gdk_cairo_create (bin_window);

	draw_layers (view, bin_cr);

        cairo_destroy (bin_cr);

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

	if (view->zoom_to_fit_flag) {
		/* Maintain best fit zoom */
		gl_view_zoom_to_fit (view);
	}

	gl_debug (DEBUG_VIEW, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE. Size allocation changed callback.                                */
/*---------------------------------------------------------------------------*/
static void
size_allocate_cb (glView         *view,
                  GtkAllocation  *allocation)
{
        GtkAdjustment *hadjustment;
        GtkAdjustment *vadjustment;

	gl_debug (DEBUG_VIEW, "START");

        hadjustment = gtk_scrollable_get_hadjustment(GTK_SCROLLABLE (view->canvas));
        vadjustment = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE (view->canvas));

        gtk_adjustment_set_page_size( hadjustment, allocation->width);
        gtk_adjustment_set_page_increment( hadjustment, allocation->width / 2);

        gtk_adjustment_set_page_size( vadjustment, allocation->height);
        gtk_adjustment_set_page_increment( vadjustment, allocation->height / 2);

        g_signal_emit_by_name (hadjustment, "changed");
        g_signal_emit_by_name (vadjustment, "changed");

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
        GtkAdjustment *hadjustment;
        GtkAdjustment *vadjustment;

	g_return_if_fail (view && GL_IS_VIEW (view));

	gl_debug (DEBUG_VIEW, "START");

        hadjustment = gtk_scrollable_get_hadjustment(GTK_SCROLLABLE (view->canvas));
        vadjustment = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE (view->canvas));

        g_signal_emit_by_name (hadjustment, "changed");
        g_signal_emit_by_name (vadjustment, "changed");

        gl_view_update (view);

	gl_debug (DEBUG_VIEW, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create, draw and order layers.                                  */
/*---------------------------------------------------------------------------*/
static void
draw_layers (glView  *view,
             cairo_t *cr)
{
        GdkWindow                 *bin_window;
	gdouble                    scale;
	gdouble                    w, h;
        gint                       canvas_w, canvas_h;

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (view->label && GL_IS_LABEL (view->label));

	gl_debug (DEBUG_VIEW, "START");

        bin_window = gtk_layout_get_bin_window (GTK_LAYOUT (view->canvas));

        scale = view->zoom * view->home_scale;

        gl_label_get_size (view->label, &w, &h);

        scale = view->home_scale * view->zoom;
        gtk_layout_set_size (GTK_LAYOUT (view->canvas), w*scale+8, h*scale+8);

        canvas_w = gdk_window_get_width (bin_window);
        canvas_h = gdk_window_get_height (bin_window);

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
        gdouble            scale;
        const lglTemplate *template;
        gboolean           rotate_flag;

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (view->label && GL_IS_LABEL (view->label));

        scale = view->home_scale * view->zoom;

        template    = gl_label_get_template (view->label);
        rotate_flag = gl_label_get_rotate_flag (view->label);

        cairo_save (cr);
        cairo_translate (cr, SHADOW_OFFSET_PIXELS/scale, SHADOW_OFFSET_PIXELS/scale);
        gl_cairo_label_path (cr, template, rotate_flag, FALSE);
        cairo_set_source_rgb (cr, SHADOW_RGB_ARGS);
        cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
        cairo_fill (cr);
        cairo_restore (cr);

        gl_cairo_label_path (cr, template, rotate_flag, FALSE);
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
        const lglTemplate         *template;
        gboolean                   rotate_flag;
	const lglTemplateFrame    *frame;
	gdouble                    w, h;
	gdouble                    x, y;
	gdouble                    x0, y0;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (view->label && GL_IS_LABEL(view->label));

        if (view->grid_visible)
        {

                template    = gl_label_get_template (view->label);
                rotate_flag = gl_label_get_rotate_flag (view->label);
                frame       = (lglTemplateFrame *)template->frames->data;

                gl_label_get_size (view->label, &w, &h);
	
                if (frame->shape == LGL_TEMPLATE_FRAME_SHAPE_RECT) {
                        x0 = view->grid_spacing;
                        y0 = view->grid_spacing;
                } else {
                        /* round labels, adjust grid to line up with center of label. */
                        x0 = fmod (w/2.0, view->grid_spacing);
                        y0 = fmod (h/2.0, view->grid_spacing);
                }


                cairo_save (cr);

                gl_cairo_label_path (cr, template, rotate_flag, FALSE);
                cairo_clip (cr);

                cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
                cairo_set_line_width (cr, GRID_LINE_WIDTH_PIXELS/(view->home_scale * view->zoom));
                cairo_set_source_rgb (cr, GRID_RGB_ARGS);

                for ( x=x0; x < w; x += view->grid_spacing )
                {
                        cairo_move_to (cr, x, 0);
                        cairo_line_to (cr, x, h);
                        cairo_stroke (cr);
                }

                for ( y=y0; y < h; y += view->grid_spacing )
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
	const lglTemplate         *template;
	const lglTemplateFrame    *frame;
        gboolean                   rotate_flag;
	GList                     *p;
	lglTemplateMarkup         *markup;
        gdouble                    width, height;

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (view->label && GL_IS_LABEL (view->label));

        if (view->markup_visible)
        {

                template    = gl_label_get_template (view->label);
                frame       = (lglTemplateFrame *)template->frames->data;
                rotate_flag = gl_label_get_rotate_flag (view->label);

                cairo_save (cr);

                if (rotate_flag)
                {
                        lgl_template_frame_get_size (frame, &width, &height);
                        cairo_rotate (cr, -G_PI/2.0);
                        cairo_translate (cr, -width, 0.0);
                }

                cairo_set_line_width (cr, MARKUP_LINE_WIDTH_PIXELS/(view->home_scale * view->zoom));
                cairo_set_source_rgb (cr, MARKUP_RGB_ARGS);

                for ( p=frame->all.markups; p != NULL; p=p->next )
                {
                        markup = (lglTemplateMarkup *)p->data;

                        gl_cairo_markup_path (cr, markup, view->label);

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
        const lglTemplate *template;
        gboolean           rotate_flag;

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (view->label && GL_IS_LABEL (view->label));

        template    = gl_label_get_template (view->label);
        rotate_flag = gl_label_get_rotate_flag (view->label);

        gl_cairo_label_path (cr, template, rotate_flag, FALSE);

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
	GList            *selection_list;
	GList            *p_obj;
	glLabelObject    *object;

	g_return_if_fail (view && GL_IS_VIEW (view));

        cairo_save (cr);

        cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

        selection_list = gl_label_get_selection_list (view->label);

	for (p_obj = selection_list; p_obj != NULL; p_obj = p_obj->next)
        {
		object = GL_LABEL_OBJECT (p_obj->data);

                gl_label_object_draw_handles (object, cr);
	}

        g_list_free (selection_list);

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
        GdkWindow *window;
	GdkCursor *cursor;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

        window = gtk_widget_get_window (view->canvas);

        cursor = gdk_cursor_new (GDK_LEFT_PTR);
	gdk_window_set_cursor (window, cursor);
        g_object_unref (G_OBJECT (cursor));

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
        GdkWindow *window;
	GdkCursor *cursor = NULL;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));

        window = gtk_widget_get_window (view->canvas);

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

	gdk_window_set_cursor (window, cursor);
        g_object_unref (G_OBJECT (cursor));

	view->mode = GL_VIEW_MODE_OBJECT_CREATE;
        view->state = GL_VIEW_IDLE;
	view->create_type = type;

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
        GtkAllocation  allocation;
	gint           w_view, h_view;
	gdouble        w_label, h_label;
	gdouble        x_scale, y_scale, scale;

	gl_debug (DEBUG_VIEW, "");

	if ( ! gtk_widget_get_window (GTK_WIDGET (view)) ) {
                /* Delay until realized. */
                view->zoom_to_fit_flag = TRUE;
		return;
	}

        gtk_widget_get_allocation (GTK_WIDGET (view), &allocation);
	w_view = allocation.width;
	h_view = allocation.height;

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
/* PRIVATE.  Focus in event handler.                                         */
/*---------------------------------------------------------------------------*/
static gboolean
focus_in_event_cb (glView            *view,
                   GdkEventFocus     *event)
{
        return FALSE;
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Focus out event handler.                                        */
/*---------------------------------------------------------------------------*/
static gboolean
focus_out_event_cb (glView            *view,
                    GdkEventFocus     *event)
{
        return FALSE;
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Enter notify event handler.                                     */
/*---------------------------------------------------------------------------*/
static gboolean
enter_notify_event_cb (glView            *view,
                       GdkEventCrossing  *event)
{
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
        GdkWindow          *bin_window;
        GdkWindow          *window;
	cairo_t            *cr;
        gdouble             scale;
        gdouble             x, y;
        GdkCursor          *cursor;
        glLabelObjectHandle handle;

        bin_window = gtk_layout_get_bin_window (GTK_LAYOUT (view->canvas));
        window = gtk_widget_get_window (view->canvas);

	cr = gdk_cairo_create (bin_window);

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
                        if ( gl_label_is_selection_atomic (view->label) &&
                             gl_label_get_handle_at (view->label, cr, event->x, event->y, &handle) )
                        {
                                cursor = gdk_cursor_new (GDK_CROSSHAIR);
                        }
                        else if (gl_label_object_at (view->label, cr, event->x, event->y))
                        {
                                cursor = gdk_cursor_new (GDK_FLEUR);
                        }
                        else
                        {
                                cursor = gdk_cursor_new (GDK_LEFT_PTR);
                        }
                        gdk_window_set_cursor (window, cursor);
                        g_object_unref (G_OBJECT (cursor));
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
                        gl_label_move_selection (view->label,
                                                (x - view->move_last_x),
                                                (y - view->move_last_y));
                        view->move_last_x = x;
                        view->move_last_y = y;
                        break;

                case GL_VIEW_ARROW_RESIZE:
                        resize_event (view, cr, event->x, event->y);
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
                gdk_pointer_grab (bin_window,
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
        GdkWindow          *bin_window;
	cairo_t            *cr;
        gdouble             scale;
        gdouble             x, y;
        glLabelObject      *object;
        glLabelObjectHandle handle;

        gtk_widget_grab_focus(GTK_WIDGET (view->canvas));

        bin_window = gtk_layout_get_bin_window (GTK_LAYOUT (view->canvas));

	cr = gdk_cairo_create (bin_window);

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
                        if ( gl_label_is_selection_atomic (view->label) &&
                             (object = gl_label_get_handle_at (view->label, cr, event->x, event->y, &handle)) )
                        {
                                view->resize_object = object;
                                view->resize_handle = handle;
                                view->resize_honor_aspect = event->state & GDK_CONTROL_MASK;

                                view->state = GL_VIEW_ARROW_RESIZE;
                        }
                        else if ((object = gl_label_object_at (view->label, cr, event->x, event->y)))
                        {
                                if (event->state & GDK_CONTROL_MASK)
                                {
                                        if (gl_label_object_is_selected (object))
                                        {
                                                /* Un-selecting a selected item */
                                                gl_label_unselect_object (view->label, object);
                                        } else {
                                                /* Add to current selection */
                                                gl_label_select_object (view->label, object);
                                        }
                                }
                                else
                                {
                                        if (!gl_label_object_is_selected (object))
                                        {
                                                /* remove any selections before adding */
                                                gl_label_unselect_all (view->label);
                                                /* Add to current selection */
                                                gl_label_select_object (view->label, object);
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
                                        gl_label_unselect_all (view->label);
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
                gdk_pointer_grab (bin_window,
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
        GdkWindow   *bin_window;
        GdkWindow   *window;
	cairo_t     *cr;
        gdouble      scale;
        gdouble      x, y;
        GdkCursor   *cursor;

        bin_window = gtk_layout_get_bin_window (GTK_LAYOUT (view->canvas));
        window = gtk_widget_get_window (view->canvas);

	cr = gdk_cairo_create (bin_window);

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

                                gl_label_select_region (view->label, &view->select_region);

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
                        gdk_window_set_cursor (window, cursor);
                        g_object_unref (G_OBJECT (cursor));

                        gl_label_select_object (view->label, view->create_object);
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
        GdkWindow *window;
        GdkCursor *cursor;

        gl_debug (DEBUG_VIEW, "");

        window = gtk_widget_get_window (view->canvas);

        if ( (view->mode == GL_VIEW_MODE_ARROW) &&
             (view->state == GL_VIEW_IDLE) )
        {
                switch (event->keyval) {

                case GDK_KEY_Left:
                case GDK_KEY_KP_Left:
                        gl_label_move_selection (view->label, -1.0 / (view->zoom), 0.0);
                        break;
                case GDK_KEY_Up:
                case GDK_KEY_KP_Up:
                        gl_label_move_selection (view->label, 0.0, -1.0 / (view->zoom));
                        break;
                case GDK_KEY_Right:
                case GDK_KEY_KP_Right:
                        gl_label_move_selection (view->label, 1.0 / (view->zoom), 0.0);
                        break;
                case GDK_KEY_Down:
                case GDK_KEY_KP_Down:
                        gl_label_move_selection (view->label, 0.0, 1.0 / (view->zoom));
                        break;
                case GDK_KEY_Delete:
                case GDK_KEY_KP_Delete:
                        gl_label_delete_selection (view->label);
                        cursor = gdk_cursor_new (GDK_LEFT_PTR);
                        gdk_window_set_cursor (window, cursor);
                        g_object_unref (G_OBJECT (cursor));
                        break;
                default:
                        return FALSE;
 
               }
        }
        return TRUE;    /* We handled this or we were dragging. */
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Resize object.                                                  */
/*---------------------------------------------------------------------------*/
static void
resize_event (glView             *view,
              cairo_t            *cr,
              gdouble             x,
              gdouble             y)
{
        cairo_matrix_t matrix;
        gdouble        x0, y0, x1, y1, x2, y2;
        gdouble        w, h;
        gdouble        dx=0, dy=0;

	gl_debug (DEBUG_VIEW, "x,y world = %g, %g", x, y);

        /*
         * Change to item relative coordinates
         */
        cairo_save (cr);
        gl_label_object_get_position (view->resize_object, &x0, &y0);
        cairo_translate (cr, x0, y0);
        gl_label_object_get_matrix (view->resize_object, &matrix);
        cairo_transform (cr, &matrix);

        /*
         * Initialize origin and 2 corners in object relative coordinates.
         */
        x0 = 0.0;
        y0 = 0.0;

        x1 = 0.0;
        y1 = 0.0;

        gl_label_object_get_size (view->resize_object, &x2, &y2);

	gl_debug (DEBUG_VIEW, "x0,y0 object = %g, %g", x0, y0);
	gl_debug (DEBUG_VIEW, "x1,y1 object = %g, %g", x1, y1);
	gl_debug (DEBUG_VIEW, "x2,y2 object = %g, %g", x2, y2);

        /*
         * Translate x,y into object relative coordinates.
         */
        cairo_device_to_user (cr, &x, &y);

	gl_debug (DEBUG_VIEW, "x,y object = %g, %g", x, y);
        
        /*
         * Get new size
         */
        switch (view->resize_handle)
        {

        case GL_LABEL_OBJECT_HANDLE_NW:
                w = MAX (x2 - x, 0);
                h = MAX (y2 - y, 0);
                break;

        case GL_LABEL_OBJECT_HANDLE_N:
                w = x2 - x1;
                h = MAX (y2 - y, 0);
                break;

        case GL_LABEL_OBJECT_HANDLE_NE:
                w = MAX (x - x1, 0);
                h = MAX (y2 - y, 0);
                break;

        case GL_LABEL_OBJECT_HANDLE_E:
                w = MAX (x - x1, 0);
                h = y2 - y1;
                break;

        case GL_LABEL_OBJECT_HANDLE_SE:
                w = MAX (x - x1, 0);
                h = MAX (y - y1, 0);
                break;

        case GL_LABEL_OBJECT_HANDLE_S:
                w = x2 - x1;
                h = MAX (y - y1, 0);
                break;

        case GL_LABEL_OBJECT_HANDLE_SW:
                w = MAX (x2 - x, 0);
                h = MAX (y - y1, 0);
                break;

        case GL_LABEL_OBJECT_HANDLE_W:
                w = MAX (x2 - x, 0);
                h = y2 - y1;
                break;
        case GL_LABEL_OBJECT_HANDLE_P1:
                x1 = x;
                y1 = y;
                dx = (x2 - x);
                dy = (y2 - y);
                x0 = x0 + x1;
                y0 = y0 + y1;
                break;

        case GL_LABEL_OBJECT_HANDLE_P2:
                dx = x - x1;
                dy = y - y1;
                x0 = x0 + x1;
                y0 = y0 + y1;
                break;

        default:
                g_print ("Invalid handle.\n");  /* Should not happen! */

        }

        if ( (view->resize_handle != GL_LABEL_OBJECT_HANDLE_P1) &&
             (view->resize_handle != GL_LABEL_OBJECT_HANDLE_P2) )
        {
                if ( view->resize_honor_aspect )
                {
                        gl_label_object_set_size_honor_aspect (view->resize_object, w, h, TRUE);
                }
                else
                {
                        gl_label_object_set_size (view->resize_object, w, h, TRUE);
                }

                /*
                 * Query the new size in case it was constrained.
                 */
                gl_label_object_get_size (view->resize_object, &w, &h);

                /*
                 * Get new position
                 */
                switch (view->resize_handle)
                {

                case GL_LABEL_OBJECT_HANDLE_NW:
                        x0 += x2 - w;
                        y0 += y2 - h;
                        break;

                case GL_LABEL_OBJECT_HANDLE_N:
                case GL_LABEL_OBJECT_HANDLE_NE:
                        /* x unchanged */
                        y0 += y2 - h;
                        break;

                case GL_LABEL_OBJECT_HANDLE_E:
                case GL_LABEL_OBJECT_HANDLE_SE:
                case GL_LABEL_OBJECT_HANDLE_S:
                        /* unchanged */
                        break;

                case GL_LABEL_OBJECT_HANDLE_SW:
                case GL_LABEL_OBJECT_HANDLE_W:
                        x0 += x2 - w;
                        /* y unchanged */
                        break;

                default:
                        g_print ("Invalid handle.\n");  /* Should not happen! */
                }
        }
        else
        {
                gl_label_object_set_size (view->resize_object, dx, dy, TRUE);
        }

        /*
         * Put new origin back into world coordinates and set.
         */
        cairo_user_to_device (cr, &x0, &y0);
        cairo_restore (cr);
        cairo_device_to_user (cr, &x0, &y0);
        gl_label_object_set_position (view->resize_object, x0, y0, FALSE);
}




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
