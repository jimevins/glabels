/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_rotate_label.c:  label rotate selection widget module
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

#include <math.h>

#include "wdgt-rotate-label.h"
#include "hig.h"
#include "marshal.h"
#include "color.h"
#include <libglabels/template.h>

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/
#define MINI_PREVIEW_MAX_PIXELS 48
#define MINI_PREVIEW_CANVAS_PIXELS (MINI_PREVIEW_MAX_PIXELS + 8)

#define LINE_COLOR             GL_COLOR(0,0,0)
#define FILL_COLOR             GL_COLOR(255,255,255)
#define UNSENSITIVE_LINE_COLOR GL_COLOR(0x66,0x66,0x66)
#define UNSENSITIVE_FILL_COLOR GL_COLOR(0xCC,0xCC,0xCC)

#define RES 5 /* Resolution in degrees for Business Card CD outlines */

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};

typedef void (*glWdgtRotateLabelSignal) (GObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GObjectClass *parent_class;

static gint wdgt_rotate_label_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_wdgt_rotate_label_class_init    (glWdgtRotateLabelClass *class);
static void gl_wdgt_rotate_label_instance_init (glWdgtRotateLabel      *rotate_select);
static void gl_wdgt_rotate_label_finalize      (GObject                *object);

static void gl_wdgt_rotate_label_construct     (glWdgtRotateLabel      *rotate_select);

static void entry_changed_cb                   (GtkToggleButton *toggle,
						gpointer         user_data);

static GtkWidget *mini_preview_canvas_new      (void);

static void mini_preview_canvas_update         (GnomeCanvas      *canvas,
						glTemplate       *template,
						gboolean          rotate_flag);

static GnomeCanvasItem *cdbc_item              (GnomeCanvasGroup *group,
						gdouble           w,
						gdouble           h,
						gdouble           r,
						guint             line_width,
						guint             line_color,
						guint             fill_color);

/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
GType
gl_wdgt_rotate_label_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (glWdgtRotateLabelClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_wdgt_rotate_label_class_init,
			NULL,
			NULL,
			sizeof (glWdgtRotateLabel),
			0,
			(GInstanceInitFunc) gl_wdgt_rotate_label_instance_init,
			NULL
		};

		type = g_type_register_static (GL_TYPE_HIG_HBOX,
					       "glWdgtRotateLabel", &info, 0);
	}

	return type;
}

static void
gl_wdgt_rotate_label_class_init (glWdgtRotateLabelClass *class)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) class;

	parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_wdgt_rotate_label_finalize;

	wdgt_rotate_label_signals[CHANGED] =
	    g_signal_new ("changed",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glWdgtRotateLabelClass, changed),
			  NULL, NULL,
			  gl_marshal_VOID__VOID,
			  G_TYPE_NONE, 0);

}

static void
gl_wdgt_rotate_label_instance_init (glWdgtRotateLabel *rotate_select)
{
	rotate_select->rotate_check = NULL;

	rotate_select->canvas = NULL;

	rotate_select->template = NULL;
}

static void
gl_wdgt_rotate_label_finalize (GObject *object)
{
	glWdgtRotateLabel      *rotate_select;
	glWdgtRotateLabelClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WDGT_ROTATE_LABEL (object));

	rotate_select = GL_WDGT_ROTATE_LABEL (object);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

GtkWidget *
gl_wdgt_rotate_label_new (void)
{
	glWdgtRotateLabel *rotate_select;

	rotate_select = g_object_new (gl_wdgt_rotate_label_get_type (), NULL);

	gl_wdgt_rotate_label_construct (rotate_select);

	return GTK_WIDGET (rotate_select);
}

/*--------------------------------------------------------------------------*/
/* Construct composite widget.                                              */
/*--------------------------------------------------------------------------*/
static void
gl_wdgt_rotate_label_construct (glWdgtRotateLabel *rotate_select)
{
	GtkWidget *whbox;

	whbox = GTK_WIDGET (rotate_select);

	/* Actual selection control */
	rotate_select->rotate_check =
	    gtk_check_button_new_with_label (_("Rotate"));
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox),
				rotate_select->rotate_check);

	/* mini_preview canvas */
	rotate_select->canvas = mini_preview_canvas_new ();
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), rotate_select->canvas);

	/* Connect signals to controls */
	g_signal_connect (G_OBJECT (rotate_select->rotate_check), "toggled",
			  G_CALLBACK (entry_changed_cb), rotate_select);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change of check button                    */
/*--------------------------------------------------------------------------*/
static void
entry_changed_cb (GtkToggleButton *toggle,
		  gpointer         user_data)
{
	glWdgtRotateLabel *rotate_select = GL_WDGT_ROTATE_LABEL (user_data);

	if (rotate_select->template != NULL) {
		/* Update mini_preview canvas & details with template */
		mini_preview_canvas_update (GNOME_CANVAS
					    (rotate_select->canvas),
					    rotate_select->template,
					    gtk_toggle_button_get_active
					    (toggle));
	}

	/* Emit our "changed" signal */
	g_signal_emit (G_OBJECT (user_data),
		       wdgt_rotate_label_signals[CHANGED], 0);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Draw a mini-preview canvas.                                    */
/*--------------------------------------------------------------------------*/
static GtkWidget *
mini_preview_canvas_new (void)
{
	GtkWidget *wcanvas = NULL;

	/* Create a canvas */
	gtk_widget_push_colormap (gdk_rgb_get_colormap ());
	wcanvas = gnome_canvas_new_aa ();
	gtk_widget_pop_colormap ();

	gtk_widget_set_size_request (GTK_WIDGET (wcanvas),
				     MINI_PREVIEW_CANVAS_PIXELS,
				     MINI_PREVIEW_CANVAS_PIXELS);

	gtk_object_set_data (GTK_OBJECT (wcanvas), "label_item", NULL);

	return wcanvas;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Update mini-preview canvas from new template.                  */
/*--------------------------------------------------------------------------*/
static void
mini_preview_canvas_update (GnomeCanvas *canvas,
			    glTemplate  *template,
			    gboolean     rotate_flag)
{
	const glTemplateLabelType *label_type;
	gdouble                    canvas_scale;
	GnomeCanvasGroup          *group = NULL;
	GnomeCanvasItem           *label_item = NULL;
	gdouble                    m, m_canvas, w, h;
	guint                      line_color, fill_color;

	/* Fetch our data from canvas */
	label_item = g_object_get_data (G_OBJECT (canvas), "label_item");

	label_type = gl_template_get_first_label_type (template);

	gl_template_get_label_size (label_type, &w, &h);
	m = MAX (w, h);
	canvas_scale = MINI_PREVIEW_MAX_PIXELS / m;
	m_canvas = MINI_PREVIEW_CANVAS_PIXELS / canvas_scale;

	/* scale and size canvas */
	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (canvas), canvas_scale);
	group = gnome_canvas_root (GNOME_CANVAS (canvas));
	gnome_canvas_set_scroll_region (GNOME_CANVAS (canvas),
					-m_canvas / 2.0, -m_canvas / 2.0,
					+m_canvas / 2.0, +m_canvas / 2.0);

	/* remove old label outline */
	if (label_item != NULL) {
		gtk_object_destroy (GTK_OBJECT (label_item));
	}

	/* Adjust sensitivity (should the canvas be grayed?) */
	if (w != h) {
		line_color = LINE_COLOR;
		fill_color = FILL_COLOR;
	} else {
		line_color = UNSENSITIVE_LINE_COLOR;
		fill_color = UNSENSITIVE_FILL_COLOR;
	}

	/* draw mini label outline */
	switch (label_type->shape) {
	case GL_TEMPLATE_SHAPE_RECT:
		label_item = gnome_canvas_item_new (group,
						    gnome_canvas_rect_get_type(),
						    "x1", -w / 2.0,
						    "y1", -h / 2.0,
						    "x2", +w / 2.0,
						    "y2", +h / 2.0,
						    "width_pixels", 1,
						    "outline_color_rgba", line_color,
						    "fill_color_rgba", fill_color,
						    NULL);
		break;
	case GL_TEMPLATE_SHAPE_ROUND:
		label_item = gnome_canvas_item_new (group,
						    gnome_canvas_ellipse_get_type(),
						    "x1", -w / 2.0,
						    "y1", -h / 2.0,
						    "x2", +w / 2.0,
						    "y2", +h / 2.0,
						    "width_pixels", 1,
						    "outline_color_rgba", line_color,
						    "fill_color_rgba", fill_color,
						    NULL);
		break;
	case GL_TEMPLATE_SHAPE_CD:
		if ( w == h ) {
			label_item = gnome_canvas_item_new (group,
							    gnome_canvas_ellipse_get_type(),
							    "x1", -w / 2.0,
							    "y1", -h / 2.0,
							    "x2", +w / 2.0,
							    "y2", +h / 2.0,
							    "width_pixels", 1,
							    "outline_color_rgba", line_color,
							    "fill_color_rgba", fill_color,
							    NULL);
		} else {
			label_item = cdbc_item (group,
						w, h, label_type->size.cd.r1,
						1, line_color, fill_color);
		}
		break;
	default:
		g_warning ("Unknown label style");
		break;
	}

	if (rotate_flag) {
		gdouble affine[6];

		art_affine_rotate (affine, 90);
		gnome_canvas_item_affine_absolute (label_item, affine);
	}

	gtk_object_set_data (GTK_OBJECT (canvas), "label_item", label_item);

}

/****************************************************************************/
/* query state of widget.                                                   */
/****************************************************************************/
gboolean
gl_wdgt_rotate_label_get_state (glWdgtRotateLabel *rotate_select)
{
	return
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
					  (rotate_select->rotate_check));
}

/****************************************************************************/
/* set state of widget.                                                     */
/****************************************************************************/
void
gl_wdgt_rotate_label_set_state (glWdgtRotateLabel *rotate_select,
				gboolean state)
{
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				      (rotate_select->rotate_check), state);
}

/****************************************************************************/
/* set template for widget.                                                 */
/****************************************************************************/
void
gl_wdgt_rotate_label_set_template_name (glWdgtRotateLabel *rotate_select,
					gchar             *name)
{
	glTemplate                *template;
	const glTemplateLabelType *label_type;
	gdouble                    raw_w, raw_h;

	template   = gl_template_from_name (name);
	label_type = gl_template_get_first_label_type (template);

	rotate_select->template = template;
	gl_template_get_label_size (label_type, &raw_w, &raw_h);

	gtk_widget_set_sensitive (rotate_select->rotate_check,
				  (raw_w != raw_h));

	mini_preview_canvas_update (GNOME_CANVAS (rotate_select->canvas),
				    template, FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				      (rotate_select->rotate_check), FALSE);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Draw CD business card item (cut-off in w and/or h).            */
/*--------------------------------------------------------------------------*/
static GnomeCanvasItem *
cdbc_item (GnomeCanvasGroup *group,
	   gdouble           w,
	   gdouble           h,
	   gdouble           r,
	   guint             line_width,
	   guint             line_color,
	   guint             fill_color)
{
	GnomeCanvasPoints         *points;
	gint                       i_coords, i_theta;
	gdouble                    theta1, theta2;
	GnomeCanvasItem           *item;

	theta1 = (180.0/G_PI) * acos (w / (2.0*r));
	theta2 = (180.0/G_PI) * asin (h / (2.0*r));

	points = gnome_canvas_points_new (360/RES + 1);
	i_coords = 0;

	points->coords[i_coords++] = r * cos (theta1 * G_PI / 180.0);
	points->coords[i_coords++] = r * sin (theta1 * G_PI / 180.0);

	for ( i_theta = theta1 + RES; i_theta < theta2; i_theta +=RES ) {
		points->coords[i_coords++] = r * cos (i_theta * G_PI / 180.0);
		points->coords[i_coords++] = r * sin (i_theta * G_PI / 180.0);
	}

	points->coords[i_coords++] = r * cos (theta2 * G_PI / 180.0);
	points->coords[i_coords++] = r * sin (theta2 * G_PI / 180.0);


	if ( fabs (theta2 - 90.0) > GNOME_CANVAS_EPSILON ) {
		points->coords[i_coords++] = r * cos ((180-theta2) * G_PI / 180.0);
		points->coords[i_coords++] = r * sin ((180-theta2) * G_PI / 180.0);
	}

	for ( i_theta = 180-theta2+RES; i_theta < (180-theta1); i_theta +=RES ) {
		points->coords[i_coords++] = r * cos (i_theta * G_PI / 180.0);
		points->coords[i_coords++] = r * sin (i_theta * G_PI / 180.0);
	}

	points->coords[i_coords++] = r * cos ((180-theta1) * G_PI / 180.0);
	points->coords[i_coords++] = r * sin ((180-theta1) * G_PI / 180.0);

	if ( fabs (theta1) > GNOME_CANVAS_EPSILON ) {
		points->coords[i_coords++] = r * cos ((180+theta1) * G_PI / 180.0);
		points->coords[i_coords++] = r * sin ((180+theta1) * G_PI / 180.0);
	}

	for ( i_theta = 180+theta1+RES; i_theta < (180+theta2); i_theta +=RES ) {
		points->coords[i_coords++] = r * cos (i_theta * G_PI / 180.0);
		points->coords[i_coords++] = r * sin (i_theta * G_PI / 180.0);
	}

	points->coords[i_coords++] = r * cos ((180+theta2) * G_PI / 180.0);
	points->coords[i_coords++] = r * sin ((180+theta2) * G_PI / 180.0);

	if ( fabs (theta2 - 90.0) > GNOME_CANVAS_EPSILON ) {
		points->coords[i_coords++] = r * cos ((360-theta2) * G_PI / 180.0);
		points->coords[i_coords++] = r * sin ((360-theta2) * G_PI / 180.0);
	}

	for ( i_theta = 360-theta2+RES; i_theta < (360-theta1); i_theta +=RES ) {
		points->coords[i_coords++] = r * cos (i_theta * G_PI / 180.0);
		points->coords[i_coords++] = r * sin (i_theta * G_PI / 180.0);
	}

	if ( fabs (theta1) > GNOME_CANVAS_EPSILON ) {
		points->coords[i_coords++] = r * cos ((360-theta1) * G_PI / 180.0);
		points->coords[i_coords++] = r * sin ((360-theta1) * G_PI / 180.0);
	}

	points->num_points = i_coords / 2;


	item = gnome_canvas_item_new (group,
				      gnome_canvas_polygon_get_type (),
				      "points", points,
				      "width_pixels", line_width,
				      "outline_color_rgba", line_color,
				      "fill_color_rgba", fill_color,
				      NULL);

	gnome_canvas_points_free (points);

	return item;
}

