/*  print_setup_editor.vala
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

	public class PrintSetupEditor : Gtk.Box
	{

		private const int DEFAULT_W = 280;
		private const int DEFAULT_H = 420;


		private Model model;


		/* Widgets */
		private Gtk.Box         preview_box;
		private MiniPreview     preview;

		private Gtk.Frame       copies_frame;
		private Gtk.RadioButton copies_sheets_radio;
		private Gtk.SpinButton  copies_sheets_spin;
		private Gtk.RadioButton copies_labels_radio;
		private Gtk.SpinButton  copies_labels_start_spin;
		private Gtk.SpinButton  copies_labels_end_spin;

		private Gtk.Frame       merge_frame;
		private Gtk.SpinButton  merge_start_spin;
		private Gtk.SpinButton  merge_copies_spin;
		private Gtk.CheckButton merge_collate_check;
		private Gtk.Image       merge_collate_image;

		private Gtk.CheckButton print_outlines_check;
		private Gtk.CheckButton print_reverse_check;
		private Gtk.CheckButton print_crop_marks_check;


		/* Signal IDs */
		private ulong sigid_print_outlines_check_toggled;
		private ulong sigid_print_reverse_check_toggled;
		private ulong sigid_print_crop_marks_check_toggled;


		public PrintSetupEditor()
		{
			/* Load the user interface. */
			Gtk.Builder builder = new Gtk.Builder();
			try
			{
				string file = GLib.Path.build_filename( Config.DATADIR, Config.GLABELS_BRANCH,
				                                        "ui", "print_setup_editor.ui" );
				string[] objects = { "print_setup_editor_box",
				                     null };
				builder.add_objects_from_file( file, objects );
			}
			catch ( Error err )
			{
				error( "Error: %s", err.message );
			}


			Gtk.Box print_setup_editor_box = builder.get_object( "print_setup_editor_box" ) as Gtk.Box;
			pack_start( print_setup_editor_box );

			preview_box              = builder.get_object( "preview_box" )              as Gtk.Box;

			preview = new MiniPreview( DEFAULT_W, DEFAULT_H );
			preview_box.pack_start( preview, true, true, 0 );


			copies_frame             = builder.get_object( "copies_frame" )             as Gtk.Frame;
			copies_sheets_radio      = builder.get_object( "copies_sheets_radio" )      as Gtk.RadioButton;
			copies_sheets_spin       = builder.get_object( "copies_sheets_spin" )       as Gtk.SpinButton;
			copies_labels_radio      = builder.get_object( "copies_labels_radio" )      as Gtk.RadioButton;
			copies_labels_start_spin = builder.get_object( "copies_labels_start_spin" ) as Gtk.SpinButton;
			copies_labels_end_spin   = builder.get_object( "copies_labels_end_spin" )   as Gtk.SpinButton;


			merge_frame              = builder.get_object( "merge_frame" )              as Gtk.Frame;
			merge_start_spin         = builder.get_object( "merge_start_spin" )         as Gtk.SpinButton;
			merge_copies_spin        = builder.get_object( "merge_copies_spin" )        as Gtk.SpinButton;
			merge_collate_check      = builder.get_object( "merge_collate_check" )      as Gtk.CheckButton;
			merge_collate_image      = builder.get_object( "merge_collate_image" )      as Gtk.Image;

			print_outlines_check     = builder.get_object( "print_outlines_check" )     as Gtk.CheckButton;
			print_reverse_check      = builder.get_object( "print_reverse_check" )      as Gtk.CheckButton;
			print_crop_marks_check   = builder.get_object( "print_crop_marks_check" )   as Gtk.CheckButton;

			sigid_print_outlines_check_toggled =
				print_outlines_check.toggled.connect( on_print_outlines_check_toggled );
			sigid_print_reverse_check_toggled =
				print_reverse_check.toggled.connect( on_print_reverse_check_toggled );
			sigid_print_crop_marks_check_toggled =
				print_crop_marks_check.toggled.connect( on_print_crop_marks_check_toggled );
		}


		public void set_model( Model model )
		{
			this.model = model;

			preview.set_model( model );

			load_copies_frame();
			load_merge_frame();
			load_print_options();

			model.label.merge_changed.connect( on_merge_changed );

			on_merge_changed();
		}


		private void on_merge_changed()
		{
			bool no_merge = model.label.merge is MergeNone;

			if ( no_merge )
			{
				copies_frame.show();
				merge_frame.hide();
			}
			else
			{
				copies_frame.hide();
				merge_frame.show();
			}
		}


		private void load_merge_frame()
		{
		}


		private void load_print_options()
		{
			load_print_outlines_check();
			load_print_reverse_check();
			load_print_crop_marks_check();
		}


		/***************************
		 * copies_* widgets
		 ***************************/
		private void on_copies_sheets_radio_toggled()
		{
			if ( copies_sheets_radio.get_active() )
			{
				TemplateFrame frame = model.label.template.frames.first().data;

				model.print.simple_n_sheets = copies_sheets_spin.get_value_as_int();
				model.print.simple_start    = 1;
				model.print.simple_end      = frame.get_n_labels();

				copies_sheets_spin.set_sensitive( true );
				copies_labels_start_spin.set_sensitive( false );
				copies_labels_end_spin.set_sensitive( false );
			}
		}
		

		private void on_copies_labels_radio_toggled()
		{
			if ( copies_labels_radio.get_active() )
			{
				model.print.simple_n_sheets = 1;
				model.print.simple_start    = copies_labels_start_spin.get_value_as_int();
				model.print.simple_end      = copies_labels_end_spin.get_value_as_int();

				copies_sheets_spin.set_sensitive( false );
				copies_labels_start_spin.set_sensitive( true );
				copies_labels_end_spin.set_sensitive( true );
			}
		}


		private void on_copies_sheets_spin_changed()
		{
			model.print.simple_n_sheets = copies_sheets_spin.get_value_as_int();
		}
		

		private void on_copies_labels_start_spin_changed()
		{
			model.print.simple_start    = copies_labels_start_spin.get_value_as_int();
		}
		

		private void on_copies_labels_end_spin_changed()
		{
			model.print.simple_end      = copies_labels_end_spin.get_value_as_int();
		}
		

		private void load_copies_frame()
		{
			bool no_merge = model.label.merge is MergeNone;

			if ( no_merge )
			{
				
			}
		}


		/***************************
		 * print_outlines_check
		 ***************************/
		private void on_print_outlines_check_toggled()
		{
			model.print.outline_flag = print_outlines_check.get_active();
		}

		private void load_print_outlines_check()
		{
			GLib.SignalHandler.block( (void*)print_outlines_check, sigid_print_outlines_check_toggled );

			print_outlines_check.set_active( model.print.outline_flag );

			GLib.SignalHandler.unblock( (void*)print_outlines_check, sigid_print_outlines_check_toggled );
		}


		/***************************
		 * print_reverse_check
		 ***************************/
		private void on_print_reverse_check_toggled()
		{
			model.print.reverse_flag = print_reverse_check.get_active();
		}

		private void load_print_reverse_check()
		{
			GLib.SignalHandler.block( (void*)print_reverse_check, sigid_print_reverse_check_toggled );

			print_reverse_check.set_active( model.print.reverse_flag );

			GLib.SignalHandler.unblock( (void*)print_reverse_check, sigid_print_reverse_check_toggled );
		}


		/***************************
		 * print_crop_marks_check
		 ***************************/
		private void on_print_crop_marks_check_toggled()
		{
			model.print.crop_marks_flag = print_crop_marks_check.get_active();
		}

		private void load_print_crop_marks_check()
		{
			GLib.SignalHandler.block( (void*)print_crop_marks_check, sigid_print_crop_marks_check_toggled );

			print_crop_marks_check.set_active( model.print.crop_marks_flag );

			GLib.SignalHandler.unblock( (void*)print_crop_marks_check, sigid_print_crop_marks_check_toggled );
		}


	}

}

