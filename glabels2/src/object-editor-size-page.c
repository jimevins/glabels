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

#include <gnome.h>
#include <math.h>

#include "object-editor.h"
#include "prefs.h"

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
gl_object_editor_prepare_size_page (glObjectEditor       *editor,
				    glObjectEditorOption  option)
{
	const gchar  *units_string;
	gdouble       climb_rate;
	gint          digits;
	GtkSizeGroup *label_size_group;
	GtkWidget    *label;

	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
	editor->priv->size_page_vbox =
		glade_xml_get_widget (editor->priv->gui, "size_page_vbox");
	editor->priv->size_w_spin =
		glade_xml_get_widget (editor->priv->gui, "size_w_spin");
	editor->priv->size_h_spin =
		glade_xml_get_widget (editor->priv->gui, "size_h_spin");
	editor->priv->size_w_units_label =
		glade_xml_get_widget (editor->priv->gui, "size_w_units_label");
	editor->priv->size_h_units_label =
		glade_xml_get_widget (editor->priv->gui, "size_h_units_label");
	editor->priv->size_aspect_checkbutton =
		glade_xml_get_widget (editor->priv->gui, "size_aspect_checkbutton");
	editor->priv->size_reset_image_button =
		glade_xml_get_widget (editor->priv->gui, "size_reset_image_button");

	/* Get configuration information */
	units_string = gl_prefs_get_units_string ();
	editor->priv->units_per_point = gl_prefs_get_units_per_point ();
	climb_rate = gl_prefs_get_units_step_size ();
	digits = gl_prefs_get_units_precision ();

	/* Modify widgets based on configuration */
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON(editor->priv->size_w_spin), digits);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON(editor->priv->size_w_spin),
					climb_rate, 10.0*climb_rate);
	gtk_label_set_text (GTK_LABEL(editor->priv->size_w_units_label), units_string);
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON(editor->priv->size_h_spin), digits);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON(editor->priv->size_h_spin),
					climb_rate, 10.0*climb_rate);
	gtk_label_set_text (GTK_LABEL(editor->priv->size_h_units_label), units_string);

	/* Align label widths */
	label_size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	label = glade_xml_get_widget (editor->priv->gui, "size_w_label");
	gtk_size_group_add_widget (label_size_group, label);
	label = glade_xml_get_widget (editor->priv->gui, "size_h_label");
	gtk_size_group_add_widget (label_size_group, label);

	/* Un-hide */
	gtk_widget_show_all (editor->priv->size_page_vbox);
	if (option != GL_OBJECT_EDITOR_SIZE_IMAGE_PAGE) {
		gtk_widget_hide (editor->priv->size_reset_image_button);
	}

	/* Connect signals */
	g_signal_connect_swapped (G_OBJECT (editor->priv->size_aspect_checkbutton),
				  "toggled",
				  G_CALLBACK (aspect_toggle_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->size_w_spin),
				  "changed",
				  G_CALLBACK (w_spin_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->size_h_spin),
				  "changed",
				  G_CALLBACK (h_spin_cb),
				  G_OBJECT (editor));

	if (option == GL_OBJECT_EDITOR_SIZE_IMAGE_PAGE) {
		g_signal_connect_swapped (G_OBJECT (editor->priv->size_reset_image_button),
					  "clicked",
					  G_CALLBACK (size_reset_cb),
					  G_OBJECT (editor));
	}

	gl_debug (DEBUG_EDITOR, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Maintain aspect ratio checkbox callback.                       */
/*--------------------------------------------------------------------------*/
static void
aspect_toggle_cb (glObjectEditor *editor)
{
        GtkToggleButton *toggle;
	gdouble          w, h;

	toggle = GTK_TOGGLE_BUTTON (editor->priv->size_aspect_checkbutton);

        if (gtk_toggle_button_get_active (toggle)) {
                                                                                
                w = gtk_spin_button_get_value (GTK_SPIN_BUTTON(editor->priv->size_w_spin));
                h = gtk_spin_button_get_value (GTK_SPIN_BUTTON(editor->priv->size_h_spin));
                                                                                
                editor->priv->size_aspect_ratio = h / w;
                                                                                
        }
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  W spin button changed callback.                                */
/*--------------------------------------------------------------------------*/
static void
w_spin_cb (glObjectEditor *editor)
{
	gdouble w, h;
        GtkToggleButton *toggle;

	toggle = GTK_TOGGLE_BUTTON (editor->priv->size_aspect_checkbutton);

        if (gtk_toggle_button_get_active (toggle)) {

		w = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->size_w_spin));
                                                                                
                h = w * editor->priv->size_aspect_ratio;
                                                                                
                /* Update our sibling control, blocking recursion. */
                g_signal_handlers_block_by_func (G_OBJECT (editor->priv->size_h_spin),
						 G_CALLBACK (h_spin_cb),
						 editor);
                gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->size_h_spin), h);
                g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->size_h_spin),
						   G_CALLBACK (h_spin_cb),
						   editor);
        }
                                                                                
        /* Emit our "changed" signal */
        g_signal_emit (G_OBJECT (editor), gl_object_editor_signals[CHANGED], 0);
                                                                                
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  H spin button changed callback.                                */
/*--------------------------------------------------------------------------*/
static void
h_spin_cb (glObjectEditor *editor)
{
	gdouble w, h;

        GtkToggleButton *toggle = GTK_TOGGLE_BUTTON (editor->priv->size_aspect_checkbutton);
                                                                                
        if (gtk_toggle_button_get_active (toggle)) {

		h = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->size_w_spin));
                                                                                
                w = h / editor->priv->size_aspect_ratio;
                                                                                
                /* Update our sibling control, blocking recursion. */
                g_signal_handlers_block_by_func (G_OBJECT (editor->priv->size_w_spin),
						 G_CALLBACK (w_spin_cb),
						 editor);
                gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->size_w_spin), w);
                g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->size_w_spin),
						   G_CALLBACK (w_spin_cb),
						   editor);
        }
                                                                                
        /* Emit our "changed" signal */
        g_signal_emit (G_OBJECT (editor), gl_object_editor_signals[CHANGED], 0);
                                                                                
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Reset image size callback.                                     */
/*--------------------------------------------------------------------------*/
static void
size_reset_cb (glObjectEditor *editor)
{
	gdouble w_base, h_base;

	g_signal_handlers_block_by_func (G_OBJECT (editor->priv->size_w_spin),
					 G_CALLBACK (h_spin_cb),
					 editor);
	g_signal_handlers_block_by_func (G_OBJECT (editor->priv->size_h_spin),
					 G_CALLBACK (h_spin_cb),
					 editor);

	w_base = editor->priv->w_base * editor->priv->units_per_point;
	h_base = editor->priv->h_base * editor->priv->units_per_point;

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->size_w_spin),
				   w_base);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->size_h_spin),
				   h_base);

	g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->size_w_spin),
					   G_CALLBACK (h_spin_cb),
					   editor);
	g_signal_handlers_unblock_by_func (G_OBJECT (editor->priv->size_h_spin),
					   G_CALLBACK (h_spin_cb),
					   editor);
        /* Emit our "changed" signal */
        g_signal_emit (G_OBJECT (editor), gl_object_editor_signals[CHANGED], 0);
}

/*****************************************************************************/
/* Set size.                                                                 */
/*****************************************************************************/
void
gl_object_editor_set_size (glObjectEditor      *editor,
			     gdouble                w,
			     gdouble                h)
{
	gl_debug (DEBUG_EDITOR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->size_w_spin),
					 w_spin_cb,
					 editor);
	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->size_h_spin),
					 h_spin_cb,
					 editor);

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

	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->size_w_spin),
					   w_spin_cb,
					   editor);
	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->size_h_spin),
					   h_spin_cb,
					   editor);

	/* Update aspect ratio */
	editor->priv->size_aspect_ratio = h / w;

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

	gl_debug (DEBUG_EDITOR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->size_w_spin),
					 w_spin_cb,
					 editor);
	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->size_h_spin),
					 h_spin_cb,
					 editor);

	/* save a copy in internal units */
	editor->priv->w_max = w_max;
	editor->priv->h_max = h_max;

	/* convert internal units to displayed units */
	gl_debug (DEBUG_EDITOR, "internal w_max,h_max = %g, %g", w_max, h_max);
	w_max *= editor->priv->units_per_point;
	h_max *= editor->priv->units_per_point;
	gl_debug (DEBUG_EDITOR, "display w_max,h_max = %g, %g", w_max, h_max);

	/* Set widget values */
	tmp = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->size_w_spin));
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (editor->priv->size_w_spin),
				   0.0, 2.0*w_max);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->size_w_spin), tmp);
	tmp = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->size_h_spin));
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (editor->priv->size_h_spin),
				   0.0, 2.0*h_max);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->size_h_spin), tmp);

	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->size_w_spin),
					   w_spin_cb,
					   editor);
	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->size_h_spin),
					   h_spin_cb,
					   editor);

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
	const gchar  *units_string;
	gdouble       climb_rate;
	gint          digits;

	gl_debug (DEBUG_EDITOR, "START");

        /* Get new configuration information */
        units_string = gl_prefs_get_units_string ();
        editor->priv->units_per_point = gl_prefs_get_units_per_point ();
        climb_rate = gl_prefs_get_units_step_size ();
        digits = gl_prefs_get_units_precision ();

	/* Update characteristics of w_spin/h_spin */
	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->size_w_spin),
					 w_spin_cb,
					 editor);
	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->size_h_spin),
					 h_spin_cb,
					 editor);
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON(editor->priv->size_w_spin),
				    digits);
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON(editor->priv->size_h_spin),
				    digits);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON(editor->priv->size_w_spin),
					climb_rate, 10.0*climb_rate);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON(editor->priv->size_h_spin),
					climb_rate, 10.0*climb_rate);
	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->size_w_spin),
					   w_spin_cb,
					   editor);
	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->size_h_spin),
					   h_spin_cb,
					   editor);

	/* Update units_labels */
	gtk_label_set_text (GTK_LABEL(editor->priv->size_w_units_label),
			    units_string);
	gtk_label_set_text (GTK_LABEL(editor->priv->size_h_units_label),
			    units_string);

	/* Update values of w_spin/h_spin */
	gl_object_editor_set_size (editor,
				   editor->priv->w,
				   editor->priv->h);
	gl_object_editor_set_max_size (editor,
				       editor->priv->w_max,
				       editor->priv->h_max);

	gl_debug (DEBUG_EDITOR, "END");
}

