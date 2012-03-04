/*  color_menu_item.vala
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

	public class ColorMenuItem : Gtk.MenuItem
	{
		private const int    SIZE = 20;

		public signal void activated( int id );

		private int          id;
		private ColorSwatch  swatch;

		public ColorMenuItem( int     id,
		                      Color   color,
		                      string? tip )
		{
			this.id = id;
			this.swatch = new ColorSwatch( SIZE, SIZE, color );

			this.add( this.swatch );

			this.set_tooltip_text( tip );

			this.activate.connect( on_activate );
		}

		public void set_color( int    id,
		                       Color  color,
		                       string tip )
		{
			this.swatch.set_color( color );
			this.set_tooltip_text( tip );
		}

		private void on_activate()
		{
			activated( id );
		}

	}

}

