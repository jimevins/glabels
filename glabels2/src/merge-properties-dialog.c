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

#include <glib/gi18n.h>

#include "view.h"
#include "merge.h"
#include "merge-properties-dialog.h"
#include "hig.h"

#include "debug.h"

/*===========================================*/
/* Private data types                        */
/*===========================================*/

struct _glMergePropertiesDialogPrivate {

	glView       *view;
	glLabel      *label;
	glMerge      *merge;

	GtkWidget    *type_entry;
	GtkWidget    *src_entry_holder;
	GtkWidget    *src_entry;

	GtkTreeStore *store;
	GtkWidget    *tree;

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

static glHigDialogClass* parent_class = NULL;

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_merge_properties_dialog_class_init (glMergePropertiesDialogClass *klass);
static void gl_merge_properties_dialog_init       (glMergePropertiesDialog      *dlg);
static void gl_merge_properties_dialog_finalize   (GObject                      *object);
static void gl_merge_properties_dialog_construct  (glMergePropertiesDialog      *dialog,
						   glView                       *view);

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



/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_merge_properties_dialog_get_type (void)
{
	static GType type = 0;

	if (!type)
    	{
      		static const GTypeInfo info =
      		{
			sizeof (glMergePropertiesDialogClass),
        		NULL,		/* base_init */
        		NULL,		/* base_finalize */
        		(GClassInitFunc) gl_merge_properties_dialog_class_init,
        		NULL,           /* class_finalize */
        		NULL,           /* class_data */
        		sizeof (glMergePropertiesDialog),
        		0,              /* n_preallocs */
        		(GInstanceInitFunc) gl_merge_properties_dialog_init,
			NULL
      		};

     		type = g_type_register_static (GL_TYPE_HIG_DIALOG,
					       "glMergePropertiesDialog", &info, 0);
    	}

	return type;
}

static void
gl_merge_properties_dialog_class_init (glMergePropertiesDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	gl_debug (DEBUG_MERGE, "");
	
  	parent_class = g_type_class_peek_parent (klass);

  	object_class->finalize = gl_merge_properties_dialog_finalize;  	
}

static void
gl_merge_properties_dialog_init (glMergePropertiesDialog *dialog)
{
	gl_debug (DEBUG_MERGE, "");

	dialog->private = g_new0 (glMergePropertiesDialogPrivate, 1);

	gtk_dialog_add_buttons (GTK_DIALOG(dialog),
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OK, GTK_RESPONSE_OK,
				NULL);

	g_signal_connect (G_OBJECT(dialog), "response",
			  G_CALLBACK(response_cb), NULL);

}

static void 
gl_merge_properties_dialog_finalize (GObject *object)
{
	glMergePropertiesDialog* dialog;
	
	gl_debug (DEBUG_MERGE, "");

	g_return_if_fail (object != NULL);
	
   	dialog = GL_MERGE_PROPERTIES_DIALOG (object);

	g_return_if_fail (GL_IS_MERGE_PROPERTIES_DIALOG (dialog));
	g_return_if_fail (dialog->private != NULL);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	if (dialog->private->merge != NULL) {
		g_object_unref (G_OBJECT(dialog->private->merge));
	}

	g_free (dialog->private);
}

/*****************************************************************************/
/* NEW merge properties dialog.                                              */
/*****************************************************************************/
GtkWidget*
gl_merge_properties_dialog_new (glView *view)
{
	GtkWidget *dialog;

	gl_debug (DEBUG_MERGE, "");

	dialog = GTK_WIDGET (g_object_new (GL_TYPE_MERGE_PROPERTIES_DIALOG, NULL));

	gl_merge_properties_dialog_construct (GL_MERGE_PROPERTIES_DIALOG(dialog),
					      view);

	return dialog;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Create merge widgets.                                          */
/*--------------------------------------------------------------------------*/
static void
gl_merge_properties_dialog_construct (glMergePropertiesDialog *dialog,
				      glView                  *view)
{
	GtkWidget         *wframe, *whbox, *wtable, *wlabel, *wcombo, *wscroll, *wentry;
	GList             *texts;
	gchar             *description;
	glMergeSrcType     src_type;
	gchar             *src;
	GtkSizeGroup      *label_size_group;
	GtkCellRenderer   *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection  *selection;
	gchar             *name, *title;

	gl_debug (DEBUG_MERGE, "START");

	dialog->private->view  = view;
	dialog->private->label = view->label;

	dialog->private->merge = gl_label_get_merge (dialog->private->label);
	description            = gl_merge_get_description (dialog->private->merge);
	src_type               = gl_merge_get_src_type (dialog->private->merge);
	src                    = gl_merge_get_src (dialog->private->merge);

	/* --- Window title --- */
	name = gl_label_get_short_name (view->label);
	title = g_strdup_printf ("%s %s", name, _("Merge Properties"));
	gtk_window_set_title (GTK_WINDOW (dialog), title);
	g_free (name);
	g_free (title);

	/* ---- Source section ---- */
	wframe = gl_hig_category_new (_("Source"));
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dialog), wframe);
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
	gl_debug (DEBUG_MERGE, "DESCRIPTIONS:");
	{
		GList *p;

		for (p=texts; p!=NULL; p=p->next) {
			gl_debug (DEBUG_MERGE, "    \"%s\"", p->data);
		}
	}
	gtk_combo_set_popdown_strings (GTK_COMBO (wcombo), texts);
	gl_merge_free_descriptions (&texts);
	dialog->private->type_entry = GTK_COMBO (wcombo)->entry;
	gtk_entry_set_editable (GTK_ENTRY (dialog->private->type_entry), FALSE);
	gtk_entry_set_text (GTK_ENTRY (dialog->private->type_entry), description);
	gl_hig_hbox_add_widget_justify (GL_HIG_HBOX(whbox), wcombo);
	g_signal_connect (G_OBJECT (dialog->private->type_entry), "changed",
			  G_CALLBACK (type_changed_cb), dialog);

	whbox = gl_hig_hbox_new();
	gl_hig_category_add_widget (GL_HIG_CATEGORY (wframe), whbox);

	/* Location line */
	wlabel = gtk_label_new (_("Location:"));
	gtk_size_group_add_widget (label_size_group, wlabel);
	gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), wlabel);

	gl_debug (DEBUG_MERGE, "Src_type = %d", src_type);
	switch (src_type) {
	case GL_MERGE_SRC_IS_FILE:
		dialog->private->src_entry =
			gnome_file_entry_new (NULL, _("Select merge-database source"));
		wentry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY(dialog->private->src_entry));
		gtk_entry_set_text (GTK_ENTRY(wentry), src);
		g_signal_connect (G_OBJECT (wentry), "changed",
				  G_CALLBACK (src_changed_cb), dialog);
		break;
	default:
		dialog->private->src_entry = gtk_label_new (_("N/A"));
		gtk_misc_set_alignment (GTK_MISC (dialog->private->src_entry), 0.0, 0.5);
		break;
	}
	dialog->private->src_entry_holder = gtk_hbox_new (FALSE, 0);
	gtk_container_add( GTK_CONTAINER(dialog->private->src_entry_holder),
			   dialog->private->src_entry);
	gl_hig_hbox_add_widget_justify (GL_HIG_HBOX(whbox),
					dialog->private->src_entry_holder);

	/* ---- Sample Fields section ---- */
	wframe = gl_hig_category_new (_("Record selection/preview:"));
	gl_hig_dialog_add_widget (GL_HIG_DIALOG(dialog), wframe);

	wscroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_set_border_width (GTK_CONTAINER (wscroll), 5);
	gtk_widget_set_usize (wscroll, 500, 350);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (wscroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gl_hig_category_add_widget (GL_HIG_CATEGORY (wframe), wscroll);

	dialog->private->store = gtk_tree_store_new (N_COLUMNS,
					  G_TYPE_BOOLEAN, /* Record selector */
					  G_TYPE_STRING,  /* Record/Field name */
					  G_TYPE_STRING,  /* Field value */
					  G_TYPE_BOOLEAN, /* Is Record? */
					  G_TYPE_POINTER  /* Pointer to record */);
	load_tree (dialog->private->store, dialog->private->merge);
	dialog->private->tree =
		gtk_tree_view_new_with_model (GTK_TREE_MODEL(dialog->private->store));
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW(dialog->private->tree), TRUE);
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(dialog->private->tree));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_NONE);
	renderer = gtk_cell_renderer_toggle_new ();
	g_signal_connect (G_OBJECT (renderer), "toggled",
			  G_CALLBACK (record_select_toggled_cb), dialog->private->store);
	column = gtk_tree_view_column_new_with_attributes (_("Select"), renderer,
							   "active", SELECT_COLUMN,
							   "visible", IS_RECORD_COLUMN,
							   NULL);
	gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(dialog->private->tree), column);
	renderer = gtk_cell_renderer_text_new ();
	gtk_object_set (GTK_OBJECT(renderer), "yalign", 0.0, NULL);
	column = gtk_tree_view_column_new_with_attributes (_("Record/Field"), renderer,
							   "text", RECORD_FIELD_COLUMN,
							   NULL);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(dialog->private->tree), column);
	gtk_tree_view_set_expander_column (GTK_TREE_VIEW(dialog->private->tree), column);
	renderer = gtk_cell_renderer_text_new ();
	gtk_object_set (GTK_OBJECT(renderer), "yalign", 0.0, NULL);
	column = gtk_tree_view_column_new_with_attributes (_("Data"), renderer,
							   "text", VALUE_COLUMN,
							   NULL);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(dialog->private->tree), column);
	gtk_container_add (GTK_CONTAINER (wscroll), dialog->private->tree);
	
	g_free (src);
	g_free (description);

        gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

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
	GtkWidget         *wentry;

	gl_debug (DEBUG_MERGE, "START");

	description = gtk_editable_get_chars (GTK_EDITABLE (dialog->private->type_entry),
					      0, -1);
	name = gl_merge_description_to_name (description);

	src = gl_merge_get_src (dialog->private->merge); /* keep current src if possible */
	if ( src != NULL ) {
		gl_debug (DEBUG_MERGE, "Saving src = \"%s\"", src);
		g_free (dialog->private->saved_src);
		dialog->private->saved_src = src;
	}

	if (dialog->private->merge != NULL) {
		g_object_unref (G_OBJECT(dialog->private->merge));
	}
	dialog->private->merge = gl_merge_new (name);

	gtk_widget_destroy (dialog->private->src_entry);
	src_type = gl_merge_get_src_type (dialog->private->merge);
	switch (src_type) {
	case GL_MERGE_SRC_IS_FILE:
		dialog->private->src_entry =
			gnome_file_entry_new (NULL, _("Select merge-database source"));
		wentry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY(dialog->private->src_entry));
		if (dialog->private->saved_src != NULL) {
			gl_debug (DEBUG_MERGE, "Setting src = \"%s\"", dialog->private->saved_src);
			gtk_entry_set_text (GTK_ENTRY(wentry), dialog->private->saved_src);
			gl_merge_set_src (dialog->private->merge, dialog->private->saved_src);
		}
		g_signal_connect (G_OBJECT (wentry), "changed",
				  G_CALLBACK (src_changed_cb), dialog);
		break;
	default:
		dialog->private->src_entry = gtk_label_new (_("N/A"));
		gtk_misc_set_alignment (GTK_MISC (dialog->private->src_entry), 0.0, 0.5);
		break;
	}
	gtk_container_add( GTK_CONTAINER(dialog->private->src_entry_holder), dialog->private->src_entry);
	gtk_widget_show (dialog->private->src_entry);

	load_tree (dialog->private->store, dialog->private->merge);

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
	gchar     *src;
	GtkWidget *wentry;

	gl_debug (DEBUG_MERGE, "START");

	wentry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY(dialog->private->src_entry));
	src = gtk_editable_get_chars (GTK_EDITABLE (wentry), 0, -1);
	gl_merge_set_src (dialog->private->merge, src);
	g_free (src);

	load_tree (dialog->private->store, dialog->private->merge);

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

		gl_label_set_merge (dialog->private->label, dialog->private->merge);
		break;
	}

	gtk_widget_destroy (GTK_WIDGET(dialog));

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

  /* get toggled iter */
  path = gtk_tree_path_new_from_string (path_str);
  gtk_tree_model_get_iter (GTK_TREE_MODEL(store), &iter, path);

  /* get current data */
  gtk_tree_model_get (GTK_TREE_MODEL(store), &iter,
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
}

