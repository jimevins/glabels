/*  text_lines.vala
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

	public class TextLines
	{
		public unowned List<TextLine> lines { get; private set; }


		public TextLines( string text )
		{
			int i_next  = 0;

			for ( int i = 0; text[i] != 0; i = i_next )
			{
				stderr.printf( "Text[%d] = %c, ", i, text[i] ); 
				lines.append( new TextLine( text, i, out i_next ) );
				stderr.printf( "i_next =%d\n", i_next ); 
			}
		}


		public string expand( MergeRecord? record )
		{
			StringBuilder builder = new StringBuilder();

			foreach ( TextLine line in lines )
			{
				line.expand( record, ref builder );
			}

			return builder.str;
		}


	}

}
