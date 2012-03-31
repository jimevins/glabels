/*  property_editor.vala
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
using libglabels;

namespace glabels
{

	class PagePreview : Gtk.Box
	{
		private Label           label;

		private Gtk.Box         title_box;
		private Gtk.Box         layout_preview_box;
		private MiniPreview     layout_preview;


		public PagePreview()
		{
			/* Load the user interface. */
			Gtk.Builder builder = new Gtk.Builder();
			try
			{
				string file = GLib.Path.build_filename( Config.DATADIR, Config.GLABELS_BRANCH, "ui", "page_preview.ui" );
				string[] objects = { "property_editor_vbox",
				                     null };
				builder.add_objects_from_file( file, objects );
			}
			catch ( Error err )
			{
				error( "Error: %s", err.message );
			}

			Gtk.VBox property_editor_vbox = builder.get_object( "property_editor_vbox" ) as Gtk.VBox;
			pack_start( property_editor_vbox );

			/* Title widgets. */
			title_box = builder.get_object( "title_box" ) as Gtk.Box;
			title_box.set_sensitive( false );

			/* Layout widgets. */
			layout_preview_box = builder.get_object( "layout_preview_box" ) as Gtk.Box;

			layout_preview = new MiniPreview( 280, 420 );
			layout_preview.show_all();
			layout_preview_box.pack_start( layout_preview, true, true, 0 );
		}


		public void set_label( Label label )
		{
			this.label = label;

			title_box.set_sensitive( true );
			layout_preview.set_label( label );
		}


	}

}

