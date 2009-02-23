/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  print-op.h:  Print operation module header file
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



GType              gl_print_op_get_type           (void) G_GNUC_CONST;

glPrintOp         *gl_print_op_new                (glLabel           *label);

void               gl_print_op_force_outline_flag (glPrintOp         *print_op);

glPrintOpSettings *gl_print_op_get_settings          (glPrintOp         *print_op);
void               gl_print_op_set_settings          (glPrintOp         *print_op,
                                                      glPrintOpSettings *settings);
void               gl_print_op_free_settings         (glPrintOpSettings *settings);
                                          
/*
 * Batch print operation
 */
glPrintOp         *gl_print_op_new_batch          (glLabel           *label,
                                                   gchar             *filename,
                                                   gint               n_sheets,
                                                   gint               n_copies,
                                                   gint               first,
                                                   gboolean           outline_flag,
                                                   gboolean           reverse_flag,
                                                   gboolean           crop_marks_flag);

G_END_DECLS

#endif
