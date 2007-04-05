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
#include <gtk/gtkcombobox.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkcellrendererpixbuf.h>
#include <gtk/gtkstock.h>
#include <string.h>

#include "mini-preview-pixbuf.h"
#include "prefs.h"
#include "util.h"
#include "color.h"
#include "marshal.h"
#include <libglabels/paper.h>
#include <libglabels/category.h>
#include <libglabels/template.h>

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

        GtkWidget    *page_size_combo;
        GtkWidget    *category_combo;

        GtkWidget    *template_treeview;
        GtkListStore *template_store;
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
static void template_selection_changed_cb      (GtkTreeSelection       *selection,
                                                gpointer                user_data);

static gchar *get_layout_desc                  (const glTemplate       *template);
static gchar *get_label_size_desc              (const glTemplate       *template);
static void   load_list                        (GtkListStore           *store,
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

        g_object_unref (media_select->priv->template_store);
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
        GList             *page_sizes = NULL;
        GList             *categories = NULL;
        GList             *template_names = NULL;
        const gchar       *page_size_id;
        gchar             *page_size_name;
        GtkCellRenderer   *renderer;
        GtkTreeViewColumn *column;
        GtkTreeSelection  *selection;

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

        media_select->priv->page_size_combo =
                glade_xml_get_widget (gui, "page_size_combo");
        media_select->priv->category_combo =
                glade_xml_get_widget (gui, "category_combo");
        media_select->priv->template_treeview =
                glade_xml_get_widget (gui, "template_treeview");

        g_object_unref (gui);

        page_size_id = gl_prefs_get_page_size ();
        page_size_name = gl_paper_lookup_name_from_id (page_size_id);

        /* Page size selection control */
        gl_util_combo_box_add_text_model (GTK_COMBO_BOX (media_select->priv->page_size_combo));
        page_sizes = gl_paper_get_name_list ();
        page_sizes = g_list_prepend (page_sizes, g_strdup (_("Any")));
        gl_util_combo_box_set_strings (GTK_COMBO_BOX (media_select->priv->page_size_combo), page_sizes);
        gl_paper_free_name_list (page_sizes);
        gl_util_combo_box_set_active_text (GTK_COMBO_BOX (media_select->priv->page_size_combo),
                                           page_size_name);

        /* Category selection control */
        gl_util_combo_box_add_text_model (GTK_COMBO_BOX (media_select->priv->category_combo));
        categories = gl_category_get_name_list ();
        categories = g_list_prepend (categories, g_strdup (_("Any")));
        gl_util_combo_box_set_strings (GTK_COMBO_BOX (media_select->priv->category_combo), categories);
        gl_util_combo_box_set_active_text (GTK_COMBO_BOX (media_select->priv->category_combo),
                                           _("Any"));
        gl_category_free_name_list (categories);

        /* Actual selection control */
        media_select->priv->template_store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING);
        gtk_tree_view_set_model (GTK_TREE_VIEW (media_select->priv->template_treeview),
                                 GTK_TREE_MODEL (media_select->priv->template_store));
        renderer = gtk_cell_renderer_pixbuf_new ();
        column = gtk_tree_view_column_new_with_attributes ("", renderer,
                                                           "pixbuf", PREVIEW_COLUMN,
                                                           "stock-id", PREVIEW_COLUMN_STOCK,
                                                           "stock-size", PREVIEW_COLUMN_STOCK_SIZE,
                                                           NULL);
        gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_append_column (GTK_TREE_VIEW (media_select->priv->template_treeview), column);
        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes ("", renderer,
                                                           "markup", DESCRIPTION_COLUMN,
                                                           NULL);
        gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_append_column (GTK_TREE_VIEW (media_select->priv->template_treeview), column);
        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (media_select->priv->template_treeview));
        template_names = gl_template_get_name_list (page_size_id, NULL);
        load_list (media_select->priv->template_store, selection, template_names);
        gl_template_free_name_list (template_names);

        /* Connect signals to controls */
        g_signal_connect (G_OBJECT (media_select->priv->page_size_combo), "changed",
                          G_CALLBACK (filter_changed_cb),
                          media_select);
        g_signal_connect (G_OBJECT (media_select->priv->category_combo), "changed",
                          G_CALLBACK (filter_changed_cb),
                          media_select);
        g_signal_connect (G_OBJECT (selection), "changed",
                          G_CALLBACK (template_selection_changed_cb),
                          media_select);

        g_free (page_size_name);

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
        gchar             *page_size_name, *page_size_id;
        gchar             *category_name, *category_id;
        GList             *template_names;
        GtkTreeSelection  *selection;

        gl_debug (DEBUG_MEDIA_SELECT, "START");


        /* Update template selections for new page size */
        page_size_name = gtk_combo_box_get_active_text (GTK_COMBO_BOX (media_select->priv->page_size_combo));
        category_name = gtk_combo_box_get_active_text (GTK_COMBO_BOX (media_select->priv->category_combo));
        if ( page_size_name && strlen(page_size_name) &&
             category_name && strlen(category_name) )
        {
                gl_debug (DEBUG_MEDIA_SELECT, "page_size_name = \"%s\"", page_size_name);
                gl_debug (DEBUG_MEDIA_SELECT, "category_name = \"%s\"", category_name);
                page_size_id = gl_paper_lookup_id_from_name (page_size_name);
                category_id = gl_category_lookup_id_from_name (category_name);
                gl_debug (DEBUG_MEDIA_SELECT, "page_size_id = \"%s\"", page_size_id);
                gl_debug (DEBUG_MEDIA_SELECT, "category_id = \"%s\"", category_id);
                template_names = gl_template_get_name_list (page_size_id, category_id);
                selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (media_select->priv->template_treeview));
                g_signal_handlers_block_by_func (G_OBJECT (selection),
                                                 template_selection_changed_cb,
                                                 media_select);
                load_list (media_select->priv->template_store, selection, template_names);
                g_signal_handlers_unblock_by_func (G_OBJECT (selection),
                                                   template_selection_changed_cb,
                                                   media_select);
                gl_template_free_name_list (template_names);
                g_free (page_size_id);

                /* Emit our "changed" signal */
                g_signal_emit (G_OBJECT (user_data),
                               wdgt_media_select_signals[CHANGED], 0);
        }
        g_free (page_size_name);

        gl_debug (DEBUG_MEDIA_SELECT, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  modify widget due to change in selection                       */
/*--------------------------------------------------------------------------*/
static void
template_selection_changed_cb (GtkTreeSelection       *selection,
                               gpointer                user_data)
{
        gl_debug (DEBUG_MEDIA_SELECT, "START");

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
        GtkTreeSelection  *selection;
        GtkTreeIter        iter;
        GtkTreeModel      *model;        
        gchar             *name;

        gl_debug (DEBUG_MEDIA_SELECT, "START");
        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (media_select->priv->template_treeview));
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

        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (media_select->priv->template_treeview));
        g_return_if_fail (GTK_IS_TREE_SELECTION (selection));

        model = GTK_TREE_MODEL (media_select->priv->template_store);

        for ( flag = gtk_tree_model_get_iter_first (model, &iter);
              flag;
              flag = gtk_tree_model_iter_next(model, &iter) )
        {
                gtk_tree_model_get (model, &iter, NAME_COLUMN, &name_i, -1); 
                if (strcasecmp(name, name_i) == 0)
                {
                        gtk_tree_selection_select_iter (selection, &iter);
                        path = gtk_tree_model_get_path (model, &iter);
                        gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (media_select->priv->template_treeview),
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

        *page_size_id = gl_paper_lookup_id_from_name (page_size_name);

        category_name =
                gtk_combo_box_get_active_text (GTK_COMBO_BOX (media_select->priv->category_combo));

        *category_id = gl_category_lookup_id_from_name (category_name);

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

        page_size_name = gl_paper_lookup_name_from_id (page_size_id);
        if (page_size_name == NULL)
        {
                page_size_name = g_strdup (_("Any"));
        }

        gl_util_combo_box_set_active_text (GTK_COMBO_BOX (media_select->priv->page_size_combo),
                                           page_size_name);

        category_name = gl_category_lookup_name_from_id (category_id);
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
get_layout_desc (const glTemplate *template)
{
        const glTemplateLabelType *label_type;
        gint                       n_labels;
        gchar                     *string;

        label_type = gl_template_get_first_label_type (template);

        n_labels = gl_template_get_n_labels (label_type);

        string = g_strdup_printf (_("%d per sheet"), n_labels);

        return string;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Get label size description.                                    */ 
/*--------------------------------------------------------------------------*/
static gchar *
get_label_size_desc (const glTemplate *template)
{
        glUnitsType                units;
        const gchar               *units_string;
        gdouble                    units_per_point;
        const glTemplateLabelType *label_type;
        gchar                     *string = NULL;

        units           = gl_prefs_get_units ();
        units_string    = gl_prefs_get_units_string ();
        units_per_point = gl_prefs_get_units_per_point ();

        label_type = gl_template_get_first_label_type (template);

        switch (label_type->shape) {
        case GL_TEMPLATE_SHAPE_RECT:
                if ( units == GL_UNITS_INCH ) {
                        gchar *xstr, *ystr;

                        xstr = gl_util_fraction (label_type->size.rect.w*units_per_point);
                        ystr = gl_util_fraction (label_type->size.rect.h*units_per_point);
                        string = g_strdup_printf (_("%s x %s %s"),
                                                  xstr, ystr, units_string);
                        g_free (xstr);
                        g_free (ystr);
                } else {
                        string = g_strdup_printf (_("%.5g x %.5g %s"),
                                                  label_type->size.rect.w*units_per_point,
                                                  label_type->size.rect.h*units_per_point,
                                                  units_string);
                }
                break;
        case GL_TEMPLATE_SHAPE_ROUND:
                if ( units == GL_UNITS_INCH ) {
                        gchar *dstr;

                        dstr = gl_util_fraction (2.0*label_type->size.round.r*units_per_point);
                        string = g_strdup_printf (_("%s %s diameter"),
                                                  dstr, units_string);
                        g_free (dstr);
                } else {
                        string = g_strdup_printf (_("%.5g %s diameter"),
                                                  2.0*label_type->size.round.r*units_per_point,
                                                  units_string);
                }
                break;
        case GL_TEMPLATE_SHAPE_CD:
                if ( units == GL_UNITS_INCH ) {
                        gchar *dstr;

                        dstr = gl_util_fraction (2.0*label_type->size.cd.r1*units_per_point);
                        string = g_strdup_printf (_("%s %s diameter"),
                                                  dstr, units_string);
                        g_free (dstr);
                } else {
                        string = g_strdup_printf (_("%.5g %s diameter"),
                                                  2.0*label_type->size.cd.r1*units_per_point,
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
load_list (GtkListStore           *store,
           GtkTreeSelection       *selection,
           GList                  *list)
{
        GList       *p;
        GtkTreeIter  iter;
        glTemplate  *template;
        GdkPixbuf   *pixbuf;
        gchar       *size;
        gchar       *layout;
        gchar       *description;

        gl_debug (DEBUG_MEDIA_SELECT, "START");

        gtk_list_store_clear (store);

        if (list)
        {

                for ( p=list; p!=NULL; p=p->next )
                {

                        gl_debug (DEBUG_MEDIA_SELECT, "p->data = \"%s\"", p->data);

                        template = gl_template_from_name (p->data);

                        pixbuf = gl_mini_preview_pixbuf_new (template, 72, 72);

                        size = get_label_size_desc (template);
                        layout = get_layout_desc (template);
                        description = g_strdup_printf ("<b>%s</b>\n%s\n%s",
                                                       (gchar *)p->data,
                                                       size,
                                                       layout);
                        g_free (size);
                        g_free (layout);

                        gl_template_free (template);

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
                                               _("Try selecting a different page size or category."));
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

