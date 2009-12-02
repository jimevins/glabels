/*
 *  view-ellipse.c
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

#include "view-ellipse.h"

#include "label-ellipse.h"

#include "pixmaps/cursor_ellipse.xbm"
#include "pixmaps/cursor_ellipse_mask.xbm"

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
gl_view_ellipse_get_create_cursor (void)
{
	GdkCursor       *cursor = NULL;
	GdkPixmap       *pixmap_data, *pixmap_mask;
	GdkColor         fg = { 0, 0, 0, 0 };
	GdkColor         bg = { 0, 65535, 65535, 65535 };

	gl_debug (DEBUG_VIEW, "START");

        pixmap_data = gdk_bitmap_create_from_data (NULL,
                                                   (gchar *)cursor_ellipse_bits,
                                                   cursor_ellipse_width,
                                                   cursor_ellipse_height);
        pixmap_mask = gdk_bitmap_create_from_data (NULL,
                                                   (gchar *)cursor_ellipse_mask_bits,
                                                   cursor_ellipse_mask_width,
                                                   cursor_ellipse_mask_height);
        cursor = gdk_cursor_new_from_pixmap (pixmap_data, pixmap_mask, &fg,
                                             &bg, cursor_ellipse_x_hot,
                                             cursor_ellipse_y_hot);

	gl_debug (DEBUG_VIEW, "END");

	return cursor;
}


/*****************************************************************************/
/* Object creation handler: button press event.                              */
/*****************************************************************************/
void
gl_view_ellipse_create_button_press_event   (glView *view,
                                             gdouble x,
                                             gdouble y)
{
	GObject             *object;

        gl_label_unselect_all (view->label);

        object = gl_label_ellipse_new (view->label);
        gl_label_object_set_position (GL_LABEL_OBJECT(object), x, y);
        gl_label_object_set_size (GL_LABEL_OBJECT(object), 0.0, 0.0);

        view->create_object = GL_LABEL_OBJECT (object);
        view->create_x0 = x;
        view->create_y0 = y;
}


/*****************************************************************************/
/* Object creation handler: motion event.                                    */
/*****************************************************************************/
void
gl_view_ellipse_create_motion_event     (glView *view,
                                         gdouble x,
                                         gdouble y)
{
        gdouble w, h;

        gl_label_object_set_position (GL_LABEL_OBJECT(view->create_object),
                                      MIN (x, view->create_x0), MIN (y, view->create_y0));
        w = MAX (x, view->create_x0) - MIN (x, view->create_x0);
        h = MAX (y, view->create_y0) - MIN (y, view->create_y0);
        gl_label_object_set_size (GL_LABEL_OBJECT(view->create_object), w, h);
}


/*****************************************************************************/
/* Object creation handler: button relesase event.                           */
/*****************************************************************************/
void
gl_view_ellipse_create_button_release_event (glView *view,
                                             gdouble x,
                                             gdouble y)
{
        gdouble              w, h;

        if ((view->create_x0 == x) && (view->create_y0 == y)) {
                x = view->create_x0 + 36.0;
                y = view->create_y0 + 36.0;
        }
        gl_label_object_set_position (GL_LABEL_OBJECT(view->create_object),
                                      MIN (x, view->create_x0), MIN (y, view->create_y0));
        w = MAX (x, view->create_x0) - MIN (x, view->create_x0);
        h = MAX (y, view->create_y0) - MIN (y, view->create_y0);
        gl_label_object_set_size (GL_LABEL_OBJECT(view->create_object), w, h);
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
