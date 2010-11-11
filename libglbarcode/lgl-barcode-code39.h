/*
 *  lgl-barcode-code39.h
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

#ifndef __LGL_BARCODE_CODE39_H__
#define __LGL_BARCODE_CODE39_H__

#include "lgl-barcode.h"
#include "lgl-barcode-type.h"

G_BEGIN_DECLS

lglBarcode *lgl_barcode_code39_new (lglBarcodeType  type,
                                    gboolean        text_flag,
                                    gboolean        checksum_flag,
                                    gdouble         w,
                                    gdouble         h,
                                    const gchar    *data);

G_END_DECLS

#endif /* __LGL_BARCODE_CODE39_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
