/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  print-dialog.c:  Print dialog module
 *
 *  Copyright (C) 2001-2003  Jim Evins <evins@snaught.com>.
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
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtknotebook.h>
#include <libgnomeprintui/gnome-print-dialog.h>
#include <libgnomeprint/gnome-print-job.h>
#include <libgnomeprintui/gnome-print-job-preview.h>

#include "print.h"
#include "label.h"

#include "wdgt-print-copies.h"
#include "wdgt-print-merge.h"

#include "debug.h"

/***************************************************************************/
/* FIXME: GnomePrinterSelector is not public in libgnomeprintui-2.2.       */
/*                                                                         */
/* I know that I'm asking for trouble, but here are standin prototypes:    */
GtkWidget          *gnome_printer_selector_new (GnomePrintConfig *config);
GnomePrintConfig   *gnome_printer_selector_get_config (GtkWidget *psel);
#define GNOME_PRINTER_SELECTOR(x) (x)
/***************************************************************************/

/*===========================================*/
/* Private data types                        */
/*===========================================*/

struct _glPrintDialogPrivate {

	glLabel   *label;

	GtkWidget *simple_frame;
	GtkWidget *copies;

	GtkWidget *merge_frame;
	GtkWidget *prmerge;

	GtkWidget *outline_check;
	GtkWidget *reverse_check;
	GtkWidget *crop_marks_check;

	GtkWidget *printer_select;
};


/*===========================================*/
/* Private globals                           */
/*===========================================*/

static glHigDialogClass* parent_class = NULL;

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void       gl_print_dialog_class_init      (glPrintDialogClass *klass);
static void       gl_print_dialog_init            (glPrintDialog      *dlg);
static void       gl_print_dialog_finalize        (GObject            *object);

static void       gl_print_dialog_construct       (glPrintDialog      *dialog,
						   glLabel            *label,
						   GtkWindow          *win);

static GtkWidget *job_page_new                    (glPrintDialog      *dialog,
						   glLabel            *label);

static GtkWidget *printer_page_new                (glPrintDialog      *dialog,
						   glLabel            *label);

static void       merge_changed_cb                (glLabel            *label,
						   glPrintDialog      *dialog);

static void       size_changed_cb                 (glLabel            *label,
						   glPrintDialog      *dialog);

static void       delete_event_cb                 (glPrintDialog      *dialog,
						   gpointer            user_data);

static void       print_response_cb               (glPrintDialog      *dialog,
						   gint                response,
						   glLabel            *label);

static void       print_sheets                    (GnomePrintConfig   *config,
						   glLabel            *label,
						   gboolean            preview_flag,
						   gint                n_sheets,
						   gint                first,
						   gint                last,
						   gboolean            outline_flag,
						   gboolean            reverse_flag,
						   gboolean            crop_marks_flag);

static void      print_sheets_merge               (GnomePrintConfig   *config,
						   glLabel            *label,
						   gboolean            preview_flag,
						   gint                n_copies,
						   gint                first,
						   gboolean            collate_flag,
						   gboolean            outline_flag,
						   gboolean            reverse_flag,
						   gboolean            crop_marks_flag);




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

     		type = g_type_register_static (GL_TYPE_HIG_DIALOG,
					       "glPrintDialog", &info, 0);
    	}

	return type;
}

static void
gl_print_dialog_class_init (glPrintDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	gl_debug (DEBUG_PRINT, "");
	
  	parent_class = g_type_class_peek_parent (klass);

  	object_class->finalize = gl_print_dialog_finalize;  	
}

static void
gl_print_dialog_init (glPrintDialog *dialog)
{
	GtkWidget *pp_button;

	gl_debug (DEBUG_PRINT, "");

	dialog->priv = g_new0 (glPrintDialogPrivate, 1);

	gtk_dialog_add_buttons (GTK_DIALOG(dialog),
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_PRINT,  GNOME_PRINT_DIALOG_RESPONSE_PRINT,
				NULL);

	pp_button =
		gtk_dialog_add_button (GTK_DIALOG (dialog),
				       GTK_STOCK_PRINT_PREVIEW, GNOME_PRINT_DIALOG_RESPONSE_PREVIEW);
	gtk_button_box_set_child_secondary (GTK_BUTTON_BOX (GTK_DIALOG (dialog)->action_area), 
					    pp_button, TRUE);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog),
					 GNOME_PRINT_DIALOG_RESPONSE_PRINT);

        gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

	g_signal_connect (G_OBJECT(dialog),
			  "delete_event",
			  G_CALLBACK (delete_event_cb),
			  NULL);
                                                                                
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
/* NEW object properties dialog.                                              */
/*****************************************************************************/
GtkWidget *
gl_print_dialog_new (glLabel      *label,
		     GtkWindow    *win)
{
	GtkWidget *dialog;

	gl_debug (DEBUG_PRINT, "");

	dialog = GTK_WIDGET (g_object_new (GL_TYPE_PRINT_DIALOG, NULL));

	gl_print_dialog_construct (GL_PRINT_DIALOG(dialog), label, win);

	return dialog;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct dialog.                                              */
/*--------------------------------------------------------------------------*/
static void
gl_print_dialog_construct (glPrintDialog      *dialog,
			   glLabel            *label,
			   GtkWindow          *win)
{
	GtkWidget *notebook, *page;
	gchar     *name, *title;

	gl_debug (DEBUG_PRINT, "START");

	g_return_if_fail (label && GL_IS_LABEL(label));

	dialog->priv->label = GL_LABEL(g_object_ref (G_OBJECT(label)));

	name = gl_label_get_short_name (label);
	title = g_strdup_printf ("%s \"%s\"", _("Print"), name);
	g_free (name);

	gtk_window_set_title (GTK_WINDOW(dialog), title);
	if (win) {
		gtk_window_set_transient_for (GTK_WINDOW(dialog), GTK_WINDOW(win));
		gtk_window_set_destroy_with_parent (GTK_WINDOW(dialog), TRUE);
	}

	notebook = gtk_notebook_new ();
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dialog), notebook);

	/* ----- Create Job notebook page ----- */
	page = job_page_new (dialog, label);
	gtk_notebook_append_page (GTK_NOTEBOOK(notebook), page,
				  gtk_label_new_with_mnemonic (_("_Job")));

	/* ----- Create Printer notebook page ----- */
	page = printer_page_new (dialog, label);
	gtk_notebook_append_page (GTK_NOTEBOOK(notebook), page,
				  gtk_label_new_with_mnemonic (_("P_rinter")));

	g_signal_connect (G_OBJECT(dialog), "response",
			  G_CALLBACK (print_response_cb), label);

	gtk_widget_show_all (GTK_WIDGET (dialog));

	merge_changed_cb (GL_LABEL(label), dialog);

	g_free (title);


	gl_debug (DEBUG_PRINT, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create "Job" page.                                              */
/*---------------------------------------------------------------------------*/
static GtkWidget *
job_page_new (glPrintDialog *dialog,
	      glLabel       *label)
{
	GtkWidget *vbox;
	GtkWidget *wframe;

	vbox = gl_hig_vbox_new (GL_HIG_VBOX_OUTER);

	/* ----------- Add simple-copies widget ------------ */
	dialog->priv->simple_frame = gl_hig_category_new (_("Copies"));
	gl_hig_vbox_add_widget (GL_HIG_VBOX(vbox), dialog->priv->simple_frame);

	dialog->priv->copies = gl_wdgt_print_copies_new (label);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(dialog->priv->simple_frame),
				    dialog->priv->copies);

	/* ------- Add merge control widget ------------ */
	dialog->priv->merge_frame = gl_hig_category_new (_("Document merge control"));
	gl_hig_vbox_add_widget (GL_HIG_VBOX(vbox), dialog->priv->merge_frame);

	dialog->priv->prmerge = gl_wdgt_print_merge_new (label);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(dialog->priv->merge_frame),
				    dialog->priv->prmerge);

	/* ----------- Add custom print options area ------------ */
	wframe = gl_hig_category_new (_("Options"));
	gl_hig_vbox_add_widget (GL_HIG_VBOX(vbox), wframe);

	/* add Outline check button */
	dialog->priv->outline_check =
	    gtk_check_button_new_with_label (
		    _("print outlines (to test printer alignment)"));
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wframe), dialog->priv->outline_check);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->outline_check),
				      FALSE);

	/* add Reverse check button */
	dialog->priv->reverse_check =
	    gtk_check_button_new_with_label (
		    _("print in reverse (i.e. a mirror image)"));
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wframe), dialog->priv->reverse_check);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->reverse_check),
				      FALSE);

	/* add Crop marks check button */
	dialog->priv->crop_marks_check =
	    gtk_check_button_new_with_label (_("print crop marks"));
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wframe), dialog->priv->crop_marks_check);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->crop_marks_check),
				      FALSE);

	g_signal_connect (G_OBJECT(label), "merge_changed",
			  G_CALLBACK (merge_changed_cb), dialog);

	g_signal_connect (G_OBJECT(label), "size_changed",
			  G_CALLBACK (size_changed_cb), dialog);

	return vbox;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create "Printer" page.                                          */
/*---------------------------------------------------------------------------*/
static GtkWidget *
printer_page_new (glPrintDialog *dialog,
		  glLabel       *label)
{
	GtkWidget *vbox;
	GtkWidget *printer_select;

	vbox = gl_hig_vbox_new (GL_HIG_VBOX_OUTER);

	/* FIXME: GnomePrinterSelector is not public in libgnomeprintui-2.2 */
	dialog->priv->printer_select =
		gnome_printer_selector_new (gnome_print_config_default ());
	gtk_widget_show (dialog->priv->printer_select);
	gtk_box_pack_start (GTK_BOX(vbox), dialog->priv->printer_select, TRUE, TRUE, 0);

	return vbox;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  "merge_changed" callback.                                      */
/*--------------------------------------------------------------------------*/
static void
merge_changed_cb (glLabel            *label,
		  glPrintDialog      *dialog)
{
	glMerge   *merge;
	gint       n_records;

	gl_debug (DEBUG_PRINT, "START");

	merge = gl_label_get_merge (label);
	if (merge == NULL) {

		gtk_widget_show_all (dialog->priv->simple_frame);
		gtk_widget_hide_all (dialog->priv->merge_frame);

	} else {

		n_records = gl_merge_get_record_count( merge );
		gl_wdgt_print_merge_set_copies (GL_WDGT_PRINT_MERGE(dialog->priv->prmerge),
						1, 1, n_records, FALSE);
		g_object_unref (G_OBJECT(merge));

		gtk_widget_hide_all (dialog->priv->simple_frame);
		gtk_widget_show_all (dialog->priv->merge_frame);
	}

	gl_debug (DEBUG_PRINT, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  "size_changed" (template changed) callback.                    */
/*--------------------------------------------------------------------------*/
static void
size_changed_cb (glLabel            *label,
		 glPrintDialog      *dialog)
{
	gl_debug (DEBUG_PRINT, "START");

	/* Remove and unref original widgets. */
	gtk_container_remove (GTK_CONTAINER(GL_HIG_CATEGORY(dialog->priv->simple_frame)->vbox),
			      dialog->priv->copies);
	gtk_container_remove (GTK_CONTAINER(GL_HIG_CATEGORY(dialog->priv->merge_frame)->vbox),
			      dialog->priv->prmerge);

	/* Create new widgets based on updated label and install */
	dialog->priv->copies = gl_wdgt_print_copies_new (label);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(dialog->priv->simple_frame),
				    dialog->priv->copies);
	dialog->priv->prmerge = gl_wdgt_print_merge_new (label);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(dialog->priv->merge_frame),
				    dialog->priv->prmerge);

	/* Update these widgets. */
	merge_changed_cb (label, dialog);

	gl_debug (DEBUG_PRINT, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  delete event callback.                                         */
/*--------------------------------------------------------------------------*/
static void
delete_event_cb (glPrintDialog *dialog,
		  gpointer       user_data)
{
	gl_debug (DEBUG_PRINT, "START");

	gtk_widget_hide (GTK_WIDGET(dialog));

	gl_debug (DEBUG_PRINT, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Print "response" callback.                                      */
/*---------------------------------------------------------------------------*/
static void
print_response_cb (glPrintDialog *dialog,
		   gint           response,
		   glLabel       *label)
{
	GnomePrintConfig *config;
	glMerge          *merge;
	gboolean          outline_flag, reverse_flag, crop_marks_flag, collate_flag;
	gint              first, last, n_sheets, n_copies;

	switch (response) {

	case GNOME_PRINT_DIALOG_RESPONSE_PRINT:
	case GNOME_PRINT_DIALOG_RESPONSE_PREVIEW:

		/* FIXME: GnomePrinterSelector is not public in libgnomeprintui-2.2. */
		config = gnome_printer_selector_get_config (GNOME_PRINTER_SELECTOR(dialog->priv->printer_select));

		outline_flag =
			gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
						      (dialog->priv->outline_check));
		reverse_flag =
			gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
						      (dialog->priv->reverse_check));
		crop_marks_flag =
			gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
						      (dialog->priv->crop_marks_check));

		merge = gl_label_get_merge (label);

		if (merge == NULL) {

			gl_wdgt_print_copies_get_range (GL_WDGT_PRINT_COPIES (dialog->priv->copies),
							&n_sheets, &first, &last);
			print_sheets (config, label,
				      (response == GNOME_PRINT_DIALOG_RESPONSE_PREVIEW),
				      n_sheets, first, last,
				      outline_flag, reverse_flag, crop_marks_flag);

		} else {

			gl_wdgt_print_merge_get_copies (GL_WDGT_PRINT_MERGE (dialog->priv->prmerge),
							&n_copies, &first,
							&collate_flag);
			print_sheets_merge (config, label,
					    (response == GNOME_PRINT_DIALOG_RESPONSE_PREVIEW),
					    n_copies, first,
					    collate_flag,
					    outline_flag,
					    reverse_flag,
					    crop_marks_flag);
			g_object_unref (G_OBJECT(merge));
		}
		break;

	default:
		break;

	}

	gtk_widget_hide (GTK_WIDGET(dialog));
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  print the sheets                                                */
/*---------------------------------------------------------------------------*/
static void
print_sheets (GnomePrintConfig *config,
	      glLabel          *label,
	      gboolean          preview_flag,
	      gint              n_sheets,
	      gint              first,
	      gint              last,
	      gboolean          outline_flag,
	      gboolean          reverse_flag,
	      gboolean          crop_marks_flag)
{
	GnomePrintJob *job;
	glPrintFlags   flags;

	job = gnome_print_job_new (config);
	flags.outline = outline_flag;
	flags.reverse = reverse_flag;
	flags.crop_marks = crop_marks_flag;
	gl_print_simple (job, label, n_sheets, first, last, &flags);
	gnome_print_job_close (job);

	if (preview_flag) {
		GtkWidget *preview_widget =
		    gnome_print_job_preview_new (job, _("Print preview"));
		gtk_widget_show (GTK_WIDGET (preview_widget));
	} else {
		gnome_print_job_print (job);
	}

	g_object_unref (G_OBJECT (job));
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  print the sheets with merge data                                */
/*---------------------------------------------------------------------------*/
static void
print_sheets_merge (GnomePrintConfig *config,
		    glLabel          *label,
		    gboolean          preview_flag,
		    gint              n_copies,
		    gint              first,
		    gboolean          collate_flag,
		    gboolean          outline_flag,
		    gboolean          reverse_flag,
		    gboolean          crop_marks_flag)
{
	GnomePrintJob *job;
	glPrintFlags   flags;

	job = gnome_print_job_new (config);
	flags.outline = outline_flag;
	flags.reverse = reverse_flag;
	flags.crop_marks = crop_marks_flag;
	if ( collate_flag ) {
		gl_print_merge_collated (job, label, n_copies, first, &flags);
	} else {
		gl_print_merge_uncollated (job, label, n_copies, first, &flags);
	}
	gnome_print_job_close (job);

	if (preview_flag) {
	        GtkWidget *preview_widget =
		    gnome_print_job_preview_new (job, _("Print preview"));
		gtk_widget_show (GTK_WIDGET (preview_widget));
	} else {
		gnome_print_job_print (job);
	}

	g_object_unref (G_OBJECT (job));
}

/*****************************************************************************/
/* Set outline flag/checkbox.                                                */
/*****************************************************************************/
void
gl_print_dialog_force_outline_flag (glPrintDialog *dialog)
{
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(dialog->priv->outline_check),
				      TRUE);

	gtk_widget_set_sensitive (dialog->priv->outline_check, FALSE);
	gtk_widget_set_sensitive (dialog->priv->reverse_check, FALSE);
	gtk_widget_set_sensitive (dialog->priv->crop_marks_check, FALSE);
}


