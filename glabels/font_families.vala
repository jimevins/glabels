/*  font_families.vala
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

	class FontFamilies
	{

		public static List<string> all;
		public static List<string> proportional;
		public static List<string> fixed_width;


		static construct
		{
			Pango.Context       context;
			Pango.FontFamily[]  families;
			int                 i;
			string              name;

			context = Gdk.pango_context_get();

			context.list_families( out families );

			for ( i = 0; i < families.length; i++ )
			{
				name = families[i].get_name();

				all.insert_sorted( name, string.collate );

				if ( families[i].is_monospace() )
				{
					fixed_width.insert_sorted( name, string.collate );
				}
				else
				{
					proportional.insert_sorted( name, string.collate );
				}

			}

		}


		public string validate_family( string family )
		{
			string good_family;

			if ( all.find_custom( family, string.collate ) != null )
			{
				good_family = family;
			}
			else if ( all.find_custom( "Sans", string.collate ) != null )
			{
				good_family = "Sans";
			}
			else if ( all != null )
			{
				good_family = all.data;
			}
			else
			{
				good_family = null;
			}

			return good_family;
		}


		public bool is_family_installed( string family  )
		{
			unowned List<string> p;

			p = all.find_custom( family, string.collate );

			return ( p != null );
		}


	}

}


