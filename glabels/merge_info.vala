/*  merge_info.vala
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

	public enum MergeSrcType { NONE, FIXED, FILE }


	public class MergeInfo : Object
	{

		public string       id          { get; protected set; }
		public string       name        { get; protected set; }
		public MergeSrcType src_type    { get; protected set; }
		public Type         merge_type  { get; protected set; }


		public MergeInfo( string       id,
		                  string       name,
		                  MergeSrcType src_type,
		                  Type         merge_type )
		{
			this.id         = id;
			this.name       = name;
			this.src_type   = src_type;
			this.merge_type = merge_type;
		}

	}

}
