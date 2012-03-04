/*  xml_category.vala
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

	namespace XmlCategory
	{

		internal void read_categories_from_file( string utf8_filename )
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
				message("\"%s\" is not a glabels category file (not XML)", filename);
				return;
			}

			parse_categories_doc( doc );
		}


		internal void parse_categories_doc( Xml.Doc doc )
		{
			unowned Xml.Node* root = doc.get_root_element();
			if ( (root == null) || (root->name == null) )
			{
				message("\"%s\" is not a glabels category file (no root node)", doc.name);
				return;
			}

			if ( root->name != "Glabels-categories" )
			{
				message ("\"%s\" is not a glabels category file (wrong root node)", doc.name);
				return;
			}

			for ( unowned Xml.Node* node = root->children; node != null; node = node->next )
			{
				if ( node->name == "Category" )
				{
					Category category = parse_category_node( node );
					Db.register_category( category );
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


		internal Category parse_category_node( Xml.Node node )
		{
			string? id       = XmlUtil.get_prop_string( node, "id", null );
			string? name     = XmlUtil.get_prop_i18n_string( node, "name", null );

			Category category = Category( id, name );

			return category;
		}


	}

}
