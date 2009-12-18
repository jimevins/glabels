/*
 *  new-label-dialog.c
 *  Copyright (C) 2006-2009  Jim Evins <evins@snaught.com>.
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

#include "new-label-dialog.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "hig.h"
#include "media-select.h"
#include "rotate-label-button.h"

#include "debug.h"


/*===========================================*/
/* Private data types                        */
/*===========================================*/

struct _glNewLabelDialogPrivate {

	GtkWidget  *media_select;
	GtkWidget  *rotate_label;

};


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void       gl_new_label_dialog_finalize        (GObject           *object);

static void       template_changed_cb                 (glMediaSelect     *select,
						       gpointer           data);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glNewLabelDialog, gl_new_label_dialog, GTK_TYPE_DIALOG);


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_new_label_dialog_class_init (glNewLabelDialogClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	gl_debug (DEBUG_FILE, "");
	
  	gl_new_label_dialog_parent_class = g_type_class_peek_parent (class);

  	object_class->finalize = gl_new_label_dialog_finalize;  	
}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_new_label_dialog_init (glNewLabelDialog *dialog)
{
	GtkWidget    *vbox;
        GtkWidget    *label;
	GtkWidget    *frame;
	gchar        *name;

	gl_debug (DEBUG_FILE, "START");

	g_return_if_fail (GL_IS_NEW_LABEL_DIALOG (dialog));

	dialog->priv = g_new0 (glNewLabelDialogPrivate, 1);

	gtk_container_set_border_width (GTK_CONTAINER (dialog), GL_HIG_PAD1);

	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
	gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                GTK_STOCK_OK, GTK_RESPONSE_OK,
                                NULL);
        gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

        vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

        label = gtk_label_new (_("<b>Media type</b>"));
        gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
        frame = gtk_frame_new ("");
        gtk_frame_set_label_widget (GTK_FRAME (frame), label);
        gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, GL_HIG_PAD1);

	dialog->priv->media_select = gl_media_select_new ();
        gtk_container_add (GTK_CONTAINER (frame), dialog->priv->media_select);

        label = gtk_label_new (_("<b>Label orientation</b>"));
        gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
        frame = gtk_frame_new ("");
        gtk_frame_set_label_widget (GTK_FRAME (frame), label);
        gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

	dialog->priv->rotate_label = gl_rotate_label_button_new ();
        gtk_container_add (GTK_CONTAINER (frame), dialog->priv->rotate_label);

	/* Sync template name from media select with rotate widget. */
	name = gl_media_select_get_name (GL_MEDIA_SELECT (dialog->priv->media_select));
	gl_rotate_label_button_set_template_name (GL_ROTATE_LABEL_BUTTON (dialog->priv->rotate_label),
                                                  name);

	g_signal_connect (G_OBJECT (dialog->priv->media_select), "changed",
			  G_CALLBACK (template_changed_cb), dialog);

	gl_debug (DEBUG_FILE, "END");
}


/*****************************************************************************/
/* Finalize Function.                                                        */
/*****************************************************************************/
static void 
gl_new_label_dialog_finalize (GObject *object)
{
	glNewLabelDialog* dialog = GL_NEW_LABEL_DIALOG (object);;
	
	gl_debug (DEBUG_FILE, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_NEW_LABEL_DIALOG (dialog));
	g_return_if_fail (dialog->priv != NULL);

	g_free (dialog->priv);

	G_OBJECT_CLASS (gl_new_label_dialog_parent_class)->finalize (object);

	gl_debug (DEBUG_FILE, "END");

}


/*****************************************************************************/
/* NEW object properties dialog.                                             */
/*****************************************************************************/
GtkWidget *
gl_new_label_dialog_new (GtkWindow    *win)
{
	GtkWidget *dialog;

	gl_debug (DEBUG_FILE, "");

	dialog = GTK_WIDGET (g_object_new (GL_TYPE_NEW_LABEL_DIALOG, NULL));

	gtk_window_set_transient_for (GTK_WINDOW (dialog), win);

	return dialog;
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  New template changed callback.                                  */
/*---------------------------------------------------------------------------*/
static void
template_changed_cb (glMediaSelect *select,
		     gpointer       data)
{
	glNewLabelDialog  *dialog = GL_NEW_LABEL_DIALOG (data);
	gchar             *name;

	gl_debug (DEBUG_FILE, "START");

	name = gl_media_select_get_name (GL_MEDIA_SELECT (select));

	gl_rotate_label_button_set_template_name (GL_ROTATE_LABEL_BUTTON (dialog->priv->rotate_label),
                                                  name);

	gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog),
					   GTK_RESPONSE_OK,
					   (name != NULL));

	g_free (name);

	gl_debug (DEBUG_FILE, "END");
}


/*****************************************************************************/
/* Get template name.                                                        */
/*****************************************************************************/
gchar *
gl_new_label_dialog_get_template_name (glNewLabelDialog *dialog)
{
	gchar *name;

	name = gl_media_select_get_name (GL_MEDIA_SELECT (dialog->priv->media_select));

	return name;
}


/*****************************************************************************/
/* Set template name.                                                        */
/*****************************************************************************/
void
gl_new_label_dialog_set_template_name (glNewLabelDialog *dialog,
				       gchar            *name)
{
	gl_media_select_set_name (GL_MEDIA_SELECT (dialog->priv->media_select), name);
}


/*****************************************************************************/
/* Get current filter parameters.                                            */
/*****************************************************************************/
void
gl_new_label_dialog_get_filter_parameters (glNewLabelDialog  *dialog,
					   gchar            **page_size_id,
					   gchar            **category_id)
{
	gl_media_select_get_filter_parameters (
		GL_MEDIA_SELECT (dialog->priv->media_select),
		page_size_id, category_id);
}


/*****************************************************************************/
/* Set current filter parameters.                                            */
/*****************************************************************************/
void
gl_new_label_dialog_set_filter_parameters (glNewLabelDialog *dialog,
					   const gchar      *page_size_id,
					   const gchar      *category_id)
{
	gl_media_select_set_filter_parameters (
		GL_MEDIA_SELECT (dialog->priv->media_select),
		page_size_id, category_id);
}


/*****************************************************************************/
/* Get rotate state.                                                         */
/*****************************************************************************/
gboolean
gl_new_label_dialog_get_rotate_state (glNewLabelDialog *dialog)
{
	return gl_rotate_label_button_get_state (
		GL_ROTATE_LABEL_BUTTON (dialog->priv->rotate_label));
}


/*****************************************************************************/
/* Set rotate state.                                                         */
/*****************************************************************************/
void
gl_new_label_dialog_set_rotate_state (glNewLabelDialog *dialog,
				      gboolean          state)
{
	gl_rotate_label_button_set_state (
		GL_ROTATE_LABEL_BUTTON (dialog->priv->rotate_label), state);
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
