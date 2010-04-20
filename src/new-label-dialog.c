/*
 *  new-label-dialog.c
 *  Copyright (C) 2006-2009  Jim Evins <evins@snaught.com>.
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

#include "new-label-dialog.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "marshal.h"
#include "builder-util.h"
#include "prefs.h"
#include "media-select.h"
#include "mini-label-preview.h"
#include "mini-preview.h"

#include "debug.h"


#define LABEL_PREVIEW_WIDTH  96
#define LABEL_PREVIEW_HEIGHT 96

#define MINI_PREVIEW_MIN_WIDTH  300
#define MINI_PREVIEW_MIN_HEIGHT 360


/*===========================================*/
/* Private data types                        */
/*===========================================*/

struct _glNewLabelDialogPrivate {

        GtkBuilder *builder;

        GtkWidget  *template_page_vbox;
        GtkWidget  *combo_hbox;
        GtkWidget  *combo;

        GtkWidget  *rotate_page_vbox;
        GtkWidget  *normal_radio;
        GtkWidget  *rotated_radio;
        GtkWidget  *normal_preview_hbox;
        GtkWidget  *rotated_preview_hbox;
        GtkWidget  *normal_preview;
        GtkWidget  *rotated_preview;

        GtkWidget  *confirm_page_vbox;
        GtkWidget  *preview_vbox;
        GtkWidget  *preview;
        GtkWidget  *desc_label;
        GtkWidget  *page_size_label;
        GtkWidget  *label_size_label;
        GtkWidget  *layout_label;
        GtkWidget  *vendor_label;
        GtkWidget  *part_label;
        GtkWidget  *similar_label;
};

/* Page numbers for traversing GtkAssistant */
enum {
        TEMPLATE_PAGE_NUM = 0,
        ROTATE_PAGE_NUM,
        CONFIRM_PAGE_NUM
};

enum {
        COMPLETE,
        LAST_SIGNAL
};


/*===========================================*/
/* Private globals                           */
/*===========================================*/

static gint signals[LAST_SIGNAL] = { 0 };


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void       gl_new_label_dialog_finalize        (GObject           *object);

static void       cancel_cb                           (glNewLabelDialog  *this);
static void       apply_cb                            (glNewLabelDialog  *this);
static void       close_cb                            (glNewLabelDialog  *this);

static gint       forward_page_function               (gint               current_page,
                                                       glNewLabelDialog  *this);

static void       combo_changed_cb                    (glNewLabelDialog  *this);
static void       rotate_toggled_cb                   (glNewLabelDialog  *this);

static gchar     *get_default_name                    (void);

static void       set_info                            (glNewLabelDialog  *this,
                                                       const gchar       *name);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glNewLabelDialog, gl_new_label_dialog, GTK_TYPE_ASSISTANT);


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
gl_new_label_dialog_class_init (glNewLabelDialogClass *class)
{
        GObjectClass *object_class = G_OBJECT_CLASS (class);

        gl_debug (DEBUG_FILE, "");

        gl_new_label_dialog_parent_class = g_type_class_peek_parent (class);

        object_class->finalize = gl_new_label_dialog_finalize;

        signals[COMPLETE] =
            g_signal_new ("complete",
                          G_OBJECT_CLASS_TYPE(object_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (glNewLabelDialogClass, complete),
                          NULL, NULL,
                          gl_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
gl_new_label_dialog_init (glNewLabelDialog *this)
{
        GtkWidget    *vbox;
        gchar        *logo_filename;
        GdkPixbuf    *logo;
        GtkBuilder   *builder;
        gchar        *builder_filename;
        static gchar *object_ids[] = { "template_page_vbox",
                                       "rotate_page_vbox",
                                       "confirm_page_vbox",
                                       NULL };
        GError       *error = NULL;
        GtkWidget    *new_label_dialog_hbox;

        gl_debug (DEBUG_FILE, "START");

        g_return_if_fail (GL_IS_NEW_LABEL_DIALOG (this));

        this->priv = g_new0 (glNewLabelDialogPrivate, 1);

        builder = gtk_builder_new ();
        builder_filename = g_build_filename (GLABELS_DATA_DIR, "builder", "new-label-dialog.builder", NULL);
        gtk_builder_add_objects_from_file (builder, builder_filename, object_ids, &error);
        g_free (builder_filename);
        if (error) {
                g_critical ("%s\n\ngLabels may not be installed correctly!", error->message);
                g_error_free (error);
                return;
        }

        gl_builder_util_get_widgets (builder,
                                     "template_page_vbox",     &this->priv->template_page_vbox,
                                     "combo_hbox",             &this->priv->combo_hbox,
                                     "rotate_page_vbox",       &this->priv->rotate_page_vbox,
                                     "normal_radio",           &this->priv->normal_radio,
                                     "rotated_radio",          &this->priv->rotated_radio,
                                     "normal_preview_hbox",    &this->priv->normal_preview_hbox,
                                     "rotated_preview_hbox",   &this->priv->rotated_preview_hbox,
                                     "confirm_page_vbox",      &this->priv->confirm_page_vbox,
                                     "preview_vbox",           &this->priv->preview_vbox,
                                     "desc_label",             &this->priv->desc_label,
                                     "page_size_label",        &this->priv->page_size_label,
                                     "label_size_label",       &this->priv->label_size_label,
                                     "layout_label",           &this->priv->layout_label,
                                     "vendor_label",           &this->priv->vendor_label,
                                     "part_label",             &this->priv->part_label,
                                     "similar_label",          &this->priv->similar_label,
                                     NULL);

        this->priv->builder = builder;

        gtk_assistant_append_page (GTK_ASSISTANT (this), this->priv->template_page_vbox);
        gtk_assistant_set_page_title (GTK_ASSISTANT (this), this->priv->template_page_vbox, _("Select Product"));
        gtk_assistant_set_page_type (GTK_ASSISTANT (this), this->priv->template_page_vbox, GTK_ASSISTANT_PAGE_INTRO);
        logo_filename = g_build_filename (GLABELS_DATA_DIR, "pixmaps", "new-select.png", NULL);
        logo = gdk_pixbuf_new_from_file (logo_filename, NULL);
        gtk_assistant_set_page_header_image (GTK_ASSISTANT (this), this->priv->template_page_vbox, logo);
        g_free (logo_filename);
        g_object_unref (logo);

        this->priv->combo = gl_media_select_new ();
        gtk_container_add (GTK_CONTAINER (this->priv->combo_hbox), this->priv->combo);

        gtk_assistant_append_page (GTK_ASSISTANT (this), this->priv->rotate_page_vbox);
        gtk_assistant_set_page_title (GTK_ASSISTANT (this), this->priv->rotate_page_vbox, _("Choose Orientation"));
        gtk_assistant_set_page_complete (GTK_ASSISTANT (this), this->priv->rotate_page_vbox, TRUE);
        logo_filename = g_build_filename (GLABELS_DATA_DIR, "pixmaps", "new-rotate.png", NULL);
        logo = gdk_pixbuf_new_from_file (logo_filename, NULL);
        gtk_assistant_set_page_header_image (GTK_ASSISTANT (this), this->priv->rotate_page_vbox, logo);
        g_free (logo_filename);
        g_object_unref (logo);

        this->priv->normal_preview = gl_mini_label_preview_new (LABEL_PREVIEW_WIDTH, LABEL_PREVIEW_HEIGHT);
        gtk_container_add (GTK_CONTAINER (this->priv->normal_preview_hbox), this->priv->normal_preview);
        this->priv->rotated_preview = gl_mini_label_preview_new (LABEL_PREVIEW_WIDTH, LABEL_PREVIEW_HEIGHT);
        gtk_container_add (GTK_CONTAINER (this->priv->rotated_preview_hbox), this->priv->rotated_preview);

        gtk_assistant_append_page (GTK_ASSISTANT (this), this->priv->confirm_page_vbox);
        gtk_assistant_set_page_title (GTK_ASSISTANT (this), this->priv->confirm_page_vbox, _("Review"));
        gtk_assistant_set_page_type (GTK_ASSISTANT (this), this->priv->confirm_page_vbox, GTK_ASSISTANT_PAGE_CONFIRM);
        gtk_assistant_set_page_complete (GTK_ASSISTANT (this), this->priv->confirm_page_vbox, TRUE);
        logo_filename = g_build_filename (GLABELS_DATA_DIR, "pixmaps", "new-confirm.png", NULL);
        logo = gdk_pixbuf_new_from_file (logo_filename, NULL);
        gtk_assistant_set_page_header_image (GTK_ASSISTANT (this), this->priv->confirm_page_vbox, logo);
        g_free (logo_filename);
        g_object_unref (logo);

        this->priv->preview = gl_mini_preview_new (MINI_PREVIEW_MIN_HEIGHT, MINI_PREVIEW_MIN_WIDTH);
        gl_mini_preview_set_draw_arrow (GL_MINI_PREVIEW (this->priv->preview), TRUE);
        gl_mini_preview_set_rotate (GL_MINI_PREVIEW (this->priv->preview), FALSE);
        gtk_container_add (GTK_CONTAINER (this->priv->preview_vbox), this->priv->preview);

        gtk_assistant_set_forward_page_func (GTK_ASSISTANT (this), (GtkAssistantPageFunc)forward_page_function, this, NULL);

        g_signal_connect_swapped (G_OBJECT(this), "cancel", G_CALLBACK(cancel_cb), this);
        g_signal_connect_swapped (G_OBJECT(this), "apply",  G_CALLBACK(apply_cb),  this);
        g_signal_connect_swapped (G_OBJECT(this), "close",  G_CALLBACK(close_cb),  this);

        g_signal_connect_swapped (G_OBJECT (this->priv->combo),         "changed", G_CALLBACK (combo_changed_cb),  this);
        g_signal_connect_swapped (G_OBJECT (this->priv->normal_radio),  "toggled", G_CALLBACK (rotate_toggled_cb), this);
        g_signal_connect_swapped (G_OBJECT (this->priv->rotated_radio), "toggled", G_CALLBACK (rotate_toggled_cb), this);

        combo_changed_cb (this);

        gl_debug (DEBUG_FILE, "END");
}


/*****************************************************************************/
/* Finalize Function.                                                        */
/*****************************************************************************/
static void 
gl_new_label_dialog_finalize (GObject *object)
{
        glNewLabelDialog* this = GL_NEW_LABEL_DIALOG (object);;

        gl_debug (DEBUG_FILE, "START");

        g_return_if_fail (object != NULL);
        g_return_if_fail (GL_IS_NEW_LABEL_DIALOG (this));
        g_return_if_fail (this->priv != NULL);

        g_object_unref (G_OBJECT (this->priv->builder));
        g_free (this->priv);

        G_OBJECT_CLASS (gl_new_label_dialog_parent_class)->finalize (object);

        gl_debug (DEBUG_FILE, "END");

}


/*****************************************************************************/
/* NEW object properties dialog.                                             */
/*****************************************************************************/
GtkWidget *
gl_new_label_dialog_new (GtkWindow    *win)
{
        GtkWidget *this;

        gl_debug (DEBUG_FILE, "");

        this = GTK_WIDGET (g_object_new (GL_TYPE_NEW_LABEL_DIALOG, NULL));

        gtk_window_set_transient_for (GTK_WINDOW (this), win);

        return this;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Handle non-linear forward traversal.                           */
/*--------------------------------------------------------------------------*/
static gint
forward_page_function (gint              current_page,
                       glNewLabelDialog *this)
{
        gchar                     *name;
        lglTemplate               *template;
        const lglTemplateFrame    *frame;
        gdouble                    w, h;

        switch (current_page)
        {
        case TEMPLATE_PAGE_NUM:
                name = gl_media_select_get_name (GL_MEDIA_SELECT (this->priv->combo));
                if ( name != NULL )
                {
                        template = lgl_db_lookup_template_from_name (name);
                        frame    = (lglTemplateFrame *)template->frames->data;
                        lgl_template_frame_get_size (frame, &w, &h);

                        if ( w == h )
                        {
                                /* Skip rotate page for square and circular labels. */
                                return CONFIRM_PAGE_NUM;
                        }
                }
                return ROTATE_PAGE_NUM;

        case ROTATE_PAGE_NUM:
                return CONFIRM_PAGE_NUM;

        case CONFIRM_PAGE_NUM:
        default:
                return -1;
        }

        return -1;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  cancel callback.                                               */
/*--------------------------------------------------------------------------*/
static void
cancel_cb (glNewLabelDialog *this)
{
                                                                               
        gtk_widget_destroy (GTK_WIDGET(this));

}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  apply callback                                                 */
/*--------------------------------------------------------------------------*/
static void
apply_cb (glNewLabelDialog *this)
{

        g_signal_emit (G_OBJECT (this), signals[COMPLETE], 0);

}

                         
/*--------------------------------------------------------------------------*/
/* PRIVATE.  close callback                                                 */
/*--------------------------------------------------------------------------*/
static void
close_cb (glNewLabelDialog *this)
{
                                                                               
        gtk_widget_destroy (GTK_WIDGET(this));

}

                         
/*---------------------------------------------------------------------------*/
/* PRIVATE.  Template changed callback.                                      */
/*---------------------------------------------------------------------------*/
static void
combo_changed_cb (glNewLabelDialog  *this)
{
        gchar             *name;

        gl_debug (DEBUG_FILE, "START");

        name = gl_media_select_get_name (GL_MEDIA_SELECT (this->priv->combo));

        gl_mini_label_preview_set_by_name (GL_MINI_LABEL_PREVIEW (this->priv->normal_preview),  name, FALSE);
        gl_mini_label_preview_set_by_name (GL_MINI_LABEL_PREVIEW (this->priv->rotated_preview), name, TRUE);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (this->priv->normal_radio), TRUE);

        gl_mini_preview_set_by_name (GL_MINI_PREVIEW (this->priv->preview), name);
        set_info (this, name);

        gtk_assistant_set_page_complete (GTK_ASSISTANT (this), this->priv->template_page_vbox, (name != NULL));

        g_free (name);

        gl_debug (DEBUG_FILE, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Rotate toggled callback.                                        */
/*---------------------------------------------------------------------------*/
static void
rotate_toggled_cb (glNewLabelDialog  *this)
{
        gboolean state;

        gl_debug (DEBUG_FILE, "START");

        state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (this->priv->rotated_radio));
        gl_mini_preview_set_rotate (GL_MINI_PREVIEW (this->priv->preview), state);

        gl_debug (DEBUG_FILE, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set information labels.                                         */
/*---------------------------------------------------------------------------*/
static void
set_info (glNewLabelDialog  *this,
          const gchar       *name)
{
        lglTemplate          *template;
        lglTemplateFrame     *frame;
        lglVendor            *vendor;
        lglUnits              units;
        gchar                *page_size_string;
        gchar                *label_size_string;
        gchar                *layout_string;
        GList                *list, *p;
        GString              *list_string;

        template = lgl_db_lookup_template_from_name (name);
        frame    = template->frames->data;
        vendor   = lgl_db_lookup_vendor_from_name (template->brand);

        units    = gl_prefs_model_get_units (gl_prefs);

        page_size_string  = lgl_db_lookup_paper_name_from_id (template->paper_id);
        label_size_string = lgl_template_frame_get_size_description (frame, units);
        layout_string     = lgl_template_frame_get_layout_description (frame);

        gtk_label_set_text (GTK_LABEL (this->priv->desc_label),       template->description);
        gtk_label_set_text (GTK_LABEL (this->priv->page_size_label),  page_size_string);
        gtk_label_set_text (GTK_LABEL (this->priv->label_size_label), label_size_string);
        gtk_label_set_text (GTK_LABEL (this->priv->layout_label),     layout_string);

        if ( vendor && vendor->url )
        {
                gchar *escaped_url;
                gchar *markup;

                escaped_url = g_markup_escape_text (vendor->url, -1);
                markup = g_strdup_printf ("<a href='%s'>%s</a>", escaped_url, vendor->name);
                gtk_label_set_markup (GTK_LABEL (this->priv->vendor_label), markup);
                g_free (escaped_url);
                g_free (markup);
        }
        else
        {
                /* FIXME: Using set_markup instead of set_text to clear out previous link. */
                gtk_label_set_markup (GTK_LABEL (this->priv->vendor_label), template->brand);
        }

        if ( template->product_url )
        {
                gchar *escaped_url;
                gchar *markup;

                escaped_url = g_markup_escape_text (template->product_url, -1);
                markup = g_strdup_printf ("<a href='%s'>%s</a>", escaped_url, template->part);
                gtk_label_set_markup (GTK_LABEL (this->priv->part_label), markup);
                g_free (escaped_url);
                g_free (markup);
        }
        else
        {
                /* FIXME: Using set_markup instead of set_text to clear out previous link. */
                gtk_label_set_markup (GTK_LABEL (this->priv->part_label), template->part);
        }

        list = lgl_db_get_similar_template_name_list (name);
        list_string = g_string_new ("");
        for ( p = list; p; p = p->next )
        {
                g_string_append (list_string, (char *)p->data);
                if ( p->next )
                {
                        g_string_append (list_string, "\n");
                }
        }
        gtk_label_set_text (GTK_LABEL (this->priv->similar_label), list_string->str);

        lgl_db_free_template_name_list (list);
        g_string_free (list_string, TRUE);

        g_free (page_size_string);
        g_free (label_size_string);
        g_free (layout_string);
}


/*****************************************************************************/
/* Get template name.                                                        */
/*****************************************************************************/
gchar *
gl_new_label_dialog_get_template_name (glNewLabelDialog *this)
{
        gchar *name;

        name = gl_media_select_get_name (GL_MEDIA_SELECT (this->priv->combo));

        return name;
}


/*****************************************************************************/
/* Set template name.                                                        */
/*****************************************************************************/
void
gl_new_label_dialog_set_template_name (glNewLabelDialog *this,
                                       gchar            *name)
{
        gl_mini_preview_set_by_name (GL_MINI_PREVIEW (this->priv->preview), name);
        gl_media_select_set_name (GL_MEDIA_SELECT (this->priv->combo), name);
        set_info (this, name);
}


/*****************************************************************************/
/* Get current filter parameters.                                            */
/*****************************************************************************/
void
gl_new_label_dialog_get_filter_parameters (glNewLabelDialog  *this,
                                           gchar            **page_size_id,
                                           gchar            **category_id)
{
}


/*****************************************************************************/
/* Set current filter parameters.                                            */
/*****************************************************************************/
void
gl_new_label_dialog_set_filter_parameters (glNewLabelDialog *this,
                                           const gchar      *page_size_id,
                                           const gchar      *category_id)
{
}


/*****************************************************************************/
/* Get rotate state.                                                         */
/*****************************************************************************/
gboolean
gl_new_label_dialog_get_rotate_state (glNewLabelDialog *this)
{
        return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (this->priv->rotated_radio));
}


/*****************************************************************************/
/* Set rotate state.                                                         */
/*****************************************************************************/
void
gl_new_label_dialog_set_rotate_state (glNewLabelDialog *this,
                                      gboolean          state)
{
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (this->priv->rotated_radio), state);
        gl_mini_preview_set_rotate (GL_MINI_PREVIEW (this->priv->preview), state);
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
