/*
 *  rotate-label-button.c
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

#include "rotate-label-button.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <math.h>

#include <libglabels/libglabels.h>
#include "mini-label-preview.h"
#include "hig.h"
#include "marshal.h"

#include "debug.h"


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

#define SIZE 48


/*===========================================*/
/* Private types                             */
/*===========================================*/

struct _glRotateLabelButtonPrivate {

        GtkWidget    *no_rotate_radio;
        GtkWidget    *rotate_radio;
        GtkWidget    *no_rotate_preview;
        GtkWidget    *rotate_preview;
};

enum {
	CHANGED,
	LAST_SIGNAL
};


/*===========================================*/
/* Private globals                           */
/*===========================================*/

static gint rotate_label_button_signals[LAST_SIGNAL] = { 0 };


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_rotate_label_button_finalize    (GObject                *object);

static void toggled_cb                         (GtkToggleButton        *toggle,
						gpointer                user_data);


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glRotateLabelButton, gl_rotate_label_button, GTK_TYPE_HBOX);


static void
gl_rotate_label_button_class_init (glRotateLabelButtonClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	gl_rotate_label_button_parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_rotate_label_button_finalize;

	rotate_label_button_signals[CHANGED] =
	    g_signal_new ("changed",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glRotateLabelButtonClass, changed),
			  NULL, NULL,
			  gl_marshal_VOID__VOID,
			  G_TYPE_NONE, 0);

}


static void
gl_rotate_label_button_init (glRotateLabelButton *this)
{
        GtkWidget *vbox;
        GtkWidget *label;

        this->priv = g_new0 (glRotateLabelButtonPrivate, 1);

        gtk_container_set_border_width (GTK_CONTAINER (this), GL_HIG_PAD2);

        this->priv->no_rotate_radio = gtk_radio_button_new (NULL);
        gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (this->priv->no_rotate_radio),
                                    FALSE);
        vbox = gtk_vbox_new (FALSE, 1);
        this->priv->no_rotate_preview = gl_mini_label_preview_new (SIZE, SIZE);
        gtk_box_pack_start (GTK_BOX (vbox), this->priv->no_rotate_preview, FALSE, FALSE, 0);
        label = gtk_label_new (_("Normal"));
        gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
        gtk_container_add (GTK_CONTAINER (this->priv->no_rotate_radio), vbox);


        this->priv->rotate_radio    = gtk_radio_button_new_from_widget (GTK_RADIO_BUTTON (this->priv->no_rotate_radio));
        gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (this->priv->rotate_radio),
                                    FALSE);
        vbox = gtk_vbox_new (FALSE, 1);
        this->priv->rotate_preview    = gl_mini_label_preview_new (SIZE, SIZE);
        gtk_box_pack_start (GTK_BOX (vbox), this->priv->rotate_preview, FALSE, FALSE, 0);
        label = gtk_label_new (_("Rotated"));
        gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
        gtk_container_add (GTK_CONTAINER (this->priv->rotate_radio), vbox);

        gtk_box_pack_start (GTK_BOX (this),
                            this->priv->no_rotate_radio,
                            FALSE, FALSE, GL_HIG_PAD1);
        gtk_box_pack_start (GTK_BOX (this),
                            this->priv->rotate_radio,
                            FALSE, FALSE, GL_HIG_PAD1);
        
        /* Connect signals to controls */
	g_signal_connect (G_OBJECT (this->priv->no_rotate_radio),
			  "toggled",
			  G_CALLBACK (toggled_cb), this);
	g_signal_connect (G_OBJECT (this->priv->rotate_radio),
			  "toggled",
			  G_CALLBACK (toggled_cb), this);
}


static void
gl_rotate_label_button_finalize (GObject *object)
{
	glRotateLabelButton      *this = GL_ROTATE_LABEL_BUTTON (object);

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_ROTATE_LABEL_BUTTON (object));

	g_free (this->priv);

	G_OBJECT_CLASS (gl_rotate_label_button_parent_class)->finalize (object);
}


GtkWidget *
gl_rotate_label_button_new (void)
{
	glRotateLabelButton *this;

	this = g_object_new (gl_rotate_label_button_get_type (), NULL);

	return GTK_WIDGET (this);
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
		       rotate_label_button_signals[CHANGED], 0);

}


/****************************************************************************/
/* query state of widget.                                                   */
/****************************************************************************/
gboolean
gl_rotate_label_button_get_state (glRotateLabelButton *this)
{
	return
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
					  (this->priv->rotate_radio));
}

/****************************************************************************/
/* set state of widget.                                                     */
/****************************************************************************/
void
gl_rotate_label_button_set_state (glRotateLabelButton *this,
                                  gboolean             state)
{
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				      (this->priv->rotate_radio), state);
}

/****************************************************************************/
/* set template for widget.                                                 */
/****************************************************************************/
void
gl_rotate_label_button_set_template_name (glRotateLabelButton *this,
                                          gchar               *name)
{
	lglTemplate               *template;
	const lglTemplateFrame    *frame;
	gdouble                    raw_w, raw_h;

	if (name == NULL)
	{
		gtk_widget_set_sensitive (this->priv->no_rotate_radio, FALSE);
		gtk_widget_set_sensitive (this->priv->rotate_radio, FALSE);
	}
	else
	{
		template = lgl_db_lookup_template_from_name (name);
                frame = (lglTemplateFrame *)template->frames->data;

                gl_mini_label_preview_set_by_name (GL_MINI_LABEL_PREVIEW (this->priv->no_rotate_preview),
                                                   name, FALSE);
                gl_mini_label_preview_set_by_name (GL_MINI_LABEL_PREVIEW (this->priv->rotate_preview),
                                                   name, TRUE);

		lgl_template_frame_get_size (frame, &raw_w, &raw_h);
		gtk_widget_set_sensitive (this->priv->no_rotate_radio,
					  (raw_w != raw_h));
		gtk_widget_set_sensitive (this->priv->rotate_radio,
					  (raw_w != raw_h));

                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                              (this->priv->no_rotate_radio), TRUE);
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
                                              (this->priv->rotate_radio), FALSE);
	}

}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
