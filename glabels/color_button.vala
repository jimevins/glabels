/*  color_button.vala
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
using Gtk;

namespace glabels
{

	public class ColorButton : Gtk.ToggleButton
	{
		private const int SWATCH_W = 100;
		private const int SWATCH_H = 24;

		public  signal void color_changed( ColorNode color_node,
		                                   bool      is_default );

		private bool           is_default_flag;
		private ColorNode      color_node;

		private Color          default_color;

		private ColorSwatch    swatch;
		private Gtk.Label      key_label;
		private ColorMenu      menu;


		public ColorButton( string? default_label,
		                    Color   default_color,
		                    Color   color )
		{
			if ( default_label == null )
			{
				default_label = _("Default color");
			}

			Gtk.HBox hbox = new Gtk.HBox( false, 3 );
			this.add( hbox );

			swatch = new ColorSwatch( SWATCH_W, SWATCH_H, color );
			hbox.pack_start( swatch, true, true, 0 );

			key_label = new Gtk.Label( "" );
			key_label.hide();
			hbox.pack_start( key_label, true, true, 0 );

			Gtk.Arrow arrow = new Gtk.Arrow( Gtk.ArrowType.DOWN, Gtk.ShadowType.IN );
			hbox.pack_end( arrow, false, false, 0 );

			this.default_color = default_color;
			this.color_node    = ColorNode.from_color( color );

			menu = new ColorMenu( default_label, default_color, color );
			menu.show_all();

			menu.color_changed.connect( on_menu_color_changed );
			menu.selection_done.connect( on_menu_selection_done );

			this.button_press_event.connect( on_button_press_event );
			this.show.connect( on_show );
		}


		public void set_color_node( ColorNode color_node )
		{
			is_default_flag = false;

			this.color_node = color_node;

			swatch.set_color( color_node.color );
		}


		public void set_color( Color color )
		{
			is_default_flag = false;

			this.color_node.field_flag = false;
			this.color_node.color      = color;
			this.color_node.key        = null;

			swatch.set_color( color );
		}


		public void set_to_default()
		{
			is_default_flag = true;

			color_node.field_flag = false;
			color_node.color      = default_color;
			color_node.key        = null;

			swatch.set_color( default_color );
		}


		public ColorNode get_color_node( out bool is_default )
		{
			is_default = is_default_flag;
			return color_node;
		}


		public void set_keys( List<string> key_list )
		{
			menu.set_keys( key_list );
		}


		public void clear_keys()
		{
			menu.clear_keys();
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


		private void on_show()
		{
			base.show();

			if ( color_node.field_flag )
			{
				swatch.hide();
				key_label.show();
			}
			else
			{
				swatch.show();
				key_label.hide();
			}
		}


		private void on_menu_color_changed( ColorNode color_node, bool is_default )
		{
			this.color_node      = color_node;
			this.is_default_flag = is_default;

			if ( color_node.field_flag )
			{
				swatch.hide();
				key_label.show();
				key_label.set_label( "$(%s)".printf( color_node.key ) );
			}
			else
			{
				swatch.show();
				swatch.set_color( color_node.color );
				key_label.hide();
			}

			color_changed( color_node, is_default );
		}


		private void on_menu_selection_done()
		{
			this.set_active( false );
		}

	}

}
