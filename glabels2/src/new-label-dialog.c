/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  new-label-dialog.c:  New label dialog module
 *
 *  Copyright (C) 2006  Jim Evins <evins@snaught.com>.
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

#include "new-label-dialog.h"

#include <glib/gi18n.h>
#include <glade/glade-xml.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkstock.h>

#include "hig.h"
#include "wdgt-media-select.h"
#include "wdgt-rotate-label.h"

#include "debug.h"

/*===========================================*/
/* Private data types                        */
/*===========================================*/

struct _glNewLabelDialogPrivate {

        GladeXML     *gui;

	GtkWidget *media_select;
	GtkWidget *rotate_label;

};


/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkDialogClass* parent_class = NULL;

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void       gl_new_label_dialog_class_init      (glNewLabelDialogClass *klass);
static void       gl_new_label_dialog_init            (glNewLabelDialog      *dlg);
static void       gl_new_label_dialog_finalize        (GObject               *object);

static void       gl_new_label_dialog_construct       (glNewLabelDialog      *dialog,
						       GtkWindow             *win);

static void       template_changed_cb                 (glWdgtMediaSelect     *select,
						       gpointer               data);



/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_new_label_dialog_get_type (void)
{
	static GType type = 0;

	if (!type)
    	{
      		static const GTypeInfo info =
      		{
			sizeof (glNewLabelDialogClass),
        		NULL,		/* base_init */
        		NULL,		/* base_finalize */
        		(GClassInitFunc) gl_new_label_dialog_class_init,
        		NULL,           /* class_finalize */
        		NULL,           /* class_data */
        		sizeof (glNewLabelDialog),
        		0,              /* n_preallocs */
        		(GInstanceInitFunc) gl_new_label_dialog_init,
			NULL
      		};

     		type = g_type_register_static (GTK_TYPE_DIALOG,
					       "glNewLabelDialog", &info, 0);
    	}

	return type;
}

static void
gl_new_label_dialog_class_init (glNewLabelDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	gl_debug (DEBUG_FILE, "");
	
  	parent_class = g_type_class_peek_parent (klass);

  	object_class->finalize = gl_new_label_dialog_finalize;  	
}

static void
gl_new_label_dialog_init (glNewLabelDialog *dialog)
{
	GtkWidget *pp_button;

	gl_debug (DEBUG_FILE, "START");

	g_return_if_fail (GL_IS_NEW_LABEL_DIALOG (dialog));

	dialog->priv = g_new0 (glNewLabelDialogPrivate, 1);

	gtk_container_set_border_width (GTK_CONTAINER(dialog), GL_HIG_PAD2);

	gtk_dialog_set_has_separator (GTK_DIALOG(dialog), FALSE);
	gtk_dialog_add_buttons (GTK_DIALOG(dialog),
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OK, GTK_RESPONSE_OK,
				NULL);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

        dialog->priv->gui = glade_xml_new (GLABELS_GLADE_DIR "new-label-dialog.glade",
                                           "new_label_dialog_vbox",
                                           NULL);

        if (!dialog->priv->gui) {
                g_critical ("Could not open new-label-dialog.glade. gLabels may not be installed correctly!");
                return;
        }

	gl_debug (DEBUG_FILE, "END");
}

static void 
gl_new_label_dialog_finalize (GObject *object)
{
	glNewLabelDialog* dialog;
	
	gl_debug (DEBUG_FILE, "START");

	g_return_if_fail (object != NULL);
	
   	dialog = GL_NEW_LABEL_DIALOG (object);

	g_return_if_fail (GL_IS_NEW_LABEL_DIALOG (dialog));
	g_return_if_fail (dialog->priv != NULL);

	g_free (dialog->priv);

	G_OBJECT_CLASS (parent_class)->finalize (object);

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

	gl_new_label_dialog_construct (GL_NEW_LABEL_DIALOG(dialog), win);

	return dialog;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct dialog.                                              */
/*--------------------------------------------------------------------------*/
static void
gl_new_label_dialog_construct (glNewLabelDialog   *dialog,
			       GtkWindow          *win)
{
	GtkWidget *vbox, *media_select_vbox, *rotate_label_vbox;
	gchar     *name;

	gl_debug (DEBUG_FILE, "START");

	gtk_window_set_transient_for (GTK_WINDOW (dialog), win);

        vbox = glade_xml_get_widget (dialog->priv->gui,
                                     "new_label_dialog_vbox");
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG (dialog)->vbox), vbox, FALSE, FALSE, 0);

        media_select_vbox =
                glade_xml_get_widget (dialog->priv->gui, "media_select_vbox");
        rotate_label_vbox =
                glade_xml_get_widget (dialog->priv->gui, "rotate_label_vbox");

	dialog->priv->media_select = gl_wdgt_media_select_new ();
	gtk_box_pack_start (GTK_BOX (media_select_vbox),
			    dialog->priv->media_select, FALSE, FALSE, 0);

	dialog->priv->rotate_label = gl_wdgt_rotate_label_new ();
	gtk_box_pack_start (GTK_BOX (rotate_label_vbox),
			    dialog->priv->rotate_label, FALSE, FALSE, 0);

	/* Sync template name from media select with rotate widget. */
	name = gl_wdgt_media_select_get_name (GL_WDGT_MEDIA_SELECT (dialog->priv->media_select));
	gl_wdgt_rotate_label_set_template_name (GL_WDGT_ROTATE_LABEL (dialog->priv->rotate_label),
						name);

	g_signal_connect (G_OBJECT (dialog->priv->media_select), "changed",
			  G_CALLBACK (template_changed_cb), dialog);

	gl_debug (DEBUG_FILE, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  New template changed callback.                                  */
/*---------------------------------------------------------------------------*/
static void
template_changed_cb (glWdgtMediaSelect *select,
		     gpointer           data)
{
	glNewLabelDialog  *dialog = GL_NEW_LABEL_DIALOG (data);
	gchar             *name;

	gl_debug (DEBUG_FILE, "START");

	name = gl_wdgt_media_select_get_name (GL_WDGT_MEDIA_SELECT (select));

	gl_wdgt_rotate_label_set_template_name (GL_WDGT_ROTATE_LABEL (dialog->priv->rotate_label),
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

	name = gl_wdgt_media_select_get_name (GL_WDGT_MEDIA_SELECT (dialog->priv->media_select));

	return name;
}

/*****************************************************************************/
/* Set template name.                                                        */
/*****************************************************************************/
void
gl_new_label_dialog_set_template_name (glNewLabelDialog *dialog,
				       gchar            *name)
{
	gl_wdgt_media_select_set_name (GL_WDGT_MEDIA_SELECT (dialog->priv->media_select), name);
}

/*****************************************************************************/
/* Get current filter parameters.                                            */
/*****************************************************************************/
void
gl_new_label_dialog_get_filter_parameters (glNewLabelDialog *dialog,
					   gchar           **page_size_id,
					   gchar           **category_id)
{
	gl_wdgt_media_select_get_filter_parameters (
		GL_WDGT_MEDIA_SELECT (dialog->priv->media_select),
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
	gl_wdgt_media_select_set_filter_parameters (
		GL_WDGT_MEDIA_SELECT (dialog->priv->media_select),
		page_size_id, category_id);
}

/*****************************************************************************/
/* Get rotate state.                                                         */
/*****************************************************************************/
gboolean
gl_new_label_dialog_get_rotate_state (glNewLabelDialog *dialog)
{
	return gl_wdgt_rotate_label_get_state (
		GL_WDGT_ROTATE_LABEL (dialog->priv->rotate_label));
}

/*****************************************************************************/
/* Set rotate state.                                                         */
/*****************************************************************************/
void
gl_new_label_dialog_set_rotate_state (glNewLabelDialog *dialog,
				      gboolean          state)
{
	gl_wdgt_rotate_label_set_state (
		GL_WDGT_ROTATE_LABEL (dialog->priv->rotate_label), state);
}

