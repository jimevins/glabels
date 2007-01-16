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

#include <math.h>
#include <glib/gi18n.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtklabel.h>


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
GType
gl_wdgt_print_merge_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (glWdgtPrintMergeClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_wdgt_print_merge_class_init,
			NULL,
			NULL,
			sizeof (glWdgtPrintMerge),
			0,
			(GInstanceInitFunc) gl_wdgt_print_merge_instance_init,
			NULL
		};

		type = g_type_register_static (GL_TYPE_HIG_HBOX,
					       "glWdgtPrintMerge", &info, 0);
	}

	return type;
}

static void
gl_wdgt_print_merge_class_init (glWdgtPrintMergeClass * class)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) class;

	parent_class = g_type_class_peek_parent (class);

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
	glTemplate                *template;
	const glTemplateLabelType *label_type;
	GtkWidget                 *whbox, *wvbox, *whbox1;
	GtkObject                 *adjust;
	GdkPixbuf                 *pixbuf;

	whbox = GTK_WIDGET (merge);

	template   = gl_label_get_template (label);
	label_type = gl_template_get_first_label_type (template);

	merge->labels_per_sheet = gl_template_get_n_labels (label_type);

	/* mini_preview canvas */
	merge->mini_preview = gl_wdgt_mini_preview_new (WDGT_MINI_PREVIEW_HEIGHT,
							WDGT_MINI_PREVIEW_WIDTH);
	gl_wdgt_mini_preview_set_template( GL_WDGT_MINI_PREVIEW (merge->mini_preview),
					   template );
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), merge->mini_preview);
	gl_wdgt_mini_preview_highlight_range (GL_WDGT_MINI_PREVIEW(merge->mini_preview),
					      1, 1);

	wvbox = gl_hig_vbox_new (GL_HIG_VBOX_INNER);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), wvbox);

	/* First Label controls */
	whbox1 = gl_hig_hbox_new ();
	gl_hig_vbox_add_widget (GL_HIG_VBOX(wvbox), whbox1);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox1),
				gtk_label_new (_("Start on label")));
	adjust =
	    gtk_adjustment_new (1, 1.0, merge->labels_per_sheet, 1.0, 10.0,
				10.0);
	merge->first_spin =
	    gtk_spin_button_new (GTK_ADJUSTMENT (adjust), 1.0, 0);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox1), merge->first_spin);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox1),
				gtk_label_new (_("on 1st sheet")));

	/* Blank line */
	gl_hig_vbox_add_widget (GL_HIG_VBOX(wvbox), gtk_label_new (""));

	/* Copy & collate controls*/
	whbox1 = gl_hig_hbox_new ();
	gl_hig_vbox_add_widget (GL_HIG_VBOX(wvbox), whbox1);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox1),
				gtk_label_new (_("Copies:")));
	adjust = gtk_adjustment_new (1, 1.0, 10.0, 1.0, 10.0, 10.0);
	merge->copies_spin = gtk_spin_button_new (GTK_ADJUSTMENT (adjust),
						   1.0, 0);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox1), merge->copies_spin);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox1), gtk_label_new ("    "));
	merge->collate_check = gtk_check_button_new_with_label (_("Collate"));
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox1), merge->collate_check);
	pixbuf = gdk_pixbuf_new_from_xpm_data ( (const char **)nocollate_xpm);
	merge->collate_image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_widget_set_sensitive (merge->collate_check, FALSE);
	gtk_widget_set_sensitive (merge->collate_image, FALSE);

	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox1), merge->collate_image);

	/* Connect signals to controls */
	g_signal_connect (G_OBJECT (merge->mini_preview), "clicked",
			  G_CALLBACK (preview_clicked), merge);
	g_signal_connect (G_OBJECT (merge->collate_check), "toggled",
			  G_CALLBACK (collate_check_cb), merge);
	g_signal_connect (G_OBJECT (merge->copies_spin), "changed",
			  G_CALLBACK (spin_cb), merge);
	g_signal_connect (G_OBJECT (merge->first_spin), "changed",
			  G_CALLBACK (spin_cb), merge);

	gl_template_free (template);
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

	gtk_widget_set_sensitive (merge->collate_check, (n_copies > 1));
	gtk_widget_set_sensitive (merge->collate_image, (n_copies > 1));
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
gl_wdgt_print_merge_get_copies (glWdgtPrintMerge *merge,
				gint             *n_copies,
				gint             *first_label,
				gboolean         *collate_flag,
				gint             *n_sheets)
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

	*n_sheets = ceil ( (double)(*first_label - 1 + (*n_copies * merge->n_records))/ (double)merge->labels_per_sheet );
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
