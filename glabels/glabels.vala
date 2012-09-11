/*  glabels.vala
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

	class Main
	{

		private static string[] filenames;
		private const OptionEntry[] option_entries = {
			{ "", 0, 0, OptionArg.FILENAME_ARRAY, ref filenames, null, N_("[FILE...]") },
			{ null }
		};


		internal static int main( string[] args )
		{
			OptionContext option_context = new OptionContext("");
			option_context.set_summary( _("Launch gLabels label and business card designer.") );
			option_context.add_main_entries( option_entries, Config.GETTEXT_PACKAGE );

			Gtk.init( ref args );

			try
			{
				option_context.parse( ref args );
			}
			catch ( OptionError e )
			{
				stderr.printf( _("%s\nRun '%s --help' to see a full list of available command line options.\n"),
				               e.message, args[0] );
				return 0;
			}

			libglabels.Db.init();
			libglabels.XmlUtil.init();
			libglabels.XmlUtil.default_units = libglabels.Units.inch();
			glbarcode.Factory.init();
			BarcodeBackends.init();

			Gtk.IconTheme icon_theme = Gtk.IconTheme.get_default();
			icon_theme.append_search_path( Path.build_filename( Config.DATADIR, Config.GLABELS_BRANCH, "icons", null ) );


			if ( filenames != null )
			{
				for ( int i = 0; filenames[i] != null; i++ )
				{
					try
					{
						Label label = XmlLabel.open_file( filenames[i] );
						Window window = new Window.from_label( label );
						window.show_all();
					}
					catch ( XmlLabel.XmlError e )
					{
						message( "Error opening file: %s", e.message );
					}
				}
			}
			else
			{
				Window window = new Window();
				window.show_all();
			}

			Gtk.main();

			return 0;
		}

	}

}


