/*  model_undo_redo.vala
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


using GLib;
using libglabels;

namespace glabels
{

	public class ModelUndoRedo
	{
		private Label label;

		private Queue<LabelState?> undo_stack;
		private Queue<LabelState?> redo_stack;
		private bool               cp_cleared_flag;
		private string             cp_desc;


		public ModelUndoRedo( Label label )
		{
			this.label = label;

			undo_stack = new Queue<LabelState?>();
			redo_stack = new Queue<LabelState?>();

			label.selection_changed.connect( on_selection_changed );
		}


		private void on_selection_changed()
		{
			cp_cleared_flag = true;
		}


		public void checkpoint( string description )
		{
			/*
			 * Do not perform consecutive checkpoints that are identical.
			 * E.g. moving an object by dragging, would produce a large number
			 * of incremental checkpoints -- what we really want is a single
			 * checkpoint so that we can undo the entire dragging effort with
			 * one "undo"
			 */
			if ( cp_cleared_flag || (cp_desc == null) || ( description != cp_desc) )
			{

				/* Sever old redo "thread" */
				stack_clear(redo_stack);

				/* Save state onto undo stack. */
				LabelState state = new LabelState( description, label );
				undo_stack.push_head( state );

				/* Track consecutive checkpoints. */
				cp_cleared_flag = false;
				cp_desc         = description;
			}

		}


		public void undo()
		{
			LabelState state_old = undo_stack.pop_head();
			LabelState state_now = new LabelState( state_old.description, label );

			redo_stack.push_head( state_now );

			state_old.restore( label );

			cp_cleared_flag = true;

			label.selection_changed();
		}


		public void redo()
		{
			LabelState state_old = redo_stack.pop_head();
			LabelState state_now = new LabelState( state_old.description, label );

			undo_stack.push_head( state_now );

			state_old.restore( label );

			cp_cleared_flag = true;

			label.selection_changed();
		}


		public bool can_undo()
		{
			return ( !undo_stack.is_empty() );
		}


		public bool can_redo()
		{
			return ( !redo_stack.is_empty() );
		}


		public string get_undo_description()
		{
			LabelState state = undo_stack.peek_head();
			if ( state != null )
			{
				return state.description;
			}
			else
			{
				return "";
			}
		}


		public string get_redo_description()
		{
			LabelState state = redo_stack.peek_head();
			if ( state != null )
			{
				return state.description;
			}
			else
			{
				return "";
			}
		}


		private void stack_clear( Queue<LabelState> stack )
		{
			while ( stack.pop_head() != null ) {}
		}


	}

}
