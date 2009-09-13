/*
 *  print-op-dialog.h
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

#ifndef __PRINT_OP_DIALOG_H__
#define __PRINT_OP_DIALOG_H__

#include "print-op.h"

G_BEGIN_DECLS

#define GL_TYPE_PRINT_OP_DIALOG            (gl_print_op_dialog_get_type ())
#define GL_PRINT_OP_DIALOG(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_PRINT_OP_DIALOG, glPrintOpDialog))
#define GL_PRINT_OP_DIALOG_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_PRINT_OP_DIALOG, glPrintOpDialogClass))
#define GL_IS_PRINT_OP_DIALOG(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_PRINT_OP_DIALOG))
#define GL_IS_PRINT_OP_DIALOG_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_PRINT_OP_DIALOG))
#define GL_PRINT_OP_DIALOG_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_PRINT_OP_DIALOG, glPrintOpDialogClass))


typedef struct _glPrintOpDialog         glPrintOpDialog;
typedef struct _glPrintOpDialogClass    glPrintOpDialogClass;

typedef struct _glPrintOpDialogPrivate  glPrintOpDialogPrivate;

struct _glPrintOpDialog
{
        glPrintOp                   parent_instance;

        glPrintOpDialogPrivate     *priv;

};

struct  _glPrintOpDialogClass
{
        glPrintOpClass              parent_class;
};


typedef struct _glPrintOpDialogSettings glPrintOpDialogSettings;



GType              gl_print_op_dialog_get_type           (void) G_GNUC_CONST;

glPrintOpDialog   *gl_print_op_dialog_new                (glLabel           *label);



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
