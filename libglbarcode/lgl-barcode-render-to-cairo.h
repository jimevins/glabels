/*
 *  lgl-barcode-render-to-cairo.h
 *  Copyright (C) 2010  Jim Evins <evins@snaught.com>.
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

#ifndef __LGL_RENDER_TO_CAIRO_H__
#define __LGL_RENDER_TO_CAIRO_H__

#include "lgl-barcode.h"
#include <cairo.h>

G_BEGIN_DECLS

void  lgl_barcode_render_to_cairo      (const lglBarcode *bc,
                                        cairo_t          *cr);

void  lgl_barcode_render_to_cairo_path (const lglBarcode *bc,
                                        cairo_t          *cr);

G_END_DECLS

#endif /* __LGL_RENDER_TO_CAIRO_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
