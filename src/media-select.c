/*
 *  media-select.c
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

#include "media-select.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <string.h>

#include <libglabels.h>
#include "mini-preview-pixbuf-cache.h"
#include "prefs.h"
#include "message-bar.h"
#include "template-history.h"
#include "template-designer.h"
#include "str-util.h"
#include "combo-util.h"
#include "builder-util.h"
#include "color.h"
#include "marshal.h"

#include "debug.h"


#define HISTORY_SIZE 5

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	NAME_COLUMN,
	PREVIEW_COLUMN,
	PREVIEW_COLUMN_STOCK,
	PREVIEW_COLUMN_STOCK_SIZE,
	DESCRIPTION_COLUMN,
	N_COLUMNS
};

struct _glMediaSelectPrivate {

        gulong        db_notify_id;

        GtkBuilder   *builder;

        GtkWidget    *notebook;
        guint         current_page_num;

        gint          recent_page_num;
        GtkWidget    *recent_tab_vbox;
        GtkWidget    *recent_info_vbox;
        GtkWidget    *recent_info_bar;
        GtkWidget    *recent_treeview;
        GtkListStore *recent_store;

        gint          search_all_page_num;
        GtkWidget    *search_all_tab_vbox;
        GtkWidget    *search_all_info_vbox;
        GtkWidget    *search_all_info_bar;
        GtkWidget    *brand_combo_hbox;
        GtkWidget    *brand_combo;
        GtkWidget    *page_size_combo_hbox;
        GtkWidget    *page_size_combo;
        GtkWidget    *category_combo_hbox;
        GtkWidget    *category_combo;
        GtkWidget    *search_all_treeview;
        GtkListStore *search_all_store;

        gint          custom_page_num;
        GtkWidget    *custom_tab_vbox;
        GtkWidget    *custom_info_vbox;
        GtkWidget    *custom_info_bar;
        GtkWidget    *custom_treeview;
        GtkListStore *custom_store;
        GtkWidget    *custom_add_button;
        GtkWidget    *custom_edit_button;
        GtkWidget    *custom_delete_button;

        /* Prevent recursion */
	gboolean    stop_signals;
};

enum {
        CHANGED,
        LAST_SIGNAL
};


/*===========================================*/
/* Private globals                           */
/*===========================================*/

static gint signals[LAST_SIGNAL] = { 0 };


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void   gl_media_select_finalize   (GObject                *object);

static void   gl_media_select_construct  (glMediaSelect          *this);

static void   filter_changed_cb          (glMediaSelect          *this);

static void   selection_changed_cb       (GtkTreeSelection       *selection,
                                          gpointer                user_data);

static void   custom_add_clicked_cb      (GtkButton              *button,
                                          gpointer                user_data);

static void   custom_edit_clicked_cb     (GtkButton              *button,
                                          gpointer                user_data);

static void   custom_delete_clicked_cb   (GtkButton              *button,
                                          gpointer                user_data);

static void   page_changed_cb            (GtkNotebook            *notebook,
                                          GtkWidget              *page,
                                          guint                   page_num,
                                          gpointer                user_data);

static void   db_changed_cb              (glMediaSelect          *this);

static void   load_recent_list           (glMediaSelect          *this,
                                          GtkListStore           *store,
                                          GtkTreeSelection       *selection,
                                          GList                  *list);

static void   load_custom_list          (glMediaSelect          *this,
                                          GtkListStore           *store,
                                          GtkTreeSelection       *selection,
                                          GList                  *list);

static void   load_search_all_list       (glMediaSelect          *this,
                                          GtkListStore           *store,
                                          GtkTreeSelection       *selection,
                                          GList                  *list);


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glMediaSelect, gl_media_select, GTK_TYPE_VBOX);


static void
gl_media_select_class_init (glMediaSelectClass *class)
{
        GObjectClass *object_class = G_OBJECT_CLASS (class);

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        gl_media_select_parent_class = g_type_class_peek_parent (class);

        object_class->finalize = gl_media_select_finalize;

        signals[CHANGED] =
            g_signal_new ("changed",
                          G_OBJECT_CLASS_TYPE(object_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (glMediaSelectClass, changed),
                          NULL, NULL,
                          gl_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}


static void
gl_media_select_init (glMediaSelect *this)
{
        gl_debug (DEBUG_MEDIA_SELECT, "START");

        this->priv = g_new0 (glMediaSelectPrivate, 1);

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}


static void
gl_media_select_finalize (GObject *object)
{
        glMediaSelect *this = GL_MEDIA_SELECT (object);

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        g_return_if_fail (object != NULL);
        g_return_if_fail (GL_IS_MEDIA_SELECT (object));

        if (this->priv->db_notify_id)
        {
                lgl_db_notify_remove (this->priv->db_notify_id);
        }

        if (this->priv->builder)
        {
                g_object_unref (this->priv->builder);
        }
        g_object_unref (this->priv->recent_store);
        g_object_unref (this->priv->search_all_store);
        g_free (this->priv);

        G_OBJECT_CLASS (gl_media_select_parent_class)->finalize (object);

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}


GtkWidget *
gl_media_select_new (void)
{
        glMediaSelect *this;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        this = g_object_new (gl_media_select_get_type (), NULL);

        gl_media_select_construct (this);

        gl_debug (DEBUG_MEDIA_SELECT, "END");

        return GTK_WIDGET (this);
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                    */
/*--------------------------------------------------------------------------*/
static void
gl_media_select_construct (glMediaSelect *this)
{
        gchar             *builder_filename;
        GtkBuilder        *builder;
        static gchar      *object_ids[] = { "media_select_hbox",
                                            "custom_buttons_sizegroup",
                                            NULL };
        GError            *error = NULL;
        GtkWidget         *hbox;
        GList             *recent_list = NULL;
        GList             *brands = NULL;
        GList             *page_sizes = NULL;
        GList             *categories = NULL;
        GList             *search_all_names = NULL;
        const gchar       *page_size_id;
        gchar             *page_size_name;
        GList             *custom_list = NULL;
        GtkCellRenderer   *renderer;
        GtkTreeViewColumn *column;
        GtkTreeSelection  *recent_selection;
        GtkTreeSelection  *search_all_selection;
        GtkTreeSelection  *custom_selection;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        g_return_if_fail (GL_IS_MEDIA_SELECT (this));
        g_return_if_fail (this->priv != NULL);

        builder = gtk_builder_new ();
        builder_filename = g_build_filename (GLABELS_DATA_DIR, "ui", "media-select.ui", NULL);
        gtk_builder_add_objects_from_file (builder, builder_filename, object_ids, &error);
        g_free (builder_filename);
	if (error) {
		g_critical ("%s\n\ngLabels may not be installed correctly!", error->message);
                g_error_free (error);
		return;
	}

        gl_builder_util_get_widgets (builder,
                                     "media_select_hbox",      &hbox,
                                     "notebook",               &this->priv->notebook,
                                     "recent_tab_vbox",        &this->priv->recent_tab_vbox,
                                     "recent_info_vbox",       &this->priv->recent_info_vbox,
                                     "recent_treeview",        &this->priv->recent_treeview,
                                     "search_all_tab_vbox",    &this->priv->search_all_tab_vbox,
                                     "brand_combo_hbox",       &this->priv->brand_combo_hbox,
                                     "page_size_combo_hbox",   &this->priv->page_size_combo_hbox,
                                     "category_combo_hbox",    &this->priv->category_combo_hbox,
                                     "search_all_info_vbox",   &this->priv->search_all_info_vbox,
                                     "search_all_treeview",    &this->priv->search_all_treeview,
                                     "custom_tab_vbox",        &this->priv->custom_tab_vbox,
                                     "custom_info_vbox",       &this->priv->custom_info_vbox,
                                     "custom_treeview",        &this->priv->custom_treeview,
                                     "custom_add_button",      &this->priv->custom_add_button,
                                     "custom_edit_button",     &this->priv->custom_edit_button,
                                     "custom_delete_button",   &this->priv->custom_delete_button,
                                     NULL);

        gtk_container_add (GTK_CONTAINER (this), hbox);
        this->priv->builder = builder;

        this->priv->recent_page_num =
                gtk_notebook_page_num (GTK_NOTEBOOK (this->priv->notebook),
                                       this->priv->recent_tab_vbox);
        this->priv->search_all_page_num =
                gtk_notebook_page_num (GTK_NOTEBOOK (this->priv->notebook),
                                       this->priv->search_all_tab_vbox);
        this->priv->custom_page_num =
                gtk_notebook_page_num (GTK_NOTEBOOK (this->priv->notebook),
                                       this->priv->custom_tab_vbox);

        this->priv->brand_combo = gtk_combo_box_text_new ();
        gtk_box_pack_start (GTK_BOX (this->priv->brand_combo_hbox), this->priv->brand_combo, FALSE, FALSE, 0);
        this->priv->page_size_combo = gtk_combo_box_text_new ();
        gtk_box_pack_start (GTK_BOX (this->priv->page_size_combo_hbox), this->priv->page_size_combo, FALSE, FALSE, 0);
        this->priv->category_combo = gtk_combo_box_text_new ();
        gtk_box_pack_start (GTK_BOX (this->priv->category_combo_hbox), this->priv->category_combo, FALSE, FALSE, 0);

        gtk_widget_show_all (GTK_WIDGET (this));

        /* Recent templates treeview */
        this->priv->recent_store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING);
        gtk_tree_view_set_model (GTK_TREE_VIEW (this->priv->recent_treeview),
                                 GTK_TREE_MODEL (this->priv->recent_store));
        renderer = gtk_cell_renderer_pixbuf_new ();
        column = gtk_tree_view_column_new_with_attributes ("", renderer,
                                                           "pixbuf", PREVIEW_COLUMN,
                                                           "stock-id", PREVIEW_COLUMN_STOCK,
                                                           "stock-size", PREVIEW_COLUMN_STOCK_SIZE,
                                                           NULL);
        gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_append_column (GTK_TREE_VIEW (this->priv->recent_treeview), column);
        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes ("", renderer,
                                                           "markup", DESCRIPTION_COLUMN,
                                                           NULL);
        gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_append_column (GTK_TREE_VIEW (this->priv->recent_treeview), column);
        recent_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (this->priv->recent_treeview));
        recent_list = gl_template_history_model_get_name_list (gl_template_history);
        load_recent_list (this, this->priv->recent_store, recent_selection, recent_list);

        page_size_id = gl_prefs_model_get_default_page_size (gl_prefs);
        page_size_name = lgl_db_lookup_paper_name_from_id (page_size_id);

        /* Brand selection control */
        brands = lgl_db_get_brand_list (NULL, NULL);
        brands = g_list_prepend (brands, g_strdup (C_("Brand", "Any")));
        gl_combo_util_set_strings (GTK_COMBO_BOX_TEXT (this->priv->brand_combo), brands);
        lgl_db_free_brand_list (brands);
        gl_combo_util_set_active_text (GTK_COMBO_BOX (this->priv->brand_combo), C_("Brand", "Any"));

        /* Page size selection control */
        page_sizes = lgl_db_get_paper_name_list ();
        page_sizes = g_list_prepend (page_sizes, g_strdup (C_("Page size", "Any")));
        gl_combo_util_set_strings (GTK_COMBO_BOX_TEXT (this->priv->page_size_combo), page_sizes);
        lgl_db_free_paper_name_list (page_sizes);
        gl_combo_util_set_active_text (GTK_COMBO_BOX (this->priv->page_size_combo), page_size_name);

        /* Category selection control */
        categories = lgl_db_get_category_name_list ();
        categories = g_list_prepend (categories, g_strdup (C_("Category", "Any")));
        gl_combo_util_set_strings (GTK_COMBO_BOX_TEXT (this->priv->category_combo), categories);
        lgl_db_free_category_name_list (categories);
        gl_combo_util_set_active_text (GTK_COMBO_BOX (this->priv->category_combo), C_("Category", "Any"));

        /* Search all treeview */
        this->priv->search_all_store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING);
        gtk_tree_view_set_model (GTK_TREE_VIEW (this->priv->search_all_treeview),
                                 GTK_TREE_MODEL (this->priv->search_all_store));
        renderer = gtk_cell_renderer_pixbuf_new ();
        column = gtk_tree_view_column_new_with_attributes ("", renderer,
                                                           "pixbuf", PREVIEW_COLUMN,
                                                           "stock-id", PREVIEW_COLUMN_STOCK,
                                                           "stock-size", PREVIEW_COLUMN_STOCK_SIZE,
                                                           NULL);
        gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_append_column (GTK_TREE_VIEW (this->priv->search_all_treeview), column);
        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes ("", renderer,
                                                           "markup", DESCRIPTION_COLUMN,
                                                           NULL);
        gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_append_column (GTK_TREE_VIEW (this->priv->search_all_treeview), column);
        search_all_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (this->priv->search_all_treeview));
        search_all_names = lgl_db_get_template_name_list_all (NULL, page_size_id, NULL);
        load_search_all_list (this, this->priv->search_all_store, search_all_selection, search_all_names);
        lgl_db_free_template_name_list (search_all_names);

        /* Custom templates treeview */
        this->priv->custom_store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING);
        gtk_tree_view_set_model (GTK_TREE_VIEW (this->priv->custom_treeview),
                                 GTK_TREE_MODEL (this->priv->custom_store));
        renderer = gtk_cell_renderer_pixbuf_new ();
        column = gtk_tree_view_column_new_with_attributes ("", renderer,
                                                           "pixbuf", PREVIEW_COLUMN,
                                                           "stock-id", PREVIEW_COLUMN_STOCK,
                                                           "stock-size", PREVIEW_COLUMN_STOCK_SIZE,
                                                           NULL);
        gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_append_column (GTK_TREE_VIEW (this->priv->custom_treeview), column);
        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes ("", renderer,
                                                           "markup", DESCRIPTION_COLUMN,
                                                           NULL);
        gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_append_column (GTK_TREE_VIEW (this->priv->custom_treeview), column);
        custom_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (this->priv->custom_treeview));
        custom_list = lgl_db_get_template_name_list_all (NULL, NULL, "user-defined");
        load_custom_list (this, this->priv->custom_store, custom_selection, custom_list);
        lgl_db_free_template_name_list (custom_list);

        page_size_id = gl_prefs_model_get_default_page_size (gl_prefs);
        page_size_name = lgl_db_lookup_paper_name_from_id (page_size_id);

        /* Connect signals to controls */
        g_signal_connect_swapped (G_OBJECT (this->priv->brand_combo), "changed",
                                  G_CALLBACK (filter_changed_cb),
                                  this);
        g_signal_connect_swapped (G_OBJECT (this->priv->page_size_combo), "changed",
                                  G_CALLBACK (filter_changed_cb),
                                  this);
        g_signal_connect_swapped (G_OBJECT (this->priv->category_combo), "changed",
                                  G_CALLBACK (filter_changed_cb),
                                  this);
        g_signal_connect (G_OBJECT (recent_selection), "changed",
                          G_CALLBACK (selection_changed_cb),
                          this);
        g_signal_connect (G_OBJECT (search_all_selection), "changed",
                          G_CALLBACK (selection_changed_cb),
                          this);
        g_signal_connect (G_OBJECT (custom_selection), "changed",
                          G_CALLBACK (selection_changed_cb),
                          this);
        g_signal_connect (G_OBJECT (this->priv->custom_add_button), "clicked",
                          G_CALLBACK (custom_add_clicked_cb),
                          this);
        g_signal_connect (G_OBJECT (this->priv->custom_edit_button), "clicked",
                          G_CALLBACK (custom_edit_clicked_cb),
                          this);
        g_signal_connect (G_OBJECT (this->priv->custom_delete_button), "clicked",
                          G_CALLBACK (custom_delete_clicked_cb),
                          this);
        g_signal_connect (G_OBJECT (this->priv->notebook), "switch-page",
                          G_CALLBACK (page_changed_cb),
                          this);

        g_free (page_size_name);

        if ( recent_list )
        {
                gtk_notebook_set_current_page (GTK_NOTEBOOK (this->priv->notebook),
                                               this->priv->recent_page_num);
        }
        else
        {
                gtk_notebook_set_current_page (GTK_NOTEBOOK (this->priv->notebook),
                                               this->priv->search_all_page_num);
        }
        gl_template_history_model_free_name_list (recent_list);

        this->priv->db_notify_id = lgl_db_notify_add ((lglDbNotifyFunc)db_changed_cb, this);

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change in selection                       */
/*--------------------------------------------------------------------------*/
static void
filter_changed_cb (glMediaSelect *this)
{
        gchar             *brand;
        gchar             *page_size_name, *page_size_id;
        gchar             *category_name, *category_id;
        GList             *search_all_names;
        GtkTreeSelection  *selection;

        gl_debug (DEBUG_MEDIA_SELECT, "START");


	this->priv->stop_signals = TRUE;

        /* Update template selections for new filter settings */
        brand = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (this->priv->brand_combo));
        page_size_name = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (this->priv->page_size_combo));
        category_name = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (this->priv->category_combo));
        if ( brand && strlen(brand) &&
             page_size_name && strlen(page_size_name) &&
             category_name && strlen(category_name) )
        {
                gl_debug (DEBUG_MEDIA_SELECT, "brand = \"%s\"", brand);
                gl_debug (DEBUG_MEDIA_SELECT, "page_size_name = \"%s\"", page_size_name);
                gl_debug (DEBUG_MEDIA_SELECT, "category_name = \"%s\"", category_name);
                if (!g_utf8_collate (brand, C_("Brand", "Any")))
                {
                        g_free (brand);
                        brand = NULL;
                }
                page_size_id = lgl_db_lookup_paper_id_from_name (page_size_name);
                category_id = lgl_db_lookup_category_id_from_name (category_name);
                gl_debug (DEBUG_MEDIA_SELECT, "page_size_id = \"%s\"", page_size_id);
                gl_debug (DEBUG_MEDIA_SELECT, "category_id = \"%s\"", category_id);
                search_all_names = lgl_db_get_template_name_list_all (brand, page_size_id, category_id);
                selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (this->priv->search_all_treeview));
                load_search_all_list (this, this->priv->search_all_store, selection, search_all_names);
                lgl_db_free_template_name_list (search_all_names);
                g_free (page_size_id);
                g_free (category_id);

                /* Emit our "changed" signal */
                g_signal_emit (G_OBJECT (this), signals[CHANGED], 0);
        }
        g_free (brand);
        g_free (page_size_name);
        g_free (category_name);


	this->priv->stop_signals = FALSE;

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change in selection                       */
/*--------------------------------------------------------------------------*/
static void
selection_changed_cb (GtkTreeSelection       *selection,
                      gpointer                user_data)
{
        glMediaSelect     *this = GL_MEDIA_SELECT (user_data);
        GtkTreeSelection  *custom_selection;

	if (this->priv->stop_signals) return;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        custom_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (this->priv->custom_treeview));
        gtk_widget_set_sensitive (GTK_WIDGET (this->priv->custom_edit_button),
                                  gtk_tree_selection_get_mode (custom_selection) != GTK_SELECTION_NONE );
        gtk_widget_set_sensitive (GTK_WIDGET (this->priv->custom_delete_button),
                                  gtk_tree_selection_get_mode (custom_selection) != GTK_SELECTION_NONE );

        /* Emit our "changed" signal */
        g_signal_emit (G_OBJECT (user_data), signals[CHANGED], 0);

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Custom add button clicked callback.                            */
/*--------------------------------------------------------------------------*/
static void
custom_add_clicked_cb (GtkButton  *button,
                       gpointer    user_data)
{
        glMediaSelect *this = GL_MEDIA_SELECT (user_data);
        GtkWidget     *window;
        GtkWidget     *dialog;

        window = gtk_widget_get_toplevel (GTK_WIDGET (this));
        
        dialog = gl_template_designer_new (GTK_WINDOW (window));

        gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
        gtk_widget_show (dialog);
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Custom edit button clicked callback.                           */
/*--------------------------------------------------------------------------*/
static void
custom_edit_clicked_cb (GtkButton  *button,
                        gpointer    user_data)
{
        glMediaSelect     *this = GL_MEDIA_SELECT (user_data);
        GtkTreeSelection  *selection;
        GtkTreeIter        iter;
        GtkTreeModel      *model;
        gchar             *name;
        GtkWidget         *window;
        GtkWidget         *dialog;

        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (this->priv->custom_treeview));

        if (!gtk_tree_selection_get_mode (selection) == GTK_SELECTION_NONE)
        {
                gtk_tree_selection_get_selected (selection, &model, &iter);
                gtk_tree_model_get (model, &iter, NAME_COLUMN, &name, -1);

                window = gtk_widget_get_toplevel (GTK_WIDGET (this));

                dialog = gl_template_designer_new (GTK_WINDOW (window));
                gl_template_designer_set_from_name (GL_TEMPLATE_DESIGNER (dialog), name);

                gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
                gtk_widget_show (dialog);

                g_free (name);
        }
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Custom delete button clicked callback.                         */
/*--------------------------------------------------------------------------*/
static void
custom_delete_clicked_cb (GtkButton  *button,
                          gpointer    user_data)
{
        glMediaSelect     *this = GL_MEDIA_SELECT (user_data);
        GtkTreeSelection  *selection;
        GtkTreeIter        iter;
        GtkTreeModel      *model;        
        gchar             *name;
        GtkWidget         *window;
        GtkWidget         *dialog;
        gint               ret;

	this->priv->stop_signals = TRUE;

        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (this->priv->custom_treeview));

        if (!gtk_tree_selection_get_mode (selection) == GTK_SELECTION_NONE)
        {
                gtk_tree_selection_get_selected (selection, &model, &iter);
                gtk_tree_model_get (model, &iter, NAME_COLUMN, &name, -1);

                window = gtk_widget_get_toplevel (GTK_WIDGET (this));
                dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                                 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                 GTK_MESSAGE_QUESTION,
                                                 GTK_BUTTONS_YES_NO,
                                                 _("Delete template \"%s\"?"), name);
                gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                                          _("This action will permanently delete this template."));

                ret = gtk_dialog_run (GTK_DIALOG (dialog));
                gtk_widget_destroy (dialog);
                
                if ( ret == GTK_RESPONSE_YES )
                {
                        lgl_db_delete_template_by_name (name);
                        gl_mini_preview_pixbuf_cache_delete_by_name (name);
                }

                g_free (name);
        }

	this->priv->stop_signals = FALSE;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change in selection                       */
/*--------------------------------------------------------------------------*/
static void
page_changed_cb (GtkNotebook            *notebook,
                 GtkWidget              *page,
                 guint                   page_num,
                 gpointer                user_data)
{
        glMediaSelect *this = GL_MEDIA_SELECT (user_data);

	if (this->priv->stop_signals) return;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        /*
         * Store new current page, because this signal is emitted before the actual page change.
         */
	this->priv->current_page_num = page_num;

        /* Emit our "changed" signal */
        g_signal_emit (G_OBJECT (user_data), signals[CHANGED], 0);

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  DB changed notification callback.                              */
/*--------------------------------------------------------------------------*/
static void
db_changed_cb (glMediaSelect *this)
{
        gchar             *brand;
        gchar             *page_size_name, *page_size_id;
        gchar             *category_name, *category_id;
        GtkTreeSelection  *selection;
        GList             *list;

	this->priv->stop_signals = TRUE;

        /* Update recent page. */
        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (this->priv->recent_treeview));
        list = gl_template_history_model_get_name_list (gl_template_history);
        load_recent_list (this, this->priv->recent_store, selection, list);
        lgl_db_free_template_name_list (list);

        /* Update search all page. */
        brand = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (this->priv->brand_combo));
        page_size_name = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (this->priv->page_size_combo));
        category_name = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (this->priv->category_combo));
        if ( brand && strlen(brand) &&
             page_size_name && strlen(page_size_name) &&
             category_name && strlen(category_name) )
        {
                if (!g_utf8_collate (brand, C_("Brand", "Any")))
                {
                        g_free (brand);
                        brand = NULL;
                }
                page_size_id = lgl_db_lookup_paper_id_from_name (page_size_name);
                category_id = lgl_db_lookup_category_id_from_name (category_name);
                list = lgl_db_get_template_name_list_all (brand, page_size_id, category_id);
                selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (this->priv->search_all_treeview));
                load_search_all_list (this, this->priv->search_all_store, selection, list);
                lgl_db_free_template_name_list (list);
                g_free (page_size_id);
                g_free (category_id);
        }
        g_free (brand);
        g_free (page_size_name);
        g_free (category_name);

        /* Update custom page. */
        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (this->priv->custom_treeview));
        list = lgl_db_get_template_name_list_all (NULL, NULL, "user-defined");
        load_custom_list (this, this->priv->custom_store, selection, list);
        lgl_db_free_template_name_list (list);

	this->priv->stop_signals = FALSE;

        /* Emit our "changed" signal */
        g_signal_emit (G_OBJECT (this), signals[CHANGED], 0);

        filter_changed_cb (this);
}


/****************************************************************************/
/* query selected label template name.                                      */
/****************************************************************************/
gchar *
gl_media_select_get_name (glMediaSelect *this)
{
        gint               page_num;
        GtkTreeSelection  *selection;
        GtkTreeIter        iter;
        GtkTreeModel      *model;        
        gchar             *name;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        page_num = this->priv->current_page_num;
        if (page_num == this->priv->recent_page_num)
        {
                selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (this->priv->recent_treeview));
        }
        else if (page_num == this->priv->search_all_page_num)
        {
                selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (this->priv->search_all_treeview));
        }
        else if (page_num == this->priv->custom_page_num)
        {
                selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (this->priv->custom_treeview));
        }
        else
        {
                g_print ("notebook page = %d\n", page_num);
                g_assert_not_reached ();
        }

        if (gtk_tree_selection_get_mode (selection) == GTK_SELECTION_NONE)
        {
                name = NULL;
        }
        else
        {
                gtk_tree_selection_get_selected (selection, &model, &iter);
                gtk_tree_model_get (model, &iter, NAME_COLUMN, &name, -1);
        }

        gl_debug (DEBUG_MEDIA_SELECT, "END");
        return name;
}


/****************************************************************************/
/* set selected label template name.                                        */
/****************************************************************************/
void
gl_media_select_set_name (glMediaSelect *this,
                          gchar         *name)
{
        GtkTreeSelection  *selection;
        GtkTreeModel      *model;
        GtkTreeIter        iter;
        GtkTreePath       *path;
        gchar             *name_i;
        gboolean           flag;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (this->priv->search_all_treeview));
        g_return_if_fail (GTK_IS_TREE_SELECTION (selection));

        model = GTK_TREE_MODEL (this->priv->search_all_store);

        for ( flag = gtk_tree_model_get_iter_first (model, &iter);
              flag;
              flag = gtk_tree_model_iter_next(model, &iter) )
        {
                gtk_tree_model_get (model, &iter, NAME_COLUMN, &name_i, -1); 
                if (strcasecmp(name, name_i) == 0)
                {
                        gtk_tree_selection_select_iter (selection, &iter);
                        path = gtk_tree_model_get_path (model, &iter);
                        gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (this->priv->search_all_treeview),
                                                      path,
                                                      NULL,
                                                      TRUE, 0.5, 0.0);
                        gtk_tree_path_free (path);
                        break;
                }
        }

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}


/****************************************************************************/
/* query current filter parameters.                                         */
/****************************************************************************/
void
gl_media_select_get_filter_parameters (glMediaSelect  *this,
                                       gchar         **page_size_id,
                                       gchar         **category_id)
{
        gchar *page_size_name, *category_name;

        gl_debug (DEBUG_MEDIA_SELECT, "");

        g_free (*page_size_id);
        g_free (*category_id);

        page_size_name =
                gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (this->priv->page_size_combo));

        *page_size_id = lgl_db_lookup_paper_id_from_name (page_size_name);

        category_name =
                gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (this->priv->category_combo));

        *category_id = lgl_db_lookup_category_id_from_name (category_name);

        g_free (page_size_name);
        g_free (category_name);
}


/****************************************************************************/
/* set filter parameters.                                                   */
/****************************************************************************/
void
gl_media_select_set_filter_parameters (glMediaSelect  *this,
                                       const gchar    *page_size_id,
                                       const gchar    *category_id)
{
        gchar *page_size_name;
        gchar *category_name;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        page_size_name = lgl_db_lookup_paper_name_from_id (page_size_id);
        if (page_size_name == NULL)
        {
                page_size_name = g_strdup (C_("Page size", "Any"));
        }

        gl_combo_util_set_active_text (GTK_COMBO_BOX (this->priv->page_size_combo),
                                       page_size_name);

        category_name = lgl_db_lookup_category_name_from_id (category_id);
        if (category_name == NULL)
        {
                category_name = g_strdup (C_("Category", "Any"));
        }

        gl_combo_util_set_active_text (GTK_COMBO_BOX (this->priv->category_combo),
                                       category_name);
        g_free (page_size_name);
        g_free (category_name);

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Load list store from template name list.                       */
/*--------------------------------------------------------------------------*/
static void
load_recent_list (glMediaSelect      *this,
                  GtkListStore       *store,
                  GtkTreeSelection   *selection,
                  GList              *list)
{
        GList            *p;
        GtkTreeIter       iter;
        lglUnits          units;
        lglTemplate      *template;
        lglTemplateFrame *frame;
        GdkPixbuf        *pixbuf;
        gchar            *size;
        gchar            *layout;
        gchar            *description;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        gtk_list_store_clear (store);


        if ( this->priv->recent_info_bar )
        {
                gtk_container_remove (GTK_CONTAINER (this->priv->recent_info_vbox),
                                      this->priv->recent_info_bar);
                this->priv->recent_info_bar = NULL;
        }

        if (list)
        {

                units = gl_prefs_model_get_units (gl_prefs);

                for ( p=list; p!=NULL; p=p->next )
                {

                        gl_debug (DEBUG_MEDIA_SELECT, "p->data = \"%s\"", p->data);

                        template = lgl_db_lookup_template_from_name (p->data);
                        frame    = (lglTemplateFrame *)template->frames->data;
                        pixbuf   = gl_mini_preview_pixbuf_cache_get_pixbuf (p->data);

                        size     = lgl_template_frame_get_size_description (frame, units);
                        layout   = lgl_template_frame_get_layout_description (frame);
                        description = g_strdup_printf ("<b>%s: %s</b>\n%s\n%s",
                                                       (gchar *)p->data,
                                                       template->description,
                                                       size,
                                                       layout);
                        g_free (size);
                        g_free (layout);

                        lgl_template_free (template);

                        gtk_list_store_append (store, &iter);
                        gtk_list_store_set (store, &iter,
                                            NAME_COLUMN, p->data,
                                            PREVIEW_COLUMN, pixbuf,
                                            DESCRIPTION_COLUMN, description,
                                            -1);

                        g_object_unref (G_OBJECT (pixbuf));
                        g_free (description);
                }

                gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);
                gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter);
                gtk_tree_selection_select_iter (selection, &iter);

        }
        else
        {
                this->priv->recent_info_bar = gl_message_bar_new (GTK_MESSAGE_INFO, GTK_BUTTONS_NONE,
                                                                  "%s", _("No recent templates found."));
                gl_message_bar_format_secondary_text (GL_MESSAGE_BAR (this->priv->recent_info_bar),
                                                      "%s", _("Try selecting a template in the \"Search all\" tab."));

                gtk_box_pack_start (GTK_BOX (this->priv->recent_info_vbox),
                                    this->priv->recent_info_bar,
                                    FALSE, FALSE, 0);
                gtk_widget_show_all (this->priv->recent_info_bar);

                gtk_tree_selection_set_mode (selection, GTK_SELECTION_NONE);

        }

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Load list store from template name list.                       */
/*--------------------------------------------------------------------------*/
static void
load_search_all_list (glMediaSelect      *this,
                      GtkListStore       *store,
                      GtkTreeSelection   *selection,
                      GList              *list)
{
        GList            *p;
        GtkTreeIter       iter;
        lglUnits          units;
        lglTemplate      *template;
        lglTemplateFrame *frame;
        GdkPixbuf        *pixbuf;
        gchar            *size;
        gchar            *layout;
        gchar            *description;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        gtk_list_store_clear (store);

        if ( this->priv->search_all_info_bar )
        {
                gtk_container_remove (GTK_CONTAINER (this->priv->search_all_info_vbox),
                                      this->priv->search_all_info_bar);
                this->priv->search_all_info_bar = NULL;
        }

        if (list)
        {

                units = gl_prefs_model_get_units (gl_prefs);

                for ( p=list; p!=NULL; p=p->next )
                {

                        gl_debug (DEBUG_MEDIA_SELECT, "p->data = \"%s\"", p->data);

                        template = lgl_db_lookup_template_from_name (p->data);
                        frame    = (lglTemplateFrame *)template->frames->data;
                        pixbuf   = gl_mini_preview_pixbuf_cache_get_pixbuf (p->data);

                        size     = lgl_template_frame_get_size_description (frame, units);
                        layout   = lgl_template_frame_get_layout_description (frame);
                        description = g_strdup_printf ("<b>%s: %s</b>\n%s\n%s",
                                                       (gchar *)p->data,
                                                       template->description,
                                                       size,
                                                       layout);
                        g_free (size);
                        g_free (layout);

                        lgl_template_free (template);

                        gtk_list_store_append (store, &iter);
                        gtk_list_store_set (store, &iter,
                                            NAME_COLUMN, p->data,
                                            PREVIEW_COLUMN, pixbuf,
                                            DESCRIPTION_COLUMN, description,
                                            -1);

                        g_object_unref (G_OBJECT (pixbuf));
                        g_free (description);
                }

                gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);
                gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter);
                gtk_tree_selection_select_iter (selection, &iter);

        }
        else
        {
                /* Translators: "No match" means that for the given template search criteria, there
                 * were no matches found. */
                this->priv->search_all_info_bar = gl_message_bar_new (GTK_MESSAGE_INFO, GTK_BUTTONS_NONE, "%s", _("No match."));
                gl_message_bar_format_secondary_text (GL_MESSAGE_BAR (this->priv->search_all_info_bar),
                                                      "%s", _("Try selecting a different brand, page size or category."));

                gtk_box_pack_start (GTK_BOX (this->priv->search_all_info_vbox),
                                    this->priv->search_all_info_bar,
                                    FALSE, FALSE, 0);
                gtk_widget_show_all (this->priv->search_all_info_bar);

                gtk_tree_selection_set_mode (selection, GTK_SELECTION_NONE);

        }

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Load list store from template name list.                       */
/*--------------------------------------------------------------------------*/
static void
load_custom_list (glMediaSelect      *this,
                  GtkListStore       *store,
                  GtkTreeSelection   *selection,
                  GList              *list)
{
        GList            *p;
        GtkTreeIter       iter;
        lglUnits          units;
        lglTemplate      *template;
        lglTemplateFrame *frame;
        GdkPixbuf        *pixbuf;
        gchar            *size;
        gchar            *layout;
        gchar            *description;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        gtk_list_store_clear (store);


        if ( this->priv->custom_info_bar )
        {
                gtk_container_remove (GTK_CONTAINER (this->priv->custom_info_vbox),
                                      this->priv->custom_info_bar);
                this->priv->custom_info_bar = NULL;
        }

        if (list)
        {

                units = gl_prefs_model_get_units (gl_prefs);

                for ( p=list; p!=NULL; p=p->next )
                {

                        gl_debug (DEBUG_MEDIA_SELECT, "p->data = \"%s\"", p->data);

                        template = lgl_db_lookup_template_from_name (p->data);
                        frame    = (lglTemplateFrame *)template->frames->data;
                        pixbuf   = gl_mini_preview_pixbuf_cache_get_pixbuf (p->data);

                        size     = lgl_template_frame_get_size_description (frame, units);
                        layout   = lgl_template_frame_get_layout_description (frame);
                        description = g_strdup_printf ("<b>%s: %s</b>\n%s\n%s",
                                                       (gchar *)p->data,
                                                       template->description,
                                                       size,
                                                       layout);
                        g_free (size);
                        g_free (layout);

                        lgl_template_free (template);

                        gtk_list_store_append (store, &iter);
                        gtk_list_store_set (store, &iter,
                                            NAME_COLUMN, p->data,
                                            PREVIEW_COLUMN, pixbuf,
                                            DESCRIPTION_COLUMN, description,
                                            -1);

                        g_object_unref (G_OBJECT (pixbuf));
                        g_free (description);
                }

                gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);
                gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter);
                gtk_tree_selection_select_iter (selection, &iter);

                gtk_widget_set_sensitive (GTK_WIDGET (this->priv->custom_edit_button), TRUE);
                gtk_widget_set_sensitive (GTK_WIDGET (this->priv->custom_delete_button), TRUE);
        }
        else
        {
                this->priv->custom_info_bar = gl_message_bar_new (GTK_MESSAGE_INFO,
                                                                          GTK_BUTTONS_NONE,
                                                                          "%s", _("No custom templates found."));
                gl_message_bar_format_secondary_text (GL_MESSAGE_BAR (this->priv->custom_info_bar),
                                                      "%s", _("You may create new templates or try searching for pre-defined templates in the \"Search all\" tab."));

                gtk_box_pack_start (GTK_BOX (this->priv->custom_info_vbox),
                                    this->priv->custom_info_bar,
                                    FALSE, FALSE, 0);
                gtk_widget_show_all (this->priv->custom_info_bar);

                gtk_tree_selection_set_mode (selection, GTK_SELECTION_NONE);

                gtk_widget_set_sensitive (GTK_WIDGET (this->priv->custom_edit_button), FALSE);
                gtk_widget_set_sensitive (GTK_WIDGET (this->priv->custom_delete_button), FALSE);
        }

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
