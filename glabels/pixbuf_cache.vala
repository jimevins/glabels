/*  pixbuf_cache.vala
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

	public class PixbufCache
	{

		private class Record : Object
		{
			public int        use_count;
			public Gdk.Pixbuf pixbuf;

			public Record( Gdk.Pixbuf pixbuf )
			{
				this.use_count = 0;
				this.pixbuf    = pixbuf;
			}

			public void increment()
			{
				use_count++;
			}

			public int decrement()
			{
				use_count--;
				return use_count;
			}
		}


		private Gee.HashMap<string,Record> map;


		public PixbufCache()
		{
			map = new Gee.HashMap<string,Record>();
		}


		public void add_pixbuf( string name, Gdk.Pixbuf pixbuf )
		{
			if ( map.has_key( name ) )
			{
				/* pixbuf already in the cache. */
				return;
			}

			Record record = new Record( pixbuf );
			map.set( name, record );
		}


		public Gdk.Pixbuf? get_pixbuf( string name )
		{
			if ( map.has_key( name ) )
			{
				Record record = map.get( name );
				record.increment();
				return record.pixbuf;
			}

			try {
				Gdk.Pixbuf? pixbuf = new Gdk.Pixbuf.from_file( name );
				Record record = new Record( pixbuf );
				record.increment();
				map.set( name, record );

				return pixbuf;
			}
			catch ( Error e )
			{
				return null;
			}
		}


		public void remove_pixbuf( string name )
		{
			Record? record = map.get( name );
			if ( record.decrement() == 0 )
			{
				map.unset( name );
			}
		}


		public List<string> get_name_list()
		{
			List<string> list = new List<string>();

			foreach ( string name in map.keys )
			{
				list.append( name );
			}

			return list;
		}


	}

}
