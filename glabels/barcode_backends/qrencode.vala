/*  qrencode.vala
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


#if HAVE_LIBQRENCODE

using GLib;
using glbarcode.Constants;

namespace glabels
{

	namespace BackendQrencode
	{

		/**
		 * QRcode Barcode
		 */
		public class QRcode : glbarcode.Barcode
		{

			private const double MIN_PIXEL_SIZE = 1.0;


			/**
			 * QRcode Barcode data validation method
			 */
			protected override bool validate( string data )
			{
				if ( data.length == 0 )
				{
					return false;
				}

				return true;
			}


			/**
			 * QRcode Barcode encoding method
			 */
			protected override string encode( string data )
			{
				return ""; /* Actual encoding done in vectorize. */
			}


			/**
			 * QRcode Barcode vectorization method
			 */
			protected override void vectorize( string coded_data, string data, string text )
			{
				/*
				 * First encode using QRENCODE Barcode library.
				 */

				qrencode.QRcode qrcode = new qrencode.QRcode.encodeString( data, 0,
				                                                           qrencode.EcLevel.M,
				                                                           qrencode.Mode.B8, 1 );

				/*
				 * Now do the actual vectorization.
				 */

				/* Treat requested size as a bounding box, scale to maintain aspect
				 * ratio while fitting it in this bounding box. */
				if ( h > w )
				{
					h = w;
				}
				else
				{
					w = h;
				}

				/* Now determine pixel size. */
				double pixel_size = w / qrcode.width;
				if ( pixel_size < MIN_PIXEL_SIZE )
				{
					pixel_size = MIN_PIXEL_SIZE;
				}

				/* Now traverse the code string and create a list of boxes */
				for ( int iy = 0; iy < qrcode.width; iy++ )
				{

					for ( int ix = 0; ix < qrcode.width; ix++ )
					{

						/* Symbol data is represented as an array contains 
						 * width*width uchars. Each uchar represents a module 
						 * (dot). If the less significant bit of the uchar 
						 * is 1, the corresponding module is black. The other
						 * bits are meaningless for us. */
						if ( (qrcode.data[iy*qrcode.width + ix] & 1) != 0 )
						{
							add_box( ix*pixel_size, iy*pixel_size, pixel_size, pixel_size );
						}

					}

				}

				/* Fill in other info */
				w = h = qrcode.width  * pixel_size;

			}


		}

	}

}

#endif
