/*  template_markup_circle.vala
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

namespace libglabels
{

	public class TemplateMarkupCircle : TemplateMarkup
	{
		public double x0  { get; private set; }
		public double y0  { get; private set; }

		public double r   { get; private set; }


		public TemplateMarkupCircle( double x0,
		                             double y0,
		                             double r )
		{
			this.x0 = x0;
			this.y0 = y0;

			this.r  = r;
		}


		public override TemplateMarkup dup()
		{
			TemplateMarkupCircle copy = new TemplateMarkupCircle( x0, y0, r );

			return (TemplateMarkup)copy;
		}


		public override void cairo_path( Cairo.Context cr,
		                                 TemplateFrame frame )
		{
			cr.arc( x0, y0, r, 0, 2*Math.PI );
			cr.close_path();
		}

	}

}
