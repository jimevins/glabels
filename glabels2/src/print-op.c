/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  print-op.c:  Print operation module
 *
 *  Copyright (C) 2001-2007  Jim Evins <evins@snaught.com>.
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

#include "print-op.h"

#include <glib/gi18n.h>
#include <glade/glade-xml.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <gtk/gtktogglebutton.h>

#include "print.h"
#include "label.h"

#include "wdgt-print-copies.h"
#include "wdgt-print-merge.h"

#include "debug.h"

/*===========================================*/
/* Private data types                        */
/*===========================================*/

struct _glPrintOpPrivate {

	glLabel   *label;

	GladeXML  *gui;

	GtkWidget *simple_frame;
	GtkWidget *copies_vbox;
	GtkWidget *copies;

	GtkWidget *merge_frame;
	GtkWidget *prmerge_vbox;
	GtkWidget *prmerge;

	GtkWidget *outline_check;
	GtkWidget *reverse_check;
	GtkWidget *crop_marks_check;

        gboolean   force_outline_flag;

        gchar     *filename;

	gboolean   outline_flag;
        gboolean   reverse_flag;
        gboolean   crop_marks_flag;
        gboolean   merge_flag;
        gboolean   collate_flag;

	gint       first;
        gint       last;
        gint       n_sheets;
        gint       n_copies;

        glPrintState state;
};


/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void     gl_print_op_finalize      (GObject            *object);

static void     gl_print_op_construct       (glPrintOp          *op,
                                             glLabel            *label);

static void     gl_print_op_construct_batch (glPrintOp          *op,
                                             glLabel            *label,
                                             gchar              *filename,
                                             gint                n_sheets,
                                             gint                n_copies,
                                             gint                first,
                                             gboolean            outline_flag,
                                             gboolean            reverse_flag,
                                             gboolean            crop_marks_flag);


static GObject *create_custom_widget_cb       (GtkPrintOperation *operation,
                                               gpointer           user_data);

static void     custom_widget_apply_cb        (GtkPrintOperation *operation,
                                               GtkWidget         *widget,
                                               gpointer           user_data);

static void     begin_print_cb                (GtkPrintOperation *operation,
                                               GtkPrintContext   *context,
                                               gpointer           user_data);

static void     draw_page_cb                  (GtkPrintOperation *operation,
                                               GtkPrintContext   *context,
                                               int                page_nr,
                                               gpointer           user_data);




/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glPrintOp, gl_print_op, GTK_TYPE_PRINT_OPERATION);

static void
gl_print_op_class_init (glPrintOpClass *class)
{
	GObjectClass           *object_class = G_OBJECT_CLASS (class);
	GtkPrintOperationClass *print_class  = GTK_PRINT_OPERATION_CLASS (class);

	gl_debug (DEBUG_PRINT, "");
	
  	gl_print_op_parent_class = g_type_class_peek_parent (class);

  	object_class->finalize = gl_print_op_finalize;
}

static void
gl_print_op_init (glPrintOp *op)
{
	GtkWidget *pp_button;

	gl_debug (DEBUG_PRINT, "");

	gtk_print_operation_set_use_full_page (GTK_PRINT_OPERATION (op), TRUE);

        gtk_print_operation_set_unit (GTK_PRINT_OPERATION (op), GTK_UNIT_POINTS);

	op->priv = g_new0 (glPrintOpPrivate, 1);

}

static void 
gl_print_op_finalize (GObject *object)
{
	glPrintOp* op = GL_PRINT_OP (object);
	
	gl_debug (DEBUG_PRINT, "");

	g_return_if_fail (object != NULL);
        g_return_if_fail (GL_IS_PRINT_OP (op));
	g_return_if_fail (op->priv != NULL);

	if (op->priv->label) {
		g_object_unref (G_OBJECT(op->priv->label));
	}
        g_free (op->priv->filename);
	g_free (op->priv);

	G_OBJECT_CLASS (gl_print_op_parent_class)->finalize (object);

	g_free (op->priv);
}

/*****************************************************************************/
/* NEW print op.                                                         */
/*****************************************************************************/
glPrintOp *
gl_print_op_new (glLabel      *label)
{
	glPrintOp *op;

	gl_debug (DEBUG_PRINT, "");

	op = GL_PRINT_OP (g_object_new (GL_TYPE_PRINT_OP, NULL));

	gl_print_op_construct (GL_PRINT_OP(op), label);

	return op;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct op.                                              */
/*--------------------------------------------------------------------------*/
static void
gl_print_op_construct (glPrintOp      *op,
                       glLabel        *label)
{
	op->priv->label              = label;
	op->priv->force_outline_flag = FALSE;

	gtk_print_operation_set_custom_tab_label ( GTK_PRINT_OPERATION (op),
						   _("Labels"));

	g_signal_connect (G_OBJECT (op), "create-custom-widget",
			  G_CALLBACK (create_custom_widget_cb), label);

	g_signal_connect (G_OBJECT (op), "custom-widget-apply",
			  G_CALLBACK (custom_widget_apply_cb), label);

	g_signal_connect (G_OBJECT (op), "begin-print",
			  G_CALLBACK (begin_print_cb), label);

	g_signal_connect (G_OBJECT (op), "draw-page",
			  G_CALLBACK (draw_page_cb), label);
}

/*****************************************************************************/
/* NEW batch print operation.                                                */
/*****************************************************************************/
glPrintOp *
gl_print_op_new_batch (glLabel       *label,
                       gchar         *filename,
                       gint           n_sheets,
                       gint           n_copies,
                       gint           first,
                       gboolean       outline_flag,
                       gboolean       reverse_flag,
                       gboolean       crop_marks_flag)
{
	glPrintOp *op;

	gl_debug (DEBUG_PRINT, "");

	op = GL_PRINT_OP (g_object_new (GL_TYPE_PRINT_OP, NULL));

	gl_print_op_construct_batch (GL_PRINT_OP(op),
                                         label,
                                         filename,
                                         n_sheets,
                                         n_copies,
                                         first,
                                         outline_flag,
                                         reverse_flag,
                                         crop_marks_flag);

	return op;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct op.                                              */
/*--------------------------------------------------------------------------*/
static void
gl_print_op_construct_batch (glPrintOp      *op,
                             glLabel        *label,
                             gchar          *filename,
                             gint            n_sheets,
                             gint            n_copies,
                             gint            first,
                             gboolean        outline_flag,
                             gboolean        reverse_flag,
                             gboolean        crop_marks_flag)

{
        glMerge                   *merge = NULL;
        glTemplate                *template = NULL;
        const glTemplateLabelType *label_type = NULL;

	op->priv->label              = label;
	op->priv->force_outline_flag = FALSE;
        op->priv->filename           = g_strdup (filename);
        op->priv->n_sheets           = n_sheets;
        op->priv->n_copies           = n_copies;
        op->priv->first              = first;
        op->priv->outline_flag       = outline_flag;
        op->priv->reverse_flag       = reverse_flag;
        op->priv->crop_marks_flag    = crop_marks_flag;

        merge = gl_label_get_merge (label);

        template = gl_label_get_template (label);
        label_type = gl_template_get_first_label_type (template);
        if (merge == NULL)
        {
                op->priv->merge_flag = FALSE;

                op->priv->last = gl_template_get_n_labels (label_type);

        }
        else
        {
                op->priv->merge_flag = TRUE;

                op->priv->n_sheets =
                        ceil ((double)(first-1 + n_copies * gl_merge_get_record_count(merge))
                              / gl_template_get_n_labels (label_type));;

                g_object_unref (G_OBJECT(merge));

        }
        gl_template_free (template);

        gtk_print_operation_set_export_filename (GTK_PRINT_OPERATION (op),
                                                 filename);

	g_signal_connect (G_OBJECT (op), "begin-print",
			  G_CALLBACK (begin_print_cb), label);

	g_signal_connect (G_OBJECT (op), "draw-page",
			  G_CALLBACK (draw_page_cb), label);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  "Create custom widget" callback                                */
/*--------------------------------------------------------------------------*/
static GObject *
create_custom_widget_cb (GtkPrintOperation *operation,
			 gpointer           user_data)
{
	glPrintOp *op = GL_PRINT_OP (operation);
	glLabel       *label  = GL_LABEL (user_data);

	GtkWidget     *vbox;

        glMerge       *merge = NULL;

	op->priv->gui = glade_xml_new (GLABELS_GLADE_DIR "print-custom-widget.glade",
                                       "print_custom_widget_vbox",
                                       NULL);

	if (!op->priv->gui) {
		g_warning ("Could not open print-op.glade, reinstall glabels!");
		return;
	}

	vbox = glade_xml_get_widget (op->priv->gui, "print_custom_widget_vbox");

	/* ----- Simple print control ----- */
	op->priv->simple_frame = glade_xml_get_widget (op->priv->gui,
							   "simple_frame");
	op->priv->copies_vbox  = glade_xml_get_widget (op->priv->gui,
							   "copies_vbox");
	op->priv->copies = gl_wdgt_print_copies_new (label);
	gtk_box_pack_start (GTK_BOX(op->priv->copies_vbox),
			    op->priv->copies, FALSE, FALSE, 0);

	/* ----- Merge print control ----- */
	op->priv->merge_frame  = glade_xml_get_widget (op->priv->gui,
							   "merge_frame");
	op->priv->prmerge_vbox = glade_xml_get_widget (op->priv->gui,
							   "prmerge_vbox");
	op->priv->prmerge = gl_wdgt_print_merge_new (label);
	gtk_box_pack_start (GTK_BOX(op->priv->prmerge_vbox),
			    op->priv->prmerge, FALSE, FALSE, 0);

	/* ----- Options ----------------- */
	op->priv->outline_check    = glade_xml_get_widget (op->priv->gui,
							       "outline_check");
	op->priv->reverse_check    = glade_xml_get_widget (op->priv->gui,
							       "reverse_check");
	op->priv->crop_marks_check = glade_xml_get_widget (op->priv->gui,
							       "crop_marks_check");

        /* ---- Activate either simple or merge print control widgets. ---- */
        merge = gl_label_get_merge (op->priv->label);
	if (merge == NULL) {

		gtk_widget_show_all (op->priv->simple_frame);
		gtk_widget_hide_all (op->priv->merge_frame);

	} else {

		gint n_records = gl_merge_get_record_count( merge );
		gl_wdgt_print_merge_set_copies (GL_WDGT_PRINT_MERGE(op->priv->prmerge),
						1, 1, n_records, FALSE);
		g_object_unref (G_OBJECT(merge));

		gtk_widget_hide_all (op->priv->simple_frame);
		gtk_widget_show_all (op->priv->merge_frame);
	}

        /* --- Do we need to force the outline flag --- */
        if (op->priv->force_outline_flag)
        {
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(op->priv->outline_check),
                                              TRUE);

                gtk_widget_set_sensitive (op->priv->outline_check, FALSE);
                gtk_widget_set_sensitive (op->priv->reverse_check, FALSE);
                gtk_widget_set_sensitive (op->priv->crop_marks_check, FALSE);
        }

	return G_OBJECT (vbox);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  "Custom widget apply" callback                                 */
/*--------------------------------------------------------------------------*/
static void
custom_widget_apply_cb (GtkPrintOperation *operation,
                        GtkWidget         *widget,
                        gpointer           user_data)
{
        glPrintOp *op = GL_PRINT_OP (operation);
        glMerge       *merge = NULL;


        op->priv->outline_flag =
                gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
                                              (op->priv->outline_check));
        op->priv->reverse_flag =
                gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
                                              (op->priv->reverse_check));
        op->priv->crop_marks_flag =
                gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
                                              (op->priv->crop_marks_check));

        merge = gl_label_get_merge (op->priv->label);

        if (merge == NULL)
        {

                op->priv->merge_flag = FALSE;
                gl_wdgt_print_copies_get_range (GL_WDGT_PRINT_COPIES (op->priv->copies),
                                                &op->priv->n_sheets,
                                                &op->priv->first,
                                                &op->priv->last);
        }
        else
        {

                op->priv->merge_flag = TRUE;
                gl_wdgt_print_merge_get_copies (GL_WDGT_PRINT_MERGE (op->priv->prmerge),
                                                &op->priv->n_copies,
                                                &op->priv->first,
                                                &op->priv->collate_flag,
                                                &op->priv->n_sheets);
                g_object_unref (G_OBJECT(merge));
        }

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  "Begin print" callback                                         */
/*--------------------------------------------------------------------------*/
static void
begin_print_cb (GtkPrintOperation *operation,
		GtkPrintContext   *context,
		gpointer           user_data)
{
        glPrintOp *op = GL_PRINT_OP (operation);

        gtk_print_operation_set_n_pages (operation, op->priv->n_sheets);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  "Draw page" callback.                                          */
/*--------------------------------------------------------------------------*/
static void
draw_page_cb (GtkPrintOperation *operation,
	      GtkPrintContext   *context,
	      int                page_nr,
	      gpointer           user_data)
{
        glPrintOp *op = GL_PRINT_OP (operation);
        cairo_t       *cr;

        cr = gtk_print_context_get_cairo_context (context);

        if (!op->priv->merge_flag)
        {
                gl_print_simple_sheet (op->priv->label,
                                       cr,
                                       page_nr,
                                       op->priv->n_sheets,
                                       op->priv->first,
                                       op->priv->last,
                                       op->priv->outline_flag,
                                       op->priv->reverse_flag,
                                       op->priv->crop_marks_flag);
        }
        else
        {
                if (op->priv->collate_flag)
                {
                        gl_print_collated_merge_sheet (op->priv->label,
                                                       cr,
                                                       page_nr,
                                                       op->priv->n_copies,
                                                       op->priv->first,
                                                       op->priv->outline_flag,
                                                       op->priv->reverse_flag,
                                                       op->priv->crop_marks_flag,
                                                       &op->priv->state);
                }
                else
                {
                        gl_print_uncollated_merge_sheet (op->priv->label,
                                                         cr,
                                                         page_nr,
                                                         op->priv->n_copies,
                                                         op->priv->first,
                                                         op->priv->outline_flag,
                                                         op->priv->reverse_flag,
                                                         op->priv->crop_marks_flag,
                                                         &op->priv->state);
                }
        }
}


/*****************************************************************************/
/* Set outline flag/checkbox.                                                */
/*****************************************************************************/
void
gl_print_op_force_outline_flag (glPrintOp *op)
{
        op->priv->force_outline_flag = TRUE;
}


