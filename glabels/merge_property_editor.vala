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
		private const int RECORD_FIELD_COLUMN = 1;
		private const int VALUE_COLUMN        = 2;
		private const int IS_RECORD_COLUMN    = 3;
		private const int DATA_COLUMN         = 4;


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
		private Gtk.TreeStore         tree_store;


		/* Signal IDs */
		private ulong sigid_type_combo_changed;
		private ulong sigid_location_file_button_selection_changed;


		public MergePropertyEditor()
		{
			/* Load the user interface. */
			Gtk.Builder builder = new Gtk.Builder();
			try
			{
				string file = GLib.Path.build_filename( Config.DATADIR, Config.GLABELS_BRANCH,
				                                        "ui", "merge_property_editor.ui" );
				string[] objects = { "merge_property_editor_box",
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

			tree_store   = new Gtk.TreeStore( 5,
			                                  /* 0 : SELECT_COLUMN */       typeof(bool),
			                                  /* 1 : RECORD_FIELD_COLUMN */ typeof(string),
			                                  /* 2 : VALUE_COLUMN */        typeof(string),
			                                  /* 3 : IS_RECORD_COLUMN */    typeof(bool),
			                                  /* 4 : DATA_COLUMN */         typeof(void*) );
			records_treeview.set_model( tree_store );

			Gtk.CellRendererToggle record_select_renderer = new Gtk.CellRendererToggle();
			Gtk.TreeViewColumn     record_select_column   = new Gtk.TreeViewColumn.with_attributes( _("Select"),
				                  record_select_renderer,
				                  "active", 0, "visible", 3, null );
			records_treeview.append_column( record_select_column );

			Gtk.CellRendererText   record_field_renderer = new Gtk.CellRendererText();
			Gtk.TreeViewColumn     record_field_column   = new Gtk.TreeViewColumn.with_attributes( _("Record/Field"),
				                 record_field_renderer,
				                 "text", 1, null );
			records_treeview.append_column( record_field_column );

			Gtk.CellRendererText   field_value_renderer = new Gtk.CellRendererText();
			Gtk.TreeViewColumn     field_value_column   = new Gtk.TreeViewColumn.with_attributes( _("Value"),
				                field_value_renderer,
				                "text", 2, null );
			records_treeview.append_column( field_value_column );

			record_field_renderer.yalign = 0;
			field_value_renderer.yalign  = 0;

			record_field_column.set_sizing( Gtk.TreeViewColumnSizing.AUTOSIZE );
			field_value_column.set_sizing( Gtk.TreeViewColumnSizing.AUTOSIZE );

			records_treeview.set_expander_column( record_field_column );

			sigid_type_combo_changed =
				type_combo.changed.connect( on_type_combo_changed );
			sigid_location_file_button_selection_changed =
				location_file_button.file_set.connect( on_location_file_button_selection_changed );
			record_select_renderer.toggled.connect( on_record_select_toggled );
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

				if ( model.label.merge.src != null )
				{
					GLib.SignalHandler.block( (void *)location_file_button, sigid_location_file_button_selection_changed );
					location_file_button.set_filename( model.label.merge.src );
					GLib.SignalHandler.unblock( (void *)location_file_button, sigid_location_file_button_selection_changed );
				}
				break;
			default:
				assert_not_reached();
			}

			load_tree( tree_store, model.label.merge );
		}


		private void load_tree( Gtk.TreeStore store, Merge merge )
		{
			store.clear();

			string primary_key = merge.get_primary_key();

			foreach ( MergeRecord record in merge.record_list )
			{
				string primary_value = record.eval_key( primary_key );

				Gtk.TreeIter iter1;
				store.append( out iter1, null );
				store.set( iter1,
				           SELECT_COLUMN,       record.selected,
				           RECORD_FIELD_COLUMN, primary_value,
				           IS_RECORD_COLUMN,    true,
				           DATA_COLUMN,         record );

				foreach ( MergeField field in record.field_list )
				{
					Gtk.TreeIter iter2;
					store.append( out iter2, iter1 );
					store.set( iter2,
					           RECORD_FIELD_COLUMN, field.key,
					           VALUE_COLUMN,        field.value,
					           IS_RECORD_COLUMN,    false );
				}
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
			tree_store.get_iter( out iter, path );

			/* get current data */
			unowned MergeRecord record;
			tree_store.get( iter, DATA_COLUMN, out record );

			/* toggle the select flag within the record */
			record.selected = !record.selected;

			/* set new value in store */
			tree_store.set( iter, SELECT_COLUMN, record.selected );
		}


		private void on_select_all_button_clicked()
		{
			Gtk.TreeIter iter;

			for ( bool good = tree_store.get_iter_first( out iter );
			      good;
			      good = tree_store.iter_next( ref iter ) )
			{
				/* get current data */
				unowned MergeRecord record;
				tree_store.get( iter, DATA_COLUMN, out record );

				/* set select flag within the record */
				record.selected = true;

				/* set new value in store */
				tree_store.set( iter, SELECT_COLUMN, record.selected );
			}
		}


		private void on_unselect_all_button_clicked()
		{
			Gtk.TreeIter iter;

			for ( bool good = tree_store.get_iter_first( out iter );
			      good;
			      good = tree_store.iter_next( ref iter ) )
			{
				/* get current data */
				unowned MergeRecord record;
				tree_store.get( iter, DATA_COLUMN, out record );

				/* clear select flag within the record */
				record.selected = false;

				/* set new value in store */
				tree_store.set( iter, SELECT_COLUMN, record.selected );
			}
		}


	}

}

