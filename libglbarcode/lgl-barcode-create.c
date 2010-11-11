/*
 *  lgl-barcode-create.c
 *  Copyright (C) 2010  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of libglbarcode.
 *
 *  libglbarcode is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libglbarcode is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libglbarcode.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "lgl-barcode-create.h"

#include "lgl-barcode-postnet.h"
#include "lgl-barcode-onecode.h"
#include "lgl-barcode-code39.h"


/*===========================================*/
/* Private macros and constants.             */
/*===========================================*/


/*===========================================*/
/* Private types                             */
/*===========================================*/

typedef lglBarcode *(*BarcodeNewFunc) (lglBarcodeType  type,
                                       gboolean        text_flag,
                                       gboolean        checksum_flag,
                                       gdouble         w,
                                       gdouble         h,
                                       const gchar    *data);


/*===========================================*/
/* Private globals                           */
/*===========================================*/

BarcodeNewFunc create_func[LGL_BARCODE_N_TYPES] = {

        /* LGL_BARCODE_TYPE_POSTNET    */ lgl_barcode_postnet_new,
        /* LGL_BARCODE_TYPE_POSTNET_5  */ lgl_barcode_postnet_new,
        /* LGL_BARCODE_TYPE_POSTNET_9  */ lgl_barcode_postnet_new,
        /* LGL_BARCODE_TYPE_POSTNET_11 */ lgl_barcode_postnet_new,
        /* LGL_BARCODE_TYPE_CEPNET     */ lgl_barcode_postnet_new,

        /* LGL_BARCODE_TYPE_ONECODE    */ lgl_barcode_onecode_new,

        /* LGL_BARCODE_TYPE_CODE39     */ lgl_barcode_code39_new,
        /* LGL_BARCODE_TYPE_CODE39_EXT */ lgl_barcode_code39_new,
};

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/


/****************************************************************************/
/**
 * lgl_barcode_create:
 * @type:           Barcode type selection (#lglBarcodeType)
 * @text_flag:      %TRUE to show text, if supported by barcode type
 * @checksum_flag:  %TRUE to include checksum, if supported or optional for barcode type
 * @w:              Suggested width of barcode
 * @h:              Suggested height of barcode
 * @data:           Data to encode into barcode
 *
 * Create a new barcode structure, encoding @data with selected barcode type and
 * characteristics.
 *
 * Barcode dimensions (@w and @h) are in points ( 1 point = 1/72 inch ).
 * If either @w or @h are zero, the barcode will be rendered in a nominal size
 * appropriate for the barcode type and data.  The actual size of the resulting
 * barcode may also be limited by required tolerances of line sizes and spacing
 * for the given barcode type. 
 *
 *
 * Returns: A newly allocated #lglBarcode structure.  Use lgl_barcode_free() to
 *          free it.
 */
lglBarcode *
lgl_barcode_create (lglBarcodeType     type,
                    gboolean           text_flag,
                    gboolean           checksum_flag,
                    gdouble            w,
                    gdouble            h,
                    const gchar       *data)
{
        if ( (type < 0) || (type >= LGL_BARCODE_N_TYPES) )
        {
                g_message ("Invalid barcode type.");
                return NULL;
        }

        return create_func[type] (type, text_flag, checksum_flag, w, h, data);
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
