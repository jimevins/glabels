/*  merge_backends.vala
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

	public class MergeBackends
	{

		private static Gee.HashMap<string,MergeInfo> id_map;
		private static Gee.HashMap<string,MergeInfo> name_map;

		private static bool initialized = false;


		static construct
		{
			id_map   = new Gee.HashMap<string,MergeInfo>();
			name_map = new Gee.HashMap<string,MergeInfo>();

			register_backend( "None", _("None"), MergeSrcType.NONE, typeof(MergeNone) );

			register_backend( "Text/Comma", _("Text: Comma Separated Values (CSV)"),
			                  MergeSrcType.FILE, typeof(MergeTextCsv) );
			register_backend( "Text/Comma/Line1Keys", _("Text: Comma Separated Values (CSV), keys on line 1"),
			                  MergeSrcType.FILE, typeof(MergeTextCsv) );
			register_backend( "Text/Tab", _("Text: Tab Separated Values (TSV)"),
			                  MergeSrcType.FILE, typeof(MergeTextTsv) );
			register_backend( "Text/Tab/Line1Keys", _("Text: Tab Separated Values (TSV), keys on line 1"),
			                  MergeSrcType.FILE, typeof(MergeTextTsv) );
			register_backend( "Text/Colon", _("Text: Colon Separated Values"),
			                  MergeSrcType.FILE, typeof(MergeTextColon) );
			register_backend( "Text/Colon/Line1Keys", _("Text: Colon Separated Values, keys on line 1"),
			                  MergeSrcType.FILE, typeof(MergeTextColon) );
			register_backend( "Text/Semicolon", _("Text: Semicolon Separated Values"),
			                  MergeSrcType.FILE, typeof(MergeTextSemicolon) );
			register_backend( "Text/Semicolon/Line1Keys", _("Text: Semicolon Separated Values, keys on line 1"),
			                  MergeSrcType.FILE, typeof(MergeTextSemicolon) );

			initialized = true;
		}


		public static void init()
		{
			if ( !initialized )
			{
				new MergeBackends();
			}
			else
			{
				message( "MergeBackends already initialized.\n" );
			}
		}


		private static void register_backend( string id, string name, MergeSrcType src_type, Type type )
		{
			if ( type.is_a( typeof(Merge) ) )
			{
				MergeInfo merge_info = new MergeInfo( id, name, src_type, type );

				id_map.set( id.casefold(), merge_info );
				name_map.set( name, merge_info );
			}
			else
			{
				message( "MergeBackends.register_backend: invalid merge type: %s\n", type.name() );
			}
		}


		public static Merge create_merge( string id )
		{
			string id_casefold = id.casefold();

			if ( id_map.has_key( id_casefold ) )
			{
				MergeInfo merge_info = id_map.get( id_casefold );

				return Object.new( merge_info.merge_type, info: merge_info ) as Merge;
			}
			else
			{
				return new MergeNone();
			}
		}
		

	}

}
