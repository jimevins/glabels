/*  text_line.vala
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

	public class TextLine
	{
		public unowned List<TextNode> nodes { get; private set; }


		public TextLine()
		{
		}


		public TextLine.parse( string text, int i_start, out int i_next )
		{
			this();

			i_next = i_start;

			for ( int i = i_start; text[i] != 0; i = i_next )
			{
				if ( text[i] != '\n' )
				{
					TextNode node = new TextNode.parse( text, i, out i_next );
					nodes.append( node );
				}
				else
				{
					i_next++;
					break;
				}
			}
		}


		public void append( TextNode node )
		{
			nodes.append( node );
		}


		public bool empty()
		{
			return nodes.first() == null;
		}


		public void expand( MergeRecord? record, ref StringBuilder builder )
		{
			/* special case: something like ${ADDRESS2} = "" on line by itself. */
			/*               in such circumstances ignore the line completely.  */
			if ( nodes.first().next == null )
			{
				if ( nodes.first().data.is_empty_field( record ) )
				{
					return;
				}
			}

			/* prepend newline unless this is the first line. */
			if ( builder.len > 0 )
			{
				builder.append_c( '\n' );
			}

			/* expand each node */
			foreach (TextNode node in nodes)
			{
				builder.append( node.expand( record ) );
			}
		}


	}

}
