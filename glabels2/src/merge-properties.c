/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  merge_properties.c:  document merge properties dialog module
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

#include "glabels.h"
#include "mdi.h"
#include "view.h"
#include "merge.h"
#include "merge-ui.h"
#include "merge-properties.h"

#include "debug.h"

/*===========================================*/
/* Private data types                        */
/*===========================================*/

typedef struct {
	GtkWidget *dialog;

	glView *view;
	glLabel *label;

	GtkWidget *type_entry;
	GtkWidget *src_entry;
	GtkWidget *field_ws;

	glMergeType src_type;
	gchar *field_ws_src;

} PropertyDialogPassback;

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void create_merge_dialog_widgets (GtkDialog * dialog,
					 PropertyDialogPassback * data);

static void type_changed_cb (GtkWidget * widget, PropertyDialogPassback * data);
static void src_changed_cb (GtkWidget * widget, PropertyDialogPassback * data);

static void response_cb (GtkDialog * dialog, gint response,
			 PropertyDialogPassback * data);

/****************************************************************************/
/* Launch merge properties dialog.                                          */
/****************************************************************************/
void
gl_merge_properties_dialog (glView * view)
{
	static PropertyDialogPassback *data = NULL;
	GtkWidget *dialog;
	BonoboWindow *win = glabels_get_active_window ();

	if (data == NULL) {
		data = g_new0 (PropertyDialogPassback, 1);
	}

	dialog = gtk_dialog_new_with_buttons (
                                   _("Edit document-merge properties"),
				   GTK_WINDOW (win),
				   GTK_DIALOG_DESTROY_WITH_PARENT,
				   GTK_STOCK_OK, GTK_RESPONSE_OK,
				   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				   NULL);

	data->dialog = dialog;
	data->view = view;
	data->label = view->label;

	data->type_entry = NULL;
	data->src_entry = NULL;
	data->field_ws = NULL;

	create_merge_dialog_widgets (GTK_DIALOG (dialog), data);

	g_signal_connect (G_OBJECT(dialog), "response",
			  G_CALLBACK(response_cb), data);

	gtk_widget_show_all (GTK_WIDGET (dialog));

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Create and add start page to druid.                            */
/*--------------------------------------------------------------------------*/
static void
create_merge_dialog_widgets (GtkDialog * dialog,
			     PropertyDialogPassback * data)
{
	GtkWidget *wvbox, *wframe, *whbox, *wtable, *wlabel, *wcombo, *wscroll;
	GList *texts;
	glMerge *merge;
	glMergeType type;
	gchar *src;
	GList *fields;

	merge = gl_label_get_merge (data->label);
	type = merge->type;
	src  = merge->src;
	fields = merge->field_defs;
	gl_merge_free (&merge);

	wvbox = dialog->vbox;

	wframe = gtk_frame_new (_("Source"));
	gtk_box_pack_start (GTK_BOX (wvbox), wframe, FALSE, FALSE, 0);

	whbox = gtk_hbox_new (FALSE, GNOME_PAD);
	gtk_container_add (GTK_CONTAINER (wframe), whbox);

	wtable = gtk_table_new (2, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (wtable), 10);
	gtk_table_set_row_spacings (GTK_TABLE (wtable), 5);
	gtk_table_set_col_spacings (GTK_TABLE (wtable), 5);
	gtk_box_pack_start (GTK_BOX (whbox), wtable, FALSE, FALSE, GNOME_PAD);

	wlabel = gtk_label_new (_("Format:"));
	gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (wtable), wlabel, 0, 1, 0, 1);

	wcombo = gtk_combo_new ();
	gtk_widget_set_usize (wcombo, 400, -1);
	texts = gl_merge_get_long_texts_list ();
	gtk_combo_set_popdown_strings (GTK_COMBO (wcombo), texts);
	gl_merge_free_long_texts_list (&texts);
	data->type_entry = GTK_COMBO (wcombo)->entry;
	gtk_entry_set_editable (GTK_ENTRY (data->type_entry), FALSE);
	gtk_table_attach_defaults (GTK_TABLE (wtable), wcombo, 1, 2, 0, 1);
	gtk_entry_set_text (GTK_ENTRY (data->type_entry),
			    gl_merge_type_to_long_text (type));

	wlabel = gtk_label_new (_("Location:"));
	gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (wtable), wlabel, 0, 1, 1, 2);

	data->src_entry = gl_merge_ui_src_new ();
	gtk_table_attach_defaults (GTK_TABLE (wtable), data->src_entry, 1, 2, 1,
				   2);
	gl_merge_ui_src_set_type (GL_MERGE_UI_SRC (data->src_entry), type);
	gl_merge_ui_src_set_value (GL_MERGE_UI_SRC (data->src_entry), src);

	wframe = gtk_frame_new (_("Fields"));
	gtk_box_pack_start (GTK_BOX (wvbox), wframe, FALSE, FALSE, 0);

	wscroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_set_border_width (GTK_CONTAINER (wscroll), 5);
	gtk_widget_set_usize (wscroll, 500, 300);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (wscroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (wframe), wscroll);

	data->field_ws = gl_merge_ui_field_ws_new ();
	gtk_container_set_border_width (GTK_CONTAINER (data->field_ws), 10);
	gl_merge_ui_field_ws_set_type_src (GL_MERGE_UI_FIELD_WS
					   (data->field_ws), type, src);
	gl_merge_ui_field_ws_set_field_defs (GL_MERGE_UI_FIELD_WS
					     (data->field_ws), fields);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (wscroll),
					       data->field_ws);

	g_signal_connect (G_OBJECT (data->type_entry), "changed",
			  G_CALLBACK (type_changed_cb), data);
	g_signal_connect (G_OBJECT (data->src_entry), "changed",
			  G_CALLBACK (src_changed_cb), data);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  type "changed" callback.                                       */
/*--------------------------------------------------------------------------*/
static void
type_changed_cb (GtkWidget * widget,
		 PropertyDialogPassback * data)
{
	glMergeType type;
	gchar *type_text;

	type_text = gtk_editable_get_chars (GTK_EDITABLE (data->type_entry),
					    0, -1);
	type = gl_merge_long_text_to_type (type_text);
	g_free (type_text);

	gl_merge_ui_src_set_type (GL_MERGE_UI_SRC (data->src_entry), type);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  source "changed" callback.                                     */
/*--------------------------------------------------------------------------*/
static void
src_changed_cb (GtkWidget * widget,
		PropertyDialogPassback * data)
{
	glMergeType type;
	gchar *type_text, *src;

	type_text = gtk_editable_get_chars (GTK_EDITABLE (data->type_entry),
					    0, -1);
	type = gl_merge_long_text_to_type (type_text);
	g_free (type_text);

	src = gl_merge_ui_src_get_value (GL_MERGE_UI_SRC (data->src_entry));
	gl_merge_ui_field_ws_set_type_src (GL_MERGE_UI_FIELD_WS
					   (data->field_ws), type, src);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  response callback.                                             */
/*--------------------------------------------------------------------------*/
static void
response_cb (GtkDialog * dialog,
	     gint response,
	     PropertyDialogPassback * data)
{
	gchar *type_text;
	glMerge *merge;

	switch (response) {

	case GTK_RESPONSE_OK:

		merge = gl_merge_new();

		type_text =
		       gtk_editable_get_chars (GTK_EDITABLE (data->type_entry),
						    0, -1);
		merge->type = gl_merge_long_text_to_type (type_text);
		g_free (type_text);

		merge->src =
			gl_merge_ui_src_get_value (GL_MERGE_UI_SRC (data->src_entry));

		merge->field_defs =
			gl_merge_ui_field_ws_get_field_defs (GL_MERGE_UI_FIELD_WS
							     (data->field_ws));

		gl_label_set_merge (data->label, merge);

		gl_merge_free (&merge);

		break;
	}

	gtk_widget_destroy (data->dialog);
}
