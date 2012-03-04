/*  help.vala
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

	namespace Help
	{

		public void display_contents( Gtk.Window window )
		{
			try {
				Gtk.show_uri( window.get_screen(), "ghelp:glabels-4.0", Gtk.get_current_event_time() );
			}
			catch ( Error e )
			{
				message( "%s", e.message );
			}
		}


		public void display_about_dialog( Gtk.Window window )
		{
			string[] authors = {
				"Jim Evins",
				_("See the file AUTHORS for additional credits,"),
				null
			};

			string[] documenters = {
				"Jim Evins",
				"Mario Blättermann",
				null
			};
        
			string[] artists = {
				"Jim Evins",
				null
			};

			string pixbuf_filename = Path.build_filename( Config.DATADIR, Config.GLABELS_BRANCH, "pixmaps", "glabels-logo.png", null );
			Gdk.Pixbuf pixbuf = new Gdk.Pixbuf.from_file( pixbuf_filename );
			
			Gtk.AboutDialog about = new Gtk.AboutDialog();
			about.title         = _("About glabels");
			about.program_name  = Config.PACKAGE_NAME;
			about.version       = Config.PACKAGE_VERSION;
			about.copyright     = "Copyright \xc2\xa9 2001-2012 Jim Evins";
			about.comments      = _("A label and business card creation program.\n");
			about.website       = "http://glabels.org";
			about.logo          = pixbuf;

			about.authors       = authors;
			about.documenters   = documenters;
			about.artists       = artists;
			about.translator_credits = _("translator-credits");
			about.license            = _(
"""gLabels is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

gLabels is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details."""
				);
        
			about.set_destroy_with_parent( true );

			about.run();
			about.hide();

		}

	}

}

