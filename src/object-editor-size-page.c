/*
 *  object-editor-size-page.c
 *  Copyright (C) 2003-2009  Jim Evins <evins@snaught.com>.
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

#include "object-editor.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <math.h>

#include "prefs.h"
#include "wdgt-chain-button.h"
#include "builder-util.h"
#include "units-util.h"

#include "object-editor-private.h"

#include "debug.h"


/*===========================================*/
/* Private macros                            */
/*===========================================*/


/*===========================================*/
/* Private data types                        */
/*===========================================*/


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void aspect_toggle_cb                    (glObjectEditor        *editor);
static void size_reset_cb                       (glObjectEditor        *editor);
static void w_spin_cb                           (glObjectEditor        *editor);
static void h_spin_cb                           (glObjectEditor        *editor);


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare size page.                                             */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_prepare_size_page (glObjectEditor       *editor)
{
        lglUnits      units;
	const gchar  *units_string;
	gdouble       climb_rate;
	gint          digits;

	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
        gl_builder_util_get_widgets (editor->priv->builder,
                                     "size_page_vbox",          &editor->priv->size_page_vbox,
                                     "size_w_spin",             &editor->priv->size_w_spin,
                                     "size_h_spin",             &editor->priv->size_h_spin,
                                     "size_w_units_label",      &editor->priv->size_w_units_label,
                                     "size_h_units_label",      &editor->priv->size_h_units_label,
                                     "size_aspect_vbox",        &editor->priv->size_aspect_vbox,
                                     "size_reset_image_button", &editor->priv->size_reset_image_button,
                                     NULL);

	editor->priv->size_aspect_checkbutton = gl_wdgt_chain_button_new (GL_WDGT_CHAIN_RIGHT);
        gtk_widget_set_tooltip_text (editor->priv->size_aspect_checkbutton, _("Lock aspect ratio."));
	gl_wdgt_chain_button_set_active (GL_WDGT_CHAIN_BUTTON(editor->priv->size_aspect_checkbutton),
                                         TRUE);
        gtk_box_pack_start (GTK_BOX (editor->priv->size_aspect_vbox),
                            editor->priv->size_aspect_checkbutton,
                            TRUE, TRUE, 0);


	/* Get configuration information */
        units = gl_prefs_model_get_units (gl_prefs);
	units_string = lgl_units_get_name (units);
	editor->priv->units_per_point = lgl_units_get_units_per_point (units);
	climb_rate = gl_units_util_get_step_size (units);
	digits = gl_units_util_get_precision (units);

	/* Modify widgets based on configuration */
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON(editor->priv->size_w_spin), digits);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON(editor->priv->size_w_spin),
					climb_rate, 10.0*climb_rate);
	gtk_label_set_text (GTK_LABEL(editor->priv->size_w_units_label), units_string);
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON(editor->priv->size_h_spin), digits);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON(editor->priv->size_h_spin),
					climb_rate, 10.0*climb_rate);
	gtk_label_set_text (GTK_LABEL(editor->priv->size_h_units_label), units_string);


	/* Connect signals */
	g_signal_connect_swapped (G_OBJECT (editor->priv->size_aspect_checkbutton),
				  "toggled",
				  G_CALLBACK (aspect_toggle_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->size_w_spin),
				  "value-changed",
				  G_CALLBACK (w_spin_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->size_h_spin),
				  "value-changed",
				  G_CALLBACK (h_spin_cb),
				  G_OBJECT (editor));
        g_signal_connect_swapped (G_OBJECT (editor->priv->size_reset_image_button),
                                  "clicked",
                                  G_CALLBACK (size_reset_cb),
                                  G_OBJECT (editor));

	gl_debug (DEBUG_EDITOR, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Maintain aspect ratio checkbox callback.                       */
/*--------------------------------------------------------------------------*/
static void
aspect_toggle_cb (glObjectEditor *editor)
{
        glWdgtChainButton *toggle;
	gdouble            w, h;

	gl_debug (DEBUG_EDITOR, "START");

	toggle = GL_WDGT_CHAIN_BUTTON (editor->priv->size_aspect_checkbutton);

        if (gl_wdgt_chain_button_get_active (toggle)) {
                                                                                
                w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(editor->priv->size_w_spin));
                h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(editor->priv->size_h_spin));
                                                                                
                editor->priv->size_aspect_ratio = h / w;
                                                                                
        }

	gl_debug (DEBUG_EDITOR, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  W spin button changed callback.                                */
/*--------------------------------------------------------------------------*/
static void
w_spin_cb (glObjectEditor *editor)
{
	gdouble            w, h;
        glWdgtChainButton *toggle;

	gl_debug (DEBUG_EDITOR, "START");

	toggle = GL_WDGT_CHAIN_BUTTON (editor->priv->size_aspect_checkbutton);

        if (gl_wdgt_chain_button_get_active (toggle)) {

		w = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->size_w_spin));
                                                                                
                h = w * editor->priv->size_aspect_ratio;
                                                                                
                /* Update our sibling control, blocking recursion. */
                g_signal_handlers_block_by_func (G_OBJECT (editor->priv->size_h_spin), h_spin_cb, editor);
                gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->size_h_spin), h);
                g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->size_h_spin), h_spin_cb, editor);
        }
                                                                                
        gl_object_editor_size_changed_cb (editor);
                                                                                
	gl_debug (DEBUG_EDITOR, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  H spin button changed callback.                                */
/*--------------------------------------------------------------------------*/
static void
h_spin_cb (glObjectEditor *editor)
{
	gdouble            w, h;
        glWdgtChainButton *toggle;

	gl_debug (DEBUG_EDITOR, "START");

        toggle = GL_WDGT_CHAIN_BUTTON (editor->priv->size_aspect_checkbutton);
                                                                                
        if (gl_wdgt_chain_button_get_active (toggle)) {

		h = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->size_h_spin));
                                                                                
                w = h / editor->priv->size_aspect_ratio;
                                                                                
                /* Update our sibling control, blocking recursion. */
                g_signal_handlers_block_by_func (G_OBJECT (editor->priv->size_w_spin), w_spin_cb, editor);
                gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->size_w_spin), w);
                g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->size_w_spin), w_spin_cb, editor);
        }
                                                                                
        gl_object_editor_size_changed_cb (editor);
                                                                                
	gl_debug (DEBUG_EDITOR, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Reset image size callback.                                     */
/*--------------------------------------------------------------------------*/
static void
size_reset_cb (glObjectEditor *editor)
{
	gdouble w_base, h_base;
	gdouble w_max, h_max, wh_max;
	gdouble aspect_ratio;

	gl_debug (DEBUG_EDITOR, "START");


        g_signal_handlers_block_by_func (G_OBJECT (editor->priv->size_w_spin), w_spin_cb, editor);
        g_signal_handlers_block_by_func (G_OBJECT (editor->priv->size_h_spin), h_spin_cb, editor);


	w_base = editor->priv->w_base;
	h_base = editor->priv->h_base;

	w_max = editor->priv->w_max;
	h_max = editor->priv->h_max;
        wh_max = MAX( w_max, h_max );

	if ( (w_base > wh_max) || (h_base > wh_max) ) {

		aspect_ratio = h_base / w_base;

		if ( aspect_ratio < 1.0 ) {
			w_base = wh_max;
			h_base = wh_max * aspect_ratio;

		} else {
			w_base = wh_max / aspect_ratio;
			h_base = wh_max;
		}
	}

	w_base *= editor->priv->units_per_point;
	h_base *= editor->priv->units_per_point;

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->size_w_spin),
				   w_base);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->size_h_spin),
				   h_base);


        g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->size_w_spin), w_spin_cb, editor);
        g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->size_h_spin), h_spin_cb, editor);


        gl_object_editor_size_changed_cb (editor);

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Set size.                                                                 */
/*****************************************************************************/
void
gl_object_editor_set_size (glObjectEditor      *editor,
                           gdouble              w,
                           gdouble              h)
{
	gl_debug (DEBUG_EDITOR, "START");


        g_signal_handlers_block_by_func (G_OBJECT (editor->priv->size_w_spin), w_spin_cb, editor);
        g_signal_handlers_block_by_func (G_OBJECT (editor->priv->size_h_spin), h_spin_cb, editor);


	/* save a copy in internal units */
	editor->priv->w = w;
	editor->priv->h = h;

	/* convert internal units to displayed units */
	gl_debug (DEBUG_EDITOR, "internal w,h = %g, %g", w, h);
	w *= editor->priv->units_per_point;
	h *= editor->priv->units_per_point;
	gl_debug (DEBUG_EDITOR, "display w,h = %g, %g", w, h);

	/* Set widget values */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->size_w_spin), w);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->size_h_spin), h);

	/* Update aspect ratio */
	editor->priv->size_aspect_ratio = h / w;


        g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->size_w_spin), w_spin_cb, editor);
        g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->size_h_spin), h_spin_cb, editor);


	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Set maximum size.                                                         */
/*****************************************************************************/
void
gl_object_editor_set_max_size (glObjectEditor      *editor,
			       gdouble              w_max,
			       gdouble              h_max)
{
	gdouble tmp;
        gdouble wh_max;

	gl_debug (DEBUG_EDITOR, "START");


        g_signal_handlers_block_by_func (G_OBJECT (editor->priv->size_w_spin), w_spin_cb, editor);
        g_signal_handlers_block_by_func (G_OBJECT (editor->priv->size_h_spin), h_spin_cb, editor);


        /* save a copy in internal units */
        editor->priv->w_max = w_max;
        editor->priv->h_max = h_max;

        /* convert internal units to displayed units */
        gl_debug (DEBUG_EDITOR, "internal w_max,h_max = %g, %g", w_max, h_max);
        w_max *= editor->priv->units_per_point;
        h_max *= editor->priv->units_per_point;
        wh_max = MAX( w_max, h_max );
        gl_debug (DEBUG_EDITOR, "display w_max,h_max = %g, %g", w_max, h_max);

        /* Set widget values */
        tmp = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->size_w_spin));
        gtk_spin_button_set_range (GTK_SPIN_BUTTON (editor->priv->size_w_spin),
                                   0.0, 2.0*wh_max);
        gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->size_w_spin), tmp);
        tmp = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->size_h_spin));
        gtk_spin_button_set_range (GTK_SPIN_BUTTON (editor->priv->size_h_spin),
                                   0.0, 2.0*wh_max);
        gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->size_h_spin), tmp);


        g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->size_w_spin), w_spin_cb, editor);
        g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->size_h_spin), h_spin_cb, editor);


	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Set base or natural size of image.                                        */
/*****************************************************************************/
void
gl_object_editor_set_base_size    (glObjectEditor      *editor,
				   gdouble              w_base,
				   gdouble              h_base)
{
	gl_debug (DEBUG_EDITOR, "Setting w_base = %g", w_base);
	gl_debug (DEBUG_EDITOR, "Setting h_base = %g", h_base);

	editor->priv->w_base = w_base;
	editor->priv->h_base = h_base;
}


/*****************************************************************************/
/* Query size.                                                               */
/*****************************************************************************/
void
gl_object_editor_get_size (glObjectEditor      *editor,
			   gdouble             *w,
			   gdouble             *h)
{
	gl_debug (DEBUG_EDITOR, "START");


	/* Get values from widgets */
	*w = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->size_w_spin));
	*h = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->size_h_spin));

	/* convert everything back to our internal units (points) */
	*w /= editor->priv->units_per_point;
	*h /= editor->priv->units_per_point;

	/* save a copy in internal units */
	editor->priv->w = *w;
	editor->priv->h = *h;

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* PRIVATE. Prefs changed callback.  Update units related items.            */
/*****************************************************************************/
void
size_prefs_changed_cb (glObjectEditor *editor)
{
        lglUnits      units;
	const gchar  *units_string;
	gdouble       climb_rate;
	gint          digits;

	gl_debug (DEBUG_EDITOR, "START");


        g_signal_handlers_block_by_func (G_OBJECT (editor->priv->size_w_spin), w_spin_cb, editor);
        g_signal_handlers_block_by_func (G_OBJECT (editor->priv->size_h_spin), h_spin_cb, editor);


        /* Get new configuration information */
        units = gl_prefs_model_get_units (gl_prefs);
        units_string = lgl_units_get_name (units);
        editor->priv->units_per_point = lgl_units_get_units_per_point (units);
        climb_rate = gl_units_util_get_step_size (units);
        digits = gl_units_util_get_precision (units);

	/* Update characteristics of w_spin/h_spin */
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON(editor->priv->size_w_spin), digits);
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON(editor->priv->size_h_spin), digits);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON(editor->priv->size_w_spin),
					climb_rate, 10.0*climb_rate);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON(editor->priv->size_h_spin),
					climb_rate, 10.0*climb_rate);

	/* Update units_labels */
	gtk_label_set_text (GTK_LABEL(editor->priv->size_w_units_label),
			    units_string);
	gtk_label_set_text (GTK_LABEL(editor->priv->size_h_units_label),
			    units_string);

	/* Update values of w_spin/h_spin */
	gl_object_editor_set_size (editor, editor->priv->w, editor->priv->h);
	gl_object_editor_set_max_size (editor, editor->priv->w_max, editor->priv->h_max);


        g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->size_w_spin), w_spin_cb, editor);
        g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->size_h_spin), h_spin_cb, editor);


	gl_debug (DEBUG_EDITOR, "END");
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
