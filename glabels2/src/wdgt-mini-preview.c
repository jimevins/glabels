/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_mini_preview.c:  mini preview widget module
 *
 *  Copyright (C) 2001  Jim Evins <evins@snaught.com>.
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

#include <math.h>

#include "wdgt-mini-preview.h"
#include "marshal.h"
#include "color.h"

#include "debug.h"

#define WDGT_MINI_PREVIEW_MAX_PIXELS 175
#define SHADOW_X_OFFSET 5
#define SHADOW_Y_OFFSET 5

#define RES 5 /* Resolution in degrees for Business Card CD outlines */

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CLICKED,
	PRESSED,
	LAST_SIGNAL
};


/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkContainerClass *parent_class;

static gint wdgt_mini_preview_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_wdgt_mini_preview_class_init    (glWdgtMiniPreviewClass * class);
static void gl_wdgt_mini_preview_instance_init (glWdgtMiniPreview * preview);
static void gl_wdgt_mini_preview_finalize      (GObject * object);

static void gl_wdgt_mini_preview_construct     (glWdgtMiniPreview * preview,
						gint height, gint width);

static GList *mini_outline_list_new            (GnomeCanvas *canvas,
						glTemplate *template);
static void mini_outline_list_free             (GList ** list);

static gint canvas_event_cb                    (GnomeCanvas * canvas,
						GdkEvent * event,
						gpointer data);

static GnomeCanvasItem *cdbc_item              (GnomeCanvasGroup *group,
						gdouble           x1,
						gdouble           y1,
						glTemplate       *template);

static void style_set_cb                       (GtkWidget        *widget,
						GtkStyle         *previous_style,
						gpointer data);




/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
guint
gl_wdgt_mini_preview_get_type (void)
{
	static guint wdgt_mini_preview_type = 0;

	if (!wdgt_mini_preview_type) {
		GTypeInfo wdgt_mini_preview_info = {
			sizeof (glWdgtMiniPreviewClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_wdgt_mini_preview_class_init,
			NULL,
			NULL,
			sizeof (glWdgtMiniPreview),
			0,
			(GInstanceInitFunc) gl_wdgt_mini_preview_instance_init,
		};

		wdgt_mini_preview_type =
			g_type_register_static (gtk_hbox_get_type (),
						"glWdgtMiniPreview",
						&wdgt_mini_preview_info, 0);
	}

	return wdgt_mini_preview_type;
}

static void
gl_wdgt_mini_preview_class_init (glWdgtMiniPreviewClass * class)
{
	GObjectClass *object_class;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	object_class = (GObjectClass *) class;

	parent_class = gtk_type_class (gtk_hbox_get_type ());

	object_class->finalize = gl_wdgt_mini_preview_finalize;

	wdgt_mini_preview_signals[CLICKED] =
	    g_signal_new ("clicked",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glWdgtMiniPreviewClass, clicked),
			  NULL, NULL,
			  gl_marshal_VOID__INT,
			  G_TYPE_NONE, 1, G_TYPE_INT);

	wdgt_mini_preview_signals[PRESSED] =
	    g_signal_new ("pressed",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glWdgtMiniPreviewClass, pressed),
			  NULL, NULL,
			  gl_marshal_VOID__INT_INT,
			  G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_INT);

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

static void
gl_wdgt_mini_preview_instance_init (glWdgtMiniPreview * preview)
{
	gl_debug (DEBUG_MINI_PREVIEW, "START");

	preview->canvas = NULL;
	preview->label_items = NULL;

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

static void
gl_wdgt_mini_preview_finalize (GObject * object)
{
	glWdgtMiniPreview *preview;
	glWdgtMiniPreviewClass *class;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WDGT_MINI_PREVIEW (object));

	preview = GL_WDGT_MINI_PREVIEW (object);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

GtkWidget *
gl_wdgt_mini_preview_new (gint height,
			  gint width)
{
	glWdgtMiniPreview *preview;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	preview = g_object_new (gl_wdgt_mini_preview_get_type (), NULL);

	gl_wdgt_mini_preview_construct (preview, height, width);

	gl_debug (DEBUG_MINI_PREVIEW, "END");

	return GTK_WIDGET (preview);
}

/*--------------------------------------------------------------------------*/
/* Construct composite widget.                                              */
/*--------------------------------------------------------------------------*/
static void
gl_wdgt_mini_preview_construct (glWdgtMiniPreview * preview,
				gint height,
				gint width)
{
	GtkWidget        *whbox;
	GnomeCanvasGroup *group;
	GtkStyle         *style;
	guint             shadow_color;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	whbox = GTK_WIDGET (preview);

	preview->height = height;
	preview->width  = width;

	/* create canvas */
	gtk_widget_push_colormap (gdk_rgb_get_colormap ());
	preview->canvas = gnome_canvas_new_aa ();
	gtk_widget_pop_colormap ();
	gtk_box_pack_start (GTK_BOX (whbox), preview->canvas, TRUE, TRUE, 0);
	gtk_widget_set_size_request (preview->canvas, width, height);
	gnome_canvas_set_scroll_region (GNOME_CANVAS (preview->canvas),
					0.0, 0.0, width, height);

	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (preview->canvas), 1.0);
	group = gnome_canvas_root (GNOME_CANVAS (preview->canvas));

	/* draw shadow */
	style = gtk_widget_get_style (GTK_WIDGET(preview));
	shadow_color = gl_color_from_gdk_color (&style->bg[GTK_STATE_ACTIVE]);
	preview->shadow_item =
		gnome_canvas_item_new (group,
				       gnome_canvas_rect_get_type (),
				       "x1", (gdouble)SHADOW_X_OFFSET,
				       "y1", (gdouble)SHADOW_Y_OFFSET,
				       "x2", (gdouble)(SHADOW_X_OFFSET + width),
				       "y2", (gdouble)(SHADOW_Y_OFFSET + height),
				       "fill_color_rgba", shadow_color,
				       NULL);

	/* draw an initial paper outline */
	preview->paper_item =
		gnome_canvas_item_new (group,
				       gnome_canvas_rect_get_type (),
				       "x1", 0.0,
				       "y1", 0.0,
				       "x2", (gdouble)width,
				       "y2", (gdouble)height,
				       "width_pixels", 1,
				       "outline_color", "black",
				       "fill_color", "white",
				       NULL);

	/* create empty list of label canvas items */
	preview->label_items = NULL;
	preview->labels_per_sheet = 0;

	/* Event handler */
	g_signal_connect (G_OBJECT (preview->canvas), "event",
			  G_CALLBACK (canvas_event_cb), preview);


	/* Style changed handler */
	g_signal_connect (G_OBJECT (preview), "style_set",
			  G_CALLBACK (style_set_cb), NULL);

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

/****************************************************************************/
/* Set label for mini-preview to determine geometry.                        */
/****************************************************************************/
void gl_wdgt_mini_preview_set_label (glWdgtMiniPreview *preview,
				     gchar             *name)
{
	glTemplate *template;
	gchar      *page_size;
	gdouble     canvas_scale;
	gdouble     w, h;
	gdouble     shadow_x, shadow_y;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	/* Fetch template */
	template = gl_template_from_name (name);

	gl_debug (DEBUG_MINI_PREVIEW, "page_size = %s, page_width = %g, page_height = %g",
		  template->page_size, template->page_width, template->page_height);

	/* get paper size and set scale */
	w = preview->width - 4 - 2*SHADOW_X_OFFSET;
	h = preview->height - 4 - 2*SHADOW_Y_OFFSET;
	if ( (w/template->page_width) > (h/template->page_height) ) {
		canvas_scale = h / template->page_height;
	} else {
		canvas_scale = w / template->page_width;
	}
	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (preview->canvas),
					  canvas_scale);
#if 0
	gnome_canvas_set_scroll_region (GNOME_CANVAS (preview->canvas),
					0.0, 0.0,
					template->page_width, template->page_height);
#else
	gnome_canvas_set_scroll_region (GNOME_CANVAS (preview->canvas),
					0.0, 0.0,
					preview->width/canvas_scale,
					preview->height/canvas_scale);
#endif

	/* update shadow */
	shadow_x = SHADOW_X_OFFSET/canvas_scale;
	shadow_y = SHADOW_Y_OFFSET/canvas_scale;
	gnome_canvas_item_set (preview->shadow_item,
			       "x1", shadow_x,
			       "y1", shadow_y,
			       "x2", shadow_x + template->page_width,
			       "y2", shadow_y + template->page_height,
			       NULL);

	/* update paper outline */
	gnome_canvas_item_set (preview->paper_item,
			       "x2", template->page_width,
			       "y2", template->page_height,
			       NULL);

	/* update label items */
	mini_outline_list_free (&preview->label_items);
	preview->label_items =
		mini_outline_list_new (GNOME_CANVAS(preview->canvas),
				       template);

	gl_template_free( &template );
	
	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Draw label outlines and return canvas item list.               */
/*--------------------------------------------------------------------------*/
static GList *
mini_outline_list_new (GnomeCanvas *canvas,
		       glTemplate  *template)
{
	GnomeCanvasGroup      *group = NULL;
	GnomeCanvasItem       *item = NULL;
	GList                 *list = NULL;
	gint                   i, n_labels;
	glTemplateOrigin      *origins;
	gdouble                x1, y1, x2, y2, w, h;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	group = gnome_canvas_root (canvas);

	/* draw mini label outlines */
	n_labels = gl_template_get_n_labels (template);
	origins  = gl_template_get_origins (template);
	gl_template_get_label_size (template, &w, &h);
	for ( i=0; i < n_labels; i++ ) {

		x1 = origins[i].x;
		y1 = origins[i].y;
		x2 = x1 + w;
		y2 = y1 + h;

		switch (template->label.style) {
		case GL_TEMPLATE_STYLE_RECT:
			item = gnome_canvas_item_new (group,
						      gnome_canvas_rect_get_type(),
						      "x1", x1,
						      "y1", y1,
						      "x2", x2,
						      "y2", y2,
						      "width_pixels", 1,
						      "outline_color", "black",
						      "fill_color", "white",
						      NULL);
			break;
		case GL_TEMPLATE_STYLE_ROUND:
			item = gnome_canvas_item_new (group,
						      gnome_canvas_ellipse_get_type(),
						      "x1", x1,
						      "y1", y1,
						      "x2", x2,
						      "y2", y2,
						      "width_pixels", 1,
						      "outline_color", "black",
						      "fill_color", "white",
						      NULL);
			break;
		case GL_TEMPLATE_STYLE_CD:
			if ( w == h ) {
				item = gnome_canvas_item_new (group,
							      gnome_canvas_ellipse_get_type(),
							      "x1", x1,
							      "y1", y1,
							      "x2", x2,
							      "y2", y2,
							      "width_pixels", 1,
							      "outline_color", "black",
							      "fill_color", "white",
							      NULL);
			} else {
				item = cdbc_item (group, x1, y1, template);
			}
			break;
		default:
			g_warning ("Unknown label style");
			return list;
			break;
		}
		g_object_set_data (G_OBJECT (item), "i",
				   GINT_TO_POINTER (i+1));
		
		list = g_list_append (list, item);
	}

	gl_debug (DEBUG_MINI_PREVIEW, "END");
	return list;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Draw label outlines and return canvas item list.               */
/*--------------------------------------------------------------------------*/
static void
mini_outline_list_free (GList ** list)
{
	GnomeCanvasItem *item;
	GList *p;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	if ( *list != NULL ) {

		for (p = *list; p != NULL; p = p->next) {
			item = GNOME_CANVAS_ITEM (p->data);
			gtk_object_destroy (GTK_OBJECT (item));
		}

		g_list_free (*list);
		*list = NULL;

	}

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Canvas event handler, select first and last items.             */
/*--------------------------------------------------------------------------*/
static gint
canvas_event_cb (GnomeCanvas * canvas,
		 GdkEvent * event,
		 gpointer data)
{
	glWdgtMiniPreview *preview = GL_WDGT_MINI_PREVIEW (data);
	GnomeCanvasItem *item;
	static gboolean dragging = FALSE;
	static gint prev_i = 0, first, last;
	gint i;
	gdouble x, y;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		gnome_canvas_window_to_world (canvas,
					      event->button.x, event->button.y,
					      &x, &y);
		switch (event->button.button) {
		case 1:
			/* Get item at cursor and make sure
			   it's a label object ("i" is valid) */
			item = gnome_canvas_get_item_at (GNOME_CANVAS (canvas),
							 x, y);
			if (item == NULL)
				break;
			i = GPOINTER_TO_INT (g_object_get_data
					     (G_OBJECT (item), "i"));
			if (i == 0)
				break;
			/* Go into dragging mode while remains pressed. */
			dragging = TRUE;
			gnome_canvas_item_grab (canvas->root,
						GDK_POINTER_MOTION_MASK |
						GDK_BUTTON_RELEASE_MASK |
						GDK_BUTTON_PRESS_MASK,
						NULL, event->button.time);
			g_signal_emit (G_OBJECT(preview),
				       wdgt_mini_preview_signals[CLICKED],
				       0, i);
			first = i;
			last = i;
			g_signal_emit (G_OBJECT(preview),
				       wdgt_mini_preview_signals[PRESSED],
				       0, first, last);
			prev_i = i;
			break;

		default:
			break;
		}
		break;

	case GDK_BUTTON_RELEASE:
		gnome_canvas_window_to_world (canvas,
					      event->button.x, event->button.y,
					      &x, &y);
		switch (event->button.button) {
		case 1:
			/* Exit dragging mode */
			dragging = FALSE;
			gnome_canvas_item_ungrab (canvas->root, event->button.time);
			break;

		default:
			break;
		}
		break;

	case GDK_MOTION_NOTIFY:
		gnome_canvas_window_to_world (canvas,
					      event->motion.x, event->motion.y,
					      &x, &y);
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			/* Get item at cursor and
			   make sure it's a label object ("i" is valid) */
			item = gnome_canvas_get_item_at (GNOME_CANVAS (canvas),
							 x, y);
			if (item == NULL)
				break;
			i = GPOINTER_TO_INT (g_object_get_data
					     (G_OBJECT (item), "i"));
			if (i == 0)
				break;
			if (prev_i != i) {
				/* Entered into a new item */
				last = i;
				g_signal_emit (G_OBJECT(preview),
					       wdgt_mini_preview_signals[PRESSED],
					       0,
					       MIN (first, last),
					       MAX (first, last));
				prev_i = i;
			}
		}
		break;

	default:
		break;
	}

	gl_debug (DEBUG_MINI_PREVIEW, "END");

	return FALSE;
}

/****************************************************************************/
/* Highlight given label outlines.                                          */
/****************************************************************************/
void
gl_wdgt_mini_preview_highlight_range (glWdgtMiniPreview *preview,
				      gint               first_label,
				      gint               last_label)
{
	GtkStyle         *style;
	guint             select_color;
	GnomeCanvasItem  *item = NULL;
	GList            *p = NULL;
	gint              i;

	gl_debug (DEBUG_MINI_PREVIEW, "START");

	style = gtk_widget_get_style (GTK_WIDGET(preview));
	select_color = gl_color_from_gdk_color (&style->base[GTK_STATE_SELECTED]);
	gl_debug (DEBUG_MINI_PREVIEW, "select color = 0x%08x", select_color);

	for (p = preview->label_items, i = 1; p != NULL; i++, p = p->next) {

		item = GNOME_CANVAS_ITEM (p->data);

		if ((i >= first_label) && (i <= last_label)) {
			gnome_canvas_item_set (item,
					       "fill_color_rgba", select_color,
					       NULL);
		} else {
			gnome_canvas_item_set (item,
					       "fill_color", "white", NULL);
		}

	}

	preview->highlight_first = first_label;
	preview->highlight_last =  last_label;

	gl_debug (DEBUG_MINI_PREVIEW, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Refresh colors, if style changed.                              */
/*--------------------------------------------------------------------------*/
static void
style_set_cb (GtkWidget        *widget,
	      GtkStyle         *previous_style,
	      gpointer          data)
{
	glWdgtMiniPreview *preview = GL_WDGT_MINI_PREVIEW (widget);
	GtkStyle         *style;
	guint             shadow_color;

	style = gtk_widget_get_style (GTK_WIDGET(preview));

	shadow_color = gl_color_from_gdk_color (&style->bg[GTK_STATE_ACTIVE]);
	gnome_canvas_item_set (preview->shadow_item,
			       "fill_color_rgba", shadow_color,
			       NULL);

	gl_wdgt_mini_preview_highlight_range (preview,
					      preview->highlight_first,
					      preview->highlight_last);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Draw CD business card item (cut-off in w and/or h).            */
/*--------------------------------------------------------------------------*/
static GnomeCanvasItem *
cdbc_item (GnomeCanvasGroup *group,
	   gdouble           x1,
	   gdouble           y1,
	   glTemplate       *template)
{
	GnomeCanvasPoints *points;
	gint               i_coords, i_theta;
	gdouble            theta1, theta2;
	gdouble            x0, y0, w, h, r;
	GnomeCanvasItem   *item;

	gl_template_get_label_size (template, &w, &h);
	r = template->label.cd.r1;
	x0 = x1 + (w/2.0);
	y0 = y1 + (h/2.0);

	theta1 = (180.0/G_PI) * acos (w / (2.0*r));
	theta2 = (180.0/G_PI) * asin (h / (2.0*r));

	points = gnome_canvas_points_new (360/RES + 1);
	i_coords = 0;

	points->coords[i_coords++] = x0 + r * cos (theta1 * G_PI / 180.0);
	points->coords[i_coords++] = y0 + r * sin (theta1 * G_PI / 180.0);

	for ( i_theta = theta1 + RES; i_theta < theta2; i_theta +=RES ) {
		points->coords[i_coords++] = x0 + r * cos (i_theta * G_PI / 180.0);
		points->coords[i_coords++] = y0 + r * sin (i_theta * G_PI / 180.0);
	}

	points->coords[i_coords++] = x0 + r * cos (theta2 * G_PI / 180.0);
	points->coords[i_coords++] = y0 + r * sin (theta2 * G_PI / 180.0);


	if ( fabs (theta2 - 90.0) > GNOME_CANVAS_EPSILON ) {
		points->coords[i_coords++] = x0 + r * cos ((180-theta2) * G_PI / 180.0);
		points->coords[i_coords++] = y0 + r * sin ((180-theta2) * G_PI / 180.0);
	}

	for ( i_theta = 180-theta2+RES; i_theta < (180-theta1); i_theta +=RES ) {
		points->coords[i_coords++] = x0 + r * cos (i_theta * G_PI / 180.0);
		points->coords[i_coords++] = y0 + r * sin (i_theta * G_PI / 180.0);
	}

	points->coords[i_coords++] = x0 + r * cos ((180-theta1) * G_PI / 180.0);
	points->coords[i_coords++] = y0 + r * sin ((180-theta1) * G_PI / 180.0);

	if ( fabs (theta1) > GNOME_CANVAS_EPSILON ) {
		points->coords[i_coords++] = x0 + r * cos ((180+theta1) * G_PI / 180.0);
		points->coords[i_coords++] = y0 + r * sin ((180+theta1) * G_PI / 180.0);
	}

	for ( i_theta = 180+theta1+RES; i_theta < (180+theta2); i_theta +=RES ) {
		points->coords[i_coords++] = x0 + r * cos (i_theta * G_PI / 180.0);
		points->coords[i_coords++] = y0 + r * sin (i_theta * G_PI / 180.0);
	}

	points->coords[i_coords++] = x0 + r * cos ((180+theta2) * G_PI / 180.0);
	points->coords[i_coords++] = y0 + r * sin ((180+theta2) * G_PI / 180.0);

	if ( fabs (theta2 - 90.0) > GNOME_CANVAS_EPSILON ) {
		points->coords[i_coords++] = x0 + r * cos ((360-theta2) * G_PI / 180.0);
		points->coords[i_coords++] = y0 + r * sin ((360-theta2) * G_PI / 180.0);
	}

	for ( i_theta = 360-theta2+RES; i_theta < (360-theta1); i_theta +=RES ) {
		points->coords[i_coords++] = x0 + r * cos (i_theta * G_PI / 180.0);
		points->coords[i_coords++] = y0 + r * sin (i_theta * G_PI / 180.0);
	}

	if ( fabs (theta1) > GNOME_CANVAS_EPSILON ) {
		points->coords[i_coords++] = x0 + r * cos ((360-theta1) * G_PI / 180.0);
		points->coords[i_coords++] = y0 + r * sin ((360-theta1) * G_PI / 180.0);
	}

	points->num_points = i_coords / 2;


	item = gnome_canvas_item_new (group,
				      gnome_canvas_polygon_get_type (),
				      "points", points,
				      "width_pixels", 1,
				      "outline_color", "black",
				      "fill_color", "white",
				      NULL);

	gnome_canvas_points_free (points);

	return item;
}

