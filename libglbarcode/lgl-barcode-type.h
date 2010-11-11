/*
 *  lgl-barcode-type.h
 *  Copyright (C) 2001-2010  Jim Evins <evins@snaught.com>.
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

#ifndef __LGL_BARCODE_TYPE_H__
#define __LGL_BARCODE_TYPE_H__

#include <glib.h>

G_BEGIN_DECLS


typedef enum {

        LGL_BARCODE_TYPE_POSTNET,        /* USPS Postnet Barcode (5, 9, or 11 digits) */
        LGL_BARCODE_TYPE_POSTNET_5,      /* USPS Postnet Barcode (5 digits) */
        LGL_BARCODE_TYPE_POSTNET_9,      /* USPS Postnet Barcode (9 digits) */
        LGL_BARCODE_TYPE_POSTNET_11,     /* USPS Postnet Barcode (11 digits) */
        LGL_BARCODE_TYPE_CEPNET,         /* Brazilian Postal Code */

        LGL_BARCODE_TYPE_ONECODE,        /* USPS Intelligent Mail Barcode */

        LGL_BARCODE_TYPE_CODE39,         /* Code 39 */
        LGL_BARCODE_TYPE_CODE39_EXT,     /* Extended Code 39 (Supports full ASCII character set) */

        /*< private >*/
        LGL_BARCODE_N_TYPES

} lglBarcodeType;


G_END_DECLS

#endif /* __LGL_BARCODE_TYPE_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
