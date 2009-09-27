/*
 *  print-op-dialog.c
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

#include "print-op-dialog.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#include "mini-preview.h"
#include "label.h"
#include "builder-util.h"

#include "pixmaps/collate.xpm"
#include "pixmaps/nocollate.xpm"

#include "debug.h"


#define MINI_PREVIEW_MIN_HEIGHT 175
#define MINI_PREVIEW_MIN_WIDTH  150


/*===========================================*/
/* Private data types                        */
/*===========================================*/

struct _glPrintOpDialogPrivate {

        GtkBuilder *builder;

	GtkWidget  *simple_frame;
	GtkWidget  *simple_sheets_radio;
	GtkWidget  *simple_sheets_spin;
	GtkWidget  *simple_labels_radio;
	GtkWidget  *simple_first_spin;
	GtkWidget  *simple_last_spin;

	GtkWidget  *merge_frame;
	GtkWidget  *merge_first_spin;
	GtkWidget  *merge_copies_spin;
	GtkWidget  *merge_collate_check;
	GtkWidget  *merge_collate_image;

	GtkWidget  *outline_check;
	GtkWidget  *reverse_check;
	GtkWidget  *crop_marks_check;

        GtkWidget  *preview;

        gboolean   force_outline_flag;

        gint       labels_per_sheet;
        gboolean   merge_flag;
        gint       n_records;

};


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void     gl_print_op_dialog_finalize   (GObject           *object);

static void     gl_print_op_dialog_construct  (glPrintOpDialog   *op,
                                               glLabel           *label);

static GObject *create_custom_widget_cb       (GtkPrintOperation *operation,
                                               glLabel           *label);

static void     custom_widget_apply_cb        (GtkPrintOperation *operation,
                                               GtkWidget         *widget,
                                               glLabel           *label);

static void     simple_sheets_radio_cb        (GtkToggleButton   *togglebutton,
                                               glPrintOpDialog   *op);

static void     simple_first_spin_cb          (GtkSpinButton     *spinbutton,
                                               glPrintOpDialog   *op);

static void     simple_last_spin_cb           (GtkSpinButton     *spinbutton,
                                               glPrintOpDialog   *op);

static void     preview_pressed_cb            (glMiniPreview     *preview,
                                               gint               first,
                                               gint               last,
                                               glPrintOpDialog   *op);

static void     preview_released_cb           (glMiniPreview     *preview,
                                               gint               first,
                                               gint               last,
                                               glPrintOpDialog   *op);

static void     merge_spin_cb                 (GtkSpinButton     *spinbutton,
                                               glPrintOpDialog   *op);

static void     merge_collate_check_cb        (GtkToggleButton   *togglebutton,
                                               glPrintOpDialog   *op);

static void     preview_clicked_cb            (glMiniPreview     *preview,
                                               gint               first,
                                               glPrintOpDialog   *op);

static void     option_toggled_cb             (GtkToggleButton   *togglebutton,
                                               glPrintOpDialog   *op);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glPrintOpDialog, gl_print_op_dialog, GL_TYPE_PRINT_OP);


static void
gl_print_op_dialog_class_init (glPrintOpDialogClass *class)
{
	GObjectClass           *object_class = G_OBJECT_CLASS (class);

	gl_debug (DEBUG_PRINT, "");
	
  	gl_print_op_dialog_parent_class = g_type_class_peek_parent (class);

  	object_class->finalize = gl_print_op_dialog_finalize;
}


static void
gl_print_op_dialog_init (glPrintOpDialog *op)
{
	gl_debug (DEBUG_PRINT, "");

	gtk_print_operation_set_use_full_page (GTK_PRINT_OPERATION (op), TRUE);

        gtk_print_operation_set_unit (GTK_PRINT_OPERATION (op), GTK_UNIT_POINTS);

	op->priv = g_new0 (glPrintOpDialogPrivate, 1);

}


static void 
gl_print_op_dialog_finalize (GObject *object)
{
	glPrintOpDialog *op = GL_PRINT_OP_DIALOG (object);
	
	gl_debug (DEBUG_PRINT, "");

	g_return_if_fail (object != NULL);
        g_return_if_fail (GL_IS_PRINT_OP (op));
	g_return_if_fail (op->priv != NULL);

        g_object_unref (G_OBJECT(op->priv->builder));
	g_free (op->priv);

	G_OBJECT_CLASS (gl_print_op_dialog_parent_class)->finalize (object);

	g_free (op->priv);
}


/*****************************************************************************/
/* NEW print op.                                                         */
/*****************************************************************************/
glPrintOpDialog *
gl_print_op_dialog_new (glLabel      *label)
{
	glPrintOpDialog *op;

	gl_debug (DEBUG_PRINT, "");

	op = GL_PRINT_OP_DIALOG (g_object_new (GL_TYPE_PRINT_OP_DIALOG, NULL));

	gl_print_op_dialog_construct (GL_PRINT_OP_DIALOG(op), label);

	return op;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct op.                                              */
/*--------------------------------------------------------------------------*/
static void
gl_print_op_dialog_construct (glPrintOpDialog      *op,
                              glLabel              *label)
{

        gl_print_op_construct(GL_PRINT_OP (op), label);

	gtk_print_operation_set_custom_tab_label ( GTK_PRINT_OPERATION (op),
						   _("Labels"));

	g_signal_connect (G_OBJECT (op), "create-custom-widget",
			  G_CALLBACK (create_custom_widget_cb), label);

	g_signal_connect (G_OBJECT (op), "custom-widget-apply",
			  G_CALLBACK (custom_widget_apply_cb), label);

}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  "Create custom widget" callback                                */
/*--------------------------------------------------------------------------*/
static GObject *
create_custom_widget_cb (GtkPrintOperation *operation,
                         glLabel           *label)
{
	GtkBuilder             *builder;
        static gchar           *object_ids[] = { "print_custom_widget_hbox",
                                                 "adjustment1", "adjustment2",
                                                 "adjustment3", "adjustment4",
                                                 "adjustment5",
                                                 NULL };
        GError                 *error = NULL;
	glPrintOpDialog        *op = GL_PRINT_OP_DIALOG (operation);
        const lglTemplateFrame *frame;
	GtkWidget              *hbox;
        glMerge                *merge = NULL;
        GdkPixbuf              *pixbuf;


        frame = (lglTemplateFrame *)label->template->frames->data;
        op->priv->labels_per_sheet = lgl_template_frame_get_n_labels (frame);

	builder = gtk_builder_new ();
        gtk_builder_add_objects_from_file (builder,
                                           GLABELS_BUILDER_DIR "print-op-dialog-custom-widget.builder",
                                           object_ids,
                                           &error);
	if (error) {
		g_critical ("%s\n\ngLabels may not be installed correctly!", error->message);
                g_error_free (error);
		return NULL;
	}

        gl_builder_util_get_widgets (builder,
                                     "print_custom_widget_hbox", &hbox,
                                     "simple_frame",             &op->priv->simple_frame,
                                     "simple_sheets_radio",      &op->priv->simple_sheets_radio,
                                     "simple_sheets_spin",       &op->priv->simple_sheets_spin,
                                     "simple_labels_radio",      &op->priv->simple_labels_radio,
                                     "simple_first_spin",        &op->priv->simple_first_spin,
                                     "simple_last_spin",         &op->priv->simple_last_spin,
                                     "merge_frame",              &op->priv->merge_frame,
                                     "merge_first_spin",         &op->priv->merge_first_spin,
                                     "merge_copies_spin",        &op->priv->merge_copies_spin,
                                     "merge_collate_check",      &op->priv->merge_collate_check,
                                     "merge_collate_image",      &op->priv->merge_collate_image,
                                     "outline_check",            &op->priv->outline_check,
                                     "reverse_check",            &op->priv->reverse_check,
                                     "crop_marks_check",         &op->priv->crop_marks_check,
                                     NULL);

        op->priv->builder = builder;

        /* ---- Install preview. ---- */
        op->priv->preview = gl_mini_preview_new (MINI_PREVIEW_MIN_HEIGHT, MINI_PREVIEW_MIN_WIDTH);
        gl_mini_preview_set_template (GL_MINI_PREVIEW(op->priv->preview), label->template);
        gl_mini_preview_set_label (GL_MINI_PREVIEW(op->priv->preview), label);
        gtk_box_pack_start (GTK_BOX(hbox), op->priv->preview, TRUE, TRUE, 0);
        gtk_widget_show_all (op->priv->preview);

        
        /* ---- Activate either simple or merge print control widgets. ---- */
        merge = gl_label_get_merge (label);
        op->priv->merge_flag = (merge != NULL);
	if (!op->priv->merge_flag) {

                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (op->priv->simple_sheets_radio),
                                              TRUE);
                gtk_widget_set_sensitive (op->priv->simple_first_spin, FALSE);
                gtk_widget_set_sensitive (op->priv->simple_last_spin, FALSE);

                gtk_spin_button_set_value (GTK_SPIN_BUTTON (op->priv->simple_sheets_spin),
                                           gl_print_op_get_n_sheets (GL_PRINT_OP(op)));

                gtk_spin_button_set_range (GTK_SPIN_BUTTON (op->priv->simple_first_spin),
                                           1, op->priv->labels_per_sheet);
                gtk_spin_button_set_value (GTK_SPIN_BUTTON (op->priv->simple_first_spin),
                                           1);

                gtk_spin_button_set_range (GTK_SPIN_BUTTON (op->priv->simple_last_spin),
                                           1, op->priv->labels_per_sheet);
                gtk_spin_button_set_value (GTK_SPIN_BUTTON (op->priv->simple_last_spin),
                                           op->priv->labels_per_sheet);

                /* Update preview. */
                gl_mini_preview_set_page     (GL_MINI_PREVIEW (op->priv->preview), 0);
                gl_mini_preview_set_n_sheets (GL_MINI_PREVIEW (op->priv->preview),
                                              gl_print_op_get_n_sheets (GL_PRINT_OP(op)));
                gl_mini_preview_set_first    (GL_MINI_PREVIEW (op->priv->preview), 1);
                gl_mini_preview_set_last     (GL_MINI_PREVIEW (op->priv->preview),
                                              op->priv->labels_per_sheet);

		gtk_widget_show_all (op->priv->simple_frame);
		gtk_widget_hide_all (op->priv->merge_frame);

                g_signal_connect (G_OBJECT (op->priv->simple_sheets_radio), "toggled",
                                  G_CALLBACK (simple_sheets_radio_cb), op);
                g_signal_connect (G_OBJECT (op->priv->simple_first_spin), "changed",
                                  G_CALLBACK (simple_first_spin_cb), op);
                g_signal_connect (G_OBJECT (op->priv->simple_last_spin), "changed",
                                  G_CALLBACK (simple_last_spin_cb), op);
                g_signal_connect (G_OBJECT (op->priv->preview), "pressed",
                                  G_CALLBACK (preview_pressed_cb), op);
                g_signal_connect (G_OBJECT (op->priv->preview), "released",
                                  G_CALLBACK (preview_released_cb), op);

	} else {

                op->priv->n_records = gl_merge_get_record_count (merge);

                gtk_spin_button_set_range (GTK_SPIN_BUTTON (op->priv->merge_first_spin),
                                           1, op->priv->labels_per_sheet);
                gtk_spin_button_set_value (GTK_SPIN_BUTTON (op->priv->merge_first_spin),
                                           gl_print_op_get_first (GL_PRINT_OP(op)));

                gtk_spin_button_set_value (GTK_SPIN_BUTTON (op->priv->merge_copies_spin),
                                           gl_print_op_get_n_copies (GL_PRINT_OP(op)));

                gtk_widget_set_sensitive (op->priv->merge_collate_check,
                                          (gl_print_op_get_n_copies (GL_PRINT_OP(op)) > 1));
                gtk_widget_set_sensitive (op->priv->merge_collate_image,
                                          (gl_print_op_get_n_copies (GL_PRINT_OP(op)) > 1));
                

                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (op->priv->simple_sheets_radio),
                                              gl_print_op_get_collate_flag (GL_PRINT_OP(op)));

                if ( gl_print_op_get_collate_flag (GL_PRINT_OP(op)) )
                {
                        pixbuf = gdk_pixbuf_new_from_xpm_data ( (const char **)nocollate_xpm);
                }
                else
                {
                        pixbuf = gdk_pixbuf_new_from_xpm_data ( (const char **)collate_xpm);
                }

                gtk_image_set_from_pixbuf (GTK_IMAGE (op->priv->merge_collate_image), pixbuf);

                /* Update preview. */
                gl_mini_preview_set_page     (GL_MINI_PREVIEW (op->priv->preview), 0);
                gl_mini_preview_set_first    (GL_MINI_PREVIEW (op->priv->preview),
                                              gl_print_op_get_first (GL_PRINT_OP(op)));
                gl_mini_preview_set_n_copies (GL_MINI_PREVIEW (op->priv->preview),
                                              gl_print_op_get_n_copies (GL_PRINT_OP(op)));
                gl_mini_preview_set_collate_flag (GL_MINI_PREVIEW (op->priv->preview),
                                                  gl_print_op_get_collate_flag (GL_PRINT_OP(op)));


		gtk_widget_hide_all (op->priv->simple_frame);
		gtk_widget_show_all (op->priv->merge_frame);

                g_signal_connect (G_OBJECT (op->priv->merge_first_spin), "changed",
                                  G_CALLBACK (merge_spin_cb), op);
                g_signal_connect (G_OBJECT (op->priv->merge_copies_spin), "changed",
                                  G_CALLBACK (merge_spin_cb), op);
                g_signal_connect (G_OBJECT (op->priv->merge_collate_check), "toggled",
                                  G_CALLBACK (merge_collate_check_cb), op);
                g_signal_connect (G_OBJECT (op->priv->preview), "clicked",
                                  G_CALLBACK (preview_clicked_cb), op);
                g_signal_connect (G_OBJECT (op->priv->preview), "pressed",
                                  G_CALLBACK (preview_pressed_cb), op);
                g_signal_connect (G_OBJECT (op->priv->preview), "released",
                                  G_CALLBACK (preview_released_cb), op);

		g_object_unref (G_OBJECT(merge));

	}

        /* --- Set options --- */
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (op->priv->outline_check),
                                      gl_print_op_get_outline_flag (GL_PRINT_OP(op)));
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (op->priv->reverse_check),
                                      gl_print_op_get_reverse_flag (GL_PRINT_OP(op)));
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (op->priv->crop_marks_check),
                                      gl_print_op_get_crop_marks_flag (GL_PRINT_OP(op)));

        /* --- Do we need to force the outline flag --- */
        if (gl_print_op_is_outline_forced (GL_PRINT_OP (op)))
        {
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(op->priv->outline_check),
                                              TRUE);

                gtk_widget_set_sensitive (op->priv->outline_check, FALSE);
                gtk_widget_set_sensitive (op->priv->reverse_check, FALSE);
                gtk_widget_set_sensitive (op->priv->crop_marks_check, FALSE);
        }

        g_signal_connect (G_OBJECT (op->priv->outline_check), "toggled",
                          G_CALLBACK (option_toggled_cb), op);
        g_signal_connect (G_OBJECT (op->priv->reverse_check), "toggled",
                          G_CALLBACK (option_toggled_cb), op);
        g_signal_connect (G_OBJECT (op->priv->crop_marks_check), "toggled",
                          G_CALLBACK (option_toggled_cb), op);

	return G_OBJECT (hbox);
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  "Custom widget apply" callback                                 */
/*--------------------------------------------------------------------------*/
static void
custom_widget_apply_cb (GtkPrintOperation *operation,
                        GtkWidget         *widget,
                        glLabel           *label)
{
        glPrintOpDialog *op    = GL_PRINT_OP_DIALOG (operation);
        glMerge         *merge = NULL;
        gint             n_records;
        gint             n_sheets, first, last, n_copies;
        gboolean         collate_flag;

        gl_print_op_set_outline_flag (GL_PRINT_OP (op),
                                      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
                                                                    (op->priv->outline_check)));
        gl_print_op_set_reverse_flag (GL_PRINT_OP (op),
                                      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
                                                                    (op->priv->reverse_check)));
        gl_print_op_set_crop_marks_flag (GL_PRINT_OP (op),
                                         gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
                                                                       (op->priv->crop_marks_check)));


        if (!op->priv->merge_flag)
        {

                n_sheets = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (op->priv->simple_sheets_spin));
                gl_print_op_set_n_sheets (GL_PRINT_OP (op), n_sheets);

                first = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (op->priv->simple_first_spin));
                gl_print_op_set_first (GL_PRINT_OP (op), first);

                last = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (op->priv->simple_last_spin));
                gl_print_op_set_last (GL_PRINT_OP (op), last);

        }
        else
        {

                merge = gl_label_get_merge (label);

                n_copies = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (op->priv->merge_copies_spin));
                gl_print_op_set_n_copies (GL_PRINT_OP (op), n_copies);

                first = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (op->priv->merge_first_spin));
                gl_print_op_set_first (GL_PRINT_OP (op), first);

                collate_flag = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (op->priv->merge_collate_check));
                gl_print_op_set_collate_flag (GL_PRINT_OP (op), collate_flag);

                n_records = gl_merge_get_record_count (merge);
                n_sheets = ceil (first - 1 + (n_copies * n_records)/(double)op->priv->labels_per_sheet);
                gl_print_op_set_n_sheets     (GL_PRINT_OP (op), n_sheets);

                g_object_unref (G_OBJECT(merge));

        }


}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Simple sheets radio "toggled" callback                         */
/*--------------------------------------------------------------------------*/
static void
simple_sheets_radio_cb (GtkToggleButton   *togglebutton,
                        glPrintOpDialog   *op)
{
        gint first, last;

        if (gtk_toggle_button_get_active (togglebutton)) {

                gtk_widget_set_sensitive (op->priv->simple_sheets_spin, TRUE);
                gtk_widget_set_sensitive (op->priv->simple_first_spin, FALSE);
                gtk_widget_set_sensitive (op->priv->simple_last_spin, FALSE);

                gtk_spin_button_set_range (GTK_SPIN_BUTTON (op->priv->simple_first_spin),
                                           1, op->priv->labels_per_sheet);
                gtk_spin_button_set_value (GTK_SPIN_BUTTON (op->priv->simple_first_spin),
                                           1);

                gtk_spin_button_set_range (GTK_SPIN_BUTTON (op->priv->simple_last_spin),
                                           1, op->priv->labels_per_sheet);
                gtk_spin_button_set_value (GTK_SPIN_BUTTON (op->priv->simple_last_spin),
                                           op->priv->labels_per_sheet);

                gl_mini_preview_set_n_sheets (GL_MINI_PREVIEW (op->priv->preview),
                                              gl_print_op_get_n_sheets (GL_PRINT_OP(op)));
                gl_mini_preview_set_first    (GL_MINI_PREVIEW (op->priv->preview), 1);
                gl_mini_preview_set_last     (GL_MINI_PREVIEW (op->priv->preview),
                                              op->priv->labels_per_sheet);

        } else {

                gtk_widget_set_sensitive (op->priv->simple_sheets_spin, FALSE);
                gtk_widget_set_sensitive (op->priv->simple_first_spin, TRUE);
                gtk_widget_set_sensitive (op->priv->simple_last_spin, TRUE);

                gtk_spin_button_set_value (GTK_SPIN_BUTTON (op->priv->simple_sheets_spin), 1);

                first = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (op->priv->simple_first_spin));
                last  = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (op->priv->simple_last_spin));

                gl_mini_preview_set_n_sheets (GL_MINI_PREVIEW (op->priv->preview), 1);
                gl_mini_preview_set_first    (GL_MINI_PREVIEW (op->priv->preview), first);
                gl_mini_preview_set_last     (GL_MINI_PREVIEW (op->priv->preview), last);

        }
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Simple first label spinbutton "changed" callback               */
/*--------------------------------------------------------------------------*/
static void
simple_first_spin_cb (GtkSpinButton     *spinbutton,
                      glPrintOpDialog   *op)
{
        gint first, last;

        first = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (op->priv->simple_first_spin));
        last  = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (op->priv->simple_last_spin));

        /* Adjust range of last spin button. */
        gtk_spin_button_set_range (GTK_SPIN_BUTTON (op->priv->simple_last_spin),
                                   first, op->priv->labels_per_sheet);

        /* Update preview. */
        gl_mini_preview_set_first    (GL_MINI_PREVIEW (op->priv->preview), first);
        gl_mini_preview_set_last     (GL_MINI_PREVIEW (op->priv->preview), last);

}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Simple last label spinbutton "changed" callback                */
/*--------------------------------------------------------------------------*/
static void
simple_last_spin_cb (GtkSpinButton     *spinbutton,
                     glPrintOpDialog   *op)
{
        gint first, last;

        first = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (op->priv->simple_first_spin));
        last  = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (op->priv->simple_last_spin));

        /* Adjust range of first spin button. */
        gtk_spin_button_set_range (GTK_SPIN_BUTTON (op->priv->simple_first_spin),
                                   1, last);

        /* Update preview. */
        gl_mini_preview_set_first    (GL_MINI_PREVIEW (op->priv->preview), first);
        gl_mini_preview_set_last     (GL_MINI_PREVIEW (op->priv->preview), last);

}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Preview "pressed" callback                                     */
/*--------------------------------------------------------------------------*/
static void
preview_pressed_cb (glMiniPreview     *preview,
                    gint               first,
                    gint               last,
                    glPrintOpDialog   *op)
{
        gint old_first;

        if ( !op->priv->merge_flag )
        {

                /* Update controls. */
                old_first = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (op->priv->simple_first_spin));
                if ( first > old_first )
                {
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON (op->priv->simple_last_spin), last);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON (op->priv->simple_first_spin), first);
                }
                else
                {
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON (op->priv->simple_first_spin), first);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON (op->priv->simple_last_spin), last);
                }
                if ( (first == 1) && (last == op->priv->labels_per_sheet) )
                {
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (op->priv->simple_sheets_radio), TRUE);
                        gtk_spin_button_set_value (GTK_SPIN_BUTTON (op->priv->simple_sheets_spin), 1);
                }
                else
                {
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (op->priv->simple_labels_radio), TRUE);
                }

        }

        gl_mini_preview_highlight_range (GL_MINI_PREVIEW (op->priv->preview), first, last);

}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Preview "pressed" callback                                     */
/*--------------------------------------------------------------------------*/
static void
preview_released_cb (glMiniPreview     *preview,
                     gint               first,
                     gint               last,
                     glPrintOpDialog   *op)
{

        gl_mini_preview_highlight_range (GL_MINI_PREVIEW (op->priv->preview), 0, 0);

}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  "changed" callback for merge first and copies spin buttons.    */
/*--------------------------------------------------------------------------*/
static void
merge_spin_cb (GtkSpinButton     *spinbutton,
               glPrintOpDialog   *op)
{
        gint first, n_copies;

        first = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (op->priv->merge_first_spin));

        n_copies = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (op->priv->merge_copies_spin));

        gl_mini_preview_set_first (GL_MINI_PREVIEW(op->priv->preview), first);
        gl_mini_preview_set_n_copies (GL_MINI_PREVIEW(op->priv->preview), n_copies);

        gtk_widget_set_sensitive (op->priv->merge_collate_check, (n_copies > 1));
        gtk_widget_set_sensitive (op->priv->merge_collate_image, (n_copies > 1));
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Merge first collate check "toggled" callback                   */
/*--------------------------------------------------------------------------*/
static void
merge_collate_check_cb (GtkToggleButton   *togglebutton,
                        glPrintOpDialog   *op)
{
        GdkPixbuf *pixbuf;

        if ( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (op->priv->merge_collate_check)) )
        {
                pixbuf = gdk_pixbuf_new_from_xpm_data ( (const char **)nocollate_xpm);
                gl_mini_preview_set_collate_flag (GL_MINI_PREVIEW (op->priv->preview), TRUE);
        }
        else
        {
                pixbuf = gdk_pixbuf_new_from_xpm_data ( (const char **)collate_xpm);
                gl_mini_preview_set_collate_flag (GL_MINI_PREVIEW (op->priv->preview), FALSE);
        }

        gtk_image_set_from_pixbuf (GTK_IMAGE (op->priv->merge_collate_image), pixbuf);
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Preview "clicked" callback                                     */
/*--------------------------------------------------------------------------*/
static void
preview_clicked_cb (glMiniPreview     *preview,
                    gint               first,
                    glPrintOpDialog   *op)
{
        gint n_copies;

        if ( op->priv->merge_flag )
        {

                /* Update controls. */
                gtk_spin_button_set_value (GTK_SPIN_BUTTON (op->priv->merge_first_spin), first);

                n_copies = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (op->priv->merge_copies_spin));
                gtk_widget_set_sensitive (op->priv->merge_collate_check, (n_copies > 1));
                gtk_widget_set_sensitive (op->priv->merge_collate_image, (n_copies > 1));

        }
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Option checkbox "toggled" callback                             */
/*--------------------------------------------------------------------------*/
static void
option_toggled_cb (GtkToggleButton   *togglebutton,
                   glPrintOpDialog   *op)
{
        gboolean flag;

        flag = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (op->priv->outline_check));
        gl_mini_preview_set_outline_flag (GL_MINI_PREVIEW (op->priv->preview), flag);

        flag = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (op->priv->reverse_check));
        gl_mini_preview_set_reverse_flag (GL_MINI_PREVIEW (op->priv->preview), flag);

        flag = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (op->priv->crop_marks_check));
        gl_mini_preview_set_crop_marks_flag (GL_MINI_PREVIEW (op->priv->preview), flag);
}




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
