/*  font_button.vala
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

	public class FontButton : Gtk.ToggleButton
	{

		public  signal void changed();

		private string       font_family;

		private Gtk.Label    font_label;
		private FontMenu     menu;


		public FontButton( string?  font_family )
		{
			this.font_family = font_family;

			Gtk.HBox hbox = new Gtk.HBox( false, 3 );
			this.add( hbox );

			font_label = new Gtk.Label( font_family );
			font_label.set_alignment( 0.0f, 0.5f );
			font_label.set_size_request( 180, -1 );
			hbox.pack_start( font_label, true, true, 0 );

			Gtk.Arrow arrow = new Gtk.Arrow( Gtk.ArrowType.DOWN, Gtk.ShadowType.IN );
			hbox.pack_end( arrow, false, false, 0 );

			this.button_press_event.connect( on_button_press_event );

			menu = new FontMenu();
			menu.show_all();

			menu.font_changed.connect( on_menu_font_changed );
			menu.selection_done.connect( on_menu_selection_done );

			this.button_press_event.connect( on_button_press_event );
		}

		public void set_family( string family )
		{
			this.font_family = family;
			this.font_label.set_text( family );
		}

		public string get_family()
		{
			return this.font_family;
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
				this.set_active( true );
				menu.popup( null, null, menu_position_function, event.button, event.time );
				break;

			default:
				break;

			}

			return false;
		}

		private void on_menu_font_changed( string family )
		{
			this.font_family = family;
			this.font_label.set_text( family );
			changed();
		}

		private void on_menu_selection_done()
		{
			this.set_active( false );
		}

	}

}




