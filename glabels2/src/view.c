/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  view.c:  GLabels View module
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

#include <gtk/gtk.h>
#include <gtk/gtkinvisible.h>

#include <string.h>
#include <math.h>

#include "view.h"
#include "view-object.h"
#include "view-box.h"
#include "view-ellipse.h"
#include "view-line.h"
#include "view-image.h"
#include "view-text.h"
#include "view-barcode.h"
#include "xml-label.h"
#include "color.h"
#include "marshal.h"

#include "debug.h"

/*==========================================================================*/
/* Private macros and constants.                                            */
/*==========================================================================*/

#define BG_COLOR        GL_COLOR (192, 192, 192)
#define OUTLINE_COLOR   GL_COLOR (173, 216, 230)
#define PAPER_COLOR     GL_COLOR (255, 255, 255)
#define GRID_COLOR      BG_COLOR
#define MARKUP_COLOR    GL_COLOR (240, 100, 100)

#define SEL_LINE_COLOR  GL_COLOR_A (0, 0, 255, 128)
#define SEL_FILL_COLOR  GL_COLOR_A (192, 192, 255, 128)

/*==========================================================================*/
/* Private types.                                                           */
/*==========================================================================*/

enum {
	SELECTION_CHANGED,
	ZOOM_CHANGED,
	POINTER_MOVED,
	POINTER_EXIT,
	MODE_CHANGED,
	LAST_SIGNAL
};


/*==========================================================================*/
/* Private globals                                                          */
/*==========================================================================*/

static GtkContainerClass *parent_class;

static guint signals[LAST_SIGNAL] = {0};

/* "CLIPBOARD" selection */
static GdkAtom clipboard_atom = GDK_NONE;

static gdouble scales[] = {
	8.0, 6.0, 4.0, 3.0,
	2.0,
	1.5, 1.0, 0.5, 0.25,
};
#define N_SCALES G_N_ELEMENTS(scales)
#define HOME_SCALE 2.0

/*==========================================================================*/
/* Local function prototypes                                                */
/*==========================================================================*/

static void       gl_view_class_init              (glViewClass *class);
static void       gl_view_init                    (glView *view);
static void       gl_view_finalize                (GObject *object);

static void       gl_view_construct               (glView *view);
static GtkWidget *gl_view_construct_canvas        (glView *view);
static void       gl_view_construct_selection     (glView *view);

static gdouble    get_apropriate_scale            (gdouble w, gdouble h);

static void       draw_layers                     (glView *view);

static void       draw_label_layer                (glView *view);

static void       draw_highlight_layer            (glView *view);

static void       draw_bg_fg_layers               (glView *view);
static void       draw_bg_fg_rect                 (glView *view);
static void       draw_bg_fg_rounded_rect         (glView *view);
static void       draw_bg_fg_round                (glView *view);
static void       draw_bg_fg_cd                   (glView *view);

static void       draw_grid_layer                 (glView *view);

static void       draw_markup_layer               (glView *view);

static void       draw_markup_margin              (glView *view,
						   glTemplateMarkupMargin *margin);
static void       draw_markup_margin_rect         (glView *view,
						   glTemplateMarkupMargin *margin);
static void       draw_markup_margin_rounded_rect (glView *view,
						   glTemplateMarkupMargin *margin);
static void       draw_markup_margin_round        (glView *view,
						   glTemplateMarkupMargin *margin);
static void       draw_markup_margin_cd           (glView *view,
						   glTemplateMarkupMargin *margin);

static void       draw_markup_line                (glView *view,
						   glTemplateMarkupLine   *line);


static void       select_object_real              (glView *view,
						   glViewObject *view_object);
static void       unselect_object_real            (glView *view,
						   glViewObject *view_object);

static gboolean   object_at                       (glView *view,
						   gdouble x, gdouble y);

static gboolean   is_item_member_of_group         (glView          *view,
						   GnomeCanvasItem *item,
						   GnomeCanvasItem *group);

static gboolean   is_object_selected              (glView *view,
						   glViewObject *view_object);

static void       move_selection                  (glView *view,
						   gdouble dx, gdouble dy);

static int        canvas_event                    (GnomeCanvas *canvas,
						   GdkEvent    *event,
						   glView      *view);
static int        canvas_event_arrow_mode         (GnomeCanvas *canvas,
						   GdkEvent    *event,
						   glView      *view);

static int        item_event_arrow_mode          (GnomeCanvasItem *item,
						  GdkEvent        *event,
						  glViewObject    *view_object);

static GtkWidget *new_selection_menu             (glView *view);

static void       popup_selection_menu           (glView       *view,
						  glViewObject *view_object,
						  GdkEvent     *event);

static void       selection_clear_cb             (GtkWidget         *widget,
						  GdkEventSelection *event,
						  gpointer          data);

static void       selection_get_cb               (GtkWidget         *widget,
						  GtkSelectionData  *selection_data,
						  guint             info,
						  guint             time,
						  gpointer          data);

static void       selection_received_cb          (GtkWidget         *widget,
						  GtkSelectionData  *selection_data,
						  guint             time,
						  gpointer          data);

/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
guint
gl_view_get_type (void)
{
	static guint view_type = 0;

	if (!view_type) {
		GTypeInfo view_info = {
			sizeof (glViewClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_view_class_init,
			NULL,
			NULL,
			sizeof (glView),
			0,
			(GInstanceInitFunc) gl_view_init,
		};

		view_type =
		    g_type_register_static (gtk_vbox_get_type (),
					    "glView", &view_info, 0);
	}

	return view_type;
}

static void
gl_view_class_init (glViewClass *class)
{
	GObjectClass *object_class = (GObjectClass *) class;

	gl_debug (DEBUG_VIEW, "START");

	parent_class = g_type_class_peek_parent (class);

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
	gl_debug (DEBUG_VIEW, "START");

	view->label = NULL;

	view->grid_spacing = 9;

	gl_debug (DEBUG_VIEW, "END");
}

static void
gl_view_finalize (GObject *object)
{
	glView *view;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_VIEW (object));

	view = GL_VIEW (object);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_VIEW, "END");
}

/****************************************************************************/
/* NEW view object.                                                         */
/****************************************************************************/
GtkWidget *
gl_view_new (glLabel *label)
{
	glView *view = g_object_new (gl_view_get_type (), NULL);

	gl_debug (DEBUG_VIEW, "START");

	view->label = label;

	gl_view_construct (view);

	gl_debug (DEBUG_VIEW, "END");

	return GTK_WIDGET (view);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                     */
/*---------------------------------------------------------------------------*/
static void
gl_view_construct (glView *view)
{
	GtkWidget *wvbox, *wscroll;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

	wvbox = GTK_WIDGET (view);

	view->state = GL_VIEW_STATE_ARROW;
	view->object_list = NULL;

	gl_view_construct_canvas (view);
	wscroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (wscroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (wvbox), wscroll, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (wscroll), view->canvas);

	gl_view_construct_selection (view);

	view->menu = new_selection_menu (view);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create canvas w/ a background in the shape of the label/card.   */
/*---------------------------------------------------------------------------*/
static GtkWidget *
gl_view_construct_canvas (glView *view)
{
	gdouble   scale;
	glLabel  *label = view->label;
	gdouble   label_width, label_height;
	GdkColor *bg_color;

	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (GL_IS_VIEW (view), NULL);
	g_return_val_if_fail (label != NULL, NULL);

	gtk_widget_push_colormap (gdk_rgb_get_colormap ());
	view->canvas = gnome_canvas_new_aa ();
	gtk_widget_pop_colormap ();

	bg_color = gl_color_to_gdk_color (BG_COLOR);
	gtk_widget_modify_bg (GTK_WIDGET(view->canvas), GTK_STATE_NORMAL, bg_color);
	g_free (bg_color);

	gl_label_get_size (label, &label_width, &label_height);
	gl_debug (DEBUG_VIEW, "Label size: w=%lf, h=%lf",
		  label_width, label_height);

	scale = get_apropriate_scale (label_width, label_height);
	gl_debug (DEBUG_VIEW, "scale =%lf", scale);

	gl_debug (DEBUG_VIEW, "Canvas size: w=%lf, h=%lf",
			      scale * label_width + 40,
			      scale * label_height + 40);
	gtk_widget_set_size_request (GTK_WIDGET(view->canvas),
				     scale * label_width + 40,
				     scale * label_height + 40);
	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (view->canvas),
					  scale);
	view->scale = scale;

	gnome_canvas_set_scroll_region (GNOME_CANVAS (view->canvas),
					0.0, 0.0, label_width, label_height);

	draw_layers (view);

	g_signal_connect (G_OBJECT (view->canvas), "event",
			  G_CALLBACK (canvas_event), view);

	gl_debug (DEBUG_VIEW, "END");

	return view->canvas;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create clipboard selection targets.                             */
/*---------------------------------------------------------------------------*/
static void
gl_view_construct_selection (glView *view)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

	view->have_selection = FALSE;
	view->selection_data = NULL;
	view->invisible = gtk_invisible_new ();

	view->selected_object_list = NULL;

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

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Determine an apropriate scale for given label & screen size     */
/*---------------------------------------------------------------------------*/
static gdouble
get_apropriate_scale (gdouble w, gdouble h)
{
	gdouble w_screen, h_screen;
	gint i;
	gdouble k;

	gl_debug (DEBUG_VIEW, "");

	w_screen = (gdouble) gdk_screen_width ();
	h_screen = (gdouble) gdk_screen_height ();

	for (i = 0; i < N_SCALES; i++) {
		k = scales[i];
		if (k <= HOME_SCALE) {
			if ((k * w < (w_screen - 256))
			    && (k * h < (h_screen - 256)))
				return k;
		}
	}

	return 0.25;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create, draw and order layers.                                  */
/*---------------------------------------------------------------------------*/
static void
draw_layers (glView *view)
{
	draw_bg_fg_layers (view);
	draw_grid_layer (view);
	draw_markup_layer (view);
	draw_highlight_layer (view); /* Must be done before label layer */
	draw_label_layer (view);

	gnome_canvas_item_raise_to_top (GNOME_CANVAS_ITEM(view->fg_group));
	gnome_canvas_item_raise_to_top (GNOME_CANVAS_ITEM(view->highlight_group));
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw label layer.                                               */
/*---------------------------------------------------------------------------*/
static void
draw_label_layer (glView *view)
{
	GnomeCanvasGroup *group;
	glLabel          *label;
	GList            *p_obj;
	glLabelObject    *object;
	glViewObject     *view_object;

	group = gnome_canvas_root (GNOME_CANVAS (view->canvas));
	view->label_group = GNOME_CANVAS_GROUP(
		gnome_canvas_item_new (group,
				       gnome_canvas_group_get_type (),
				       "x", 0.0,
				       "y", 0.0,
				       NULL));

	label = view->label;

	for (p_obj = label->objects; p_obj != NULL; p_obj = p_obj->next) {
		object = (glLabelObject *) p_obj->data;

		if (GL_IS_LABEL_BOX (object)) {
			view_object = gl_view_box_new (GL_LABEL_BOX(object),
						       view);
		} else if (GL_IS_LABEL_ELLIPSE (object)) {
			view_object = gl_view_ellipse_new (GL_LABEL_ELLIPSE(object),
							   view);
		} else if (GL_IS_LABEL_LINE (object)) {
			view_object = gl_view_line_new (GL_LABEL_LINE(object),
							view);
		} else if (GL_IS_LABEL_IMAGE (object)) {
			view_object = gl_view_image_new (GL_LABEL_IMAGE(object),
							 view);
		} else if (GL_IS_LABEL_TEXT (object)) {
			view_object = gl_view_text_new (GL_LABEL_TEXT(object),
							view);
		} else if (GL_IS_LABEL_BARCODE (object)) {
			view_object = gl_view_barcode_new (GL_LABEL_BARCODE(object),
							   view);
		} else {
			/* Should not happen! */
			view_object = NULL;
			g_warning ("Invalid label object type.");
		}
	}
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create highlight layer.                                         */
/*---------------------------------------------------------------------------*/
static void
draw_highlight_layer (glView *view)
{
	GnomeCanvasGroup *group;

	group = gnome_canvas_root (GNOME_CANVAS (view->canvas));
	view->highlight_group = GNOME_CANVAS_GROUP(
		gnome_canvas_item_new (group,
				       gnome_canvas_group_get_type (),
				       "x", 0.0,
				       "y", 0.0,
				       NULL));
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw background and foreground outlines.                        */
/*---------------------------------------------------------------------------*/
static void
draw_bg_fg_layers (glView *view)
{
	glLabel          *label;
	glTemplate       *template;
	GnomeCanvasGroup *group;

	group = gnome_canvas_root (GNOME_CANVAS (view->canvas));
	view->bg_group = GNOME_CANVAS_GROUP(
		gnome_canvas_item_new (group,
				       gnome_canvas_group_get_type (),
				       "x", 0.0,
				       "y", 0.0,
				       NULL));
	view->fg_group = GNOME_CANVAS_GROUP(
		gnome_canvas_item_new (group,
				       gnome_canvas_group_get_type (),
				       "x", 0.0,
				       "y", 0.0,
				       NULL));

	label = view->label;
	template = gl_label_get_template (label);

	switch (template->label.style) {

	case GL_TEMPLATE_STYLE_RECT:
		if (template->label.rect.r == 0.0) {
			/* Square corners. */
			draw_bg_fg_rect (view);
		} else {
			/* Rounded corners. */
			draw_bg_fg_rounded_rect (view);
		}
		break;

	case GL_TEMPLATE_STYLE_ROUND:
		draw_bg_fg_round (view);
		break;

	case GL_TEMPLATE_STYLE_CD:
		draw_bg_fg_cd (view);
		break;

	default:
		g_warning ("Unknown template label style");
		break;
	}
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw simple recangular background.                              */
/*---------------------------------------------------------------------------*/
static void
draw_bg_fg_rect (glView *view)
{
	glLabel          *label = view->label;
	glTemplate       *template;
	gdouble           w, h;
	GnomeCanvasItem  *item;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));
	g_return_if_fail (label != NULL);

	gl_label_get_size (label, &w, &h);
	template = gl_label_get_template (label);

	/* Background */
	item = gnome_canvas_item_new (view->bg_group,
				      gnome_canvas_rect_get_type (),
				      "x1", 0.0,
				      "y1", 0.0,
				      "x2", w,
				      "y2", h,
				      "fill_color_rgba", PAPER_COLOR,
				      NULL);

	/* Foreground */
	item = gnome_canvas_item_new (view->fg_group,
				      gnome_canvas_rect_get_type (),
				      "x1", 0.0,
				      "y1", 0.0,
				      "x2", w,
				      "y2", h,
				      "width_pixels", 2,
				      "outline_color_rgba", OUTLINE_COLOR,
				      NULL);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw rounded recangular background.                             */
/*---------------------------------------------------------------------------*/
static void
draw_bg_fg_rounded_rect (glView *view)
{
	glLabel           *label = view->label;
	GnomeCanvasPoints *points;
	gint               i_coords, i_theta;
	glTemplate        *template;
	gdouble            r, w, h;
	GnomeCanvasItem   *item;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));
	g_return_if_fail (label != NULL);

	gl_label_get_size (label, &w, &h);
	template = gl_label_get_template (label);
	r = template->label.rect.r;

	points = gnome_canvas_points_new (4 * (1 + 90 / 5));
	i_coords = 0;
	for (i_theta = 0; i_theta <= 90; i_theta += 5) {
		points->coords[i_coords++] =
		    r - r * sin (i_theta * M_PI / 180.0);
		points->coords[i_coords++] =
		    r - r * cos (i_theta * M_PI / 180.0);
	}
	for (i_theta = 0; i_theta <= 90; i_theta += 5) {
		points->coords[i_coords++] =
		    r - r * cos (i_theta * M_PI / 180.0);
		points->coords[i_coords++] =
		    (h - r) + r * sin (i_theta * M_PI / 180.0);
	}
	for (i_theta = 0; i_theta <= 90; i_theta += 5) {
		points->coords[i_coords++] =
		    (w - r) + r * sin (i_theta * M_PI / 180.0);
		points->coords[i_coords++] =
		    (h - r) + r * cos (i_theta * M_PI / 180.0);
	}
	for (i_theta = 0; i_theta <= 90; i_theta += 5) {
		points->coords[i_coords++] =
		    (w - r) + r * cos (i_theta * M_PI / 180.0);
		points->coords[i_coords++] =
		    r - r * sin (i_theta * M_PI / 180.0);
	}

	/* Background */
	item = gnome_canvas_item_new (view->bg_group,
				      gnome_canvas_polygon_get_type (),
				      "points", points,
				      "fill_color_rgba", PAPER_COLOR,
				      NULL);

	/* Foreground */
	item = gnome_canvas_item_new (view->fg_group,
				      gnome_canvas_polygon_get_type (),
				      "points", points,
				      "width_pixels", 2,
				      "outline_color_rgba", OUTLINE_COLOR,
				      NULL);

	gnome_canvas_points_free (points);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw round background.                                          */
/*---------------------------------------------------------------------------*/
static void
draw_bg_fg_round (glView *view)
{
	glLabel          *label = view->label;
	glTemplate       *template;
	gdouble           r;
	GnomeCanvasItem  *item;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));
	g_return_if_fail (label != NULL);

	template = gl_label_get_template (label);

	r = template->label.round.r;

	/* Background */
	item = gnome_canvas_item_new (view->bg_group,
				      gnome_canvas_ellipse_get_type (),
				      "x1", 0.0,
				      "y1", 0.0,
				      "x2", 2.0*r,
				      "y2", 2.0*r,
				      "fill_color_rgba", PAPER_COLOR,
				      NULL);

	/* Foreground */
	item = gnome_canvas_item_new (view->fg_group,
				      gnome_canvas_ellipse_get_type (),
				      "x1", 0.0,
				      "y1", 0.0,
				      "x2", 2.0*r,
				      "y2", 2.0*r,
				      "width_pixels", 2,
				      "outline_color_rgba", OUTLINE_COLOR,
				      NULL);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw CD style background, circular w/ concentric hole.          */
/*---------------------------------------------------------------------------*/
static void
draw_bg_fg_cd (glView *view)
{
	glLabel          *label = view->label;
	glTemplate       *template;
	gdouble           r1, r2;
	GnomeCanvasItem  *item;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));
	g_return_if_fail (label != NULL);

	template = gl_label_get_template (label);

	r1 = template->label.cd.r1;
	r2 = template->label.cd.r2;

	/* Background */
	/* outer circle */
	item = gnome_canvas_item_new (view->bg_group,
				      gnome_canvas_ellipse_get_type (),
				      "x1", 0.0,
				      "y1", 0.0,
				      "x2", 2.0*r1,
				      "y2", 2.0*r1,
				      "fill_color_rgba", PAPER_COLOR,
				      NULL);
	/* hole */
	item = gnome_canvas_item_new (view->bg_group,
				      gnome_canvas_ellipse_get_type (),
				      "x1", r1 - r2,
				      "y1", r1 - r2,
				      "x2", r1 + r2,
				      "y2", r1 + r2,
				      "fill_color_rgba", GRID_COLOR,
				      NULL);

	/* Foreground */
	/* outer circle */
	item = gnome_canvas_item_new (view->fg_group,
				      gnome_canvas_ellipse_get_type (),
				      "x1", 0.0,
				      "y1", 0.0,
				      "x2", 2.0*r1,
				      "y2", 2.0*r1,
				      "width_pixels", 2,
				      "outline_color_rgba", OUTLINE_COLOR,
				      NULL);
	/* hole */
	item = gnome_canvas_item_new (view->fg_group,
				      gnome_canvas_ellipse_get_type (),
				      "x1", r1 - r2,
				      "y1", r1 - r2,
				      "x2", r1 + r2,
				      "y2", r1 + r2,
				      "width_pixels", 2,
				      "outline_color_rgba", OUTLINE_COLOR,
				      NULL);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw grid lines.                                                */
/*---------------------------------------------------------------------------*/
static void
draw_grid_layer (glView *view)
{
	gdouble            w, h, x, y;
	GnomeCanvasPoints *points;
	GnomeCanvasItem  *item;
	GnomeCanvasGroup *group;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (view && GL_IS_VIEW (view));
	g_return_if_fail (view->label && GL_IS_LABEL(view->label));

	gl_label_get_size (view->label, &w, &h);

	group = gnome_canvas_root (GNOME_CANVAS (view->canvas));
	view->grid_group = GNOME_CANVAS_GROUP(
		gnome_canvas_item_new (group,
				       gnome_canvas_group_get_type (),
				       "x", 0.0,
				       "y", 0.0,
				       NULL));
	points = gnome_canvas_points_new (2);

	points->coords[1] = 0.0;
	points->coords[3] = h;
	for ( x=0.0; x < w; x += view->grid_spacing ) {
		points->coords[0] = points->coords[2] = x;
		item = gnome_canvas_item_new (view->grid_group,
					      gnome_canvas_line_get_type (),
					      "points", points,
					      "width_pixels", 1,
					      "fill_color_rgba", GRID_COLOR,
					      NULL);
	}
	points->coords[0] = points->coords[2] = w;
	item = gnome_canvas_item_new (view->grid_group,
				      gnome_canvas_line_get_type (),
				      "points", points,
				      "width_pixels", 1,
				      "fill_color_rgba", GRID_COLOR,
				      NULL);

	points->coords[0] = 0.0;
	points->coords[2] = w;
	for ( y=0.0; y < h; y += view->grid_spacing ) {
		points->coords[1] = points->coords[3] = y;
		item = gnome_canvas_item_new (view->grid_group,
					      gnome_canvas_line_get_type (),
					      "points", points,
					      "width_pixels", 1,
					      "fill_color_rgba", GRID_COLOR,
					      NULL);
	}
	points->coords[1] = points->coords[3] = h;
	item = gnome_canvas_item_new (view->grid_group,
				      gnome_canvas_line_get_type (),
				      "points", points,
				      "width_pixels", 1,
				      "fill_color_rgba", GRID_COLOR,
				      NULL);

	gnome_canvas_points_free (points);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw markup lines.                                              */
/*---------------------------------------------------------------------------*/
static void
draw_markup_layer (glView *view)
{
	GnomeCanvasGroup *group;
	glLabel          *label;
	glTemplate       *template;
	GList            *p;
	glTemplateMarkup *markup;

	group = gnome_canvas_root (GNOME_CANVAS (view->canvas));
	view->markup_group = GNOME_CANVAS_GROUP(
		gnome_canvas_item_new (group,
				       gnome_canvas_group_get_type (),
				       "x", 0.0,
				       "y", 0.0,
				       NULL));
	label = view->label;
	template = gl_label_get_template (label);

	for ( p=template->label.any.markups; p != NULL; p=p->next ) {
		markup = (glTemplateMarkup *)p->data;

		switch (markup->type) {
		case GL_TEMPLATE_MARKUP_MARGIN:
			draw_markup_margin (view,
					    (glTemplateMarkupMargin *)markup);
			break;
		case GL_TEMPLATE_MARKUP_LINE:
			draw_markup_line (view,
					  (glTemplateMarkupLine *)markup);
			break;
		default:
			g_warning ("Unknown template markup type");
			break;
		}
	}
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw margin markup.                                             */
/*---------------------------------------------------------------------------*/
static void
draw_markup_margin (glView                 *view,
		    glTemplateMarkupMargin *margin)
{
	glLabel    *label;
	glTemplate *template;

	label = view->label;
	template = gl_label_get_template (label);

	switch (template->label.style) {

	case GL_TEMPLATE_STYLE_RECT:
		if (template->label.rect.r == 0.0) {
			/* Square corners. */
			draw_markup_margin_rect (view, margin);
		} else {
			if ( margin->size < template->label.rect.r) {
				/* Rounded corners. */
				draw_markup_margin_rounded_rect (view, margin);
			} else {
				/* Square corners. */
				draw_markup_margin_rect (view, margin);
			}
		}
		break;

	case GL_TEMPLATE_STYLE_ROUND:
		draw_markup_margin_round (view, margin);
		break;

	case GL_TEMPLATE_STYLE_CD:
		draw_markup_margin_cd (view, margin);
		break;

	default:
		g_warning ("Unknown template label style");
		break;
	}
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw simple recangular margin.                                  */
/*---------------------------------------------------------------------------*/
static void
draw_markup_margin_rect (glView                 *view,
			 glTemplateMarkupMargin *margin)
{
	glLabel          *label = view->label;
	glTemplate       *template;
	gdouble           w, h, m;
	GnomeCanvasItem  *item;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));
	g_return_if_fail (label != NULL);

	gl_label_get_size (label, &w, &h);
	template = gl_label_get_template (label);
	m = margin->size;

	/* Bounding box @ margin */
	gnome_canvas_item_new (view->markup_group,
			       gnome_canvas_rect_get_type (),
			       "x1", m,
			       "y1", m,
			       "x2", w - m,
			       "y2", h - m,
			       "width_pixels", 1,
			       "outline_color_rgba", MARKUP_COLOR,
			       NULL);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw rounded recangular markup.                                 */
/*---------------------------------------------------------------------------*/
static void
draw_markup_margin_rounded_rect (glView                 *view,
				 glTemplateMarkupMargin *margin)
{
	glLabel           *label = view->label;
	GnomeCanvasPoints *points;
	gint               i_coords, i_theta;
	glTemplate        *template;
	gdouble            r, w, h, m;
	GnomeCanvasItem   *item;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));
	g_return_if_fail (label != NULL);

	gl_label_get_size (label, &w, &h);
	template = gl_label_get_template (label);
	r = template->label.rect.r;
	m = margin->size;

	r = r - m;
	w = w - 2 * m;
	h = h - 2 * m;

	/* rectangle with rounded corners */
	points = gnome_canvas_points_new (4 * (1 + 90 / 5));
	i_coords = 0;
	for (i_theta = 0; i_theta <= 90; i_theta += 5) {
		points->coords[i_coords++] =
			m + r - r * sin (i_theta * M_PI / 180.0);
		points->coords[i_coords++] =
			m + r - r * cos (i_theta * M_PI / 180.0);
	}
	for (i_theta = 0; i_theta <= 90; i_theta += 5) {
		points->coords[i_coords++] =
			m + r - r * cos (i_theta * M_PI / 180.0);
		points->coords[i_coords++] =
			m + (h - r) + r * sin (i_theta * M_PI / 180.0);
	}
	for (i_theta = 0; i_theta <= 90; i_theta += 5) {
		points->coords[i_coords++] =
			m + (w - r) + r * sin (i_theta * M_PI / 180.0);
		points->coords[i_coords++] =
			m + (h - r) + r * cos (i_theta * M_PI / 180.0);
	}
	for (i_theta = 0; i_theta <= 90; i_theta += 5) {
		points->coords[i_coords++] =
			m + (w - r) + r * cos (i_theta * M_PI / 180.0);
		points->coords[i_coords++] =
			m + r - r * sin (i_theta * M_PI / 180.0);
	}
	item = gnome_canvas_item_new (view->markup_group,
				      gnome_canvas_polygon_get_type (),
				      "points", points,
				      "width_pixels", 1,
				      "outline_color_rgba", MARKUP_COLOR,
				      NULL);
	gnome_canvas_points_free (points);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw round margin.                                              */
/*---------------------------------------------------------------------------*/
static void
draw_markup_margin_round (glView                 *view,
			  glTemplateMarkupMargin *margin)
{
	glLabel          *label = view->label;
	glTemplate       *template;
	gdouble           r, m;
	GnomeCanvasItem  *item;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));
	g_return_if_fail (label != NULL);

	template = gl_label_get_template (label);

	r = template->label.round.r;
	m = margin->size;

	/* Margin outline */
	item = gnome_canvas_item_new (view->markup_group,
				      gnome_canvas_ellipse_get_type (),
				      "x1", m,
				      "y1", m,
				      "x2", 2.0*r - m,
				      "y2", 2.0*r - m,
				      "width_pixels", 1,
				      "outline_color_rgba", MARKUP_COLOR,
				      NULL);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw CD margins.                                                */
/*---------------------------------------------------------------------------*/
static void
draw_markup_margin_cd (glView                 *view,
		       glTemplateMarkupMargin *margin)
{
	glLabel          *label = view->label;
	glTemplate       *template;
	gdouble           m, r1, r2;
	GnomeCanvasItem  *item;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));
	g_return_if_fail (label != NULL);

	template = gl_label_get_template (label);

	r1 = template->label.cd.r1;
	r2 = template->label.cd.r2;
	m  = margin->size;

	/* outer margin */
	item = gnome_canvas_item_new (view->markup_group,
				      gnome_canvas_ellipse_get_type (),
				      "x1", m,
				      "y1", m,
				      "x2", 2.0*r1 - m,
				      "y2", 2.0*r1 - m,
				      "width_pixels", 1,
				      "outline_color_rgba", MARKUP_COLOR,
				      NULL);
	/* inner margin */
	item = gnome_canvas_item_new (view->markup_group,
				      gnome_canvas_ellipse_get_type (),
				      "x1", r1 - r2 - m,
				      "y1", r1 - r2 - m,
				      "x2", r1 + r2 + m,
				      "y2", r1 + r2 + m,
				      "width_pixels", 1,
				      "outline_color_rgba", MARKUP_COLOR,
				      NULL);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw line markup.                                               */
/*---------------------------------------------------------------------------*/
static void
draw_markup_line (glView               *view,
		  glTemplateMarkupLine *line)
{
	GnomeCanvasPoints *points;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

	points = gnome_canvas_points_new (2);
	points->coords[0] = line->x1;
	points->coords[1] = line->y1;
	points->coords[2] = line->x2;
	points->coords[3] = line->y2;

	/* Bounding box @ margin */
	gnome_canvas_item_new (view->markup_group,
			       gnome_canvas_line_get_type (),
			       "points", points,
			       "width_pixels", 1,
			       "fill_color_rgba", MARKUP_COLOR,
			       NULL);

	gnome_canvas_points_free (points);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Show grid.                                                                */
/*****************************************************************************/
void       gl_view_show_grid               (glView            *view)
{
	gnome_canvas_item_show (GNOME_CANVAS_ITEM(view->grid_group));
}

/*****************************************************************************/
/* Hide grid.                                                                */
/*****************************************************************************/
void       gl_view_hide_grid               (glView            *view)
{
	gnome_canvas_item_hide (GNOME_CANVAS_ITEM(view->grid_group));
}

/*****************************************************************************/
/* Set grid spacing.                                                         */
/*****************************************************************************/
void       gl_view_set_grid_spacing        (glView            *view,
					    gdouble            spacing)
{
	view->grid_spacing = spacing;

	gtk_object_destroy (GTK_OBJECT(view->grid_group));
	draw_grid_layer (view);
}

/*****************************************************************************/
/* Show markup.                                                              */
/*****************************************************************************/
void       gl_view_show_markup             (glView            *view)
{
	gnome_canvas_item_show (GNOME_CANVAS_ITEM(view->markup_group));
}

/*****************************************************************************/
/* Hide markup.                                                              */
/*****************************************************************************/
void       gl_view_hide_markup             (glView            *view)
{
	gnome_canvas_item_hide (GNOME_CANVAS_ITEM(view->markup_group));
}

/*****************************************************************************/
/* Set arrow mode.                                                           */
/*****************************************************************************/
void
gl_view_arrow_mode (glView *view)
{
	static GdkCursor *cursor = NULL;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

	if (!cursor) {
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
	}

	gdk_window_set_cursor (view->canvas->window, cursor);

	view->state = GL_VIEW_STATE_ARROW;

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Set create text object mode.                                              */
/*****************************************************************************/
void
gl_view_object_create_mode (glView            *view,
			    glLabelObjectType type)
{
	GdkCursor *cursor;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

	switch (type) {
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
		g_warning ("Invalid label object type.");/*Should not happen!*/
		break;
	}

	gdk_window_set_cursor (view->canvas->window, cursor);

	view->state = GL_VIEW_STATE_OBJECT_CREATE;
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

	g_return_if_fail (GL_IS_VIEW (view));

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
	GList *p, *p_next;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

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
gl_view_select_region (glView  *view,
		       gdouble  x1,
		       gdouble  y1,
		       gdouble  x2,
		       gdouble  y2)
{
	GList *p;
	glViewObject *view_object;
	glLabelObject *object;
	gdouble i_x1, i_y1, i_x2, i_y2;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));
	g_return_if_fail ((x1 <= x2) && (y1 <= y2));

	for (p = view->object_list; p != NULL; p = p->next) {
		view_object = GL_VIEW_OBJECT(p->data);
		if (!is_object_selected (view, view_object)) {

			object = gl_view_object_get_object (view_object);

			gl_label_object_get_extent (object, &i_x1, &i_y1, &i_x2, &i_y2);
			if ((i_x1 >= x1) && (i_x2 <= x2) && (i_y1 >= y1)
			    && (i_y2 <= y2)) {
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

	g_return_if_fail (GL_IS_VIEW (view));
	g_return_if_fail (GL_IS_VIEW_OBJECT (view_object));

	if (!is_object_selected (view, view_object)) {
		view->selected_object_list =
		    g_list_prepend (view->selected_object_list, view_object);
	}
	gl_view_object_show_highlight (view_object);
	gtk_widget_grab_focus (GTK_WIDGET (view->canvas));

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

	g_return_if_fail (GL_IS_VIEW (view));
	g_return_if_fail (GL_IS_VIEW_OBJECT (view_object));

	gl_view_object_hide_highlight (view_object);

	view->selected_object_list =
	    g_list_remove (view->selected_object_list, view_object);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. Return object at (x,y).                                          */
/*---------------------------------------------------------------------------*/
static gboolean
object_at (glView  *view,
	   gdouble  x,
	   gdouble  y)
{
	GnomeCanvasItem *item, *p_item;
	GList *p;

	gl_debug (DEBUG_VIEW, "");

	g_return_val_if_fail (GL_IS_VIEW (view), FALSE);

	item = gnome_canvas_get_item_at (GNOME_CANVAS (view->canvas), x, y);

	/* No item is at x, y */
	if (item == NULL)
		return FALSE;

	/* ignore items not in label or highlight layers, e.g. background items */
	if (!is_item_member_of_group(view, item, GNOME_CANVAS_ITEM(view->label_group)) &&
	    !is_item_member_of_group(view, item, GNOME_CANVAS_ITEM(view->highlight_group)))
		return FALSE;

	return TRUE;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Is the item a child (or grandchild, etc.) of group.             */
/*---------------------------------------------------------------------------*/
static gboolean
is_item_member_of_group (glView          *view,
			 GnomeCanvasItem *item,
			 GnomeCanvasItem *group)
{
	GnomeCanvasItem *parent;
	GnomeCanvasItem *root_group;

	root_group = GNOME_CANVAS_ITEM(gnome_canvas_root (GNOME_CANVAS (view->canvas)));

	for ( parent=item->parent; parent && (parent!=root_group); parent=parent->parent) {
		if (parent == group) return TRUE;
	}
	return FALSE;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Is the object in our current selection?                         */
/*---------------------------------------------------------------------------*/
static gboolean
is_object_selected (glView       *view,
		    glViewObject *view_object)
{
	gl_debug (DEBUG_VIEW, "");

	g_return_val_if_fail (GL_IS_VIEW (view), FALSE);
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

	g_return_val_if_fail (GL_IS_VIEW (view), FALSE);

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

	g_return_val_if_fail (GL_IS_VIEW (view), FALSE);

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
	GList *p, *p_next;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p_next) {
		p_next = p->next;
		g_object_unref (G_OBJECT (p->data));
	}

	g_signal_emit (G_OBJECT(view), signals[SELECTION_CHANGED], 0);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Edit properties of selected object.                                       */
/*****************************************************************************/
void
gl_view_edit_object_props (glView *view)
{
	glViewObject *view_object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

	if (gl_view_is_selection_atomic (view)) {

		view_object = GL_VIEW_OBJECT(view->selected_object_list->data);
		gl_view_object_show_dialog (view_object);

	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Raise selected items to top.                                              */
/*****************************************************************************/
void
gl_view_raise_selection (glView *view)
{
	GList *p;
	glViewObject *view_object;
	glLabelObject *label_object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {
		view_object = GL_VIEW_OBJECT (p->data);
		label_object = gl_view_object_get_object (view_object);
		gl_label_object_raise_to_top (label_object);
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Lower selected items to bottom.                                           */
/*****************************************************************************/
void
gl_view_lower_selection (glView *view)
{
	GList *p;
	glViewObject *view_object;
	glLabelObject *label_object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {
		view_object = GL_VIEW_OBJECT (p->data);
		label_object = gl_view_object_get_object (view_object);
		gl_label_object_lower_to_bottom (label_object);
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
	GList *p;
	glViewObject *view_object;
	glLabelObject *label_object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {
		view_object = GL_VIEW_OBJECT (p->data);
		label_object = gl_view_object_get_object (view_object);
		gl_label_object_rotate (label_object, theta_degs);
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Flip selected objects horizontally.                                       */
/*****************************************************************************/
void
gl_view_flip_selection_horiz (glView *view)
{
	GList *p;
	glViewObject *view_object;
	glLabelObject *label_object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {
		view_object = GL_VIEW_OBJECT (p->data);
		label_object = gl_view_object_get_object (view_object);
		gl_label_object_flip_horiz (label_object);
	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Flip selected objects vertically.                                         */
/*****************************************************************************/
void
gl_view_flip_selection_vert (glView *view)
{
	GList *p;
	glViewObject *view_object;
	glLabelObject *label_object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {
		view_object = GL_VIEW_OBJECT (p->data);
		label_object = gl_view_object_get_object (view_object);
		gl_label_object_flip_vert (label_object);
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

	g_return_if_fail (GL_IS_VIEW (view));

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
	GList *p;
	glViewObject *view_object;
	glLabelObject *object;
	glTemplate *template;
	gboolean rotate_flag;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

	if (view->selected_object_list) {

		if ( view->selection_data ) {
			g_object_unref (view->selection_data);
		}
		template = gl_label_get_template (view->label);
		rotate_flag = gl_label_get_rotate_flag (view->label);
		view->selection_data = GL_LABEL(gl_label_new ());
		gl_label_set_template (view->selection_data, template);
		gl_label_set_rotate_flag (view->selection_data, rotate_flag);
		gl_template_free (&template);

		for (p = view->selected_object_list; p != NULL; p = p->next) {

			view_object = GL_VIEW_OBJECT (p->data);
			object = gl_view_object_get_object (view_object);

			gl_debug (DEBUG_VIEW, "Object copied");

			if (GL_IS_LABEL_BOX (object)) {
				gl_label_box_dup (GL_LABEL_BOX(object),
						  view->selection_data);
			} else if (GL_IS_LABEL_ELLIPSE (object)) {
				gl_label_ellipse_dup (GL_LABEL_ELLIPSE(object),
						      view->selection_data);
			} else if (GL_IS_LABEL_LINE (object)) {
				gl_label_line_dup (GL_LABEL_LINE(object),
						   view->selection_data);
			} else if (GL_IS_LABEL_IMAGE (object)) {
				gl_label_image_dup (GL_LABEL_IMAGE(object),
						    view->selection_data);
			} else if (GL_IS_LABEL_TEXT (object)) {
				gl_label_text_dup (GL_LABEL_TEXT(object),
						   view->selection_data);
			} else if (GL_IS_LABEL_BARCODE (object)) {
				gl_label_barcode_dup (GL_LABEL_BARCODE(object),
						      view->selection_data);
			} else {
				/* Should not happen! */
				g_warning ("Invalid label object type.");
			}


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

	g_return_if_fail (GL_IS_VIEW (view));

	gtk_selection_convert (GTK_WIDGET (view->invisible),
			       clipboard_atom, GDK_SELECTION_TYPE_STRING,
			       GDK_CURRENT_TIME);

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  move selected objects                                           */
/*---------------------------------------------------------------------------*/
static void
move_selection (glView  *view,
		gdouble  dx,
		gdouble  dy)
{
	GList *p;
	glLabelObject *object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

	for (p = view->selected_object_list; p != NULL; p = p->next) {

		object = gl_view_object_get_object(GL_VIEW_OBJECT (p->data));
		gl_label_object_set_position_relative (object, dx, dy);

	}

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Zoom in one "notch"                                                       */
/*****************************************************************************/
void
gl_view_zoom_in (glView *view)
{
	gint i, i_min;
	gdouble dist, dist_min;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

	/* Find index of current scale (or best match) */
	i_min = 1;		/* start with 2nd largest scale */
	dist_min = fabs (scales[1] - view->scale);
	for (i = 2; i < N_SCALES; i++) {
		dist = fabs (scales[i] - view->scale);
		if (dist < dist_min) {
			i_min = i;
			dist_min = dist;
		}
	}

	/* zoom in one "notch" */
	i = MAX (0, i_min - 1);
	gl_view_set_zoom (view, scales[i] / HOME_SCALE);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Zoom out one "notch"                                                      */
/*****************************************************************************/
void
gl_view_zoom_out (glView *view)
{
	gint i, i_min;
	gdouble dist, dist_min;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

	/* Find index of current scale (or best match) */
	i_min = 0;		/* start with largest scale */
	dist_min = fabs (scales[0] - view->scale);
	for (i = 1; i < N_SCALES; i++) {
		dist = fabs (scales[i] - view->scale);
		if (dist < dist_min) {
			i_min = i;
			dist_min = dist;
		}
	}

	/* zoom out one "notch" */
	if (i_min >= N_SCALES)
		return;
	i = i_min + 1;
	if (i >= N_SCALES)
		return;
	gl_view_set_zoom (view, scales[i] / HOME_SCALE);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Set current zoom factor to explicit value.                                */
/*****************************************************************************/
void
gl_view_set_zoom (glView  *view,
		  gdouble scale)
{
	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));
	g_return_if_fail (scale > 0.0);

	view->scale = scale * HOME_SCALE;
	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (view->canvas),
					  scale * HOME_SCALE);

	g_signal_emit (G_OBJECT(view), signals[ZOOM_CHANGED], 0, scale);

	gl_debug (DEBUG_VIEW, "END");
}

/*****************************************************************************/
/* Get current zoom factor.                                                  */
/*****************************************************************************/
gdouble
gl_view_get_zoom (glView *view)
{
	gl_debug (DEBUG_VIEW, "");

	g_return_val_if_fail (GL_IS_VIEW (view), 1.0);

	return view->scale / HOME_SCALE;
}

/*****************************************************************************/
/* Is this the maximum zoom level.                                           */
/*****************************************************************************/
gboolean
gl_view_is_zoom_max (glView *view)
{
	gl_debug (DEBUG_VIEW, "");

	g_return_val_if_fail (GL_IS_VIEW (view), FALSE);

	return view->scale >= scales[0];
}

/*****************************************************************************/
/* Is this the minimum zoom level.                                           */
/*****************************************************************************/
gboolean
gl_view_is_zoom_min (glView *view)
{
	gl_debug (DEBUG_VIEW, "");

	g_return_val_if_fail (GL_IS_VIEW (view), FALSE);

	return view->scale <= scales[N_SCALES-1];
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Canvas event handler.                                           */
/*---------------------------------------------------------------------------*/
static int
canvas_event (GnomeCanvas *canvas,
	      GdkEvent    *event,
	      glView      *view)
{
	gdouble x, y;

	gl_debug (DEBUG_VIEW, "");

	/* emit pointer signals regardless of state */
	switch (event->type) {
	case GDK_MOTION_NOTIFY:
		gl_debug (DEBUG_VIEW, "MOTION_NOTIFY");
		gnome_canvas_window_to_world (canvas,
					      event->motion.x,
					      event->motion.y, &x, &y);
		g_signal_emit (G_OBJECT(view), signals[POINTER_MOVED], 0, x, y);
		break; /* fall through */

	case GDK_LEAVE_NOTIFY:
		gl_debug (DEBUG_VIEW, "LEAVEW_NOTIFY");
		g_signal_emit (G_OBJECT(view), signals[POINTER_EXIT], 0);
		break; /* fall through */

	default:
		break; /* fall through */
	}


	switch (view->state) {

	case GL_VIEW_STATE_ARROW:
		return canvas_event_arrow_mode (canvas, event, view);

	case GL_VIEW_STATE_OBJECT_CREATE:
		switch (view->create_type) {
		case GL_LABEL_OBJECT_BOX:
			return gl_view_box_create_event_handler (canvas,
								 event,
								 view);
			break;
		case GL_LABEL_OBJECT_ELLIPSE:
			return gl_view_ellipse_create_event_handler (canvas,
								     event,
								     view);
			break;
		case GL_LABEL_OBJECT_LINE:
			return gl_view_line_create_event_handler (canvas,
								  event,
								  view);
			break;
		case GL_LABEL_OBJECT_IMAGE:
			return gl_view_image_create_event_handler (canvas,
								   event,
								   view);
			break;
		case GL_LABEL_OBJECT_TEXT:
			return gl_view_text_create_event_handler (canvas,
								  event,
								  view);
			break;
		case GL_LABEL_OBJECT_BARCODE:
			return gl_view_barcode_create_event_handler (canvas,
								     event,
								     view);
			break;
		default:
                        /*Should not happen!*/
			g_warning ("Invalid label object type.");
			return FALSE;
	}

	default:
		g_warning ("Invalid view state.");	/*Should not happen!*/
		return FALSE;

	}
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Canvas event handler (arrow mode)                               */
/*---------------------------------------------------------------------------*/
static int
canvas_event_arrow_mode (GnomeCanvas *canvas,
			 GdkEvent    *event,
			 glView      *view)
{
	static gdouble x0, y0;
	static gboolean dragging = FALSE;
	static GnomeCanvasItem *item;
	gdouble x, y, x1, y1, x2, y2;
	GnomeCanvasGroup *group;
	GdkCursor *cursor;

	gl_debug (DEBUG_VIEW, "");

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		gl_debug (DEBUG_VIEW, "BUTTON_PRESS");
		switch (event->button.button) {
		case 1:
			gnome_canvas_window_to_world (canvas,
						      event->button.x,
						      event->button.y, &x, &y);

			if (!object_at (view, x, y)) {
				if (!(event->button.state & GDK_CONTROL_MASK)) {
					gl_view_unselect_all (view);
				}

				dragging = TRUE;
				gnome_canvas_item_grab (canvas->root,
							GDK_POINTER_MOTION_MASK |
							GDK_BUTTON_RELEASE_MASK |
							GDK_BUTTON_PRESS_MASK,
							NULL, event->button.time);
				group =
				    gnome_canvas_root (GNOME_CANVAS
						       (view->canvas));
				item =
				    gnome_canvas_item_new (group,
							   gnome_canvas_rect_get_type (),
							   "x1", x, "y1", y,
							   "x2", x, "y2", y,
							   "width_pixels", 2,
							   "outline_color_rgba",
							   SEL_LINE_COLOR,
							   "fill_color_rgba",
							   SEL_FILL_COLOR,
							   NULL);
				x0 = x;
				y0 = y;

			}
			return FALSE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		gl_debug (DEBUG_VIEW, "BUTTON_RELEASE");
		switch (event->button.button) {
		case 1:
			if (dragging) {
				dragging = FALSE;
				gnome_canvas_item_ungrab (canvas->root,
							  event->button.time);
				gnome_canvas_window_to_world (canvas,
							      event->button.x,
							      event->button.y,
							      &x, &y);
				x1 = MIN (x, x0);
				y1 = MIN (y, y0);
				x2 = MAX (x, x0);
				y2 = MAX (y, y0);
				gl_view_select_region (view, x1, y1, x2, y2);
				gtk_object_destroy (GTK_OBJECT (item));
				return TRUE;
			}
			return FALSE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		gl_debug (DEBUG_VIEW, "MOTION_NOTIFY");
		gnome_canvas_window_to_world (canvas,
					      event->motion.x,
					      event->motion.y, &x, &y);
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			gnome_canvas_item_set (item,
					       "x1", MIN (x, x0),
					       "y1", MIN (y, y0),
					       "x2", MAX (x, x0),
					       "y2", MAX (y, y0), NULL);
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_KEY_PRESS:
		gl_debug (DEBUG_VIEW, "KEY_PRESS");
		if (!dragging) {
			switch (event->key.keyval) {
			case GDK_Left:
			case GDK_KP_Left:
				move_selection (view,
						-1.0 / (view->scale), 0.0);
				break;
			case GDK_Up:
			case GDK_KP_Up:
				move_selection (view,
						0.0, -1.0 / (view->scale));
				break;
			case GDK_Right:
			case GDK_KP_Right:
				move_selection (view,
						1.0 / (view->scale), 0.0);
				break;
			case GDK_Down:
			case GDK_KP_Down:
				move_selection (view,
						0.0, 1.0 / (view->scale));
				break;
			case GDK_Delete:
			case GDK_KP_Delete:
				gl_view_delete_selection (view);
				cursor = gdk_cursor_new (GDK_LEFT_PTR);
				gdk_window_set_cursor (view->canvas->window,
						       cursor);
				gdk_cursor_unref (cursor);
				break;
			default:
				return FALSE;
			}
		}
		return TRUE;	/* We handled this or we were dragging. */

	default:
		gl_debug (DEBUG_VIEW, "default");
		return FALSE;
	}

}

/*****************************************************************************/
/* Item event handler.                                                       */
/*****************************************************************************/
gint
gl_view_item_event_handler (GnomeCanvasItem *item,
			    GdkEvent        *event,
			    glViewObject    *view_object)
{
	glView *view;

	gl_debug (DEBUG_VIEW, "");

	view = gl_view_object_get_view(view_object);
	switch (view->state) {

	case GL_VIEW_STATE_ARROW:
		return item_event_arrow_mode (item, event, view_object);

	default:
		return FALSE;

	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Item event handler (arrow mode)                                 */
/*---------------------------------------------------------------------------*/
static int
item_event_arrow_mode (GnomeCanvasItem *item,
		       GdkEvent        *event,
		       glViewObject    *view_object)
{
	static gdouble x, y;
	static gboolean dragging = FALSE;
	glView *view;
	GdkCursor *cursor;
	gdouble item_x, item_y;
	gdouble new_x, new_y;
	gboolean control_key_pressed;

	gl_debug (DEBUG_VIEW, "");

	item_x = event->button.x;
	item_y = event->button.y;
	gnome_canvas_item_w2i (item->parent, &item_x, &item_y);

	view = gl_view_object_get_view(view_object);

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		gl_debug (DEBUG_VIEW, "BUTTON_PRESS");
		control_key_pressed = event->button.state & GDK_CONTROL_MASK;
		switch (event->button.button) {
		case 1:
			if (control_key_pressed) {
				if (is_object_selected (view, view_object)) {
					/* Un-selecting a selected item */
					gl_view_unselect_object (view,
								 view_object);
					return TRUE;
				} else {
					/* Add to current selection */
					gl_view_select_object (view,
							       view_object);
				}
			} else {
				if (!is_object_selected (view, view_object)) {
					/* No control, key so remove any selections before adding */
					gl_view_unselect_all (view);
					/* Add to current selection */
					gl_view_select_object (view,
							       view_object);
				}
			}
			/* Go into dragging mode while button remains pressed. */
			x = item_x;
			y = item_y;
			cursor = gdk_cursor_new (GDK_FLEUR);
			gnome_canvas_item_grab (item,
						GDK_POINTER_MOTION_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_BUTTON_PRESS_MASK,
						cursor, event->button.time);
			gdk_cursor_unref (cursor);
			dragging = TRUE;
			return TRUE;

		case 3:
			if (!is_object_selected (view, view_object)) {
				if (!control_key_pressed) {
					/* No control, key so remove any selections before adding */
					gl_view_unselect_all (view);
				}
			}
			/* Add to current selection */
			gl_view_select_object (view, view_object);
			/* bring up apropriate menu for selection. */
			popup_selection_menu (view, view_object, event);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		gl_debug (DEBUG_VIEW, "BUTTON_RELEASE");
		switch (event->button.button) {
		case 1:
			/* Exit dragging mode */
			gnome_canvas_item_ungrab (item, event->button.time);
			dragging = FALSE;
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		gl_debug (DEBUG_VIEW, "MOTION_NOTIFY");
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			/* Dragging mode, move selection */
			new_x = item_x;
			new_y = item_y;
			move_selection (view, (new_x - x), (new_y - y));
			x = new_x;
			y = new_y;
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_2BUTTON_PRESS:
		gl_debug (DEBUG_VIEW, "2BUTTON_PRESS");
		switch (event->button.button) {
		case 1:
			/* Also exit dragging mode w/ double-click, run dlg */
			gnome_canvas_item_ungrab (item, event->button.time);
			dragging = FALSE;
			gl_view_select_object (view, view_object);
			gl_view_object_show_dialog (view_object);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		gl_debug (DEBUG_VIEW, "ENTER_NOTIFY");
		cursor = gdk_cursor_new (GDK_FLEUR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		gl_debug (DEBUG_VIEW, "LEAVE_NOTIFY");
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (view->canvas->window, cursor);
		gdk_cursor_unref (cursor);
		return TRUE;

	default:
		gl_debug (DEBUG_VIEW, "default");
		return FALSE;
	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  create menu for multiple selections.                            */
/*---------------------------------------------------------------------------*/
GtkWidget *
new_selection_menu (glView *view)
{
	GtkWidget *menu, *menuitem;

	gl_debug (DEBUG_VIEW, "START");

	g_return_val_if_fail (GL_IS_VIEW (view), NULL);

	menu = gtk_menu_new ();

	menuitem = gtk_menu_item_new_with_label (_("Delete"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	gtk_widget_show (menuitem);
	g_signal_connect_swapped (G_OBJECT (menuitem), "activate",
				  G_CALLBACK (gl_view_delete_selection), view);

	menuitem = gtk_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	gtk_widget_show (menuitem);

	menuitem = gtk_menu_item_new_with_label (_("Bring to front"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	gtk_widget_show (menuitem);
	g_signal_connect_swapped (G_OBJECT (menuitem), "activate",
				  G_CALLBACK (gl_view_raise_selection), view);

	menuitem = gtk_menu_item_new_with_label (_("Send to back"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	gtk_widget_show (menuitem);
	g_signal_connect_swapped (G_OBJECT (menuitem), "activate",
				  G_CALLBACK (gl_view_lower_selection), view);

	gl_debug (DEBUG_VIEW, "END");

	return menu;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  popup menu for given item.                                      */
/*---------------------------------------------------------------------------*/
static void
popup_selection_menu (glView       *view,
		      glViewObject *view_object,
		      GdkEvent     *event)
{
	GtkMenu *menu;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));
	g_return_if_fail (GL_IS_VIEW_OBJECT (view_object));

	if (gl_view_is_selection_atomic (view)) {

		menu = gl_view_object_get_menu (view_object);
		if (menu != NULL) {
			gtk_menu_popup (GTK_MENU (menu),
					NULL, NULL, NULL, NULL,
					event->button.button,
					event->button.time);
		}

	} else {

		if (view->menu != NULL) {
			gtk_menu_popup (GTK_MENU (view->menu),
					NULL, NULL, NULL, NULL,
					event->button.button,
					event->button.time);
		}

	}

	gl_debug (DEBUG_VIEW, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Handle "selection-clear" signal.                                */
/*---------------------------------------------------------------------------*/
static void
selection_clear_cb (GtkWidget         *widget,
		    GdkEventSelection *event,
		    gpointer          data)
{
	glView *view = GL_VIEW (data);

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

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
		  guint            info,
		  guint            time,
		  gpointer         data)
{
	glView *view = GL_VIEW (data);
	gchar *buffer;
	glXMLLabelStatus status;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

	if (view->have_selection) {

		buffer = gl_xml_label_save_buffer (view->selection_data,
						   &status);
		gtk_selection_data_set (selection_data,
					GDK_SELECTION_TYPE_STRING, 8, buffer,
					strlen (buffer));
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
		       guint            time,
		       gpointer         data)
{
	glView *view = GL_VIEW (data);
	glLabel *label = NULL;
	glXMLLabelStatus status;
	GList *p, *p_next;
	glLabelObject *object, *newobject;
	glViewObject *view_object;

	gl_debug (DEBUG_VIEW, "START");

	g_return_if_fail (GL_IS_VIEW (view));

	if (selection_data->length < 0) {
		return;
	}
	if (selection_data->type != GDK_SELECTION_TYPE_STRING) {
		return;
	}

	gl_view_unselect_all (view);

	label = gl_xml_label_open_buffer (selection_data->data, &status);
	for (p = label->objects; p != NULL; p = p_next) {
		p_next = p->next;

		object = (glLabelObject *) p->data;
		gl_label_object_set_parent (object, view->label);

		gl_debug (DEBUG_VIEW, "object pasted");

		if (GL_IS_LABEL_BOX (object)) {
			view_object = gl_view_box_new (GL_LABEL_BOX(object),
						       view);
		} else if (GL_IS_LABEL_ELLIPSE (object)) {
			view_object = gl_view_ellipse_new (GL_LABEL_ELLIPSE(object),
							   view);
		} else if (GL_IS_LABEL_LINE (object)) {
			view_object = gl_view_line_new (GL_LABEL_LINE(object),
							view);
		} else if (GL_IS_LABEL_IMAGE (object)) {
			view_object = gl_view_image_new (GL_LABEL_IMAGE(object),
							 view);
		} else if (GL_IS_LABEL_TEXT (object)) {
			view_object = gl_view_text_new (GL_LABEL_TEXT(object),
							view);
		} else if (GL_IS_LABEL_BARCODE (object)) {
			view_object = gl_view_barcode_new (GL_LABEL_BARCODE(object),
							   view);
		} else {
			/* Should not happen! */
			view_object = NULL;
			g_warning ("Invalid label object type.");
		}
		gl_view_select_object (view, view_object);
	}
	g_object_unref (label);

	gl_debug (DEBUG_VIEW, "END");
}

