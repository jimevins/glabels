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
#include "hig.h"

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
	static PropertyDialogPassback *data = NULL;
	GtkWidget *dialog;
	BonoboWindow *win = glabels_get_active_window ();

	if (data == NULL) {
		data = g_new0 (PropertyDialogPassback, 1);
	}

	dialog = gl_hig_dialog_new_with_buttons (
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
	GtkWidget *wframe, *whbox, *wtable, *wlabel, *wcombo, *wscroll;
	GList *texts;
	glMerge *merge;
	glMergeType type;
	gchar *src;
	GList *fields;
	GtkSizeGroup *label_size_group;

	merge = gl_label_get_merge (data->label);
	type = merge->type;
	src  = merge->src;
	fields = merge->field_defs;

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
	texts = gl_merge_get_long_texts_list ();
	gtk_combo_set_popdown_strings (GTK_COMBO (wcombo), texts);
	gl_merge_free_long_texts_list (&texts);
	data->type_entry = GTK_COMBO (wcombo)->entry;
	gtk_entry_set_editable (GTK_ENTRY (data->type_entry), FALSE);
	gtk_entry_set_text (GTK_ENTRY (data->type_entry),
			    gl_merge_type_to_long_text (type));
	gl_hig_hbox_add_widget_justify (GL_HIG_HBOX(whbox), wcombo);

	whbox = gl_hig_hbox_new();
	gl_hig_category_add_widget (GL_HIG_CATEGORY (wframe), whbox);

	/* Location line */
	wlabel = gtk_label_new (_("Location:"));
	gtk_size_group_add_widget (label_size_group, wlabel);
	gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), wlabel);

	data->src_entry = gl_merge_ui_src_new ();
	gl_merge_ui_src_set_type (GL_MERGE_UI_SRC (data->src_entry), type);
	gl_merge_ui_src_set_value (GL_MERGE_UI_SRC (data->src_entry), src);
	gl_hig_hbox_add_widget_justify (GL_HIG_HBOX(whbox), data->src_entry);

	/* ---- Fields section ---- */
	wframe = gl_hig_category_new (_("Fields"));
	gl_hig_dialog_add_widget (dialog, wframe);

	wscroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_set_border_width (GTK_CONTAINER (wscroll), 5);
	gtk_widget_set_usize (wscroll, 400, 250);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (wscroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gl_hig_category_add_widget (GL_HIG_CATEGORY(wframe), wscroll);

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

	gl_merge_free (&merge);
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
response_cb (glHigDialog * dialog,
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
