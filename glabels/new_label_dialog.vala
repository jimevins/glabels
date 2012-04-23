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
		private Prefs               prefs;
		private TemplateHistory     template_history;

		private Gtk.Notebook        notebook;

		private Gtk.Box             recent_box;
		private Gtk.Box             recent_info_box;
		private MessageBar          recent_info_bar;
		private Gtk.IconView        recent_icon_view;

		private Gtk.Box             search_box;
		private Gtk.Box             search_info_box;
		private MessageBar          search_info_bar;
		private Gtk.Entry           search_entry;
		private Gtk.IconView        search_icon_view;

		private Gtk.Box             custom_box;
		private Gtk.Box             custom_info_box;
		private MessageBar          custom_info_bar;
		private Gtk.IconView        custom_icon_view;
		private Gtk.Button          custom_add_button;
		private Gtk.Button          custom_edit_button;
		private Gtk.Button          custom_delete_button;

		private Gtk.ListStore       recent_model;

		private string              search_string = "";
		private Gtk.TreeModelFilter search_filtered_model;
		private bool                search_filtered_model_empty;
		private Gtk.ListStore       search_model;

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
				string[] objects = { "main_box", null };
				builder.add_objects_from_file( file, objects );
			}
			catch ( Error err )
			{
				error( "Error: %s", err.message );
			}

			Gtk.Box main_box = builder.get_object( "main_box" ) as Gtk.Box;
			((Gtk.Box)get_content_area()).pack_start( main_box );

			notebook             = builder.get_object( "notebook" )             as Gtk.Notebook;

			/* Recent templates widgets */
			recent_box           = builder.get_object( "recent_box" )           as Gtk.Box;
			recent_info_box      = builder.get_object( "recent_info_box" )      as Gtk.Box;
			recent_icon_view     = builder.get_object( "recent_icon_view" )     as Gtk.IconView;

			/* Search widgets */
			search_box           = builder.get_object( "search_box" )           as Gtk.Box;
			search_info_box      = builder.get_object( "search_info_box" )      as Gtk.Box;
			search_entry         = builder.get_object( "search_entry" )         as Gtk.Entry;
			search_icon_view     = builder.get_object( "search_icon_view" )     as Gtk.IconView;

			/* Custom templates widgets */
			custom_box           = builder.get_object( "custom_box" )           as Gtk.Box;
			custom_info_box      = builder.get_object( "custom_info_box" )      as Gtk.Box;
			custom_icon_view     = builder.get_object( "custom_icon_view" )     as Gtk.IconView;
			custom_add_button    = builder.get_object( "custom_add_button" )    as Gtk.Button;
			custom_edit_button   = builder.get_object( "custom_edit_button" )   as Gtk.Button;
			custom_delete_button = builder.get_object( "custom_delete_button" ) as Gtk.Button;


			/* Recent page preparation */
			recent_info_bar = new MessageBar( Gtk.MessageType.INFO,
			                                  Gtk.ButtonsType.NONE,
			                                  _("No recent templates found.") );
			recent_info_bar.format_secondary_text( _("Try selecting a template in the \"Search all\" tab.") );
			recent_info_box.pack_start( recent_info_bar, false, false, 0 );
			recent_info_bar.show();
			recent_info_bar.set_no_show_all( true );
			setup_recent_model();

			/* Search page preparation */
			search_info_bar = new MessageBar( Gtk.MessageType.INFO,
			                                  Gtk.ButtonsType.NONE,
			                                  _("No match.") );
			search_info_bar.format_secondary_text( _("Try modifying the search") );
			search_info_box.pack_start( search_info_bar, false, false, 0 );
			search_info_bar.hide();
			search_info_bar.set_no_show_all( true );
			setup_search_model();

			/* Custom page preparation */
			custom_info_bar = new MessageBar( Gtk.MessageType.INFO,
			                                  Gtk.ButtonsType.NONE,
			                                  _("No custom templates found.") );
			custom_info_bar.format_secondary_text( _("You may create new templates or try searching for pre-defined templates in the \"Search all\" tab.") );
			custom_info_box.pack_start( custom_info_bar, false, false, 0 );
			custom_info_bar.show();

			/* Connect signals. */
			search_entry.changed.connect( on_search_entry_changed );
			search_entry.icon_release.connect( on_search_entry_clear );
			recent_icon_view.selection_changed.connect( on_recent_icon_view_selection_changed );
			recent_icon_view.button_release_event.connect( on_recent_icon_view_button_release_event );
			search_icon_view.selection_changed.connect( on_search_icon_view_selection_changed );
			search_icon_view.button_release_event.connect( on_search_icon_view_button_release_event );

			/* Set initial page. */
			if ( template_history.get_template_list().length() == 0 )
			{
				notebook.set_current_page( notebook.page_num( search_box ) );
			}
			else
			{
				notebook.set_current_page( notebook.page_num( recent_box ) );
			}
		}


		private void setup_recent_model()
		{
			template_history = new TemplateHistory( 5 );

			/* Create and set icon view model. */
			recent_model = new Gtk.ListStore( 3, typeof(string), typeof(Gdk.Pixbuf), typeof(string) );

			recent_icon_view.set_model( recent_model );
			recent_icon_view.set_text_column( 0 );
			recent_icon_view.set_pixbuf_column( 1 );
			recent_icon_view.set_tooltip_column( 2 );

			/* Set "follow-state" property of pixbuf renderer. (pre-light) */
			List<weak Gtk.CellRenderer> renderer_list = recent_icon_view.cell_area.get_cells();
			foreach ( Gtk.CellRenderer renderer in renderer_list )
			{
				if ( renderer is Gtk.CellRendererPixbuf )
				{
					((Gtk.CellRendererPixbuf)renderer).follow_state = true;
				}
			}

			/* Intialize model. */
			foreach ( string name in template_history.get_template_list() )
			{
				libglabels.Template template = libglabels.Db.lookup_template_from_name( name );

				Gtk.TreeIter iter;
				recent_model.append( out iter );

				string tooltip = build_tooltip( template );

				recent_model.set( iter,
				                  0, template.name,
				                  1, template.preview_pixbuf,
				                  2, tooltip,
				                  -1);

				recent_info_bar.hide();
			}

			template_history.changed.connect( on_template_history_changed );
		}


		private void on_template_history_changed()
		{
			recent_model.clear();

			/* Re-intialize model. */
			foreach ( string name in template_history.get_template_list() )
			{
				libglabels.Template template = libglabels.Db.lookup_template_from_name( name );

				Gtk.TreeIter iter;
				recent_model.append( out iter );

				string tooltip = build_tooltip( template );

				recent_model.set( iter,
				                  0, template.name,
				                  1, template.preview_pixbuf,
				                  2, tooltip,
				                  -1);

				recent_info_bar.hide();
			}
		}


		private void setup_search_model()
		{
			/* Create and set icon view model. */
			search_model = new Gtk.ListStore( 3, typeof(string), typeof(Gdk.Pixbuf), typeof(string) );

			search_filtered_model = new Gtk.TreeModelFilter( search_model, null );
			search_filtered_model.set_visible_func( search_filter_func );

			search_icon_view.set_model( search_filtered_model );
			search_icon_view.set_text_column( 0 );
			search_icon_view.set_pixbuf_column( 1 );
			search_icon_view.set_tooltip_column( 2 );

			/* Set "follow-state" property of pixbuf renderer. (pre-light) */
			List<weak Gtk.CellRenderer> renderer_list = search_icon_view.cell_area.get_cells();
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
				search_model.append( out iter );

				string tooltip = build_tooltip( template );

				search_model.set( iter,
				                  0, template.name,
				                  1, template.preview_pixbuf,
				                  2, tooltip,
				                  -1);
			}
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
				search_filtered_model_empty = false;
				return true;
			}

			Value value; 
			model.get_value( iter, 0, out value );
			string name = value.get_string();

			string needle   = search_string.casefold();
			string haystack = name.casefold();

			if ( needle in haystack )
			{
				search_filtered_model_empty = false;
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

			search_filtered_model_empty = true;
			search_filtered_model.refilter();

			if ( search_filtered_model_empty )
			{
				search_info_bar.show();
			}
			else
			{
				search_info_bar.hide();
			}
		}


		private void on_recent_icon_view_selection_changed()
		{
			List<Gtk.TreePath> list = recent_icon_view.get_selected_items();

			Gtk.TreeIter iter;
			if ( recent_model.get_iter( out iter, list.first().data ) )
			{
				Value value;
				recent_model.get_value( iter, 0, out value );

				template_name = value.get_string();
			}
		}


		private bool on_recent_icon_view_button_release_event( Gdk.EventButton event )
		{
			if ( template_name != null )
			{
				response( Gtk.ResponseType.OK );
				template_history.changed.disconnect( on_template_history_changed );
			}
			return true;
		}


		private void on_search_icon_view_selection_changed()
		{
			List<Gtk.TreePath> list = search_icon_view.get_selected_items();

			Gtk.TreeIter iter;
			if ( search_filtered_model.get_iter( out iter, list.first().data ) )
			{
				Value value;
				search_filtered_model.get_value( iter, 0, out value );

				template_name = value.get_string();
			}
		}


		private bool on_search_icon_view_button_release_event( Gdk.EventButton event )
		{
			if ( template_name != null )
			{
				response( Gtk.ResponseType.OK );
			}
			return true;
		}


	}

}

