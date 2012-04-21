/*  model_clipboard.vala
 *
 *  Copyright (C) 2011-2012  Jim Evins <evins@snaught.com>
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

	public class ModelClipboard : GLib.Object
	{
		private Label label;

		private string?     clipboard_xml_buffer;
		private string?     clipboard_text;
		private Gdk.Pixbuf? clipboard_pixbuf;


		public ModelClipboard( Label label )
		{
			this.label = label;
		}


		public void cut_selection()
		{
			copy_selection();
			label.delete_selection();
		}


		public void copy_selection()
		{
			const Gtk.TargetEntry glabels_targets[] = {
				{ "application/glabels", 0, 0 },
				{ "text/xml",            0, 0 }
			};

			Gtk.Clipboard clipboard = Gtk.Clipboard.get( Gdk.SELECTION_CLIPBOARD );

			List<LabelObject> selection_list = label.get_selection_list();

			if ( selection_list != null )
			{

				Gtk.TargetList target_list = new Gtk.TargetList( glabels_targets );

				/*
				 * Serialize selection by encoding as an XML label document.
				 */
				Label label_copy = new Label();

				label_copy.template = label.template;
				label_copy.rotate = label.rotate;

				foreach ( LabelObject object in selection_list )
				{
					label_copy.add_object( object );
				}

				// TODO: set clipboard_xml_buffer from label_copy

				/*
				 * Is it an atomic text selection?  If so, also make available as text.
				 */
				if ( label.is_selection_atomic() /* && TODO: first object is LabelObjectText */ )
				{
					target_list.add_text_targets( 1 );
					// TODO: set clipboard_text from LabelObjectText get_text()
				}

				/*
				 * Is it an atomic image selection?  If so, also make available as pixbuf.
				 */
				if ( label.is_selection_atomic() /* && TODO: first object is LabelObjectImage */ )
				{
					// TODO: pixbuf = LabelObjectImage get_pixbuf
					// TODO: if ( pixbuf != null )
					{
						target_list.add_image_targets( 2, true );
						// TODO: set clipboard_pixbuf = pixbuf
					}
				}

				Gtk.TargetEntry[] target_table = Gtk.target_table_new_from_list( target_list );

				clipboard.set_with_owner( target_table,
				                          (Gtk.ClipboardGetFunc)clipboard_get_cb,
				                          (Gtk.ClipboardClearFunc)clipboard_clear_cb, this );

			}

		}


		public void paste()
		{
			Gtk.Clipboard clipboard = Gtk.Clipboard.get( Gdk.SELECTION_CLIPBOARD );

			clipboard.request_targets( clipboard_receive_targets_cb );
		}


		public bool can_paste()
		{
			Gtk.Clipboard clipboard = Gtk.Clipboard.get( Gdk.SELECTION_CLIPBOARD );

			return ( clipboard.wait_is_target_available( Gdk.Atom.intern("application/glabels", true) ) ||
			         clipboard.wait_is_text_available()                                                 ||
			         clipboard.wait_is_image_available() );
		}


		private void clipboard_get_cb( Gtk.Clipboard     clipboard,
		                               Gtk.SelectionData selection_data,
		                               uint              info,
		                               void*             user_data )
		{
			switch (info)
			{
			case 0:
				selection_data.set( selection_data.get_target(),
				                    8,
				                    (uchar[])clipboard_xml_buffer );
				break;

			case 1:
				selection_data.set_text( clipboard_text, -1 );
				break;

			case 2:
				selection_data.set_pixbuf( clipboard_pixbuf );
				break;

			default:
				assert_not_reached();

			}
		}


		private void clipboard_clear_cb( Gtk.Clipboard clipboard,
		                                 void*         user_data )
		{
			clipboard_xml_buffer = null;
			clipboard_text       = null;
			clipboard_pixbuf     = null;
		}


		private void clipboard_receive_targets_cb( Gtk.Clipboard clipboard,
		                                           Gdk.Atom[]    targets )
		{

			/*
			 * Application/glabels
			 */
			for ( int i = 0; i < targets.length; i++ )
			{
				if ( targets[i].name() == "application/glabels" )
				{
					clipboard.request_contents( targets[i], paste_xml_received_cb );
					return;
				}
			}

			/*
			 * Text
			 */
			if ( Gtk.targets_include_text( targets ) )
			{
				clipboard.request_text( paste_text_received_cb );
				return;
			}

			/*
			 * Image
			 */
			if ( Gtk.targets_include_image( targets, true ) )
			{
				clipboard.request_image( paste_image_received_cb );
				return;
			}

		}


		private void paste_xml_received_cb( Gtk.Clipboard     clipboard,
		                                    Gtk.SelectionData selection_data )
		{
			string xml_buffer = (string)selection_data.get_data();

			/*
			 * Deserialize XML label document and extract objects.
			 */
			// TODO:  label_copy = xml_label_open_buffer( xml_buffer )
			// label.unselect all
			// foreach object in label copy, add to this, select each object as added.

		}


		private void paste_text_received_cb( Gtk.Clipboard     clipboard,
		                                     string?           text )
		{
			label.unselect_all();
			// TODO:  create new LabelObjectText object from text.  set to a default location, select.
		}


		private void paste_image_received_cb( Gtk.Clipboard     clipboard,
		                                      Gdk.Pixbuf        pixbuf )
		{
			label.unselect_all();
			// TODO:  create new LabelObjectImage object from pixbuf.  set to a default location, select.
		}


	}

}
