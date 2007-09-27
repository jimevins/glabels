/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_rotate_label.c:  label rotate selection widget module
 *
 *  Copyright (C) 2001-2006  Jim Evins <evins@snaught.com>.
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

#include "wdgt-rotate-label.h"

#include <glib/gi18n.h>
#include <glade/glade-xml.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkdrawingarea.h>
#include <math.h>

#include "marshal.h"
#include "color.h"
#include <libglabels/template.h>
#include "cairo-label-path.h"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/
#define MINI_PREVIEW_MAX_PIXELS 48
#define MINI_PREVIEW_CANVAS_PIXELS (MINI_PREVIEW_MAX_PIXELS + 8)

#define LINE_COLOR             GL_COLOR(0,0,0)
#define FILL_COLOR             GL_COLOR(255,255,255)

#define LINE_WIDTH_PIXELS    1.0

#define SHADOW_X_OFFSET 3
#define SHADOW_Y_OFFSET 3


/*===========================================*/
/* Private types                             */
/*===========================================*/

struct _glWdgtRotateLabelPrivate {

        GtkWidget    *rotate_check;
        GtkWidget    *rotate_drawingarea;

	lglTemplate  *template;
};

enum {
	CHANGED,
	LAST_SIGNAL
};

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static gint wdgt_rotate_label_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_wdgt_rotate_label_finalize      (GObject                *object);

static void gl_wdgt_rotate_label_construct     (glWdgtRotateLabel      *rotate_label);

static void entry_changed_cb                   (GtkToggleButton        *toggle,
						gpointer                user_data);

static void drawingarea_update                 (GtkDrawingArea         *drawing_area,
						lglTemplate            *template,
						gboolean                rotate_flag);

static gboolean expose_cb                      (GtkWidget              *drawingarea,
						GdkEventExpose         *event,
						gpointer                user_data);


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glWdgtRotateLabel, gl_wdgt_rotate_label, GTK_TYPE_VBOX);


static void
gl_wdgt_rotate_label_class_init (glWdgtRotateLabelClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	gl_wdgt_rotate_label_parent_class = g_type_class_peek_parent (class);

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
gl_wdgt_rotate_label_init (glWdgtRotateLabel *rotate_label)
{
        rotate_label->priv = g_new0 (glWdgtRotateLabelPrivate, 1);
}

static void
gl_wdgt_rotate_label_finalize (GObject *object)
{
	glWdgtRotateLabel      *rotate_label = GL_WDGT_ROTATE_LABEL (object);

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WDGT_ROTATE_LABEL (object));

	if (rotate_label->priv->template) {
		lgl_template_free (rotate_label->priv->template);
		rotate_label->priv->template = NULL;
	}
	g_free (rotate_label->priv);

	G_OBJECT_CLASS (gl_wdgt_rotate_label_parent_class)->finalize (object);
}

GtkWidget *
gl_wdgt_rotate_label_new (void)
{
	glWdgtRotateLabel *rotate_label;

	rotate_label = g_object_new (gl_wdgt_rotate_label_get_type (), NULL);

	gl_wdgt_rotate_label_construct (rotate_label);

	return GTK_WIDGET (rotate_label);
}

/*--------------------------------------------------------------------------*/
/* Construct composite widget.                                              */
/*--------------------------------------------------------------------------*/
static void
gl_wdgt_rotate_label_construct (glWdgtRotateLabel *rotate_label)
{
        GladeXML  *gui;
	GtkWidget *hbox;

        g_return_if_fail (GL_IS_WDGT_ROTATE_LABEL (rotate_label));
        g_return_if_fail (rotate_label->priv != NULL);

        gui = glade_xml_new (GLABELS_GLADE_DIR "wdgt-rotate-label.glade",
                             "rotate_hbox", NULL);

        if (!gui) {
                g_critical ("Could not open wdgt-media-select.glade. gLabels may not be installed correctly!");
                return;
        }

        hbox = glade_xml_get_widget (gui, "rotate_hbox");
        gtk_container_add (GTK_CONTAINER (rotate_label), hbox);

        rotate_label->priv->rotate_check       = glade_xml_get_widget (gui, "rotate_check");
        rotate_label->priv->rotate_drawingarea = glade_xml_get_widget (gui, "rotate_drawingarea");

        g_object_unref (gui);


	gtk_widget_set_size_request (rotate_label->priv->rotate_drawingarea,
                                     MINI_PREVIEW_CANVAS_PIXELS,
                                     MINI_PREVIEW_CANVAS_PIXELS);

	/* Connect signals to controls */
	g_signal_connect (G_OBJECT (rotate_label->priv->rotate_check),
			  "toggled",
			  G_CALLBACK (entry_changed_cb), rotate_label);
	g_signal_connect (G_OBJECT (rotate_label->priv->rotate_drawingarea),
			  "expose_event",
			  G_CALLBACK (expose_cb), rotate_label);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change of check button                    */
/*--------------------------------------------------------------------------*/
static void
entry_changed_cb (GtkToggleButton *toggle,
		  gpointer         user_data)
{
	glWdgtRotateLabel *rotate_label = GL_WDGT_ROTATE_LABEL (user_data);

	if (rotate_label->priv->template != NULL) {
		/* Update mini_preview canvas & details with template */
		drawingarea_update (GTK_DRAWING_AREA (rotate_label->priv->rotate_drawingarea),
				    rotate_label->priv->template,
				    gtk_toggle_button_get_active (toggle));
	}

	/* Emit our "changed" signal */
	g_signal_emit (G_OBJECT (user_data),
		       wdgt_rotate_label_signals[CHANGED], 0);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Update mini-preview from template           .                  */
/*--------------------------------------------------------------------------*/
static void
drawingarea_update (GtkDrawingArea *drawing_area,
		    lglTemplate    *template,
		    gboolean        rotate_flag)
{
	const lglTemplateFrame    *frame;
	gdouble                    m, m_canvas, w, h, scale;
	GtkStyle                  *style;
	guint                      line_color, fill_color, shadow_color;
	cairo_t                   *cr;

	if (!GTK_WIDGET_DRAWABLE (GTK_WIDGET (drawing_area)))
	{
		return;
	}

        /* Clear surface */
	gdk_window_clear (GTK_WIDGET (drawing_area)->window);

	if (template == NULL)
	{
		return;
	}

	frame = lgl_template_get_first_frame (template);

        if (rotate_flag)
        {
                lgl_template_frame_get_size (frame, &h, &w);
        }
        else
        {
                lgl_template_frame_get_size (frame, &w, &h);
        }
	m = MAX (w, h);
	scale = MINI_PREVIEW_MAX_PIXELS / m;
	m_canvas = MINI_PREVIEW_CANVAS_PIXELS / scale;

        style = gtk_widget_get_style (GTK_WIDGET (drawing_area));

	/* Adjust sensitivity (should the canvas be grayed?) */
	if (w != h) {
		line_color = LINE_COLOR;
		fill_color = FILL_COLOR;
	} else {
		line_color = gl_color_from_gdk_color (&style->text[GTK_STATE_INSENSITIVE]);
		fill_color = gl_color_from_gdk_color (&style->base[GTK_STATE_INSENSITIVE]);
	}

	shadow_color = gl_color_from_gdk_color (&style->bg[GTK_STATE_ACTIVE]);


	cr = gdk_cairo_create (GTK_WIDGET (drawing_area)->window);
  
	cairo_identity_matrix (cr);
	cairo_translate (cr, MINI_PREVIEW_CANVAS_PIXELS/2, MINI_PREVIEW_CANVAS_PIXELS/2);
        cairo_scale (cr, scale, scale);
        cairo_translate (cr, -w/2.0, -h/2.0);

        /*
         * Shadow
         */
        cairo_save (cr);
        cairo_translate (cr, SHADOW_X_OFFSET/scale, SHADOW_Y_OFFSET/scale);
        gl_cairo_label_path (cr, template, rotate_flag, FALSE);

	cairo_set_source_rgb (cr, GL_COLOR_RGB_ARGS (shadow_color));
        cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
	cairo_fill (cr);
        cairo_restore (cr);

        /*
         * Label + outline
         */
        gl_cairo_label_path (cr, template, rotate_flag, FALSE);

	cairo_set_source_rgb (cr, GL_COLOR_RGB_ARGS (fill_color));
        cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
	cairo_fill_preserve (cr);

	cairo_set_line_width (cr, LINE_WIDTH_PIXELS/scale);
	cairo_set_source_rgb (cr, GL_COLOR_RGB_ARGS (line_color));
	cairo_stroke (cr);


	cairo_destroy (cr);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Expose handler.                                                */
/*--------------------------------------------------------------------------*/
static gboolean
expose_cb (GtkWidget *drawingarea, GdkEventExpose *event, gpointer user_data)
{
	glWdgtRotateLabel *rotate_label = GL_WDGT_ROTATE_LABEL (user_data);

	drawingarea_update (GTK_DRAWING_AREA (drawingarea),
			    rotate_label->priv->template,
			    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rotate_label->priv->rotate_check)));

	return FALSE;
}

/****************************************************************************/
/* query state of widget.                                                   */
/****************************************************************************/
gboolean
gl_wdgt_rotate_label_get_state (glWdgtRotateLabel *rotate_label)
{
	return
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
					  (rotate_label->priv->rotate_check));
}

/****************************************************************************/
/* set state of widget.                                                     */
/****************************************************************************/
void
gl_wdgt_rotate_label_set_state (glWdgtRotateLabel *rotate_label,
				gboolean state)
{
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				      (rotate_label->priv->rotate_check), state);
}

/****************************************************************************/
/* set template for widget.                                                 */
/****************************************************************************/
void
gl_wdgt_rotate_label_set_template_name (glWdgtRotateLabel *rotate_label,
					gchar             *name)
{
	lglTemplate               *template;
	const lglTemplateFrame    *frame;
	gdouble                    raw_w, raw_h;

	if (name == NULL)
	{
		rotate_label->priv->template = NULL;

		gtk_widget_set_sensitive (rotate_label->priv->rotate_check,
					  FALSE);

		drawingarea_update (GTK_DRAWING_AREA (rotate_label->priv->rotate_drawingarea),
				    NULL,
				    FALSE);
	}
	else
	{
		template   = lgl_template_from_name (name);
		frame = lgl_template_get_first_frame (template);

		rotate_label->priv->template = template;
		lgl_template_frame_get_size (frame, &raw_w, &raw_h);

		gtk_widget_set_sensitive (rotate_label->priv->rotate_check,
					  (raw_w != raw_h));

		drawingarea_update (GTK_DRAWING_AREA (rotate_label->priv->rotate_drawingarea),
				    rotate_label->priv->template,
				    FALSE);
	}
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				      (rotate_label->priv->rotate_check), FALSE);
}

