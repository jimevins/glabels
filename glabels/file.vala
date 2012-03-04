/*  file.vala
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

	namespace File
	{
		private static string? previous_open_path = null;
		private static string? previous_save_path = null;


		public void new_label( Window window )
		{
			NewLabelDialog dialog = new NewLabelDialog( window );
			dialog.set_title( _("New Label or Card") );

			dialog.show_all();
			int response = dialog.run();

			if ( response == Gtk.ResponseType.OK )
			{
				Label label = new Label();
				label.template = libglabels.Db.lookup_template_from_name( dialog.template_name );

				if ( window.is_empty() )
				{
					window.set_label( label );
				}
				else
				{
					Window new_window = new Window.from_label( label );
					new_window.show_all();
				}

				dialog.hide();
			}
		}


		public void open( Window window )
		{
			OpenDialog chooser = new OpenDialog( window );

			chooser.response.connect( on_open_response );

			chooser.show();
		}


		private void on_open_response( Gtk.Dialog gtk_dialog, int response )
		{
			OpenDialog dialog = (OpenDialog)gtk_dialog;

			switch (response)
			{

			case Gtk.ResponseType.ACCEPT:
				string raw_filename = dialog.get_filename();

				string? filename = null;
				try
				{
					filename = Filename.to_utf8( raw_filename, -1, null, null );
				}
				catch ( ConvertError e )
				{
					message( "Utf8 filename conversion: %s", e.message );
				}

				if ( (raw_filename == null) || (filename == null) ||
				     FileUtils.test( filename, FileTest.IS_DIR ) )
				{
					var md = new Gtk.MessageDialog( dialog,
					                                Gtk.DialogFlags.MODAL | Gtk.DialogFlags.DESTROY_WITH_PARENT,
					                                Gtk.MessageType.WARNING,
					                                Gtk.ButtonsType.CLOSE,
					                                _("Empty file name selection") );
					md.format_secondary_text( _("Please select a file or supply a valid file name") );

					md.run();
					md.destroy();
				}
				else
				{

					if ( !FileUtils.test( filename, FileTest.IS_REGULAR ) )
					{
						var md = new Gtk.MessageDialog( dialog,
						                                Gtk.DialogFlags.MODAL | Gtk.DialogFlags.DESTROY_WITH_PARENT,
						                                Gtk.MessageType.WARNING,
						                                Gtk.ButtonsType.CLOSE,
						                                _("File does not exist") );
						md.format_secondary_text( _("Please select a file or supply a valid file name") );

						md.run();
						md.destroy();
					}
					else
					{
						if ( open_real( filename, dialog.parent_window ) )
						{
							dialog.destroy();
						}
					}

				}
				break;

			default:
				dialog.destroy();
				break;

			}
		}


		private bool open_real( string filename, Window parent )
		{
			string abs_filename = FileUtil.make_absolute( filename );
			Label? label = null;
			try {
				label = XmlLabel.open_file( abs_filename );
			}
			catch ( XmlLabel.XmlError e )
			{
				var md = new Gtk.MessageDialog( parent,
				                                Gtk.DialogFlags.MODAL | Gtk.DialogFlags.DESTROY_WITH_PARENT,
				                                Gtk.MessageType.WARNING,
				                                Gtk.ButtonsType.CLOSE,
				                                _("Could not open file \"%s\""), filename );
				md.format_secondary_text( _("Not a supported file format") );

				md.run();
				md.destroy();

				return false;
			}

			if ( parent.is_empty() )
			{
				parent.set_label( label );
			}
			else
			{
				Window new_window = new Window.from_label( label );
				new_window.show_all();
			}

			/* TODO: add abs_filename to recents. */

			previous_open_path = Path.get_dirname( abs_filename );

			return true;
		}


		private class OpenDialog : Gtk.FileChooserDialog
		{
			public Window parent_window { get; private set; }

			public OpenDialog( Window window )
			{
				parent_window = window;

				set_title( _("Open label") );
				action = Gtk.FileChooserAction.OPEN;
				add_button( Gtk.Stock.CANCEL, Gtk.ResponseType.CANCEL );
				add_button(	Gtk.Stock.OPEN, Gtk.ResponseType.ACCEPT );

				set_transient_for( parent_window );

				/* Recover state of open dialog */
				if ( previous_open_path != null )
				{
					set_current_folder( previous_open_path );
				}

				Gtk.FileFilter filter_all = new Gtk.FileFilter();
				filter_all.add_pattern( "*" );
				filter_all.set_name( _("All files") );
				add_filter( filter_all );

				Gtk.FileFilter filter_glabels = new Gtk.FileFilter();
				filter_glabels.add_pattern( "*.glabels" );
				filter_glabels.set_name( _("gLabels documents") );
				add_filter( filter_glabels );

				set_filter( filter_glabels );

			}

		}


		public bool save( Label label, Window parent )
		{

			if ( label.is_untitled() )
			{
				return save_as( label, parent );
			}

			if ( !label.modified )
			{
				return true;
			}

			try
			{
				XmlLabel.save_file( label, label.filename );
			}
			catch ( XmlLabel.XmlError e )
			{
				var md = new Gtk.MessageDialog( parent,
				                                Gtk.DialogFlags.MODAL | Gtk.DialogFlags.DESTROY_WITH_PARENT,
				                                Gtk.MessageType.WARNING,
				                                Gtk.ButtonsType.CLOSE,
				                                _("Could not save file \"%s\""), label.filename );
				md.format_secondary_text( _("Error encountered during save.  The file is still not saved.") );

				md.run();
				md.destroy();

				return false;
			}

			/* TODO: add filename to recents. */

			return true;
		}


		public bool save_as( Label label, Window parent )
		{
			SaveAsDialog chooser = new SaveAsDialog( label, parent );
			chooser.response.connect( on_save_as_response );

			chooser.show();

			/* Hold here and process events until we are done with this dialog. */
			/* This is so we can return a bollean result of our save attempt.   */
			Gtk.main();

			bool ret = chooser.saved_flag;

			chooser.destroy();

			return ret;
		}


		private void on_save_as_response( Gtk.Dialog gtk_dialog, int response )
		{
			SaveAsDialog dialog = (SaveAsDialog)gtk_dialog;

			switch (response)
			{

			case Gtk.ResponseType.ACCEPT:
				string raw_filename = dialog.get_filename();

				if ( (raw_filename == null) || FileUtils.test( raw_filename, FileTest.IS_DIR ) )
				{
					var md = new Gtk.MessageDialog( dialog,
					                                Gtk.DialogFlags.MODAL | Gtk.DialogFlags.DESTROY_WITH_PARENT,
					                                Gtk.MessageType.WARNING,
					                                Gtk.ButtonsType.CLOSE,
					                                _("Empty file name selection") );
					md.format_secondary_text( _("Please select a file or supply a valid file name") );

					md.run();
					md.destroy();
				}
				else
				{
					bool cancel_flag = false;

					string full_filename = FileUtil.add_extension( raw_filename );

					string? filename = null;
					try
					{
						filename = Filename.to_utf8( full_filename, -1, null, null );
					}
					catch ( ConvertError e )
					{
						message( "Utf8 filename conversion: %s", e.message );
					}

					if ( FileUtils.test( filename, FileTest.IS_REGULAR ) )
					{
						var md = new Gtk.MessageDialog( dialog,
						                                Gtk.DialogFlags.MODAL | Gtk.DialogFlags.DESTROY_WITH_PARENT,
						                                Gtk.MessageType.WARNING,
						                                Gtk.ButtonsType.YES_NO,
						                                _("Overwrite file \"%s\"?"), filename );
						md.format_secondary_text( _("File already exists.") );

						int ret = md.run();
						if ( ret == Gtk.ResponseType.NO )
						{
							cancel_flag = true;
						}

						md.destroy();
					}

					if ( !cancel_flag )
					{
						try
						{
							XmlLabel.save_file( dialog.label, filename );
						}
						catch ( XmlLabel.XmlError e )
						{
							var md = new Gtk.MessageDialog( dialog,
							                                Gtk.DialogFlags.MODAL | Gtk.DialogFlags.DESTROY_WITH_PARENT,
							                                Gtk.MessageType.WARNING,
							                                Gtk.ButtonsType.CLOSE,
							                                _("Could not save file \"%s\""), filename );
							md.format_secondary_text( _("Error encountered during save.  The file is still not saved.") );

							md.run();
							md.destroy();

							return;
						}

						dialog.saved_flag = true;

						/* TODO: add filename to recents. */

						previous_save_path = Path.get_dirname( filename );

						Gtk.main_quit();

					}

				}
				break;

			default:
				Gtk.main_quit();
				break;

			}
		}


		private class SaveAsDialog : Gtk.FileChooserDialog
		{
			public Label  label         { get; private set; }
			public Window parent_window { get; private set; }
			public bool   saved_flag    { get; set; default = false; }

			public SaveAsDialog( Label label, Window window )
			{
				this.label         = label;
				this.parent_window = window;

				string title = _("Save \"%s\" as").printf( label.get_short_name() );

				set_title( title );
				action = Gtk.FileChooserAction.SAVE;
				add_button( Gtk.Stock.CANCEL, Gtk.ResponseType.CANCEL );
				add_button(	Gtk.Stock.SAVE, Gtk.ResponseType.ACCEPT );

				set_transient_for( parent_window );

				/* Recover state of open dialog */
				if ( previous_save_path != null )
				{
					set_current_folder( previous_save_path );
				}

				Gtk.FileFilter filter_all = new Gtk.FileFilter();
				filter_all.add_pattern( "*" );
				filter_all.set_name( _("All files") );
				add_filter( filter_all );

				Gtk.FileFilter filter_glabels = new Gtk.FileFilter();
				filter_glabels.add_pattern( "*.glabels" );
				filter_glabels.set_name( _("gLabels documents") );
				add_filter( filter_glabels );

				set_filter( filter_glabels );

			}

		}


		private void close( Window window )
		{
			bool close_flag = true;

			if ( !window.is_empty() )
			{
				if ( window.view.label.modified )
				{
					var md = new Gtk.MessageDialog( window,
					                                Gtk.DialogFlags.MODAL | Gtk.DialogFlags.DESTROY_WITH_PARENT,
					                                Gtk.MessageType.WARNING,
					                                Gtk.ButtonsType.NONE,
					                                _("Save changes to document \"%s\" before closing?"),
					                                window.view.label.get_short_name() );
					md.format_secondary_text( _("Your changes will be lost if you don't save them.") );

					md.add_button( _("Close without saving"), Gtk.ResponseType.NO );
					md.add_button( Gtk.Stock.CANCEL, Gtk.ResponseType.CANCEL );
					md.add_button( Gtk.Stock.SAVE, Gtk.ResponseType.YES );
					md.set_default_response( Gtk.ResponseType.YES );

					md.set_resizable( false );

					int ret = md.run();

					md.destroy();

					switch (ret)
					{
					case Gtk.ResponseType.YES:
						close_flag = save( window.view.label, window );
						break;
					case Gtk.ResponseType.NO:
						close_flag = true;
						break;
					default:
						close_flag = false;
						break;
					}
					

				}
			}

			if ( close_flag )
			{
				window.destroy();
			}

		}


		private void exit()
		{
			unowned List<Window> p;
			unowned List<Window> p_next = null;

			for ( p = Window.window_list; p != null; p = p_next )
			{
				p_next = p.next; /* Squirrel away next pointer since close may be destructive to list. */

				close( p.data );
			}
		}


	}

}

