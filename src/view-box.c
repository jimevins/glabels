/*
 *  view-box.c
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

#include "view-box.h"

#include <gdk-pixbuf/gdk-pixdata.h>

#include "label-box.h"
#include "cursors/cursor_pixdata.h"

#include "debug.h"


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

#define X_HOTSPOT 7
#define Y_HOTSPOT 7


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
gl_view_box_get_create_cursor (void)
{
	GdkCursor       *cursor = NULL;
	GdkPixbuf       *pixbuf;

	gl_debug (DEBUG_VIEW, "START");

        pixbuf = gdk_pixbuf_from_pixdata (&cursor_box_pixdata, FALSE, NULL);
        cursor = gdk_cursor_new_from_pixbuf (gdk_display_get_default (), pixbuf, X_HOTSPOT, Y_HOTSPOT);
        g_object_unref (pixbuf);

	gl_debug (DEBUG_VIEW, "END");

	return cursor;
}


/*****************************************************************************/
/* Object creation handler: button press event.                              */
/*****************************************************************************/
void
gl_view_box_create_button_press_event   (glView *view,
                                         gdouble x,
                                         gdouble y)
{
	GObject             *object;

        gl_label_unselect_all (view->label);

        object = gl_label_box_new (view->label, TRUE);

        gl_label_object_set_position (GL_LABEL_OBJECT(object), x, y, FALSE);
        gl_label_object_set_size (GL_LABEL_OBJECT(object), 0.0, 0.0, FALSE);
			
        view->create_object = GL_LABEL_OBJECT (object);
        view->create_x0 = x;
        view->create_y0 = y;
}


/*****************************************************************************/
/* Object creation handler: motion event.                                    */
/*****************************************************************************/
void
gl_view_box_create_motion_event         (glView *view,
                                         gdouble x,
                                         gdouble y)
{
        gdouble w, h;

        gl_label_object_set_position (GL_LABEL_OBJECT(view->create_object),
                                      MIN (x, view->create_x0), MIN (y, view->create_y0),
                                      FALSE);

        w = MAX (x, view->create_x0) - MIN (x, view->create_x0);
        h = MAX (y, view->create_y0) - MIN (y, view->create_y0);
        gl_label_object_set_size (GL_LABEL_OBJECT(view->create_object), w, h, FALSE);
}


/*****************************************************************************/
/* Object creation handler: button relesase event.                           */
/*****************************************************************************/
void
gl_view_box_create_button_release_event (glView *view,
                                         gdouble x,
                                         gdouble y)
{
        gdouble              w, h;

        if ((view->create_x0 == x) && (view->create_y0 == y)) {
                x = view->create_x0 + 36.0;
                y = view->create_y0 + 36.0;
        }
        gl_label_object_set_position (GL_LABEL_OBJECT(view->create_object),
                                      MIN (x, view->create_x0), MIN (y, view->create_y0),
                                      FALSE);

        w = MAX (x, view->create_x0) - MIN (x, view->create_x0);
        h = MAX (y, view->create_y0) - MIN (y, view->create_y0);
        gl_label_object_set_size (GL_LABEL_OBJECT(view->create_object), w, h, FALSE);
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
