/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  object-editor.c:  object properties editor module
 *
 *  Copyright (C) 2003  Jim Evins <evins@snaught.com>.
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

#include "object-editor.h"

#include <glib/gi18n.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkspinbutton.h>
#include <math.h>

#include "prefs.h"

#include "object-editor-private.h"

#include "debug.h"

/*===========================================*/
/* Private macros                            */
/*===========================================*/

#define LENGTH(x,y) sqrt( (x)*(x) + (y)*(y) )
#define ANGLE(x,y)  ( (180.0/G_PI)*atan2( -(y), (x) ) )
#define COMP_X(l,a) ( (l) * cos( (G_PI/180.0)*(a) ) )
#define COMP_Y(l,a) ( -(l) * sin( (G_PI/180.0)*(a) ) )
                                                                                

/*===========================================*/
/* Private data types                        */
/*===========================================*/

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/



/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare line size page.                                        */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_prepare_lsize_page (glObjectEditor       *editor)
{
	const gchar  *units_string;
	gdouble       climb_rate;
	gint          digits;

	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
        gl_util_get_builder_widgets (editor->priv->gui,
                                     "lsize_page_vbox",     &editor->priv->lsize_page_vbox,
                                     "lsize_r_spin",        &editor->priv->lsize_r_spin,
                                     "lsize_theta_spin",    &editor->priv->lsize_theta_spin,
                                     "lsize_r_units_label", &editor->priv->lsize_r_units_label,
                                     NULL);

	/* Get configuration information */
	units_string = gl_prefs_get_units_string ();
	editor->priv->units_per_point = gl_prefs_get_units_per_point ();
	climb_rate = gl_prefs_get_units_step_size ();
	digits = gl_prefs_get_units_precision ();

	/* Modify widgets based on configuration */
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON(editor->priv->lsize_r_spin), digits);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON(editor->priv->lsize_r_spin),
					climb_rate, 10.0*climb_rate);
	gtk_label_set_text (GTK_LABEL(editor->priv->lsize_r_units_label), units_string);

	/* Un-hide */
	gtk_widget_show_all (editor->priv->lsize_page_vbox);

	/* Connect signals */
	g_signal_connect_swapped (G_OBJECT (editor->priv->lsize_r_spin),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->lsize_theta_spin),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Set line size.                                                            */
/*****************************************************************************/
void
gl_object_editor_set_lsize (glObjectEditor      *editor,
			    gdouble              dx,
			    gdouble              dy)
{
	gdouble r, theta;

	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->stop_signals = TRUE;

	/* save a copy in internal units */
	editor->priv->dx = dx;
	editor->priv->dy = dy;

	/* convert internal units to displayed units */
	gl_debug (DEBUG_EDITOR, "internal dx,dy = %g, %g", dx, dy);
	dx *= editor->priv->units_per_point;
	dy *= editor->priv->units_per_point;
	gl_debug (DEBUG_EDITOR, "display dx,dy = %g, %g", dx, dy);

	r     = LENGTH (dx, dy);
	theta = ANGLE (dx, dy);

	/* Set widget values */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->lsize_r_spin), r);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->lsize_theta_spin),
				   theta);

        editor->priv->stop_signals = FALSE;

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Set maximum line size.                                                    */
/*****************************************************************************/
void
gl_object_editor_set_max_lsize (glObjectEditor      *editor,
				gdouble              dx_max,
				gdouble              dy_max)
{
	gdouble tmp;

	gl_debug (DEBUG_EDITOR, "START");

        if (editor->priv->lsize_page_vbox)
        {

                editor->priv->stop_signals = TRUE;

                /* save a copy in internal units */
                editor->priv->dx_max = dx_max;
                editor->priv->dy_max = dy_max;

                /* convert internal units to displayed units */
                gl_debug (DEBUG_EDITOR, "internal dx_max,dy_max = %g, %g", dx_max, dy_max);
                dx_max *= editor->priv->units_per_point;
                dy_max *= editor->priv->units_per_point;
                gl_debug (DEBUG_EDITOR, "display dx_max,dy_max = %g, %g", dx_max, dy_max);

                /* Set widget values */
                tmp = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->lsize_r_spin));
                gtk_spin_button_set_range (GTK_SPIN_BUTTON (editor->priv->lsize_r_spin),
                                           0.0, 2.0*LENGTH (dx_max, dy_max));
                gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->lsize_r_spin), tmp);

                editor->priv->stop_signals = FALSE;

        }

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Query line size.                                                          */
/*****************************************************************************/
void
gl_object_editor_get_lsize (glObjectEditor      *editor,
			    gdouble             *dx,
			    gdouble             *dy)
{
	gdouble r, theta;

	gl_debug (DEBUG_EDITOR, "START");

	/* Get values from widgets */
	r = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->lsize_r_spin));
	theta = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->lsize_theta_spin));

	/* convert everything back to our internal units (points) */
	r /= editor->priv->units_per_point;

	*dx = COMP_X (r, theta);
	*dy = COMP_Y (r, theta);

	/* save a copy in internal units */
	editor->priv->dx = *dx;
	editor->priv->dy = *dy;

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* PRIVATE. Prefs changed callback.  Update units related items.            */
/*****************************************************************************/
void
lsize_prefs_changed_cb (glObjectEditor *editor)
{
	const gchar  *units_string;
	gdouble       climb_rate;
	gint          digits;

	gl_debug (DEBUG_EDITOR, "START");

        /* Get new configuration information */
        units_string = gl_prefs_get_units_string ();
        editor->priv->units_per_point = gl_prefs_get_units_per_point ();
        climb_rate = gl_prefs_get_units_step_size ();
        digits = gl_prefs_get_units_precision ();

	/* Update characteristics of r_spin */
        editor->priv->stop_signals = TRUE;
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON(editor->priv->lsize_r_spin),
				    digits);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON(editor->priv->lsize_r_spin),
					climb_rate, 10.0*climb_rate);
        editor->priv->stop_signals = FALSE;

	/* Update r_units_label */
	gtk_label_set_text (GTK_LABEL(editor->priv->lsize_r_units_label),
			    units_string);

	/* Update values of r_spin/theta_spin */
	gl_object_editor_set_lsize (editor,
				    editor->priv->dx,
				    editor->priv->dy);
	gl_object_editor_set_max_lsize (editor,
					editor->priv->dx_max,
					editor->priv->dy_max);

	gl_debug (DEBUG_EDITOR, "END");
}

