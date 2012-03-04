/*  xml_template.vala
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

	public const string NAME_SPACE = "http://glabels.org/xmlns/3.0/";

	namespace XmlTemplate
	{

		internal void read_templates_from_file( string utf8_filename )
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
				message("\"%s\" is not a glabels template file (not XML)", filename);
				return;
			}

			parse_templates_doc( doc );
		}


		internal void parse_templates_doc( Xml.Doc doc )
		{
			unowned Xml.Node* root = doc.get_root_element();
			if ( (root == null) || (root->name == null) )
			{
				message("\"%s\" is not a glabels template file (no root node)", doc.name);
				return;
			}

			if ( root->name != "Glabels-templates" )
			{
				message ("\"%s\" is not a glabels template file (wrong root node)", doc.name);
				return;
			}

			for ( unowned Xml.Node* node = root->children; node != null; node = node->next )
			{
				if ( node->name == "Template" )
				{
					Template template = parse_template_node( node );
					Db.register_template( template );
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


		public Template? parse_template_node( Xml.Node template_node )
		{
			Template? template = null;

			string? brand = XmlUtil.get_prop_string( template_node, "brand", null );
			string? part  = XmlUtil.get_prop_string( template_node, "part", null );

			if ( (brand == null) || (part == null) )
			{
				/* Try the deprecated "name" property. */
				string? name = XmlUtil.get_prop_string( template_node, "name", null );
				if ( name != null )
				{
					string[] fields = name.split( " ", 2 );
					brand = fields[0];
					part  = fields[1];
				}
				else
				{
					message("Missing name or brand/part attributes.");
					return null;
				}
			}


			string? equiv_part = XmlUtil.get_prop_string( template_node, "equiv", null );

			if ( equiv_part != null )
			{

				template = new Template.from_equiv( brand, part, equiv_part );

				for ( unowned Xml.Node* node = template_node.children; node != null; node = node->next )
				{
					if ( node->name == "Meta" )
					{
						parse_meta_node( node, template );
					}
					else if ( node->is_text() == 0 )
					{
						if ( node->name != "comment" )
						{
							message("bad node = \"%s\"", node->name);
						}
					}
				}

			}
			else
			{

				string? description = XmlUtil.get_prop_i18n_string( template_node, "description", null );
				string? paper_id    = XmlUtil.get_prop_string( template_node, "size", null );

				if ( !Db.is_paper_id_other( paper_id ) )
				{
					Paper? paper = Db.lookup_paper_from_id( paper_id );
					if ( paper == null )
					{
						message("Invalid paper ID \"%s\".", paper_id );
						return null;
					}

					template = new Template( brand, part, description,
					                         paper.id, paper.width, paper.height );
				}
				else
				{
					double width  = XmlUtil.get_prop_length( template_node, "width", 0 );
					double height = XmlUtil.get_prop_length( template_node, "height", 0 );
					template = new Template( brand, part, description,
					                         paper_id, width, height );
				}

				for ( unowned Xml.Node* node = template_node.children; node != null; node = node->next )
				{
					if ( node->name == "Meta" )
					{
						parse_meta_node( node, template );
					}
					else if ( node->name == "Label-rectangle" )
					{
						parse_label_rectangle_node( node, template );
					}
					else if ( node->name == "Label-ellipse" )
					{
						parse_label_ellipse_node( node, template );
					}
					else if ( node->name == "Label-round" )
					{
						parse_label_round_node( node, template );
					}
					else if ( node->name == "Label-cd" )
					{
						parse_label_cd_node( node, template );
					}
					else if ( node->is_text() == 0 )
					{
						if ( node->name != "comment" )
						{
							message("bad node = \"%s\"", node->name);
						}
					}
				}

				template.init_preview_pixbuf();

			}

			return template;
		}


		private void parse_meta_node( Xml.Node meta_node,
		                              Template template )
		{
			string? product_url = XmlUtil.get_prop_string( meta_node, "product_url", null );
			if ( product_url != null )
			{
				template.product_url = product_url;
			}
			
			string? category_id = XmlUtil.get_prop_string( meta_node, "category", null );
			if ( category_id != null )
			{
				template.add_category( category_id );
			}

			parse_empty_node_common( meta_node );
		}


		private void parse_label_rectangle_node( Xml.Node label_node,
		                                         Template template )
		{
			double x_waste, y_waste;

			string? id = XmlUtil.get_prop_string( label_node, "id", null );

			double w = XmlUtil.get_prop_length( label_node, "width", 0 );
			double h = XmlUtil.get_prop_length( label_node, "height", 0 );
			double r = XmlUtil.get_prop_length( label_node, "round", 0 );

			if ( XmlUtil.get_prop_string( label_node, "waste", null ) != null )
			{
				x_waste = y_waste = XmlUtil.get_prop_length( label_node, "waste", 0 );
			}
			else
			{
				x_waste = XmlUtil.get_prop_length( label_node, "x_waste", 0 );
				y_waste = XmlUtil.get_prop_length( label_node, "y_waste", 0 );
			}

			TemplateFrame frame = new TemplateFrameRect( id, w, h, r, x_waste, y_waste );
			template.add_frame( frame );

			parse_label_node_common( label_node, frame );
		}


		private void parse_label_ellipse_node( Xml.Node label_node,
		                                       Template template )
		{
			string? id = XmlUtil.get_prop_string( label_node, "id", null );

			double w     = XmlUtil.get_prop_length( label_node, "width", 0 );
			double h     = XmlUtil.get_prop_length( label_node, "height", 0 );
			double waste = XmlUtil.get_prop_length( label_node, "waste", 0 );

			TemplateFrame frame = new TemplateFrameEllipse( id, w, h, waste );
			template.add_frame( frame );

			parse_label_node_common( label_node, frame );
		}


		private void parse_label_round_node( Xml.Node label_node,
		                                     Template template )
		{
			string? id = XmlUtil.get_prop_string( label_node, "id", null );

			double r     = XmlUtil.get_prop_length( label_node, "radius", 0 );
			double waste = XmlUtil.get_prop_length( label_node, "waste", 0 );

			TemplateFrame frame = new TemplateFrameRound( id, r, waste );
			template.add_frame( frame );

			parse_label_node_common( label_node, frame );
		}


		private void parse_label_cd_node( Xml.Node label_node,
		                                  Template template )
		{
			string? id = XmlUtil.get_prop_string( label_node, "id", null );

			double r1    = XmlUtil.get_prop_length( label_node, "radius", 0 );
			double r2    = XmlUtil.get_prop_length( label_node, "hole", 0 );
			double w     = XmlUtil.get_prop_length( label_node, "width", 0 );
			double h     = XmlUtil.get_prop_length( label_node, "height", 0 );
			double waste = XmlUtil.get_prop_length( label_node, "waste", 0 );

			TemplateFrame frame = new TemplateFrameCD( id, r1, r2, w, h, waste );
			template.add_frame( frame );

			parse_label_node_common( label_node, frame );
		}


		private void parse_label_node_common( Xml.Node      label_node,
		                                      TemplateFrame frame )
		{
			for ( unowned Xml.Node* node = label_node.children; node != null; node = node->next )
			{
				if ( node->name == "Layout" )
				{
					parse_layout_node( node, frame );
				}
				else if ( node->name == "Markup-margin" )
				{
					parse_markup_margin_node( node, frame );
				}
				else if ( node->name == "Markup-line" )
				{
					parse_markup_line_node( node, frame );
				}
				else if ( node->name == "Markup-circle" )
				{
					parse_markup_circle_node( node, frame );
				}
				else if ( node->name == "Markup-rect" )
				{
					parse_markup_rect_node( node, frame );
				}
				else if ( node->name == "Markup-ellipse" )
				{
					parse_markup_ellipse_node( node, frame );
				}
				else if ( node->is_text() == 0 )
				{
					if ( node->name != "comment" )
					{
						message("bad node = \"%s\"", node->name);
					}
				}

			}
		}


		private void parse_layout_node( Xml.Node      label_node,
		                                TemplateFrame frame )
		{
			int nx    = XmlUtil.get_prop_int( label_node, "nx", 1 );
			int ny    = XmlUtil.get_prop_int( label_node, "ny", 1 );

			double x0 = XmlUtil.get_prop_length( label_node, "x0", 0 );
			double y0 = XmlUtil.get_prop_length( label_node, "y0", 0 );

			double dx = XmlUtil.get_prop_length( label_node, "dx", 0 );
			double dy = XmlUtil.get_prop_length( label_node, "dy", 0 );

			frame.add_layout( new TemplateLayout( nx, ny, x0, y0, dx, dy ) );

			parse_empty_node_common( label_node );
		}


		private void parse_markup_margin_node( Xml.Node      label_node,
		                                       TemplateFrame frame )
		{
			double size = XmlUtil.get_prop_length( label_node, "size", 0 );

			frame.add_markup( new TemplateMarkupMargin( size ) );

			parse_empty_node_common( label_node );
		}


		private void parse_markup_line_node( Xml.Node      label_node,
		                                     TemplateFrame frame )
		{
			double x1 = XmlUtil.get_prop_length( label_node, "x1", 0 );
			double y1 = XmlUtil.get_prop_length( label_node, "y1", 0 );
			double x2 = XmlUtil.get_prop_length( label_node, "x2", 0 );
			double y2 = XmlUtil.get_prop_length( label_node, "y2", 0 );

			frame.add_markup( new TemplateMarkupLine( x1, y1, x2, y2 ) );

			parse_empty_node_common( label_node );
		}


		private void parse_markup_circle_node( Xml.Node      label_node,
		                                       TemplateFrame frame )
		{
			double x0 = XmlUtil.get_prop_length( label_node, "x0", 0 );
			double y0 = XmlUtil.get_prop_length( label_node, "y0", 0 );
			double r  = XmlUtil.get_prop_length( label_node, "radius", 0 );

			frame.add_markup( new TemplateMarkupCircle( x0, y0, r ) );

			parse_empty_node_common( label_node );
		}


		private void parse_markup_rect_node( Xml.Node      label_node,
		                                     TemplateFrame frame )
		{
			double x1 = XmlUtil.get_prop_length( label_node, "x1", 0 );
			double y1 = XmlUtil.get_prop_length( label_node, "y1", 0 );
			double w  = XmlUtil.get_prop_length( label_node, "w", 0 );
			double h  = XmlUtil.get_prop_length( label_node, "h", 0 );
			double r  = XmlUtil.get_prop_length( label_node, "r", 0 );

			frame.add_markup( new TemplateMarkupRect( x1, y1, w, h, r ) );

			parse_empty_node_common( label_node );
		}


		private void parse_markup_ellipse_node( Xml.Node      label_node,
		                                        TemplateFrame frame )
		{
			double x1 = XmlUtil.get_prop_length( label_node, "x1", 0 );
			double y1 = XmlUtil.get_prop_length( label_node, "y1", 0 );
			double w  = XmlUtil.get_prop_length( label_node, "w", 0 );
			double h  = XmlUtil.get_prop_length( label_node, "h", 0 );

			frame.add_markup( new TemplateMarkupEllipse( x1, y1, w, h ) );

			parse_empty_node_common( label_node );
		}


		private void parse_empty_node_common( Xml.Node empty_node )
		{
			for ( unowned Xml.Node* node = empty_node.children; node != null; node = node->next )
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


		internal int write_templates_to_file( List<Template> templates,
		                                      string         utf8_filename )
		{
			Xml.Doc doc = new Xml.Doc( "1.0" );
			unowned Xml.Node *root_node = new Xml.Node(null, "Glabels-templates");
			doc.set_root_element( root_node );
			unowned Xml.Ns *ns = new Xml.Ns( root_node, NAME_SPACE, "glabels" );
			root_node->ns = ns;

			foreach ( Template template in templates )
			{
				create_template_node( template, root_node, ns );
			}

			string filename;
			try
			{
				filename = Filename.from_utf8( utf8_filename, -1, null, null );
			}
			catch ( ConvertError e )
			{
				message("Utf8 filename conversion: %s", e.message);
				return 0;
			}

			doc.set_compress_mode( 0 );
			return doc.save_format_file( filename, 1 );
		}


		internal int write_template_to_file( Template template,
		                                     string   utf8_filename )
		{
			List<Template> templates = null;

			templates.append( template );

			return write_templates_to_file( templates, utf8_filename );
		}


		public void create_template_node( Template template,
		                                  Xml.Node root,
		                                  Xml.Ns   ns )
		{
			unowned Xml.Node *node = root.new_child( ns, "Template" );

			XmlUtil.set_prop_string( node, "brand", template.brand );
			XmlUtil.set_prop_string( node, "part",  template.part );

			XmlUtil.set_prop_string( node, "size", template.paper_id );
			if ( template.paper_id == "Other" )
			{
				XmlUtil.set_prop_length( node, "width",  template.page_width );
				XmlUtil.set_prop_length( node, "height", template.page_height );
			}

			XmlUtil.set_prop_string( node, "description", template.description );

			create_meta_node( "product_url", template.product_url, node, ns );
			foreach ( string category_id in template.category_ids )
			{
				create_meta_node( "category", category_id, node, ns );
			}

			foreach ( TemplateFrame frame in template.frames )
			{
				create_label_node( frame, node, ns );
			}
		}


		private void create_meta_node( string    attr,
		                               string?   value,
		                               Xml.Node  root,
		                               Xml.Ns    ns )
		{
			if ( value != null )
			{
				unowned Xml.Node *node = root.new_child( ns, "Meta" );
				XmlUtil.set_prop_string( node, attr, value );
			}
		}


		private void create_label_node( TemplateFrame frame,
		                                Xml.Node      root,
		                                Xml.Ns        ns )
		{
			if ( frame is TemplateFrameRect )
			{
				create_label_rectangle_node( (TemplateFrameRect)frame, root, ns );
			}
			else if ( frame is TemplateFrameEllipse )
			{
				create_label_ellipse_node( (TemplateFrameEllipse)frame, root, ns );
			}
			else if ( frame is TemplateFrameRound )
			{
				create_label_round_node( (TemplateFrameRound)frame, root, ns );
			}
			else if ( frame is TemplateFrameCD )
			{
				create_label_cd_node( (TemplateFrameCD)frame, root, ns );
			}
			else
			{
				error( "Unknown label style" );
			}

		}


		private void create_label_rectangle_node( TemplateFrameRect frame,
		                                          Xml.Node          root,
		                                          Xml.Ns            ns )
		{
			unowned Xml.Node *node = root.new_child( ns, "Label-rectangle" );

			XmlUtil.set_prop_string( node, "id",      frame.id );
			XmlUtil.set_prop_length( node, "width",   frame.w );
			XmlUtil.set_prop_length( node, "height",  frame.h );
			XmlUtil.set_prop_length( node, "round",   frame.r );
			XmlUtil.set_prop_length( node, "x_waste", frame.x_waste );
			XmlUtil.set_prop_length( node, "y_waste", frame.y_waste );

			create_label_node_common( frame, node, ns );
		}


		private void create_label_ellipse_node( TemplateFrameEllipse frame,
		                                        Xml.Node             root,
		                                        Xml.Ns               ns )
		{
			unowned Xml.Node *node = root.new_child( ns, "Label-ellipse" );

			XmlUtil.set_prop_string( node, "id",      frame.id );
			XmlUtil.set_prop_length( node, "width",   frame.w );
			XmlUtil.set_prop_length( node, "height",  frame.h );
			XmlUtil.set_prop_length( node, "waste",   frame.waste );

			create_label_node_common( frame, node, ns );
		}


		private void create_label_round_node( TemplateFrameRound frame,
		                                      Xml.Node           root,
		                                      Xml.Ns             ns )
		{
			unowned Xml.Node *node = root.new_child( ns, "Label-round" );

			XmlUtil.set_prop_string( node, "id",      frame.id );
			XmlUtil.set_prop_length( node, "radius",  frame.r );
			XmlUtil.set_prop_length( node, "waste",   frame.waste );

			create_label_node_common( frame, node, ns );
		}


		private void create_label_cd_node( TemplateFrameCD frame,
		                                   Xml.Node        root,
		                                   Xml.Ns          ns )
		{
			unowned Xml.Node *node = root.new_child( ns, "Label-cd" );

			XmlUtil.set_prop_string( node, "id",      frame.id );
			XmlUtil.set_prop_length( node, "radius",  frame.r1 );
			XmlUtil.set_prop_length( node, "hole",    frame.r2 );
			XmlUtil.set_prop_length( node, "waste",   frame.waste );
			if ( frame.w != 0 )
			{
				XmlUtil.set_prop_length( node, "width",   frame.w );
			}
			if ( frame.h != 0 )
			{
				XmlUtil.set_prop_length( node, "height",  frame.h );
			}

			create_label_node_common( frame, node, ns );
		}


		private void create_label_node_common( TemplateFrame   frame,
		                                       Xml.Node        node,
		                                       Xml.Ns          ns )
		{
			foreach ( TemplateMarkup markup in frame.markups )
			{
				if ( markup is TemplateMarkupMargin )
				{
					create_markup_margin_node( (TemplateMarkupMargin)markup, node, ns );
				}
				else if ( markup is TemplateMarkupLine )
				{
					create_markup_line_node( (TemplateMarkupLine)markup, node, ns );
				}
				else if ( markup is TemplateMarkupCircle )
				{
					create_markup_circle_node( (TemplateMarkupCircle)markup, node, ns );
				}
				else if ( markup is TemplateMarkupRect )
				{
					create_markup_rect_node( (TemplateMarkupRect)markup, node, ns );
				}
				else if ( markup is TemplateMarkupEllipse )
				{
					create_markup_ellipse_node( (TemplateMarkupEllipse)markup, node, ns );
				}
				else
				{
					error( "Unknown markup type" );
				}
			}

			foreach ( TemplateLayout layout in frame.layouts )
			{
				create_layout_node( layout, node, ns );
			}
		}


		private void create_markup_margin_node( TemplateMarkupMargin   markup,
		                                        Xml.Node               root,
		                                        Xml.Ns                 ns )
		{
			unowned Xml.Node *node = root.new_child( ns, "Markup-margin" );

			XmlUtil.set_prop_length( node, "size", markup.size );
		}


		private void create_markup_line_node( TemplateMarkupLine   markup,
		                                      Xml.Node             root,
		                                      Xml.Ns               ns )
		{
			unowned Xml.Node *node = root.new_child( ns, "Markup-line" );

			XmlUtil.set_prop_length( node, "x1", markup.x1 );
			XmlUtil.set_prop_length( node, "y1", markup.y1 );
			XmlUtil.set_prop_length( node, "x2", markup.x2 );
			XmlUtil.set_prop_length( node, "y2", markup.y2 );
		}


		private void create_markup_circle_node( TemplateMarkupCircle   markup,
		                                        Xml.Node               root,
		                                        Xml.Ns                 ns )
		{
			unowned Xml.Node *node = root.new_child( ns, "Markup-circle" );

			XmlUtil.set_prop_length( node, "x0",     markup.x0 );
			XmlUtil.set_prop_length( node, "y0",     markup.y0 );
			XmlUtil.set_prop_length( node, "radius", markup.r );
		}


		private void create_markup_rect_node( TemplateMarkupRect   markup,
		                                      Xml.Node             root,
		                                      Xml.Ns               ns )
		{
			unowned Xml.Node *node = root.new_child( ns, "Markup-rect" );

			XmlUtil.set_prop_length( node, "x1", markup.x1 );
			XmlUtil.set_prop_length( node, "y1", markup.y1 );
			XmlUtil.set_prop_length( node, "w",  markup.w );
			XmlUtil.set_prop_length( node, "h",  markup.h );
			XmlUtil.set_prop_length( node, "r",  markup.r );
		}


		private void create_markup_ellipse_node( TemplateMarkupEllipse  markup,
		                                         Xml.Node               root,
		                                         Xml.Ns                 ns )
		{
			unowned Xml.Node *node = root.new_child( ns, "Markup-ellipse" );

			XmlUtil.set_prop_length( node, "x1", markup.x1 );
			XmlUtil.set_prop_length( node, "y1", markup.y1 );
			XmlUtil.set_prop_length( node, "w",  markup.w );
			XmlUtil.set_prop_length( node, "h",  markup.h );
		}


		private void create_layout_node( TemplateLayout  layout,
		                                 Xml.Node        root,
		                                 Xml.Ns          ns )
		{
			unowned Xml.Node *node = root.new_child( ns, "Layout" );

			XmlUtil.set_prop_int(    node, "nx", layout.nx );
			XmlUtil.set_prop_int(    node, "ny", layout.ny );
			XmlUtil.set_prop_length( node, "x0", layout.x0 );
			XmlUtil.set_prop_length( node, "y0", layout.y0 );
			XmlUtil.set_prop_length( node, "dx", layout.dx );
			XmlUtil.set_prop_length( node, "dy", layout.dy );
		}


	}

}
