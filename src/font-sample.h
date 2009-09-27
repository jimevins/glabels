/*
 *  font-sample.h
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

#ifndef __FONT_SAMPLE_H__
#define __FONT_SAMPLE_H__


#include <gtk/gtk.h>


G_BEGIN_DECLS

#define GL_TYPE_FONT_SAMPLE (gl_font_sample_get_type ())
#define GL_FONT_SAMPLE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), GL_TYPE_FONT_SAMPLE, glFontSample ))
#define GL_FONT_SAMPLE_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_FONT_SAMPLE, glFontSampleClass))
#define GL_IS_FONT_SAMPLE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_FONT_SAMPLE))
#define GL_IS_FONT_SAMPLE_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_FONT_SAMPLE))

typedef struct _glFontSample        glFontSample;
typedef struct _glFontSamplePrivate glFontSamplePrivate;
typedef struct _glFontSampleClass   glFontSampleClass;

struct _glFontSample {
	GtkDrawingArea         parent_widget;

	glFontSamplePrivate   *priv;
};

struct _glFontSampleClass {
	GtkDrawingAreaClass    parent_class;
};


GType      gl_font_sample_get_type (void) G_GNUC_CONST;

GtkWidget *gl_font_sample_new      (gint           w,
                                    gint           h,
                                    const gchar   *sample_text,
                                    const gchar   *font_family);


G_END_DECLS

#endif /* __FONT_SAMPLE_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
