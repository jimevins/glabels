/*  str_util.vala
 *
 *  Copyright (C) 2011  Jim Evins <evins@snaught.com>
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


using GLib;
using Math;

namespace libglabels
{

	namespace StrUtil
	{

		public string format_fraction( double x )
		{
			const double   FRAC_EPSILON = 0.00005;
			const double[] denom        = {  1.0,  2.0, 3.0,  4.0,  8.0,  16.0,  32.0,  0.0 };
			string[] denom_string = { "1", "₂", "₃", "₄", "₈", "₁₆", "₃₂", null };
			string[] num_string   = {  "⁰",  "¹",  "²",  "³",  "⁴",  "⁵",  "⁶",  "⁷",  "⁸",  "⁹",
			                           "¹⁰", "¹¹", "¹²", "¹³", "¹⁴", "¹⁵", "¹⁶", "¹⁷", "¹⁸", "¹⁹",
			                           "²⁰", "²¹", "²²", "²³", "²⁴", "²⁵", "²⁶", "²⁷", "²⁸", "²⁹",
			                           "³⁰", "³¹" };
			int i;
			double product, remainder;
			int n, d;

			for ( i=0; denom[i] != 0.0; i++ )
			{
				product = x * denom[i];
				remainder = fabs(product - ((int)(product+0.5)));
				if ( remainder < FRAC_EPSILON ) break;
			}

			if ( denom[i] == 0.0 )
			{
				/* None of our denominators work. */
				return "%.5g".printf( x );
			}
			if ( denom[i] == 1.0 )
			{
				/* Simple integer. */
				return "%.0f".printf( x );
			}
			n = (int)( x * denom[i] + 0.5 );
			d = (int)denom[i];
			if ( n > d )
			{
				return "%d%s/%s".printf( (n/d), num_string[n%d], denom_string[i] );
			}
			else
			{
				return "%s/%s".printf( num_string[n%d], denom_string[i] );
			}
		}


		/**
		 * Compare part names
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
		public int compare_part_names( string s1,
		                               string s2 )
		{
			if ( s1 == s2 ) return 0;
			if ( s1 == "" ) return -1;
			if ( s2 == "" ) return 1;

			string folded_s1 = s1.casefold( -1 );
			string folded_s2 = s2.casefold( -1 );

			int i1 = 0;
			int i2 = 0;
			int result = 0;
			bool done = false;

			while ( (result == 0) && !done )
			{
				string chunk1, chunk2;
				bool   isnum1, isnum2;

				if ( folded_s1.get_char( i1 ).isdigit() )
				{
					chunk1 = span_digits( folded_s1, ref i1 );
					isnum1 = true;
				}
				else
				{
					chunk1 = span_non_digits( folded_s1, ref i1 );
					isnum1 = false;
				}
                
				if ( folded_s2.get_char( i2 ).isdigit() )
				{
					chunk2 = span_digits( folded_s2, ref i2 );
					isnum2 = true;
				}
				else
				{
					chunk2 = span_non_digits( folded_s2, ref i2 );
					isnum2 = false;
				}

				if ( ( chunk1 == "" ) && ( chunk2 == "" ) )
				{
					/* Case 1: Both are empty. */
					done = true;
				}
				else if ( isnum1 && isnum2 )
				{
					/* Case 2: They both contain numbers */
					uint64 n1 = uint64.parse( chunk1 );
					uint64 n2 = uint64.parse( chunk2 );

					if ( n1 < n2 ) result = -1;
					else if ( n1 > n2 ) result =  1;
				}
				else
				{
					/* Case 3: One or both do not contain numbers */
					if ( chunk1 < chunk2 ) result = -1;
					else if( chunk1 > chunk2 ) result = 1;
				}

			}

			return result;
		}


		private string span_digits( string s, ref int i )
		{
			StringBuilder chunk = new StringBuilder();

			bool not_end;
			unichar c;
			int j;
			for ( j = i, not_end = s.get_next_char( ref j, out c );
			      not_end && c.isdigit();
			      not_end = s.get_next_char( ref j, out c ) )
			{
				chunk.append_unichar( c );
				i = j; /* only advance i, if character is used. */
			}

			return chunk.str;
		}


		private string span_non_digits( string s, ref int i )
		{
			StringBuilder chunk = new StringBuilder();

			bool not_end;
			unichar c;
			int j;
			for ( j = i, not_end = s.get_next_char( ref j, out c );
			      not_end && !c.isdigit();
			      not_end = s.get_next_char( ref j, out c ) )
			{
				chunk.append_unichar( c );
				i = j; /* only advance i, if character is used. */
			}

			return chunk.str;
		}


	}

}
