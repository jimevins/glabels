/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  cairo_ellipse_path.c:  Cairo ellipse path module
 *
 *  Copyright (C) 2001-2007  Jim Evins <evins@snaught.com>.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <config.h>

#include "cairo-ellipse-path.h"

#include <math.h>
#include <glib.h>

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


