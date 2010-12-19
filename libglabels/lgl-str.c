/*
 *  lgl-str.c
 *  Copyright (C) 2007-2010  Jim Evins <evins@snaught.com>.
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

#include "lgl-str.h"

#include <string.h>
#include <math.h>

#define FRAC_EPSILON 0.00005


/*===========================================*/
/* Private types                             */
/*===========================================*/


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static gchar *span_digits (gchar **p);
static gchar *span_non_digits (gchar **p);

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


/**
 * lgl_str_part_name_cmp:
 * @s1: string to compare with s2.
 * @s2: string to compare with s1.
 *
 * Compare two UTF-8 strings representing part names or numbers.  This function
 * uses a natural sort order:
 *
 *  - Ignores case.
 *
 *  - Strings are divided into chunks (numeric and non-numeric)
 *
 *  - Non-numeric chunks are compared character by character
 *
 *  - Numerical chunks are compared numerically, so that "20" precedes "100".
 *
 *  - Comparison of chunks is performed left to right until the first difference
 *    is encountered or all chunks evaluate as equal.
 *
 * This function should be used only on strings that are known to be encoded
 * in UTF-8 or a compatible UTF-8 subset.
 *
 * Numeric chunks are converted to 64 bit unsigned integers for comparison,
 * so the behaviour may be unpredictable for numeric chunks that exceed
 * 18446744073709551615.
 *
 * Returns: 0 if the strings match, a negative value if s1 < s2,
 *          or a positive value if s1 > s2.
 *
 */
gint
lgl_str_part_name_cmp (const gchar *s1,
                       const gchar *s2)
{
        gchar *folded_s1, *p1, *chunk1;
        gchar *folded_s2, *p2, *chunk2;
        gboolean isnum1, isnum2;
        guint64 n1, n2;
        gboolean done;
        gint   result;

	if ( s1 == s2 ) return 0;
	if (s1 == NULL) return -1;
	if (s2 == NULL) return 1;

        folded_s1 = g_utf8_casefold (s1, -1);
        folded_s2 = g_utf8_casefold (s2, -1);

        result = 0;
        done = FALSE;
        p1 = folded_s1;
        p2 = folded_s2;
        while ( (result == 0) && !done )
        {

                if ( g_ascii_isdigit (*p1) )
                {
                        chunk1 = span_digits (&p1);
                        isnum1 = TRUE;
                }
                else
                {
                        chunk1 = span_non_digits (&p1);
                        isnum1 = FALSE;
                }
                
                if ( g_ascii_isdigit (*p2) )
                {
                        chunk2 = span_digits (&p2);
                        isnum2 = TRUE;
                }
                else
                {
                        chunk2 = span_non_digits (&p2);
                        isnum2 = FALSE;
                }

                if ( (strlen(chunk1) == 0) && (strlen(chunk2) == 0) )
                {
                        /* Case 1: Both are empty. */
                        done = TRUE;
                }
                else if ( isnum1 && isnum2 )
                {
                        /* Case 2: They both contain numbers */
                        n1 = g_ascii_strtoull (chunk1, NULL, 10);
                        n2 = g_ascii_strtoull (chunk2, NULL, 10);

                        if ( n1 < n2 ) result = -1;
                        if ( n1 > n2 ) result =  1;
                }
                else
                {
                        /* Case 3: One or both do not contain numbers */
                        result = g_utf8_collate (chunk1, chunk2);
                }

                g_free (chunk1);
                g_free (chunk2);
        }

        g_free (folded_s1);
        g_free (folded_s2);

        return result;
}


static gchar *
span_digits (gchar **p)
{
        gchar *chunk = g_new0 (gchar, strlen (*p) + 1);
        gint i;

        for ( i = 0; **p && g_ascii_isdigit (**p); i++, *p = g_utf8_next_char(*p) )
        {
                chunk[i] = **p;
        }

        return chunk;
}


static gchar *
span_non_digits (gchar **p)
{
        gchar *chunk = g_new0 (gchar, strlen (*p) + 1);
        gint i;

        for ( i = 0; **p && !g_ascii_isdigit (**p); i++, *p = g_utf8_next_char(*p) )
        {
                chunk[i] = **p;
        }

        return chunk;
}


/**
 * lgl_str_format_fraction:
 * @x: Floating point number to convert to fractional notation
 *
 * Create fractional representation of number, if possible.  Uses UTF-8 superscripts and
 * subscripts for numerator and denominator values respecively.
 *
 * Returns: UTF-8 string containing fractional representation of x.
 */
gchar *
lgl_str_format_fraction (gdouble x)
{
	static gdouble denom[]        = {  1.,  2., 3.,  4.,  8.,  16.,  32.,  0. };
        static gchar  *denom_string[] = { "1", "₂", "₃", "₄", "₈", "₁₆", "₃₂", NULL };
        static gchar  *num_string[]   = {  "⁰",  "¹",  "²",  "³",  "⁴",  "⁵",  "⁶",  "⁷",  "⁸",  "⁹",
                                          "¹⁰", "¹¹", "¹²", "¹³", "¹⁴", "¹⁵", "¹⁶", "¹⁷", "¹⁸", "¹⁹",
                                          "²⁰", "²¹", "²²", "²³", "²⁴", "²⁵", "²⁶", "²⁷", "²⁸", "²⁹",
                                          "³⁰", "³¹" };
	gint i;
	gdouble product, remainder;
	gint n, d;

	for ( i=0; denom[i] != 0.0; i++ ) {
		product = x * denom[i];
		remainder = fabs(product - ((gint)(product+0.5)));
		if ( remainder < FRAC_EPSILON ) break;
	}

	if ( denom[i] == 0.0 ) {
		/* None of our denominators work. */
		return g_strdup_printf ("%.5g", x);
	}
	if ( denom[i] == 1.0 ) {
		/* Simple integer. */
		return g_strdup_printf ("%.0f", x);
	}
	n = (gint)( x * denom[i] + 0.5 );
	d = (gint)denom[i];
	if ( n > d ) {
		return g_strdup_printf ("%d%s/%s", (n/d), num_string[n%d], denom_string[i]);
	} else {
		return g_strdup_printf ("%s/%s", num_string[n%d], denom_string[i]);
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
