/*  color_history.vala
 *
 *  Copyright (C) 2011  Jim Evins <evins@snaught.com>
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

	public class ColorHistory : Object
	{

		public signal void changed();

		private static GLib.Settings history;
		private int                  max_n;

		static construct
		{
			history = new GLib.Settings( "org.gnome.glabels-3.history" );
		}

		public ColorHistory( int n )
		{
			max_n = n;

			history.changed["recent-colors"].connect( on_history_changed );
		}

		public void add_color( Color color )
		{
			size_t n;
			uint[] old_colors = get_color_array( out n );
			uint[] new_colors = new uint[max_n];
			size_t i;

			new_colors[0] = color.to_legacy_color();

			for ( i = 0; (i < (max_n-1)) && (i < n); i++ )
			{
				new_colors[i+1] = old_colors[i];
			}

			set_color_array( new_colors, i+1 );
		}

		public Color get_color( int i )
		{
			size_t n;
			uint[] colors = get_color_array( out n );
			Color  color;

			if ( (n > 0) && (i < n))
			{
				color = Color.from_legacy_color( colors[i] );
			}
			else
			{
				color = Color.none();
			}

			return color;
		}

		private void on_history_changed()
		{
			changed();
		}

		private uint[] get_color_array( out size_t n )
		{
			GLib.Variant value;
			GLib.Variant child_value;
			size_t       i;

			value = history.get_value( "recent-colors" );
			n     = value.n_children();

			uint[] array = new uint[n];

			for ( i = 0; i < n; i++ )
			{
				child_value = value.get_child_value( i );
				array[i] = child_value.get_uint32();
			}

			return array;
		}

		private void set_color_array( uint[] array, size_t n )
		{
			GLib.Variant   value;
			GLib.Variant[] child_values;
			size_t         i;

			child_values = new GLib.Variant[n];

			for ( i = 0; i < n; i++ )
			{
				child_values[i] = new Variant.uint32( array[i] );
			}

			value = new Variant.array( GLib.VariantType.UINT32, child_values );

			history.set_value( "recent-colors", value );
		}

	}

}
