/*  outline.vala
 *
 *  Copyright (C) 2011  Jim Evins <evins@snaught.com>
 *
 *  This file is part of gLabels.
 *
 *  gLabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gLabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gLabels.  If not, see <http://www.gnu.org/licenses/>.
 */


using GLib;

namespace glabels
{

	private const double OUTLINE_WIDTH_PIXELS = 2;
	private const double SELECTION_SLOP_PIXELS = 4;

	private const Color OUTLINE_COLOR = { 0.0,  0.0,   0.0,  0.8 };


	public class Outline
	{
		public weak LabelObject owner { get; protected set; }


		public Outline( LabelObject owner )
		{
			this.owner = owner;
		}


		public void draw( Cairo.Context cr )
		{
			double dashes[2] = { 2, 2 };

			cr.save();

			cr.rectangle( 0, 0, owner.w, owner.h );

			double scale_x = 1.0;
			double scale_y = 1.0;
			cr.device_to_user_distance( ref scale_x, ref scale_y );
			cr.scale( scale_x, scale_y );

			cr.set_dash( dashes, 0 );
			cr.set_line_width( OUTLINE_WIDTH_PIXELS );
			cr.set_source_rgba( OUTLINE_COLOR.r, OUTLINE_COLOR.g, OUTLINE_COLOR.b,
			                    OUTLINE_COLOR.a );
			cr.stroke();

			cr.restore();
		}


		public bool in_stroke( Cairo.Context cr, double x, double y )
		{
			cr.rectangle( 0, 0, owner.w, owner.h );

			double scale_x = 1.0;
			double scale_y = 1.0;
			cr.device_to_user_distance( ref scale_x, ref scale_y );

			cr.set_line_width( 2*SELECTION_SLOP_PIXELS*scale_x );

			return cr.in_stroke( x, y );
		}


	}


}

