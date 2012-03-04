/*  template_markup_rect.vala
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

	public class TemplateMarkupRect : TemplateMarkup
	{
		public double x1  { get; private set; }
		public double y1  { get; private set; }

		public double w   { get; private set; }
		public double h   { get; private set; }

		public double r   { get; private set; }


		public TemplateMarkupRect( double x1,
		                           double y1,
		                           double w,
		                           double h,
		                           double r )
		{
			this.x1 = x1;
			this.y1 = y1;

			this.w  = w;
			this.h  = h;

			this.r  = r;
		}


		public override TemplateMarkup dup()
		{
			TemplateMarkupRect copy = new TemplateMarkupRect( x1, y1, w, h, r );

			return (TemplateMarkup)copy;
		}


		public override void cairo_path( Cairo.Context cr,
		                                 TemplateFrame frame )
		{
			if ( r == 0 )
			{
				cr.rectangle( x1, y1, w, h );
			}
			else
			{
				cr.new_path();
				cr.arc_negative( x1+r,   y1+r,   r, 3*Math.PI/2, Math.PI );
				cr.arc_negative( x1+r,   y1+h-r, r, Math.PI,     Math.PI/2 );
				cr.arc_negative( x1+w-r, y1+h-r, r, Math.PI/2,   0 );
				cr.arc_negative( x1+w-r, y1+r,   r, 2*Math.PI,   3*Math.PI/2 );
				cr.close_path();
			}
		}

	}

}
