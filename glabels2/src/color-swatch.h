/*
 *  color-swatch.h
 *  Copyright (C) 2009  Jim Evins <evins@snaught.com>.
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

#ifndef __COLOR_SWATCH_H__
#define __COLOR_SWATCH_H__


#include <gtk/gtkdrawingarea.h>


G_BEGIN_DECLS

#define GL_TYPE_COLOR_SWATCH (gl_color_swatch_get_type ())
#define GL_COLOR_SWATCH(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), GL_TYPE_COLOR_SWATCH, glColorSwatch ))
#define GL_COLOR_SWATCH_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_COLOR_SWATCH, glColorSwatchClass))
#define GL_IS_COLOR_SWATCH(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_COLOR_SWATCH))
#define GL_IS_COLOR_SWATCH_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_COLOR_SWATCH))

typedef struct _glColorSwatch        glColorSwatch;
typedef struct _glColorSwatchPrivate glColorSwatchPrivate;
typedef struct _glColorSwatchClass   glColorSwatchClass;

struct _glColorSwatch {
	GtkDrawingArea                    parent_widget;

	glColorSwatchPrivate *priv;
};

struct _glColorSwatchClass {
	GtkDrawingAreaClass               parent_class;
};


GType      gl_color_swatch_get_type (void) G_GNUC_CONST;

GtkWidget *gl_color_swatch_new      (gint           w,
                                     gint           h,
                                     guint          color);

void       gl_color_swatch_set_color(glColorSwatch *this,
                                     guint          color);


G_END_DECLS

#endif /* __COLOR_SWATCH_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
