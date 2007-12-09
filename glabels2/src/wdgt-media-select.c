/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_media_select.c:  media selection widget module
 *
 *  Copyright (C) 2001-2006  Jim Evins <evins@snaught.com>.
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

#include "wdgt-media-select.h"

#include <glib/gi18n.h>
#include <glade/glade-xml.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkcombobox.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkcellrendererpixbuf.h>
#include <gtk/gtkstock.h>
#include <string.h>

#include "mini-preview-pixbuf-cache.h"
#include "prefs.h"
#include "util.h"
#include "color.h"
#include "marshal.h"
#include <libglabels/db.h>

#include "debug.h"

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

struct _glWdgtMediaSelectPrivate {

        GtkWidget    *notebook;
        guint         current_page_num;

        gint          recent_page_num;
        GtkWidget    *recent_tab_vbox;
        GtkWidget    *recent_treeview;
        GtkListStore *recent_store;

        gint          search_all_page_num;
        GtkWidget    *search_all_tab_vbox;
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

typedef void (*glWdgtMediaSelectSignal) (GObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static gint wdgt_media_select_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_wdgt_media_select_finalize      (GObject                *object);

static void gl_wdgt_media_select_construct     (glWdgtMediaSelect      *media_select);

static void filter_changed_cb                  (GtkComboBox            *combo,
                                                gpointer                user_data);
static void selection_changed_cb               (GtkTreeSelection       *selection,
                                                gpointer                user_data);
static void page_changed_cb                    (GtkNotebook            *notebook,
                                                GtkNotebookPage        *page,
                                                guint                   page_num,
                                                gpointer                user_data);

static gchar *get_layout_desc                  (const lglTemplate      *template);
static gchar *get_label_size_desc              (const lglTemplate      *template);
static void   load_recent_list                 (GtkListStore           *store,
                                                GtkTreeSelection       *selection,
                                                GSList                 *list);
static void   load_search_all_list             (GtkListStore           *store,
                                                GtkTreeSelection       *selection,
                                                GList                  *list);

/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
G_DEFINE_TYPE (glWdgtMediaSelect, gl_wdgt_media_select, GTK_TYPE_VBOX);


static void
gl_wdgt_media_select_class_init (glWdgtMediaSelectClass *class)
{
        GObjectClass *object_class = G_OBJECT_CLASS (class);

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        gl_wdgt_media_select_parent_class = g_type_class_peek_parent (class);

        object_class->finalize = gl_wdgt_media_select_finalize;

        wdgt_media_select_signals[CHANGED] =
            g_signal_new ("changed",
                          G_OBJECT_CLASS_TYPE(object_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (glWdgtMediaSelectClass, changed),
                          NULL, NULL,
                          gl_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}

static void
gl_wdgt_media_select_init (glWdgtMediaSelect *media_select)
{
        gl_debug (DEBUG_MEDIA_SELECT, "START");

        media_select->priv = g_new0 (glWdgtMediaSelectPrivate, 1);

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}

static void
gl_wdgt_media_select_finalize (GObject *object)
{
        glWdgtMediaSelect *media_select = GL_WDGT_MEDIA_SELECT (object);

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        g_return_if_fail (object != NULL);
        g_return_if_fail (GL_IS_WDGT_MEDIA_SELECT (object));

        g_object_unref (media_select->priv->search_all_store);
        g_free (media_select->priv);

        G_OBJECT_CLASS (gl_wdgt_media_select_parent_class)->finalize (object);

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}

GtkWidget *
gl_wdgt_media_select_new (void)
{
        glWdgtMediaSelect *media_select;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        media_select = g_object_new (gl_wdgt_media_select_get_type (), NULL);

        gl_wdgt_media_select_construct (media_select);

        gl_debug (DEBUG_MEDIA_SELECT, "END");

        return GTK_WIDGET (media_select);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                    */
/*--------------------------------------------------------------------------*/
static void
gl_wdgt_media_select_construct (glWdgtMediaSelect *media_select)
{
        GladeXML          *gui;
        GtkWidget         *hbox;
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

        g_return_if_fail (GL_IS_WDGT_MEDIA_SELECT (media_select));
        g_return_if_fail (media_select->priv != NULL);

        gui = glade_xml_new (GLABELS_GLADE_DIR "wdgt-media-select.glade",
                             "wdgt_media_select_hbox", NULL);

        if (!gui) {
                g_critical ("Could not open wdgt-media-select.glade. gLabels may not be installed correctly!");
                return;
        }

        hbox = glade_xml_get_widget (gui, "wdgt_media_select_hbox");
        gtk_container_add (GTK_CONTAINER (media_select), hbox);

        media_select->priv->notebook =
                glade_xml_get_widget (gui, "notebook");

        media_select->priv->recent_tab_vbox =
                glade_xml_get_widget (gui, "recent_tab_vbox");
        media_select->priv->recent_treeview =
                glade_xml_get_widget (gui, "recent_treeview");

        media_select->priv->search_all_tab_vbox =
                glade_xml_get_widget (gui, "search_all_tab_vbox");
        media_select->priv->brand_combo =
                glade_xml_get_widget (gui, "brand_combo");
        media_select->priv->page_size_combo =
                glade_xml_get_widget (gui, "page_size_combo");
        media_select->priv->category_combo =
                glade_xml_get_widget (gui, "category_combo");
        media_select->priv->search_all_treeview =
                glade_xml_get_widget (gui, "search_all_treeview");

        g_object_unref (gui);

        media_select->priv->recent_page_num =
                gtk_notebook_page_num (GTK_NOTEBOOK (media_select->priv->notebook),
                                       media_select->priv->recent_tab_vbox);
        media_select->priv->search_all_page_num =
                gtk_notebook_page_num (GTK_NOTEBOOK (media_select->priv->notebook),
                                       media_select->priv->search_all_tab_vbox);

        /* Recent templates treeview */
        media_select->priv->recent_store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING);
        gtk_tree_view_set_model (GTK_TREE_VIEW (media_select->priv->recent_treeview),
                                 GTK_TREE_MODEL (media_select->priv->recent_store));
        renderer = gtk_cell_renderer_pixbuf_new ();
        column = gtk_tree_view_column_new_with_attributes ("", renderer,
                                                           "pixbuf", PREVIEW_COLUMN,
                                                           "stock-id", PREVIEW_COLUMN_STOCK,
                                                           "stock-size", PREVIEW_COLUMN_STOCK_SIZE,
                                                           NULL);
        gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_append_column (GTK_TREE_VIEW (media_select->priv->recent_treeview), column);
        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes ("", renderer,
                                                           "markup", DESCRIPTION_COLUMN,
                                                           NULL);
        gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_append_column (GTK_TREE_VIEW (media_select->priv->recent_treeview), column);
        recent_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (media_select->priv->recent_treeview));
        load_recent_list (media_select->priv->recent_store, recent_selection, gl_prefs->recent_templates);

        page_size_id = gl_prefs_get_page_size ();
        page_size_name = lgl_db_lookup_paper_name_from_id (page_size_id);

        /* Brand selection control */
        gl_util_combo_box_add_text_model (GTK_COMBO_BOX (media_select->priv->brand_combo));
        brands = lgl_db_get_brand_list (NULL, NULL);
        brands = g_list_prepend (brands, g_strdup (_("Any")));
        gl_util_combo_box_set_strings (GTK_COMBO_BOX (media_select->priv->brand_combo), brands);
        lgl_db_free_brand_list (brands);
        gl_util_combo_box_set_active_text (GTK_COMBO_BOX (media_select->priv->brand_combo),
                                           _("Any"));

        /* Page size selection control */
        gl_util_combo_box_add_text_model (GTK_COMBO_BOX (media_select->priv->page_size_combo));
        page_sizes = lgl_db_get_paper_name_list ();
        page_sizes = g_list_prepend (page_sizes, g_strdup (_("Any")));
        gl_util_combo_box_set_strings (GTK_COMBO_BOX (media_select->priv->page_size_combo), page_sizes);
        lgl_db_free_paper_name_list (page_sizes);
        gl_util_combo_box_set_active_text (GTK_COMBO_BOX (media_select->priv->page_size_combo),
                                           page_size_name);

        /* Category selection control */
        gl_util_combo_box_add_text_model (GTK_COMBO_BOX (media_select->priv->category_combo));
        categories = lgl_db_get_category_name_list ();
        categories = g_list_prepend (categories, g_strdup (_("Any")));
        gl_util_combo_box_set_strings (GTK_COMBO_BOX (media_select->priv->category_combo), categories);
        gl_util_combo_box_set_active_text (GTK_COMBO_BOX (media_select->priv->category_combo),
                                           _("Any"));
        lgl_db_free_category_name_list (categories);

        /* Search all treeview */
        media_select->priv->search_all_store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING);
        gtk_tree_view_set_model (GTK_TREE_VIEW (media_select->priv->search_all_treeview),
                                 GTK_TREE_MODEL (media_select->priv->search_all_store));
        renderer = gtk_cell_renderer_pixbuf_new ();
        column = gtk_tree_view_column_new_with_attributes ("", renderer,
                                                           "pixbuf", PREVIEW_COLUMN,
                                                           "stock-id", PREVIEW_COLUMN_STOCK,
                                                           "stock-size", PREVIEW_COLUMN_STOCK_SIZE,
                                                           NULL);
        gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_append_column (GTK_TREE_VIEW (media_select->priv->search_all_treeview), column);
        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes ("", renderer,
                                                           "markup", DESCRIPTION_COLUMN,
                                                           NULL);
        gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_append_column (GTK_TREE_VIEW (media_select->priv->search_all_treeview), column);
        search_all_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (media_select->priv->search_all_treeview));
        search_all_names = lgl_db_get_template_name_list_all (NULL, page_size_id, NULL);
        load_search_all_list (media_select->priv->search_all_store, search_all_selection, search_all_names);
        lgl_db_free_template_name_list (search_all_names);

        /* Connect signals to controls */
        g_signal_connect (G_OBJECT (media_select->priv->brand_combo), "changed",
                          G_CALLBACK (filter_changed_cb),
                          media_select);
        g_signal_connect (G_OBJECT (media_select->priv->page_size_combo), "changed",
                          G_CALLBACK (filter_changed_cb),
                          media_select);
        g_signal_connect (G_OBJECT (media_select->priv->category_combo), "changed",
                          G_CALLBACK (filter_changed_cb),
                          media_select);
        g_signal_connect (G_OBJECT (recent_selection), "changed",
                          G_CALLBACK (selection_changed_cb),
                          media_select);
        g_signal_connect (G_OBJECT (search_all_selection), "changed",
                          G_CALLBACK (selection_changed_cb),
                          media_select);
        g_signal_connect (G_OBJECT (media_select->priv->notebook), "switch-page",
                          G_CALLBACK (page_changed_cb),
                          media_select);

        g_free (page_size_name);

        gtk_widget_show_all (GTK_WIDGET (media_select));
        if ( gl_prefs->recent_templates )
        {
                gtk_notebook_set_current_page (GTK_NOTEBOOK (media_select->priv->notebook),
                                               media_select->priv->recent_page_num);
        }
        else
        {
                gtk_notebook_set_current_page (GTK_NOTEBOOK (media_select->priv->notebook),
                                               media_select->priv->search_all_page_num);
        }

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change in selection                       */
/*--------------------------------------------------------------------------*/
static void
filter_changed_cb (GtkComboBox *combo,
                   gpointer     user_data)
{
        glWdgtMediaSelect *media_select = GL_WDGT_MEDIA_SELECT (user_data);
        gchar             *brand;
        gchar             *page_size_name, *page_size_id;
        gchar             *category_name, *category_id;
        GList             *search_all_names;
        GtkTreeSelection  *selection;

        gl_debug (DEBUG_MEDIA_SELECT, "START");


	media_select->priv->stop_signals = TRUE;

        /* Update template selections for new filter settings */
        brand = gtk_combo_box_get_active_text (GTK_COMBO_BOX (media_select->priv->brand_combo));
        page_size_name = gtk_combo_box_get_active_text (GTK_COMBO_BOX (media_select->priv->page_size_combo));
        category_name = gtk_combo_box_get_active_text (GTK_COMBO_BOX (media_select->priv->category_combo));
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
                selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (media_select->priv->search_all_treeview));
                load_search_all_list (media_select->priv->search_all_store, selection, search_all_names);
                lgl_db_free_template_name_list (search_all_names);
                g_free (page_size_id);
                g_free (category_id);

                /* Emit our "changed" signal */
                g_signal_emit (G_OBJECT (user_data),
                               wdgt_media_select_signals[CHANGED], 0);
        }
        g_free (brand);
        g_free (page_size_name);
        g_free (category_name);


	media_select->priv->stop_signals = FALSE;

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change in selection                       */
/*--------------------------------------------------------------------------*/
static void
selection_changed_cb (GtkTreeSelection       *selection,
                      gpointer                user_data)
{
        glWdgtMediaSelect *media_select = GL_WDGT_MEDIA_SELECT (user_data);

	if (media_select->priv->stop_signals) return;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        /* Emit our "changed" signal */
        g_signal_emit (G_OBJECT (user_data),
                       wdgt_media_select_signals[CHANGED], 0);

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
        glWdgtMediaSelect *media_select = GL_WDGT_MEDIA_SELECT (user_data);

	if (media_select->priv->stop_signals) return;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        /*
         * Store new current page, because this signal is emitted before the actual page change.
         */
	media_select->priv->current_page_num = page_num;

        /* Emit our "changed" signal */
        g_signal_emit (G_OBJECT (user_data),
                       wdgt_media_select_signals[CHANGED], 0);

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}

/****************************************************************************/
/* query selected label template name.                                      */
/****************************************************************************/
gchar *
gl_wdgt_media_select_get_name (glWdgtMediaSelect *media_select)
{
        gint               page_num;
        GtkTreeSelection  *selection;
        GtkTreeIter        iter;
        GtkTreeModel      *model;        
        gchar             *name;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        page_num = media_select->priv->current_page_num;
        if (page_num == media_select->priv->recent_page_num)
        {
                selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (media_select->priv->recent_treeview));
        }
        else if (page_num == media_select->priv->search_all_page_num)
        {
                selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (media_select->priv->search_all_treeview));
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
gl_wdgt_media_select_set_name (glWdgtMediaSelect *media_select,
                               gchar             *name)
{
        GtkTreeSelection  *selection;
        GtkTreeModel      *model;
        GtkTreeIter        iter;
        GtkTreePath       *path;
        gchar             *name_i;
        gboolean           flag;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (media_select->priv->search_all_treeview));
        g_return_if_fail (GTK_IS_TREE_SELECTION (selection));

        model = GTK_TREE_MODEL (media_select->priv->search_all_store);

        for ( flag = gtk_tree_model_get_iter_first (model, &iter);
              flag;
              flag = gtk_tree_model_iter_next(model, &iter) )
        {
                gtk_tree_model_get (model, &iter, NAME_COLUMN, &name_i, -1); 
                if (strcasecmp(name, name_i) == 0)
                {
                        gtk_tree_selection_select_iter (selection, &iter);
                        path = gtk_tree_model_get_path (model, &iter);
                        gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (media_select->priv->search_all_treeview),
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
gl_wdgt_media_select_get_filter_parameters (glWdgtMediaSelect *media_select,
                                            gchar            **page_size_id,
                                            gchar            **category_id)
{
        gchar *page_size_name, *category_name;

        gl_debug (DEBUG_MEDIA_SELECT, "");

        g_free (*page_size_id);
        g_free (*category_id);

        page_size_name =
                gtk_combo_box_get_active_text (GTK_COMBO_BOX (media_select->priv->page_size_combo));

        *page_size_id = lgl_db_lookup_paper_id_from_name (page_size_name);

        category_name =
                gtk_combo_box_get_active_text (GTK_COMBO_BOX (media_select->priv->category_combo));

        *category_id = lgl_db_lookup_category_id_from_name (category_name);

        g_free (page_size_name);
        g_free (category_name);
}

/****************************************************************************/
/* set filter parameters.                                                   */
/****************************************************************************/
void
gl_wdgt_media_select_set_filter_parameters (glWdgtMediaSelect *media_select,
                                            const gchar       *page_size_id,
                                            const gchar       *category_id)
{
        gchar *page_size_name;
        gchar *category_name;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        page_size_name = lgl_db_lookup_paper_name_from_id (page_size_id);
        if (page_size_name == NULL)
        {
                page_size_name = g_strdup (_("Any"));
        }

        gl_util_combo_box_set_active_text (GTK_COMBO_BOX (media_select->priv->page_size_combo),
                                           page_size_name);

        category_name = lgl_db_lookup_category_name_from_id (category_id);
        if (category_name == NULL)
        {
                category_name = g_strdup (_("Any"));
        }

        gl_util_combo_box_set_active_text (GTK_COMBO_BOX (media_select->priv->category_combo),
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
        lglUnitsType               units;
        const gchar               *units_string;
        gdouble                    units_per_point;
        const lglTemplateFrame    *frame;
        gchar                     *string = NULL;

        units           = gl_prefs_get_units ();
        units_string    = gl_prefs_get_units_string ();
        units_per_point = gl_prefs_get_units_per_point ();

        frame = (lglTemplateFrame *)template->frames->data;

        switch (frame->shape) {
        case LGL_TEMPLATE_FRAME_SHAPE_RECT:
                if ( units == LGL_UNITS_INCH ) {
                        gchar *xstr, *ystr;

                        xstr = gl_util_fraction (frame->rect.w*units_per_point);
                        ystr = gl_util_fraction (frame->rect.h*units_per_point);
                        string = g_strdup_printf (_("%s x %s %s"),
                                                  xstr, ystr, units_string);
                        g_free (xstr);
                        g_free (ystr);
                } else {
                        string = g_strdup_printf (_("%.5g x %.5g %s"),
                                                  frame->rect.w*units_per_point,
                                                  frame->rect.h*units_per_point,
                                                  units_string);
                }
                break;
        case LGL_TEMPLATE_FRAME_SHAPE_ROUND:
                if ( units == LGL_UNITS_INCH ) {
                        gchar *dstr;

                        dstr = gl_util_fraction (2.0*frame->round.r*units_per_point);
                        string = g_strdup_printf (_("%s %s diameter"),
                                                  dstr, units_string);
                        g_free (dstr);
                } else {
                        string = g_strdup_printf (_("%.5g %s diameter"),
                                                  2.0*frame->round.r*units_per_point,
                                                  units_string);
                }
                break;
        case LGL_TEMPLATE_FRAME_SHAPE_CD:
                if ( units == LGL_UNITS_INCH ) {
                        gchar *dstr;

                        dstr = gl_util_fraction (2.0*frame->cd.r1*units_per_point);
                        string = g_strdup_printf (_("%s %s diameter"),
                                                  dstr, units_string);
                        g_free (dstr);
                } else {
                        string = g_strdup_printf (_("%.5g %s diameter"),
                                                  2.0*frame->cd.r1*units_per_point,
                                                  units_string);
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
load_recent_list (GtkListStore           *store,
                  GtkTreeSelection       *selection,
                  GSList                  *list)
{
        GSList      *p;
        GtkTreeIter  iter;
        lglTemplate *template;
        GdkPixbuf   *pixbuf;
        gchar       *size;
        gchar       *layout;
        gchar       *description;
        gchar       *name;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        gtk_list_store_clear (store);

        if (list)
        {

                for ( p=list; p!=NULL; p=p->next )
                {

                        gl_debug (DEBUG_MEDIA_SELECT, "p->data = \"%s\"", p->data);

                        template = lgl_db_lookup_template_from_name (p->data);
                        
                        name = lgl_template_get_name (template);
                        pixbuf = gl_mini_preview_pixbuf_cache_get_pixbuf (name);
                        g_free (name);

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
                gchar *text = g_strdup_printf ("<span weight=\"bold\" size=\"larger\">%s</span>\n%s",
                                               _("No recent templates found."),
                                               _("Try selecting a template from the \"Search all templates\" page."));
                gtk_list_store_append (store, &iter);
                gtk_list_store_set (store, &iter,
                                    NAME_COLUMN, "empty",
                                    PREVIEW_COLUMN_STOCK, GTK_STOCK_DIALOG_WARNING,
                                    PREVIEW_COLUMN_STOCK_SIZE, GTK_ICON_SIZE_DIALOG,
                                    DESCRIPTION_COLUMN, text,
                                    -1);
                g_free (text);

                gtk_tree_selection_set_mode (selection, GTK_SELECTION_NONE);

        }

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Load list store from template name list.                       */
/*--------------------------------------------------------------------------*/
static void
load_search_all_list (GtkListStore           *store,
                      GtkTreeSelection       *selection,
                      GList                  *list)
{
        GList       *p;
        GtkTreeIter  iter;
        lglTemplate *template;
        GdkPixbuf   *pixbuf;
        gchar       *size;
        gchar       *layout;
        gchar       *description;
        gchar       *name;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        gtk_list_store_clear (store);

        if (list)
        {

                for ( p=list; p!=NULL; p=p->next )
                {

                        gl_debug (DEBUG_MEDIA_SELECT, "p->data = \"%s\"", p->data);

                        template = lgl_db_lookup_template_from_name (p->data);
                        
                        name = lgl_template_get_name (template);
                        pixbuf = gl_mini_preview_pixbuf_cache_get_pixbuf (name);
                        g_free (name);

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
                gchar *text = g_strdup_printf ("<span weight=\"bold\" size=\"larger\">%s</span>\n%s",
                                               _("No match."),
                                               _("Try selecting a different brand, page size or category."));
                gtk_list_store_append (store, &iter);
                gtk_list_store_set (store, &iter,
                                    NAME_COLUMN, "empty",
                                    PREVIEW_COLUMN_STOCK, GTK_STOCK_DIALOG_WARNING,
                                    PREVIEW_COLUMN_STOCK_SIZE, GTK_ICON_SIZE_DIALOG,
                                    DESCRIPTION_COLUMN, text,
                                    -1);
                g_free (text);

                gtk_tree_selection_set_mode (selection, GTK_SELECTION_NONE);

        }

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}

