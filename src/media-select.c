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
        GtkWidget    *brand_combo;
        GtkWidget    *page_size_combo;
        GtkWidget    *category_combo;
        GtkWidget    *search_all_treeview;
        GtkListStore *search_all_store;

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

static gint media_select_signals[LAST_SIGNAL] = { 0 };


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void   gl_media_select_finalize   (GObject                *object);

static void   gl_media_select_construct  (glMediaSelect          *this);

static void   filter_changed_cb          (GtkComboBox            *combo,
                                          gpointer                user_data);

static void   selection_changed_cb       (GtkTreeSelection       *selection,
                                          gpointer                user_data);

static void   page_changed_cb            (GtkNotebook            *notebook,
                                          GtkNotebookPage        *page,
                                          guint                   page_num,
                                          gpointer                user_data);

static gchar *get_layout_desc            (const lglTemplate      *template);

static gchar *get_label_size_desc        (const lglTemplate      *template);

static void   load_recent_list           (glMediaSelect          *this,
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

        media_select_signals[CHANGED] =
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
        GtkBuilder        *builder;
        static gchar      *object_ids[] = { "media_select_hbox", NULL };
        GError            *error = NULL;
        GtkWidget         *hbox;
        GList             *recent_list = NULL;
        GList             *brands = NULL;
        GList             *page_sizes = NULL;
        GList             *categories = NULL;
        GList             *search_all_names = NULL;
        const gchar       *page_size_id;
        gchar             *page_size_name;
        GtkCellRenderer   *renderer;
        GtkTreeViewColumn *column;
        GtkTreeSelection  *recent_selection;
        GtkTreeSelection  *search_all_selection;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        g_return_if_fail (GL_IS_MEDIA_SELECT (this));
        g_return_if_fail (this->priv != NULL);

        builder = gtk_builder_new ();
        gtk_builder_add_objects_from_file (builder,
                                           GLABELS_BUILDER_DIR "media-select.builder",
                                           object_ids,
                                           &error);
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
                                     "brand_combo",            &this->priv->brand_combo,
                                     "page_size_combo",        &this->priv->page_size_combo,
                                     "category_combo",         &this->priv->category_combo,
                                     "search_all_info_vbox",   &this->priv->search_all_info_vbox,
                                     "search_all_treeview",    &this->priv->search_all_treeview,
                                     NULL);

        gtk_container_add (GTK_CONTAINER (this), hbox);
        this->priv->builder = builder;

        this->priv->recent_page_num =
                gtk_notebook_page_num (GTK_NOTEBOOK (this->priv->notebook),
                                       this->priv->recent_tab_vbox);
        this->priv->search_all_page_num =
                gtk_notebook_page_num (GTK_NOTEBOOK (this->priv->notebook),
                                       this->priv->search_all_tab_vbox);

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
        gl_combo_util_add_text_model (GTK_COMBO_BOX (this->priv->brand_combo));
        brands = lgl_db_get_brand_list (NULL, NULL);
        brands = g_list_prepend (brands, g_strdup (_("Any")));
        gl_combo_util_set_strings (GTK_COMBO_BOX (this->priv->brand_combo), brands);
        lgl_db_free_brand_list (brands);
        gl_combo_util_set_active_text (GTK_COMBO_BOX (this->priv->brand_combo),
                                       _("Any"));

        /* Page size selection control */
        gl_combo_util_add_text_model (GTK_COMBO_BOX (this->priv->page_size_combo));
        page_sizes = lgl_db_get_paper_name_list ();
        page_sizes = g_list_prepend (page_sizes, g_strdup (_("Any")));
        gl_combo_util_set_strings (GTK_COMBO_BOX (this->priv->page_size_combo), page_sizes);
        lgl_db_free_paper_name_list (page_sizes);
        gl_combo_util_set_active_text (GTK_COMBO_BOX (this->priv->page_size_combo),
                                       page_size_name);

        /* Category selection control */
        gl_combo_util_add_text_model (GTK_COMBO_BOX (this->priv->category_combo));
        categories = lgl_db_get_category_name_list ();
        categories = g_list_prepend (categories, g_strdup (_("Any")));
        gl_combo_util_set_strings (GTK_COMBO_BOX (this->priv->category_combo), categories);
        gl_combo_util_set_active_text (GTK_COMBO_BOX (this->priv->category_combo),
                                       _("Any"));
        lgl_db_free_category_name_list (categories);

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

        /* Connect signals to controls */
        g_signal_connect (G_OBJECT (this->priv->brand_combo), "changed",
                          G_CALLBACK (filter_changed_cb),
                          this);
        g_signal_connect (G_OBJECT (this->priv->page_size_combo), "changed",
                          G_CALLBACK (filter_changed_cb),
                          this);
        g_signal_connect (G_OBJECT (this->priv->category_combo), "changed",
                          G_CALLBACK (filter_changed_cb),
                          this);
        g_signal_connect (G_OBJECT (recent_selection), "changed",
                          G_CALLBACK (selection_changed_cb),
                          this);
        g_signal_connect (G_OBJECT (search_all_selection), "changed",
                          G_CALLBACK (selection_changed_cb),
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

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change in selection                       */
/*--------------------------------------------------------------------------*/
static void
filter_changed_cb (GtkComboBox *combo,
                   gpointer     user_data)
{
        glMediaSelect     *this = GL_MEDIA_SELECT (user_data);
        gchar             *brand;
        gchar             *page_size_name, *page_size_id;
        gchar             *category_name, *category_id;
        GList             *search_all_names;
        GtkTreeSelection  *selection;

        gl_debug (DEBUG_MEDIA_SELECT, "START");


	this->priv->stop_signals = TRUE;

        /* Update template selections for new filter settings */
        brand = gtk_combo_box_get_active_text (GTK_COMBO_BOX (this->priv->brand_combo));
        page_size_name = gtk_combo_box_get_active_text (GTK_COMBO_BOX (this->priv->page_size_combo));
        category_name = gtk_combo_box_get_active_text (GTK_COMBO_BOX (this->priv->category_combo));
        if ( brand && strlen(brand) &&
             page_size_name && strlen(page_size_name) &&
             category_name && strlen(category_name) )
        {
                gl_debug (DEBUG_MEDIA_SELECT, "brand = \"%s\"", brand);
                gl_debug (DEBUG_MEDIA_SELECT, "page_size_name = \"%s\"", page_size_name);
                gl_debug (DEBUG_MEDIA_SELECT, "category_name = \"%s\"", category_name);
                if (!g_utf8_collate (brand, _("Any")))
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
                g_signal_emit (G_OBJECT (user_data),
                               media_select_signals[CHANGED], 0);
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
        glMediaSelect *this = GL_MEDIA_SELECT (user_data);

	if (this->priv->stop_signals) return;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        /* Emit our "changed" signal */
        g_signal_emit (G_OBJECT (user_data),
                       media_select_signals[CHANGED], 0);

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change in selection                       */
/*--------------------------------------------------------------------------*/
static void
page_changed_cb (GtkNotebook            *notebook,
                 GtkNotebookPage        *page,
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
        g_signal_emit (G_OBJECT (user_data),
                       media_select_signals[CHANGED], 0);

        gl_debug (DEBUG_MEDIA_SELECT, "END");
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
                gtk_combo_box_get_active_text (GTK_COMBO_BOX (this->priv->page_size_combo));

        *page_size_id = lgl_db_lookup_paper_id_from_name (page_size_name);

        category_name =
                gtk_combo_box_get_active_text (GTK_COMBO_BOX (this->priv->category_combo));

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
                page_size_name = g_strdup (_("Any"));
        }

        gl_combo_util_set_active_text (GTK_COMBO_BOX (this->priv->page_size_combo),
                                       page_size_name);

        category_name = lgl_db_lookup_category_name_from_id (category_id);
        if (category_name == NULL)
        {
                category_name = g_strdup (_("Any"));
        }

        gl_combo_util_set_active_text (GTK_COMBO_BOX (this->priv->category_combo),
                                       category_name);
        g_free (page_size_name);
        g_free (category_name);

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Get a description of the layout and number of labels.          */
/*--------------------------------------------------------------------------*/
static gchar *
get_layout_desc (const lglTemplate *template)
{
        const lglTemplateFrame *frame;
        gint                    n_labels;
        gchar                  *string;

        frame = (lglTemplateFrame *)template->frames->data;

        n_labels = lgl_template_frame_get_n_labels (frame);

        string = g_strdup_printf (_("%d per sheet"), n_labels);

        return string;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Get label size description.                                    */ 
/*--------------------------------------------------------------------------*/
static gchar *
get_label_size_desc (const lglTemplate *template)
{
        lglUnits                   units;
        const gchar               *units_string;
        gdouble                    units_per_point;
        const lglTemplateFrame    *frame;
        gchar                     *string = NULL;

        units           = gl_prefs_model_get_units (gl_prefs);
        units_string    = lgl_units_get_name (units);
        units_per_point = lgl_units_get_units_per_point (units);

        frame = (lglTemplateFrame *)template->frames->data;

        switch (frame->shape) {
        case LGL_TEMPLATE_FRAME_SHAPE_RECT:
                if ( units == LGL_UNITS_INCH ) {
                        gchar *xstr, *ystr;

                        xstr = gl_str_util_fraction_to_string (frame->rect.w*units_per_point);
                        ystr = gl_str_util_fraction_to_string (frame->rect.h*units_per_point);
                        string = g_strdup_printf ("%s × %s %s",
                                                  xstr, ystr, units_string);
                        g_free (xstr);
                        g_free (ystr);
                } else {
                        string = g_strdup_printf ("%.5g × %.5g %s",
                                                  frame->rect.w*units_per_point,
                                                  frame->rect.h*units_per_point,
                                                  units_string);
                }
                break;
        case LGL_TEMPLATE_FRAME_SHAPE_ROUND:
                if ( units == LGL_UNITS_INCH ) {
                        gchar *dstr;

                        dstr = gl_str_util_fraction_to_string (2.0*frame->round.r*units_per_point);
                        string = g_strdup_printf ("%s %s %s",
                                                  dstr, units_string,
                                                  _("diameter"));
                        g_free (dstr);
                } else {
                        string = g_strdup_printf ("%.5g %s %s",
                                                  2.0*frame->round.r*units_per_point,
                                                  units_string,
                                                  _("diameter"));
                }
                break;
        case LGL_TEMPLATE_FRAME_SHAPE_CD:
                if ( units == LGL_UNITS_INCH ) {
                        gchar *dstr;

                        dstr = gl_str_util_fraction_to_string (2.0*frame->cd.r1*units_per_point);
                        string = g_strdup_printf ("%s %s %s",
                                                  dstr, units_string,
                                                  _("diameter"));
                        g_free (dstr);
                } else {
                        string = g_strdup_printf ("%.5g %s %s",
                                                  2.0*frame->cd.r1*units_per_point,
                                                  units_string,
                                                  _("diameter"));
                }
                break;
        default:
                break;
        }

        return string;
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
        GList       *p;
        GtkTreeIter  iter;
        lglTemplate *template;
        GdkPixbuf   *pixbuf;
        gchar       *size;
        gchar       *layout;
        gchar       *description;

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

                for ( p=list; p!=NULL; p=p->next )
                {

                        gl_debug (DEBUG_MEDIA_SELECT, "p->data = \"%s\"", p->data);

                        template = lgl_db_lookup_template_from_name (p->data);
                        pixbuf = gl_mini_preview_pixbuf_cache_get_pixbuf (p->data);

                        size = get_label_size_desc (template);
                        layout = get_layout_desc (template);
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
                this->priv->recent_info_bar = gl_message_bar_new (GTK_MESSAGE_INFO,
                                                                          GTK_BUTTONS_NONE,
                                                                          "%s", _("No recent templates found."));
                gl_message_bar_format_secondary_text (GL_MESSAGE_BAR (this->priv->recent_info_bar),
                                                      "%s", _("Try selecting a template from the \"Search all templates\" page."));

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
        GList       *p;
        GtkTreeIter  iter;
        lglTemplate *template;
        GdkPixbuf   *pixbuf;
        gchar       *size;
        gchar       *layout;
        gchar       *description;

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

                for ( p=list; p!=NULL; p=p->next )
                {

                        gl_debug (DEBUG_MEDIA_SELECT, "p->data = \"%s\"", p->data);

                        template = lgl_db_lookup_template_from_name (p->data);
                        pixbuf = gl_mini_preview_pixbuf_cache_get_pixbuf (p->data);

                        size = get_label_size_desc (template);
                        layout = get_layout_desc (template);
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
                this->priv->search_all_info_bar = gl_message_bar_new (GTK_MESSAGE_INFO,
                                                                              GTK_BUTTONS_NONE,
                                                                              "%s", _("No match."));
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



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
