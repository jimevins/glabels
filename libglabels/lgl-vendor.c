/*
 *  lgl-vendor.c
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

#include <config.h>

#include "lgl-vendor.h"

#include <glib/gi18n.h>
#include <glib.h>
#include <string.h>

#include "libglabels-private.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/


/*===========================================*/
/* Functions.                                */
/*===========================================*/

/**
 * lgl_vendor_new:
 * @name:     Localized name of vendor.
 *
 * Allocates and constructs a new #lglVendor structure.
 *
 * Returns: a pointer to a newly allocated #lglVendor structure.
 *
 */
lglVendor *
lgl_vendor_new (gchar             *name)
{
        lglVendor *vendor;

        vendor           = g_new0 (lglVendor,1);

        vendor->name     = g_strdup (name);

        return vendor;
}


/**
 * lgl_vendor_dup:
 * @orig:  #lglVendor structure to be duplicated.
 *
 * Duplicates an existing #lglVendor structure.
 *
 * Returns: a pointer to a newly allocated #lglVendor structure.
 *
 */
lglVendor *lgl_vendor_dup (const lglVendor *orig)
{
        lglVendor       *vendor;

        g_return_val_if_fail (orig, NULL);

        vendor = g_new0 (lglVendor,1);

        vendor->name  = g_strdup (orig->name);
        vendor->url   = g_strdup (orig->url);

        return vendor;
}


/**
 * lgl_vendor_free:
 * @vendor:  pointer to #lglVendor structure to be freed.
 *
 * Free all memory associated with an existing #lglVendor structure.
 *
 */
void lgl_vendor_free (lglVendor *vendor)
{

        if ( vendor != NULL )
        {
                g_free (vendor->name);
                vendor->name = NULL;

                g_free (vendor->url);
                vendor->url = NULL;

                g_free (vendor);
        }

}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
