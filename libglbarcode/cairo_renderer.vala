/*  cairo_renderer.vala
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

	public class CairoRenderer : Renderer
	{

		private const double FONT_SCALE  = 72.0/96;
		private const string FONT_FAMILY = "Sans";


		private Cairo.Context cr;
		private bool          fill;


		public CairoRenderer( Cairo.Context cr, bool fill = true )
		{
			this.cr   = cr;
			this.fill = fill;
		}


		public override void draw_begin( double w, double h )
		{
		}


		public override void draw_end()
		{
		}


		public override void draw_box( ShapeBox box )
		{
			cr.rectangle( box.x, box.y, box.w, box.h );
			if ( fill )
			{
				cr.fill();
			}
		}


		public override void draw_text( ShapeText text )
		{
			Pango.FontDescription desc = new Pango.FontDescription();
			desc.set_family( FONT_FAMILY );
			desc.set_size( (int)(text.fsize * Pango.SCALE * FONT_SCALE + 0.5) );

			Pango.Layout layout = Pango.cairo_create_layout( cr );
			layout.set_font_description( desc );

			layout.set_text( text.s, -1);

			int iw, ih;
			layout.get_size( out iw, out ih );
			double layout_width = (double)iw / (double)Pango.SCALE;

			double x_offset = layout_width / 2.0;
			double y_offset = 0.2 * text.fsize;

			cr.move_to( (text.x - x_offset), (text.y - y_offset) );
			if ( fill )
			{
				Pango.cairo_show_layout( cr, layout );
			}
			else
			{
				Pango.cairo_layout_path( cr, layout );
			}
		}


		public override void draw_ring( ShapeRing ring )
		{
			cr.new_sub_path();
			cr.arc( ring.x, ring.y, ring.r + ring.line_width/2, 0.0, 2*Math.PI );
			cr.close_path();
			cr.new_sub_path();
			cr.arc( ring.x, ring.y, ring.r - ring.line_width/2, 0.0, -2*Math.PI );
			cr.close_path();
			if ( fill )
			{
				cr.fill();
			}
		}


		public override void draw_hexagon( ShapeHexagon hexagon )
		{
			cr.move_to( hexagon.x,                   hexagon.y );
			cr.line_to( hexagon.x + 0.433*hexagon.h, hexagon.y + 0.25*hexagon.h );
			cr.line_to( hexagon.x + 0.433*hexagon.h, hexagon.y + 0.75*hexagon.h );
			cr.line_to( hexagon.x,                   hexagon.y +      hexagon.h );
			cr.line_to( hexagon.x - 0.433*hexagon.h, hexagon.y + 0.75*hexagon.h );
			cr.line_to( hexagon.x - 0.433*hexagon.h, hexagon.y + 0.25*hexagon.h );
			cr.close_path();
			if ( fill )
			{
				cr.fill();
			}
		}

	}

}
