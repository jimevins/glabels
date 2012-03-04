/*  merge_text.vala
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

	public class MergeText : Merge
	{
		public int    delim          { get; construct; default=','; }
		public bool   line1_has_keys { get; construct; default=false; }


		private FileStream fp;

		private string[] keys;
		private int      n_fields_max;
		

		public MergeText( string name           = "text/csv",
		                  string description    = "",
		                  int    delim          = ',',
		                  bool   line1_has_keys = false )
		{
			Object( name:name, description:description, delim:delim, line1_has_keys:line1_has_keys );

			src_type = MergeSrcType.FILE;
		}


		public override List<string> get_key_list()
		{
			int n_fields;
			if ( line1_has_keys )
			{
				n_fields = keys.length;
			}
			else
			{
				n_fields = n_fields_max;
			}

			List<string> key_list = new List<string>();
			for ( int i_field = 0; i_field < n_fields; i_field++ )
			{
				key_list.append( key_from_index( i_field ) );
			}

			return key_list;
		}


		private string key_from_index( int i_field )
		{
			if ( line1_has_keys && (i_field < keys.length) )
			{
				return keys[ i_field ];
			}
			else
			{
				return "%d".printf( i_field+1 );
			}
		}


		public override string get_primary_key()
		{
			/* For now, let's always assume the first column is the primary key. */
			return key_from_index( 0 );
		}


		protected override void open()
		{
			if ( src == "-" )
			{
				fp = FileStream.fdopen( Posix.STDIN_FILENO, "r" );
			}
			else
			{
				fp = FileStream.open( src, "r" );
			}

			keys = null;
			n_fields_max = 0;

			if ( line1_has_keys )
			{
				/*
				 * Extract keys from first line and discard line.
				 */
				List<string> line1_fields = parse_line();

				foreach (string field in line1_fields)
				{
					keys += field;
				}
			}
		}


		protected override void close()
		{
			fp = null;
		}


		protected override MergeRecord? get_record()
		{
			List<string?> values = parse_line();
			if ( values == null )
			{
				return null;
			}

			MergeRecord record = new MergeRecord();

			int i_field = 0;
			foreach ( string value in values )
			{
				MergeField field = MergeField();
				field.key = key_from_index( i_field );
				field.value = value;
				i_field++;

				record.field_list.append( field );
			}

			if ( i_field > n_fields_max )
			{
				n_fields_max = i_field;
			}
			return record;
		}


		public override Merge dup()
		{
			MergeText copy = new MergeText();
			return copy;
		}

		
		private enum State { DELIM,
		                     QUOTED, QUOTED_QUOTE1, QUOTED_ESCAPED,
		                     SIMPLE, SIMPLE_ESCAPED,
		                     DONE }

		/*---------------------------------------------------------------------------
		 * PRIVATE.  Parse line.                                                     
		 *                                                                           
		 * Attempt to be a robust parser of various CSV (and similar) formats.       
		 *                                                                           
		 * Based on CSV format described in RFC 4180 section 2.                      
		 *                                                                           
		 * Additions to RFC 4180 rules:                                              
		 *   - delimeters and other special characters may be "escaped" by a leading 
		 *     backslash (\)                                                         
		 *   - C escape sequences for newline (\n) and tab (\t) are also translated. 
		 *   - if quoted text is not followed by a delimeter, any additional text is 
		 *     concatenated with quoted portion.                                     
		 *                                                                           
		 * Returns a list of fields.  A blank line is considered a line with one     
		 * empty field.  Returns empty (NULL) when done.                             
		 *--------------------------------------------------------------------------*/
		private List<string> parse_line()
		{
			List<string> list = new List<string>();

			if ( fp == null )
			{
				return list;
			}
	       
			State state = State.DELIM;
			StringBuilder field = new StringBuilder();

			while ( state != State.DONE ) {
				int c = fp.getc();

				switch (state) {

				case State.DELIM:
					switch (c) {
					case '\n':
						/* last field is empty. */
						list.append("");
						state = State.DONE;
						break;
					case '\r':
						/* ignore */
						state = State.DELIM;
						break;
					case FileStream.EOF:
						/* end of file, no more lines. */
						state = State.DONE;
						break;
					case '"':
						/* start a quoted field. */
						state = State.QUOTED;
						break;
					case '\\':
						/* simple field, but 1st character is an escape. */
						state = State.SIMPLE_ESCAPED;
						break;
					default:
						if ( c == delim )
						{
							/* field is empty. */
							list.append("");
							state = State.DELIM;
						}
						else
						{
							/* begining of a simple field. */
							field.append_c( (char)c );
							state = State.SIMPLE;
						}
						break;
					}
					break;

				case State.QUOTED:
					switch (c) {
					case FileStream.EOF:
						/* File ended mid way through quoted item, truncate field. */
						list.append( field.str );
						state = State.DONE;
						break;
					case '"':
						/* Possible end of field, but could be 1st of a pair. */
						state = State.QUOTED_QUOTE1;
						break;
					case '\\':
						/* Escape next character, or special escape, e.g. \n. */
						state = State.QUOTED_ESCAPED;
						break;
					default:
						/* Use character literally. */
						field.append_c( (char)c );
						break;
					}
					break;

				case State.QUOTED_QUOTE1:
					switch (c) {
					case '\n':
					case FileStream.EOF:
						/* line or file ended after quoted item */
						list.append( field.str );
						state = State.DONE;
						break;
					case '"':
						/* second quote, insert and stay quoted. */
						field.append_c( (char)c );
						state = State.QUOTED;
						break;
					case '\r':
						/* ignore and go to fallback */
						state = State.SIMPLE;
						break;
					default:
						if ( c == delim )
						{
							/* end of field. */
							list.append( field.str );
							field.truncate( 0 );
							state = State.DELIM;
						}
						else
						{
							/* fallback if not a delim or another quote. */
							field.append_c( (char)c );
							state = State.SIMPLE;
						}
						break;
					}
					break;

				case State.QUOTED_ESCAPED:
					switch (c) {
					case FileStream.EOF:
						/* File ended mid way through quoted item */
						list.append( field.str );
						state = State.DONE;
						break;
					case 'n':
						/* Decode "\n" as newline. */
						field.append_c( (char)'\n' );
						state = State.QUOTED;
						break;
					case 't':
						/* Decode "\t" as tab. */
						field.append_c( (char)'\t' );
						state = State.QUOTED;
						break;
					default:
						/* Use character literally. */
						field.append_c( (char)c );
						state = State.QUOTED;
						break;
					}
					break;

				case State.SIMPLE:
					switch (c) {
					case '\n':
					case FileStream.EOF:
						/* line or file ended */
						list.append( field.str );
						state = State.DONE;
						break;
					case '\r':
						/* ignore */
						state = State.SIMPLE;
						break;
					case '\\':
						/* Escape next character, or special escape, e.g. \n. */
						state = State.SIMPLE_ESCAPED;
						break;
					default:
						if ( c == delim )
						{
							/* end of field. */
							list.append( field.str );
							field.truncate( 0 );
							state = State.DELIM;
						}
						else
						{
							/* Use character literally. */
							field.append_c( (char)c );
							state = State.SIMPLE;
						}
						break;
					}
					break;

				case State.SIMPLE_ESCAPED:
					switch (c) {
					case FileStream.EOF:
						/* File ended mid way through quoted item */
						list.append( field.str );
						state = State.DONE;
						break;
					case 'n':
						/* Decode "\n" as newline. */
						field.append_c( (char)'\n' );
						state = State.SIMPLE;
						break;
					case 't':
						/* Decode "\t" as tab. */
						field.append_c( (char)'\t' );
						state = State.SIMPLE;
						break;
					default:
						/* Use character literally. */
						field.append_c( (char)c );
						state = State.SIMPLE;
						break;
					}
					break;

				default:
					assert_not_reached();
				}

			}

			return list;
		}


	}

}
