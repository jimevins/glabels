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
#include <gtk/gtk.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#include <libglabels.h>
#include "print.h"
#include "label.h"

#include "debug.h"


/*===========================================*/
/* Private data types                        */
/*===========================================*/

struct _glPrintOpPrivate {

	glLabel   *label;

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

static void     gl_print_op_finalize          (GObject           *object);

static void     set_page_size                 (glPrintOp         *op,
                                               glLabel           *label);

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
G_DEFINE_TYPE (glPrintOp, gl_print_op, GTK_TYPE_PRINT_OPERATION)


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

        g_object_unref (G_OBJECT(op->priv->label));
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


/*****************************************************************************/
/* Construct print op.                                                       */
/*****************************************************************************/
void
gl_print_op_construct (glPrintOp      *op,
                       glLabel        *label)
{
        glMerge                *merge = NULL;
        const lglTemplate      *template;
        const lglTemplateFrame *frame;

	op->priv->label              = label;
	op->priv->force_outline_flag = FALSE;

        merge    = gl_label_get_merge (label);
        template = gl_label_get_template (label);
        frame    = (lglTemplateFrame *)template->frames->data;

        op->priv->merge_flag         = (merge != NULL);
        op->priv->n_sheets           = 1;
        op->priv->first              = 1;
        op->priv->last               = lgl_template_frame_get_n_labels (frame);
        op->priv->n_copies           = 1;

        set_page_size (op, label);

	gtk_print_operation_set_custom_tab_label ( GTK_PRINT_OPERATION (op),
						   _("Labels"));

	g_signal_connect (G_OBJECT (op), "begin-print",
			  G_CALLBACK (begin_print_cb), label);

	g_signal_connect (G_OBJECT (op), "draw-page",
			  G_CALLBACK (draw_page_cb), label);
}


/*****************************************************************************/
/* Set outline flag/checkbox.                                                */
/*****************************************************************************/
void
gl_print_op_force_outline (glPrintOp *op)
{
        op->priv->force_outline_flag = TRUE;
}


/*****************************************************************************/
/* Set outline flag/checkbox.                                                */
/*****************************************************************************/
gboolean
gl_print_op_is_outline_forced (glPrintOp *op)
{
        return op->priv->force_outline_flag;
}


/*****************************************************************************/
/* Set job parameters.                                                       */
/*****************************************************************************/
void
gl_print_op_set_filename (glPrintOp *op,
                          gchar     *filename)
{
        gtk_print_operation_set_export_filename (GTK_PRINT_OPERATION (op),
                                                 filename);
}


void
gl_print_op_set_n_sheets (glPrintOp *op,
                          gint       n_sheets)
{
        op->priv->n_sheets = n_sheets;
}


void
gl_print_op_set_n_copies (glPrintOp *op,
                          gint       n_copies)
{
        op->priv->n_copies = n_copies;
}


void
gl_print_op_set_first (glPrintOp *op,
                       gint       first)
{
        op->priv->first = first;
}


void
gl_print_op_set_last (glPrintOp *op,
                      gint       last)
{
        op->priv->last = last;
}


void
gl_print_op_set_collate_flag (glPrintOp *op,
                              gboolean   collate_flag)
{
        op->priv->collate_flag = collate_flag;
}


void
gl_print_op_set_outline_flag (glPrintOp *op,
                              gboolean   outline_flag)
{
        op->priv->outline_flag = outline_flag;
}


void
gl_print_op_set_reverse_flag (glPrintOp *op,
                              gboolean   reverse_flag)
{
        op->priv->reverse_flag = reverse_flag;
}


void
gl_print_op_set_crop_marks_flag (glPrintOp *op,
                                 gboolean   crop_marks_flag)
{
        op->priv->crop_marks_flag = crop_marks_flag;
}


/*****************************************************************************/
/* Get job parameters.                                                       */
/*****************************************************************************/
gchar *
gl_print_op_get_filename (glPrintOp *op)
{
        gchar *filename = NULL;

        g_object_get (G_OBJECT (op),
                      "export_filename", filename,
                      NULL);

        return filename;
}


gint
gl_print_op_get_n_sheets (glPrintOp *op)
{
        return op->priv->n_sheets;
}


gint
gl_print_op_get_n_copies (glPrintOp *op)
{
        return op->priv->n_copies;
}


gint
gl_print_op_get_first (glPrintOp *op)
{
        return op->priv->first;
}


gint
gl_print_op_get_last (glPrintOp *op)
{
        return op->priv->last;
}


gboolean
gl_print_op_get_collate_flag (glPrintOp *op)
{
        return op->priv->collate_flag;
}


gboolean
gl_print_op_get_outline_flag (glPrintOp *op)
{
        return op->priv->outline_flag;
}


gboolean
gl_print_op_get_reverse_flag (glPrintOp *op)
{
        return op->priv->reverse_flag;
}


gboolean
gl_print_op_get_crop_marks_flag (glPrintOp *op)
{
        return op->priv->crop_marks_flag;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Set page size.                                                 */
/*--------------------------------------------------------------------------*/
static void
set_page_size (glPrintOp  *op,
               glLabel    *label)
{
        const lglTemplate *template;
        GtkPaperSize      *psize;
        GtkPageSetup      *su;
        lglPaper          *paper;

	gl_debug (DEBUG_PRINT, "begin");

        template = gl_label_get_template (label);
        paper = lgl_db_lookup_paper_from_id (template->paper_id);

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
                                                   template->page_width,
                                                   template->page_height,
                                                   GTK_UNIT_POINTS);
                gl_debug (DEBUG_PRINT, "Using custom size = %g x %g points",
                          template->page_width,
                          template->page_height);

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




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
