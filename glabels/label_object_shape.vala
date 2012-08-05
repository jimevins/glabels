/*  label_object_shape.vala
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

	public abstract class LabelObjectShape : LabelObject
	{

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


		/**
		 * Fill color node
		 */
		public override ColorNode fill_color_node
		{
			get { return _fill_color_node; }

			set
			{
				if ( _fill_color_node != value )
				{
					_fill_color_node = value;
					changed();
				}
			}
		}
		private ColorNode _fill_color_node;


		public LabelObjectShape()
		{
			handles.append( new HandleSouthEast( this ) );
			handles.append( new HandleSouthWest( this ) );
			handles.append( new HandleNorthEast( this ) );
			handles.append( new HandleNorthWest( this ) );
			handles.append( new HandleEast( this ) );
			handles.append( new HandleSouth( this ) );
			handles.append( new HandleWest( this ) );
			handles.append( new HandleNorth( this ) );

			Prefs prefs = new Prefs();

			_line_width              = prefs.default_line_width;
			_line_color_node         = ColorNode.from_color( prefs.default_line_color );
			_fill_color_node         = ColorNode.from_color( prefs.default_fill_color );
		}


		public override bool can_fill()
		{
			return true;
		}

		public override bool can_line_color()
		{
			return true;
		}

		public override bool can_line_width()
		{
			return true;
		}


	}

}
