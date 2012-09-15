/*  barcode_code39.vala
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
	 * Code39 Barcode
	 */
	public class BarcodeCode39 : Barcode
	{

		/*
		 * Constants
		 */
		private const double MIN_X       = ( 0.01 *  PTS_PER_INCH );
		private const double N           = 2.5;
		private const double MIN_I       = MIN_X;
		private const double MIN_HEIGHT  = ( 0.25 *  PTS_PER_INCH );
		private const double MIN_QUIET   = ( 0.10 *  PTS_PER_INCH );

		private const double MIN_TEXT_AREA_HEIGHT = 14.0;
		private const double MIN_TEXT_SIZE        = 10.0;


		/* Code 39 alphabet. Position indicates value. */
		private const string alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%";

		/* Code 39 symbols. Position must match position in alphabet. */
		private const string[] symbols = {
			/*        BsBsBsBsB */
			/* 0 */  "NnNwWnWnN",
			/* 1 */  "WnNwNnNnW",
			/* 2 */  "NnWwNnNnW",
			/* 3 */  "WnWwNnNnN",
			/* 4 */  "NnNwWnNnW",
			/* 5 */  "WnNwWnNnN",
			/* 6 */  "NnWwWnNnN",
			/* 7 */  "NnNwNnWnW",
			/* 8 */  "WnNwNnWnN",
			/* 9 */  "NnWwNnWnN",
			/* A */  "WnNnNwNnW",
			/* B */  "NnWnNwNnW",
			/* C */  "WnWnNwNnN",
			/* D */  "NnNnWwNnW",
			/* E */  "WnNnWwNnN",
			/* F */  "NnWnWwNnN",
			/* G */  "NnNnNwWnW",
			/* H */  "WnNnNwWnN",
			/* I */  "NnWnNwWnN",
			/* J */  "NnNnWwWnN",
			/* K */  "WnNnNnNwW",
			/* L */  "NnWnNnNwW",
			/* M */  "WnWnNnNwN",
			/* N */  "NnNnWnNwW",
			/* O */  "WnNnWnNwN",
			/* P */  "NnWnWnNwN",
			/* Q */  "NnNnNnWwW",
			/* R */  "WnNnNnWwN",
			/* S */  "NnWnNnWwN",
			/* T */  "NnNnWnWwN",
			/* U */  "WwNnNnNnW",
			/* V */  "NwWnNnNnW",
			/* W */  "WwWnNnNnN",
			/* X */  "NwNnWnNnW",
			/* Y */  "WwNnWnNnN",
			/* Z */  "NwWnWnNnN",
			/* - */  "NwNnNnWnW",
			/* . */  "WwNnNnWnN",
			/*   */  "NwWnNnWnN",
			/* $ */  "NwNwNwNnN",
			/* / */  "NwNwNnNwN",
			/* + */  "NwNnNwNwN",
			/* % */  "NnNwNwNwN"
		};

		private const string frame_symbol = "NwNnWnWnN";


		/**
		 * Code39 data validation method
		 */
		protected override bool validate( string data )
		{
			for ( int i = 0; i < data.length; i++ )
			{
				char c = data[i].toupper();

				if ( !alphabet.contains( c.to_string() ) )
				{
					return false;
				}
			}

			return true;
		}


		/**
		 * Code39 data encoding method
		 */
		protected override string encode( string canon_data )
		{
			StringBuilder code = new StringBuilder();

			/* Left frame symbol */
			code.append( frame_symbol );
			code.append( "i" );

			int sum = 0;
			for ( int i=0; i < canon_data.length; i++ )
			{
				int c_value = alphabet.index_of_char( canon_data[i].toupper() );

				code.append( symbols[c_value] );
				code.append( "i" );

				sum += c_value;
			}

			if ( checksum_flag )
			{
				code.append( symbols[sum % 43] );
				code.append( "i" );
			}

			/* Right frame bar */
			code.append( frame_symbol );

			return code.str;
		}


		/**
		 * Code39 vectorize method
		 */
		protected override void vectorize( string coded_data, string data, string text )
		{
			/* determine width and establish horizontal scale */
			double min_l;
			if (!checksum_flag)
			{
				min_l = (data.length + 2)*(3*N + 6)*MIN_X + (data.length + 1)*MIN_I;
			}
			else
			{
				min_l = (data.length + 3)*(3*N + 6)*MIN_X + (data.length + 2)*MIN_I;
			}
        
			double scale;
			if ( w == 0 )
			{
				scale = 1.0;
			}
			else
			{
				scale = w / (min_l + 2*MIN_QUIET);

				if ( scale < 1.0 )
				{
					scale = 1.0;
				}
			}
			double width = min_l * scale;

			/* determine text parameters */
			double h_text_area = scale * MIN_TEXT_AREA_HEIGHT;
			double text_size   = scale * MIN_TEXT_SIZE;

			/* determine height of barcode */
			double height = text_flag ? h - h_text_area : h;
			height = double.max( height, double.max( 0.15*width, MIN_HEIGHT ) );

			/* determine horizontal quiet zone */
			double x_quiet = double.max( (10 * scale * MIN_X), MIN_QUIET );

			/* Now traverse the code string and draw each bar */
			double x1 = x_quiet;
			for ( int i=0; i < coded_data.length; i++ )
			{
				
				switch ( coded_data[i] )
				{

				case 'i':
					/* Inter-character gap */
					x1 += scale * MIN_I;
					break;

				case 'N':
					/* Narrow bar */
					add_box( x1, 0.0, (scale*MIN_X - INK_BLEED), height );
					x1 += scale*MIN_X;
					break;

				case 'W':
					/* Wide bar */
					add_box( x1, 0.0, (scale*N*MIN_X - INK_BLEED), height );
					x1 += scale * N * MIN_X;
					break;

				case 'n':
					/* Narrow space */
					x1 += scale * MIN_X;
					break;

				case 'w':
					/* Wide space */
					x1 += scale * N * MIN_X;
					break;

				default:
					assert_not_reached();
				}
			}

			if ( text_flag )
			{
				string starred_text = "*%s*".printf( text );
				add_string( x_quiet + width/2, height + (h_text_area-text_size)/2, text_size, starred_text );
			}

			/* Overwrite requested size with actual size. */
			w = width + 2*x_quiet;
			h = text_flag ? height + h_text_area : height;
		}

	}

}
