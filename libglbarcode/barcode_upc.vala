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

	/**
	 * UPC-A Barcode
	 */
	public class BarcodeUPCA : BarcodeBaseUPC
	{
		static construct
		{
			end_bars_thresh  = 7;
			end_bars_modules = 11;
		}

		protected override bool validate_digits( int n_digits )
		{
			return (n_digits == 11);
		}

		protected override void vectorize_text( string text,
		                                        double size1, double size2,
		                                        double x1_left, double x1_right, double y1,
		                                        double x2_left, double x2_right, double y2 )
		{
			add_string( x2_left,  y2, size2, text[0:1] );
			add_string( x1_left,  y1, size1, text[1:6] );
			add_string( x1_right, y1, size1, text[6:11] );
			add_string( x2_right, y2, size2, text[11:12] );
		}
	}


	/**
	 * EAN-13 Barcode
	 */
	public class BarcodeEAN13 : BarcodeBaseUPC
	{
		static construct
		{
			end_bars_thresh  = 3;
			end_bars_modules = 3;
		}

		protected override bool validate_digits( int n_digits )
		{
			return (n_digits == 12);
		}

		protected override void vectorize_text( string text,
		                                        double size1, double size2,
		                                        double x1_left, double x1_right, double y1,
		                                        double x2_left, double x2_right, double y2 )
		{
			add_string( x2_left,  y2, size2, text[0:1] );
			add_string( x1_left,  y1, size1, text[1:7] );
			add_string( x1_right, y1, size1, text[7:13] );
		}
	}


	/**
	 * Base class for UPC-A and EAN-13
	 */
	public abstract class BarcodeBaseUPC : Barcode
	{

		/*
		 * Constants
		 */
		private const int    QUIET_MODULES   = 9;

		private const double BASE_MODULE_SIZE      = ( 0.01 *  PTS_PER_INCH );
		private const double BASE_FONT_SIZE        = 7;
		private const double BASE_TEXT_AREA_HEIGHT = 11;


		/*
		 * Symbology
		 */
		private string[,] symbols = {
			/*          Odd     Even  */
			/*    Left: sBsB    sBsB  */
			/*   Right: BsBs    ----  */
			/*                        */
			/* 0 */  { "3211", "1123" },
			/* 1 */  { "2221", "1222" },
			/* 2 */  { "2122", "2212" },
			/* 3 */  { "1411", "1141" },
			/* 4 */  { "1132", "2311" },
			/* 5 */  { "1231", "1321" },
			/* 6 */  { "1114", "4111" },
			/* 7 */  { "1312", "2131" },
			/* 8 */  { "1213", "3121" },
			/* 9 */  { "3112", "2113" }
		};

		private const string s_symbol = "111";   /* BsB */
		private const string e_symbol = "111";   /* BsB */
		private const string m_symbol = "11111"; /* sBsBs */


		/*
		 * Parity selection
		 */
		private enum Parity { ODD=0, EVEN=1 }

		private Parity[,] parity = {
			/*                Position 1,  Position 2,  Position 3,  Position 4,  Position 5,  Position 6
			/* 0 (UPC-A) */ { Parity.ODD,  Parity.ODD,  Parity.ODD,  Parity.ODD,  Parity.ODD,  Parity.ODD  },
			/* 1         */ { Parity.ODD,  Parity.ODD,  Parity.EVEN, Parity.ODD,  Parity.EVEN, Parity.EVEN },
			/* 2         */ { Parity.ODD,  Parity.ODD,  Parity.EVEN, Parity.EVEN, Parity.ODD,  Parity.EVEN },
			/* 3         */ { Parity.ODD,  Parity.ODD,  Parity.EVEN, Parity.EVEN, Parity.EVEN, Parity.ODD  },
			/* 4         */ { Parity.ODD,  Parity.EVEN, Parity.ODD,  Parity.ODD,  Parity.EVEN, Parity.EVEN },
			/* 5         */ { Parity.ODD,  Parity.EVEN, Parity.EVEN, Parity.ODD,  Parity.ODD,  Parity.EVEN },
			/* 6         */ { Parity.ODD,  Parity.EVEN, Parity.EVEN, Parity.EVEN, Parity.ODD,  Parity.ODD  },
			/* 7         */ { Parity.ODD,  Parity.EVEN, Parity.ODD,  Parity.EVEN, Parity.ODD,  Parity.EVEN },
			/* 8         */ { Parity.ODD,  Parity.EVEN, Parity.ODD,  Parity.EVEN, Parity.EVEN, Parity.ODD  },
			/* 9         */ { Parity.ODD,  Parity.EVEN, Parity.EVEN, Parity.ODD,  Parity.EVEN, Parity.ODD  }
		};


		private int first_digit_val;
		private int check_digit_val;

		/*
		 * Adaptation parameters and methods that are unique to each concrete class.
		 */
		protected static int end_bars_thresh;  /* Number of end bars and spaces that are long. */
		protected static int end_bars_modules; /* Number of modules consumed by long end bars. */

		protected abstract bool validate_digits( int n_digits );

		protected abstract void vectorize_text( string text,
		                                        double size1, double size2,
		                                        double x1_left, double x1_right, double y1,
		                                        double x2_left, double x2_right, double y2 );


		/*
		 * Validate data
		 */
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
					/* Only allow digits and spaces -- ignoring spaces. */
					return false;
				}
			}

			/* validate n_digits (call implementation from concrete class) */
			return validate_digits( n_digits );
		}


		/*
		 * Pre-process data before encoding
		 */
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

			if ( stripped_data.len == 11 )
			{
				/* UPC-A */
				first_digit_val = 0;
				return stripped_data.str;
			}
			else
			{
				/* EAN-13 */
				first_digit_val = stripped_data.str[0].digit_value();
				return stripped_data.str[1:stripped_data.len];
			}

		}


		/*
		 * Encode data
		 */
		protected override string encode( string canon_data )
		{
			int sum_odd  = 0;
			int sum_even = first_digit_val;

			StringBuilder code = new StringBuilder();

			/* Left frame symbol */
			code.append( s_symbol );

			/* Left 6 digits */
			for ( int i = 0; i < 6; i++ )
			{
				int c_value = canon_data[i].digit_value();
				code.append( symbols[c_value,parity[first_digit_val,i]] );

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
			for ( int i = 6; i < 11; i++ )
			{
				int c_value = canon_data[i].digit_value();
				code.append( symbols[c_value,Parity.ODD] );

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
			check_digit_val = (3*sum_odd + sum_even) % 10;
			if ( check_digit_val != 0 )
			{
				check_digit_val = 10 - check_digit_val;
			}
			code.append( symbols[check_digit_val,Parity.ODD] );

			/* Right frame symbol */
			code.append( e_symbol );

			/* Append a final zero length space to make the length of the encoded string even. */
			code.append( "0" );

			return code.str;
		}


		/*
		 * Process data for display as text
		 */
		protected override string prepare_text( string data )
		{
			return data.concat( check_digit_val.to_string(), null );
		}


		/*
		 * Vectorize
		 */
		protected override void vectorize( string coded_data, string data, string text )
		{
			/* determine width and establish horizontal scale */
			int n_modules     = 7*(data.length+1) + 11;

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
			double width       = scale * BASE_MODULE_SIZE * (n_modules + 2*QUIET_MODULES);
			double x_quiet     = scale * BASE_MODULE_SIZE * QUIET_MODULES;

			/* determine bar height */
			double h_text_area = scale * BASE_TEXT_AREA_HEIGHT;
			double h_bar1      = double.max( (h - h_text_area), width/2 );
			double h_bar2      = h_bar1 + h_text_area/2;

			/* determine text parameters */
			double text_size1    = scale * BASE_FONT_SIZE;
			double text_size2    = 0.75*text_size1;

			double text_x1_left  = scale * BASE_MODULE_SIZE * (QUIET_MODULES + end_bars_modules/2 + n_modules/4);
			double text_x1_right = scale * BASE_MODULE_SIZE * (QUIET_MODULES - end_bars_modules/2 + 3*n_modules/4);
			double text_x2_left  = scale * BASE_MODULE_SIZE * QUIET_MODULES/2;
			double text_x2_right = scale * BASE_MODULE_SIZE * (n_modules + 3*QUIET_MODULES/2);

			double text_y1       = h_bar2 - text_size1/2;
			double text_y2       = h_bar2 - text_size2/2;


			/* now traverse the code string and draw each bar */
			int n_bars_spaces = coded_data.length - 1; /* coded data has dummy "0" on end. */

			double x1 = x_quiet;
			for ( int i = 0; i < n_bars_spaces; i += 2 )
			{
				double h_bar;

				if ( ( (i > end_bars_thresh)     && (i < (n_bars_spaces/2-1))             ) ||
				     ( (i > (n_bars_spaces/2+1)) && (i < (n_bars_spaces-end_bars_thresh)) ) )
				{
					h_bar = h_bar1;
				}
				else
				{
					h_bar = h_bar2;
				}

				/* Bar */
				int w_bar = coded_data[i].digit_value();
				add_box( x1, 0.0, (w_bar*scale*BASE_MODULE_SIZE - INK_BLEED), h_bar );
				x1 += w_bar * scale * BASE_MODULE_SIZE;

				/* Space */
				int w_space = coded_data[i+1].digit_value();
				x1 += w_space * scale * BASE_MODULE_SIZE;
			}

			/* draw text (call implementation from concrete class) */
			vectorize_text( text,
			                text_size1, text_size2,
			                text_x1_left, text_x1_right, text_y1,
			                text_x2_left, text_x2_right, text_y2 );

			/* Overwrite requested size with actual size. */
			w = width;
			h = h_bar1 + h_text_area;
		}


	}

}
