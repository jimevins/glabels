/*  font_menu.vala
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

	public class FontMenu : Gtk.Menu
	{
		public  signal void font_changed( string family );

		private	Gtk.MenuItem   recent_menu_item;
		private	Gtk.Menu       recent_sub_menu;

		private const string[] standard_families = { "Sans", "Serif", "Monospace" };

		private FontHistory    font_history;

		public FontMenu()
		{
			int          i;
			FontMenuItem font_menu_item;
			Gtk.MenuItem separator_item;
			Gtk.MenuItem menu_item;
			List<string> list;


			for ( i = 0; i < standard_families.length; i++ )
			{
				font_menu_item = new FontMenuItem( standard_families[i] );
				this.append( font_menu_item );
				font_menu_item.activated.connect( on_menu_item_activated );
			}


			separator_item = new Gtk.SeparatorMenuItem();
			this.append( separator_item );


			recent_menu_item = new Gtk.MenuItem.with_label( "Recent fonts" );
			this.append( recent_menu_item );

			font_history = new FontHistory( 10 );
			list = font_history.get_family_list();
			recent_sub_menu = create_font_sub_menu( list );
			recent_menu_item.set_submenu( recent_sub_menu );
			recent_menu_item.set_sensitive( list != null );


			menu_item = new Gtk.MenuItem.with_label( "Proportional fonts" );
			this.append( menu_item );
			menu_item.set_submenu( create_font_sub_menu( FontFamilies.proportional ) );
			menu_item.set_sensitive( FontFamilies.proportional != null );


			menu_item = new Gtk.MenuItem.with_label( "Fixed-width fonts" );
			this.append( menu_item );
			menu_item.set_submenu( create_font_sub_menu( FontFamilies.fixed_width ) );
			menu_item.set_sensitive( FontFamilies.fixed_width != null );


			menu_item = new Gtk.MenuItem.with_label( "All fonts" );
			this.append( menu_item );
			menu_item.set_submenu( create_font_sub_menu( FontFamilies.all ) );
			menu_item.set_sensitive( FontFamilies.all != null );


			this.show_all();

			font_history.changed.connect( on_font_history_changed );

		}


		private void on_menu_item_activated( string family )
		{
			font_changed( family );
		}


		private void on_font_history_changed()
		{
			List<string> list;

			list = font_history.get_family_list();
			recent_sub_menu = create_font_sub_menu( list );

			recent_menu_item.set_submenu( recent_sub_menu );
			recent_menu_item.set_sensitive( list != null );
		}


		private Gtk.Menu create_font_sub_menu( List<string> list )
		{
			Gtk.Menu menu = new Gtk.Menu();

			foreach ( string s in list )
			{
				FontMenuItem menu_item = new FontMenuItem( s );
				menu_item.show_all();
				menu_item.activated.connect( on_menu_item_activated );
				menu.append( menu_item );
			}

			return menu;
		}

	}

}





