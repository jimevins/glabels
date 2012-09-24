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

	/**
	 * Base class for all barcode types
	 */
	public abstract class Barcode : Object
	{

		/*
		 * Construction parameters
		 */
		public string? data          { get; construct; }
		public bool    text_flag     { get; construct; }
		public bool    checksum_flag { get; construct; }
		public double  w             { get; construct set; }
		public double  h             { get; construct set; }

		/*
		 * Status parameters
		 */
		public bool    is_empty      { get; protected set; }
		public bool    is_data_valid { get; protected set; }

		/*
		 * List of vectorized drawing primitives
		 */
		private List<Shape>   shapes;


		/**
		 * Validate data method.
		 */
		protected abstract bool   validate( string data );


		/**
		 * Pre-process data method.
		 */
		protected virtual string preprocess( string data )
		{
			return data;
		}


		/**
		 * Encode data method.
		 */
		protected abstract string encode( string canon_data );


		/**
		 * Vectorize encoded data method.
		 */
		protected abstract void   vectorize( string coded_data, string cooked_data, string display_text );


		/**
		 * Prepare data for display as text part of barcode.
		 */
		protected virtual string prepare_text( string data )
		{
			return data;
		}


		/**
		 * Construction.
		 */
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

					string cooked_data  = preprocess( data );
					string coded_data   = encode( cooked_data );

					string display_text = prepare_text( data );

					vectorize( coded_data, cooked_data, display_text );
				}
				else
				{
					is_data_valid = false;
				}
			}
		}


		/**
		 * Add box.  Used by vectorize method.
		 */
		protected void add_box( double x, double y, double w, double h )
		{
			ShapeBox box = new ShapeBox( x, y, w, h );
			shapes.prepend( box );
		}


		/**
		 * Add string.  Used by vectorize method.
		 */
		protected void add_string(  double x, double y, double fsize, string s )
		{
			ShapeText text = new ShapeText( x, y, fsize, s );
			shapes.prepend( text );
		}


		/**
		 * Add ring.  Used by vectorize method.
		 */
		protected void add_ring(    double x, double y, double r, double line_width )
		{
			ShapeRing ring = new ShapeRing( x, y, r, line_width );
			shapes.prepend( ring );
		}


		/**
		 * Add hexagon.  Used by vectorize method.
		 */
		protected void add_hexagon( double x, double y, double h )
		{
			ShapeHexagon hexagon = new ShapeHexagon( x, y, h );
			shapes.prepend( hexagon );
		}


		/**
		 * Render barcode using supplied renderer.
		 */
		public void render( Renderer renderer )
		{
			renderer.render( w, h, shapes );
		}

	}

}
