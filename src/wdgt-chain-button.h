/*
 *  wdgt-chain-button.h
 *  Modified version of gimpchainbutton.h for gLabels:
 *
 *  LIBGIMP - The GIMP Library
 *  Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 *  gimpchainbutton.h
 *  Copyright (C) 1999-2000 Sven Neumann <sven@gimp.org>
 *
 *  Modified or gLabels by Jim Evins <evins@snaught.com>
 *
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

/*
 * This implements a widget derived from GtkTable that visualizes
 * it's state with two different pixmaps showing a closed and a
 * broken chain. It's intented to be used with the GimpSizeEntry
 * widget. The usage is quite similar to the one the GtkToggleButton
 * provides.
 */

#ifndef __WDGT_CHAIN_BUTTON_H__
#define __WDGT_CHAIN_BUTTON_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS


typedef enum
{
  GL_WDGT_CHAIN_TOP,
  GL_WDGT_CHAIN_LEFT,
  GL_WDGT_CHAIN_BOTTOM,
  GL_WDGT_CHAIN_RIGHT
} glWdgtChainPosition;


#define GL_WDGT_TYPE_CHAIN_BUTTON            (gl_wdgt_chain_button_get_type ())
#define GL_WDGT_CHAIN_BUTTON(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_WDGT_TYPE_CHAIN_BUTTON, glWdgtChainButton))
#define GL_WDGT_CHAIN_BUTTON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GL_WDGT_TYPE_CHAIN_BUTTON, glWdgtChainButtonClass))
#define GL_WDGT_IS_CHAIN_BUTTON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_WDGT_TYPE_CHAIN_BUTTON))
#define GL_WDGT_IS_CHAIN_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_WDGT_TYPE_CHAIN_BUTTON))
#define GL_WDGT_CHAIN_BUTTON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_WDGT_TYPE_CHAIN_BUTTON, glWdgtChainButtonClass))


typedef struct _glWdgtChainButton       glWdgtChainButton;
typedef struct _glWdgtChainButtonClass  glWdgtChainButtonClass;

struct _glWdgtChainButton
{
  GtkTable           parent_instance;

  glWdgtChainPosition  position;
  gboolean           active;

  GtkWidget         *button;
  GtkWidget         *line1;
  GtkWidget         *line2;
  GtkWidget         *image;
};

struct _glWdgtChainButtonClass
{
  GtkTableClass  parent_class;

  void (* toggled)  (glWdgtChainButton *button);
};


GType       gl_wdgt_chain_button_get_type   (void) G_GNUC_CONST;

GtkWidget * gl_wdgt_chain_button_new        (glWdgtChainPosition  position);

void        gl_wdgt_chain_button_set_active (glWdgtChainButton   *button,
					     gboolean             active);
gboolean    gl_wdgt_chain_button_get_active (glWdgtChainButton   *button);


G_END_DECLS

#endif /* __WDGT_CHAIN_BUTTON_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
