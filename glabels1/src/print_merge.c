/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  print_merge.c:  print merge widget module
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

#include "print_merge.h"
#include "mini_preview.h"

#include "pixmaps/collate.xpm"
#include "pixmaps/nocollate.xpm"

#include "debug.h"

#define MINI_PREVIEW_HEIGHT 175
#define MINI_PREVIEW_WIDTH  150

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkContainerClass *parent_class;

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_print_merge_class_init (glPrintMergeClass * class);
static void gl_print_merge_init (glPrintMerge * merge);
static void gl_print_merge_destroy (GtkObject * object);

static void gl_print_merge_construct (glPrintMerge * merge,
					     glLabel * label);

static void preview_clicked (glMiniPreview *mini_preview,
			     gint i_label, gpointer user_data);

static void collate_check_cb (GtkToggleButton * togglebutton,
			      gpointer user_data);
static void spin_cb (GtkSpinButton * spinbutton, gpointer user_data);


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
guint
gl_print_merge_get_type (void)
{
	static guint print_merge_type = 0;

	if (!print_merge_type) {
		GtkTypeInfo print_merge_info = {
			"glPrintMerge",
			sizeof (glPrintMerge),
			sizeof (glPrintMergeClass),
			(GtkClassInitFunc) gl_print_merge_class_init,
			(GtkObjectInitFunc) gl_print_merge_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
		};

		print_merge_type = gtk_type_unique (gtk_hbox_get_type (),
						    &print_merge_info);
	}

	return print_merge_type;
}

static void
gl_print_merge_class_init (glPrintMergeClass * class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	parent_class = gtk_type_class (gtk_hbox_get_type ());

	object_class->destroy = gl_print_merge_destroy;
}

static void
gl_print_merge_init (glPrintMerge * merge)
{
	merge->mini_preview = NULL;

	merge->copies_spin = NULL;
	merge->first_spin = NULL;
	merge->collate_image = NULL;
	merge->collate_check = NULL;
}

static void
gl_print_merge_destroy (GtkObject * object)
{
	glPrintMerge *merge;
	glPrintMergeClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_PRINT_MERGE (object));

	merge = GL_PRINT_MERGE (object);
	class = GL_PRINT_MERGE_CLASS (GTK_OBJECT (merge)->klass);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

GtkWidget *
gl_print_merge_new (glLabel * label)
{
	glPrintMerge *merge;

	merge = gtk_type_new (gl_print_merge_get_type ());

	gl_print_merge_construct (merge, label);

	return GTK_WIDGET (merge);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                    */
/*--------------------------------------------------------------------------*/
static void
gl_print_merge_construct (glPrintMerge * merge,
			  glLabel * label)
{
	GtkWidget *whbox, *wvbox, *whbox1;
	GtkObject *adjust;

	whbox = GTK_WIDGET (merge);

	merge->labels_per_sheet = label->template->nx * label->template->ny;

	/* mini_preview canvas */
	merge->mini_preview = gl_mini_preview_new (MINI_PREVIEW_HEIGHT,
						   MINI_PREVIEW_WIDTH);
	gl_mini_preview_set_label( GL_MINI_PREVIEW (merge->mini_preview),
				   label->template->name->data );
	gtk_box_pack_start (GTK_BOX (whbox), merge->mini_preview,
			    TRUE, TRUE, GNOME_PAD);
	gl_mini_preview_highlight_range (GL_MINI_PREVIEW(merge->mini_preview),
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
	merge->collate_image = gnome_pixmap_new_from_xpm_d (nocollate_xpm);
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
	gtk_signal_connect (GTK_OBJECT (merge->mini_preview), "clicked",
			    GTK_SIGNAL_FUNC (preview_clicked), merge);
	gtk_signal_connect (GTK_OBJECT (merge->collate_check), "toggled",
			    GTK_SIGNAL_FUNC (collate_check_cb), merge);
	gtk_signal_connect (GTK_OBJECT (merge->copies_spin), "changed",
			    GTK_SIGNAL_FUNC (spin_cb), merge);
	gtk_signal_connect (GTK_OBJECT (merge->first_spin), "changed",
			    GTK_SIGNAL_FUNC (spin_cb), merge);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Enable/Disable appropriate controls due to radio button toggle.*/
/*--------------------------------------------------------------------------*/
static void
collate_check_cb (GtkToggleButton * togglebutton,
		  gpointer user_data)
{
	glPrintMerge *merge = GL_PRINT_MERGE (user_data);

	if (gtk_toggle_button_get_active (togglebutton)) {

		gnome_pixmap_load_xpm_d (GNOME_PIXMAP (merge->collate_image),
					 (char **) collate_xpm);

	} else {

		gnome_pixmap_load_xpm_d (GNOME_PIXMAP (merge->collate_image),
					 (char **) nocollate_xpm);

	}
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change of first spin button               */
/*--------------------------------------------------------------------------*/
static void
spin_cb (GtkSpinButton * spinbutton,
	 gpointer user_data)
{
	glPrintMerge *merge = GL_PRINT_MERGE (user_data);
	gint first, last, n_copies;

	first =
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
					      (merge->first_spin));

	n_copies =
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
					      (merge->copies_spin));
	last = first + (n_copies * merge->n_records) - 1;
	gl_mini_preview_highlight_range (GL_MINI_PREVIEW(merge->mini_preview),
					 first, last );
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Canvas event handler, select first and last items.             */
/*--------------------------------------------------------------------------*/
static void
preview_clicked (glMiniPreview *mini_preview,
		 gint first, gpointer user_data)
{
	glPrintMerge *merge = GL_PRINT_MERGE (user_data);
	gint n_copies, last;

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (merge->first_spin),
				   first);

	n_copies =
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
					      (merge->copies_spin));
	last = first + (n_copies * merge->n_records) - 1;
	gl_mini_preview_highlight_range (GL_MINI_PREVIEW (merge->mini_preview),
					 first, last);

}

/****************************************************************************/
/* query selected range of labels within sheet or number of sheets.         */
/****************************************************************************/
void
gl_print_merge_get_copies (glPrintMerge * merge,
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
gl_print_merge_set_copies (glPrintMerge * merge,
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
	gl_mini_preview_highlight_range (GL_MINI_PREVIEW (merge->mini_preview),
					 first_label, last_label );
}
