/*  template_frame_cd.vala
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

	public class TemplateFrameCD : TemplateFrame
	{
		public double    r1      { get; protected set; }
		public double    r2      { get; protected set; }

		public double    w       { get; protected set; }
		public double    h       { get; protected set; }

		public double    waste   { get; protected set; }


		public TemplateFrameCD( string id,
		                        double r1,
		                        double r2,
		                        double w,
		                        double h,
		                        double waste )
		{
			this.id      = id;

			this.r1      = r1;
			this.r2      = r2;

			this.w       = w;
			this.h       = h;

			this.waste   = waste;
		}


		public override TemplateFrame dup()
		{
			TemplateFrameCD copy = new TemplateFrameCD( id, r1, r2, w, h, waste );

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
			if ( this.w == 0 )
			{
				w = 2.0 * r1;
			}
			else
			{
				w = this.w;
			}

			if ( this.h == 0 )
			{
				h = 2.0 *r1;
			}
			else
			{
				h = this.h;
			}
		}


		public override bool is_similar( TemplateFrame frame2 )
		{
			if ( frame2 is TemplateFrameCD )
			{
				TemplateFrameCD cd2 = (TemplateFrameCD) frame2;

				if ( (fabs( w  - cd2.w )  <= EPSILON) &&
				     (fabs( h  - cd2.h )  <= EPSILON) &&
				     (fabs( r1 - cd2.r1 ) <= EPSILON) &&
				     (fabs( r2 - cd2.r2 ) <= EPSILON) )
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
				string dstr = StrUtil.format_fraction( 2 * r1 * units_per_point );

				description = "%s %s %s".printf( dstr, units_string, _("diameter") );
			}
			else
			{
				description = "%.5g %s %s".printf( 2 * r1 * units_per_point,
				                                   units_string,
				                                   _("diameter") );
			}

			return description;
		}


		public override void cairo_path( Cairo.Context cr,
		                                 bool          waste_flag )
		{
			double w, h;

			get_size( out w, out h );

			double xc = w/2.0;
			double yc = h/2.0;

			double waste = 0.0;
			if (waste_flag)
			{
				waste = this.waste;
			}

			/*
			 * Outer path (may be clipped in the case of a business card type CD)
			 */
			double theta1 = Math.acos( w / (2.0*r1) );
			double theta2 = Math.asin( h / (2.0*r1) );

			cr.new_path();
			cr.arc( xc, yc, r1+waste, theta1, theta2 );
			cr.arc( xc, yc, r1+waste, Math.PI-theta2, Math.PI-theta1 );
			cr.arc( xc, yc, r1+waste, Math.PI+theta1, Math.PI+theta2 );
			cr.arc( xc, yc, r1+waste, 2*Math.PI-theta2, 2*Math.PI-theta1 );
			cr.close_path();


			/*
			 * Inner path (hole)
			 */
			cr.new_sub_path();
			cr.arc( xc, yc, r2-waste, 0.0, 2*Math.PI );
			cr.close_path();
		}


	}

}
