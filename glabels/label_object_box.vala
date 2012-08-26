/*  label_object_box.vala
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

	public class LabelObjectBox : LabelObjectShape
	{

		public LabelObjectBox.with_parent( Label parent )
		{
			this();
			parent.add_object( this );
		}


		public override LabelObject dup()
		{
			LabelObjectBox copy = new LabelObjectBox();

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

			cr.rectangle( 0, 0, w, h );

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
					cr.rectangle( -line_width/2, -line_width/2, w+line_width, h+line_width );
				}
				else
				{
					/* Has FILL, but no OUTLINE. */
					cr.rectangle( 0, 0, w, h );
				}

				/* Draw shadow */
				cr.fill();
			}
			else
			{
				if ( line_color.has_alpha() )
				{
					/* Has only OUTLINE. */
					cr.rectangle( 0, 0, w, h );

					/* Draw shdow of OUTLINE. */
					cr.set_line_width( line_width );
					cr.stroke();
				}
			}
		}


		public override bool is_object_located_at( Cairo.Context cr, double x, double y )
		{
			cr.new_path();
			cr.rectangle( 0, 0, w, h );

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


	}

}
