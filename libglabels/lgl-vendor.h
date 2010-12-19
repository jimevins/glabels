/*
 *  lgl-vendor.h
 *  Copyright (C) 2003-2010  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of libglabels.
 *
 *  libglabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libglabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libglabels.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __LGL_VENDOR_H__
#define __LGL_VENDOR_H__

#include <glib.h>

G_BEGIN_DECLS


/*
 *   lglVendor structure
 */
typedef struct _lglVendor lglVendor;

struct _lglVendor {
	gchar               *name;           /* Vendor name */
	gchar               *url;            /* Vendor URL */
};


/*
 * Vendor construction
 */
lglVendor           *lgl_vendor_new                (gchar           *name);

lglVendor           *lgl_vendor_dup                (const lglVendor *orig);

void                 lgl_vendor_free               (lglVendor       *vendor);


G_END_DECLS

#endif /* __LGL_VENDOR_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
