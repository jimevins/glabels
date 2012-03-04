/*  merge_factory.vala
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

	public class MergeFactory
	{

		/* TODO: register backends rather than hard code.  Problem gathering varargs of unknown
		 * type to set Values. */

		public static Merge create_merge( string name )
		{
			switch (name)
			{

			case "Text/Comma":
				return new MergeText( name, _("Text: Comma Separated Values (CSV)"),
				                      ',', false );

			case "Text/Comma/Line1Keys":
				return new MergeText( name, _("Text: Comma Separated Values (CSV) with keys on line 1"),
				                      ',', true );

			case "Text/Tab":
				return new MergeText( name, _("Text: Tab Separated Values (TSV)"),
				                      '\t', false );

			case "Text/Tab/Line1Keys":
				return new MergeText( name, _("Text: Tab Separated Values (TSV) with keys on line 1"),
				                      '\t', true );

			case "Text/Colon":
				return new MergeText( name, _("Text: Colon Separated Values"),
				                      ':', false );

			case "Text/Colon/Line1Keys":
				return new MergeText( name, _("Text: Colon Separated Values with keys on line 1"),
				                      ':', true );

			case "Text/Semicolon":
				return new MergeText( name, _("Text: Semicolon Separated Values"),
				                      ';', false );

			case "Text/Semicolon/Line1Keys":
				return new MergeText( name, _("Text: Semicolon Separated Values with keys on line 1"),
				                      ';', true );

			default:
				return new MergeNone();

			}
		}
		
	}

}
