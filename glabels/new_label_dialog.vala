/*  new_label_dialog.vala
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

	class NewLabelDialog : Gtk.Dialog
	{
		private Prefs prefs;


		private Gtk.IconView  icon_view;

		private Gtk.Entry           search_entry;
		private string              search_string = "";
		private Gtk.TreeModelFilter search_filtered_model;

		private Gtk.ListStore model;


		public string? template_name { get; private set; }


		public NewLabelDialog( Window window )
		{
			prefs = new Prefs();

			this.set_transient_for( window );
			this.set_destroy_with_parent( true );

			this.set_default_size( 788, 600 );


			/* Load the user interface. */
			Gtk.Builder builder = new Gtk.Builder();
			try
			{
				string file = GLib.Path.build_filename( Config.DATADIR, Config.GLABELS_BRANCH, "ui", "new_label_dialog.ui" );
				string[] objects = { "main_vbox", null };
				builder.add_objects_from_file( file, objects );
			}
			catch ( Error err )
			{
				error( "Error: %s", err.message );
			}

			Gtk.VBox main_vbox = builder.get_object( "main_vbox" ) as Gtk.VBox;
			((Gtk.Box)get_content_area()).pack_start( main_vbox );

			search_entry = builder.get_object( "search_entry" ) as Gtk.Entry;
			icon_view    = builder.get_object( "icon_view" )    as Gtk.IconView;


			/* Create and set icon view model. */
			model = new Gtk.ListStore( 3, typeof(string), typeof(Gdk.Pixbuf), typeof(string) );

			search_filtered_model = new Gtk.TreeModelFilter( model, null );
			search_filtered_model.set_visible_func( search_filter_func );

			icon_view.set_model( search_filtered_model );
			icon_view.set_text_column( 0 );
			icon_view.set_pixbuf_column( 1 );
			icon_view.set_tooltip_column( 2 );

			/* Set "follow-state" property of pixbuf renderer. (pre-light) */
			List<weak Gtk.CellRenderer> renderer_list = icon_view.cell_area.get_cells();
			foreach ( Gtk.CellRenderer renderer in renderer_list )
			{
				if ( renderer is Gtk.CellRendererPixbuf )
				{
					((Gtk.CellRendererPixbuf)renderer).follow_state = true;
				}
			}

			/* Intialize model. */
			foreach ( libglabels.Template template in libglabels.Db.templates )
			{
				Gtk.TreeIter iter;
				model.append( out iter );

				string tooltip = build_tooltip( template );

				model.set( iter,
				           0, template.name,
				           1, template.preview_pixbuf,
				           2, tooltip,
				           -1);
			}

			/* Connect signals. */
			search_entry.changed.connect( on_search_entry_changed );
			search_entry.icon_release.connect( on_search_entry_clear );
			icon_view.selection_changed.connect( on_icon_view_selection_changed );
			icon_view.button_release_event.connect( on_icon_view_button_release_event );
		}


		private string build_tooltip( libglabels.Template template )
		{
			libglabels.Units units = prefs.units;

			libglabels.TemplateFrame frame = template.frames.first().data;
			string size_string = frame.get_size_description( units );
			string count_string = frame.get_layout_description();

			string tip = "<span weight=\"bold\">%s:  %s\n</span>%s\n%s".printf(
				template.name, template.description,
				size_string,
				count_string );

			return tip;
		}


		private bool search_filter_func( Gtk.TreeModel model, Gtk.TreeIter iter )
		{
			if ( search_string == "" )
			{
				return true;
			}

			Value value; 
			model.get_value( iter, 0, out value );
			string name = value.get_string();

			string needle   = search_string.casefold();
			string haystack = name.casefold();

			if ( needle in haystack )
			{
				return true;
			}

			return false;
		}


		private void on_search_entry_clear()
		{
			search_entry.set_text( "" );
		}


		private void on_search_entry_changed()
		{
			string new_search_string = search_entry.get_text();

			new_search_string.strip();
			if ( search_string == new_search_string )
			{
				return;
			}
			search_string = new_search_string;


			if ( search_string == "" )
			{
				search_entry.secondary_icon_name        = "edit-find-symbolic";
				search_entry.secondary_icon_activatable = false;
				search_entry.secondary_icon_sensitive   = false;
			}
			else
			{
				search_entry.secondary_icon_name        = "edit-clear-symbolic";
				search_entry.secondary_icon_activatable = true;
				search_entry.secondary_icon_sensitive   = true;
			}

			search_filtered_model.refilter();
		}


		private void on_icon_view_selection_changed()
		{
			List<Gtk.TreePath> list = icon_view.get_selected_items();

			Gtk.TreeIter iter;
			if ( search_filtered_model.get_iter( out iter, list.first().data ) )
			{
				Value value;
				search_filtered_model.get_value( iter, 0, out value );

				template_name = value.get_string();
			}
		}


		private bool on_icon_view_button_release_event( Gdk.EventButton event )
		{
			response( Gtk.ResponseType.OK );
			return true;
		}


	}

}

