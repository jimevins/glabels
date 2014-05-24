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
			string color_spec;
			string name;
		}

		private const ColorTableEntry color_table[] = {

			{ "#ef2929", NC_("Color name", "Light Scarlet Red") },
			{ "#fcaf3e", NC_("Color name", "Light Orange")      },
			{ "#fce94f", NC_("Color name", "Light Butter")      },
			{ "#8ae234", NC_("Color name", "Light Chameleon")   },
			{ "#729fcf", NC_("Color name", "Light Sky Blue")    },
			{ "#ad7fa8", NC_("Color name", "Light Plum")        },
			{ "#e9b96e", NC_("Color name", "Light Chocolate")   },
			{ "#888a85", NC_("Color name", "Light Aluminum 1")  },
			{ "#eeeeec", NC_("Color name", "Light Aluminum 2")  },

			{ "#cc0000", NC_("Color name", "Scarlet Red")       },
			{ "#f57900", NC_("Color name", "Orange")            },
			{ "#edd400", NC_("Color name", "Butter")            },
			{ "#73d216", NC_("Color name", "Chameleon")         },
			{ "#3465a4", NC_("Color name", "Sky Blue")          },
			{ "#75507b", NC_("Color name", "Plum")              },
			{ "#c17d11", NC_("Color name", "Chocolate")         },
			{ "#555753", NC_("Color name", "Aluminum 1")        },
			{ "#d3d7cf", NC_("Color name", "Aluminum 2")        },

			{ "#a40000", NC_("Color name", "Dark Scarlet Red")  },
			{ "#ce5c00", NC_("Color name", "Dark Orange")       },
			{ "#c4a000", NC_("Color name", "Dark Butter")       },
			{ "#4e9a06", NC_("Color name", "Dark Chameleon")    },
			{ "#204a87", NC_("Color name", "Dark Sky Blue")     },
			{ "#5c3566", NC_("Color name", "Dark Plum")         },
			{ "#8f5902", NC_("Color name", "Dark Chocolate")    },
			{ "#2e3436", NC_("Color name", "Dark Aluminum 1")   },
			{ "#babdb6", NC_("Color name", "Dark Aluminum 2")   },

			{ "#000000", NC_("Color name", "Black")             },
			{ "#2e3436", NC_("Color name", "Very Dark Gray")    },
			{ "#555753", NC_("Color name", "Darker Gray")       },
			{ "#888a85", NC_("Color name", "Dark Gray")         },
			{ "#babdb6", NC_("Color name", "Medium Gray")       },
			{ "#d3d7cf", NC_("Color name", "Light Gray")        },
			{ "#eeeeec", NC_("Color name", "Lighter Gray")      },
			{ "#f3f3f3", NC_("Color name", "Very Light Gray")   },
			{ "#ffffff", NC_("Color name", "White")             }
		};

		private const int      PALETTE_COLS = 9;
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

				var palette_menu_item = new ColorMenuItem( i,
				                                           Color.from_spec(color_table[i].color_spec),
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
			color_node.color      = Color.from_spec( color_table[id].color_spec );
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
			Gtk.ColorChooserDialog dialog  = new Gtk.ColorChooserDialog( _("Custom Color"), null );

			dialog.use_alpha   = true;
			dialog.show_editor = true; // TODO: This gets overwritten when dialog is realized.
			                           //       I would like to skip over the redundant pallete.

			Gdk.RGBA gdk_rgba = color_node.color.to_gdk_rgba();
			dialog.set_rgba( gdk_rgba );

			int response = dialog.run();

			switch (response) {
			case Gtk.ResponseType.OK:
				gdk_rgba = dialog.get_rgba();

				color_node.field_flag = false;
				color_node.color      = Color.from_gdk_rgba( gdk_rgba );
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




