/*  merge_property_editor.vala
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

	public class MergePropertyEditor : Gtk.Box
	{
		private const int SELECT_COLUMN       = 0;
		private const int DATA_COLUMN         = 1;
		private const int FIRST_FIELD_COLUMN  = 2;

		private Gee.HashMap<string,int> column_map;


		private Model model;


		/* Widgets */
		private Gtk.ComboBoxText      type_combo;
		private Gtk.Label             location_na_label;
		private Gtk.Label             location_fixed_label;
		private Gtk.FileChooserButton location_file_button;
		private Gtk.TreeView          records_treeview;
		private Gtk.Button            select_all_button;
		private Gtk.Button            unselect_all_button;

		/* Treeview Model */
		private Gtk.ListStore         list_store;


		/* Signal IDs */
		private ulong sigid_type_combo_changed;
		private ulong sigid_location_file_button_selection_changed;


		public MergePropertyEditor()
		{
			column_map = new Gee.HashMap<string,int>();

			/* Load the user interface. */
			Gtk.Builder builder = new Gtk.Builder();
			try
			{
				string file = GLib.Path.build_filename( Config.DATADIR, Config.GLABELS_BRANCH,
				                                        "ui", "merge_property_editor.ui" );
				string[] objects = { "merge_property_editor_box",
				                     "source_sizegroup",
				                     null };
				builder.add_objects_from_file( file, objects );
			}
			catch ( Error err )
			{
				error( "Error: %s", err.message );
			}

			Gtk.Box merge_property_editor_box = builder.get_object( "merge_property_editor_box" ) as Gtk.Box;
			pack_start( merge_property_editor_box );

			type_combo           = builder.get_object( "type_combo" )           as Gtk.ComboBoxText;
			location_na_label    = builder.get_object( "location_na_label" )    as Gtk.Label;
			location_fixed_label = builder.get_object( "location_fixed_label" ) as Gtk.Label;
			location_file_button = builder.get_object( "location_file_button" ) as Gtk.FileChooserButton;
			records_treeview     = builder.get_object( "records_treeview" )     as Gtk.TreeView;
			select_all_button    = builder.get_object( "select_all_button" )    as Gtk.Button;
			unselect_all_button  = builder.get_object( "unselect_all_button" )  as Gtk.Button;

			foreach ( string name in MergeBackends.get_name_list() )
			{
				type_combo.append_text( name );
			}

			sigid_type_combo_changed =
				type_combo.changed.connect( on_type_combo_changed );
			sigid_location_file_button_selection_changed =
				location_file_button.file_set.connect( on_location_file_button_selection_changed );
			select_all_button.clicked.connect( on_select_all_button_clicked );
			unselect_all_button.clicked.connect( on_unselect_all_button_clicked );
		}


		public void set_model( Model model )
		{
			this.model = model;

			on_merge_changed();
			model.label.merge_changed.connect( on_merge_changed );
		}


		private void on_merge_changed()
		{
			GLib.SignalHandler.block( (void *)type_combo, sigid_type_combo_changed );
			ComboUtil.set_active_text( type_combo, model.label.merge.info.name );
			GLib.SignalHandler.unblock( (void *)type_combo, sigid_type_combo_changed );

			switch ( model.label.merge.info.src_type )
			{
			case MergeSrcType.NONE:
				location_na_label.show();
				location_fixed_label.hide();
				location_file_button.hide();
				break;
			case MergeSrcType.FIXED:
				location_na_label.hide();
				location_fixed_label.show();
				location_file_button.hide();
				break;
			case MergeSrcType.FILE:
				location_na_label.hide();
				location_fixed_label.hide();
				location_file_button.show();

				GLib.SignalHandler.block( (void *)location_file_button, sigid_location_file_button_selection_changed );
				if ( model.label.merge.src != null )
				{
					location_file_button.set_filename( model.label.merge.src );
				}
				else
				{
					location_file_button.unselect_all();
				}
				GLib.SignalHandler.unblock( (void *)location_file_button, sigid_location_file_button_selection_changed );
				break;
			default:
				assert_not_reached();
			}

			load_new_column_set( model.label.merge );
			load_tree( model.label.merge );

			model.label.merge.source_changed.connect( on_merge_source_changed );
			model.label.merge.selection_changed.connect( on_merge_selection_changed );
		}


		private void on_merge_source_changed()
		{
			load_new_column_set( model.label.merge );
			load_tree( model.label.merge );
		}


		private void on_merge_selection_changed()
		{
			load_selected_column();
		}


		private void load_new_column_set( Merge merge )
		{
			/* Remove old columns from view. */
			foreach ( Gtk.TreeViewColumn column in records_treeview.get_columns() )
			{
				records_treeview.remove_column( column );
			}

			/* Initialize types array with fixed column types */
			Type[] types = { typeof(bool), typeof(void*) };


			/* Append field column types.  Build key to column map. */
			column_map.clear();
			int key_index = FIRST_FIELD_COLUMN;

			/* Make primary key first field column */
			string primary_key = merge.get_primary_key();
			types += typeof(string);
			column_map.set( primary_key, key_index++ );

			/* Now the rest. */
			foreach ( string key in merge.get_key_list() )
			{
				if ( key != primary_key )
				{
					types += typeof(string);
					column_map.set( key, key_index++ );
				}
			}


			/* Create new model for these types. */
			list_store = new Gtk.ListStore.newv( types );
			records_treeview.set_model( list_store );


			/* Add Select column to view. */
			Gtk.CellRendererToggle record_select_renderer = new Gtk.CellRendererToggle();
			Gtk.TreeViewColumn     record_select_column   = new Gtk.TreeViewColumn.with_attributes( "",
				                  record_select_renderer,
				                  "active", 0, null );
			records_treeview.append_column( record_select_column );

			record_select_renderer.toggled.connect( on_record_select_toggled );


			/* Add Field value columns to view. */
			foreach ( string key in merge.get_key_list() )
			{
				Gtk.CellRendererText field_value_renderer = new Gtk.CellRendererText();
				field_value_renderer.yalign  = 0;
				Gtk.TreeViewColumn field_value_column = new Gtk.TreeViewColumn.with_attributes( "${%s}".printf(key),
					          field_value_renderer,
					          "text", column_map.get(key), null );
				field_value_column.set_sizing( Gtk.TreeViewColumnSizing.AUTOSIZE );
				field_value_column.set_alignment( 0.5f );

				records_treeview.append_column( field_value_column );
			}

			/* Add an empty column to view. (Bounds last column for a better visual appearance.) */
			Gtk.TreeViewColumn empty_column = new Gtk.TreeViewColumn();
			records_treeview.append_column( empty_column );

		}


		private void load_tree( Merge merge )
		{
			list_store.clear();

			bool empty_record_list = true;

			foreach ( MergeRecord record in merge.record_list )
			{
				empty_record_list = false;

				Gtk.TreeIter iter;
				list_store.append( out iter );

				list_store.set( iter,
				                SELECT_COLUMN, record.selected,
				                DATA_COLUMN,   record );

				foreach ( MergeField field in record.field_list )
				{
					list_store.set( iter,
					                column_map.get(field.key), field.value );
				}
			}

			select_all_button.set_sensitive( !empty_record_list );
			unselect_all_button.set_sensitive( !empty_record_list );
		}


		private void load_selected_column()
		{
			Gtk.TreeIter iter;

			for ( bool good = list_store.get_iter_first( out iter );
			      good;
			      good = list_store.iter_next( ref iter ) )
			{
				/* get record */
				unowned MergeRecord record;
				list_store.get( iter, DATA_COLUMN, out record );

				/* set selected value in store accordingly */
				list_store.set( iter, SELECT_COLUMN, record.selected );
			}
		}


		private void on_type_combo_changed()
		{
			string name = type_combo.get_active_text();
			string id = MergeBackends.name_to_id( name );
			model.label.merge = MergeBackends.create_merge( id );
		}


		private void on_location_file_button_selection_changed()
		{
			model.label.merge.src = location_file_button.get_filename();
		}


		private void on_record_select_toggled( string path_str )
		{
			/* get toggled iter */
			Gtk.TreePath path = new Gtk.TreePath.from_string( path_str );
			Gtk.TreeIter iter;
			list_store.get_iter( out iter, path );

			/* get current data */
			unowned MergeRecord record;
			list_store.get( iter, DATA_COLUMN, out record );

			/* toggle the select flag within the record */
			if ( record.selected )
			{
				model.label.merge.unselect( record );
			}
			else
			{
				model.label.merge.select( record );
			}
		}


		private void on_select_all_button_clicked()
		{
			model.label.merge.select_all();
		}


		private void on_unselect_all_button_clicked()
		{
			model.label.merge.unselect_all();
		}


	}

}

