/*  template_markup_line.vala
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

	public class TemplateMarkupLine : TemplateMarkup
	{
		public double x1  { get; private set; }
		public double y1  { get; private set; }

		public double x2  { get; private set; }
		public double y2  { get; private set; }


		public TemplateMarkupLine( double x1,
		                           double y1,
		                           double x2,
		                           double y2 )
		{
			this.x1 = x1;
			this.y1 = y1;

			this.x2 = x2;
			this.y2 = y2;
		}


		public override TemplateMarkup dup()
		{
			TemplateMarkupLine copy = new TemplateMarkupLine( x1, y1, x2, y2 );

			return (TemplateMarkup)copy;
		}


		public override void cairo_path( Cairo.Context cr,
		                                 TemplateFrame frame )
		{
			cr.move_to( x1, y1 );
			cr.line_to( x2, y2 );
		}

	}

}
