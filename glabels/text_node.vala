/*  text_node.vala
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

	public class TextNode
	{
		public  bool    field_flag { get; private set; }
		public  string? data       { get; private set; }


		public TextNode( bool field_flag, string? data )
		{
			this.field_flag = field_flag;
			this.data       = data;
		}


		private enum State { START,
		                     LITERAL, LITERAL_DOLLAR,
		                     START_DOLLAR, FIELD,
		                     DONE }


		public TextNode.parse( string text, int i_start, out int i_next )
		{
			State         state        = State.START;
			StringBuilder literal_text = new StringBuilder();
			StringBuilder field_name   = new StringBuilder();
			bool          field_flag   = false;

			int i = i_start;

			while ( state != State.DONE )
			{
				char c = text[i];

				switch (state) {

				case State.START:
					switch (c) {
					case '$':
						/* May be start of a field node. */
						i++;
						state = State.START_DOLLAR;
						break;
					case '\n':
						state = State.DONE;
						break;
					case 0:
						state = State.DONE;
						break;
					default:
						/* Start a literal text node. */
						literal_text.append_c( c );
						i++;
						state = State.LITERAL;
						break;
					}
					break;

				case State.LITERAL:
					switch (c) {
					case '$':
						/* May be the beginning of a field node. */
						i++;
						state = State.LITERAL_DOLLAR;
						break;
					case '\n':
						state = State.DONE;
						break;
					case 0:
						state = State.DONE;
						break;
					default:
						literal_text.append_c( c );
						i++;
						state = State.LITERAL;
						break;
					}
					break;

				case State.LITERAL_DOLLAR:
					switch (c) {
					case '{':
						/* "${" indicates the start of a new field node, gather for literal too. */
						literal_text.append_c( '$' );
						i++;
						state = State.DONE;
						break;
					case '\n':
						/* Append "$" to literal text, don't gather newline. */
						literal_text.append_c( '$' );
						i++;
						state = State.DONE;
						break;
					case 0:
						/* Append "$" to literal text, don't gather null. */
						literal_text.append_c( '$' );
						i++;
						state = State.DONE;
						break;
					default:
						/* Append "$" to literal text, gather this character too. */
						literal_text.append_c( '$' );
						literal_text.append_c( c );
						i+=2;
						state = State.LITERAL;
						break;
					}
					break;

				case State.START_DOLLAR:
					switch (c) {
					case '{':
						/* This is probably the begging of a field node, gather for literal too. */
						literal_text.append_c( c );
						i++;
						state = State.FIELD;
						break;
					case '\n':
						state = State.DONE;
						break;
					case 0:
						state = State.DONE;
						break;
					default:
						/* The "$" was literal. */
						literal_text.append_c( c );
						i++;
						state = State.LITERAL;
						break;
					}
					break;

				case State.FIELD:
					switch (c) {
					case '}':
						/* We now finally know that this node is really field node. */
						field_flag = true;
						i++;
						state = State.DONE;
						break;
					case '\n':
						state = State.DONE;
						break;
					case 0:
						state = State.DONE;
						break;
					default:
						/* Gather for field name and literal, just in case. */
						field_name.append_unichar( c );
						literal_text.append_c( c );
						i++;
						state = State.FIELD;
						break;
					}
					break;

				}

			}

			string data;
			if ( field_flag )
			{
				data = field_name.str;
			}
			else
			{
				data = literal_text.str;
			}

			this( field_flag, data );

			i_next = i;
		}


		public TextNode dup()
		{
			TextNode copy = new TextNode( field_flag, data );

			return copy;
		}


		public bool equal( TextNode node2 )
		{
			if ( this.field_flag != node2.field_flag )
			{
				return false;
			}

			return ( this.data == node2.data );
		}


		public string expand( MergeRecord? record )
		{
			if ( field_flag )
			{

				if ( record == null )
				{
					return "${%s}".printf( data );
				}
				else
				{
					string? text = record.eval_key( data );
					if ( text != null )
					{
						return text;
					}
					else
					{
						return "";
					}
				}

			}
			else
			{
				return data;
			}
		}


		public bool is_empty_field( MergeRecord? record )
		{
			if ( (record !=null) && field_flag )
			{
				string? text = record.eval_key( data );
				return ( (text == null) || (text == "") );
			}
			else
			{
				return false;
			}
		}


	}

}
