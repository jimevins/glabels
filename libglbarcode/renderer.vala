/*  renderer.vala
 *
 *  Copyright (C) 2012  Jim Evins <evins@snaught.com>
 *
 *  This file is part of libglbarcode.
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

namespace glbarcode
{

	public abstract class Renderer
	{
		public void render( double w, double h, List<Shape> shapes )
		{
			draw_begin( w, h );

			foreach (Shape shape in shapes)
			{
				if ( shape is ShapeBox )
				{
					draw_box( shape as ShapeBox );
				}
				else if ( shape is ShapeText )
				{
					draw_text( shape as ShapeText );
				}
				else if ( shape is ShapeRing )
				{
					draw_ring( shape as ShapeRing );
				}
				else if ( shape is ShapeHexagon )
				{
					draw_hexagon( shape as ShapeHexagon );
				}
				else
				{
					assert_not_reached();
				}
			}

			draw_end();
		}


		protected abstract void draw_begin( double w, double h );
		protected abstract void draw_end();

		protected abstract void draw_box(     ShapeBox     box );
		protected abstract void draw_text(    ShapeText    text );
		protected abstract void draw_ring(    ShapeRing    ring );
		protected abstract void draw_hexagon( ShapeHexagon hexagon );

	}

}
