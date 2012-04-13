/*  message_bar.vala
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

	class MessageBar : Gtk.InfoBar
	{
		Gtk.Image  image;
		Gtk.Label  label;

		string     primary_text;
		string     secondary_text;


		public MessageBar( Gtk.MessageType type,
		                   Gtk.ButtonsType buttons,
		                   string          message_format,
		                   ... )
		{
			set_message_type( type );

			image = new Gtk.Image();
			switch( type )
			{
			case Gtk.MessageType.INFO:
				image.set_from_stock( Gtk.Stock.DIALOG_INFO, Gtk.IconSize.DIALOG );
				break;
			case Gtk.MessageType.WARNING:
				image.set_from_stock( Gtk.Stock.DIALOG_WARNING, Gtk.IconSize.DIALOG );
				break;
			case Gtk.MessageType.QUESTION:
				image.set_from_stock( Gtk.Stock.DIALOG_QUESTION, Gtk.IconSize.DIALOG );
				break;
			case Gtk.MessageType.ERROR:
				image.set_from_stock( Gtk.Stock.DIALOG_ERROR, Gtk.IconSize.DIALOG );
				break;
			default:
				break;
			}

			var args = va_list();
			primary_text = message_format.vprintf( args );
			label = new Gtk.Label( primary_text );

			Gtk.Container container = get_content_area() as Gtk.Container;
			Gtk.HBox hbox = new Gtk.HBox( false, 6 );
			container.add( hbox );

			hbox.pack_start( image, false, false, 0 );
			hbox.pack_start( label, false, false, 0 );

			hbox.show_all();

			switch (buttons)
			{
			case Gtk.ButtonsType.OK:
				add_button( Gtk.Stock.OK, Gtk.ResponseType.ACCEPT );
				break;
			case Gtk.ButtonsType.CLOSE:
				add_button( Gtk.Stock.CLOSE, Gtk.ResponseType.ACCEPT );
				break;
			case Gtk.ButtonsType.CANCEL:
				add_button( Gtk.Stock.CANCEL, Gtk.ResponseType.REJECT );
				break;
			case Gtk.ButtonsType.YES_NO:
				add_button( Gtk.Stock.YES, Gtk.ResponseType.ACCEPT );
				add_button( Gtk.Stock.NO,  Gtk.ResponseType.REJECT );
				break;
			case Gtk.ButtonsType.OK_CANCEL:
				add_button( Gtk.Stock.OK,     Gtk.ResponseType.ACCEPT );
				add_button( Gtk.Stock.CANCEL, Gtk.ResponseType.REJECT );
				break;
			default:
				break;
			}

		}


		public void format_secondary_text( string message_format, ... )
		{
			var args = va_list();
			secondary_text = message_format.vprintf( args );

			string markup = "<span weight=\"bold\" size=\"larger\">%s</span>\n%s".printf( primary_text,
				                                                                          secondary_text );

			label.set_markup( markup );
		}

	}

}