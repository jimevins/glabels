/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  print_copies.c:  custom print copies widget module
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

#include "print_copies.h"
#include "mini_preview.h"

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

static void gl_print_copies_class_init (glPrintCopiesClass * class);
static void gl_print_copies_init (glPrintCopies * copies);
static void gl_print_copies_destroy (GtkObject * object);

static void gl_print_copies_construct (glPrintCopies * copies, glLabel * label);

static void sheets_radio_cb (GtkToggleButton * togglebutton, gpointer user_data);
static void first_spin_cb (GtkSpinButton * spinbutton, gpointer user_data);
static void last_spin_cb (GtkSpinButton * spinbutton, gpointer user_data);

static void
preview_pressed (glMiniPreview *mini_preview,
		 gint first, gint last, gpointer user_data);


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
guint
gl_print_copies_get_type (void)
{
	static guint print_copies_type = 0;

	if (!print_copies_type) {
		GtkTypeInfo print_copies_info = {
			"glPrintCopies",
			sizeof (glPrintCopies),
			sizeof (glPrintCopiesClass),
			(GtkClassInitFunc) gl_print_copies_class_init,
			(GtkObjectInitFunc) gl_print_copies_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
		};

		print_copies_type = gtk_type_unique (gtk_hbox_get_type (),
						     &print_copies_info);
	}

	return print_copies_type;
}

static void
gl_print_copies_class_init (glPrintCopiesClass * class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	parent_class = gtk_type_class (gtk_hbox_get_type ());

	object_class->destroy = gl_print_copies_destroy;
}

static void
gl_print_copies_init (glPrintCopies * copies)
{
	copies->labels_per_sheet = 0;

	copies->mini_preview = NULL;

	copies->sheets_radio = NULL;
	copies->sheets_spin = NULL;

	copies->labels_radio = NULL;
	copies->first_spin = NULL;
	copies->last_spin = NULL;
}

static void
gl_print_copies_destroy (GtkObject * object)
{
	glPrintCopies *copies;
	glPrintCopiesClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_PRINT_COPIES (object));

	copies = GL_PRINT_COPIES (object);
	class = GL_PRINT_COPIES_CLASS (GTK_OBJECT (copies)->klass);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

GtkWidget *
gl_print_copies_new (glLabel * label)
{
	glPrintCopies *copies;

	copies = gtk_type_new (gl_print_copies_get_type ());

	gl_print_copies_construct (copies, label);

	return GTK_WIDGET (copies);
}

/*--------------------------------------------------------------------------*/
/* Construct composite widget.                                              */
/*--------------------------------------------------------------------------*/
static void
gl_print_copies_construct (glPrintCopies * copies,
			   glLabel * label)
{
	GtkWidget *whbox, *wvbox, *whbox1;
	GSList *radio_group = NULL;
	GtkObject *adjust;

	whbox = GTK_WIDGET (copies);

	copies->labels_per_sheet = label->template->nx * label->template->ny;

	/* mini_preview canvas */
	copies->mini_preview = gl_mini_preview_new (MINI_PREVIEW_HEIGHT,
						    MINI_PREVIEW_WIDTH);
	gl_mini_preview_set_label (GL_MINI_PREVIEW(copies->mini_preview),
				   label->template->name->data);
	gtk_box_pack_start (GTK_BOX (whbox), copies->mini_preview,
			    TRUE, TRUE, GNOME_PAD);

	wvbox = gtk_vbox_new (FALSE, GNOME_PAD);
	gtk_box_pack_start (GTK_BOX (whbox), wvbox, TRUE, TRUE, GNOME_PAD);

	/* Sheet controls */
	whbox1 = gtk_hbox_new (FALSE, GNOME_PAD);
	gtk_box_pack_start (GTK_BOX (wvbox), whbox1, FALSE, FALSE, GNOME_PAD);
	copies->sheets_radio =
	    gtk_radio_button_new_with_label (radio_group, _("Sheets:"));
	gtk_box_pack_start (GTK_BOX (whbox1), copies->sheets_radio,
			    FALSE, FALSE, 0);
	adjust = gtk_adjustment_new (1, 1.0, 10.0, 1.0, 10.0, 10.0);
	copies->sheets_spin = gtk_spin_button_new (GTK_ADJUSTMENT (adjust),
						   1.0, 0);
	gtk_box_pack_start (GTK_BOX (whbox1), copies->sheets_spin,
			    FALSE, FALSE, 0);
	gl_mini_preview_highlight_range (GL_MINI_PREVIEW(copies->mini_preview),
					 1, copies->labels_per_sheet);

	/* Label controls */
	whbox1 = gtk_hbox_new (FALSE, GNOME_PAD);
	gtk_box_pack_start (GTK_BOX (wvbox), whbox1, FALSE, FALSE, GNOME_PAD);
	radio_group =
	    gtk_radio_button_group (GTK_RADIO_BUTTON (copies->sheets_radio));
	copies->labels_radio =
	    gtk_radio_button_new_with_label (radio_group, _("Labels"));
	gtk_box_pack_start (GTK_BOX (whbox1), copies->labels_radio,
			    FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (whbox1), gtk_label_new (_("from:")),
			    FALSE, FALSE, 0);
	adjust = gtk_adjustment_new (1, 1.0, copies->labels_per_sheet,
				     1.0, 10.0, 10.0);
	copies->first_spin = gtk_spin_button_new (GTK_ADJUSTMENT (adjust),
						  1.0, 0);
	gtk_box_pack_start (GTK_BOX (whbox1), copies->first_spin,
			    FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (whbox1), gtk_label_new (_("to:")),
			    FALSE, FALSE, 0);
	adjust = gtk_adjustment_new (copies->labels_per_sheet,
				     1.0, copies->labels_per_sheet,
				     1.0, 10.0, 10.0);
	copies->last_spin = gtk_spin_button_new (GTK_ADJUSTMENT (adjust),
						 1.0, 0);
	gtk_box_pack_start (GTK_BOX (whbox1), copies->last_spin,
			    FALSE, FALSE, 0);
	gtk_widget_set_sensitive (copies->first_spin, FALSE);
	gtk_widget_set_sensitive (copies->last_spin, FALSE);

	/* Connect signals to controls */
	gtk_signal_connect (GTK_OBJECT (copies->mini_preview), "pressed",
			    GTK_SIGNAL_FUNC (preview_pressed), copies);
	gtk_signal_connect (GTK_OBJECT (copies->sheets_radio), "toggled",
			    GTK_SIGNAL_FUNC (sheets_radio_cb), copies);
	gtk_signal_connect (GTK_OBJECT (copies->first_spin), "changed",
			    GTK_SIGNAL_FUNC (first_spin_cb), copies);
	gtk_signal_connect (GTK_OBJECT (copies->last_spin), "changed",
			    GTK_SIGNAL_FUNC (last_spin_cb), copies);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Enable/Disable appropriate controls due to radio button toggle.*/
/*--------------------------------------------------------------------------*/
static void
sheets_radio_cb (GtkToggleButton * togglebutton,
		 gpointer user_data)
{
	glPrintCopies *copies = GL_PRINT_COPIES (user_data);
	gint first, last;

	if (gtk_toggle_button_get_active (togglebutton)) {

		gtk_widget_set_sensitive (copies->sheets_spin, TRUE);
		gtk_widget_set_sensitive (copies->first_spin, FALSE);
		gtk_widget_set_sensitive (copies->last_spin, FALSE);

		gl_mini_preview_highlight_range (GL_MINI_PREVIEW(copies->mini_preview),
					1, copies->labels_per_sheet);

	} else {

		gtk_widget_set_sensitive (copies->sheets_spin, FALSE);
		gtk_widget_set_sensitive (copies->first_spin, TRUE);
		gtk_widget_set_sensitive (copies->last_spin, TRUE);

		first =
		    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
						      (copies->first_spin));
		last =
		    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
						      (copies->last_spin));
		gl_mini_preview_highlight_range (GL_MINI_PREVIEW(copies->mini_preview),
						 first, last);

	}
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change of first spin button               */
/*--------------------------------------------------------------------------*/
static void
first_spin_cb (GtkSpinButton * spinbutton,
	       gpointer user_data)
{
	glPrintCopies *copies = GL_PRINT_COPIES (user_data);
	gint first, last;

	first =
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
					      (copies->first_spin));
	last =
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
					      (copies->last_spin));

	gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (copies->last_spin))->
	    lower = first;

	gl_mini_preview_highlight_range (GL_MINI_PREVIEW(copies->mini_preview),
					 first, last);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change of last spin button                */
/*--------------------------------------------------------------------------*/
static void
last_spin_cb (GtkSpinButton * spinbutton,
	      gpointer user_data)
{
	glPrintCopies *copies = GL_PRINT_COPIES (user_data);
	gint first, last;

	first =
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
					      (copies->first_spin));
	last =
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
					      (copies->last_spin));

	gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (copies->first_spin))->
	    upper = last;

	gl_mini_preview_highlight_range (GL_MINI_PREVIEW(copies->mini_preview),
					 first, last);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Canvas event handler, select first and last items.             */
/*--------------------------------------------------------------------------*/
static void
preview_pressed (glMiniPreview *mini_preview,
		 gint first, gint last, gpointer user_data)
{
	glPrintCopies *copies = GL_PRINT_COPIES (user_data);

	gl_print_copies_set_range (copies, 1, first, last);
}

/****************************************************************************/
/* query selected range of labels within sheet or number of sheets.         */
/****************************************************************************/
void
gl_print_copies_get_range (glPrintCopies * copies,
			   gint * n_sheets,
			   gint * first_label,
			   gint * last_label)
{
	gboolean sheets_active;

	sheets_active =
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
					  (copies->sheets_radio));

	if (sheets_active) {
		*n_sheets =
		    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
						      (copies->sheets_spin));
		*first_label = 1;
		*last_label = copies->labels_per_sheet;
	} else {
		*n_sheets = 1;
		*first_label =
		    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
						      (copies->first_spin));
		*last_label =
		    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
						      (copies->last_spin));
	}
}

/****************************************************************************/
/* set range of labels within sheet or number of sheets                     */
/****************************************************************************/
void
gl_print_copies_set_range (glPrintCopies * copies,
			   gint n_sheets,
			   gint first_label,
			   gint last_label)
{
	gint old_first_label;

	old_first_label =
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
					      (copies->first_spin));

	if (first_label > old_first_label) {
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (copies->last_spin),
					   last_label);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (copies->first_spin),
					   first_label);
	} else {
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (copies->first_spin),
					   first_label);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (copies->last_spin),
					   last_label);
	}
	if ((first_label == 1) && (last_label == copies->labels_per_sheet)) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (copies->sheets_radio), TRUE);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON
					   (copies->sheets_spin), n_sheets);
	} else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (copies->labels_radio), TRUE);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON
					   (copies->sheets_spin), 1.0);
	}
}
