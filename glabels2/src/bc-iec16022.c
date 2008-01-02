/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  bc-iec16022.c:  front-end to iec16022-library module
 *
 *  Copyright (C) 2001-2006  Jim Evins <evins@snaught.com>.
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

#include "bc-iec16022.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <glib/gmessages.h>

#include "iec16022ecc200.h"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/
#define MIN_PIXEL_SIZE 1.0

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/
static glBarcode *render_iec16022 (const gchar *grid,
                                   gint         i_width,
                                   gint         i_height,
                                   gdouble      w,
                                   gdouble      h);

/*****************************************************************************/
/* Generate intermediate representation of barcode.                          */
/*****************************************************************************/
glBarcode *
gl_barcode_iec16022_new (const gchar    *id,
                         gboolean        text_flag,
                         gboolean        checksum_flag,
                         gdouble         w,
                         gdouble         h,
                         const gchar    *digits)
{
        gchar               *grid;
        gint                 i_width, i_height;
        glBarcode           *gbc;

        if ( strlen (digits) == 0 )
        {
                return NULL;
        }

        i_width  = 0;
        i_height = 0;

        grid = (gchar *)iec16022ecc200 (&i_width, &i_height, NULL,
                                        strlen (digits), (unsigned char *)digits,
                                        NULL, NULL, NULL);

        /* now render with our custom back-end,
           to create appropriate intermdediate format */
        gbc = render_iec16022 (grid, i_width, i_height, w, h);

        free (grid);
        return gbc;
}

/*--------------------------------------------------------------------------
 * PRIVATE.  Render to glBarcode intermediate representation of barcode.
 *--------------------------------------------------------------------------*/
static glBarcode *
render_iec16022 (const gchar *grid,
                 gint         i_width,
                 gint         i_height,
                 gdouble      w,
                 gdouble      h)
{
        glBarcode     *gbc;
        glBarcodeLine *line;
        gint           x, y;
        gdouble        aspect_ratio, pixel_size;

	/* Treat requested size as a bounding box, scale to maintain aspect
	 * ratio while fitting it in this bounding box. */
	aspect_ratio = (gdouble)i_height / (gdouble)i_width;
	if ( h > w*aspect_ratio ) {
		h = w * aspect_ratio;
	} else {
		w = h / aspect_ratio;
	}

        /* Now determine pixel size. */
        pixel_size = w / i_width;
        if ( pixel_size < MIN_PIXEL_SIZE )
        {
                pixel_size = MIN_PIXEL_SIZE;
        }

        gbc = g_new0 (glBarcode, 1);

        /* Now traverse the code string and create a list of boxes */
        for ( y = i_height-1; y >= 0; y-- )
        {

                for ( x = 0; x < i_width; x++ )
                {

                        if (*grid++)
                        {
                                line = g_new0 (glBarcodeLine, 1);
                                line->x      = x*pixel_size + pixel_size/2.0;
                                line->y      = y*pixel_size;
                                line->length = pixel_size;
                                line->width  = pixel_size;
                                gbc->lines = g_list_append (gbc->lines, line);
                        }

                }

        }

        /* Fill in other info */
        gbc->height = i_height * pixel_size;
        gbc->width  = i_width  * pixel_size;

#if 0
        g_print ("w=%f, h=%f\n", gbc->width, gbc->height);
#endif

        return gbc;
}

