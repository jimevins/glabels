/*  merge.vala
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

	public abstract class Merge : Object
	{

		public signal void source_changed();
		public signal void selection_changed();


		public MergeInfo info { get; construct; }


		public unowned List<MergeRecord> record_list
		{
			get { return _record_list; }
		}
		private List<MergeRecord> _record_list;


		public uint record_count
		{
			get { return _record_list.length(); }
		}


		public string? src
		{
			get { return _src; }

			set
			{
				_record_list = null;
				_src = value;

				if ( _src != null )
				{
					MergeRecord? record;

					this.open();
					while ( (record = this.get_record()) != null )
					{
						_record_list.append( record );
					}
					this.close();

					source_changed();
				}

			}

		}
		private string? _src;


		public void select( MergeRecord record )
		{
			record.selected = true;
			selection_changed();
		}


		public void unselect( MergeRecord record )
		{
			record.selected = false;
			selection_changed();
		}


		public void select_all()
		{
			foreach ( MergeRecord record in _record_list )
			{
				record.selected = true;
			}
			selection_changed();
		}


		public void unselect_all()
		{
			foreach ( MergeRecord record in _record_list )
			{
				record.selected = false;
			}
			selection_changed();
		}


		public Gee.ArrayList<MergeRecord> get_selected_records()
		{
			Gee.ArrayList<MergeRecord> list = new Gee.ArrayList<MergeRecord>();

			foreach ( MergeRecord record in _record_list )
			{
				if ( record.selected )
				{
					list.add( record );
				}
			}

			return list;
		}


		public    abstract List<string>  get_key_list();
		public    abstract string        get_primary_key();
		protected abstract void          open();
		protected abstract void          close();
		protected abstract MergeRecord?  get_record();
		public    abstract Merge         dup();

	}

}
