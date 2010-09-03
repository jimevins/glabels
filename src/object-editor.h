/*
 *  object-editor.h
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

#ifndef __OBJECT_EDITOR_H__
#define __OBJECT_EDITOR_H__

#include <gtk/gtk.h>

#include "text-node.h"
#include "label.h"
#include "merge.h"
#include "color.h"

G_BEGIN_DECLS

#define GL_TYPE_OBJECT_EDITOR            (gl_object_editor_get_type ())
#define GL_OBJECT_EDITOR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_OBJECT_EDITOR, glObjectEditor))
#define GL_OBJECT_EDITOR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_OBJECT_EDITOR, glObjectEditorClass))
#define GL_IS_OBJECT_EDITOR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_OBJECT_EDITOR))
#define GL_IS_OBJECT_EDITOR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_OBJECT_EDITOR))
#define GL_OBJECT_EDITOR_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_OBJECT_EDITOR, glObjectEditorClass))


typedef struct _glObjectEditor         glObjectEditor;
typedef struct _glObjectEditorClass    glObjectEditorClass;

typedef struct _glObjectEditorPrivate  glObjectEditorPrivate;

struct _glObjectEditor
{
	GtkVBox                parent_instance;

	glObjectEditorPrivate *priv;

};

struct  _glObjectEditorClass
{
	GtkVBoxClass             parent_class;
};




GType       gl_object_editor_get_type             (void) G_GNUC_CONST;

GtkWidget  *gl_object_editor_new                  (void);

void        gl_object_editor_set_label            (glObjectEditor      *editor,
                                                   glLabel             *label);



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
