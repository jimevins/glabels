/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  merge-properties-dialog.c:  document merge properties dialog module
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

#include "view.h"
#include "merge.h"
#include "merge-properties-dialog.h"
#include "hig.h"

#include "debug.h"

/*===========================================*/
/* Private data types                        */
/*===========================================*/

typedef struct {
	GtkWidget *dialog;

	glView  *view;
	glLabel *label;
	glMerge *merge;

	GtkWidget *type_entry;
	GtkWidget *src_entry;
	GtkWidget *sample;

	glMergeSrcType  src_type;
	gchar          *sample_src;

} PropertyDialogPassback;

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void create_merge_dialog_widgets (glHigDialog            *dialog,
					 PropertyDialogPassback *data);

static void type_changed_cb             (GtkWidget              *widget,
					 PropertyDialogPassback *data);

static void src_changed_cb              (GtkWidget              *widget,
					 PropertyDialogPassback *data);

static void response_cb                 (glHigDialog            *dialog,
					 gint                    response,
					 PropertyDialogPassback *data);

/****************************************************************************/
/* Launch merge properties dialog.                                          */
/****************************************************************************/
void
gl_merge_properties_dialog (glView *view)
{
	PropertyDialogPassback *data;
	GtkWidget *dialog;

	data = g_new0 (PropertyDialogPassback, 1);

	dialog = gl_hig_dialog_new_with_buttons (
		_("Edit document-merge properties"),
		NULL,
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK, GTK_RESPONSE_OK,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		NULL);

	data->dialog = dialog;
	data->view = view;
	data->label = view->label;

	create_merge_dialog_widgets (GL_HIG_DIALOG (dialog), data);

	g_signal_connect (G_OBJECT(dialog), "response",
			  G_CALLBACK(response_cb), data);

	gtk_widget_show_all (GTK_WIDGET (dialog));

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Create merge widgets.                                          */
/*--------------------------------------------------------------------------*/
static void
create_merge_dialog_widgets (glHigDialog * dialog,
			     PropertyDialogPassback * data)
{
	GtkWidget         *wframe, *whbox, *wtable, *wlabel, *wcombo, *wscroll, *wentry;
	GList             *texts;
	gchar             *description;
	glMergeSrcType     src_type;
	gchar             *src;
	GtkSizeGroup      *label_size_group;

	data->merge = gl_label_get_merge (data->label);
	description = gl_merge_get_description (data->merge);
	src_type    = gl_merge_get_src_type (data->merge);
	src         = gl_merge_get_src (data->merge);

	/* ---- Source section ---- */
	wframe = gl_hig_category_new (_("Source"));
	gl_hig_dialog_add_widget (dialog, wframe);
	label_size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	/* Format line */
	whbox = gl_hig_hbox_new();
	gl_hig_category_add_widget (GL_HIG_CATEGORY (wframe), whbox);

	wlabel = gtk_label_new (_("Format:"));
	gtk_size_group_add_widget (label_size_group, wlabel);
	gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), wlabel);

	wcombo = gtk_combo_new ();
	texts = gl_merge_get_descriptions ();
	gtk_combo_set_popdown_strings (GTK_COMBO (wcombo), texts);
	gl_merge_free_descriptions (&texts);
	data->type_entry = GTK_COMBO (wcombo)->entry;
	gtk_entry_set_editable (GTK_ENTRY (data->type_entry), FALSE);
	gtk_entry_set_text (GTK_ENTRY (data->type_entry), description);
	gl_hig_hbox_add_widget_justify (GL_HIG_HBOX(whbox), wcombo);
	g_signal_connect (G_OBJECT (data->type_entry), "changed",
			  G_CALLBACK (type_changed_cb), data);

	whbox = gl_hig_hbox_new();
	gl_hig_category_add_widget (GL_HIG_CATEGORY (wframe), whbox);

	/* Location line */
	wlabel = gtk_label_new (_("Location:"));
	gtk_size_group_add_widget (label_size_group, wlabel);
	gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), wlabel);

	switch (src_type) {
	case GL_MERGE_SRC_IS_FILE:
		data->src_entry =
			gnome_file_entry_new (NULL, _("Select merge-database source"));
		wentry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY(data->src_entry));
		gtk_entry_set_text (GTK_ENTRY(wentry), src);
		g_signal_connect (G_OBJECT (wentry), "changed",
				  G_CALLBACK (src_changed_cb), data);
		break;
	default:
		data->src_entry = gtk_label_new (_("N/A"));
		break;
	}
	gl_hig_hbox_add_widget_justify (GL_HIG_HBOX(whbox), data->src_entry);

	/* ---- Sample Fields section ---- */
	wframe = gl_hig_category_new (_("Sample fields"));
	gl_hig_dialog_add_widget (dialog, wframe);

#if 0
	wscroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_set_border_width (GTK_CONTAINER (wscroll), 5);
	gtk_widget_set_usize (wscroll, 400, 250);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (wscroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wframe), wscroll);

	data->sample = gl_merge_ui_field_ws_new ();
	gtk_container_set_border_width (GTK_CONTAINER (data->sample), 10);
	gl_merge_ui_field_ws_set_type_src (GL_MERGE_UI_FIELD_WS
					   (data->field_ws), type, src);
	gl_merge_ui_field_ws_set_field_defs (GL_MERGE_UI_FIELD_WS
					     (data->sample), fields);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (wscroll),
					       data->sample);
#endif

	g_free (src);
	g_free (description);

        gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  type "changed" callback.                                       */
/*--------------------------------------------------------------------------*/
static void
type_changed_cb (GtkWidget * widget,
		 PropertyDialogPassback * data)
{
	gchar             *description;
	gchar             *name;
	gchar             *src;
	glMergeSrcType     src_type;
	GtkWidget         *wentry;

	description = gtk_editable_get_chars (GTK_EDITABLE (data->type_entry),
					      0, -1);
	name = gl_merge_description_to_name (description);

	src = gl_merge_get_src (data->merge); /* keep current source if possible */

	g_object_unref (G_OBJECT(data->merge));
	data->merge = gl_merge_new (name);

	gtk_widget_destroy (data->src_entry);
	src_type = gl_merge_get_src_type (data->merge);
	switch (src_type) {
	case GL_MERGE_SRC_IS_FILE:
		data->src_entry =
			gnome_file_entry_new (NULL, _("Select merge-database source"));
		wentry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY(data->src_entry));
		gtk_entry_set_text (GTK_ENTRY(wentry), src);
		g_signal_connect (G_OBJECT (wentry), "changed",
				  G_CALLBACK (src_changed_cb), data);
		break;
	default:
		data->src_entry = gtk_label_new (_("N/A"));
		break;
	}

	g_free (description);
	g_free (name);
	g_free (src);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  source "changed" callback.                                     */
/*--------------------------------------------------------------------------*/
static void
src_changed_cb (GtkWidget * widget,
		PropertyDialogPassback * data)
{
	gchar     *src;
	GtkWidget *wentry;

	wentry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY(data->src_entry));
	src = gtk_editable_get_chars (GTK_EDITABLE (wentry), 0, -1);

#if 0
	gl_merge_ui_field_ws_set_type_src (GL_MERGE_UI_FIELD_WS
					   (data->sample), type, src);
#endif
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  response callback.                                             */
/*--------------------------------------------------------------------------*/
static void
response_cb (glHigDialog * dialog,
	     gint response,
	     PropertyDialogPassback * data)
{
	switch (response) {

	case GTK_RESPONSE_OK:

		gl_label_set_merge (data->label, data->merge);
		break;
	}


	g_object_unref (G_OBJECT(data->merge));
	g_free (data);
	gtk_widget_destroy (GTK_WIDGET(dialog));
}
