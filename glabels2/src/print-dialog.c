/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  print.c:  Print module
 *
 *  Copyright (C) 2001  Jim Evins <evins@snaught.com>.
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

#include <math.h>
#include <time.h>
#include <ctype.h>
#include <gtk/gtk.h>
#include <libgnomeprint/gnome-print-paper.h>
#include <libgnomeprintui/gnome-printer-dialog.h>
#include <libgnomeprintui/gnome-print-dialog.h>
#include <libgnomeprint/gnome-print-master.h>
#include <libgnomeprintui/gnome-print-master-preview.h>

#include "print-dialog.h"
#include "hig.h"
#include "print.h"
#include "label.h"
#include "bc.h"
#include "template.h"

#include "wdgt-print-copies.h"
#include "wdgt-print-merge.h"

#include "debug.h"

/*===========================================*/
/* Private types.                            */
/*===========================================*/

/*===========================================*/
/* Private globals.                          */
/*===========================================*/

/* remember state of dialog. */
static gboolean outline_flag = FALSE;
static gboolean reverse_flag = FALSE;
static gboolean collate_flag = FALSE;
static gint first = 1, last = 1, n_sheets = 0, n_copies = 1;

/*===========================================*/
/* Private function prototypes.              */
/*===========================================*/
static GtkWidget *job_page_new     (GtkWidget *dlg, glLabel *label);
static GtkWidget *printer_page_new (GtkWidget *dlg, glLabel *label);

static void print_response (GtkDialog *dlg,
			    gint      response,
			    glLabel   *label);

static void print_sheets (GnomePrintConfig *config, glLabel * label,
			  gboolean preview_flag,
			  gint n_sheets, gint first, gint last,
			  gboolean outline_flag, gboolean reverse_flag);

static void print_sheets_merge (GnomePrintConfig *config, glLabel * label,
				GList *record_list,
				gboolean preview_flag,
				gint n_copies, gint first,
				gboolean collate_flag,
				gboolean outline_flag, gboolean reverse_flag);


/*****************************************************************************/
/* "Print" dialog.                                                           */
/*****************************************************************************/
void
gl_print_dialog (glLabel *label, BonoboWindow *win)
{
	GtkWidget *dlg;
	GtkWidget *pp_button, *notebook, *page;

	g_return_if_fail (label != NULL);
	g_return_if_fail (win != NULL);

	/* ----- Contstruct basic print dialog with notebook ----- */
	dlg = gl_hig_dialog_new_with_buttons (_("Print"), GTK_WINDOW(win),
					   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_PRINT, GNOME_PRINT_DIALOG_RESPONSE_PRINT,

					   NULL);
	pp_button = gtk_dialog_add_button (GTK_DIALOG (dlg),
					   GTK_STOCK_PRINT_PREVIEW, 
					   GNOME_PRINT_DIALOG_RESPONSE_PREVIEW);
	gtk_button_box_set_child_secondary (GTK_BUTTON_BOX (GTK_DIALOG (dlg)->action_area), 
					    pp_button, TRUE);
	gtk_dialog_set_default_response (GTK_DIALOG (dlg),
					 GNOME_PRINT_DIALOG_RESPONSE_PRINT);
	notebook = gtk_notebook_new ();
	gtk_widget_show (notebook);
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dlg), notebook);

	/* ----- Create Job notebook page ----- */
	page = job_page_new (dlg, label);
	gtk_notebook_append_page (GTK_NOTEBOOK(notebook), page,
				  gtk_label_new_with_mnemonic (_("_Job")));

	/* ----- Create Printer notebook page ----- */
	page = printer_page_new (dlg, label);
	gtk_notebook_append_page (GTK_NOTEBOOK(notebook), page,
				  gtk_label_new_with_mnemonic (_("P_rinter")));

	g_signal_connect (G_OBJECT(dlg), "response",
			  G_CALLBACK (print_response), label);

	gtk_widget_show_all (GTK_WIDGET (dlg));
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create "Job" page.                                              */
/*---------------------------------------------------------------------------*/
static GtkWidget *
job_page_new (GtkWidget *dlg,
	      glLabel   *label)
{
	GtkWidget *vbox;
	glMerge *merge;
	GtkWidget *wframe;
	GtkWidget *copies = NULL, *prmerge = NULL;
	GtkWidget *wvbox, *outline_check, *reverse_check;
	GList *record_list;
	gint n_records;

	vbox = gl_hig_vbox_new (GL_HIG_VBOX_OUTER);

	merge = gl_label_get_merge (label);
	if (merge == NULL) {

		/* ----------- Add simple-copies widget ------------ */
		wframe = gl_hig_category_new (_("Copies"));
		gl_hig_vbox_add_widget (GL_HIG_VBOX(vbox), wframe);

		copies = gl_wdgt_print_copies_new (label);
		gl_hig_category_add_widget (GL_HIG_CATEGORY(wframe), copies);

		if (n_sheets) {
			gl_wdgt_print_copies_set_range (GL_WDGT_PRINT_COPIES (copies),
							n_sheets, first, last);
		}

	} else {

		/* ------- Otherwise add merge control widget ------------ */
		wframe = gl_hig_category_new (_("Document merge control"));
		gl_hig_vbox_add_widget (GL_HIG_VBOX(vbox), wframe);

		prmerge = gl_wdgt_print_merge_new (label);
		gl_hig_category_add_widget (GL_HIG_CATEGORY(wframe), prmerge);

		record_list = gl_merge_read_record_list (merge);

		n_records = gl_merge_count_records( record_list );
		gl_wdgt_print_merge_set_copies (GL_WDGT_PRINT_MERGE(prmerge),
					   n_copies, first, n_records,
					   collate_flag);
		g_object_unref (G_OBJECT(merge));
	}
	gtk_widget_show_all (wframe);
	g_object_set_data (G_OBJECT(dlg), "copies", copies);
	g_object_set_data (G_OBJECT(dlg), "prmerge", prmerge);

	/* ----------- Add custom print options area ------------ */
	wframe = gl_hig_category_new (_("Options"));
	gl_hig_vbox_add_widget (GL_HIG_VBOX(vbox), wframe);

	/* add Outline check button */
	outline_check =
	    gtk_check_button_new_with_label (
		    _("print outlines (to test printer alignment)"));
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wframe), outline_check);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (outline_check),
				      outline_flag);
	g_object_set_data (G_OBJECT(dlg), "outline_check", outline_check);

	/* add Reverse check button */
	reverse_check =
	    gtk_check_button_new_with_label (
		    _("print in reverse (i.e. a mirror image)"));
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wframe), reverse_check);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (reverse_check),
				      reverse_flag);
	g_object_set_data (G_OBJECT(dlg), "reverse_check", reverse_check);

	gtk_widget_show_all (wframe);

	return vbox;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Create "Printer" page.                                          */
/*---------------------------------------------------------------------------*/
static GtkWidget *
printer_page_new (GtkWidget *dlg,
		  glLabel   *label)
{
	GtkWidget *vbox;
	GtkWidget *printer_select;

	vbox = gl_hig_vbox_new (GL_HIG_VBOX_OUTER);

	printer_select =
		gnome_printer_selection_new (gnome_print_config_default ());
	gtk_widget_show (printer_select);
	gl_hig_vbox_add_widget (GL_HIG_VBOX(vbox), printer_select);

	g_object_set_data (G_OBJECT(dlg), "printer_select", printer_select);

	return vbox;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Print "response" callback.                                      */
/*---------------------------------------------------------------------------*/
static void
print_response (GtkDialog *dlg,
		gint      response,
		glLabel   *label)
{
	GtkWidget *copies, *prmerge;
	GtkWidget *outline_check, *reverse_check;
	GtkWidget *printer_select;
	GnomePrintConfig *config;
	glMerge *merge;
	GList *record_list;

	switch (response) {

	case GNOME_PRINT_DIALOG_RESPONSE_PRINT:
	case GNOME_PRINT_DIALOG_RESPONSE_PREVIEW:
		copies         = g_object_get_data (G_OBJECT(dlg), "copies");
		prmerge        = g_object_get_data (G_OBJECT(dlg), "prmerge");
		outline_check  = g_object_get_data (G_OBJECT(dlg),
						    "outline_check");
		reverse_check  = g_object_get_data (G_OBJECT(dlg),
						    "reverse_check");
		printer_select = g_object_get_data (G_OBJECT(dlg),
						    "printer_select");

		config = gnome_printer_selection_get_config (GNOME_PRINTER_SELECTION(printer_select));

		outline_flag =
			gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
						      (outline_check));
		reverse_flag =
			gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
						      (reverse_check));

		merge = gl_label_get_merge (label);

		if (merge == NULL) {

			gl_wdgt_print_copies_get_range (GL_WDGT_PRINT_COPIES (copies),
							&n_sheets, &first, &last);
			print_sheets (config, label,
				      (response == GNOME_PRINT_DIALOG_RESPONSE_PREVIEW),
				      n_sheets, first, last,
				      outline_flag, reverse_flag);

		} else {

			record_list = gl_merge_read_record_list (merge);
			gl_wdgt_print_merge_get_copies (GL_WDGT_PRINT_MERGE (prmerge),
							&n_copies, &first,
							&collate_flag);
			print_sheets_merge (config, label, record_list,
					    (response == GNOME_PRINT_DIALOG_RESPONSE_PREVIEW),
					    n_copies, first,
					    collate_flag,
					    outline_flag,
					    reverse_flag);
			g_object_unref (G_OBJECT(merge));
		}
		break;

	default:
		break;

	}

	gtk_widget_destroy (GTK_WIDGET (dlg));
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  print the sheets                                                */
/*---------------------------------------------------------------------------*/
static void
print_sheets (GnomePrintConfig *config,
	      glLabel * label,
	      gboolean preview_flag,
	      gint n_sheets,
	      gint first,
	      gint last,
	      gboolean outline_flag,
	      gboolean reverse_flag)
{
	GnomePrintMaster *master;

	master = gnome_print_master_new_from_config (config);
	gl_print_simple (master, label, n_sheets, first, last,
			 outline_flag, reverse_flag);
	gnome_print_master_close (master);

	if (preview_flag) {
		GtkWidget *preview_widget =
		    gnome_print_master_preview_new (master, _("Print preview"));
		gtk_widget_show (GTK_WIDGET (preview_widget));
	} else {
		gnome_print_master_print (master);
	}

	g_object_unref (G_OBJECT (master));
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  print the sheets with merge data                                */
/*---------------------------------------------------------------------------*/
static void
print_sheets_merge (GnomePrintConfig *config,
		    glLabel * label,
		    GList * record_list,
		    gboolean preview_flag,
		    gint n_copies,
		    gint first,
		    gboolean collate_flag,
		    gboolean outline_flag,
		    gboolean reverse_flag)
{
	GnomePrintMaster *master;

	master = gnome_print_master_new_from_config (config);
	if ( collate_flag ) {
		gl_print_merge_collated (master, label, record_list,
					 n_copies, first,
					 outline_flag, reverse_flag);
	} else {
		gl_print_merge_uncollated (master, label, record_list,
					   n_copies, first,
					   outline_flag, reverse_flag);
	}
	gnome_print_master_close (master);

	if (preview_flag) {
	        GtkWidget *preview_widget =
		    gnome_print_master_preview_new (master, _("Print preview"));
		gtk_widget_show (GTK_WIDGET (preview_widget));
	} else {
		gnome_print_master_print (master);
	}

	g_object_unref (G_OBJECT (master));
}

