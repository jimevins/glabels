/*
 *  print-op.h
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

#ifndef __PRINT_OP_H__
#define __PRINT_OP_H__

#include <gtk/gtkprintoperation.h>
#include "label.h"

G_BEGIN_DECLS

#define GL_TYPE_PRINT_OP            (gl_print_op_get_type ())
#define GL_PRINT_OP(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_PRINT_OP, glPrintOp))
#define GL_PRINT_OP_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_PRINT_OP, glPrintOpClass))
#define GL_IS_PRINT_OP(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_PRINT_OP))
#define GL_IS_PRINT_OP_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_PRINT_OP))
#define GL_PRINT_OP_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_PRINT_OP, glPrintOpClass))


typedef struct _glPrintOp         glPrintOp;
typedef struct _glPrintOpClass    glPrintOpClass;

typedef struct _glPrintOpPrivate  glPrintOpPrivate;

struct _glPrintOp
{
        GtkPrintOperation     parent_instance;

        glPrintOpPrivate     *priv;

};

struct  _glPrintOpClass
{
        GtkPrintOperationClass        parent_class;
};


typedef struct _glPrintOpSettings glPrintOpSettings;



GType              gl_print_op_get_type            (void) G_GNUC_CONST;

glPrintOp         *gl_print_op_new                 (glLabel           *label);

void               gl_print_op_construct           (glPrintOp         *print_op,
                                                    glLabel           *label);

void               gl_print_op_set_filename        (glPrintOp         *print_op,
                                                    gchar             *filename);
void               gl_print_op_set_n_sheets        (glPrintOp         *print_op,
                                                    gint               n_sheets);
void               gl_print_op_set_n_copies        (glPrintOp         *print_op,
                                                    gint               n_copies);
void               gl_print_op_set_first           (glPrintOp         *print_op,
                                                    gint               first);
void               gl_print_op_set_last            (glPrintOp         *print_op,
                                                    gint               last);
void               gl_print_op_set_collate_flag    (glPrintOp         *print_op,
                                                    gboolean           collate_flag);
void               gl_print_op_set_outline_flag    (glPrintOp         *print_op,
                                                    gboolean           outline_flag);
void               gl_print_op_set_reverse_flag    (glPrintOp         *print_op,
                                                    gboolean           reverse_flag);
void               gl_print_op_set_crop_marks_flag (glPrintOp         *print_op,
                                                    gboolean           crop_marks_flag);

gchar             *gl_print_op_get_filename        (glPrintOp         *print_op);
gint               gl_print_op_get_n_sheets        (glPrintOp         *print_op);
gint               gl_print_op_get_n_copies        (glPrintOp         *print_op);
gint               gl_print_op_get_first           (glPrintOp         *print_op);
gint               gl_print_op_get_last            (glPrintOp         *print_op);
gboolean           gl_print_op_get_collate_flag    (glPrintOp         *print_op);
gboolean           gl_print_op_get_outline_flag    (glPrintOp         *print_op);
gboolean           gl_print_op_get_reverse_flag    (glPrintOp         *print_op);
gboolean           gl_print_op_get_crop_marks_flag (glPrintOp         *print_op);

void               gl_print_op_force_outline       (glPrintOp         *print_op);
gboolean           gl_print_op_is_outline_forced   (glPrintOp         *print_op);

glPrintOpSettings *gl_print_op_get_settings        (glPrintOp         *print_op);
void               gl_print_op_set_settings        (glPrintOp         *print_op,
                                                    glPrintOpSettings *settings);
void               gl_print_op_free_settings       (glPrintOpSettings *settings);
                                          

G_END_DECLS

#endif



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
