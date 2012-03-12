/*  field_button_menu.vala
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

	public class FieldMenu : Gtk.Menu
	{
		public  signal void key_selected( string key );

		private List<unowned FieldMenuItem> menu_items;

		private const int MAX_MENU_ROWS = 25;


		public void set_keys( List<string> key_list )
		{
			/* Remove old menu items and cleanup list. */
			unowned List<FieldMenuItem> p;
			unowned List<FieldMenuItem> p_next = null;
			for ( p = menu_items; p != null; p = p_next )
			{
				remove( p.first().data );

				p_next = p.next;
				menu_items.delete_link( p );
			}
			unrealize();

			/* Load up new keys. */
			int i = 0;
			foreach ( string key in key_list )
			{
				FieldMenuItem menu_item = new FieldMenuItem( key );
				menu_item.show();
				menu_item.activate.connect( on_menu_item_activated );

				menu_items.append( menu_item );

				int i_row = i % MAX_MENU_ROWS;
				int i_col = i / MAX_MENU_ROWS;
				attach( menu_item, i_col, i_col+1, i_row, i_row+1 );

				i++;
			}
		}


		private void on_menu_item_activated( Gtk.MenuItem menu_item )
		{
			FieldMenuItem f_menu_item = (FieldMenuItem)menu_item;
			key_selected( f_menu_item.key );
		}


	}

}


