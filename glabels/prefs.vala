/*  prefs.vala
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
using libglabels;

namespace glabels
{

	public class Prefs : Object
	{
		private const string DEFAULT_UNITS_STRING_US     = "in";
		private const string DEFAULT_PAGE_SIZE_US        = "US-Letter";

		private const string DEFAULT_UNITS_STRING_METRIC = "mm";
		private const string DEFAULT_PAGE_SIZE_METRIC    = "A4";


		public signal void changed();


		private static GLib.Settings locale;
		private static GLib.Settings objects;
		private static GLib.Settings ui;


		static construct
		{
			locale  = new GLib.Settings( "org.gnome.glabels-3.locale" );
			objects = new GLib.Settings( "org.gnome.glabels-3.objects" );
			ui      = new GLib.Settings( "org.gnome.glabels-3.ui" );
		}

		public Prefs()
		{
			locale.changed.connect( on_changed );
			objects.changed.connect( on_changed );
			ui.changed.connect( on_changed );
		}


		private void on_changed()
		{
			changed();
		}


		public Units units
		{
			get
			{
				string? units_string = locale.get_string( "units" );

				/* If not set, make educated guess about locale default. */
				if ( (units_string == null) || (units_string == "") )
				{
					string pgsize = Gtk.PaperSize.get_default();
					if ( pgsize == Gtk.PAPER_NAME_LETTER )
					{
						units_string = DEFAULT_UNITS_STRING_US;
					}
					else
					{
						units_string = DEFAULT_UNITS_STRING_METRIC;
					}
				}

				_units = Units.from_id( units_string );

				/* If invalid, make an educated guess from locale. */
				if ( _units.id == "pt" )
				{
					string pgsize = Gtk.PaperSize.get_default();
					if ( pgsize == Gtk.PAPER_NAME_LETTER )
					{
						_units = Units.inch();
					}
					else
					{
						_units = Units.mm();
					}
				}

				return _units;
			}

			set
			{
				locale.set_string( "units", value.id );
			}
		}
		private Units _units;


		public string default_page_size
		{
			get
			{
				_default_page_size = locale.get_string( "default-page-size" );

				/* If not set, make educated guess about locale default. */
				/* Also proof read the default page size -- it must be a valid id. */
				/* (For compatability with older versions.) */
				if ( (_default_page_size == null) ||
				     (_default_page_size == "")   ||
				     ( Db.lookup_paper_from_id( _default_page_size ) == null ) )
				{
					string pgsize = Gtk.PaperSize.get_default();
					if ( pgsize == Gtk.PAPER_NAME_LETTER )
					{
						_default_page_size = DEFAULT_PAGE_SIZE_US;
					}
					else
					{
						_default_page_size = DEFAULT_PAGE_SIZE_METRIC;
					}
				}

				return _default_page_size;
			}

			set
			{
				locale.set_string( "default-page-size", value );
			}
		}
		private string? _default_page_size;


		public string default_font_family
		{
			get
			{
				_default_font_family = objects.get_string( "default-font-family" );
				return _default_font_family;
			}

			set
			{
				objects.set_string( "default-font-family", value );
			}
		}
		private string? _default_font_family;


		public double default_font_size
		{
			get
			{
				return objects.get_double( "default-font-size" );
			}

			set
			{
				objects.set_double( "default-font-size", value );
			}
		}


		public Pango.Weight default_font_weight
		{
			get
			{
				return EnumUtil.string_to_weight( objects.get_string( "default-font-weight" ) );
			}

			set
			{
				objects.set_string( "default-font-weight", EnumUtil.weight_to_string( value ) );
			}
		}


		public bool default_font_italic_flag
		{
			get
			{
				return objects.get_boolean( "default-font-italic-flag" );
			}

			set
			{
				objects.set_boolean( "default-font-italic-flag", value );
			}
		}


		public Color default_text_color
		{
			get
			{
				return Color.from_legacy_color( objects.get_uint( "default-text-color" ) );
			}

			set
			{
				objects.set_uint( "default-text-color", value.to_legacy_color() );
			}
		}


		public Pango.Alignment default_text_alignment
		{
			get
			{
				return EnumUtil.string_to_align( objects.get_string( "default-text-alignment" ) );
			}

			set
			{
				objects.set_string( "default-text-alignment", EnumUtil.align_to_string( value ) );
			}
		}


		public double default_text_line_spacing
		{
			get
			{
				return objects.get_double( "default-text-line-spacing" );
			}

			set
			{
				objects.set_double( "default-text-line-spacing", value );
			}
		}


		public double default_line_width
		{
			get
			{
				return objects.get_double( "default-line-width" );
			}

			set
			{
				objects.set_double( "default-line-width", value );
			}
		}


		public Color default_line_color
		{
			get
			{
				return Color.from_legacy_color( objects.get_uint( "default-line-color" ) );
			}

			set
			{
				objects.set_uint( "default-line-color", value.to_legacy_color() );
			}
		}


		public Color default_fill_color
		{
			get
			{
				return Color.from_legacy_color( objects.get_uint( "default-fill-color" ) );
			}

			set
			{
				objects.set_uint( "default-fill-color", value.to_legacy_color() );
			}
		}


		public bool main_toolbar_visible
		{
			get
			{
				return ui.get_boolean( "main-toolbar-visible" );
			}

			set
			{
				ui.set_boolean( "main-toolbar-visible", value );
			}
		}


		public bool drawing_toolbar_visible
		{
			get
			{
				return ui.get_boolean( "drawing-toolbar-visible" );
			}

			set
			{
				ui.set_boolean( "drawing-toolbar-visible", value );
			}
		}


		public bool property_toolbar_visible
		{
			get
			{
				return ui.get_boolean( "property-toolbar-visible" );
			}

			set
			{
				ui.set_boolean( "property-toolbar-visible", value );
			}
		}


		public bool grid_visible
		{
			get
			{
				return ui.get_boolean( "grid-visible" );
			}

			set
			{
				ui.set_boolean( "grid-visible", value );
			}
		}


		public bool markup_visible
		{
			get
			{
				return ui.get_boolean( "markup-visible" );
			}

			set
			{
				ui.set_boolean( "markup-visible", value );
			}
		}


		public int max_recents
		{
			get
			{
				return ui.get_int( "max-recents" );
			}

			set
			{
				ui.set_int( "max-recents", value );
			}
		}


	}

}
