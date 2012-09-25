/*  barcode_menu.vala
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

	public class BarcodeMenu : Gtk.Menu
	{

		public  signal void style_changed();


		public  BarcodeStyle bc_style;


		public BarcodeMenu()
		{
			List<weak string> id_list = BarcodeBackends.get_id_list();

			foreach ( string id in id_list )
			{
				BarcodeStyle bc_style = BarcodeBackends.lookup_style_from_id( id );
				if ( bc_style.backend_id == "" )
				{
					BarcodeMenuItem bc_menu_item = new BarcodeMenuItem( bc_style );
					bc_menu_item.show();
					bc_menu_item.activate.connect( on_menu_item_activated );

					append( bc_menu_item );
				}
			}

			Gtk.MenuItem separator_item = new Gtk.SeparatorMenuItem();
			separator_item.show();
			append( separator_item );

			foreach ( string backend_id in BarcodeBackends.get_backend_id_list() )
			{
				string backend_name = BarcodeBackends.backend_id_to_name( backend_id );
				Gtk.MenuItem menu_item = new Gtk.MenuItem.with_label( backend_name );
				menu_item.show();
				append( menu_item );

				Gtk.Menu sub_menu = new Gtk.Menu();
				menu_item.set_submenu( sub_menu );

				foreach ( string id in id_list )
				{
					BarcodeStyle bc_style = BarcodeBackends.lookup_style_from_id( id );
					if ( bc_style.backend_id == backend_id )
					{
						BarcodeMenuItem bc_menu_item = new BarcodeMenuItem( bc_style );
						bc_menu_item.show();
						bc_menu_item.activate.connect( on_menu_item_activated );

						sub_menu.append( bc_menu_item );
					}
				}

			}

		}


		private void on_menu_item_activated( Gtk.MenuItem menu_item )
		{
			BarcodeMenuItem bc_menu_item = (BarcodeMenuItem)menu_item;
			bc_style = bc_menu_item.bc_style;
			style_changed();
		}


	}

}


