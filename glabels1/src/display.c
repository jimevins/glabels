/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  display.c:  GLabels Display module
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
#include <gtk/gtkinvisible.h>

#include <string.h>
#include <math.h>

#include "display.h"
#include "item.h"

#include "debug.h"

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkContainerClass *parent_class;

/* "CLIPBOARD" selection */
static GdkAtom clipboard_atom = GDK_NONE;

#define HOME_SCALE 2.0
static gdouble scales[] = { 8.0, 6.0, 4.0, 3.0,
	2.0,
	1.5, 1.0, 0.5, 0.25,
	0.0
};

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_display_class_init (glDisplayClass * class);
static void gl_display_init (glDisplay * display);
static void gl_display_destroy (GtkObject * object);

static void gl_display_construct (glDisplay * display);
static GtkWidget *gl_display_construct_canvas (glDisplay * display);
static void gl_display_construct_selection (glDisplay * display);

static gdouble get_apropriate_scale (glLabel * label);

static void draw_rect_bg (glDisplay * display);
static void draw_rounded_rect_bg (glDisplay * display);
static void draw_round_bg (glDisplay * display);
static void draw_cd_bg (glDisplay * display);

static int canvas_event (GnomeCanvas * canvas,
			 GdkEvent * event, gpointer data);
static int canvas_event_arrow_mode (GnomeCanvas * canvas,
				    GdkEvent * event, gpointer data);

static GnomeCanvasItem *display_item_at (glDisplay * display,
					 gdouble x, gdouble y);
static gboolean item_selected (glDisplay * display,
			       GnomeCanvasItem * item);
static gboolean multiple_items_selected (glDisplay * display);

static int item_event_arrow_mode (GnomeCanvasItem * item,
				  GdkEvent * event, gpointer data);

static void popup_selection_menu (glDisplay * display,
				  GnomeCanvasItem * item, GdkEvent * event);

static void delete_item_cb (GtkWidget * widget, GnomeCanvasItem * item);
static void raise_item_cb (GtkWidget * widget, GnomeCanvasItem * item);
static void lower_item_cb (GtkWidget * widget, GnomeCanvasItem * item);

static void move_selected_items (glDisplay * display, gdouble dx, gdouble dy);
static void move_item (GnomeCanvasItem * item, gdouble dx, gdouble dy);

static void delete_selection_cb (GtkWidget * widget, glDisplay * display);
static void raise_selection_cb (GtkWidget * widget, glDisplay * display);
static void lower_selection_cb (GtkWidget * widget, glDisplay * display);

static void selection_clear_cb (GtkWidget * widget,
				GdkEventSelection * event, gpointer data);

static void selection_get_cb (GtkWidget * widget,
			      GtkSelectionData * selection_data, guint info,
			      guint time, gpointer data);

static void selection_received_cb (GtkWidget * widget,
				   GtkSelectionData * selection_data,
				   guint time, gpointer data);

/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
guint
gl_display_get_type (void)
{
	static guint display_type = 0;

	if (!display_type) {
		GtkTypeInfo display_info = {
			"glDisplay",
			sizeof (glDisplay),
			sizeof (glDisplayClass),
			(GtkClassInitFunc) gl_display_class_init,
			(GtkObjectInitFunc) gl_display_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
		};

		display_type =
		    gtk_type_unique (gtk_vbox_get_type (), &display_info);
	}

	return display_type;
}

static void
gl_display_class_init (glDisplayClass * class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	object_class->destroy = gl_display_destroy;
}

static void
gl_display_init (glDisplay * display)
{
	display->label = NULL;
}

static void
gl_display_destroy (GtkObject * object)
{
	glDisplay *display;
	glDisplayClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_DISPLAY (object));

	display = GL_DISPLAY (object);
	class = GL_DISPLAY_CLASS (GTK_OBJECT (display)->klass);

	display->label = NULL;

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

GtkWidget *
gl_display_new (glLabel * label)
{
	glDisplay *display = gtk_type_new (gl_display_get_type ());

	display->label = label;

	gl_display_construct (display);

	gl_display_clear_modified (display);

	return GTK_WIDGET (display);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                     */
/*---------------------------------------------------------------------------*/
static void
gl_display_construct (glDisplay * display)
{
	GtkWidget *wvbox, *wscroll;

	g_return_if_fail (GL_IS_DISPLAY (display));

	wvbox = GTK_WIDGET (display);

	display->state = GL_DISPLAY_STATE_ARROW;
	display->item_list = NULL;

	gl_display_construct_canvas (display);
	wscroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (wscroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (wvbox), wscroll, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (wscroll), display->canvas);

	gl_display_construct_selection (display);

	display->menu = gl_display_new_selection_menu (display);

	display->modified = FALSE;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create canvas w/ a background in the shape of the label/card.   */
/*---------------------------------------------------------------------------*/
static GtkWidget *
gl_display_construct_canvas (glDisplay * display)
{
	gdouble scale;
	glLabel *label = display->label;
	GList *p_obj;
	glLabelObject *object;
	GnomeCanvasItem *item;

	g_return_val_if_fail (GL_IS_DISPLAY (display), NULL);
	g_return_val_if_fail (label != NULL, NULL);

#ifdef AA_CANVAS
	gtk_widget_push_visual (gdk_rgb_get_visual ());
	gtk_widget_push_colormap (gdk_rgb_get_cmap ());
	display->canvas = gnome_canvas_new_aa ();
	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();
#else
	gtk_widget_push_visual (gdk_imlib_get_visual ());
	gtk_widget_push_colormap (gdk_imlib_get_colormap ());
	display->canvas = gnome_canvas_new ();
	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();
#endif

	scale = get_apropriate_scale (label);

	gtk_widget_set_usize (display->canvas,
			      scale * label->width + 40,
			      scale * label->height + 40);
	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (display->canvas),
					  scale);
	display->scale = scale;

	gnome_canvas_set_scroll_region (GNOME_CANVAS (display->canvas),
					0.0, 0.0, label->width, label->height);

	/* Draw background shape of label/card */
	switch (label->template->style) {

	case GL_TEMPLATE_STYLE_RECT:
		if (label->template->label_round == 0.0) {
			/* Square corners. */
			draw_rect_bg (display);
		} else {
			/* Rounded corners. */
			draw_rounded_rect_bg (display);
		}
		break;

	case GL_TEMPLATE_STYLE_ROUND:
		draw_round_bg (display);
		break;

	case GL_TEMPLATE_STYLE_CD:
		draw_cd_bg (display);
		break;

	default:
		WARN ("Unknown template label style");
		break;
	}

	gtk_signal_connect (GTK_OBJECT (display->canvas), "event",
			    GTK_SIGNAL_FUNC (canvas_event), display);

	for (p_obj = label->objects; p_obj != NULL; p_obj = p_obj->next) {
		object = (glLabelObject *) p_obj->data;
		item = gl_item_new (object, display);
		gl_display_add_item (display, item);
	}

	return display->canvas;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create selection targets.                                       */
/*---------------------------------------------------------------------------*/
static void
gl_display_construct_selection (glDisplay * display)
{
	g_return_if_fail (GL_IS_DISPLAY (display));

	display->have_selection = FALSE;
	display->selection_data = NULL;
	display->invisible = gtk_invisible_new ();

	display->selected_item_list = NULL;

	if (!clipboard_atom) {
		clipboard_atom = gdk_atom_intern ("GLABELS_CLIPBOARD", FALSE);
	}

	gtk_selection_add_target (display->invisible,
				  clipboard_atom, GDK_SELECTION_TYPE_STRING, 1);

	gtk_signal_connect (GTK_OBJECT (display->invisible),
			    "selection_clear_event",
			    GTK_SIGNAL_FUNC (selection_clear_cb), display);

	gtk_signal_connect (GTK_OBJECT (display->invisible), "selection_get",
			    GTK_SIGNAL_FUNC (selection_get_cb), display);

	gtk_signal_connect (GTK_OBJECT (display->invisible),
			    "selection_received",
			    GTK_SIGNAL_FUNC (selection_received_cb), display);

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Determine an apropriate scale for given label & screen size     */
/*---------------------------------------------------------------------------*/
static gdouble
get_apropriate_scale (glLabel * label)
{
	gdouble w, h;
	gdouble w_screen, h_screen;
	gint i;
	gdouble k;

	g_return_val_if_fail (label != NULL, 1.0);

	w = label->width;
	h = label->height;
	w_screen = (gdouble) gdk_screen_width ();
	h_screen = (gdouble) gdk_screen_height ();

	for (i = 0; scales[i] > 0.0; i++) {
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
/* PRIVATE.  Draw simple recangular background.                              */
/*---------------------------------------------------------------------------*/
static void
draw_rect_bg (glDisplay * display)
{
	glLabel *label = display->label;
	GnomeCanvasItem *item;
	GnomeCanvasGroup *group;
	gdouble margin;

	g_return_if_fail (GL_IS_DISPLAY (display));
	g_return_if_fail (label != NULL);

	display->n_bg_items = 0;
	display->bg_item_list = NULL;

	group = gnome_canvas_root (GNOME_CANVAS (display->canvas));

	item = gnome_canvas_item_new (group,
				      gnome_canvas_rect_get_type (),
				      "x1", 0.0,
				      "y1", 0.0,
				      "x2", label->width,
				      "y2", label->height,
				      "fill_color", "white",
				      NULL);
	display->n_bg_items++;
	display->bg_item_list = g_list_append (display->bg_item_list, item);

	/* Bounding box @ margin */
	margin = label->template->label_margin;
	gnome_canvas_item_new (group,
			       gnome_canvas_rect_get_type (),
			       "x1", margin,
			       "y1", margin,
			       "x2", label->width - margin,
			       "y2", label->height - margin,
			       "width_pixels", 1,
			       "outline_color", "light blue",
			       NULL);
	display->n_bg_items++;
	display->bg_item_list = g_list_append (display->bg_item_list, item);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw rounded recangular background.                             */
/*---------------------------------------------------------------------------*/
static void
draw_rounded_rect_bg (glDisplay * display)
{
	glLabel *label = display->label;
	GnomeCanvasPoints *points;
	gint i_coords, i_theta;
	gdouble r, w, h, m;
	GnomeCanvasItem *item;
	GnomeCanvasGroup *group;

	g_return_if_fail (GL_IS_DISPLAY (display));
	g_return_if_fail (label != NULL);

	group = gnome_canvas_root (GNOME_CANVAS (display->canvas));

	display->n_bg_items = 0;
	display->bg_item_list = NULL;

	r = label->template->label_round;
	w = label->width;
	h = label->height;
	m = label->template->label_margin;

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
	item = gnome_canvas_item_new (group,
				      gnome_canvas_polygon_get_type (),
				      "points", points,
				      "fill_color", "white",
				      NULL);
	gnome_canvas_points_free (points);
	display->n_bg_items++;
	display->bg_item_list = g_list_append (display->bg_item_list, item);

	/* Bounding box @ margin */
	if (label->template->label_margin >= label->template->label_round) {
		/* simple rectangle */
		item = gnome_canvas_item_new (group,
					      gnome_canvas_rect_get_type (),
					      "x1", m,
					      "y1", m,
					      "x2", w - m,
					      "y2", h - m,
					      "width_pixels", 1,
					      "outline_color", "light blue",
					      NULL);
		display->n_bg_items++;
		display->bg_item_list =
		    g_list_append (display->bg_item_list, item);
	} else {
		r = label->template->label_round - m;
		w = label->width - 2 * label->template->label_margin;
		h = label->height - 2 * label->template->label_margin;

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
		item = gnome_canvas_item_new (group,
					      gnome_canvas_polygon_get_type (),
					      "points", points,
					      "width_pixels", 1,
					      "outline_color", "light blue",
					      NULL);
		gnome_canvas_points_free (points);
		display->n_bg_items++;
		display->bg_item_list =
		    g_list_append (display->bg_item_list, item);
	}
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw round background.                                          */
/*---------------------------------------------------------------------------*/
static void
draw_round_bg (glDisplay * display)
{
	glLabel *label = display->label;
	GnomeCanvasPoints *points;
	gint i_coords, i_theta;
	gdouble r, r1;
	GnomeCanvasItem *item;
	GnomeCanvasGroup *group;

	g_return_if_fail (GL_IS_DISPLAY (display));
	g_return_if_fail (label != NULL);

	group = gnome_canvas_root (GNOME_CANVAS (display->canvas));

	display->n_bg_items = 0;
	display->bg_item_list = NULL;

	r1 = label->template->label_radius;
	points = gnome_canvas_points_new (1 + 360/2);
	i_coords = 0;
	for (i_theta = 0; i_theta <= 360; i_theta += 2) {
		points->coords[i_coords++] =
		    r1 - r1 * sin (i_theta * M_PI / 180.0);
		points->coords[i_coords++] =
		    r1 - r1 * cos (i_theta * M_PI / 180.0);
	}
	item = gnome_canvas_item_new (group,
				      gnome_canvas_polygon_get_type (),
				      "points", points,
				      "fill_color", "white",
				      NULL);
	gnome_canvas_points_free (points);
	display->n_bg_items++;
	display->bg_item_list = g_list_append (display->bg_item_list, item);

	/* Bounding box @ margin */
	r = label->template->label_radius - label->template->label_margin;
	points = gnome_canvas_points_new (360 / 2);
	i_coords = 0;
	for (i_theta = 0; i_theta < 360; i_theta += 2) {
		points->coords[i_coords++] =
		    r1 - r * sin (i_theta * M_PI / 180.0);
		points->coords[i_coords++] =
		    r1 - r * cos (i_theta * M_PI / 180.0);
	}
	item = gnome_canvas_item_new (group,
				      gnome_canvas_polygon_get_type (),
				      "points", points,
				      "width_pixels", 1,
				      "outline_color", "light blue", NULL);
	gnome_canvas_points_free (points);
	display->n_bg_items++;
	display->bg_item_list = g_list_append (display->bg_item_list, item);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Draw CD style background, circular w/ concentric hole.          */
/*---------------------------------------------------------------------------*/
static void
draw_cd_bg (glDisplay * display)
{
	glLabel *label = display->label;
	GnomeCanvasPoints *points;
	gint i_coords, i_theta;
	gdouble r, r1, r2;
	GnomeCanvasItem *item;
	GnomeCanvasGroup *group;

	g_return_if_fail (GL_IS_DISPLAY (display));
	g_return_if_fail (label != NULL);

	group = gnome_canvas_root (GNOME_CANVAS (display->canvas));

	display->n_bg_items = 0;
	display->bg_item_list = NULL;

	r1 = label->template->label_radius;
	r2 = label->template->label_hole;
	points = gnome_canvas_points_new (2 * (1 + 360 / 2));
	i_coords = 0;
	for (i_theta = 0; i_theta <= 360; i_theta += 2) {
		points->coords[i_coords++] =
		    r1 - r1 * sin (i_theta * M_PI / 180.0);
		points->coords[i_coords++] =
		    r1 - r1 * cos (i_theta * M_PI / 180.0);
	}
	for (i_theta = 0; i_theta <= 360; i_theta += 2) {
		points->coords[i_coords++] =
		    r1 - r2 * sin (i_theta * M_PI / 180.0);
		points->coords[i_coords++] =
		    r1 - r2 * cos (i_theta * M_PI / 180.0);
	}
	item = gnome_canvas_item_new (group,
				      gnome_canvas_polygon_get_type (),
				      "points", points,
				      "fill_color", "white",
				      NULL);
	gnome_canvas_points_free (points);
	display->n_bg_items++;
	display->bg_item_list = g_list_append (display->bg_item_list, item);

	/* Bounding box @ margin */
	/* outer margin */
	r = label->template->label_radius - label->template->label_margin;
	points = gnome_canvas_points_new (360 / 2);
	i_coords = 0;
	for (i_theta = 0; i_theta < 360; i_theta += 2) {
		points->coords[i_coords++] =
		    r1 - r * sin (i_theta * M_PI / 180.0);
		points->coords[i_coords++] =
		    r1 - r * cos (i_theta * M_PI / 180.0);
	}
	item = gnome_canvas_item_new (group,
				      gnome_canvas_polygon_get_type (),
				      "points", points,
				      "width_pixels", 1,
				      "outline_color", "light blue", NULL);
	gnome_canvas_points_free (points);
	display->n_bg_items++;
	display->bg_item_list = g_list_append (display->bg_item_list, item);

	/* inner margin */
	r = label->template->label_hole + label->template->label_margin;
	points = gnome_canvas_points_new (360 / 2);
	i_coords = 0;
	for (i_theta = 0; i_theta < 360; i_theta += 2) {
		points->coords[i_coords++] =
		    r1 - r * sin (i_theta * M_PI / 180.0);
		points->coords[i_coords++] =
		    r1 - r * cos (i_theta * M_PI / 180.0);
	}
	item = gnome_canvas_item_new (group,
				      gnome_canvas_polygon_get_type (),
				      "points", points,
				      "width_pixels", 1,
				      "outline_color", "light blue",
				      NULL);
	gnome_canvas_points_free (points);
	display->n_bg_items++;
	display->bg_item_list = g_list_append (display->bg_item_list, item);
}

/*****************************************************************************/
/* Set arrow mode.                                                           */
/*****************************************************************************/
void
gl_display_arrow_mode (glDisplay * display)
{
	static GdkCursor *cursor = NULL;

	g_return_if_fail (GL_IS_DISPLAY (display));

	if (!cursor) {
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
	}

	gdk_window_set_cursor (display->canvas->window, cursor);

	display->state = GL_DISPLAY_STATE_ARROW;
}

/*****************************************************************************/
/* Set create text object mode.                                              */
/*****************************************************************************/
void
gl_display_object_create_mode (glDisplay * display,
			       glLabelObjectType type)
{
	GdkCursor *cursor;

	g_return_if_fail (GL_IS_DISPLAY (display));

	cursor = gl_item_get_create_cursor (type);
	gdk_window_set_cursor (display->canvas->window, cursor);

	display->state = GL_DISPLAY_STATE_OBJECT_CREATE;
	display->create_type = type;
}

/*****************************************************************************/
/* Add canvas item to list of display items.                                 */
/*****************************************************************************/
void
gl_display_add_item (glDisplay * display,
		     GnomeCanvasItem * item)
{
	g_return_if_fail (GL_IS_DISPLAY (display));

	display->item_list = g_list_prepend (display->item_list, item);
}

/*****************************************************************************/
/* Select all items.                                                         */
/*****************************************************************************/
void
gl_display_select_all (glDisplay * display)
{
	GList *p;

	g_return_if_fail (GL_IS_DISPLAY (display));

	gl_display_unselect_all (display);

	for (p = display->item_list; p != NULL; p = p->next) {
		gl_display_select_item (display, GNOME_CANVAS_ITEM (p->data));
	}
}

/*****************************************************************************/
/* Select all items within given rectangular region                          */
/*****************************************************************************/
void
gl_display_select_region (glDisplay * display,
			  gdouble x1,
			  gdouble y1,
			  gdouble x2,
			  gdouble y2)
{
	GList *p;
	GnomeCanvasItem *item;
	gdouble i_x1, i_y1, i_x2, i_y2;

	g_return_if_fail (GL_IS_DISPLAY (display));
	g_return_if_fail ((x1 <= x2) && (y1 <= y2));

	for (p = display->item_list; p != NULL; p = p->next) {
		item = GNOME_CANVAS_ITEM (p->data);
		if (!item_selected (display, item)) {

			gl_item_get_bounds (item, &i_x1, &i_y1, &i_x2, &i_y2);
			if ((i_x1 >= x1) && (i_x2 <= x2) && (i_y1 >= y1)
			    && (i_y2 <= y2)) {
				gl_display_select_item (display, item);
			}

		}
	}
}

/*****************************************************************************/
/* Remove all selections                                                     */
/*****************************************************************************/
void
gl_display_unselect_all (glDisplay * display)
{
	GList *p, *p_next;

	g_return_if_fail (GL_IS_DISPLAY (display));

	for (p = display->selected_item_list; p != NULL; p = p_next) {
		p_next = p->next;
		gl_display_unselect_item (display, GNOME_CANVAS_ITEM (p->data));
	}
}

/*****************************************************************************/
/* Select an item.                                                           */
/*****************************************************************************/
void
gl_display_select_item (glDisplay * display,
			GnomeCanvasItem * item)
{
	g_return_if_fail (GL_IS_DISPLAY (display));
	g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));

	if (!item_selected (display, item)) {
		display->selected_item_list =
		    g_list_prepend (display->selected_item_list, item);
	}
	gl_item_highlight (item);
	gtk_widget_grab_focus (GTK_WIDGET (display->canvas));
}

/*****************************************************************************/
/* Un-select items.                                                          */
/*****************************************************************************/
void
gl_display_unselect_item (glDisplay * display,
			  GnomeCanvasItem * item)
{
	g_return_if_fail (GL_IS_DISPLAY (display));
	g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));

	gl_item_unhighlight (item);

	display->selected_item_list =
	    g_list_remove (display->selected_item_list, item);
}

/*****************************************************************************/
/* Has display been modified?                                                */
/*****************************************************************************/
gboolean
gl_display_modified (glDisplay * display)
{
	g_return_val_if_fail (GL_IS_DISPLAY (display), FALSE);

	return display->modified;
}

/*****************************************************************************/
/* Set to modified state.                                                    */
/*****************************************************************************/
void
gl_display_set_modified (glDisplay * display)
{
	g_return_if_fail (GL_IS_DISPLAY (display));

	display->modified = TRUE;
}

/*****************************************************************************/
/* Reset to un-modified state.                                               */
/*****************************************************************************/
void
gl_display_clear_modified (glDisplay * display)
{
	g_return_if_fail (GL_IS_DISPLAY (display));

	display->modified = FALSE;
}

/*****************************************************************************/
/* "Cut" selected items and place in clipboard selections.                   */
/*****************************************************************************/
void
gl_display_cut (glDisplay * display)
{
	g_return_if_fail (GL_IS_DISPLAY (display));

	gl_display_copy (display);
	delete_selection_cb (GTK_WIDGET (display), display);
}

/*****************************************************************************/
/* "Copy" selected items to clipboard selections.                            */
/*****************************************************************************/
void
gl_display_copy (glDisplay * display)
{
	GList *p;
	GnomeCanvasItem *item;
	glLabelObject *object;

	g_return_if_fail (GL_IS_DISPLAY (display));

	if (display->selected_item_list) {

		gl_label_free (&display->selection_data);
		display->selection_data =
		    gl_label_new_with_template (display->label->
						template_name,
						display->label->rotate_flag);

		for (p = display->selected_item_list; p != NULL; p = p->next) {

			item = GNOME_CANVAS_ITEM (p->data);
			object = gl_item_get_object (item);
			gl_label_object_new_from_object (display->
							 selection_data,
							 object);

		}

		gtk_selection_owner_set (display->invisible,
					 clipboard_atom, GDK_CURRENT_TIME);
		display->have_selection = TRUE;

	}
}

/*****************************************************************************/
/* "Paste" from private clipboard selection.                                 */
/*****************************************************************************/
void
gl_display_paste (glDisplay * display)
{
	g_return_if_fail (GL_IS_DISPLAY (display));

	gtk_selection_convert (GTK_WIDGET (display->invisible),
			       clipboard_atom, GDK_SELECTION_TYPE_STRING,
			       GDK_CURRENT_TIME);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Canvas event handler.                                           */
/*---------------------------------------------------------------------------*/
static int
canvas_event (GnomeCanvas * canvas,
	      GdkEvent * event,
	      gpointer data)
{
	glDisplay *display = GL_DISPLAY (data);

	switch (display->state) {

	case GL_DISPLAY_STATE_ARROW:
		return canvas_event_arrow_mode (canvas, event, data);

	case GL_DISPLAY_STATE_OBJECT_CREATE:
		return gl_item_create_event_handler (canvas, event, data);

	default:
		WARN ("Invalid display state.");	/* Should not happen! */
		return FALSE;

	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Canvas event handler (arrow mode)                               */
/*---------------------------------------------------------------------------*/
static int
canvas_event_arrow_mode (GnomeCanvas * canvas,
			 GdkEvent * event,
			 gpointer data)
{
	static gdouble x0, y0;
	static gboolean dragging = FALSE;
	static GnomeCanvasItem *item;
	glDisplay *display = GL_DISPLAY (data);
	gdouble x, y, x1, y1, x2, y2;
	GnomeCanvasGroup *group;
	GdkCursor *cursor;

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			gnome_canvas_window_to_world (canvas,
						      event->button.x,
						      event->button.y, &x, &y);

			if (display_item_at (display, x, y) == NULL) {
				if (!(event->button.state & GDK_CONTROL_MASK)) {
					gl_display_unselect_all (display);
				}

				dragging = TRUE;
				gdk_pointer_grab (GTK_WIDGET (display->canvas)->
						  window, FALSE,
						  GDK_POINTER_MOTION_MASK |
						  GDK_BUTTON_RELEASE_MASK |
						  GDK_BUTTON_PRESS_MASK, NULL,
						  NULL, event->button.time);
				group =
				    gnome_canvas_root (GNOME_CANVAS
						       (display->canvas));
				item =
				    gnome_canvas_item_new (group,
							   gnome_canvas_rect_get_type (),
							   "x1", x, "y1", y,
							   "x2", x, "y2", y,
							   "width_pixels", 2,
							   "outline_color_rgba",
							   GNOME_CANVAS_COLOR_A
							   (0, 0, 255, 128),
							   NULL);
				x0 = x;
				y0 = y;

			}
			return FALSE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			if (dragging) {
				dragging = FALSE;
				gdk_pointer_ungrab (event->button.time);
				gnome_canvas_window_to_world (canvas,
							      event->button.x,
							      event->button.y,
							      &x, &y);
				x1 = MIN (x, x0);
				y1 = MIN (y, y0);
				x2 = MAX (x, x0);
				y2 = MAX (y, y0);
				gl_display_select_region (display, x1, y1, x2,
							  y2);
				gtk_object_destroy (GTK_OBJECT (item));
				return TRUE;
			}
			return FALSE;

		default:
			return FALSE;
		}

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			gnome_canvas_window_to_world (canvas,
						      event->motion.x,
						      event->motion.y, &x, &y);

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
		if (!dragging) {
			switch (event->key.keyval) {
			case GDK_Left:
			case GDK_KP_Left:
				move_selected_items (display,
						     -1.0 / (display->scale),
						     0.0);
				break;
			case GDK_Up:
			case GDK_KP_Up:
				move_selected_items (display, 0.0,
						     -1.0 / (display->scale));
				break;
			case GDK_Right:
			case GDK_KP_Right:
				move_selected_items (display,
						     1.0 / (display->scale),
						     0.0);
				break;
			case GDK_Down:
			case GDK_KP_Down:
				move_selected_items (display, 0.0,
						     1.0 / (display->scale));
				break;
			case GDK_Delete:
			case GDK_KP_Delete:
				delete_selection_cb (GTK_WIDGET (canvas),
						     display);
				cursor = gdk_cursor_new (GDK_LEFT_PTR);
				gdk_window_set_cursor (display->canvas->window,
						       cursor);
				gdk_cursor_destroy (cursor);
				break;
			default:
				return FALSE;
			}
		}
		return TRUE;	/* We handled this or we were dragging. */

	default:
		return FALSE;
	}

}

/*****************************************************************************/
/* Item event handler.                                                       */
/*****************************************************************************/
gint
gl_display_item_event_handler (GnomeCanvasItem * item,
			       GdkEvent * event,
			       gpointer data)
{
	glDisplay *display = GL_DISPLAY (data);

	switch (display->state) {

	case GL_DISPLAY_STATE_ARROW:
		return item_event_arrow_mode (item, event, data);

	default:
		return FALSE;

	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Item event handler (arrow mode)                                 */
/*---------------------------------------------------------------------------*/
static int
item_event_arrow_mode (GnomeCanvasItem * item,
		       GdkEvent * event,
		       gpointer data)
{
	glDisplay *display = GL_DISPLAY (data);
	static gdouble x, y;
	static gboolean dragging = FALSE;
	GdkCursor *cursor;
	gdouble item_x, item_y;
	gdouble new_x, new_y;
	gboolean control_key_pressed;

	item_x = event->button.x;
	item_y = event->button.y;
	gnome_canvas_item_w2i (item->parent, &item_x, &item_y);

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		control_key_pressed = event->button.state & GDK_CONTROL_MASK;
		switch (event->button.button) {
		case 1:
			if (control_key_pressed) {
				if (item_selected (display, item)) {
					/* Un-selecting an already selected item */
					gl_display_unselect_item (display,
								  item);
					return TRUE;
				} else {
					/* Add to current selection */
					gl_display_select_item (display, item);
				}
			} else {
				if (!item_selected (display, item)) {
					/* No control, key so remove any selections before adding */
					gl_display_unselect_all (display);
					/* Add to current selection */
					gl_display_select_item (display, item);
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
			gdk_cursor_destroy (cursor);
			dragging = TRUE;
			return TRUE;

		case 3:
			if (!item_selected (display, item)) {
				if (!control_key_pressed) {
					/* No control, key so remove any selections before adding */
					gl_display_unselect_all (display);
				}
			}
			/* Add to current selection */
			gl_display_select_item (display, item);
			/* bring up apropriate menu for selection. */
			popup_selection_menu (display, item, event);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_BUTTON_RELEASE:
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
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			/* Dragging mode, move selection */
			new_x = item_x;
			new_y = item_y;
			move_selected_items (display, (new_x - x), (new_y - y));
			x = new_x;
			y = new_y;
			return TRUE;
		} else {
			return FALSE;
		}

	case GDK_2BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			/* Also exit dragging mode on a a double-click, bring up menu */
			gnome_canvas_item_ungrab (item, event->button.time);
			dragging = FALSE;
			gl_display_select_item (display, item);
			gl_item_edit_dialog (item);
			return TRUE;

		default:
			return FALSE;
		}

	case GDK_ENTER_NOTIFY:
		cursor = gdk_cursor_new (GDK_FLEUR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	case GDK_LEAVE_NOTIFY:
		cursor = gdk_cursor_new (GDK_LEFT_PTR);
		gdk_window_set_cursor (display->canvas->window, cursor);
		gdk_cursor_destroy (cursor);
		return TRUE;

	default:
		return FALSE;
	}

}

/*****************************************************************************/
/* create menu for multiple selections.                                      */
/*****************************************************************************/
GtkWidget *
gl_display_new_selection_menu (glDisplay * display)
{
	GtkWidget *menu, *menuitem;

	g_return_val_if_fail (GL_IS_DISPLAY (display), NULL);

	menu = gtk_menu_new ();

	menuitem = gtk_menu_item_new_with_label (_("Delete"));
	gtk_menu_append (GTK_MENU (menu), menuitem);
	gtk_widget_show (menuitem);
	gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			    GTK_SIGNAL_FUNC (delete_selection_cb), display);

	menuitem = gtk_menu_item_new ();
	gtk_menu_append (GTK_MENU (menu), menuitem);
	gtk_widget_show (menuitem);

	menuitem = gtk_menu_item_new_with_label (_("Bring to front"));
	gtk_menu_append (GTK_MENU (menu), menuitem);
	gtk_widget_show (menuitem);
	gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			    GTK_SIGNAL_FUNC (raise_selection_cb), display);

	menuitem = gtk_menu_item_new_with_label (_("Send to back"));
	gtk_menu_append (GTK_MENU (menu), menuitem);
	gtk_widget_show (menuitem);
	gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			    GTK_SIGNAL_FUNC (lower_selection_cb), display);

	return menu;
}

/*****************************************************************************/
/* create menu for given item.                                               */
/*****************************************************************************/
GtkWidget *
gl_display_new_item_menu (GnomeCanvasItem * item)
{
	GtkWidget *menu, *menuitem;

	g_return_val_if_fail (GNOME_IS_CANVAS_ITEM (item), NULL);

	menu = gtk_menu_new ();

	menuitem = gtk_menu_item_new_with_label (_("Edit properties..."));
	gtk_menu_append (GTK_MENU (menu), menuitem);
	gtk_widget_show (menuitem);
	gtk_signal_connect_object (GTK_OBJECT (menuitem), "activate",
				   GTK_SIGNAL_FUNC (gl_item_edit_dialog),
				   GTK_OBJECT (item));

	menuitem = gtk_menu_item_new ();
	gtk_menu_append (GTK_MENU (menu), menuitem);
	gtk_widget_show (menuitem);

	menuitem = gtk_menu_item_new_with_label (_("Delete"));
	gtk_menu_append (GTK_MENU (menu), menuitem);
	gtk_widget_show (menuitem);
	gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			    GTK_SIGNAL_FUNC (delete_item_cb), item);

	menuitem = gtk_menu_item_new ();
	gtk_menu_append (GTK_MENU (menu), menuitem);
	gtk_widget_show (menuitem);

	menuitem = gtk_menu_item_new_with_label (_("Bring to front"));
	gtk_menu_append (GTK_MENU (menu), menuitem);
	gtk_widget_show (menuitem);
	gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			    GTK_SIGNAL_FUNC (raise_item_cb), item);

	menuitem = gtk_menu_item_new_with_label (_("Send to back"));
	gtk_menu_append (GTK_MENU (menu), menuitem);
	gtk_widget_show (menuitem);
	gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			    GTK_SIGNAL_FUNC (lower_item_cb), item);

	gtk_object_set_data (GTK_OBJECT (item), "object_menu", menu);

	return menu;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  popup menu for given item.                                      */
/*---------------------------------------------------------------------------*/
static void
popup_selection_menu (glDisplay * display,
		      GnomeCanvasItem * item,
		      GdkEvent * event)
{
	GtkWidget *menu;

	g_return_if_fail (GL_IS_DISPLAY (display));
	g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));

	if (multiple_items_selected (display)) {
		if (display->menu != NULL) {
			gtk_menu_popup (GTK_MENU (display->menu),
					NULL, NULL, NULL, NULL,
					event->button.button,
					event->button.time);
		}
	} else {

		menu = gl_item_get_menu (item);
		if (menu != NULL) {
			gtk_menu_popup (GTK_MENU (menu),
					NULL, NULL, NULL, NULL,
					event->button.button,
					event->button.time);
		}

	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  delete selection callback.                                      */
/*---------------------------------------------------------------------------*/
static void
delete_selection_cb (GtkWidget * widget,
		     glDisplay * display)
{
	GList *p, *p_next;

	g_return_if_fail (GL_IS_DISPLAY (display));

	for (p = display->selected_item_list; p != NULL; p = p_next) {
		p_next = p->next;
		delete_item_cb (widget, GNOME_CANVAS_ITEM (p->data));
	}
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  raise item to front callback.                                   */
/*---------------------------------------------------------------------------*/
static void
raise_selection_cb (GtkWidget * widget,
		    glDisplay * display)
{
	GList *p;

	g_return_if_fail (GL_IS_DISPLAY (display));

	for (p = display->selected_item_list; p != NULL; p = p->next) {
		raise_item_cb (widget, GNOME_CANVAS_ITEM (p->data));
	}
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  lower item to back callback.                                    */
/*---------------------------------------------------------------------------*/
static void
lower_selection_cb (GtkWidget * widget,
		    glDisplay * display)
{
	GList *p;

	g_return_if_fail (GL_IS_DISPLAY (display));

	for (p = display->selected_item_list; p != NULL; p = p->next) {
		lower_item_cb (widget, GNOME_CANVAS_ITEM (p->data));
	}
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  delete item callback.                                           */
/*---------------------------------------------------------------------------*/
static void
delete_item_cb (GtkWidget * widget,
		GnomeCanvasItem * item)
{
	glDisplay *display;

	g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));

	display = gl_item_get_display (item);

	gl_display_unselect_item (display, item);
	display->item_list = g_list_remove (display->item_list, item);

	gl_item_free (&item);

	gl_display_set_modified (display);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  raise item to front callback.                                   */
/*---------------------------------------------------------------------------*/
static void
raise_item_cb (GtkWidget * widget,
	       GnomeCanvasItem * item)
{
	glLabelObject *object;

	g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));

	object = gl_item_get_object (item);
	gl_label_object_raise_to_front (object);

	gnome_canvas_item_raise_to_top (item);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  lower item to back callback.                                    */
/*---------------------------------------------------------------------------*/
static void
lower_item_cb (GtkWidget * widget,
	       GnomeCanvasItem * item)
{
	glLabelObject *object;
	glDisplay *display;

	g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));

	object = gl_item_get_object (item);
	display = gl_item_get_display (item);

	gl_label_object_lower_to_back (object);

	/* Send to bottom */
	gnome_canvas_item_lower_to_bottom (item);

	/* now raise it above all items that form the backgound */
	gnome_canvas_item_raise (item, display->n_bg_items);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  move selected items                                             */
/*---------------------------------------------------------------------------*/
static void
move_selected_items (glDisplay * display,
		     gdouble dx,
		     gdouble dy)
{
	GList *p;
	GnomeCanvasItem *item;

	g_return_if_fail (GL_IS_DISPLAY (display));

	for (p = display->selected_item_list; p != NULL; p = p->next) {

		item = GNOME_CANVAS_ITEM (p->data);

		move_item (item, dx, dy);
		gl_item_highlight (item);

	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  move item/object                                                */
/*---------------------------------------------------------------------------*/
static void
move_item (GnomeCanvasItem * item,
	   gdouble dx,
	   gdouble dy)
{
	glLabelObject *object;
	glDisplay *display;

	g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));

	object = gl_item_get_object (item);

	object->x += dx;
	object->y += dy;

	gnome_canvas_item_move (item, dx, dy);

	display = gl_item_get_display (item);
	gl_display_set_modified (display);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE. Return item at (x,y) if it is in our list of managed items.      */
/*---------------------------------------------------------------------------*/
static GnomeCanvasItem *
display_item_at (glDisplay * display,
		 gdouble x,
		 gdouble y)
{
	GnomeCanvasItem *item;

	g_return_val_if_fail (GL_IS_DISPLAY (display), NULL);

	item = gnome_canvas_get_item_at (GNOME_CANVAS (display->canvas), x, y);

	/* No item is at x, y */
	if (item == NULL)
		return NULL;

	/* Don't include our background items */
	if (g_list_find (display->bg_item_list, item) != NULL)
		return NULL;

	return item;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Is the item in our current selection?                           */
/*---------------------------------------------------------------------------*/
static gboolean
item_selected (glDisplay * display,
	       GnomeCanvasItem * item)
{
	g_return_val_if_fail (GL_IS_DISPLAY (display), FALSE);
	g_return_val_if_fail (GNOME_IS_CANVAS_ITEM (item), FALSE);

	if (g_list_find (display->selected_item_list, item) == NULL) {
		return FALSE;
	}
	return TRUE;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Are there multiple items in our current selection?              */
/*---------------------------------------------------------------------------*/
static gboolean
multiple_items_selected (glDisplay * display)
{
	g_return_val_if_fail (GL_IS_DISPLAY (display), FALSE);

	if (display->selected_item_list == NULL)
		return FALSE;
	if (display->selected_item_list->next == NULL)
		return FALSE;
	return TRUE;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Handle "selection-clear" signal.                                */
/*---------------------------------------------------------------------------*/
static void
selection_clear_cb (GtkWidget * widget,
		    GdkEventSelection * event,
		    gpointer data)
{
	glDisplay *display = GL_DISPLAY (data);

	g_return_if_fail (GL_IS_DISPLAY (display));

	display->have_selection = FALSE;
	gl_label_free (&display->selection_data);
	display->selection_data = NULL;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Handle "selection-get" signal.                                  */
/*---------------------------------------------------------------------------*/
static void
selection_get_cb (GtkWidget * widget,
		  GtkSelectionData * selection_data,
		  guint info,
		  guint time,
		  gpointer data)
{
	glDisplay *display = GL_DISPLAY (data);
	gchar *buffer;

	g_return_if_fail (GL_IS_DISPLAY (display));

	if (display->have_selection) {

		gl_label_save_xml_buffer (display->selection_data, &buffer);
		gtk_selection_data_set (selection_data,
					GDK_SELECTION_TYPE_STRING, 8, buffer,
					strlen (buffer));
		g_free (buffer);
	}
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Handle "selection-received" signal.  (Result of Paste)          */
/*---------------------------------------------------------------------------*/
static void
selection_received_cb (GtkWidget * widget,
		       GtkSelectionData * selection_data,
		       guint time,
		       gpointer data)
{
	glDisplay *display = GL_DISPLAY (data);
	glLabel *label = NULL;
	GList *p;
	glLabelObject *object, *newobject;
	GnomeCanvasItem *item;

	g_return_if_fail (GL_IS_DISPLAY (display));

	if (selection_data->length < 0) {
		return;
	}
	if (selection_data->type != GDK_SELECTION_TYPE_STRING) {
		return;
	}

	gl_display_unselect_all (display);

	gl_label_open_xml_buffer (&label, selection_data->data);
	for (p = label->objects; p != NULL; p = p->next) {
		object = (glLabelObject *) p->data;
		newobject =
		    gl_label_object_new_from_object (display->label, object);
		item = gl_item_new (newobject, display);
		gl_display_add_item (display, item);
		gl_display_select_item (display, item);
	}
	gl_label_free (&label);

	gl_display_set_modified (display);
}

/*****************************************************************************/
/* Zoom in one "notch"                                                       */
/*****************************************************************************/
void
gl_display_zoom_in (glDisplay * display)
{
	gint i, i_min;
	gdouble dist, dist_min;

	g_return_if_fail (GL_IS_DISPLAY (display));

	/* Find index of current scale (or best match) */
	i_min = 1;		/* start with 2nd largest scale */
	dist_min = fabs (scales[1] - display->scale);
	for (i = 2; scales[i] != 0.0; i++) {
		dist = fabs (scales[i] - display->scale);
		if (dist < dist_min) {
			i_min = i;
			dist_min = dist;
		}
	}

	/* zoom in one "notch" */
	i = MAX (0, i_min - 1);
	gl_display_set_zoom (display, scales[i] / HOME_SCALE);
}

/*****************************************************************************/
/* Zoom out one "notch"                                                      */
/*****************************************************************************/
void
gl_display_zoom_out (glDisplay * display)
{
	gint i, i_min;
	gdouble dist, dist_min;

	g_return_if_fail (GL_IS_DISPLAY (display));

	/* Find index of current scale (or best match) */
	i_min = 0;		/* start with largest scale */
	dist_min = fabs (scales[0] - display->scale);
	for (i = 1; scales[i] != 0.0; i++) {
		dist = fabs (scales[i] - display->scale);
		if (dist < dist_min) {
			i_min = i;
			dist_min = dist;
		}
	}

	/* zoom out one "notch" */
	if (scales[i_min] == 0.0)
		return;
	i = i_min + 1;
	if (scales[i] == 0.0)
		return;
	gl_display_set_zoom (display, scales[i] / HOME_SCALE);

}

/*****************************************************************************/
/* Set current zoom factor to explicit value.                                */
/*****************************************************************************/
void
gl_display_set_zoom (glDisplay * display,
		     gdouble scale)
{
	g_return_if_fail (GL_IS_DISPLAY (display));
	g_return_if_fail (scale > 0.0);

	display->scale = scale * HOME_SCALE;
	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (display->canvas),
					  scale * HOME_SCALE);
}

/*****************************************************************************/
/* Get current zoom factor.                                                  */
/*****************************************************************************/
gdouble
gl_display_get_zoom (glDisplay * display)
{
	g_return_val_if_fail (GL_IS_DISPLAY (display), 1.0);

	return display->scale / HOME_SCALE;
}
