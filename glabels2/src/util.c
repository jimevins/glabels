/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  util.c:  various small utility functions
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

#include <string.h>
#include <glib.h>
#include <math.h>
#include <libgnomeprint/gnome-font.h>

#include "util.h"

#define FRAC_DELTA 0.00005


/****************************************************************************/
/* Append ".glabels" extension to filename if needed.                       */
/****************************************************************************/
gchar *
gl_util_add_extension (const gchar * orig_filename)
{
	gchar *new_filename, *extension;

	extension = strrchr (orig_filename, '.');
	if (extension == NULL) {
		new_filename = g_strconcat (orig_filename, ".glabels", NULL);
	} else {
		if (g_strcasecmp (extension, ".glabels") != 0) {
			new_filename =
			    g_strconcat (orig_filename, ".glabels", NULL);
		} else {
			new_filename = g_strdup (orig_filename);
		}
	}

	return new_filename;
}

/****************************************************************************/
/* Make sure we have an absolute path to filename.                          */
/****************************************************************************/
gchar *
gl_util_make_absolute (const gchar * filename)
{
	gchar *pwd, *absolute_filename;

	if (g_path_is_absolute (filename)) {
		absolute_filename = g_strdup (filename);
	} else {
		pwd = g_get_current_dir ();
		absolute_filename =
		    g_strjoin (G_DIR_SEPARATOR_S, pwd, filename, NULL);
		g_free (pwd);
	}

	return absolute_filename;
}

/****************************************************************************/
/* Create fractional representation of number, if possible.                 */
/****************************************************************************/
gchar *
gl_util_fraction( gdouble x )
{
	static gdouble denom[] = { 1., 2., 3., 4., 8., 16., 32., 0. };
	gint i;
	gdouble product, remainder;
	gint n, d;

	for ( i=0; denom[i] != 0.0; i++ ) {
		product = x * denom[i];
		remainder = fabs(product - ((gint)(product+0.5)));
		if ( remainder < FRAC_DELTA ) break;
	}

	if ( denom[i] == 0.0 ) {
		/* None of our denominators work. */
		return g_strdup_printf ("%.5g", x);
	}
	if ( denom[i] == 1.0 ) {
		/* Simple integer. */
		return g_strdup_printf ("%d", (gint)x);
	}
	n = (gint)( x * denom[i] + 0.5 );
	d = (gint)denom[i];
	if ( n > d ) {
		return g_strdup_printf ("%d_%d/%d", (n/d), (n%d), d);
	} else {
		return g_strdup_printf ("%d/%d", (n%d), d);
	}
}

/****************************************************************************/
/* Create button w/ both text and stock image.                              */
/****************************************************************************/
GtkWidget* 
gl_button_new_with_stock_image (const gchar* text, const gchar* stock_id)
{
	GtkWidget *button;
	GtkStockItem item;
	GtkWidget *label;
	GtkWidget *image;
	GtkWidget *hbox;
	GtkWidget *align;

	button = gtk_button_new ();

 	if (GTK_BIN (button)->child)
    		gtk_container_remove (GTK_CONTAINER (button),
				      GTK_BIN (button)->child);

  	if (gtk_stock_lookup (stock_id, &item))
    	{
      		label = gtk_label_new_with_mnemonic (text);

		gtk_label_set_mnemonic_widget (GTK_LABEL (label), GTK_WIDGET (button));
      
		image = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_BUTTON);
      		hbox = gtk_hbox_new (FALSE, 2);

      		align = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
      
      		gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
      		gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);
      
      		gtk_container_add (GTK_CONTAINER (button), align);
      		gtk_container_add (GTK_CONTAINER (align), hbox);
      		gtk_widget_show_all (align);

      		return button;
    	}

      	label = gtk_label_new_with_mnemonic (text);
      	gtk_label_set_mnemonic_widget (GTK_LABEL (label), GTK_WIDGET (button));
  
  	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);

  	gtk_widget_show (label);
  	gtk_container_add (GTK_CONTAINER (button), label);

	return button;
}

/****************************************************************************/
/* Add button to gtk_dialog w/ text and stock image.                        */
/****************************************************************************/
GtkWidget*
gl_util_dialog_add_button (GtkDialog *dialog,
			   const gchar* text,
			   const gchar* stock_id,
			   gint response_id)
{
	GtkWidget *button;
	
	g_return_val_if_fail (GTK_IS_DIALOG (dialog), NULL);
	g_return_val_if_fail (text != NULL, NULL);
	g_return_val_if_fail (stock_id != NULL, NULL);

	button = gl_button_new_with_stock_image (text, stock_id);
	g_return_val_if_fail (button != NULL, NULL);

	GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

	gtk_widget_show (button);

	gtk_dialog_add_action_widget (dialog, button, response_id);	

	return button;
}

/****************************************************************************/
/* Utilities to deal with GTK_JUSTIFICATION types.                          */
/****************************************************************************/
const gchar *
gl_util_just_to_string (GtkJustification just)
{
	switch (just) {
	case GTK_JUSTIFY_LEFT:
		return "Left";
	case GTK_JUSTIFY_CENTER:
		return "Center";
	case GTK_JUSTIFY_RIGHT:
		return "Right";
	default:
		return "?";
	}
}

GtkJustification
gl_util_string_to_just (const gchar * string)
{

	if (g_strcasecmp (string, "Left") == 0) {
		return GTK_JUSTIFY_LEFT;
	} else if (g_strcasecmp (string, "Center") == 0) {
		return GTK_JUSTIFY_CENTER;
	} else if (g_strcasecmp (string, "Right") == 0) {
		return GTK_JUSTIFY_RIGHT;
	} else {
		return GTK_JUSTIFY_LEFT;
	}

}

/****************************************************************************/
/* Utilities to deal with GNOME_FONT_WEIGHT types                           */
/****************************************************************************/
const gchar *
gl_util_weight_to_string (GnomeFontWeight weight)
{
	switch (weight) {
	case GNOME_FONT_BOOK:
		return "Regular";
	case GNOME_FONT_BOLD:
		return "Bold";
	default:
		return "?";
	}
}

GnomeFontWeight
gl_util_string_to_weight (const gchar * string)
{

	if (g_strcasecmp (string, "Regular") == 0) {
		return GNOME_FONT_BOOK;
	} else if (g_strcasecmp (string, "Bold") == 0) {
		return GNOME_FONT_BOLD;
	} else {
		return GNOME_FONT_BOOK;
	}

}

/****************************************************************************/
/* Create a message dialog that attempts to be HIG compliant.               */
/****************************************************************************/
GtkWidget* gl_util_hig_dialog_new      (GtkWindow      *parent,
                                        GtkDialogFlags  flags,
                                        GtkMessageType  type,
                                        GtkButtonsType  buttons,
					const gchar    *primary_text,
                                        const gchar    *secondary_text)
{
	GtkWidget *dialog, *hbox, *image, *label;
	gchar *label_text;
	const gchar *stock_id = NULL;
	GtkStockItem item;
  
	/* Create bare dialog */
	dialog = gtk_dialog_new_with_buttons ("", parent, flags, NULL);

	/* Create HBOX */
	hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog)->vbox),
			    hbox, FALSE, FALSE, 0);

	/* Create image */
	switch (type) {
	case GTK_MESSAGE_INFO:
		stock_id = GTK_STOCK_DIALOG_INFO;
		break;
	case GTK_MESSAGE_QUESTION:
		stock_id = GTK_STOCK_DIALOG_QUESTION;
		break;
	case GTK_MESSAGE_WARNING:
		stock_id = GTK_STOCK_DIALOG_WARNING;
		break;
	case GTK_MESSAGE_ERROR:
		stock_id = GTK_STOCK_DIALOG_ERROR;
		break;
	default:
		stock_id = GTK_STOCK_DIALOG_INFO;
		g_warning ("Unknown GtkMessageType %d", type);
		break;
	}
	if (gtk_stock_lookup (stock_id, &item)) {
		image = gtk_image_new_from_stock (stock_id,
						  GTK_ICON_SIZE_DIALOG);
	} else {
		image = gtk_image_new_from_stock (NULL,
						  GTK_ICON_SIZE_DIALOG);
		g_warning ("Stock dialog ID doesn't exist?");
	}
	gtk_box_pack_start (GTK_BOX(hbox), image, FALSE, FALSE, 0);

	/* Create label containing primary and secondary text */
	label_text = g_strdup_printf ("<span weight=\"bold\" size=\"larger\">%s</span>\n\n%s",
				      primary_text, secondary_text);
	label = gtk_label_new (label_text);
	g_free (label_text);
	gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 0);

	/* Adjust dialog properties */
	gtk_window_set_title (GTK_WINDOW(dialog), "");
	gtk_container_set_border_width (GTK_CONTAINER(dialog), 6);
	gtk_dialog_set_has_separator (GTK_DIALOG(dialog), FALSE);

	/* Adjust VBOX properties */
	gtk_box_set_spacing (GTK_BOX(GTK_DIALOG(dialog)->vbox), 12);

	/* Adjust HBOX properties */
	gtk_box_set_spacing (GTK_BOX(hbox), 12);
	gtk_container_set_border_width (GTK_CONTAINER(hbox), 6);

	/* Adjust IMAGE properties */
	gtk_misc_set_alignment (GTK_MISC(image), 0.5, 0.0);

	/* Adjust LABEL properties */
	gtk_label_set_line_wrap (GTK_LABEL(label), TRUE);
	gtk_label_set_use_markup (GTK_LABEL(label), TRUE);
	gtk_misc_set_alignment (GTK_MISC(image), 0.5, 0.0);

	/* Add buttons */
	switch (buttons) {
	case GTK_BUTTONS_NONE:
		/* nothing */
		break;
	case GTK_BUTTONS_OK:
		gtk_dialog_add_button (GTK_DIALOG(dialog),
				       GTK_STOCK_OK, GTK_RESPONSE_OK);
		break;
	case GTK_BUTTONS_CLOSE:
		gtk_dialog_add_button (GTK_DIALOG(dialog),
				       GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
		break;
	case GTK_BUTTONS_CANCEL:
		gtk_dialog_add_button (GTK_DIALOG(dialog),
				       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
		break;
	case GTK_BUTTONS_YES_NO:
		gtk_dialog_add_button (GTK_DIALOG(dialog),
				       GTK_STOCK_NO, GTK_RESPONSE_NO);
		gtk_dialog_add_button (GTK_DIALOG(dialog),
				       GTK_STOCK_YES, GTK_RESPONSE_YES);
		break;
	case GTK_BUTTONS_OK_CANCEL:
		gtk_dialog_add_button (GTK_DIALOG(dialog),
				       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
		gtk_dialog_add_button (GTK_DIALOG(dialog),
				       GTK_STOCK_OK, GTK_RESPONSE_OK);
		break;
	default:
		g_warning ("Unknown GtkButtonsType");
		break;
	}

	/* Show dialog widgets */
	gtk_widget_show_all (hbox);

	return dialog;
}

