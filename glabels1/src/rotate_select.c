/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  rotate_select.c:  label rotate selection widget module
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

#include "rotate_select.h"
#include "template.h"

#include "debug.h"

#define MINI_PREVIEW_MAX_PIXELS 48

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};

typedef void (*glRotateSelectSignal) (GtkObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkContainerClass *parent_class;

static gint rotate_select_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_rotate_select_class_init (glRotateSelectClass * class);
static void gl_rotate_select_init (glRotateSelect * rotate_select);
static void gl_rotate_select_destroy (GtkObject * object);

static void gl_rotate_select_construct (glRotateSelect * rotate_select);

static void entry_changed_cb (GtkToggleButton * toggle, gpointer user_data);

static GtkWidget *mini_preview_canvas_new (void);

static void mini_preview_canvas_update (GnomeCanvas * canvas,
					glTemplate * template,
					gboolean rotate_flag);

/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
guint
gl_rotate_select_get_type (void)
{
	static guint rotate_select_type = 0;

	if (!rotate_select_type) {
		GtkTypeInfo rotate_select_info = {
			"glRotateSelect",
			sizeof (glRotateSelect),
			sizeof (glRotateSelectClass),
			(GtkClassInitFunc) gl_rotate_select_class_init,
			(GtkObjectInitFunc) gl_rotate_select_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
		};

		rotate_select_type = gtk_type_unique (gtk_hbox_get_type (),
						      &rotate_select_info);
	}

	return rotate_select_type;
}

static void
gl_rotate_select_class_init (glRotateSelectClass * class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	parent_class = gtk_type_class (gtk_hbox_get_type ());

	object_class->destroy = gl_rotate_select_destroy;

	rotate_select_signals[CHANGED] =
	    gtk_signal_new ("changed", GTK_RUN_LAST, object_class->type,
			    GTK_SIGNAL_OFFSET (glRotateSelectClass, changed),
			    gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals (object_class, rotate_select_signals,
				      LAST_SIGNAL);

	class->changed = NULL;
}

static void
gl_rotate_select_init (glRotateSelect * rotate_select)
{
	rotate_select->rotate_check = NULL;

	rotate_select->canvas = NULL;

	rotate_select->template = NULL;
}

static void
gl_rotate_select_destroy (GtkObject * object)
{
	glRotateSelect *rotate_select;
	glRotateSelectClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_ROTATE_SELECT (object));

	rotate_select = GL_ROTATE_SELECT (object);
	class = GL_ROTATE_SELECT_CLASS (GTK_OBJECT (rotate_select)->klass);

	gl_template_free (&rotate_select->template);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

GtkWidget *
gl_rotate_select_new (void)
{
	glRotateSelect *rotate_select;

	rotate_select = gtk_type_new (gl_rotate_select_get_type ());

	gl_rotate_select_construct (rotate_select);

	return GTK_WIDGET (rotate_select);
}

/*--------------------------------------------------------------------------*/
/* Construct composite widget.                                              */
/*--------------------------------------------------------------------------*/
static void
gl_rotate_select_construct (glRotateSelect * rotate_select)
{
	GtkWidget *whbox;

	whbox = GTK_WIDGET (rotate_select);

	/* Actual selection control */
	rotate_select->rotate_check =
	    gtk_check_button_new_with_label (_("Rotate"));
	gtk_box_pack_start (GTK_BOX (whbox), rotate_select->rotate_check, TRUE,
			    TRUE, GNOME_PAD);

	/* mini_preview canvas */
	rotate_select->canvas = mini_preview_canvas_new ();
	gtk_box_pack_start (GTK_BOX (whbox), rotate_select->canvas,
			    TRUE, TRUE, GNOME_PAD);

	/* Connect signals to controls */
	gtk_signal_connect (GTK_OBJECT (rotate_select->rotate_check), "toggled",
			    GTK_SIGNAL_FUNC (entry_changed_cb), rotate_select);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change of check button                    */
/*--------------------------------------------------------------------------*/
static void
entry_changed_cb (GtkToggleButton * toggle,
		  gpointer user_data)
{
	glRotateSelect *rotate_select = GL_ROTATE_SELECT (user_data);

	if (rotate_select->template != NULL) {
		/* Update mini_preview canvas & details with template */
		mini_preview_canvas_update (GNOME_CANVAS
					    (rotate_select->canvas),
					    rotate_select->template,
					    gtk_toggle_button_get_active
					    (toggle));
	}

	/* Emit our "changed" signal */
	gtk_signal_emit (GTK_OBJECT (user_data),
			 rotate_select_signals[CHANGED]);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Draw a mini-preview canvas.                                    */
/*--------------------------------------------------------------------------*/
static GtkWidget *
mini_preview_canvas_new (void)
{
	GtkWidget *wcanvas = NULL;

	/* Create a canvas */
	gtk_widget_push_visual (gdk_rgb_get_visual ());
	gtk_widget_push_colormap (gdk_rgb_get_cmap ());
	wcanvas = gnome_canvas_new_aa ();
	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();

	gtk_widget_set_usize (GTK_WIDGET (wcanvas),
			      MINI_PREVIEW_MAX_PIXELS + 4,
			      MINI_PREVIEW_MAX_PIXELS + 4);

	gtk_object_set_data (GTK_OBJECT (wcanvas), "label_item", NULL);

	return wcanvas;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Update mini-preview canvas from new template.                  */
/*--------------------------------------------------------------------------*/
static void
mini_preview_canvas_update (GnomeCanvas * canvas,
			    glTemplate * template,
			    gboolean rotate_flag)
{
	gdouble canvas_scale;
	GnomeCanvasGroup *group = NULL;
	GnomeCanvasItem *label_item = NULL;
	gdouble m, w, h;

	/* Fetch our data from canvas */
	label_item = gtk_object_get_data (GTK_OBJECT (canvas), "label_item");

	m = MAX (template->label_width, template->label_height);
	canvas_scale = (MINI_PREVIEW_MAX_PIXELS) / m;

	/* scale and size canvas */
	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (canvas), canvas_scale);
	group = gnome_canvas_root (GNOME_CANVAS (canvas));
	gnome_canvas_set_scroll_region (GNOME_CANVAS (canvas),
					-m / 2.0, -m / 2.0, +m / 2.0, +m / 2.0);

	/* remove old label outline */
	if (label_item != NULL) {
		gtk_object_destroy (GTK_OBJECT (label_item));
	}

	/* draw mini label outline */
	if (!rotate_flag) {
		w = template->label_width;
		h = template->label_height;
	} else {
		w = template->label_height;
		h = template->label_width;
	}
	switch (template->style) {
	case GL_TEMPLATE_STYLE_RECT:
		label_item = gnome_canvas_item_new (group,
						    gnome_canvas_rect_get_type(),
						    "x1", -w / 2.0,
						    "y1", -h / 2.0,
						    "x2", +w / 2.0,
						    "y2", +h / 2.0,
						    "width_pixels", 1,
						    "outline_color", "black",
						    "fill_color", "white",
						    NULL);
		break;
	case GL_TEMPLATE_STYLE_ROUND:
	case GL_TEMPLATE_STYLE_CD:
		label_item = gnome_canvas_item_new (group,
						    gnome_canvas_ellipse_get_type(),
						    "x1", -w / 2.0,
						    "y1", -h / 2.0,
						    "x2", +w / 2.0,
						    "y2", +h / 2.0,
						    "width_pixels", 1,
						    "outline_color", "black",
						    "fill_color", "white",
						    NULL);
		break;
	default:
		WARN ("Unknown label style");
		break;
	}

	gtk_object_set_data (GTK_OBJECT (canvas), "label_item", label_item);

}

/****************************************************************************/
/* query state of widget.                                                   */
/****************************************************************************/
gboolean
gl_rotate_select_get_state (glRotateSelect * rotate_select)
{
	return
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
					  (rotate_select->rotate_check));
}

/****************************************************************************/
/* set state of widget.                                                     */
/****************************************************************************/
void
gl_rotate_select_set_state (glRotateSelect * rotate_select,
			    gboolean state)
{
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				      (rotate_select->rotate_check), state);
}

/****************************************************************************/
/* set template for widget.                                                 */
/****************************************************************************/
void
gl_rotate_select_set_template_name (glRotateSelect * rotate_select,
				    gchar * name)
{
	glTemplate *template;

	template = gl_template_from_name (name);
	rotate_select->template = template;

	if (template->label_width != template->label_height) {
		gtk_widget_set_sensitive (rotate_select->rotate_check, TRUE);
	} else {
		gtk_widget_set_sensitive (rotate_select->rotate_check, FALSE);
	}

	mini_preview_canvas_update (GNOME_CANVAS (rotate_select->canvas),
				    template, FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				      (rotate_select->rotate_check), FALSE);
}
