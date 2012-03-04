/*  label_state.vala
 *
 *  Copyright (C) 2011  Jim Evins <evins@snaught.com>
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

	public class LabelState
	{
		public string   description { get; private set; }

		public bool     modified    { get; private set; }
		public TimeVal  time_stamp  { get; private set; }

		public Template template    { get; private set; }
		public bool     rotate      { get; private set; }

		// TODO: Merge

		public unowned List<LabelObject> object_list { get; private set; }


		public LabelState( string description,
		                   Label  label )
		{
			this.description = description;

			modified     = label.modified;
			time_stamp   = label.time_stamp;

			template     = label.template;
			rotate       = label.rotate;

			// TODO: Merge

			foreach ( LabelObject object in label.object_list )
			{
				object_list.append( object.dup() );
			}

		}


		public void restore( Label label )
		{
			label.rotate = rotate;
			label.template = template;

			foreach ( LabelObject object in label.object_list )
			{
				label.delete_object( object );
			}

			foreach ( LabelObject object in object_list )
			{
				label.add_object( object.dup() );
			}

			// TODO: Merge

			if ( !modified &&
			     (time_stamp.tv_sec  == label.time_stamp.tv_sec) &&
			     (time_stamp.tv_usec == label.time_stamp.tv_usec) )
			{
				label.modified = false;
			}
		}


	}

}

