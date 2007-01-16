/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  print-dialog.c:  Print dialog module
 *
 *  Copyright (C) 2001-2007  Jim Evins <evins@snaught.com>.
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

#include "print-dialog.h"

#include <glib/gi18n.h>
#include <glade/glade-xml.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <gtk/gtktogglebutton.h>

#include "print.h"
#include "label.h"

#include "wdgt-print-copies.h"
#include "wdgt-print-merge.h"

#include "debug.h"

/*===========================================*/
/* Private data types                        */
/*===========================================*/

struct _glPrintDialogPrivate {

	glLabel   *label;

	GladeXML  *gui;

	GtkWidget *simple_frame;
	GtkWidget *copies_vbox;
	GtkWidget *copies;

	GtkWidget *merge_frame;
	GtkWidget *prmerge_vbox;
	GtkWidget *prmerge;

	GtkWidget *outline_check;
	GtkWidget *reverse_check;
	GtkWidget *crop_marks_check;

        gboolean   force_outline_flag;

	gboolean   outline_flag;
        gboolean   reverse_flag;
        gboolean   crop_marks_flag;
        gboolean   merge_flag;
        gboolean   collate_flag;

	gint       first;
        gint       last;
        gint       n_sheets;
        gint       n_copies;

        glPrintState state;
};


/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkPrintOperationClass* parent_class = NULL;

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void     gl_print_dialog_class_init    (glPrintDialogClass *klass);
static void     gl_print_dialog_init          (glPrintDialog      *dialog);
static void     gl_print_dialog_finalize      (GObject            *object);

static void     gl_print_dialog_construct     (glPrintDialog      *dialog,
                                               glLabel            *label);


static GObject *create_custom_widget_cb       (GtkPrintOperation *operation,
                                               gpointer           user_data);

static void     custom_widget_apply_cb        (GtkPrintOperation *operation,
                                               GtkWidget         *widget,
                                               gpointer           user_data);

static void     begin_print_cb                (GtkPrintOperation *operation,
                                               GtkPrintContext   *context,
                                               gpointer           user_data);

static void     draw_page_cb                  (GtkPrintOperation *operation,
                                               GtkPrintContext   *context,
                                               int                page_nr,
                                               gpointer           user_data);




/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_print_dialog_get_type (void)
{
	static GType type = 0;

	if (!type)
    	{
      		static const GTypeInfo info =
      		{
			sizeof (glPrintDialogClass),
        		NULL,		/* base_init */
        		NULL,		/* base_finalize */
        		(GClassInitFunc) gl_print_dialog_class_init,
        		NULL,           /* class_finalize */
        		NULL,           /* class_data */
        		sizeof (glPrintDialog),
        		0,              /* n_preallocs */
        		(GInstanceInitFunc) gl_print_dialog_init,
			NULL
      		};

     		type = g_type_register_static (GTK_TYPE_PRINT_OPERATION,
					       "glPrintDialog", &info, 0);
    	}

	return type;
}

static void
gl_print_dialog_class_init (glPrintDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkPrintOperationClass *print_class = GTK_PRINT_OPERATION_CLASS (klass);

	gl_debug (DEBUG_PRINT, "");
	
  	parent_class = g_type_class_peek_parent (klass);

  	object_class->finalize = gl_print_dialog_finalize;
}

static void
gl_print_dialog_init (glPrintDialog *dialog)
{
	GtkWidget *pp_button;

	gl_debug (DEBUG_PRINT, "");

	gtk_print_operation_set_use_full_page (GTK_PRINT_OPERATION (dialog),
					       TRUE);

        gtk_print_operation_set_unit (GTK_PRINT_OPERATION (dialog),
                                      GTK_UNIT_POINTS);

	dialog->priv = g_new0 (glPrintDialogPrivate, 1);

}

static void 
gl_print_dialog_finalize (GObject *object)
{
	glPrintDialog* dialog;
	
	gl_debug (DEBUG_PRINT, "");

	g_return_if_fail (object != NULL);
	
   	dialog = GL_PRINT_DIALOG (object);

	g_return_if_fail (GL_IS_PRINT_DIALOG (dialog));
	g_return_if_fail (dialog->priv != NULL);

	if (dialog->priv->label) {
		g_object_unref (G_OBJECT(dialog->priv->label));
	}
	g_free (dialog->priv);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	g_free (dialog->priv);
}

/*****************************************************************************/
/* NEW print dialog.                                                         */
/*****************************************************************************/
glPrintDialog *
gl_print_dialog_new (glLabel      *label)
{
	glPrintDialog *dialog;

	gl_debug (DEBUG_PRINT, "");

	dialog = GL_PRINT_DIALOG (g_object_new (GL_TYPE_PRINT_DIALOG, NULL));

	gl_print_dialog_construct (GL_PRINT_DIALOG(dialog), label);

	return dialog;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct dialog.                                              */
/*--------------------------------------------------------------------------*/
static void
gl_print_dialog_construct (glPrintDialog      *dialog,
			   glLabel            *label)
{
	dialog->priv->label              = label;
	dialog->priv->force_outline_flag = FALSE;

	gtk_print_operation_set_custom_tab_label ( GTK_PRINT_OPERATION (dialog),
						   _("Labels"));

	g_signal_connect (G_OBJECT (dialog), "create-custom-widget",
			  G_CALLBACK (create_custom_widget_cb), label);

	g_signal_connect (G_OBJECT (dialog), "custom-widget-apply",
			  G_CALLBACK (custom_widget_apply_cb), label);

	g_signal_connect (G_OBJECT (dialog), "begin-print",
			  G_CALLBACK (begin_print_cb), label);

	g_signal_connect (G_OBJECT (dialog), "draw-page",
			  G_CALLBACK (draw_page_cb), label);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  "Create custom widget" callback                                */
/*--------------------------------------------------------------------------*/
static GObject *
create_custom_widget_cb (GtkPrintOperation *operation,
			 gpointer           user_data)
{
	glPrintDialog *dialog = GL_PRINT_DIALOG (operation);
	glLabel       *label  = GL_LABEL (user_data);

	GtkWidget     *vbox;

        glMerge       *merge = NULL;

	dialog->priv->gui = glade_xml_new (GLABELS_GLADE_DIR "print-custom-widget.glade",
					   "print_custom_widget_vbox",
					   NULL);

	if (!dialog->priv->gui) {
		g_warning ("Could not open print-dialog.glade, reinstall glabels!");
		return;
	}

	vbox = glade_xml_get_widget (dialog->priv->gui, "print_custom_widget_vbox");

	/* ----- Simple print control ----- */
	dialog->priv->simple_frame = glade_xml_get_widget (dialog->priv->gui,
							   "simple_frame");
	dialog->priv->copies_vbox  = glade_xml_get_widget (dialog->priv->gui,
							   "copies_vbox");
	dialog->priv->copies = gl_wdgt_print_copies_new (label);
	gtk_box_pack_start (GTK_BOX(dialog->priv->copies_vbox),
			    dialog->priv->copies, FALSE, FALSE, 0);

	/* ----- Merge print control ----- */
	dialog->priv->merge_frame  = glade_xml_get_widget (dialog->priv->gui,
							   "merge_frame");
	dialog->priv->prmerge_vbox = glade_xml_get_widget (dialog->priv->gui,
							   "prmerge_vbox");
	dialog->priv->prmerge = gl_wdgt_print_merge_new (label);
	gtk_box_pack_start (GTK_BOX(dialog->priv->prmerge_vbox),
			    dialog->priv->prmerge, FALSE, FALSE, 0);

	/* ----- Options ----------------- */
	dialog->priv->outline_check    = glade_xml_get_widget (dialog->priv->gui,
							       "outline_check");
	dialog->priv->reverse_check    = glade_xml_get_widget (dialog->priv->gui,
							       "reverse_check");
	dialog->priv->crop_marks_check = glade_xml_get_widget (dialog->priv->gui,
							       "crop_marks_check");

        /* ---- Activate either simple or merge print control widgets. ---- */
        merge = gl_label_get_merge (dialog->priv->label);
	if (merge == NULL) {

		gtk_widget_show_all (dialog->priv->simple_frame);
		gtk_widget_hide_all (dialog->priv->merge_frame);

	} else {

		gint n_records = gl_merge_get_record_count( merge );
		gl_wdgt_print_merge_set_copies (GL_WDGT_PRINT_MERGE(dialog->priv->prmerge),
						1, 1, n_records, FALSE);
		g_object_unref (G_OBJECT(merge));

		gtk_widget_hide_all (dialog->priv->simple_frame);
		gtk_widget_show_all (dialog->priv->merge_frame);
	}

        /* --- Do we need to force the outline flag --- */
        if (dialog->priv->force_outline_flag)
        {
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(dialog->priv->outline_check),
                                              TRUE);

                gtk_widget_set_sensitive (dialog->priv->outline_check, FALSE);
                gtk_widget_set_sensitive (dialog->priv->reverse_check, FALSE);
                gtk_widget_set_sensitive (dialog->priv->crop_marks_check, FALSE);
        }

	return G_OBJECT (vbox);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  "Custom widget apply" callback                                 */
/*--------------------------------------------------------------------------*/
static void
custom_widget_apply_cb (GtkPrintOperation *operation,
                        GtkWidget         *widget,
                        gpointer           user_data)
{
        glPrintDialog *dialog = GL_PRINT_DIALOG (operation);
        glMerge       *merge = NULL;


        dialog->priv->outline_flag =
                gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
                                              (dialog->priv->outline_check));
        dialog->priv->reverse_flag =
                gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
                                              (dialog->priv->reverse_check));
        dialog->priv->crop_marks_flag =
                gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
                                              (dialog->priv->crop_marks_check));

        merge = gl_label_get_merge (dialog->priv->label);

        if (merge == NULL)
        {

                dialog->priv->merge_flag = FALSE;
                gl_wdgt_print_copies_get_range (GL_WDGT_PRINT_COPIES (dialog->priv->copies),
                                                &dialog->priv->n_sheets,
                                                &dialog->priv->first,
                                                &dialog->priv->last);
        }
        else
        {

                dialog->priv->merge_flag = TRUE;
                gl_wdgt_print_merge_get_copies (GL_WDGT_PRINT_MERGE (dialog->priv->prmerge),
                                                &dialog->priv->n_copies,
                                                &dialog->priv->first,
                                                &dialog->priv->collate_flag,
                                                &dialog->priv->n_sheets);
                g_object_unref (G_OBJECT(merge));
        }

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  "Begin print" callback                                         */
/*--------------------------------------------------------------------------*/
static void
begin_print_cb (GtkPrintOperation *operation,
		GtkPrintContext   *context,
		gpointer           user_data)
{
        glPrintDialog *dialog = GL_PRINT_DIALOG (operation);

        gtk_print_operation_set_n_pages (operation, dialog->priv->n_sheets);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  "Draw page" callback.                                          */
/*--------------------------------------------------------------------------*/
static void
draw_page_cb (GtkPrintOperation *operation,
	      GtkPrintContext   *context,
	      int                page_nr,
	      gpointer           user_data)
{
        glPrintDialog *dialog = GL_PRINT_DIALOG (operation);
        cairo_t       *cr;

        cr = gtk_print_context_get_cairo_context (context);

        if (!dialog->priv->merge_flag)
        {
                gl_print_simple_sheet (dialog->priv->label,
                                       cr,
                                       page_nr,
                                       dialog->priv->n_sheets,
                                       dialog->priv->first,
                                       dialog->priv->last,
                                       dialog->priv->outline_flag,
                                       dialog->priv->reverse_flag,
                                       dialog->priv->crop_marks_flag);
        }
        else
        {
                if (dialog->priv->collate_flag)
                {
                        gl_print_collated_merge_sheet (dialog->priv->label,
                                                       cr,
                                                       page_nr,
                                                       dialog->priv->n_copies,
                                                       dialog->priv->first,
                                                       dialog->priv->outline_flag,
                                                       dialog->priv->reverse_flag,
                                                       dialog->priv->crop_marks_flag,
                                                       &dialog->priv->state);
                }
                else
                {
                        gl_print_uncollated_merge_sheet (dialog->priv->label,
                                                         cr,
                                                         page_nr,
                                                         dialog->priv->n_copies,
                                                         dialog->priv->first,
                                                         dialog->priv->outline_flag,
                                                         dialog->priv->reverse_flag,
                                                         dialog->priv->crop_marks_flag,
                                                         &dialog->priv->state);
                }
        }
}


/*****************************************************************************/
/* Set outline flag/checkbox.                                                */
/*****************************************************************************/
void
gl_print_dialog_force_outline_flag (glPrintDialog *dialog)
{
        dialog->priv->force_outline_flag = TRUE;
}


