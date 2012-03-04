/*  template_markup_ellipse.vala
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

	public class TemplateMarkupEllipse : TemplateMarkup
	{
		private const int ARC_FINE = 2;


		public double x1  { get; private set; }
		public double y1  { get; private set; }

		public double w   { get; private set; }
		public double h   { get; private set; }


		public TemplateMarkupEllipse( double x1,
		                              double y1,
		                              double w,
		                              double h )
		{
			this.x1 = x1;
			this.y1 = y1;

			this.w  = w;
			this.h  = h;
		}


		public override TemplateMarkup dup()
		{
			TemplateMarkupEllipse copy = new TemplateMarkupEllipse( x1, y1, w, h );

			return (TemplateMarkup)copy;
		}


		public override void cairo_path( Cairo.Context cr,
		                                 TemplateFrame frame )
		{
			cr.save();

			cr.translate( x1, y1 );

			cr.new_path();
			cr.move_to( w, h/2 );
			for ( int i_theta = ARC_FINE; i_theta <= 360; i_theta += ARC_FINE )
			{
				double x = (w/2) + (w/2) * Math.cos( i_theta * Math.PI / 180 );
				double y = (h/2) + (h/2) * Math.sin( i_theta * Math.PI / 180 );

				cr.line_to( x, y );
			}
			cr.close_path();

			cr.restore();
		}

	}

}
