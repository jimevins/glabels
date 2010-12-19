/*
 *  bc-builtin.c
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

/*
 * This module implements the BUILTIN barcode specified in the USPS
 * publication 25, Mar 2001.
 */

#include <config.h>

#include "bc-builtin.h"

#include "debug.h"


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/


/****************************************************************************/
/* Generate list of lines that form the barcode for the given digits.       */
/****************************************************************************/
lglBarcode *
gl_barcode_builtin_new (const gchar    *id,
                        gboolean        text_flag,
                        gboolean        checksum_flag,
                        gdouble         w,
                        gdouble         h,
                        const gchar    *digits)
{
        if ( (g_ascii_strcasecmp (id, "POSTNET") == 0) )
        {
                return lgl_barcode_create (LGL_BARCODE_TYPE_POSTNET, text_flag, checksum_flag, w, h, digits);
        }
        if ( (g_ascii_strcasecmp (id, "POSTNET-5") == 0) )
        {
                return lgl_barcode_create (LGL_BARCODE_TYPE_POSTNET_5, text_flag, checksum_flag, w, h, digits);
        }
        if ( (g_ascii_strcasecmp (id, "POSTNET-9") == 0) )
        {
                return lgl_barcode_create (LGL_BARCODE_TYPE_POSTNET_9, text_flag, checksum_flag, w, h, digits);
        }
        if ( (g_ascii_strcasecmp (id, "POSTNET-11") == 0) )
        {
                return lgl_barcode_create (LGL_BARCODE_TYPE_POSTNET_11, text_flag, checksum_flag, w, h, digits);
        }
        if ( (g_ascii_strcasecmp (id, "CEPNET") == 0) )
        {
                return lgl_barcode_create (LGL_BARCODE_TYPE_CEPNET, text_flag, checksum_flag, w, h, digits);
        }
        if ( (g_ascii_strcasecmp (id, "ONECODE") == 0) )
        {
                return lgl_barcode_create (LGL_BARCODE_TYPE_ONECODE, text_flag, checksum_flag, w, h, digits);
        }
        if ( (g_ascii_strcasecmp (id, "Code39") == 0) )
        {
                return lgl_barcode_create (LGL_BARCODE_TYPE_CODE39, text_flag, checksum_flag, w, h, digits);
        }
        if ( (g_ascii_strcasecmp (id, "Code39Ext") == 0) )
        {
                return lgl_barcode_create (LGL_BARCODE_TYPE_CODE39_EXT, text_flag, checksum_flag, w, h, digits);
        }

        g_message ("Invalid builtin barcode ID: \"%s\"\n", id);
        return NULL;
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
