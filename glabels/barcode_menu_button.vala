/*  barcode_menu_button.vala
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

	public class BarcodeMenuButton : Gtk.ToggleButton
	{

		public BarcodeStyle bc_style
		{
			get { return _bc_style; }

			set
			{
				_bc_style = value;
				if ( bc_style.backend_id == "" )
				{
					bc_label.set_text( bc_style.name );
				}
				else
				{
					bc_label.set_text( "%s : %s".printf( BarcodeBackends.backend_id_to_name( bc_style.backend_id ),
					                                     bc_style.name ) );
				}

				style_changed();
			}
		}
		BarcodeStyle _bc_style;


		private Gtk.Label bc_label;
		private BarcodeMenu menu;

		public  signal void style_changed();


		public BarcodeMenuButton()
		{
			Gtk.Box hbox = new Gtk.Box( Gtk.Orientation.HORIZONTAL, 3 );
			this.add( hbox );

			bc_label = new Gtk.Label( "" );
			bc_label.set_alignment( 0, 0.5f );
			hbox.pack_start( bc_label, true, true, 0 );

			Gtk.Arrow arrow = new Gtk.Arrow( Gtk.ArrowType.DOWN, Gtk.ShadowType.IN );
			hbox.pack_end( arrow, false, false, 0 );

			bc_style = BarcodeBackends.lookup_style_from_id( "" ); /* Default style */

			menu = new BarcodeMenu();

			button_press_event.connect( on_button_press_event );
			menu.style_changed.connect( on_menu_style_changed );
			menu.selection_done.connect( on_menu_selection_done );
		}


		private void menu_position_function( Gtk.Menu menu,
		                                     out int  x,
		                                     out int  y,
		                                     out bool push_in )
		{
			Gdk.Screen screen = this.get_screen();
			int w_screen = screen.get_width();
			int h_screen = screen.get_height();

			Gdk.Window window = this.get_window();
			int x_window, y_window;
			window.get_origin( out x_window, out y_window );

			Gtk.Allocation allocation;
			this.get_allocation( out allocation );
			int x_this = allocation.x;
			int y_this = allocation.y;
			int h_this = allocation.height;

			int w_menu, h_menu;
			menu.get_size_request( out w_menu, out h_menu );

			x = x_window + x_this;
			y = y_window + y_this + h_this;

			if ( (y + h_menu) > h_screen )
			{
				y = y_window + y_this - h_menu;

				if ( y < 0 )
				{
					y = h_screen - h_menu;
				}
			}

			if ( (x + w_menu) > w_screen )
			{
				x = w_screen - w_menu;
			}

			push_in = true;
		}


		private bool on_button_press_event( Gdk.EventButton event )
		{
			switch (event.button)
			{

			case 1:
				set_active( true );
				menu.popup( null, null, menu_position_function, event.button, event.time );
				break;

			default:
				break;

			}

			return false;
		}


		private void on_menu_style_changed()
		{
			bc_style = menu.bc_style;
		}


		private void on_menu_selection_done()
		{
			this.set_active( false );
		}


	}

}


