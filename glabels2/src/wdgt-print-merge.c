/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_print_merge.c:  print merge widget module
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

#include "wdgt-print-merge.h"
#include "wdgt-mini-preview.h"
#include "marshal.h"

#include "pixmaps/collate.xpm"
#include "pixmaps/nocollate.xpm"

#include "debug.h"

#define WDGT_MINI_PREVIEW_HEIGHT 175
#define WDGT_MINI_PREVIEW_WIDTH  150

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GObjectClass *parent_class;

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_wdgt_print_merge_class_init    (glWdgtPrintMergeClass * class);
static void gl_wdgt_print_merge_instance_init (glWdgtPrintMerge * merge);
static void gl_wdgt_print_merge_finalize      (GObject * object);

static void gl_wdgt_print_merge_construct     (glWdgtPrintMerge * merge,
					       glLabel * label);

static void preview_clicked                   (glWdgtMiniPreview *mini_preview,
					       gint i_label,
					       gpointer user_data);

static void collate_check_cb                  (GtkToggleButton * togglebutton,
					       gpointer user_data);
static void spin_cb                           (GtkSpinButton * spinbutton,
					       gpointer user_data);


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
guint
gl_wdgt_print_merge_get_type (void)
{
	static guint wdgt_print_merge_type = 0;

	if (!wdgt_print_merge_type) {
		GTypeInfo wdgt_print_merge_info = {
			sizeof (glWdgtPrintMergeClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_wdgt_print_merge_class_init,
			NULL,
			NULL,
			sizeof (glWdgtPrintMerge),
			0,
			(GInstanceInitFunc) gl_wdgt_print_merge_instance_init,
		};

		wdgt_print_merge_type =
			g_type_register_static (gtk_hbox_get_type (),
						"glWdgtPrintMerge",
						&wdgt_print_merge_info, 0);
	}

	return wdgt_print_merge_type;
}

static void
gl_wdgt_print_merge_class_init (glWdgtPrintMergeClass * class)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) class;

	parent_class = gtk_type_class (gtk_hbox_get_type ());

	object_class->finalize = gl_wdgt_print_merge_finalize;
}

static void
gl_wdgt_print_merge_instance_init (glWdgtPrintMerge * merge)
{
	merge->mini_preview = NULL;

	merge->copies_spin = NULL;
	merge->first_spin = NULL;
	merge->collate_image = NULL;
	merge->collate_check = NULL;
}

static void
gl_wdgt_print_merge_finalize (GObject * object)
{
	glWdgtPrintMerge *merge;
	glWdgtPrintMergeClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WDGT_PRINT_MERGE (object));

	merge = GL_WDGT_PRINT_MERGE (object);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

GtkWidget *
gl_wdgt_print_merge_new (glLabel * label)
{
	glWdgtPrintMerge *merge;

	merge = g_object_new (gl_wdgt_print_merge_get_type (), NULL);

	gl_wdgt_print_merge_construct (merge, label);

	return GTK_WIDGET (merge);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                    */
/*--------------------------------------------------------------------------*/
static void
gl_wdgt_print_merge_construct (glWdgtPrintMerge * merge,
			       glLabel * label)
{
	GtkWidget *whbox, *wvbox, *whbox1;
	GtkObject *adjust;
	GdkPixbuf *pixbuf;
	const glTemplate *template;

	whbox = GTK_WIDGET (merge);

	template = gl_label_get_template (label);
	merge->labels_per_sheet = template->nx * template->ny;

	/* mini_preview canvas */
	merge->mini_preview = gl_wdgt_mini_preview_new (WDGT_MINI_PREVIEW_HEIGHT,
							WDGT_MINI_PREVIEW_WIDTH);
	gl_wdgt_mini_preview_set_label( GL_WDGT_MINI_PREVIEW (merge->mini_preview),
				   template->name->data );
	gtk_box_pack_start (GTK_BOX (whbox), merge->mini_preview,
			    TRUE, TRUE, GNOME_PAD);
	gl_wdgt_mini_preview_highlight_range (GL_WDGT_MINI_PREVIEW(merge->mini_preview),
					      1, 1);

	wvbox = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_box_pack_start (GTK_BOX (whbox), wvbox, TRUE, TRUE, GNOME_PAD);

	/* First Label controls */
	whbox1 = gtk_hbox_new (FALSE, GNOME_PAD);
	gtk_box_pack_start (GTK_BOX (wvbox), whbox1, FALSE, FALSE, GNOME_PAD);
	gtk_box_pack_start (GTK_BOX (whbox1),
			    gtk_label_new (_("Start on label")), FALSE, FALSE,
			    0);
	adjust =
	    gtk_adjustment_new (1, 1.0, merge->labels_per_sheet, 1.0, 10.0,
				10.0);
	merge->first_spin =
	    gtk_spin_button_new (GTK_ADJUSTMENT (adjust), 1.0, 0);
	gtk_box_pack_start (GTK_BOX (whbox1), merge->first_spin, FALSE, FALSE,
			    0);
	gtk_box_pack_start (GTK_BOX (whbox1), gtk_label_new (_("on 1st sheet")),
			    FALSE, FALSE, 0);

	/* Collate controls */
	whbox1 = gtk_hbox_new (FALSE, GNOME_PAD);
	gtk_box_pack_end (GTK_BOX (wvbox), whbox1, FALSE, FALSE, GNOME_PAD);
	pixbuf = gdk_pixbuf_new_from_xpm_data ( (const char **)nocollate_xpm);
	merge->collate_image = gtk_image_new_from_pixbuf(pixbuf);

	gtk_box_pack_start (GTK_BOX (whbox1), merge->collate_image,
			    FALSE, FALSE, 0);
	merge->collate_check = gtk_check_button_new_with_label (_("Collate"));
	gtk_box_pack_start (GTK_BOX (whbox1), merge->collate_check,
			    FALSE, FALSE, 0);

	/* Copy controls */
	whbox1 = gtk_hbox_new (FALSE, GNOME_PAD);
	gtk_box_pack_end (GTK_BOX (wvbox), whbox1, FALSE, FALSE, GNOME_PAD);
	gtk_box_pack_start (GTK_BOX (whbox1), gtk_label_new (_("Copies:")),
			    FALSE, FALSE, 0);
	adjust = gtk_adjustment_new (1, 1.0, 10.0, 1.0, 10.0, 10.0);
	merge->copies_spin = gtk_spin_button_new (GTK_ADJUSTMENT (adjust),
						   1.0, 0);
	gtk_box_pack_start (GTK_BOX (whbox1), merge->copies_spin,
			    FALSE, FALSE, 0);

	/* Connect signals to controls */
	g_signal_connect (G_OBJECT (merge->mini_preview), "clicked",
			  G_CALLBACK (preview_clicked), merge);
	g_signal_connect (G_OBJECT (merge->collate_check), "toggled",
			  G_CALLBACK (collate_check_cb), merge);
	g_signal_connect (G_OBJECT (merge->copies_spin), "changed",
			  G_CALLBACK (spin_cb), merge);
	g_signal_connect (G_OBJECT (merge->first_spin), "changed",
			  G_CALLBACK (spin_cb), merge);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Enable/Disable appropriate controls due to radio button toggle.*/
/*--------------------------------------------------------------------------*/
static void
collate_check_cb (GtkToggleButton * togglebutton,
		  gpointer user_data)
{
	glWdgtPrintMerge *merge = GL_WDGT_PRINT_MERGE (user_data);
	GdkPixbuf *pixbuf;

	if (gtk_toggle_button_get_active (togglebutton)) {

	        pixbuf =
		  gdk_pixbuf_new_from_xpm_data ( (const char **)collate_xpm);

	} else {

	        pixbuf =
		  gdk_pixbuf_new_from_xpm_data ( (const char **)nocollate_xpm);

	}
	gtk_image_set_from_pixbuf (GTK_IMAGE (merge->collate_image),
				   pixbuf);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change of first spin button               */
/*--------------------------------------------------------------------------*/
static void
spin_cb (GtkSpinButton * spinbutton,
	 gpointer user_data)
{
	glWdgtPrintMerge *merge = GL_WDGT_PRINT_MERGE (user_data);
	gint first, last, n_copies;

	first =
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
					      (merge->first_spin));

	n_copies =
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
					      (merge->copies_spin));
	last = first + (n_copies * merge->n_records) - 1;
	gl_wdgt_mini_preview_highlight_range (GL_WDGT_MINI_PREVIEW(merge->mini_preview),
					 first, last );
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Canvas event handler, select first and last items.             */
/*--------------------------------------------------------------------------*/
static void
preview_clicked (glWdgtMiniPreview *mini_preview,
		 gint first,
		 gpointer user_data)
{
	glWdgtPrintMerge *merge = GL_WDGT_PRINT_MERGE (user_data);
	gint n_copies, last;

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (merge->first_spin),
				   first);

	n_copies =
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
					      (merge->copies_spin));
	last = first + (n_copies * merge->n_records) - 1;
	gl_wdgt_mini_preview_highlight_range (GL_WDGT_MINI_PREVIEW (merge->mini_preview),
					 first, last);

}

/****************************************************************************/
/* query selected range of labels within sheet or number of sheets.         */
/****************************************************************************/
void
gl_wdgt_print_merge_get_copies (glWdgtPrintMerge * merge,
				gint * n_copies,
				gint * first_label,
				gboolean * collate_flag)
{
	*n_copies =
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
					      (merge->copies_spin));
	*first_label =
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
					      (merge->first_spin));
	*collate_flag =
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
					  (merge->collate_check));
}

/****************************************************************************/
/* set range of labels within sheet or number of sheets                     */
/****************************************************************************/
void
gl_wdgt_print_merge_set_copies (glWdgtPrintMerge * merge,
				gint n_copies,
				gint first_label,
				gint n_records,
				gboolean collate_flag)
{
	gint last_label;

	merge->n_records = n_records;

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (merge->copies_spin),
				   n_copies);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (merge->first_spin),
				   first_label);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (merge->collate_check),
				      collate_flag);

	last_label = first_label + (n_copies * n_records) - 1;
	gl_wdgt_mini_preview_highlight_range (GL_WDGT_MINI_PREVIEW (merge->mini_preview),
					 first_label, last_label );
}
