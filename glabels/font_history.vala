/*  font_history.vala
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

	public class FontHistory : Object
	{

		public signal void changed();

		private static GLib.Settings history;
		private int                  max_n;

		private FontFamilies         font_families;

		static construct
		{
			history = new GLib.Settings( "org.gnome.glabels-3.history" );
		}

		public FontHistory( int n )
		{
			max_n = n;

			history.changed["recent-fonts"].connect( on_history_changed );

			font_families = new FontFamilies();
		}

		public void add_familty( string family )
		{
			string[] old_families;
			string[] new_families;
			int      i, j;

			old_families = history.get_strv( "recent-fonts" );

			new_families = new string[1];
			new_families += family;

			for ( i = 0, j = 1; (j < (max_n-1)) && (i < old_families.length); i++ )
			{
				if ( family != old_families[i] )
				{
					new_families += old_families[i];
				}
			}

			history.set_strv( "recent-fonts", new_families );
		}

		public List<string> get_family_list()
		{
			string[]     families;
			List<string> family_list = new List<string>();
			int          i;

			families = history.get_strv( "recent-fonts" );

			for ( i = 0; i < families.length; i++ )
			{
				if ( font_families.is_family_installed( families[i] ) )
				{
					family_list.append( families[i] );
				}
			}

			return family_list;
		}

		private void on_history_changed()
		{
			changed();
		}

	}

}


