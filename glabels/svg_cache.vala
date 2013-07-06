/*  svg_cache.vala
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

	public class SvgCache
	{

		private class Record : Object
		{
			public int         use_count;
			public Rsvg.Handle handle;
			public string      svg;

			public Record( Rsvg.Handle handle, string svg )
			{
				this.use_count = 0;
				this.handle    = handle;
				this.svg       = svg;
			}

			public Record.from_svg( string svg ) throws Error
			{
				Rsvg.Handle? handle;

				try {
					handle = new Rsvg.Handle.from_data( svg.data );
				}
				catch ( Error e )
				{
					handle = null;
					throw e;
				}

				this( handle, svg );
			}

			public Record.from_file( string file_name ) throws Error
			{
				Rsvg.Handle? handle;
				string       svg;

				try {
					FileUtils.get_contents( file_name, out svg );
					handle = new Rsvg.Handle.from_data( svg.data );
				}
				catch ( Error e )
				{
					handle = null;
					svg    = "";
					throw e;
				}

				this( handle, svg );
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


		public SvgCache()
		{
			map = new Gee.HashMap<string,Record>();
		}


		public void add_svg( string name, string svg )
		{
			if ( map.has_key( name ) )
			{
				/* svg already in the cache. */
				return;
			}

			try {
				map.set( name, new Record.from_svg( svg ) );
			}
			catch ( Error e )
			{
			}
		}


		public string? get_svg( string name )
		{
			if ( map.has_key( name ) )
			{
				Record record = map.get( name );
				record.increment();
				return record.svg;
			}

			try
			{
				Record record = new Record.from_file( name );
				record.increment();
				map.set( name, record );

				return record.svg;
			}
			catch ( Error e )
			{
				return null;
			}
		}


		public Rsvg.Handle? get_handle( string name )
		{
			if ( map.has_key( name ) )
			{
				Record record = map.get( name );
				record.increment();
				return record.handle;
			}

			try
			{
				Record record = new Record.from_file( name );
				record.increment();
				map.set( name, record );

				return record.handle;
			}
			catch ( Error e )
			{
				return null;
			}
		}


		public void remove_svg( string name )
		{
			Record? record = map.get( name );
			if ( record.decrement() == 0 )
			{
				map.unset( name );
				record.unref();
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
