/*
 *  object-editor.c
 *  Copyright (C) 2003-2009  Jim Evins <evins@snaught.com>.
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

#include "object-editor.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <math.h>

#include "prefs.h"
#include "stock.h"
#include "color-combo.h"
#include "color.h"
#include "wdgt-chain-button.h"
#include "field-button.h"
#include "marshal.h"
#include "combo-util.h"
#include "builder-util.h"
#include "label-box.h"
#include "label-ellipse.h"
#include "label-line.h"
#include "label-image.h"
#include "label-text.h"
#include "label-barcode.h"

#include "object-editor-private.h"

#include "debug.h"


/*===========================================*/
/* Private macros                            */
/*===========================================*/


/*===========================================*/
/* Private data types                        */
/*===========================================*/


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_object_editor_finalize           (GObject              *object);

static void set_object                          (glObjectEditor       *editor,
                                                 glLabelObject        *object);

static void prefs_changed_cb                    (glObjectEditor       *editor);

static void label_changed_cb                    (glLabel              *label,
                                                 glObjectEditor       *editor);

static void selection_changed_cb                (glLabel              *label,
                                                 glObjectEditor       *editor);

static void merge_changed_cb                    (glLabel              *label,
                                                 glObjectEditor       *editor);

static void set_key_names                       (glObjectEditor       *editor,
                                                 glMerge              *merge);

static void object_changed_cb                   (glLabelObject        *object,
                                                 glObjectEditor       *editor);



/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glObjectEditor, gl_object_editor, GTK_TYPE_VBOX);


static void
gl_object_editor_class_init (glObjectEditorClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	gl_debug (DEBUG_EDITOR, "START");
	
  	gl_object_editor_parent_class = g_type_class_peek_parent (class);

  	object_class->finalize = gl_object_editor_finalize;  	
}


static void
gl_object_editor_init (glObjectEditor *editor)
{
        static gchar *object_ids[] = { "editor_vbox",
                                       "adjustment1",  "adjustment2",  "adjustment3",
                                       "adjustment4",  "adjustment5",  "adjustment6",
                                       "adjustment7",  "adjustment8",  "adjustment9",
                                       "adjustment10", "adjustment11", "adjustment12",
                                       "adjustment13",
                                       NULL };
        GError       *error = NULL;
        gchar        *s;

	gl_debug (DEBUG_EDITOR, "START");
	
	editor->priv = g_new0 (glObjectEditorPrivate, 1);

        editor->priv->builder = gtk_builder_new ();

        gtk_builder_add_objects_from_file (editor->priv->builder,
                                           GLABELS_BUILDER_DIR "object-editor.builder",
                                           object_ids,
                                           &error);
	if (error) {
		g_critical ("%s\n\ngLabels may not be installed correctly!", error->message);
                g_error_free (error);
		return;
	}

        gl_builder_util_get_widgets (editor->priv->builder,
                                     "editor_vbox", &editor->priv->editor_vbox,
                                     "title_image", &editor->priv->title_image,
                                     "title_label", &editor->priv->title_label,
                                     "notebook",    &editor->priv->notebook,
                                     NULL);

	gtk_box_pack_start (GTK_BOX(editor),
			    editor->priv->editor_vbox,
			    FALSE, FALSE, 0);

	gtk_widget_show_all (GTK_WIDGET(editor));

        gtk_image_set_from_stock (GTK_IMAGE(editor->priv->title_image),
                                  GL_STOCK_PROPERTIES,
                                  GTK_ICON_SIZE_LARGE_TOOLBAR);

        s = g_strdup_printf ("<span weight=\"bold\">%s</span>",
                             _("Object properties"));
        gtk_label_set_text (GTK_LABEL(editor->priv->title_label), s);
        gtk_label_set_use_markup (GTK_LABEL(editor->priv->title_label), TRUE);
        g_free (s);

        gtk_widget_set_sensitive (editor->priv->title_image, FALSE);
        gtk_widget_set_sensitive (editor->priv->title_label, FALSE);

	/* Hide all notebook pages to start with. */
	gtk_widget_hide_all (editor->priv->notebook);
	gtk_widget_set_no_show_all (editor->priv->notebook, TRUE);

	gl_debug (DEBUG_EDITOR, "END");
}


static void 
gl_object_editor_finalize (GObject *object)
{
	glObjectEditor* editor = GL_OBJECT_EDITOR (object);;
	
	gl_debug (DEBUG_EDITOR, "START");
	
	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_OBJECT_EDITOR (editor));
	g_return_if_fail (editor->priv != NULL);

	g_signal_handlers_disconnect_by_func (G_OBJECT(gl_prefs),
					      prefs_changed_cb, editor);

        if (editor->priv->label)
        {
                g_signal_handlers_disconnect_by_func (G_OBJECT(editor->priv->label),
                                                      label_changed_cb, editor);
                g_signal_handlers_disconnect_by_func (G_OBJECT(editor->priv->label),
                                                      merge_changed_cb, editor);
                g_object_unref (editor->priv->label);
        }

        if (editor->priv->object)
        {
                g_signal_handlers_disconnect_by_func (G_OBJECT(editor->priv->object),
                                                      object_changed_cb, editor);
                g_object_unref (editor->priv->object);
        }

        g_object_unref (editor->priv->builder);

	g_free (editor->priv);

	G_OBJECT_CLASS (gl_object_editor_parent_class)->finalize (object);

	gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* NEW object editor.                                                        */
/*****************************************************************************/
GtkWidget*
gl_object_editor_new (void)
{
	glObjectEditor *editor;

	gl_debug (DEBUG_EDITOR, "START");

	editor = GL_OBJECT_EDITOR (g_object_new (GL_TYPE_OBJECT_EDITOR, NULL));

        gtk_widget_set_sensitive (editor->priv->title_image, FALSE);
        gtk_widget_set_sensitive (editor->priv->title_label, FALSE);

        gl_object_editor_prepare_position_page (editor);
        gl_object_editor_prepare_size_page (editor);
        gl_object_editor_prepare_lsize_page (editor);
        gl_object_editor_prepare_fill_page (editor);
        gl_object_editor_prepare_line_page (editor);
        gl_object_editor_prepare_image_page (editor);
        gl_object_editor_prepare_text_page (editor);
        gl_object_editor_prepare_edit_page (editor);
        gl_object_editor_prepare_bc_page (editor);
        gl_object_editor_prepare_data_page (editor);
        gl_object_editor_prepare_shadow_page (editor);

	g_signal_connect_swapped (G_OBJECT (gl_prefs), "changed",
				  G_CALLBACK (prefs_changed_cb), editor);

	gl_debug (DEBUG_EDITOR, "END");

	return GTK_WIDGET(editor);
}


/*****************************************************************************/
/* Set label.                                                                */
/*****************************************************************************/
void
gl_object_editor_set_label (glObjectEditor  *editor,
                            glLabel         *label)
{
	gl_debug (DEBUG_EDITOR, "START");

        editor->priv->label = g_object_ref (label);

        label_changed_cb (label, editor);
        merge_changed_cb (label, editor);

	g_signal_connect (G_OBJECT(label), "selection_changed",
			  G_CALLBACK(selection_changed_cb), editor);
        g_signal_connect (G_OBJECT (label), "size_changed",
                          G_CALLBACK (label_changed_cb), editor);
        g_signal_connect (G_OBJECT (label), "merge_changed",
                          G_CALLBACK (merge_changed_cb), editor);

	gl_debug (DEBUG_EDITOR, "END");
}


/*--------------------------------------------------------------------------*/
/* Private. Set object.                                                     */
/*--------------------------------------------------------------------------*/
static void
set_object (glObjectEditor  *editor,
            glLabelObject   *object)
{
        gchar         *image;
        gchar         *title;
        gchar         *s;
        GtkTextBuffer *buffer;
        gint           old_page, new_page;

	gl_debug (DEBUG_EDITOR, "START");

        if ( editor->priv->object != NULL )
        {
                g_signal_handlers_disconnect_by_func (G_OBJECT(editor->priv->object),
                                                      object_changed_cb, editor);
                g_object_unref (editor->priv->object);
        }

        if (object)
        {
                editor->priv->object = g_object_ref (object);

                object_changed_cb (object, editor);

                old_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (editor->priv->notebook));

                if (GL_IS_LABEL_BOX (object))
                {
                        image = GL_STOCK_BOX;
                        title = _("Box object properties");

                        gtk_widget_show_all (editor->priv->pos_page_vbox);
                        gtk_widget_show_all (editor->priv->size_page_vbox);
                        gtk_widget_hide     (editor->priv->lsize_page_vbox);
                        gtk_widget_show_all (editor->priv->fill_page_vbox);
                        gtk_widget_show_all (editor->priv->line_page_vbox);
                        gtk_widget_hide     (editor->priv->img_page_vbox);
                        gtk_widget_hide     (editor->priv->text_page_vbox);
                        gtk_widget_hide     (editor->priv->edit_page_vbox);
                        gtk_widget_hide     (editor->priv->bc_page_vbox);
                        gtk_widget_hide     (editor->priv->data_page_vbox);
                        gtk_widget_show_all (editor->priv->shadow_page_vbox);

                        gtk_widget_hide     (editor->priv->size_reset_image_button);
                }
                else if (GL_IS_LABEL_ELLIPSE (object))
                {
                        image = GL_STOCK_ELLIPSE;
                        title = _("Ellipse object properties");

                        gtk_widget_show_all (editor->priv->pos_page_vbox);
                        gtk_widget_show_all (editor->priv->size_page_vbox);
                        gtk_widget_hide     (editor->priv->lsize_page_vbox);
                        gtk_widget_show_all (editor->priv->fill_page_vbox);
                        gtk_widget_show_all (editor->priv->line_page_vbox);
                        gtk_widget_hide     (editor->priv->img_page_vbox);
                        gtk_widget_hide     (editor->priv->text_page_vbox);
                        gtk_widget_hide     (editor->priv->edit_page_vbox);
                        gtk_widget_hide     (editor->priv->bc_page_vbox);
                        gtk_widget_hide     (editor->priv->data_page_vbox);
                        gtk_widget_show_all (editor->priv->shadow_page_vbox);

                        gtk_widget_hide     (editor->priv->size_reset_image_button);
                }
                else if (GL_IS_LABEL_LINE (object))
                {
                        image = GL_STOCK_LINE;
                        title = _("Line object properties");

                        gtk_widget_show_all (editor->priv->pos_page_vbox);
                        gtk_widget_hide     (editor->priv->size_page_vbox);
                        gtk_widget_show_all (editor->priv->lsize_page_vbox);
                        gtk_widget_hide     (editor->priv->fill_page_vbox);
                        gtk_widget_show_all (editor->priv->line_page_vbox);
                        gtk_widget_hide     (editor->priv->img_page_vbox);
                        gtk_widget_hide     (editor->priv->text_page_vbox);
                        gtk_widget_hide     (editor->priv->edit_page_vbox);
                        gtk_widget_hide     (editor->priv->bc_page_vbox);
                        gtk_widget_hide     (editor->priv->data_page_vbox);
                        gtk_widget_show_all (editor->priv->shadow_page_vbox);
                }
                else if (GL_IS_LABEL_IMAGE (object))
                {
                        image = GL_STOCK_IMAGE;
                        title = _("Image object properties");

                        gtk_widget_show_all (editor->priv->pos_page_vbox);
                        gtk_widget_show_all (editor->priv->size_page_vbox);
                        gtk_widget_hide     (editor->priv->lsize_page_vbox);
                        gtk_widget_hide     (editor->priv->fill_page_vbox);
                        gtk_widget_hide     (editor->priv->line_page_vbox);
                        gtk_widget_show_all (editor->priv->img_page_vbox);
                        gtk_widget_hide     (editor->priv->text_page_vbox);
                        gtk_widget_hide     (editor->priv->edit_page_vbox);
                        gtk_widget_hide     (editor->priv->bc_page_vbox);
                        gtk_widget_hide     (editor->priv->data_page_vbox);
                        gtk_widget_hide     (editor->priv->shadow_page_vbox);
                }
                else if (GL_IS_LABEL_TEXT (object))
                {
                        image = GL_STOCK_TEXT;
                        title = _("Text object properties");

                        gtk_widget_show_all (editor->priv->pos_page_vbox);
                        gtk_widget_show_all (editor->priv->size_page_vbox);
                        gtk_widget_hide     (editor->priv->lsize_page_vbox);
                        gtk_widget_hide     (editor->priv->fill_page_vbox);
                        gtk_widget_hide     (editor->priv->line_page_vbox);
                        gtk_widget_hide     (editor->priv->img_page_vbox);
                        gtk_widget_show_all (editor->priv->text_page_vbox);
                        gtk_widget_show_all (editor->priv->edit_page_vbox);
                        gtk_widget_hide     (editor->priv->bc_page_vbox);
                        gtk_widget_hide     (editor->priv->data_page_vbox);
                        gtk_widget_show_all (editor->priv->shadow_page_vbox);

                        gtk_widget_hide     (editor->priv->size_reset_image_button);

                        buffer = gl_label_text_get_buffer (GL_LABEL_TEXT(object));
                        gl_object_editor_set_text_buffer (editor, buffer);
                }
                else if (GL_IS_LABEL_BARCODE (object))
                {
                        image = GL_STOCK_BARCODE;
                        title = _("Barcode object properties");

                        gtk_widget_show_all (editor->priv->pos_page_vbox);
                        gtk_widget_show_all (editor->priv->size_page_vbox);
                        gtk_widget_hide     (editor->priv->lsize_page_vbox);
                        gtk_widget_hide     (editor->priv->fill_page_vbox);
                        gtk_widget_hide     (editor->priv->line_page_vbox);
                        gtk_widget_hide     (editor->priv->img_page_vbox);
                        gtk_widget_hide     (editor->priv->text_page_vbox);
                        gtk_widget_hide     (editor->priv->edit_page_vbox);
                        gtk_widget_show_all (editor->priv->bc_page_vbox);
                        gtk_widget_show_all (editor->priv->data_page_vbox);
                        gtk_widget_hide     (editor->priv->shadow_page_vbox);

                        gtk_widget_hide     (editor->priv->size_reset_image_button);
                }

                gtk_image_set_from_stock (GTK_IMAGE(editor->priv->title_image),
                                          image,
					  GTK_ICON_SIZE_LARGE_TOOLBAR);

		s = g_strdup_printf ("<span weight=\"bold\">%s</span>",
				     title);
		gtk_label_set_text (GTK_LABEL(editor->priv->title_label), s);
		gtk_label_set_use_markup (GTK_LABEL(editor->priv->title_label), TRUE);
		g_free (s);

                gtk_widget_set_sensitive (editor->priv->title_image, TRUE);
                gtk_widget_set_sensitive (editor->priv->title_label, TRUE);

                gtk_widget_show (editor->priv->notebook);

                /* if the old active page is no longer visible, set to 1st visible page. */
                new_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (editor->priv->notebook));
                if ( old_page != new_page )
                {
                        /* Trick: try in reverse order, only the last visible attempt will stick. */
                        gtk_notebook_set_current_page (GTK_NOTEBOOK (editor->priv->notebook), 5);
                        gtk_notebook_set_current_page (GTK_NOTEBOOK (editor->priv->notebook), 4);
                        gtk_notebook_set_current_page (GTK_NOTEBOOK (editor->priv->notebook), 3);
                        gtk_notebook_set_current_page (GTK_NOTEBOOK (editor->priv->notebook), 2);
                        gtk_notebook_set_current_page (GTK_NOTEBOOK (editor->priv->notebook), 1);
                        gtk_notebook_set_current_page (GTK_NOTEBOOK (editor->priv->notebook), 0);
                }

                g_signal_connect (G_OBJECT (object), "changed",
                                  G_CALLBACK (object_changed_cb), editor);
        }
        else
        {
                editor->priv->object = NULL;

                gtk_image_set_from_stock (GTK_IMAGE(editor->priv->title_image),
					  GL_STOCK_PROPERTIES,
					  GTK_ICON_SIZE_LARGE_TOOLBAR);

		s = g_strdup_printf ("<span weight=\"bold\">%s</span>",
				     _("Object properties"));
		gtk_label_set_text (GTK_LABEL(editor->priv->title_label), s);
		gtk_label_set_use_markup (GTK_LABEL(editor->priv->title_label), TRUE);
		g_free (s);

                gtk_widget_set_sensitive (editor->priv->title_image, FALSE);
                gtk_widget_set_sensitive (editor->priv->title_label, FALSE);

                gtk_widget_hide (editor->priv->notebook);
        }

	gl_debug (DEBUG_EDITOR, "END");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE. Prefs changed callback.  Update units related items.            */
/*--------------------------------------------------------------------------*/
static void
prefs_changed_cb (glObjectEditor *editor)
{

	gl_debug (DEBUG_EDITOR, "START");

	if (editor->priv->lsize_r_spin) {
		lsize_prefs_changed_cb (editor);
	}
		
	if (editor->priv->size_w_spin) {
		size_prefs_changed_cb (editor);
	}
		
	if (editor->priv->pos_x_spin) {
		position_prefs_changed_cb (editor);
	}

	if (editor->priv->shadow_x_spin) {
		shadow_prefs_changed_cb (editor);
	}

	gl_debug (DEBUG_EDITOR, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Label "selection state changed" callback.                       */
/*---------------------------------------------------------------------------*/
static void 
selection_changed_cb (glLabel        *label,
		      glObjectEditor *editor)
{
        glLabelObject *object;

	gl_debug (DEBUG_EDITOR, "START");

	g_return_if_fail (label && GL_IS_LABEL (label));
	g_return_if_fail (editor && GL_IS_OBJECT_EDITOR (editor));

	if (!gl_label_is_selection_atomic (label))
        {
                set_object (editor, NULL);
	}
        else
        {
                object = gl_label_get_1st_selected_object (label);
                set_object (editor, object);
	}

	gl_debug (DEBUG_EDITOR, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE. label "size_changed" callback.                                   */
/*---------------------------------------------------------------------------*/
static void
label_changed_cb (glLabel        *label,
                  glObjectEditor *editor)
{
	gdouble   label_width, label_height;

	gl_debug (DEBUG_EDITOR, "START");

	gl_label_get_size (label, &label_width, &label_height);
	gl_object_editor_set_max_position (GL_OBJECT_EDITOR (editor),
					   label_width, label_height);
	gl_object_editor_set_max_size (GL_OBJECT_EDITOR (editor),
				       label_width, label_height);
	gl_object_editor_set_max_lsize (GL_OBJECT_EDITOR (editor),
				       label_width, label_height);
	gl_object_editor_set_max_shadow_offset (GL_OBJECT_EDITOR (editor),
						label_width, label_height);

	gl_debug (DEBUG_EDITOR, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE. label "merge_changed" callback.                                  */
/*---------------------------------------------------------------------------*/
static void
merge_changed_cb (glLabel        *label,
                  glObjectEditor *editor)
{
	glMerge	 *merge;

	gl_debug (DEBUG_EDITOR, "START");

	merge = gl_label_get_merge (label);
	set_key_names (editor, merge);

	gl_debug (DEBUG_EDITOR, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE. Load up possible key names from merge into various widgets.      */
/*---------------------------------------------------------------------------*/
static void
set_key_names (glObjectEditor      *editor,
               glMerge             *merge)
{
        GList     *keys;
	GtkWidget *button;
	gboolean   fixed_flag;
	gboolean   state;
 
        gl_debug (DEBUG_EDITOR, "START");

        gtk_widget_set_sensitive (editor->priv->text_auto_shrink_check,
                                  merge != NULL);
 
        gtk_widget_set_sensitive (editor->priv->text_color_key_radio, merge != NULL);
        if (merge == NULL)
        {
                gtk_toggle_button_set_active (
                        GTK_TOGGLE_BUTTON(editor->priv->text_color_radio), TRUE);
                gtk_widget_set_sensitive (editor->priv->text_color_combo, TRUE);
                gtk_widget_set_sensitive (editor->priv->text_color_key_combo, FALSE);
        }
        else
        {
                state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(editor->priv->text_color_key_radio));
                gtk_widget_set_sensitive (editor->priv->text_color_combo, !state);
                gtk_widget_set_sensitive (editor->priv->text_color_key_combo, state);
	}

        gtk_widget_set_sensitive (editor->priv->edit_insert_field_button,
                                  merge != NULL);

        gtk_widget_set_sensitive (editor->priv->img_key_combo, merge != NULL);
 
        gtk_widget_set_sensitive (editor->priv->img_key_radio, merge != NULL);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(editor->priv->img_file_radio),
                                      merge == NULL);

        gtk_widget_set_sensitive (editor->priv->data_key_combo, merge != NULL);
 
        gtk_widget_set_sensitive (editor->priv->data_key_radio, merge != NULL);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(editor->priv->data_literal_radio),
                                      merge == NULL);
	
	fixed_flag = editor->priv->data_format_fixed_flag;
        gtk_widget_set_sensitive (editor->priv->data_format_label, merge != NULL);
        gtk_widget_set_sensitive (editor->priv->data_ex_label, merge != NULL);
        gtk_widget_set_sensitive (editor->priv->data_digits_label,
                                  (merge != NULL) && !fixed_flag);
        gtk_widget_set_sensitive (editor->priv->data_digits_spin,
                                  (merge != NULL) && !fixed_flag);

        gtk_widget_set_sensitive (editor->priv->fill_key_radio, merge != NULL);
        if (merge == NULL)
        {
                gtk_toggle_button_set_active (
                        GTK_TOGGLE_BUTTON(editor->priv->fill_color_radio), TRUE);
                gtk_widget_set_sensitive (editor->priv->fill_color_combo, TRUE);
                gtk_widget_set_sensitive (editor->priv->fill_key_combo, FALSE);
        }
        else
        {
                state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(editor->priv->fill_key_radio));
                gtk_widget_set_sensitive (editor->priv->fill_color_combo, !state);
                gtk_widget_set_sensitive (editor->priv->fill_key_combo, state);
        }

        gtk_widget_set_sensitive (editor->priv->line_key_radio, merge != NULL);
        if (merge == NULL)
        {
                gtk_toggle_button_set_active (
                        GTK_TOGGLE_BUTTON(editor->priv->line_color_radio), TRUE);
                gtk_widget_set_sensitive (editor->priv->line_color_combo, TRUE);
                gtk_widget_set_sensitive (editor->priv->line_key_combo, FALSE);
        }
        else
        {
                state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(editor->priv->line_key_radio));
                gtk_widget_set_sensitive (editor->priv->line_color_combo, !state);
                gtk_widget_set_sensitive (editor->priv->line_key_combo, state);
        }

        gtk_widget_set_sensitive (editor->priv->bc_key_radio, merge != NULL);
        if (merge == NULL)
        {
                gtk_toggle_button_set_active (
                        GTK_TOGGLE_BUTTON(editor->priv->bc_color_radio), TRUE);
                gtk_widget_set_sensitive (editor->priv->bc_color_combo, TRUE);
                gtk_widget_set_sensitive (editor->priv->bc_key_combo, FALSE);
        }
        else
        {
                state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(editor->priv->bc_key_radio));
                gtk_widget_set_sensitive (editor->priv->bc_color_combo, !state);
                gtk_widget_set_sensitive (editor->priv->bc_key_combo, state);
        }

        gtk_widget_set_sensitive (editor->priv->shadow_key_radio, merge != NULL);
        if (merge == NULL)
        {
                gtk_toggle_button_set_active (
                        GTK_TOGGLE_BUTTON(editor->priv->shadow_color_radio), TRUE);
                gtk_widget_set_sensitive (editor->priv->shadow_color_combo, TRUE);
                gtk_widget_set_sensitive (editor->priv->shadow_key_combo, FALSE);
        }
        else
        {
                state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(editor->priv->shadow_key_radio));
                gtk_widget_set_sensitive (editor->priv->shadow_color_combo, !state);
                gtk_widget_set_sensitive (editor->priv->shadow_key_combo, state);
        }

        keys = gl_merge_get_key_list (merge);
        if ( keys == NULL ) {
                keys = g_list_append (keys, g_strdup (""));
	}

	button = editor->priv->img_key_combo;
        gl_field_button_set_keys (GL_FIELD_BUTTON (button), keys);

	button = editor->priv->edit_insert_field_button;
        gl_field_button_set_keys (GL_FIELD_BUTTON(button), keys);

	button = editor->priv->data_key_combo;
        gl_field_button_set_keys (GL_FIELD_BUTTON (button), keys);
		
	button = editor->priv->fill_key_combo;
        gl_field_button_set_keys (GL_FIELD_BUTTON (button), keys);

	button = editor->priv->text_color_key_combo;
        gl_field_button_set_keys (GL_FIELD_BUTTON (button), keys);

	button = editor->priv->line_key_combo;
        gl_field_button_set_keys (GL_FIELD_BUTTON (button), keys);
		
	button = editor->priv->bc_key_combo;
        gl_field_button_set_keys (GL_FIELD_BUTTON (button), keys);
		
	button = editor->priv->shadow_key_combo;
        gl_field_button_set_keys (GL_FIELD_BUTTON (button), keys);

	gl_merge_free_key_list (&keys);
 
        gl_debug (DEBUG_EDITOR, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE. object "changed" callback.                                       */
/*---------------------------------------------------------------------------*/
static void
object_changed_cb (glLabelObject  *object,
                   glObjectEditor *editor)
{
        gdouble          x, y;
        gdouble          w, h;
        glColorNode     *line_color_node;
        gdouble          line_width;
        glColorNode     *fill_color_node;
        gchar           *font_family;
        gdouble          font_size;
        PangoWeight      font_weight;
        gboolean         font_italic_flag;
        glColorNode     *text_color_node;
        PangoAlignment   align;
        gdouble          text_line_spacing;
        gboolean         auto_shrink;
        const GdkPixbuf *pixbuf;
        gdouble          image_w, image_h;
        glTextNode      *filename;
        glTextNode      *bc_data;
        gchar           *id;
        gboolean         text_flag, cs_flag;
        guint            format_digits;
        gboolean         shadow_state;
        gdouble          shadow_x, shadow_y;
        glColorNode     *shadow_color_node;
        gdouble          shadow_opacity;
        glMerge         *merge;

        gl_debug (DEBUG_EDITOR, "BEGIN");

        if (editor->priv->stop_signals) return;
        editor->priv->stop_signals = TRUE;


        gl_label_object_get_position (object, &x, &y);
        gl_object_editor_set_position (editor, x, y);

        merge = gl_label_get_merge (GL_LABEL(object->parent));


        if ( GL_IS_LABEL_BOX (object) || GL_IS_LABEL_ELLIPSE (object) )
        {

                gl_label_object_get_size (object, &w, &h);
                fill_color_node   = gl_label_object_get_fill_color (GL_LABEL_OBJECT(object));
                line_color_node   = gl_label_object_get_line_color (GL_LABEL_OBJECT(object));
                line_width        = gl_label_object_get_line_width (GL_LABEL_OBJECT(object));

                gl_object_editor_set_size (editor, w, h);
                gl_object_editor_set_fill_color (editor, (merge != NULL), fill_color_node);
                gl_object_editor_set_line_color (editor, (merge != NULL), line_color_node);
                gl_object_editor_set_line_width (editor, line_width);

                gl_color_node_free (&fill_color_node);
                gl_color_node_free (&line_color_node);

        }
        else if ( GL_IS_LABEL_LINE (object) )
        {

                gl_label_object_get_size (object, &w, &h);
                line_color_node   = gl_label_object_get_line_color (GL_LABEL_OBJECT(object));
                line_width        = gl_label_object_get_line_width (GL_LABEL_OBJECT(object));

                gl_object_editor_set_lsize (editor, w, h);
                gl_object_editor_set_line_color (editor, (merge != NULL), line_color_node);
                gl_object_editor_set_line_width (editor, line_width);

                gl_color_node_free (&line_color_node);

        }
        else if ( GL_IS_LABEL_IMAGE (object) )
        {

                gl_label_object_get_size (object, &w, &h);
                pixbuf   = gl_label_image_get_pixbuf (GL_LABEL_IMAGE(object), NULL);
                filename = gl_label_image_get_filename (GL_LABEL_IMAGE(object));

                image_w = gdk_pixbuf_get_width (pixbuf);
                image_h = gdk_pixbuf_get_height (pixbuf);

                gl_object_editor_set_size (editor, w, h);
                gl_object_editor_set_base_size (editor, image_w, image_h);
                gl_object_editor_set_image (editor, (merge != NULL), filename);

                gl_text_node_free (&filename);

        }
        else if ( GL_IS_LABEL_TEXT (object) )
        {

                gl_label_object_get_size (object, &w, &h);
                font_family       = gl_label_object_get_font_family (object);
                font_size         = gl_label_object_get_font_size (object);
                font_weight       = gl_label_object_get_font_weight (object);
                font_italic_flag  = gl_label_object_get_font_italic_flag (object);
                text_color_node   = gl_label_object_get_text_color (object);
                align             = gl_label_object_get_text_alignment (object);
                text_line_spacing = gl_label_object_get_text_line_spacing (object);
                auto_shrink       = gl_label_text_get_auto_shrink (GL_LABEL_TEXT (object));

                gl_object_editor_set_size (editor, w, h);
                gl_object_editor_set_font_family (editor, font_family);
                gl_object_editor_set_font_size (editor, font_size);
                gl_object_editor_set_font_weight (editor, font_weight);
                gl_object_editor_set_font_italic_flag (editor, font_italic_flag);
                gl_object_editor_set_text_color (editor, (merge != NULL), text_color_node);
                gl_object_editor_set_text_alignment (editor, align);
                gl_object_editor_set_text_line_spacing (editor, text_line_spacing);
                gl_object_editor_set_text_auto_shrink (editor, auto_shrink);

                gl_color_node_free (&text_color_node);
                g_free (font_family);

        }
        else if ( GL_IS_LABEL_BARCODE (object) )
        {

                gl_label_object_get_size (object, &w, &h);
                bc_data = gl_label_barcode_get_data (GL_LABEL_BARCODE(object));
                gl_label_barcode_get_props (GL_LABEL_BARCODE(object),
                                            &id, &text_flag, &cs_flag, &format_digits);
                line_color_node   = gl_label_object_get_line_color (GL_LABEL_OBJECT(object));

                gl_object_editor_set_size (editor, w, h);
                gl_object_editor_set_data (editor, (merge != NULL), bc_data);
                gl_object_editor_set_bc_style (editor, id, text_flag, cs_flag, format_digits);
                gl_object_editor_set_bc_color (editor, (merge != NULL), line_color_node);

                gl_text_node_free (&bc_data);

        }


        shadow_state      = gl_label_object_get_shadow_state (object);
        gl_label_object_get_shadow_offset (object, &shadow_x, &shadow_y);
        shadow_color_node = gl_label_object_get_shadow_color (object);
        shadow_opacity    = gl_label_object_get_shadow_opacity (object);

        gl_object_editor_set_shadow_state (editor, shadow_state);
        gl_object_editor_set_shadow_offset (editor, shadow_x, shadow_y);
        gl_object_editor_set_shadow_color (editor, (merge != NULL), shadow_color_node);
        gl_object_editor_set_shadow_opacity (editor, shadow_opacity);

        gl_color_node_free (&shadow_color_node);


        editor->priv->stop_signals = FALSE;

        gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Object editor "changed" callback.                                         */
/*****************************************************************************/
void
gl_object_editor_changed_cb (glObjectEditor *editor)
{
        glLabelObject     *object = editor->priv->object;
        gdouble            x, y;
        gdouble            w, h;
        glColorNode       *line_color_node;
        gdouble            line_width;
        glColorNode       *fill_color_node;
        gchar             *font_family;
        gdouble            font_size;
        PangoWeight        font_weight;
        gboolean           font_italic_flag;
        glColorNode       *text_color_node;
        PangoAlignment     align;
        gdouble            text_line_spacing;
        gboolean           auto_shrink;
        glTextNode        *filename;
        const GdkPixbuf   *pixbuf;
        gdouble            image_w, image_h;
        gdouble            new_w, new_h;
        glTextNode        *bc_data;
        gchar             *id;
        gboolean           text_flag, cs_flag;
        guint              format_digits;
        gboolean           shadow_state;
        gdouble            shadow_x, shadow_y;
        glColorNode       *shadow_color_node;
        gdouble            shadow_opacity;

        gl_debug (DEBUG_EDITOR, "BEGIN");

        if (editor->priv->stop_signals) return;
        editor->priv->stop_signals = TRUE;


        gl_object_editor_get_position (editor, &x, &y);
        gl_label_object_set_position (object, x, y);


        if ( GL_IS_LABEL_BOX (object) || GL_IS_LABEL_ELLIPSE (object) )
        {

                gl_object_editor_get_size (editor, &w, &h);
                fill_color_node   = gl_object_editor_get_fill_color (editor);
                line_color_node   = gl_object_editor_get_line_color (editor);
                line_width        = gl_object_editor_get_line_width (editor);

                gl_label_object_set_size (object, w, h);
                gl_label_object_set_fill_color (object, fill_color_node);
                gl_label_object_set_line_color (object, line_color_node);
                gl_label_object_set_line_width (object, line_width);

                gl_color_node_free (&fill_color_node);
                gl_color_node_free (&line_color_node);

        }
        else if (GL_IS_LABEL_LINE (object))
        {

                gl_object_editor_get_lsize (editor, &w, &h);
                line_color_node   = gl_object_editor_get_line_color (editor);
                line_width        = gl_object_editor_get_line_width (editor);

                gl_label_object_set_size (object, w, h);
                gl_label_object_set_line_color (object, line_color_node);
                gl_label_object_set_line_width (object, line_width);

                gl_color_node_free (&line_color_node);

        }
        else if ( GL_IS_LABEL_IMAGE (object) )
        {

                gl_object_editor_get_size (editor, &w, &h);
                filename = gl_object_editor_get_image (editor);

                gl_label_object_set_size (object, w, h);
                gl_label_image_set_filename (GL_LABEL_IMAGE(object), filename);

                /* Setting filename may have modified the size. */
                gl_label_object_get_size (object, &new_w, &new_h);
                if ( (new_w != w) || (new_h != h) )
                {
                        gl_object_editor_set_size (editor, new_w, new_h);
                }

                /* It may also have a new base size. */
                pixbuf = gl_label_image_get_pixbuf (GL_LABEL_IMAGE(object), NULL);
                image_w = gdk_pixbuf_get_width (pixbuf);
                image_h = gdk_pixbuf_get_height (pixbuf);
                gl_object_editor_set_base_size (editor, image_w, image_h);

                gl_text_node_free (&filename);

        }
        else if (GL_IS_LABEL_TEXT (object))
        {

                gl_object_editor_get_size (editor, &w, &h);
                font_family       = gl_object_editor_get_font_family (editor);
                font_size         = gl_object_editor_get_font_size (editor);
                font_weight       = gl_object_editor_get_font_weight (editor);
                font_italic_flag  = gl_object_editor_get_font_italic_flag (editor);
                text_color_node   = gl_object_editor_get_text_color (editor);
                align             = gl_object_editor_get_text_alignment (editor);
                text_line_spacing = gl_object_editor_get_text_line_spacing (editor);
                auto_shrink       = gl_object_editor_get_text_auto_shrink (editor);

                gl_label_object_set_size (object, w, h);
                gl_label_object_set_font_family (object, font_family);
                gl_label_object_set_font_size (object, font_size);
                gl_label_object_set_font_weight (object, font_weight);
                gl_label_object_set_font_italic_flag (object, font_italic_flag);
                gl_label_object_set_text_color (object, text_color_node);
                gl_label_object_set_text_alignment (object, align);
                gl_label_object_set_text_line_spacing (object, text_line_spacing);
                gl_label_text_set_auto_shrink (GL_LABEL_TEXT (object), auto_shrink);

                gl_color_node_free (&text_color_node);
                g_free (font_family);

        }
        else if (GL_IS_LABEL_BARCODE (object))
        {

                gl_object_editor_get_size (editor, &w, &h);
                line_color_node = gl_object_editor_get_bc_color (editor);
                bc_data = gl_object_editor_get_data (editor);
                gl_object_editor_get_bc_style (editor,
                                               &id, &text_flag, &cs_flag, &format_digits);

                gl_label_object_set_size (object, w, h);
                gl_label_object_set_line_color (object, line_color_node);
                gl_label_barcode_set_data (GL_LABEL_BARCODE(object), bc_data);
                gl_label_barcode_set_props (GL_LABEL_BARCODE(object),
                                            id, text_flag, cs_flag, format_digits);

                gl_color_node_free (&line_color_node);
                gl_text_node_free (&bc_data);
                g_free (id);

        }


        shadow_state      = gl_object_editor_get_shadow_state (editor);
        gl_object_editor_get_shadow_offset (editor, &shadow_x, &shadow_y);
        shadow_color_node = gl_object_editor_get_shadow_color (editor);
        shadow_opacity    = gl_object_editor_get_shadow_opacity (editor);

        gl_label_object_set_position (object, x, y);
        gl_label_object_set_shadow_state (object, shadow_state);
        gl_label_object_set_shadow_offset (object, shadow_x, shadow_y);
        gl_label_object_set_shadow_color (object, shadow_color_node);
        gl_label_object_set_shadow_opacity (object, shadow_opacity);

        gl_color_node_free (&shadow_color_node);


        editor->priv->stop_signals = FALSE;

        gl_debug (DEBUG_EDITOR, "END");
}


/*****************************************************************************/
/* Object editor "size_changed" callback.                                    */
/*****************************************************************************/
void
gl_object_editor_size_changed_cb (glObjectEditor *editor)
{
        glLabelObject     *object = editor->priv->object;
        gdouble            w, h;

        gl_debug (DEBUG_EDITOR, "BEGIN");

        if (editor->priv->stop_signals) return;
        editor->priv->stop_signals = TRUE;


        if ( GL_IS_LABEL_LINE (object) )
        {
                gl_object_editor_get_lsize (editor, &w, &h);
        }
        else
        {
                gl_object_editor_get_size (editor, &w, &h);
        }

        gl_label_object_set_size (object, w, h);


        editor->priv->stop_signals = FALSE;

        gl_debug (DEBUG_EDITOR, "END");
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
