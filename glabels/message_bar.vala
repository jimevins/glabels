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
				image.set_from_icon_name( "dialog-information", Gtk.IconSize.DIALOG );
				break;
			case Gtk.MessageType.WARNING:
				image.set_from_icon_name( "dialog-warning", Gtk.IconSize.DIALOG );
				break;
			case Gtk.MessageType.QUESTION:
				image.set_from_icon_name( "dialog-question", Gtk.IconSize.DIALOG );
				break;
			case Gtk.MessageType.ERROR:
				image.set_from_icon_name( "dialog-error", Gtk.IconSize.DIALOG );
				break;
			default:
				break;
			}

			var args = va_list();
			primary_text = message_format.vprintf( args );
			label = new Gtk.Label( primary_text );

			Gtk.Container container = get_content_area() as Gtk.Container;
			Gtk.Box hbox = new Gtk.Box( Gtk.Orientation.HORIZONTAL, 6 );
			container.add( hbox );

			hbox.pack_start( image, false, false, 0 );
			hbox.pack_start( label, false, false, 0 );

			hbox.show_all();

			switch (buttons)
			{
			case Gtk.ButtonsType.OK:
				add_button( "gtk-ok", Gtk.ResponseType.ACCEPT );
				break;
			case Gtk.ButtonsType.CLOSE:
				add_button( "gtk-close", Gtk.ResponseType.ACCEPT );
				break;
			case Gtk.ButtonsType.CANCEL:
				add_button( "gtk-cancel", Gtk.ResponseType.REJECT );
				break;
			case Gtk.ButtonsType.YES_NO:
				add_button( "gtk-yes", Gtk.ResponseType.ACCEPT );
				add_button( "gtk-no",  Gtk.ResponseType.REJECT );
				break;
			case Gtk.ButtonsType.OK_CANCEL:
				add_button( "gtk-ok",     Gtk.ResponseType.ACCEPT );
				add_button( "gtk-cancel", Gtk.ResponseType.REJECT );
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