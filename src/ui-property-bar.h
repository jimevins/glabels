/*
 *  ui-property-bar.h
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

#ifndef __UI_PROPERTY_BAR_H__
#define __UI_PROPERTY_BAR_H__

#include <gtk/gtk.h>

#include "view.h"

G_BEGIN_DECLS

#define GL_TYPE_UI_PROPERTY_BAR (gl_ui_property_bar_get_type ())
#define GL_UI_PROPERTY_BAR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), GL_TYPE_UI_PROPERTY_BAR, glUIPropertyBar ))
#define GL_UI_PROPERTY_BAR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_UI_PROPERTY_BAR, glUIPropertyBarClass))
#define GL_IS_UI_PROPERTY_BAR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_UI_PROPERTY_BAR))
#define GL_IS_UI_PROPERTY_BAR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_UI_PROPERTY_BAR))

typedef struct _glUIPropertyBar        glUIPropertyBar;
typedef struct _glUIPropertyBarClass   glUIPropertyBarClass;

typedef struct _glUIPropertyBarPrivate glUIPropertyBarPrivate;

struct _glUIPropertyBar {
	GtkHBox                 parent_widget;

	glUIPropertyBarPrivate *priv;

};

struct _glUIPropertyBarClass {
	GtkHBoxClass            parent_class;
};

GType        gl_ui_property_bar_get_type          (void) G_GNUC_CONST;

GtkWidget   *gl_ui_property_bar_new               (void);

void         gl_ui_property_bar_set_view          (glUIPropertyBar *this,
						   glView          *view);


G_END_DECLS

#endif /* __UI_PROPERTY_BAR_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
