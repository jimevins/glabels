/*  iec16022.vala
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


#if HAVE_LIBIEC16022

using GLib;
using glbarcode.Constants;

namespace glabels
{

	namespace BackendIec16022
	{

		/**
		 * Datamatrix Barcode
		 */
		public class Datamatrix : glbarcode.Barcode
		{

			private const double MIN_PIXEL_SIZE = 1.0;


			/**
			 * Datamatrix Barcode data validation method
			 */
			protected override bool validate( string data )
			{
				if ( (data.length == 0) || (data.length > iec16022.MAXBARCODE ) )
				{
					return false;
				}

				for ( int i = 0; i < data.length; i++ )
				{
					if ( (data[i] & 0x80) != 0 )
					{
						return false;
					}
				}

				return true;
			}


			/**
			 * Datamatrix Barcode encoding method
			 */
			protected override string encode( string data )
			{
				return ""; /* Actual encoding done in vectorize. */
			}


			/**
			 * Datamatrix Barcode vectorization method
			 */
			protected override void vectorize( string coded_data, string data, string text )
			{
				/*
				 * First encode using IEC16022 Barcode library.
				 */
				int i_width  = 0;
				int i_height = 0;

				char[] grid = iec16022.ecc200( ref i_width, ref i_height, null,
				                               data.length, data,
				                               null, null, null );

				/*
				 * Now do the actual vectorization.
				 */

				/* Treat requested size as a bounding box, scale to maintain aspect
				 * ratio while fitting it in this bounding box. */
				double aspect_ratio = (double)i_height / (double)i_width;
				if ( h > w*aspect_ratio )
				{
					h = w * aspect_ratio;
				}
				else
				{
					w = h / aspect_ratio;
				}

				/* Now determine pixel size. */
				double pixel_size = w / i_width;
				if ( pixel_size < MIN_PIXEL_SIZE )
				{
					pixel_size = MIN_PIXEL_SIZE;
				}

				/* Now traverse the code string and create a list of boxes */
				for ( int iy = i_height-1, i = 0; iy >= 0; iy-- )
				{

					for ( int ix = 0; ix < i_width; ix++, i++ )
					{

						if ( grid[i] != 0 )
						{
							add_box( ix*pixel_size, iy*pixel_size, pixel_size, pixel_size );
						}

					}

				}

				/* Fill in other info */
				w = i_width  * pixel_size;
				h = i_height * pixel_size;

			}


		}

	}

}

#endif
