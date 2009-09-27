/*
 *  rotate-label-button.h
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

#ifndef __ROTATE_LABEL_BUTTON_H__
#define __ROTATE_LABEL_BUTTON_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GL_TYPE_ROTATE_LABEL_BUTTON (gl_rotate_label_button_get_type ())
#define GL_ROTATE_LABEL_BUTTON(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), GL_TYPE_ROTATE_LABEL_BUTTON, glRotateLabelButton ))
#define GL_ROTATE_LABEL_BUTTON_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_ROTATE_LABEL_BUTTON, glRotateLabelButtonClass))
#define GL_IS_ROTATE_LABEL_BUTTON(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_ROTATE_LABEL_BUTTON))
#define GL_IS_ROTATE_LABEL_BUTTON_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_ROTATE_LABEL_BUTTON))


typedef struct _glRotateLabelButton         glRotateLabelButton;
typedef struct _glRotateLabelButtonClass    glRotateLabelButtonClass;

typedef struct _glRotateLabelButtonPrivate  glRotateLabelButtonPrivate;

struct _glRotateLabelButton {
	GtkHBox                      parent_widget;

	glRotateLabelButtonPrivate  *priv;
};

struct _glRotateLabelButtonClass {
	GtkHBoxClass                 parent_class;

	void (*changed) (glRotateLabelButton  *rotate_label_button,
			 gpointer              user_data);
};


GType      gl_rotate_label_button_get_type          (void) G_GNUC_CONST;

GtkWidget *gl_rotate_label_button_new               (void);

gboolean   gl_rotate_label_button_get_state         (glRotateLabelButton *rotate_label);

void       gl_rotate_label_button_set_state         (glRotateLabelButton *rotate_label,
                                                     gboolean             state);

void       gl_rotate_label_button_set_template_name (glRotateLabelButton *rotate_label,
                                                     gchar               *name);

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
