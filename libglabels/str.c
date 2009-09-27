/*
 *  str.c
 *  Copyright (C) 2007-2009  Jim Evins <evins@snaught.com>.
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

#include "str.h"


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
 * lgl_str_utf8_casecmp:
 * @s1: string to compare with s2.
 * @s2: string to compare with s1.
 *
 * Compare two UTF-8 strings, ignoring the case of characters.
 *
 * This function should be used only on strings that are known to be encoded
 * in UTF-8 or a compatible UTF-8 subset.
 *
 * Returns: 0 if the strings match, a negative value if s1 < s2,
 *          or a positive value if s1 > s2.
 *
 */
gint
lgl_str_utf8_casecmp (const gchar *s1,
                      const gchar *s2)
{
        gchar *folded_s1;
        gchar *folded_s2;
        gint   result;

        folded_s1 = g_utf8_casefold (s1, -1);
        folded_s2 = g_utf8_casefold (s2, -1);

        result = g_utf8_collate (folded_s1, folded_s2);

        g_free (folded_s1);
        g_free (folded_s2);

        return result;
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
