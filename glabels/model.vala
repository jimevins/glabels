/*  model.vala
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


// ****************************************************************************************
// TODO:  do checkpointing in UI code before invoking changes to Label or LabelObject*s.
// ****************************************************************************************

using GLib;
using libglabels;

namespace glabels
{

	public class Model
	{
		public Label          label     { get; private set; }
		public ModelClipboard clipboard { get; private set; }
		public ModelUndoRedo  undo_redo { get; private set; }
		public ModelPrint     print     { get; private set; }


		public Model( Label label )
		{
			this.label = label;

			clipboard = new ModelClipboard( label );
			undo_redo = new ModelUndoRedo( label );
			print     = new ModelPrint( label );
		}


	}

}
