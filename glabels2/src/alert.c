/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  alert.c:  a HIG inspired alert dialog
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

#include <glib.h>

#include "alert.h"



/****************************************************************************/
/* Create a message dialog that attempts to be HIG compliant.               */
/****************************************************************************/
GtkWidget* gl_alert_dialog_new      (GtkWindow      *parent,
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

