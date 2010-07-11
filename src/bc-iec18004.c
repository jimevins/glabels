/*
 *  bc-iec18004.c
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

#ifdef HAVE_LIBQRENCODE

#include "bc-iec18004.h"

#include <glib.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <qrencode.h>

#include "debug.h"


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

#define MIN_PIXEL_SIZE 1.0


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/
static glBarcode *render_iec18004 (const gchar *grid,
                                   gint         i_width,
                                   gint         i_height,
                                   gdouble      w,
                                   gdouble      h);


/*****************************************************************************/
/* Generate intermediate representation of barcode.                          */
/*****************************************************************************/
glBarcode *
gl_barcode_iec18004_new (const gchar    *id,
                         gboolean        text_flag,
                         gboolean        checksum_flag,
                         gdouble         w,
                         gdouble         h,
                         const gchar    *digits)
{
        gint             i_width, i_height;
        glBarcode       *gbc;
	QRcode          *qrcode;

        if ( strlen (digits) == 0 )
        {
                return NULL;
        }

        i_width  = 0;
        i_height = 0;

        qrcode = QRcode_encodeString ((const char *)digits, 0, QR_ECLEVEL_M,
                                      QR_MODE_8, 1);
        if (qrcode == NULL)
        {
                return NULL;
        }
        
	i_width = i_height = qrcode->width;
	gbc = render_iec18004 ((const gchar *)qrcode->data, i_width, i_height,
                               w, h);

	QRcode_free ( qrcode );
	 
        return gbc;
}


/*--------------------------------------------------------------------------
 * PRIVATE.  Render to glBarcode intermediate representation of barcode.
 *--------------------------------------------------------------------------*/
static glBarcode *
render_iec18004 (const gchar *grid,
                 gint         i_width,
                 gint         i_height,
                 gdouble      w,
                 gdouble      h)
{
        glBarcode          *gbc;
        gint                x, y;
        gdouble             aspect_ratio, pixel_size;

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
        for ( y = 0; y < i_height; y++ )
        {
                for ( x = 0; x < i_width; x++ )
                {

                        /* Symbol data is represented as an array contains 
                         * width*width uchars. Each uchar represents a module 
                         * (dot). If the less significant bit of the uchar 
                         * is 1, the corresponding module is black. The other
                         * bits are meaningless for us. */
                        if ((*grid++) & 1)
                        {
                                gl_barcode_add_box (gbc, x*pixel_size, y*pixel_size, pixel_size, pixel_size);
                        }

                }

        }

        /* Fill in other info */
        gbc->height = i_height * pixel_size;
        gbc->width  = i_width  * pixel_size;

        return gbc;
}

#endif /* HAVE_LIBQRENCODE */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
