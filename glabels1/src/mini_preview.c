/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  mini_preview.c:  mini preview widget module
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

#include "mini_preview.h"

#include "debug.h"

#define MINI_PREVIEW_MAX_PIXELS 175

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

static gint mini_preview_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_mini_preview_class_init (glMiniPreviewClass * class);
static void gl_mini_preview_init (glMiniPreview * preview);
static void gl_mini_preview_destroy (GtkObject * object);

static void gl_mini_preview_construct (glMiniPreview * preview,
				       gint height, gint width);

static GList *mini_outline_list_new (GnomeCanvas *canvas,
				     glTemplate *template);
static void mini_outline_list_free (GList ** list);

static gint canvas_event_cb (GnomeCanvas * canvas, GdkEvent * event,
			     gpointer data);

/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
guint
gl_mini_preview_get_type (void)
{
	static guint mini_preview_type = 0;

	if (!mini_preview_type) {
		GtkTypeInfo mini_preview_info = {
			"glMiniPreview",
			sizeof (glMiniPreview),
			sizeof (glMiniPreviewClass),
			(GtkClassInitFunc) gl_mini_preview_class_init,
			(GtkObjectInitFunc) gl_mini_preview_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
		};

		mini_preview_type = gtk_type_unique (gtk_hbox_get_type (),
						     &mini_preview_info);
	}

	return mini_preview_type;
}

static void
gl_mini_preview_class_init (glMiniPreviewClass * class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	parent_class = gtk_type_class (gtk_hbox_get_type ());

	object_class->destroy = gl_mini_preview_destroy;

	mini_preview_signals[CLICKED] =
	    gtk_signal_new ("clicked", GTK_RUN_LAST, object_class->type,
			    GTK_SIGNAL_OFFSET (glMiniPreviewClass, clicked),
			    gtk_marshal_NONE__INT,
			    GTK_TYPE_NONE, 1, GTK_TYPE_INT);

	mini_preview_signals[PRESSED] =
	    gtk_signal_new ("pressed", GTK_RUN_LAST, object_class->type,
			    GTK_SIGNAL_OFFSET (glMiniPreviewClass, pressed),
			    gtk_marshal_NONE__INT_INT,
			    GTK_TYPE_NONE, 2, GTK_TYPE_INT, GTK_TYPE_INT);

	gtk_object_class_add_signals (object_class, mini_preview_signals,
				      LAST_SIGNAL);

	class->clicked = NULL;
	class->pressed = NULL;
}

static void
gl_mini_preview_init (glMiniPreview * preview)
{
	preview->canvas = NULL;
	preview->label_items = NULL;
}

static void
gl_mini_preview_destroy (GtkObject * object)
{
	glMiniPreview *preview;
	glMiniPreviewClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_MINI_PREVIEW (object));

	preview = GL_MINI_PREVIEW (object);
	class = GL_MINI_PREVIEW_CLASS (GTK_OBJECT (preview)->klass);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

GtkWidget *
gl_mini_preview_new (gint height,
		     gint width)
{
	glMiniPreview *preview;

	preview = gtk_type_new (gl_mini_preview_get_type ());

	gl_mini_preview_construct (preview, height, width);

	return GTK_WIDGET (preview);
}

/*--------------------------------------------------------------------------*/
/* Construct composite widget.                                              */
/*--------------------------------------------------------------------------*/
static void
gl_mini_preview_construct (glMiniPreview * preview,
			   gint height,
			   gint width)
{
	GtkWidget *whbox;
	GnomeCanvasGroup *group;

	whbox = GTK_WIDGET (preview);

	preview->height = height;
	preview->width  = width;

	/* create canvas */
	gtk_widget_push_visual (gdk_rgb_get_visual ());
	gtk_widget_push_colormap (gdk_rgb_get_cmap ());
	preview->canvas = gnome_canvas_new_aa ();
	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();
	gtk_box_pack_start (GTK_BOX (whbox), preview->canvas, TRUE, TRUE, 0);
	gtk_widget_set_usize (preview->canvas, width, height);
	gnome_canvas_set_scroll_region (GNOME_CANVAS (preview->canvas),
					0.0, 0.0, width, height);

	/* draw an initial paper outline */
	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (preview->canvas), 1.0);
	group = gnome_canvas_root (GNOME_CANVAS (preview->canvas));
	preview->paper_item =
		gnome_canvas_item_new (group,
				       gnome_canvas_rect_get_type (),
				       "x1", 0.0,
				       "y1", 0.0,
				       "x2", (gdouble)width,
				       "y2", (gdouble)height,
				       "width_pixels", 1,
				       "outline_color", "black",
				       "fill_color", "white", NULL);

	/* create empty list of label canvas items */
	preview->label_items = NULL;
	preview->labels_per_sheet = 0;

	/* Event handler */
	gtk_signal_connect (GTK_OBJECT (preview->canvas), "event",
			    GTK_SIGNAL_FUNC (canvas_event_cb), preview);

}

/****************************************************************************/
/* Set label for mini-preview to determine geometry.                        */
/****************************************************************************/
void gl_mini_preview_set_label (glMiniPreview * preview,
				gchar *name)
{
	glTemplate *template;
	const GnomePaper *paper = NULL;
	gdouble paper_width, paper_height;
	gdouble canvas_scale;
	gdouble w, h;

	/* Fetch template */
	template = gl_template_from_name (name);

	/* get paper size and set scale */
	paper = gnome_paper_with_name (template->page_size);
	paper_width = gnome_paper_pswidth (paper);
	paper_height = gnome_paper_psheight (paper);
	w = preview->width - 4;
	h = preview->height - 4;
	if ( (w/paper_width) > (h/paper_height) ) {
		canvas_scale = h / paper_height;
	} else {
		canvas_scale = w / paper_width;
	}
	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (preview->canvas),
					  canvas_scale);
	gnome_canvas_set_scroll_region (GNOME_CANVAS (preview->canvas),
					0.0, 0.0, paper_width, paper_height);

	/* update paper outline */
	gnome_canvas_item_set (preview->paper_item,
			       "x1", 0.0,
			       "y1", 0.0,
			       "x2", paper_width,
			       "y2", paper_height,
			       NULL);

	/* update label items */
	mini_outline_list_free (&preview->label_items);
	preview->label_items =
		mini_outline_list_new (GNOME_CANVAS(preview->canvas),
				       template);

	gl_template_free( &template );
	
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Draw label outlines and return canvas item list.               */
/*--------------------------------------------------------------------------*/
static GList *
mini_outline_list_new (GnomeCanvas * canvas,
		       glTemplate * template)
{
	GnomeCanvasGroup *group = NULL;
	GnomeCanvasItem *item = NULL;
	GList *list = NULL;
	gint i, ix, iy;
	gdouble x1, y1, x2, y2;

	group = gnome_canvas_root (canvas);

	/* draw mini label outlines */
	i = 1;
	for (iy = 0; iy < template->ny; iy++) {
		for (ix = 0; ix < template->nx; ix++, i++) {

			x1 = ix * (template->dx) + template->x0;
			y1 = iy * (template->dy) + template->y0;
			x2 = x1 + template->label_width;
			y2 = y1 + template->label_height;

			switch (template->style) {
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
			case GL_TEMPLATE_STYLE_CD:
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
			default:
				WARN ("Unknown label style");
				return list;
				break;
			}
			gtk_object_set_data (GTK_OBJECT (item), "i",
					     GINT_TO_POINTER (i));

			list = g_list_append (list, item);
		}
	}

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

	if ( *list != NULL ) {

		for (p = *list; p != NULL; p = p->next) {
			item = GNOME_CANVAS_ITEM (p->data);
			gtk_object_destroy (GTK_OBJECT (item));
		}

		g_list_free (*list);
		*list = NULL;

	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Canvas event handler, select first and last items.             */
/*--------------------------------------------------------------------------*/
static gint
canvas_event_cb (GnomeCanvas * canvas,
		 GdkEvent * event,
		 gpointer data)
{
	glMiniPreview *preview = GL_MINI_PREVIEW (data);
	GnomeCanvasItem *item;
	static gboolean dragging = FALSE;
	static gint prev_i = 0, first, last;
	gint i;
	gdouble x, y;

	gnome_canvas_window_to_world (canvas,
				      event->button.x, event->button.y,
				      &x, &y);

	switch (event->type) {

	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
		case 1:
			/* Get item at cursor and make sure
			   it's a label object ("i" is valid) */
			item = gnome_canvas_get_item_at (GNOME_CANVAS (canvas),
							 x, y);
			if (item == NULL)
				break;
			i = GPOINTER_TO_INT (gtk_object_get_data
					     (GTK_OBJECT (item), "i"));
			if (i == 0)
				break;
			/* Go into dragging mode while remains pressed. */
			dragging = TRUE;
			gdk_pointer_grab (GTK_WIDGET (canvas)->window,
					  FALSE,
					  GDK_POINTER_MOTION_MASK |
					  GDK_BUTTON_RELEASE_MASK |
					  GDK_BUTTON_PRESS_MASK, NULL, NULL,
					  event->button.time);
			gtk_signal_emit (GTK_OBJECT(preview),
					 mini_preview_signals[CLICKED],
					 i);
			first = i;
			last = i;
			gtk_signal_emit (GTK_OBJECT(preview),
					 mini_preview_signals[PRESSED],
					 first, last);
			prev_i = i;
			break;

		default:
			break;
		}
		break;

	case GDK_BUTTON_RELEASE:
		switch (event->button.button) {
		case 1:
			/* Exit dragging mode */
			dragging = FALSE;
			gdk_pointer_ungrab (event->button.time);
			break;

		default:
			break;
		}
		break;

	case GDK_MOTION_NOTIFY:
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			/* Get item at cursor and
			   make sure it's a label object ("i" is valid) */
			item = gnome_canvas_get_item_at (GNOME_CANVAS (canvas),
							 x, y);
			if (item == NULL)
				break;
			i = GPOINTER_TO_INT (gtk_object_get_data
					     (GTK_OBJECT (item), "i"));
			if (i == 0)
				break;
			if (prev_i != i) {
				/* Entered into a new item */
				last = i;
				gtk_signal_emit (GTK_OBJECT(preview),
						 mini_preview_signals[PRESSED],
						 MIN (first, last),
						 MAX (first, last));
				prev_i = i;
			}
		}
		break;

	default:
		break;
	}

	return FALSE;

}

/****************************************************************************/
/* Highlight given label outlines.                                          */
/****************************************************************************/
void
gl_mini_preview_highlight_range (glMiniPreview * preview,
				 gint first_label,
				 gint last_label)
{
	GnomeCanvasItem *item = NULL;
	GList *p = NULL;
	gint i;

	for (p = preview->label_items, i = 1; p != NULL; i++, p = p->next) {

		item = GNOME_CANVAS_ITEM (p->data);

		if ((i >= first_label) && (i <= last_label)) {
			gnome_canvas_item_set (item,
					       "fill_color", "light blue",
					       NULL);
		} else {
			gnome_canvas_item_set (item,
					       "fill_color", "white", NULL);
		}

	}

}

