/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  template-designer.h:  Template designer module header file
 *
 *  Copyright (C) 2003  Jim Evins <evins@snaught.com>.
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

#ifndef __TEMPLATE_DESIGNER_H__
#define __TEMPLATE_DESIGNER_H__

#include <gtk/gtkassistant.h>
#include <gtk/gtkwindow.h>

G_BEGIN_DECLS

#define GL_TYPE_TEMPLATE_DESIGNER (gl_template_designer_get_type ())
#define GL_TEMPLATE_DESIGNER(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_TEMPLATE_DESIGNER, glTemplateDesigner))
#define GL_TEMPLATE_DESIGNER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_TEMPLATE_DESIGNER, glTemplateDesignerClass))
#define GL_IS_TEMPLATE_DESIGNER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_TEMPLATE_DESIGNER))
#define GL_IS_TEMPLATE_DESIGNER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_TEMPLATE_DESIGNER))
#define GL_TEMPLATE_DESIGNER_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_TEMPLATE_DESIGNER, glTemplateDesignerClass))


typedef struct _glTemplateDesigner              glTemplateDesigner;
typedef struct _glTemplateDesignerClass         glTemplateDesignerClass;

typedef struct _glTemplateDesignerPrivate       glTemplateDesignerPrivate;

struct _glTemplateDesigner
{
        GtkAssistant               parent_instance;

        glTemplateDesignerPrivate *priv;

};

struct  _glTemplateDesignerClass
{
        GtkAssistantClass          parent_class;
};

GType           gl_template_designer_get_type               (void) G_GNUC_CONST;

GtkWidget      *gl_template_designer_new                    (GtkWindow *parent);


G_END_DECLS

#endif /* __TEMPLATE_DESIGNER_H__ */
