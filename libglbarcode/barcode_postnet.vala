/*  barcode_postnet.vala
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

	public class BarcodePostnet5 : BarcodePostnet
	{
		protected override bool validate_digits( int n_digits )
		{
			return (n_digits == 5);
		}
	}


	public class BarcodePostnet9 : BarcodePostnet
	{
		protected override bool validate_digits( int n_digits )
		{
			return (n_digits == 9);
		}
	}


	public class BarcodePostnet11 : BarcodePostnet
	{
		protected override bool validate_digits( int n_digits )
		{
			return (n_digits == 11);
		}
	}


	public class BarcodeCepnet : BarcodePostnet
	{
		protected override bool validate_digits( int n_digits )
		{
			return (n_digits == 8);
		}
	}


	public class BarcodePostnet : Barcode
	{

		private const double POSTNET_BAR_WIDTH      = ( 0.02    * PTS_PER_INCH );
		private const double POSTNET_FULLBAR_HEIGHT = ( 0.125   * PTS_PER_INCH );
		private const double POSTNET_HALFBAR_HEIGHT = ( 0.05    * PTS_PER_INCH );
		private const double POSTNET_BAR_PITCH      = ( 0.04545 * PTS_PER_INCH );
		private const double POSTNET_HORIZ_MARGIN   = ( 0.125   * PTS_PER_INCH );
		private const double POSTNET_VERT_MARGIN    = ( 0.04    * PTS_PER_INCH );

		private const string[] symbols = {
			/* 0 */ "11000",
			/* 1 */ "00011",
			/* 2 */ "00101",
			/* 3 */ "00110",
			/* 4 */ "01001",
			/* 5 */ "01010",
			/* 6 */ "01100",
			/* 7 */ "10001",
			/* 8 */ "10010",
			/* 9 */ "10100"
		};

		private const string frame_symbol = "1";


		protected virtual bool validate_digits( int n_digits )
		{
			/* Accept any valid POSTNET length for general Postnet type. */
			return (n_digits == 5) || (n_digits == 9) || (n_digits ==11);
		}


		protected override bool validate( string data )
		{

			int n_digits = 0;
			for ( int i = 0; i < data.length; i++ )
			{
				if ( data[i].isdigit() )
                {
                        n_digits++;
                }
                else if ( (data[i] != '-') && (data[i] != ' ') )
                {
                        /* Only allow digits, dashes, and spaces. */
                        return false;
                }
			}

			return validate_digits( n_digits );
		}


		protected override string encode( string data, bool checksum_flag )
		{
			StringBuilder code = new StringBuilder();

			/* Left frame bar */
			code.append( frame_symbol );

			/* process each digit, adding approptiate symbol */
			int sum = 0;
			for ( int i = 0; i < data.length; i++ )
			{
				if ( data[i].isdigit() )
				{
					/* Only translate the digits (0-9) */
					int d = data[i] - '0';
					code.append( symbols[d] );
					sum += d;
                }
			}

			/* Create mandatory correction character */
			code.append( symbols[ (10 - (sum % 10)) % 10 ] );

			/* Right frame bar */
			code.append( frame_symbol );

			return code.str;
		}


		protected override void vectorize( string coded_data,
		                                   bool text_flag, bool checksum_flag,
		                                   double w, double h,
		                                   string data, string text )
		{
			double x = POSTNET_HORIZ_MARGIN;
			for ( int i=0; i < coded_data.length; i++ )
			{
				double length, width;

				double y = POSTNET_VERT_MARGIN;

				switch ( coded_data[i] )
				{
				case '0':
					y += POSTNET_FULLBAR_HEIGHT - POSTNET_HALFBAR_HEIGHT;
					length = POSTNET_HALFBAR_HEIGHT;
					break;
				case '1':
					length = POSTNET_FULLBAR_HEIGHT;
					break;
				default:
					assert_not_reached();
				}
				width = POSTNET_BAR_WIDTH;

				add_box( x, y, width, length );

				x += POSTNET_BAR_PITCH;
			}

			this.w = x + POSTNET_HORIZ_MARGIN;
			this.h = POSTNET_FULLBAR_HEIGHT + 2 * POSTNET_VERT_MARGIN;
		}


	}

}
