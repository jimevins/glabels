/*  merge_none.vala
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

	public class MergeNone : Merge
	{

		public MergeNone()
		{
			src_type = MergeSrcType.NONE;
		}


		public override List<string> get_key_list()
		{
			return new List<string>();
		}


		public override string get_primary_key()
		{
			return "";
		}


		protected override void open()
		{
		}


		protected override void close()
		{
		}


		protected override MergeRecord? get_record()
		{
			return null;
		}


		public override Merge dup()
		{
			MergeNone copy = new MergeNone();
			return copy;
		}
		
	}

}
