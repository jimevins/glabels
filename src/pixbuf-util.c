/*
 *  pixbuf-util.c
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

#include <config.h>

#include "pixbuf-util.h"

#include "color.h"

#include "debug.h"


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/


/*========================================================*/
/* Private types.                                         */
/*========================================================*/


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/


/****************************************************************************/
/* Create shadow version of given pixbuf.                                   */
/****************************************************************************/
GdkPixbuf *
gl_pixbuf_util_create_shadow_pixbuf (const GdkPixbuf *pixbuf,
                                     guint            shadow_color,
                                     gdouble          shadow_opacity)
{
        gint             bits_per_sample, channels;
        gboolean         src_has_alpha;
        gint             width, height, src_rowstride, dest_rowstride;
        GdkPixbuf       *dest_pixbuf;
        guchar          *buf_src, *buf_dest;
        guchar          *p_src, *p_dest;
        gint             ix, iy;
        guchar           shadow_r, shadow_g, shadow_b;

        g_return_val_if_fail (pixbuf && GDK_IS_PIXBUF (pixbuf), NULL);

        shadow_r = GL_COLOR_F_RED   (shadow_color) * 255.0;
        shadow_g = GL_COLOR_F_GREEN (shadow_color) * 255.0;
        shadow_b = GL_COLOR_F_BLUE  (shadow_color) * 255.0;

        /* extract pixels and parameters from source pixbuf. */
        buf_src         = gdk_pixbuf_get_pixels (pixbuf);
        bits_per_sample = gdk_pixbuf_get_bits_per_sample (pixbuf);
        channels        = gdk_pixbuf_get_n_channels (pixbuf);
        src_has_alpha   = gdk_pixbuf_get_has_alpha (pixbuf);
        width           = gdk_pixbuf_get_width (pixbuf);
        height          = gdk_pixbuf_get_height (pixbuf);
        src_rowstride   = gdk_pixbuf_get_rowstride (pixbuf);

        /* validate assumptions about source pixbuf. */
        g_return_val_if_fail (buf_src, NULL);
        g_return_val_if_fail (bits_per_sample == 8, NULL);
        g_return_val_if_fail ((channels >= 3) && (channels <= 4), NULL);
        g_return_val_if_fail (width > 0, NULL);
        g_return_val_if_fail (height > 0, NULL);
        g_return_val_if_fail (src_rowstride > 0, NULL);

        /* Allocate a destination pixbuf */
        dest_pixbuf    = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, bits_per_sample, width, height);
        dest_rowstride = gdk_pixbuf_get_rowstride (dest_pixbuf);
        buf_dest       = gdk_pixbuf_get_pixels (dest_pixbuf);
        if (!buf_dest) {
                return NULL;
        }

        /* Process pixels: set rgb components and composite alpha with shadow_opacity. */
        p_src  = buf_src;
        p_dest = buf_dest;
        for ( iy=0; iy < height; iy++ )
        {
        
                p_src  = buf_src + iy*src_rowstride;
                p_dest = buf_dest + iy*dest_rowstride;

                for ( ix=0; ix < width; ix++ )
                {

                        p_src += 3; /* skip RGB */

                        *p_dest++ = shadow_r;
                        *p_dest++ = shadow_g;
                        *p_dest++ = shadow_b;

                        if ( src_has_alpha )
                        {
                                *p_dest++ = *p_src++ * shadow_opacity;
                        }
                        else
                        {
                                *p_dest++ = shadow_opacity * 255.0;
                        }


                }

        }

        return dest_pixbuf;
}




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
