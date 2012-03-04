/*  template_frame_round.vala
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

	public class TemplateFrameRound : TemplateFrame
	{
		public double    r       { get; protected set; }

		public double    waste   { get; protected set; }


		public TemplateFrameRound( string id,
		                           double r,
		                           double waste )
		{
			this.id      = id;

			this.r       = r;

			this.waste   = waste;
		}


		public override TemplateFrame dup()
		{
			TemplateFrameRound copy = new TemplateFrameRound( id, r, waste );

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
			w = h = 2.0 * r;
		}


		public override bool is_similar( TemplateFrame frame2 )
		{
			if ( frame2 is TemplateFrameRound )
			{
				TemplateFrameRound round2 = (TemplateFrameRound) frame2;

				if ( fabs( r - round2.r) <= EPSILON )
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
				string dstr = StrUtil.format_fraction( 2 * r * units_per_point );

				description = "%s %s %s".printf( dstr, units_string, _("diameter") );
			}
			else
			{
				description = "%.5g %s %s".printf( 2 * r * units_per_point,
												   units_string,
												   _("diameter") );
			}

			return description;
		}


		public override void cairo_path( Cairo.Context cr,
		                                 bool          waste_flag )
		{
			double waste = 0.0;
			if (waste_flag)
			{
				waste = this.waste;
			}

			cr.new_path();
			cr.arc( r, r, r+waste, 0.0, 2*Math.PI );
			cr.close_path();
		}


	}

}
