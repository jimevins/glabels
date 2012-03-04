/*  template_frame_rect.vala
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

	public class TemplateFrameRect : TemplateFrame
	{
		public double    w       { get; protected set; }
		public double    h       { get; protected set; }

		public double    r       { get; protected set; }

		public double    x_waste { get; protected set; }
		public double    y_waste { get; protected set; }


		public TemplateFrameRect( string id,
		                          double w,
		                          double h,
		                          double r,
		                          double x_waste,
		                          double y_waste )
		{
			this.id      = id;

			this.w       = w;
			this.h       = h;

			this.r       = r;

			this.x_waste = x_waste;
			this.y_waste = y_waste;
		}


		public override TemplateFrame dup()
		{
			TemplateFrameRect copy = new TemplateFrameRect( id, w, h, r, x_waste, y_waste );

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
			if ( frame2 is TemplateFrameRect )
			{
				TemplateFrameRect rect2 = (TemplateFrameRect) frame2;

				if ( (fabs( w - rect2.w) <= EPSILON) &&
					 (fabs( h - rect2.h) <= EPSILON) )
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
			double x_waste = 0.0;
			double y_waste = 0.0;

			double w, h;

			w = this.w;
			h = this.h;

			if (waste_flag)
			{
				x_waste = this.x_waste;
				y_waste = this.y_waste;
			}

			if ( r == 0.0 )
			{
				cr.rectangle( -x_waste, -y_waste, w+2*x_waste, h+2*y_waste );
			}
			else
			{
				cr.new_path();
				cr.arc_negative( r-x_waste,   r-y_waste,   r, 3*Math.PI/2, Math.PI );
				cr.arc_negative( r-x_waste,   h-r+y_waste, r, Math.PI,     Math.PI/2 );
				cr.arc_negative( w-r+x_waste, h-r+y_waste, r, Math.PI/2,   0.0 );
				cr.arc_negative( w-r+x_waste, r-y_waste,   r, 2*Math.PI,   3*Math.PI/2 );
				cr.close_path();
			}
		}


	}

}
