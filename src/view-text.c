/*
 *  view-text.c
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

#include <config.h>

#include "view-text.h"

#include <glib/gi18n.h>

#include "label-text.h"

#include "pixmaps/cursor_text.xbm"
#include "pixmaps/cursor_text_mask.xbm"

#include "debug.h"


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/


/*========================================================*/
/* Private types.                                         */
/*========================================================*/


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/


/*****************************************************************************/
/* Return apropos cursor for create object mode.                             */
/*****************************************************************************/
GdkCursor *
gl_view_text_get_create_cursor (void)
{
	GdkCursor       *cursor = NULL;
	GdkPixmap       *pixmap_data, *pixmap_mask;
	GdkColor         fg = { 0, 0, 0, 0 };
	GdkColor         bg = { 0, 65535, 65535, 65535 };

	gl_debug (DEBUG_VIEW, "START");

        pixmap_data = gdk_bitmap_create_from_data (NULL,
                                                   (gchar *)cursor_text_bits,
                                                   cursor_text_width,
                                                   cursor_text_height);
        pixmap_mask = gdk_bitmap_create_from_data (NULL,
                                                   (gchar *)cursor_text_mask_bits,
                                                   cursor_text_mask_width,
                                                   cursor_text_mask_height);
        cursor = gdk_cursor_new_from_pixmap (pixmap_data, pixmap_mask, &fg,
                                             &bg, cursor_text_x_hot,
                                             cursor_text_y_hot);

	gl_debug (DEBUG_VIEW, "END");

	return cursor;
}


/*****************************************************************************/
/* Object creation handler: button press event.                              */
/*****************************************************************************/
void
gl_view_text_create_button_press_event   (glView *view,
                                          gdouble x,
                                          gdouble y)
{
	GObject             *object;
	GList               *lines;

        gl_label_unselect_all (view->label);

        object = gl_label_text_new (view->label, TRUE);

        gl_label_object_set_position (GL_LABEL_OBJECT(object), x, y, FALSE);
        lines = gl_text_node_lines_new_from_text (_("Text"));
        gl_label_text_set_lines (GL_LABEL_TEXT(object), lines, FALSE);

        view->create_object = GL_LABEL_OBJECT (object);
        view->create_x0 = x;
        view->create_y0 = y;
}


/*****************************************************************************/
/* Object creation handler: motion event.                                    */
/*****************************************************************************/
void
gl_view_text_create_motion_event     (glView *view,
                                      gdouble x,
                                      gdouble y)
{
        gl_label_object_set_position (GL_LABEL_OBJECT(view->create_object), x, y, FALSE);
}


/*****************************************************************************/
/* Object creation handler: button relesase event.                           */
/*****************************************************************************/
void
gl_view_text_create_button_release_event (glView *view,
                                          gdouble x,
                                          gdouble y)
{
        gl_label_object_set_position (GL_LABEL_OBJECT(view->create_object), x, y, FALSE);
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
