/*  xml_vendor.vala
 *
 *  Copyright (C) 2011  Jim Evins <evins@snaught.com>
 *
 *  This file is part of libglabels.
 *
 *  libglabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libglabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libglabels.  If not, see <http://www.gnu.org/licenses/>.
 */


using GLib;

namespace libglabels
{

	namespace XmlVendor
	{

		internal void read_vendors_from_file( string utf8_filename )
		{
			string filename;
			try
			{
				filename = Filename.from_utf8( utf8_filename, -1, null, null );
			}
			catch ( ConvertError e )
			{
				message("Utf8 filename conversion: %s", e.message);
				return;
			}

			unowned Xml.Doc* doc = Xml.Parser.parse_file( filename );
			if ( doc == null )
			{
				message("\"%s\" is not a glabels vendor file (not XML)", filename);
				return;
			}

			parse_vendors_doc( doc );
		}


		internal void parse_vendors_doc( Xml.Doc doc )
		{
			unowned Xml.Node* root = doc.get_root_element();
			if ( (root == null) || (root->name == null) )
			{
				message("\"%s\" is not a glabels vendor file (no root node)", doc.name);
				return;
			}

			if ( root->name != "Glabels-vendors" )
			{
				message ("\"%s\" is not a glabels vendor file (wrong root node)", doc.name);
				return;
			}

			for ( unowned Xml.Node* node = root->children; node != null; node = node->next )
			{
				if ( node->name == "Vendor" )
				{
					Vendor vendor = parse_vendor_node( node );
					Db.register_vendor( vendor );
				}
				else
				{
					if ( node->is_text() == 0 )
					{
						if ( node->name != "comment" )
						{
							message("bad node = \"%s\"", node->name);
						}
					}
				}
			}

		}


		internal Vendor parse_vendor_node( Xml.Node node )
		{
			string? name     = XmlUtil.get_prop_string( node, "name", null );
			string? url      = XmlUtil.get_prop_string( node, "url", null );

			Vendor vendor = Vendor( name, url );

			return vendor;
		}


	}

}
