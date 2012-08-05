/*  label_object_ellipse.vala
 *
 *  Copyright (C) 2012  Jim Evins <evins@snaught.com>
 *
 *  This file is part of gLabels.
 *
 *  gLabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gLabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gLabels.  If not, see <http://www.gnu.org/licenses/>.
 */


using GLib;

namespace glabels
{

	public class LabelObjectEllipse : LabelObjectShape
	{

		private const int ARC_FINE = 2;


		public override LabelObject dup()
		{
			LabelObjectEllipse copy = new LabelObjectEllipse();

			copy.set_common_properties_from_object( this );

			return copy;
		}


		public override void draw_object( Cairo.Context cr, bool in_editor, MergeRecord? record )
		{
			Color line_color = line_color_node.expand( record );
			Color fill_color = fill_color_node.expand( record );

			if ( in_editor && line_color_node.field_flag )
			{
				line_color = Color.from_rgba( 0, 0, 0, 0.5 );
			}

			if ( in_editor && fill_color_node.field_flag )
			{
				fill_color = Color.from_rgba( 0.5, 0.5, 0.5, 0.5 );
			}

			ellipse_path( cr, 0, 0, w, h );

			/* Paint fill color */
			cr.set_source_rgba( fill_color.r, fill_color.g, fill_color.b, fill_color.a );
			cr.fill_preserve();

			/* Draw outline */
			cr.set_source_rgba( line_color.r, line_color.g, line_color.b, line_color.a );
			cr.set_line_width( line_width );
			cr.stroke();
		}


		public override void draw_shadow( Cairo.Context cr, bool in_editor, MergeRecord? record )
		{
			Color line_color   = line_color_node.expand( record );
			Color fill_color   = fill_color_node.expand( record );
			Color shadow_color = shadow_color_node.expand( record );

			if ( in_editor && line_color_node.field_flag )
			{
				line_color = Color.from_rgba( 0, 0, 0, 0.5 );
			}

			if ( in_editor && fill_color_node.field_flag )
			{
				fill_color = Color.from_rgba( 0.5, 0.5, 0.5, 0.5 );
			}

			if ( in_editor && shadow_color_node.field_flag )
			{
				shadow_color = Color.black();
			}

			shadow_color.set_opacity( shadow_opacity );

			cr.set_source_rgba( shadow_color.r, shadow_color.g, shadow_color.b, shadow_color.a );

			if ( fill_color.has_alpha() )
			{
				if ( line_color.has_alpha() )
				{
					/* Has FILL and OUTLINE: adjust size to account for line width. */
					ellipse_path( cr, -line_width/2, -line_width/2, w+line_width, h+line_width );
				}
				else
				{
					/* Has FILL, but no OUTLINE. */
					ellipse_path( cr, 0, 0, w, h );
				}

				/* Draw shadow */
				cr.fill();
			}
			else
			{
				if ( line_color.has_alpha() )
				{
					/* Has only OUTLINE. */
					ellipse_path( cr, 0, 0, w, h );

					/* Draw shdow of OUTLINE. */
					cr.set_line_width( line_width );
					cr.stroke();
				}
			}
		}


		public override bool is_object_located_at( Cairo.Context cr, double x, double y )
		{
			cr.new_path();
			ellipse_path( cr, 0, 0, w, h );

			if ( cr.in_fill( x, y ) )
			{
				return true;
			}

			cr.set_line_width( line_width );
			if ( cr.in_stroke( x, y ) )
			{
				return true;
			}

			return false;
		}


		public void ellipse_path( Cairo.Context cr,
		                          double        x0,
		                          double        y0,
		                          double        w,
		                          double        h )
		{
			cr.save();

			cr.translate( x0, y0 );

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
