/*  color_menu.vala
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

	public class ColorMenu : Gtk.Menu
	{
		public  signal void color_changed( ColorNode color_node,
		                                   bool      is_default );

		private Color          default_color;
		private ColorNode      color_node;

		private struct ColorTableEntry {
			uchar  r;
			uchar  g;
			uchar  b;
			string name;
		}

		private const ColorTableEntry color_table[] = {

			{ 139,   0,   0, N_("Dark Red") },
			{ 165,  42,  42, N_("Brown") },
			{ 205, 149,  12, N_("Dark Goldenrod") },
			{   0, 100,   0, N_("Dark Green") },
			{   0, 139, 139, N_("Dark Cyan") },
			{   0,   0, 128, N_("Navy Blue") },
			{ 148,   0, 211, N_("Dark Violet") },

			{ 255,   0,   0, N_("Red") },
			{ 255, 165,   0, N_("Orange") },
			{ 205, 205,   0, N_("Dark Yellow") },
			{   0, 205,   0, N_("Medium green") },
			{  64, 224, 208, N_("Turquoise") },
			{   0,   0, 255, N_("Blue") },
			{ 160,  32, 240, N_("Purple") },

			{ 250, 128, 114, N_("Salmon") },
			{ 255, 215,   0, N_("Gold") },
			{ 255, 255,   0, N_("Yellow") },
			{   0, 255,   0, N_("Green") },
			{   0, 255, 255, N_("Cyan") },
			{ 135, 206, 235, N_("SkyBlue") },
			{ 238, 130, 238, N_("Violet") },

			{ 255, 192, 203, N_("Pink") },
			{ 255, 246, 143, N_("Khaki") },
			{ 255, 255, 224, N_("Light Yellow") },
			{ 144, 238, 144, N_("Light Green") },
			{ 224, 255, 255, N_("Light Cyan") },
			{ 198, 226, 255, N_("Slate Gray") },
			{ 216, 191, 216, N_("Thistle") },

			{ 255, 255, 255, N_("White") },
			/* xgettext: no-c-format */
			{ 230, 230, 230, N_("10% Gray") },
			/* xgettext: no-c-format */
			{ 192, 192, 192, N_("25% Gray") },
			/* xgettext: no-c-format */
			{ 153, 153, 153, N_("40% Gray") },
			/* xgettext: no-c-format */
			{ 128, 128, 128, N_("50% Gray") },
			/* xgettext: no-c-format */
			{ 102, 102, 102, N_("60% Gray") },
			{   0,   0,   0, N_("Black") }

		};

		private const int      PALETTE_COLS = 7;
		private const int      PALETTE_ROWS = color_table.length/PALETTE_COLS + 1;

		private const int      ROW_DEFAULT = 0;
		private const int      ROW_SEP_1   = ROW_DEFAULT + 1;
		private const int      ROW_PALETTE = ROW_SEP_1   + 1;
		private const int      ROW_SEP_2   = ROW_PALETTE + PALETTE_ROWS;
		private const int      ROW_HISTORY = ROW_SEP_2   + 1;
		private const int      ROW_SEP_3   = ROW_HISTORY + 1;
		private const int      ROW_CUSTOM  = ROW_SEP_3   + 1;
		private const int      ROW_SEP_4   = ROW_CUSTOM  + 1;
		private const int      ROW_FIELD   = ROW_SEP_4   + 1;

		private	Gtk.MenuItem       default_menu_item;

		private ColorHistory       custom_color_history;
		private ColorMenuItem      history_menu_item[7];

		private	Gtk.MenuItem       custom_menu_item;

		private	Gtk.MenuItem       use_merge_field_menu_item;
		private FieldMenu          merge_field_menu;


		public ColorMenu( string default_label,
		                  Color  default_color,
		                  Color  color )
		{
			Gtk.SeparatorMenuItem separator_menu_item;

			this.default_color = default_color;
			this.color_node    = ColorNode.from_color( color );

			this.default_menu_item = new Gtk.MenuItem.with_label( default_label );
			this.attach( this.default_menu_item, 0, PALETTE_COLS, ROW_DEFAULT, ROW_DEFAULT+1 );
			this.default_menu_item.activate.connect( on_default_menu_item_activate );

			separator_menu_item = new Gtk.SeparatorMenuItem();
			this.attach( separator_menu_item, 0, PALETTE_COLS, ROW_SEP_1, ROW_SEP_1+1 );

			for ( int i=0; i < color_table.length; i++ )
			{
				int i_row = i / PALETTE_COLS;
				int i_col = i % PALETTE_COLS;

				uchar r = color_table[i].r;
				uchar g = color_table[i].g;
				uchar b = color_table[i].b;

				var palette_menu_item = new ColorMenuItem( i,
				                                           Color.from_byte_rgb(r, g, b),
				                                           dgettext(null, color_table[i].name) );

				palette_menu_item.activated.connect( on_palette_menu_item_activated );

				this.attach( palette_menu_item, i_col, i_col+1, ROW_PALETTE+i_row, ROW_PALETTE+i_row+1 );
			}

			separator_menu_item = new Gtk.SeparatorMenuItem();
			this.attach( separator_menu_item, 0, PALETTE_COLS, ROW_SEP_2, ROW_SEP_2+1 );

			for ( int i=0; i < PALETTE_COLS; i++ )
			{

				history_menu_item[i] = new ColorMenuItem( i, Color.none(), null );
				history_menu_item[i].set_sensitive( false );

				history_menu_item[i].activated.connect( on_history_menu_item_activated );

				this.attach( history_menu_item[i], i, i+1, ROW_HISTORY, ROW_HISTORY+1 );
			}

			separator_menu_item = new Gtk.SeparatorMenuItem();
			this.attach( separator_menu_item, 0, PALETTE_COLS, ROW_SEP_3, ROW_SEP_3+1 );

			custom_menu_item = new Gtk.MenuItem.with_label( _("Custom color") );
			custom_menu_item.activate.connect( on_custom_menu_item_activate );

			this.attach( custom_menu_item, 0, PALETTE_COLS, ROW_CUSTOM, ROW_CUSTOM+1 );

			separator_menu_item = new Gtk.SeparatorMenuItem();
			this.attach( separator_menu_item, 0, PALETTE_COLS, ROW_SEP_4, ROW_SEP_4+1 );

			use_merge_field_menu_item = new Gtk.MenuItem.with_label( _("Use merge field") );
			this.attach( use_merge_field_menu_item, 0, PALETTE_COLS, ROW_FIELD, ROW_FIELD+1 );
			merge_field_menu = new FieldMenu();
			use_merge_field_menu_item.set_submenu( merge_field_menu );
			use_merge_field_menu_item.set_sensitive( false );

			merge_field_menu.key_selected.connect( on_merge_field_menu_key_selected );

			custom_color_history = new ColorHistory( PALETTE_COLS );
			this.map_event.connect( on_map_event );
		}


		public void set_keys( List<string> key_list )
		{
			use_merge_field_menu_item.set_sensitive( true );
			merge_field_menu.set_keys( key_list );
		}


		public void clear_keys()
		{
			use_merge_field_menu_item.set_sensitive( false );
		}


		private void on_default_menu_item_activate()
		{
			color_node.field_flag = false;
			color_node.color      = default_color;
			color_node.key        = null;

			color_changed( color_node, true );
		}


		private void on_palette_menu_item_activated( int id )
		{
			color_node.field_flag = false;
			color_node.color      = Color.from_byte_rgb( color_table[id].r,
			                                             color_table[id].g,
			                                             color_table[id].b );
			color_node.key        = null;

			color_changed( color_node, false );
		}


		private void on_history_menu_item_activated( int id )
		{
			color_node.field_flag = false;
			color_node.color      = custom_color_history.get_color( id );
			color_node.key        = null;

			color_changed( color_node, false );
		}


		private void on_custom_menu_item_activate()
		{
			Gtk.ColorSelectionDialog dialog     = new Gtk.ColorSelectionDialog( _("Custom Color") );
			Gtk.ColorSelection       colorsel   = dialog.get_color_selection() as Gtk.ColorSelection;
			Gdk.Color                gdk_color;
			int                      response;

			gdk_color = color_node.color.to_gdk_color();
			colorsel.set_current_color( gdk_color );

			response = dialog.run();

			switch (response) {
			case Gtk.ResponseType.OK:
				colorsel.get_current_color( out gdk_color );

				color_node.field_flag = false;
				color_node.color      = Color.from_gdk_color( gdk_color );
				color_node.key        = null;

				custom_color_history.add_color( color_node.color );

				color_changed( color_node, false );

				dialog.destroy();
				break;

			default:
				dialog.destroy();
				break;
			}
			
		}


		private void on_merge_field_menu_key_selected( string key )
		{
			color_node.field_flag = true;
			color_node.color      = Color.none();
			color_node.key        = key;

			color_changed( color_node, false );
		}


		private void load_custom_color_history()
		{
			int i;

			for ( i = 0; i < PALETTE_COLS; i++ )
			{
				Color color = custom_color_history.get_color( i );

				if (color.a != 0)
				{
					string tip = _("Custom color #%d").printf( i+1 );

					history_menu_item[i].set_color( i, color, tip );
					history_menu_item[i].set_sensitive( true );
				}
			}

		}


		private bool on_map_event()
		{
			load_custom_color_history();
			return false;
		}

	}

}




