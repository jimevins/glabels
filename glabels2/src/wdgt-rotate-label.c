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
#include <gtk/gtkradiobutton.h>
#include <gtk/gtkdrawingarea.h>
#include <math.h>

#include "marshal.h"
#include "color.h"
#include "hig.h"
#include <libglabels/db.h>
#include "cairo-label-path.h"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/
#define MINI_PREVIEW_MAX_PIXELS 48
#define MINI_PREVIEW_CANVAS_PIXELS (MINI_PREVIEW_MAX_PIXELS + 8)

#define LINE_COLOR             GL_COLOR(0,0,0)
#define FILL_COLOR             GL_COLOR(255,255,255)
#define SHADOW_COLOR           GL_COLOR_A(192,192,192,128)

#define LINE_WIDTH_PIXELS    1.0

#define SHADOW_X_OFFSET 3
#define SHADOW_Y_OFFSET 3


/*===========================================*/
/* Private types                             */
/*===========================================*/

struct _glWdgtRotateLabelPrivate {

        GtkWidget    *no_rotate_radio;
        GtkWidget    *rotate_radio;
        GtkWidget    *no_rotate_image;
        GtkWidget    *rotate_image;

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

static void toggled_cb                         (GtkToggleButton        *toggle,
						gpointer                user_data);

static GdkPixbuf *create_pixbuf                (lglTemplate            *template,
						gboolean                rotate_flag);


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glWdgtRotateLabel, gl_wdgt_rotate_label, GTK_TYPE_HBOX);


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

        gtk_container_set_border_width (GTK_CONTAINER (rotate_label), GL_HIG_PAD2);

        rotate_label->priv->no_rotate_radio = gtk_radio_button_new_with_label (NULL, _("Normal"));
        rotate_label->priv->rotate_radio    = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (rotate_label->priv->no_rotate_radio), _("Rotated"));

        gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (rotate_label->priv->no_rotate_radio),
                                    FALSE);
        gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (rotate_label->priv->rotate_radio),
                                    FALSE);

        gtk_button_set_image_position (GTK_BUTTON (rotate_label->priv->no_rotate_radio),
                                       GTK_POS_TOP);
        gtk_button_set_image_position (GTK_BUTTON (rotate_label->priv->rotate_radio),
                                       GTK_POS_TOP);

        rotate_label->priv->no_rotate_image = gtk_image_new ();
        rotate_label->priv->rotate_image    = gtk_image_new ();

        gtk_button_set_image (GTK_BUTTON (rotate_label->priv->no_rotate_radio),
                              rotate_label->priv->no_rotate_image);
        gtk_button_set_image (GTK_BUTTON (rotate_label->priv->rotate_radio),
                              rotate_label->priv->rotate_image);

        gtk_box_pack_start (GTK_BOX (rotate_label),
                            rotate_label->priv->no_rotate_radio,
                            FALSE, FALSE, GL_HIG_PAD1);
        gtk_box_pack_start (GTK_BOX (rotate_label),
                            rotate_label->priv->rotate_radio,
                            FALSE, FALSE, GL_HIG_PAD1);
        
        /* Connect signals to controls */
	g_signal_connect (G_OBJECT (rotate_label->priv->no_rotate_radio),
			  "toggled",
			  G_CALLBACK (toggled_cb), rotate_label);
	g_signal_connect (G_OBJECT (rotate_label->priv->rotate_radio),
			  "toggled",
			  G_CALLBACK (toggled_cb), rotate_label);
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

	return GTK_WIDGET (rotate_label);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change of check button                    */
/*--------------------------------------------------------------------------*/
static void
toggled_cb (GtkToggleButton *toggle,
            gpointer         user_data)
{

	/* Emit our "changed" signal */
	g_signal_emit (G_OBJECT (user_data),
		       wdgt_rotate_label_signals[CHANGED], 0);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Update mini-preview from template           .                  */
/*--------------------------------------------------------------------------*/
static GdkPixbuf *
create_pixbuf (lglTemplate    *template,
               gboolean        rotate_flag)
{
        GdkPixbuf                 *pixbuf;
        cairo_surface_t           *surface;
	const lglTemplateFrame    *frame;
	gdouble                    m, m_canvas, w, h, scale;
	GtkStyle                  *style;
	cairo_t                   *cr;

        /* Create pixbuf and cairo context. */
        pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, MINI_PREVIEW_CANVAS_PIXELS, MINI_PREVIEW_CANVAS_PIXELS);
        surface = cairo_image_surface_create_for_data (gdk_pixbuf_get_pixels (pixbuf),
                                                       CAIRO_FORMAT_RGB24,
                                                       gdk_pixbuf_get_width (pixbuf),
                                                       gdk_pixbuf_get_height (pixbuf),
                                                       gdk_pixbuf_get_rowstride (pixbuf));


	if (template == NULL)
	{
		return pixbuf;
	}

        frame = (lglTemplateFrame *)template->frames->data;

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

	cr = cairo_create (surface);
        cairo_surface_destroy (surface);

        /* Clear pixbuf */
        cairo_save (cr);
        cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
        cairo_paint (cr);
        cairo_restore (cr);

        cairo_set_antialias (cr, CAIRO_ANTIALIAS_GRAY);
  
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

	cairo_set_source_rgb (cr, GL_COLOR_RGB_ARGS (SHADOW_COLOR));
        cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
	cairo_fill (cr);
        cairo_restore (cr);

        /*
         * Label + outline
         */
        gl_cairo_label_path (cr, template, rotate_flag, FALSE);

	cairo_set_source_rgb (cr, GL_COLOR_RGB_ARGS (FILL_COLOR));
        cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
	cairo_fill_preserve (cr);

	cairo_set_line_width (cr, LINE_WIDTH_PIXELS/scale);
	cairo_set_source_rgb (cr, GL_COLOR_RGB_ARGS (LINE_COLOR));
	cairo_stroke (cr);


	cairo_destroy (cr);

        return pixbuf;
}

/****************************************************************************/
/* query state of widget.                                                   */
/****************************************************************************/
gboolean
gl_wdgt_rotate_label_get_state (glWdgtRotateLabel *rotate_label)
{
	return
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
					  (rotate_label->priv->rotate_radio));
}

/****************************************************************************/
/* set state of widget.                                                     */
/****************************************************************************/
void
gl_wdgt_rotate_label_set_state (glWdgtRotateLabel *rotate_label,
				gboolean state)
{
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				      (rotate_label->priv->rotate_radio), state);
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

		gtk_widget_set_sensitive (rotate_label->priv->no_rotate_radio, FALSE);
		gtk_widget_set_sensitive (rotate_label->priv->rotate_radio, FALSE);
	}
	else
	{
		template = lgl_db_lookup_template_from_name (name);
                frame = (lglTemplateFrame *)template->frames->data;

		rotate_label->priv->template = template;
		lgl_template_frame_get_size (frame, &raw_w, &raw_h);

                gtk_image_set_from_pixbuf (GTK_IMAGE (rotate_label->priv->no_rotate_image),
                                           create_pixbuf (rotate_label->priv->template, FALSE));
                gtk_image_set_from_pixbuf (GTK_IMAGE (rotate_label->priv->rotate_image),
                                           create_pixbuf (rotate_label->priv->template, TRUE));

		gtk_widget_set_sensitive (rotate_label->priv->no_rotate_radio,
					  (raw_w != raw_h));
		gtk_widget_set_sensitive (rotate_label->priv->rotate_radio,
					  (raw_w != raw_h));

                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                              (rotate_label->priv->no_rotate_radio), TRUE);
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                              (rotate_label->priv->rotate_radio), FALSE);

	}

}

