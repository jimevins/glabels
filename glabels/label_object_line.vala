/*  label_object_line.vala
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

	public class LabelObjectLine : LabelObject
	{

		private const double SELECTION_SLOP_PIXELS = 4.0;

		/**
		 * Line width
		 */
		public override double line_width
		{
			get { return _line_width; }

			set
			{
				if ( _line_width != value )
				{
					_line_width = value;
					changed();
				}
			}
		}
		private double _line_width;


		/**
		 * Line color node
		 */
		public override ColorNode line_color_node
		{
			get { return _line_color_node; }

			set
			{
				if ( _line_color_node != value )
				{
					_line_color_node = value;
					changed();
				}
			}
		}
		private ColorNode _line_color_node;



		public LabelObjectLine()
		{
			handles.append( new HandleP1( this ) );
			handles.append( new HandleP2( this ) );

			Prefs prefs = new Prefs();

			_line_width              = prefs.default_line_width;
			_line_color_node         = ColorNode.from_color( prefs.default_line_color );
		}


		public override bool can_line_color()
		{
			return true;
		}

		public override bool can_line_width()
		{
			return true;
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

			if ( in_editor && line_color_node.field_flag )
			{
				line_color = Color.from_rgba( 0, 0, 0, 0.5 );
			}

			cr.move_to( 0, 0 );
			cr.line_to( w, h );

			/* Draw line */
			cr.set_source_rgba( line_color.r, line_color.g, line_color.b, line_color.a );
			cr.set_line_width( line_width );
			cr.stroke();
		}


		public override void draw_shadow( Cairo.Context cr, bool in_editor, MergeRecord? record )
		{
			Color line_color   = line_color_node.expand( record );
			Color shadow_color = shadow_color_node.expand( record );

			if ( in_editor && line_color_node.field_flag )
			{
				line_color = Color.from_rgba( 0, 0, 0, 0.5 );
			}

			if ( in_editor && shadow_color_node.field_flag )
			{
				shadow_color = Color.black();
			}

			shadow_color.set_opacity( shadow_opacity );

			cr.set_source_rgba( shadow_color.r, shadow_color.g, shadow_color.b, shadow_color.a );

			if ( line_color.has_alpha() )
			{
				cr.move_to( 0, 0 );
				cr.line_to( w, h );

				/* Draw shadow. */
				cr.set_line_width( line_width );
				cr.stroke();
			}
		}


		public override bool is_object_located_at( Cairo.Context cr, double x, double y )
		{
			cr.new_path();
			cr.move_to( 0, 0 );
			cr.line_to( w, h );

			cr.set_line_width( line_width + 2*SELECTION_SLOP_PIXELS );
			if ( cr.in_stroke( x, y ) )
			{
				return true;
			}

			return false;
		}


	}

}
