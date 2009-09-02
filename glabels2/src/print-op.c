/*
 *  print-op.c
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

#include "print-op.h"

#include <glib/gi18n.h>
#include <gtk/gtkbuilder.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <gtk/gtktogglebutton.h>

#include <libglabels/db.h>
#include "print.h"
#include "label.h"
#include "util.h"

#include "wdgt-print-copies.h"
#include "wdgt-print-merge.h"

#include "debug.h"


/*===========================================*/
/* Private data types                        */
/*===========================================*/

struct _glPrintOpPrivate {

	glLabel   *label;

        GtkBuilder *builder;

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

struct _glPrintOpSettings
{

        GtkPrintSettings *gtk_settings;

        gboolean          outline_flag;
        gboolean          reverse_flag;
        gboolean          crop_marks_flag;
        gboolean          collate_flag;

        gint              first;
        gint              last;
        gint              n_sheets;
        gint              n_copies;
        
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


static void     set_page_size                 (glPrintOp         *op,
                                               glLabel           *label);

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

	gl_debug (DEBUG_PRINT, "");
	
  	gl_print_op_parent_class = g_type_class_peek_parent (class);

  	object_class->finalize = gl_print_op_finalize;
}


static void
gl_print_op_init (glPrintOp *op)
{
	gl_debug (DEBUG_PRINT, "");

	gtk_print_operation_set_use_full_page (GTK_PRINT_OPERATION (op), TRUE);

        gtk_print_operation_set_unit (GTK_PRINT_OPERATION (op), GTK_UNIT_POINTS);

	op->priv = g_new0 (glPrintOpPrivate, 1);

}


static void 
gl_print_op_finalize (GObject *object)
{
	glPrintOp *op = GL_PRINT_OP (object);
	
	gl_debug (DEBUG_PRINT, "");

	g_return_if_fail (object != NULL);
        g_return_if_fail (GL_IS_PRINT_OP (op));
	g_return_if_fail (op->priv != NULL);

	if (op->priv->label) {
		g_object_unref (G_OBJECT(op->priv->label));
	}
	if (op->priv->builder) {
		g_object_unref (G_OBJECT(op->priv->builder));
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
        const lglTemplateFrame *frame;

	op->priv->label              = label;
	op->priv->force_outline_flag = FALSE;

        frame = (lglTemplateFrame *)label->template->frames->data;

        op->priv->n_sheets           = 1;
        op->priv->first              = 1;
        op->priv->last               = lgl_template_frame_get_n_labels (frame);
        op->priv->n_copies           = 1;

        set_page_size (op, label);

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


/*****************************************************************************/
/* Get print operation settings.                                             */
/*****************************************************************************/
glPrintOpSettings *
gl_print_op_get_settings (glPrintOp         *print_op)
{
        glPrintOpSettings *settings;

        settings = g_new0 (glPrintOpSettings, 1);

        if ( settings )
        {
                settings->gtk_settings =
                        gtk_print_operation_get_print_settings (GTK_PRINT_OPERATION (print_op));

                settings->outline_flag     = print_op->priv->outline_flag;
                settings->reverse_flag     = print_op->priv->reverse_flag;
                settings->crop_marks_flag  = print_op->priv->crop_marks_flag;
                settings->collate_flag     = print_op->priv->collate_flag;

                settings->first            = print_op->priv->first;
                settings->last             = print_op->priv->last;
                settings->n_sheets         = print_op->priv->n_sheets;
                settings->n_copies         = print_op->priv->n_copies;
        }

        return settings;
}


/*****************************************************************************/
/* Set print operation settings.                                             */
/*****************************************************************************/
void
gl_print_op_set_settings (glPrintOp         *print_op,
                          glPrintOpSettings *settings)
{

        if ( settings )
        {
                gtk_print_operation_set_print_settings (GTK_PRINT_OPERATION (print_op),
                                                        settings->gtk_settings);

                print_op->priv->outline_flag     = settings->outline_flag;
                print_op->priv->reverse_flag     = settings->reverse_flag;
                print_op->priv->crop_marks_flag  = settings->crop_marks_flag;
                print_op->priv->collate_flag     = settings->collate_flag;

                print_op->priv->first            = settings->first;
                print_op->priv->last             = settings->last;
                print_op->priv->n_sheets         = settings->n_sheets;
                print_op->priv->n_copies         = settings->n_copies;
        }

}


/*****************************************************************************/
/* Free print operation settings structure.                                  */
/*****************************************************************************/
void
gl_print_op_free_settings(glPrintOpSettings *settings)
{
        
        if ( settings )
        {
                if ( settings->gtk_settings )
                {
                        g_object_unref (settings->gtk_settings);
                }

                g_free (settings);
        }
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct op.                                                  */
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
        glMerge                *merge = NULL;
        const lglTemplateFrame *frame = NULL;

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

        frame = (lglTemplateFrame *)label->template->frames->data;
        if (merge == NULL)
        {
                op->priv->merge_flag = FALSE;

                op->priv->last = lgl_template_frame_get_n_labels (frame);

        }
        else
        {
                op->priv->merge_flag = TRUE;

                op->priv->n_sheets =
                        ceil ((double)(first-1 + n_copies * gl_merge_get_record_count(merge))
                              / lgl_template_frame_get_n_labels (frame));;

                g_object_unref (G_OBJECT(merge));

        }

        set_page_size (op, label);

        gtk_print_operation_set_export_filename (GTK_PRINT_OPERATION (op),
                                                 filename);

	g_signal_connect (G_OBJECT (op), "begin-print",
			  G_CALLBACK (begin_print_cb), label);

	g_signal_connect (G_OBJECT (op), "draw-page",
			  G_CALLBACK (draw_page_cb), label);
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Set page size.                                                 */
/*--------------------------------------------------------------------------*/
static void
set_page_size (glPrintOp  *op,
               glLabel    *label)
{
        GtkPaperSize *psize;
        GtkPageSetup *su;
        lglPaper     *paper;

	gl_debug (DEBUG_PRINT, "begin");

        paper = lgl_db_lookup_paper_from_id (label->template->paper_id);

        if (!paper)
        {
                const gchar *name;

                name = gtk_paper_size_get_default ();
                psize = gtk_paper_size_new (name);

                gl_debug (DEBUG_PRINT, "Using default size = \"%s\"", name);
        }
        else if (lgl_db_is_paper_id_other (paper->id))
        {
                psize = gtk_paper_size_new_custom (paper->id,
                                                   paper->name,
                                                   label->template->page_width,
                                                   label->template->page_height,
                                                   GTK_UNIT_POINTS);
                gl_debug (DEBUG_PRINT, "Using custom size = %g x %g points",
                          label->template->page_width,
                          label->template->page_height);

        }
        else
        {
                psize = gtk_paper_size_new (paper->pwg_size);
                gl_debug (DEBUG_PRINT, "Using PWG size \"%s\"", paper->pwg_size);
        }
        lgl_paper_free (paper);

        su = gtk_page_setup_new ();
        gtk_page_setup_set_paper_size (su, psize);
        gtk_print_operation_set_default_page_setup (GTK_PRINT_OPERATION (op), su);
        g_object_unref (su);

        gtk_paper_size_free (psize);

	gl_debug (DEBUG_PRINT, "end");
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  "Create custom widget" callback                                */
/*--------------------------------------------------------------------------*/
static GObject *
create_custom_widget_cb (GtkPrintOperation *operation,
			 gpointer           user_data)
{
	GtkBuilder    *builder;
        static gchar  *object_ids[] = { "print_custom_widget_vbox", NULL };
        GError        *error = NULL;
	glPrintOp     *op = GL_PRINT_OP (operation);
	glLabel       *label  = GL_LABEL (user_data);
	GtkWidget     *vbox;
        glMerge       *merge = NULL;

	builder = gtk_builder_new ();
        gtk_builder_add_objects_from_file (builder,
                                           GLABELS_BUILDER_DIR "print-custom-widget.builder",
                                           object_ids,
                                           &error);
	if (error) {
		g_critical ("%s\n\ngLabels may not be installed correctly!", error->message);
                g_error_free (error);
		return NULL;
	}

        gl_util_get_builder_widgets (builder,
                                     "print_custom_widget_vbox", &vbox,
                                     "simple_frame",             &op->priv->simple_frame,
                                     "copies_vbox",              &op->priv->copies_vbox,
                                     "merge_frame",              &op->priv->merge_frame,
                                     "prmerge_vbox",             &op->priv->prmerge_vbox,
                                     "outline_check",            &op->priv->outline_check,
                                     "reverse_check",            &op->priv->reverse_check,
                                     "crop_marks_check",         &op->priv->crop_marks_check,
                                     NULL);

	/* ----- Simple print control ----- */
	op->priv->copies = gl_wdgt_print_copies_new (label);
	gtk_box_pack_start (GTK_BOX(op->priv->copies_vbox),
			    op->priv->copies, FALSE, FALSE, 0);

	/* ----- Merge print control ----- */
	op->priv->prmerge = gl_wdgt_print_merge_new (label);
	gtk_box_pack_start (GTK_BOX(op->priv->prmerge_vbox),
			    op->priv->prmerge, FALSE, FALSE, 0);


        op->priv->builder = builder;

        
        /* ---- Activate either simple or merge print control widgets. ---- */
        merge = gl_label_get_merge (op->priv->label);
	if (merge == NULL) {

                gl_wdgt_print_copies_set_range (GL_WDGT_PRINT_COPIES (op->priv->copies),
                                                op->priv->n_sheets,
                                                op->priv->first,
                                                op->priv->last);

		gtk_widget_show_all (op->priv->simple_frame);
		gtk_widget_hide_all (op->priv->merge_frame);

	} else {

		gint n_records = gl_merge_get_record_count( merge );
                gl_wdgt_print_merge_set_copies (GL_WDGT_PRINT_MERGE (op->priv->prmerge),
                                                op->priv->n_copies,
                                                op->priv->first,
                                                n_records,
                                                op->priv->collate_flag);
		g_object_unref (G_OBJECT(merge));

		gtk_widget_hide_all (op->priv->simple_frame);
		gtk_widget_show_all (op->priv->merge_frame);
	}

        /* --- Set options --- */
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (op->priv->outline_check),
                                      op->priv->outline_flag);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (op->priv->reverse_check),
                                      op->priv->reverse_flag);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (op->priv->crop_marks_check),
                                      op->priv->crop_marks_flag);

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



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
