/*
 *  message-bar.h
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

#ifndef __MESSAGE_BAR_H__
#define __MESSAGE_BAR_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GL_TYPE_MESSAGE_BAR            (gl_message_bar_get_type ())
#define GL_MESSAGE_BAR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_MESSAGE_BAR, glMessageBar))
#define GL_MESSAGE_BAR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_MESSAGE_BAR, glMessageBarClass))
#define GL_IS_MESSAGE_BAR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_MESSAGE_BAR))
#define GL_IS_MESSAGE_BAR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_MESSAGE_BAR))
#define GL_MESSAGE_BAR_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_MESSAGE_BAR, glMessageBarClass))


typedef struct _glMessageBar         glMessageBar;
typedef struct _glMessageBarClass    glMessageBarClass;

typedef struct _glMessageBarPrivate  glMessageBarPrivate;

struct _glMessageBar
{
	GtkInfoBar           parent_instance;

	glMessageBarPrivate *priv;

};

struct  _glMessageBarClass
{
	GtkInfoBarClass      parent_class;
};


GType      gl_message_bar_get_type                (void) G_GNUC_CONST;

GtkWidget *gl_message_bar_new                     (GtkMessageType  type,
                                                   GtkButtonsType  buttons,
                                                   const gchar    *message_format,
                                                   ...);

void       gl_message_bar_format_secondary_text   (glMessageBar   *this,
                                                   const gchar    *message_format,
                                                   ...);




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
