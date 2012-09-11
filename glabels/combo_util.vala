/*  combo_util.vala
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

	namespace ComboUtil
	{

		public void load_strings( Gtk.ComboBoxText combo,
		                          List<string>     list )
		{
			combo.remove_all();

			foreach ( string s in list )
			{
				combo.append_text( s );
			}
		}


		public void set_active_text( Gtk.ComboBoxText combo,
		                             string           text )
		{
			Gtk.TreeModel model = combo.get_model();

			Gtk.TreeIter iter;

			if ( model.get_iter_first( out iter ) )
			{
				do
				{
					string model_text = "";

					model.get( iter, 0, ref model_text, -1 );
					if ( model_text == text )
					{
						combo.set_active_iter( iter );
						return;
					}
				}
				while ( model.iter_next( ref iter ) );
			}

			combo.set_active( -1 );
		}

	}

}



