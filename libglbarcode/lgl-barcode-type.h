/*
 *  lgl-barcode-type.h
 *  Copyright (C) 2001-2010  Jim Evins <evins@snaught.com>.
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

#ifndef __LGL_BARCODE_TYPE_H__
#define __LGL_BARCODE_TYPE_H__

#include <glib.h>

G_BEGIN_DECLS


typedef enum {

        LGL_BARCODE_TYPE_POSTNET,
        LGL_BARCODE_TYPE_POSTNET_5,
        LGL_BARCODE_TYPE_POSTNET_9,
        LGL_BARCODE_TYPE_POSTNET_11,
        LGL_BARCODE_TYPE_CEPNET,
        LGL_BARCODE_TYPE_ONECODE,
        LGL_BARCODE_TYPE_CODE39,
        LGL_BARCODE_TYPE_CODE39_EXT,

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
