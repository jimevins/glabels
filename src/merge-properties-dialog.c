/*
 *  merge-properties-dialog.c
 *  Copyright (C) 2001-2009  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of gLabels.
 *
 *  gLabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gLabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gLabels.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "merge-properties-dialog.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <string.h>

#include "label.h"
#include "merge.h"
#include "combo-util.h"
#include "builder-util.h"
#include "hig.h"

#include "debug.h"


/*===========================================*/
/* Private data types                        */
/*===========================================*/

struct _glMergePropertiesDialogPrivate {

	glLabel      *label;
	glMerge      *merge;

        GtkBuilder   *builder;

	GtkWidget    *type_combo;
	GtkWidget    *location_vbox;
	GtkWidget    *src_entry;

	GtkTreeStore *store;
	GtkWidget    *treeview;

	GtkWidget    *select_all_button;
	GtkWidget    *unselect_all_button;

        GtkWidget    *ok_button;

	gchar        *saved_src;

};

enum {
	/* Real columns */
	SELECT_COLUMN,
	RECORD_FIELD_COLUMN,
	VALUE_COLUMN,

	/* Invisible columns */
	IS_RECORD_COLUMN,
	DATA_COLUMN, /* points to glMergeRecord */

	N_COLUMNS
};


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_merge_properties_dialog_finalize   (GObject                      *object);
static void gl_merge_properties_dialog_construct  (glMergePropertiesDialog      *dialog,
						   glLabel                      *label,
						   GtkWindow                    *window);

static void type_changed_cb                       (GtkWidget                    *widget,
						   glMergePropertiesDialog      *dialog);

static void src_changed_cb                        (GtkWidget                    *widget,
						   glMergePropertiesDialog      *dialog);

static void response_cb                           (glMergePropertiesDialog      *dialog,
					           gint                          response,
						   gpointer                      user_data);

static void load_tree                             (GtkTreeStore                 *store,
					           glMerge                      *merge);

static void record_select_toggled_cb              (GtkCellRendererToggle        *cell,
						   gchar                        *path_str,
						   GtkTreeStore                 *store);

static void select_all_button_clicked_cb          (GtkWidget                    *widget,
						   glMergePropertiesDialog      *dialog);

static void unselect_all_button_clicked_cb        (GtkWidget                    *widget,
						   glMergePropertiesDialog      *dialog);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glMergePropertiesDialog, gl_merge_properties_dialog, GTK_TYPE_DIALOG);


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_merge_properties_dialog_class_init (glMergePropertiesDialogClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	gl_debug (DEBUG_MERGE, "");
	
  	gl_merge_properties_dialog_parent_class = g_type_class_peek_parent (class);

  	object_class->finalize = gl_merge_properties_dialog_finalize;  	
}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_merge_properties_dialog_init (glMergePropertiesDialog *dialog)
{
	GtkBuilder        *builder;
        static gchar      *object_ids[] = { "merge_properties_vbox", NULL };
        GError            *error = NULL;
	GtkWidget         *vbox;
	GtkWidget         *merge_properties_vbox;

	gl_debug (DEBUG_MERGE, "START");

	dialog->priv = g_new0 (glMergePropertiesDialogPrivate, 1);

        vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

 	gtk_container_set_border_width (GTK_CONTAINER(dialog), GL_HIG_PAD1);

	gtk_dialog_set_has_separator (GTK_DIALOG(dialog), FALSE);
	gtk_dialog_add_button (GTK_DIALOG (dialog),
                               GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
	dialog->priv->ok_button = gtk_dialog_add_button (GTK_DIALOG (dialog),
                                                         GTK_STOCK_OK, GTK_RESPONSE_OK);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

        builder = gtk_builder_new ();
        gtk_builder_add_objects_from_file (builder,
                                           GLABELS_BUILDER_DIR "merge-properties-dialog.builder",
                                           object_ids,
                                           &error);
	if (error) {
		g_critical ("%s\n\ngLabels may not be installed correctly!", error->message);
                g_error_free (error);
		return;
	}

        gl_builder_util_get_widgets (builder,
                                     "merge_properties_vbox", &merge_properties_vbox,
                                     "type_combo",            &dialog->priv->type_combo,
                                     "location_vbox",         &dialog->priv->location_vbox,
                                     "treeview",              &dialog->priv->treeview,
                                     "select_all_button",     &dialog->priv->select_all_button,
                                     "unselect_all_button",   &dialog->priv->unselect_all_button,
                                     NULL);

	gtk_container_add (GTK_CONTAINER (vbox), merge_properties_vbox);
        dialog->priv->builder = builder;

	gl_combo_util_add_text_model (GTK_COMBO_BOX (dialog->priv->type_combo));


	gl_debug (DEBUG_MERGE, "END");
}


/*****************************************************************************/
/* Finalize Function.                                                        */
/*****************************************************************************/
static void 
gl_merge_properties_dialog_finalize (GObject *object)
{
	glMergePropertiesDialog* dialog = GL_MERGE_PROPERTIES_DIALOG (object);
	
	gl_debug (DEBUG_MERGE, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_MERGE_PROPERTIES_DIALOG (dialog));
	g_return_if_fail (dialog->priv != NULL);

	if (dialog->priv->merge != NULL) {
		g_object_unref (G_OBJECT (dialog->priv->merge));
	}
	if (dialog->priv->builder != NULL) {
		g_object_unref (G_OBJECT (dialog->priv->builder));
	}
	g_free (dialog->priv);

	G_OBJECT_CLASS (gl_merge_properties_dialog_parent_class)->finalize (object);
	gl_debug (DEBUG_MERGE, "END");
}


/*****************************************************************************/
/* NEW merge properties dialog.                                              */
/*****************************************************************************/
GtkWidget*
gl_merge_properties_dialog_new (glLabel   *label,
	                        GtkWindow *window)
{
	GtkWidget *dialog;

	gl_debug (DEBUG_MERGE, "START");

	dialog = GTK_WIDGET (g_object_new (GL_TYPE_MERGE_PROPERTIES_DIALOG, NULL));

	gl_merge_properties_dialog_construct (GL_MERGE_PROPERTIES_DIALOG (dialog),
					      label, window);

	gl_debug (DEBUG_MERGE, "END");

	return dialog;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Create merge widgets.                                          */
/*--------------------------------------------------------------------------*/
static void
gl_merge_properties_dialog_construct (glMergePropertiesDialog *dialog,
				      glLabel                 *label,
				      GtkWindow               *window)
{
	gchar             *description;
	glMergeSrcType     src_type;
	gchar             *src;
	gchar             *name, *title;
	GList             *texts;
	GtkCellRenderer   *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection  *selection;

	gl_debug (DEBUG_MERGE, "START");

	g_return_if_fail (GL_IS_MERGE_PROPERTIES_DIALOG (dialog));
	g_return_if_fail (dialog->priv != NULL);

	if (window) {
		gtk_window_set_transient_for (GTK_WINDOW(dialog), GTK_WINDOW(window));
		gtk_window_set_destroy_with_parent (GTK_WINDOW(dialog), TRUE);
	}

	dialog->priv->label = label;

	dialog->priv->merge = gl_label_get_merge (dialog->priv->label);
	description         = gl_merge_get_description (dialog->priv->merge);
	src_type            = gl_merge_get_src_type (dialog->priv->merge);
	src                 = gl_merge_get_src (dialog->priv->merge);

	/* --- Window title --- */
	name = gl_label_get_short_name (label);
	title = g_strdup_printf ("%s %s", name, _("Merge Properties"));
	gtk_window_set_title (GTK_WINDOW (dialog), title);
	g_free (name);
	g_free (title);

	texts = gl_merge_get_descriptions ();
	gl_debug (DEBUG_MERGE, "DESCRIPTIONS:");
	{
		GList *p;

		for (p=texts; p!=NULL; p=p->next) {
			gl_debug (DEBUG_MERGE, "    \"%s\"", p->data);
		}
	}
	gl_combo_util_set_strings (GTK_COMBO_BOX (dialog->priv->type_combo),
                                   texts);
	gl_merge_free_descriptions (&texts);
	gl_combo_util_set_active_text (GTK_COMBO_BOX (dialog->priv->type_combo),
                                       description);
	g_signal_connect (G_OBJECT (dialog->priv->type_combo), "changed",
			  G_CALLBACK (type_changed_cb), dialog);

	gl_debug (DEBUG_MERGE, "Src_type = %d", src_type);
	switch (src_type) {
	case GL_MERGE_SRC_IS_FILE:
		dialog->priv->src_entry =
			gtk_file_chooser_button_new (_("Select merge-database source"),
						     GTK_FILE_CHOOSER_ACTION_OPEN);
		gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog->priv->src_entry),
					       src);
                gtk_file_chooser_button_set_focus_on_click (GTK_FILE_CHOOSER_BUTTON (dialog->priv->src_entry),
                                                            FALSE);
		g_signal_connect (G_OBJECT (dialog->priv->src_entry),
				  "selection-changed",
				  G_CALLBACK (src_changed_cb), dialog);
		break;
	default:
		dialog->priv->src_entry = gtk_label_new (_("N/A"));
		gtk_misc_set_alignment (GTK_MISC (dialog->priv->src_entry), 0.0, 0.5);
		break;
	}
	gtk_box_pack_start (GTK_BOX (dialog->priv->location_vbox),
			    dialog->priv->src_entry, FALSE, FALSE, 0);
	gtk_widget_show_all (GTK_WIDGET (dialog->priv->location_vbox));

	dialog->priv->store = gtk_tree_store_new (N_COLUMNS,
					  G_TYPE_BOOLEAN, /* Record selector */
					  G_TYPE_STRING,  /* Record/Field name */
					  G_TYPE_STRING,  /* Field value */
					  G_TYPE_BOOLEAN, /* Is Record? */
					  G_TYPE_POINTER  /* Pointer to record */);
	load_tree (dialog->priv->store, dialog->priv->merge);

	gtk_tree_view_set_model (GTK_TREE_VIEW (dialog->priv->treeview),
				 GTK_TREE_MODEL (dialog->priv->store));

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (dialog->priv->treeview),
				      TRUE);
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->priv->treeview));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_NONE);
	renderer = gtk_cell_renderer_toggle_new ();
	g_signal_connect (G_OBJECT (renderer), "toggled",
			  G_CALLBACK (record_select_toggled_cb), dialog->priv->store);
	column = gtk_tree_view_column_new_with_attributes (_("Select"), renderer,
							   "active", SELECT_COLUMN,
							   "visible", IS_RECORD_COLUMN,
							   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (dialog->priv->treeview), column);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer), "yalign", 0.0, NULL);
	column = gtk_tree_view_column_new_with_attributes (_("Record/Field"), renderer,
							   "text", RECORD_FIELD_COLUMN,
							   NULL);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (dialog->priv->treeview), column);
	gtk_tree_view_set_expander_column (GTK_TREE_VIEW (dialog->priv->treeview), column);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer), "yalign", 0.0, NULL);
	column = gtk_tree_view_column_new_with_attributes (_("Data"), renderer,
							   "text", VALUE_COLUMN,
							   NULL);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (dialog->priv->treeview), column);

	g_signal_connect (G_OBJECT (dialog->priv->select_all_button),
			  "clicked",
			  G_CALLBACK (select_all_button_clicked_cb), dialog);

	g_signal_connect (G_OBJECT (dialog->priv->unselect_all_button),
			  "clicked",
			  G_CALLBACK (unselect_all_button_clicked_cb), dialog);


	g_free (src);
	g_free (description);

        gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

        gtk_widget_grab_focus (dialog->priv->ok_button);
        gtk_widget_grab_default (dialog->priv->ok_button);

	g_signal_connect(G_OBJECT (dialog), "response",
			 G_CALLBACK (response_cb), NULL);

	gl_debug (DEBUG_MERGE, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  type "changed" callback.                                       */
/*--------------------------------------------------------------------------*/
static void
type_changed_cb (GtkWidget               *widget,
		 glMergePropertiesDialog *dialog)
{
	gchar             *description;
	gchar             *name;
	gchar             *src;
	glMergeSrcType     src_type;

	gl_debug (DEBUG_MERGE, "START");

	description = gtk_combo_box_get_active_text (GTK_COMBO_BOX (dialog->priv->type_combo));
	name = gl_merge_description_to_name (description);

	src = gl_merge_get_src (dialog->priv->merge); /* keep current src if possible */
	if ( src != NULL ) {
		gl_debug (DEBUG_MERGE, "Saving src = \"%s\"", src);
		g_free (dialog->priv->saved_src);
		dialog->priv->saved_src = src;
	}

	if (dialog->priv->merge != NULL) {
		g_object_unref (G_OBJECT(dialog->priv->merge));
	}
	dialog->priv->merge = gl_merge_new (name);

	gtk_widget_destroy (dialog->priv->src_entry);
	src_type = gl_merge_get_src_type (dialog->priv->merge);
	switch (src_type) {
	case GL_MERGE_SRC_IS_FILE:
		dialog->priv->src_entry =
			gtk_file_chooser_button_new (_("Select merge-database source"),
						     GTK_FILE_CHOOSER_ACTION_OPEN);
                gtk_file_chooser_button_set_focus_on_click (GTK_FILE_CHOOSER_BUTTON (dialog->priv->src_entry),
                                                            FALSE);
		if (dialog->priv->saved_src != NULL) {
			gl_debug (DEBUG_MERGE, "Setting src = \"%s\"", dialog->priv->saved_src);
			gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog->priv->src_entry),
						       dialog->priv->saved_src);
			gl_merge_set_src (dialog->priv->merge, dialog->priv->saved_src);
		}
		g_signal_connect (G_OBJECT (dialog->priv->src_entry),
				  "selection-changed",
				  G_CALLBACK (src_changed_cb), dialog);
		break;
	case GL_MERGE_SRC_IS_FIXED:
		dialog->priv->src_entry = gtk_label_new (_("Fixed"));
		gtk_misc_set_alignment (GTK_MISC (dialog->priv->src_entry), 0.0, 0.5);

		gl_debug (DEBUG_MERGE, "Setting src = \"%s\"", dialog->priv->saved_src);
		gl_merge_set_src (dialog->priv->merge, "Fixed");
		break;
	default:
		dialog->priv->src_entry = gtk_label_new (_("N/A"));
		gtk_misc_set_alignment (GTK_MISC (dialog->priv->src_entry), 0.0, 0.5);
		break;
	}
	gtk_box_pack_start( GTK_BOX (dialog->priv->location_vbox),
			    dialog->priv->src_entry, FALSE, FALSE, 0);
	gtk_widget_show_all (dialog->priv->location_vbox);

	load_tree (dialog->priv->store, dialog->priv->merge);

	g_free (description);
	g_free (name);

	gl_debug (DEBUG_MERGE, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  source "changed" callback.                                     */
/*--------------------------------------------------------------------------*/
static void
src_changed_cb (GtkWidget               *widget,
		glMergePropertiesDialog *dialog)
{
	gchar     *src, *orig_src;

	gl_debug (DEBUG_MERGE, "START");

	orig_src = gl_merge_get_src (dialog->priv->merge);
	src = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog->priv->src_entry));

	gl_debug (DEBUG_MERGE, "orig=\"%s\", new=\"%s\"\n", orig_src, src);

	if (((orig_src == NULL) && (src != NULL)) ||
	    ((orig_src != NULL) && (src == NULL)) ||
	    ((orig_src != NULL) && (src != NULL) && strcmp (src, orig_src)))
	{
		gl_merge_set_src (dialog->priv->merge, src);
		load_tree (dialog->priv->store, dialog->priv->merge);
	}

	g_free (orig_src);
	g_free (src);


	gl_debug (DEBUG_MERGE, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  response callback.                                             */
/*--------------------------------------------------------------------------*/
static void
response_cb (glMergePropertiesDialog *dialog,
	     gint                     response,
	     gpointer                 user_data)
{
	gl_debug (DEBUG_MERGE, "START");

	switch (response) {

	case GTK_RESPONSE_OK:
		gl_label_set_merge (dialog->priv->label, dialog->priv->merge);
		gtk_widget_hide (GTK_WIDGET (dialog));
		break;
	case GTK_RESPONSE_CANCEL:
		/* Let the dialog get rebuilt next time to recover state. */
		gtk_widget_destroy (GTK_WIDGET (dialog));
		break;
	case GTK_RESPONSE_DELETE_EVENT:
		break;
	default:
		g_print ("response = %d", response);
		g_assert_not_reached ();
	}

	gl_debug (DEBUG_MERGE, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Load tree store from merge data.                               */
/*--------------------------------------------------------------------------*/
static void
load_tree (GtkTreeStore           *store,
	   glMerge                *merge)
{
	const GList   *record_list;
	GList         *p_rec, *p_field;
	glMergeRecord *record;
	glMergeField  *field;
	GtkTreeIter    iter1, iter2;
	gchar         *primary_key;
	gchar         *primary_value;

	gl_debug (DEBUG_MERGE, "START");

	gtk_tree_store_clear (store);

	primary_key = gl_merge_get_primary_key (merge);
	record_list = gl_merge_get_record_list (merge);

	for ( p_rec=(GList *)record_list; p_rec!=NULL; p_rec=p_rec->next ) {
		record = (glMergeRecord *)p_rec->data;
		
		primary_value = gl_merge_eval_key (record, primary_key);

		gtk_tree_store_append (store, &iter1, NULL);
		gtk_tree_store_set (store, &iter1,
				    SELECT_COLUMN,       record->select_flag,
				    RECORD_FIELD_COLUMN, primary_value,
				    IS_RECORD_COLUMN,    TRUE,
				    DATA_COLUMN,         record,
				    -1);

		g_free (primary_value);

		for ( p_field=record->field_list; p_field!=NULL; p_field=p_field->next ) {
			field = (glMergeField *)p_field->data;

			gtk_tree_store_append (store, &iter2, &iter1);
			gtk_tree_store_set (store, &iter2,
					    RECORD_FIELD_COLUMN, field->key,
					    VALUE_COLUMN,        field->value,
					    IS_RECORD_COLUMN,    FALSE,
					    -1);
		}
	}

	g_free (primary_key);

	gl_debug (DEBUG_MERGE, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Record select toggled.                                         */
/*--------------------------------------------------------------------------*/
static void
record_select_toggled_cb (GtkCellRendererToggle *cell,
			  gchar                 *path_str,
			  GtkTreeStore          *store)
{
	GtkTreePath   *path;
	GtkTreeIter    iter;
	glMergeRecord *record;

	gl_debug (DEBUG_MERGE, "START");

	/* get toggled iter */
	path = gtk_tree_path_new_from_string (path_str);
	gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path);

	/* get current data */
	gtk_tree_model_get (GTK_TREE_MODEL (store), &iter,
			    DATA_COLUMN,   &record,
			    -1);

	/* toggle the select flag within the record */
	record->select_flag ^= 1;

	/* set new value in store */
	gtk_tree_store_set (store, &iter,
			    SELECT_COLUMN, record->select_flag,
			    -1);

	/* clean up */
	gtk_tree_path_free (path);

	gl_debug (DEBUG_MERGE, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  "Select All" button callback.                                  */
/*--------------------------------------------------------------------------*/
static void
select_all_button_clicked_cb (GtkWidget                    *widget,
			      glMergePropertiesDialog      *dialog)
{
	GtkTreeModel  *store = GTK_TREE_MODEL (dialog->priv->store);
	GtkTreeIter    iter;
	glMergeRecord *record;
	gboolean       good;

	gl_debug (DEBUG_MERGE, "START");

	for ( good = gtk_tree_model_get_iter_first (store, &iter);
	      good;
	      good = gtk_tree_model_iter_next (store, &iter) )
	{
		
		/* get current data */
		gtk_tree_model_get (GTK_TREE_MODEL (store), &iter,
				    DATA_COLUMN,   &record,
				    -1);

		
		/* Set select flag within the record */
		record->select_flag = TRUE;

		/* set new value in store */
		gtk_tree_store_set (GTK_TREE_STORE (store), &iter,
				    SELECT_COLUMN, record->select_flag,
				    -1);

	}

	gl_debug (DEBUG_MERGE, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  "Unselect All" button callback.                                */
/*--------------------------------------------------------------------------*/
static void
unselect_all_button_clicked_cb (GtkWidget                    *widget,
				glMergePropertiesDialog      *dialog)
{
	GtkTreeModel  *store = GTK_TREE_MODEL (dialog->priv->store);
	GtkTreeIter    iter;
	glMergeRecord *record;
	gboolean       good;

	gl_debug (DEBUG_MERGE, "START");

	for ( good = gtk_tree_model_get_iter_first (store, &iter);
	      good;
	      good = gtk_tree_model_iter_next (store, &iter) )
	{
		
		/* get current data */
		gtk_tree_model_get (GTK_TREE_MODEL (store), &iter,
				    DATA_COLUMN,   &record,
				    -1);

		
		/* Set select flag within the record */
		record->select_flag = FALSE;

		/* set new value in store */
		gtk_tree_store_set (GTK_TREE_STORE (store), &iter,
				    SELECT_COLUMN, record->select_flag,
				    -1);

	}

	gl_debug (DEBUG_MERGE, "END");
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
