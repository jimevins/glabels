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
#include <libgnome/gnome-paper.h>
#include <libgnomeprint/gnome-printer.h>
#include <libgnomeprint/gnome-print-dialog.h>
#include <libgnomeprint/gnome-print-master.h>
#include <libgnomeprint/gnome-print-master-preview.h>

#include "print_dialog.h"
#include "print.h"
#include "label.h"
#include "bc.h"
#include "template.h"
#include "mdi.h"
#include "hack.h"

#include "print_copies.h"
#include "print_merge.h"

#include "debug.h"

#define RED(x)   ( (((x)>>24) & 0xff) / 255.0 )
#define GREEN(x) ( (((x)>>16) & 0xff) / 255.0 )
#define BLUE(x)  ( (((x)>>8)  & 0xff) / 255.0 )
#define ALPHA(x) ( ( (x)      & 0xff) / 255.0 )

/*===========================================*/
/* Private types.                            */
/*===========================================*/

/*===========================================*/
/* Private function prototypes.              */
/*===========================================*/
static void print_sheets (GtkWidget * dlg, glLabel * label,
			  gboolean preview_flag,
			  gint n_sheets, gint first, gint last,
			  gboolean outline_flag, gboolean reverse_flag);

static void print_sheets_merge (GtkWidget * dlg, glLabel * label,
				GList *record_list,
				gboolean preview_flag,
				gint n_copies, gint first,
				gboolean collate_flag,
				gboolean outline_flag, gboolean reverse_flag);


/*****************************************************************************/
/* "Print" menu item callback.                                               */
/*****************************************************************************/
void
gl_print_dialog_cb (GtkWidget * widget, gpointer data)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (GNOME_MDI (data));
	GnomeApp *app = gnome_mdi_get_active_window (GNOME_MDI (data));
	GtkWidget *dlg = NULL;
	GtkWidget *copies = NULL;
	GtkWidget *merge = NULL;
	GtkWidget *outline_check = NULL;
	GtkWidget *reverse_check = NULL;
	GtkWidget *wframe, *wvbox;
	glLabel *label;
	gint ret;
	static gboolean outline_flag = FALSE;
	static gboolean reverse_flag = FALSE;
	static gboolean collate_flag = FALSE;
	static gint first = 1, last = 1, n_sheets = 0, n_copies = 1;
	GList *record_list = NULL;
	gint n_records;

	g_return_if_fail (child != NULL);
	g_return_if_fail (app != NULL);

	label = gl_mdi_get_label (child);

	dlg = gnome_print_dialog_new (_("Print"), 0);

	if (label->merge_type == GL_MERGE_NONE) {

		/* ----------- Add simple-copies widget ------------ */
		wframe = gtk_frame_new (_("Copies"));
		gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dlg)->vbox), wframe,
				    FALSE, FALSE, 0);

		copies = gl_print_copies_new (label);
		gtk_container_set_border_width (GTK_CONTAINER (copies),
						GNOME_PAD_SMALL);
		gtk_container_add (GTK_CONTAINER (wframe), copies);

		if (n_sheets) {
			gl_print_copies_set_range (GL_PRINT_COPIES (copies),
						   n_sheets, first, last);
		}

	} else {

		/* -------Otherwise add merge control widget ------------ */
		wframe = gtk_frame_new (_("Document merge control"));
		gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dlg)->vbox), wframe,
				    FALSE, FALSE, 0);

		merge = gl_print_merge_new (label);
		gtk_container_set_border_width (GTK_CONTAINER (merge),
						GNOME_PAD_SMALL);
		gtk_container_add (GTK_CONTAINER (wframe), merge);

		record_list = gl_merge_read_data (label->merge_type,
						  label->merge_fields,
						  label->merge_src);
		n_records = gl_merge_count_records( record_list );
		gl_print_merge_set_copies (GL_PRINT_MERGE(merge),
					   n_copies, first, n_records,
					   collate_flag);
	}
	gtk_widget_show_all (wframe);

	/* ----------- Add custom print options area ------------ */
	wframe = gtk_frame_new (_("Options"));
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dlg)->vbox), wframe,
			    FALSE, FALSE, 0);
	wvbox = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox),
					GNOME_PAD_SMALL);
	gtk_container_add (GTK_CONTAINER (wframe), wvbox);

	/* add Outline check button */
	outline_check =
	    gtk_check_button_new_with_label (
		    _("print outlines (to test printer alignment)"));
	gtk_box_pack_start (GTK_BOX (wvbox), outline_check, FALSE, FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (outline_check),
				      outline_flag);

	/* add Reverse check button */
	reverse_check =
	    gtk_check_button_new_with_label (
		    _("print in reverse (i.e. a mirror image)"));
	gtk_box_pack_start (GTK_BOX (wvbox), reverse_check, FALSE, FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (reverse_check),
				      reverse_flag);

	gtk_widget_show_all (wframe);

	gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
			    GTK_SIGNAL_FUNC (gtk_widget_destroyed), &dlg);

	gtk_window_set_transient_for (GTK_WINDOW (dlg), GTK_WINDOW (app));

	/* ----------- Now run the dialog and act ---------------- */
	ret = gnome_dialog_run (GNOME_DIALOG (dlg));

	switch (ret) {
		
	case GNOME_PRINT_PRINT:
	case GNOME_PRINT_PREVIEW:
		outline_flag =
			gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
						      (outline_check));
		reverse_flag =
			gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
						      (reverse_check));

		if (label->merge_type == GL_MERGE_NONE) {

			gl_print_copies_get_range (GL_PRINT_COPIES (copies),
						   &n_sheets, &first, &last);
			print_sheets (dlg, label,
				      (ret == GNOME_PRINT_PREVIEW),
				      n_sheets, first, last,
				      outline_flag, reverse_flag);

		} else {

			gl_print_merge_get_copies (GL_PRINT_MERGE (merge),
						   &n_copies, &first,
						   &collate_flag);
			print_sheets_merge (dlg, label, record_list,
					    (ret == GNOME_PRINT_PREVIEW),
					    n_copies, first,
					    collate_flag,
					    outline_flag,
					    reverse_flag);
		}
		break;

	default:
		break;

	}

	if (dlg) gtk_widget_destroy (GTK_WIDGET (dlg));


}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  print the sheets                                                */
/*---------------------------------------------------------------------------*/
static void
print_sheets (GtkWidget * dlg,
	      glLabel * label,
	      gboolean preview_flag,
	      gint n_sheets,
	      gint first,
	      gint last,
	      gboolean outline_flag,
	      gboolean reverse_flag)
{
	GnomePrintMaster *master;

	master = gnome_print_master_new_from_dialog (GNOME_PRINT_DIALOG (dlg));
	gl_print_simple (master, label, n_sheets, first, last,
			 outline_flag, reverse_flag);
	gnome_print_master_close (master);

	if (preview_flag) {
		GnomePrintMasterPreview *preview_widget =
		    gnome_print_master_preview_new (master, _("Print preview"));
		gtk_widget_show (GTK_WIDGET (preview_widget));
	} else {
		gnome_print_master_print (master);
	}

	gtk_object_unref (GTK_OBJECT (master));
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  print the sheets with merge data                                */
/*---------------------------------------------------------------------------*/
static void
print_sheets_merge (GtkWidget * dlg,
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

	master = gnome_print_master_new_from_dialog (GNOME_PRINT_DIALOG (dlg));
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
		GnomePrintMasterPreview *preview_widget =
		    gnome_print_master_preview_new (master, _("Print preview"));
		gtk_widget_show (GTK_WIDGET (preview_widget));
	} else {
		gnome_print_master_print (master);
	}

	gtk_object_unref (GTK_OBJECT (master));
}

