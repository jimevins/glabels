/*  font_menu_item.vala
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

	public class FontMenuItem : Gtk.MenuItem
	{
		public signal void activated( string family );

		private const int    SAMPLE_W = 32;
		private const int    SAMPLE_H = 24;

		/* Translators: very short sample text, used in building font menu item icons */
		private const string short_sample_text    = _("Aa");
		/* Translators: lower case sample text */
		private const string lower_case_text      = _("abcdefghijklmnopqrstuvwxyz");
		/* Translators: upper case sample text */
		private const string upper_case_text      = _("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
		/* Translators: numbers and special characters sample text */
		private const string numbers_special_text = _("0123456789 .:,;(*!?)");

		private string       font_family;

		public FontMenuItem( string font_family )
		{
			this.font_family = font_family;

			Gtk.HBox hbox = new Gtk.HBox( false, 6 );
			this.add( hbox );

			FontSample sample = new FontSample( SAMPLE_W, SAMPLE_H, short_sample_text, font_family );
			hbox.pack_start( sample, false, false, 0 );

			Gtk.Label label = new Gtk.Label( font_family );
			hbox.pack_start( label, false, false, 0 );

			unowned Pango.Language language = Pango.Language.get_default();
			string sample_text = language.get_sample_string();

			string tip = "<span font_family=\"%s\" size=\"x-large\" weight=\"bold\">%s\n</span>%s:\n\n<span font_family=\"%s\" size=\"large\">%s\n%s\n%s\n\n%s</span>".printf(
				font_family, font_family,
				_("Sample text"),
				font_family,
				lower_case_text,
				upper_case_text,
				numbers_special_text,
				sample_text );

			this.set_tooltip_markup( tip );

			this.activate.connect( on_menu_item_activate );
		}


		private void on_menu_item_activate()
		{
			activated( font_family );
		}


	}

}

