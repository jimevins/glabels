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

	/**
	 * 5 Digit POSTNET Barcode (ZIP only)
	 */
	public class BarcodePostnet5 : BarcodePostnet
	{
		protected override bool validate_digits( int n_digits )
		{
			return (n_digits == 5);
		}
	}


	/**
	 * 9 Digit POSTNET Barcode (ZIP+4)
	 */
	public class BarcodePostnet9 : BarcodePostnet
	{
		protected override bool validate_digits( int n_digits )
		{
			return (n_digits == 9);
		}
	}


	/**
	 * 11 Digit POSTNET Barcode (ZIP + 4 + Delivery Point)
	 */
	public class BarcodePostnet11 : BarcodePostnet
	{
		protected override bool validate_digits( int n_digits )
		{
			return (n_digits == 11);
		}
	}


	/**
	 * 8 Digit CEPNET Barcode (Brazillian, based on POSTNET)
	 */
	public class BarcodeCepnet : BarcodePostnet
	{
		protected override bool validate_digits( int n_digits )
		{
			return (n_digits == 8);
		}
	}


	/**
	 * POSTNET Barcode (Any USPS Length)
	 */
	public class BarcodePostnet : Barcode
	{

		/*
		 * Constants
		 */
		private const double POSTNET_BAR_WIDTH      = ( 0.02    * PTS_PER_INCH );
		private const double POSTNET_FULLBAR_HEIGHT = ( 0.125   * PTS_PER_INCH );
		private const double POSTNET_HALFBAR_HEIGHT = ( 0.05    * PTS_PER_INCH );
		private const double POSTNET_BAR_PITCH      = ( 0.04545 * PTS_PER_INCH );
		private const double POSTNET_HORIZ_MARGIN   = ( 0.125   * PTS_PER_INCH );
		private const double POSTNET_VERT_MARGIN    = ( 0.04    * PTS_PER_INCH );


		/*
		 * Encoding symbology
		 */
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

		
		/**
		 * POSTNET general validate length method
		 */
		protected virtual bool validate_digits( int n_digits )
		{
			/* Accept any valid POSTNET length. */
			return (n_digits == 5) || (n_digits == 9) || (n_digits ==11);
		}


		/**
		 * POSTNET data validation method
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
				else if ( (data[i] != '-') && (data[i] != ' ') )
				{
					/* Only allow digits, dashes, and spaces. */
					return false;
				}
			}

			return validate_digits( n_digits );
		}


		/**
		 * POSTNET data encoding method
		 */
		protected override string encode( string data )
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


		/**
		 * POSTNET vectorization method
		 */
		protected override void vectorize( string coded_data, string data, string text )
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

			/* Overwrite requested size with actual size. */
			w = x + POSTNET_HORIZ_MARGIN;
			h = POSTNET_FULLBAR_HEIGHT + 2 * POSTNET_VERT_MARGIN;
		}


	}

}
