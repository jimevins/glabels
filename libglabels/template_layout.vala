/*  template_layout.vala
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

	public class TemplateLayout
	{
		public int    nx    { get; protected set; }
		public int    ny    { get; protected set; }

		public double x0    { get; protected set; }
		public double y0    { get; protected set; }

		public double dx    { get; protected set; }
		public double dy    { get; protected set; }


		public TemplateLayout( int    nx,
		                       int    ny,
		                       double x0,
		                       double y0,
		                       double dx,
		                       double dy )
		{
			this.nx = nx;
			this.ny = ny;

			this.x0 = x0;
			this.y0 = y0;

			this.dx = dx;
			this.dy = dy;
		}


		public TemplateLayout dup()
		{
			TemplateLayout copy = new TemplateLayout( nx, ny, x0, y0, dx, dy );

			return copy;
		}


		public bool is_similar( TemplateLayout layout2 )
		{
			if ( (nx == layout2.nx)                &&
			     (ny == layout2.ny)                &&
			     (fabs(x0 - layout2.x0) < EPSILON) &&
			     (fabs(y0 - layout2.y0) < EPSILON) &&
			     (fabs(dx - layout2.dx) < EPSILON) &&
			     (fabs(dy - layout2.dy) < EPSILON) )
			{
				return true;
			}
			else
			{
				return false;
			}
		}

			
	}

}
