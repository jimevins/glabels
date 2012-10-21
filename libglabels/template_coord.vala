/*  template_coord.vala
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

	public struct TemplateCoord
	{
		public double   x { get; set; } /* x coordinate of label left edge relative to left edge of paper */
		public double   y { get; set; } /* y coordinate of label top edge relative to top edge of paper */

		public TemplateCoord( double x,
		                      double y )
		{
			this.x = x;
			this.y = y;
		}


		public int compare_to( TemplateCoord b )
		{
			if ( this.y < b.y )
			{
				return -1;
			}
			else if ( this.y > b.y )
			{
				return 1;
			}
			else
			{
				if ( this.x < b.x )
				{
					return -1;
				}
				else if ( this.x > b.x )
				{
					return 1;
				}
				else
				{
					return 0; /* hopefully 2 label frames won't have the same origin. */
				}
			}
		}

	}

}

