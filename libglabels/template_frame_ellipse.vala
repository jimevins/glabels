/*  template_frame_ellipse.vala
 *
 *  Copyright (C) 2011  Jim Evins <evins@snaught.com>
 *
 *  This file is part of libglabels.
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
using Math;

namespace libglabels
{

	public class TemplateFrameEllipse : TemplateFrame
	{

		private const int ARC_FINE = 2;


		public double    w       { get; protected set; }
		public double    h       { get; protected set; }

		public double    waste   { get; protected set; }


		public TemplateFrameEllipse( string id,
		                             double w,
		                             double h,
		                             double waste )
		{
			this.id      = id;

			this.w       = w;
			this.h       = h;

			this.waste   = waste;
		}


		public override TemplateFrame dup()
		{
			TemplateFrameEllipse copy = new TemplateFrameEllipse( id, w, h, waste );

			foreach (TemplateLayout layout in layouts)
			{
				copy.add_layout( layout.dup() );
			}

			foreach (TemplateMarkup markup in markups)
			{
				copy.add_markup( markup.dup() );
			}

			return (TemplateFrame)copy;
		}


		public override void get_size( out double w,
		                               out double h )
		{
			w = this.w;
			h = this.h;
		}


		public override bool is_similar( TemplateFrame frame2 )
		{
			if ( frame2 is TemplateFrameEllipse )
			{
				TemplateFrameEllipse ellipse2 = (TemplateFrameEllipse) frame2;

				if ( (fabs( w - ellipse2.w) <= EPSILON) &&
					 (fabs( h - ellipse2.h) <= EPSILON) )
				{
					return true;
				}
			}
			return false;
		}


		public override string  get_size_description( Units units )
		{
			string description;

			string units_string    = units.name;
			double units_per_point = units.units_per_point;

			if ( units.id == "in" )
			{
				string xstr = StrUtil.format_fraction( w * units_per_point );
				string ystr = StrUtil.format_fraction( h * units_per_point );

				description = "%s × %s %s".printf( xstr, ystr, units_string );
			}
			else
			{
				description = "%.5g × %.5g %s".printf( w * units_per_point,
													   h * units_per_point,
													   units_string );
			}

			return description;
		}


		public override void cairo_path( Cairo.Context cr,
		                                 bool          waste_flag )
		{
			double w, h;

			w = this.w;
			h = this.h;

			double waste = 0.0;
			if (waste_flag)
			{
				waste = this.waste;
			}

			cr.save();

			cr.translate( -waste, -waste );
			double rx = (w+waste)/2.0;
			double ry = (h+waste)/2.0;

			cr.new_path();
			cr.move_to( 2*rx, ry );
			for ( int i_theta = ARC_FINE; i_theta <= 360; i_theta += ARC_FINE )
			{
				double x = rx + rx*Math.cos( i_theta * Math.PI / 180.0 );
				double y = ry + ry*Math.sin( i_theta * Math.PI / 180.0 );
				cr.line_to( x, y );
			}
			cr.close_path();

			cr.restore();
		}


	}

}
