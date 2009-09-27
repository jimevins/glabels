/*
 *  cairo-ellipse-path.c
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

#include "cairo-ellipse-path.h"

#include <math.h>

#include "debug.h"


/*===========================================*/
/* Private macros and constants.             */
/*===========================================*/

#define ARC_FINE   2


/*===========================================*/
/* Private types                             */
/*===========================================*/


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/


/*****************************************************************************/
/* Create ellipse path                                                       */
/*****************************************************************************/
void
gl_cairo_ellipse_path (cairo_t           *cr,
                       gdouble            rx,
                       gdouble            ry)
{
        gdouble x, y;
        gint i_theta;

        gl_debug (DEBUG_VIEW, "START");

        cairo_new_path (cr);
        cairo_move_to (cr, 2*rx, ry);
        for (i_theta = ARC_FINE; i_theta <= 360; i_theta += ARC_FINE) {
                x = rx + rx * cos (i_theta * G_PI / 180.0);
                y = ry + ry * sin (i_theta * G_PI / 180.0);
                cairo_line_to (cr, x, y);
        }
        cairo_close_path (cr);

        gl_debug (DEBUG_VIEW, "END");
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
