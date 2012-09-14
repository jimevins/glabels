/*  barcode_upc.vala
 *
 *  Copyright (C) 2012  Jim Evins <evins@snaught.com>
 *
 *  This file is part of libglbarcode.
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
using glbarcode.Constants;

namespace glbarcode
{

	public class BarcodeUPCA : Barcode
	{

		private const int    QUIET_MODULES   = 9;

		private const double BASE_MODULE_SIZE      = ( 0.01 *  PTS_PER_INCH );
		private const double BASE_FONT_SIZE        = 7.2;
		private const double BASE_TEXT_AREA_HEIGHT = 11;


		private const string[] symbols = {
			/* left:  sBsB */
			/* right: BsBs */
			/* 0 */  "3211",
			/* 1 */  "2221",
			/* 2 */  "2122",
			/* 3 */  "1411",
			/* 4 */  "1132",
			/* 5 */  "1231",
			/* 6 */  "1114",
			/* 7 */  "1312",
			/* 8 */  "1213",
			/* 9 */  "3112"
		};

		private const string s_symbol = "111";   /* BsB */
		private const string e_symbol = "111";   /* BsB */
		private const string m_symbol = "11111"; /* sBsBs */


		private string check_digit;


		protected override bool validate( string data )
		{
			int n_digits = 0;

			for ( int i = 0; i < data.length; i++ )
			{
				if ( data[i].isdigit() )
				{
					n_digits++;
				}
				else if ( data[i] != ' ')
				{
					/* Only allow digits and spaces. */
					return false;
				}
			}

			if ( n_digits != 11 )
			{
				return false;
			}

			return true;
		}


		protected override string preprocess( string data )
		{
			StringBuilder stripped_data = new StringBuilder();

			for ( int i = 0; i < data.length; i++ )
			{
				if ( data[i].isdigit() )
				{
					stripped_data.append_c( data[i] );
				}
			}

			return stripped_data.str;
		}


		protected override string encode( string canon_data, bool checksum_flag )
		{
			int sum_odd  = 0;
			int sum_even = 0;

			StringBuilder code = new StringBuilder();

			/* Left frame symbol */
			code.append( s_symbol );

			/* Left 6 digits */
			for ( int i=0; i < 6; i++ )
			{
				int c_value = canon_data[i].digit_value();
				code.append( symbols[c_value] );

				if ( (i & 1) == 0 )
				{
					sum_odd += c_value;
				}
				else
				{
					sum_even += c_value;
				}
			}

			/* Middle frame symbol */
			code.append( m_symbol );

			/* Right 5 digits */
			for ( int i=6; i < 11; i++ )
			{
				int c_value = canon_data[i].digit_value();
				code.append( symbols[c_value] );

				if ( (i & 1) == 0 )
				{
					sum_odd += c_value;
				}
				else
				{
					sum_even += c_value;
				}
			}

			/* Check digit */
			int check = (3*sum_odd + sum_even) % 10;
			if ( check != 0 )
			{
				check = 10 - check;
			}
			code.append( symbols[check] );
			check_digit = check.to_string(); /* Save for display. */


			/* Right frame symbol */
			code.append( e_symbol );

			return code.str;
		}


		protected override void vectorize( string coded_data,
		                                   bool text_flag, bool checksum_flag,
		                                   double w, double h,
		                                   string data, string text )
		{
			/* determine width and establish horizontal scale */
			int n_modules = 7*(data.length+1) + 11;

			double scale;
			if ( w == 0 )
			{
				scale = 1.0;
			}
			else
			{
				scale = w / ((n_modules + 2*QUIET_MODULES) * BASE_MODULE_SIZE);

				if ( scale < 1.0 )
				{
					scale = 1.0;
				}
			}
			double width = (n_modules + 2*QUIET_MODULES) * scale * BASE_MODULE_SIZE;

			/* determine text parameters */
			double h_text_area = scale * BASE_TEXT_AREA_HEIGHT;
			double text_size1  = scale * BASE_FONT_SIZE;
			double text_size2  = 0.75*text_size1;
			double text_c1 = (QUIET_MODULES + 5 + n_modules/4) * scale * BASE_MODULE_SIZE;
			double text_c2 = (QUIET_MODULES - 4 + 3*n_modules/4) * scale * BASE_MODULE_SIZE;

			/* determine bar height */
			double h_bar1 = double.max( (h - h_text_area), width/2 );
			double h_bar2 = h_bar1 + h_text_area/2;

			/* determine horizontal quiet zone */
			double x_quiet = QUIET_MODULES * scale * BASE_MODULE_SIZE;

			/* now traverse the code string and draw each bar */
			double x1 = x_quiet;
			for ( int i=0; i < coded_data.length; i+=2 )
			{
				double h_bar;

				if ( ( (i > 7) && (i < (coded_data.length/2-1)) ) ||
				     ( (i > (coded_data.length/2+1)) && (i < (coded_data.length-7)) ) )
				{
					h_bar = h_bar1;
				}
				else
				{
					h_bar = h_bar2;
				}

				/* Bar */
				int bar_w = coded_data[i].digit_value();

				add_box( x1, 0.0, (bar_w*scale*BASE_MODULE_SIZE - INK_BLEED), h_bar );
				x1 += bar_w * scale * BASE_MODULE_SIZE;

				/* Space */
				int space_w = coded_data[i+1].digit_value();

				x1 += space_w * scale * BASE_MODULE_SIZE;
			}

			/* draw text */
			add_string( x_quiet/2,       h_bar2 - text_size2/2, text_size2, text[0:1] );
			add_string( text_c1,         h_bar2 - text_size1/2, text_size1, text[1:6] );
			add_string( text_c2,         h_bar2 - text_size1/2, text_size1, text[6:11] );
			add_string( width-x_quiet/2, h_bar2 - text_size2/2, text_size2, check_digit );

			this.w = width;
			this.h = h_bar1 + h_text_area;
		}


	}

}
