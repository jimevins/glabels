/*  barcode.vala
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

namespace glbarcode
{

	public abstract class Barcode : Object
	{

		public string? data          { get; construct; }
		public bool    text_flag     { get; construct; }
		public bool    checksum_flag { get; construct; }
		public double  w             { get; construct set; }
		public double  h             { get; construct set; }

		public bool    is_empty      { get; private set; }
		public bool    is_data_valid { get; private set; }

		private List<Shape>   shapes;


		protected abstract bool   validate( string data );
		protected abstract string encode( string canon_data, bool checksum_flag );
		protected abstract void   vectorize( string coded_data,
		                                     bool text_flag, bool checksum_flag,
		                                     double w, double h,
		                                     string data, string text );


		protected virtual string to_text( string data )
		{
			return data;
		}


		protected virtual string preprocess( string data )
		{
			return data;
		}


		construct
		{
			if ( (data == null) || (data == "") )
			{
				is_empty      = true;
				is_data_valid = false;
			}
			else
			{
				is_empty = false;

				if ( validate( data ) )
				{
					is_data_valid = true;

					string display_text = to_text( data );
					string cooked_data  = preprocess( data );
					string coded_data   = encode( cooked_data, checksum_flag );

					vectorize( coded_data, text_flag, checksum_flag, w, h, cooked_data, display_text );
				}
				else
				{
					is_data_valid = false;
				}
			}
		}


		protected void add_box( double x, double y, double w, double h )
		{
			ShapeBox box = new ShapeBox( x, y, w, h );
			shapes.prepend( box );
		}


		protected void add_string(  double x, double y, double fsize, string s )
		{
			ShapeText text = new ShapeText( x, y, fsize, s );
			shapes.prepend( text );
		}


		protected void add_ring(    double x, double y, double r, double line_width )
		{
			ShapeRing ring = new ShapeRing( x, y, r, line_width );
			shapes.prepend( ring );
		}


		protected void add_hexagon( double x, double y, double h )
		{
			ShapeHexagon hexagon = new ShapeHexagon( x, y, h );
			shapes.prepend( hexagon );
		}


		public void render( Renderer renderer )
		{
			renderer.render( w, h, shapes );
		}

	}

}
