/*  xml_label.vala
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
using libglabels;

namespace glabels
{

	namespace XmlLabel
	{

		public errordomain XmlError
		{
			OPEN_ERROR,
			UNKNOWN_MEDIA,
			PARSE_ERROR,
			SAVE_ERROR
		}

		/* TODO: pull HUGE from libxml vapi file when available. */
		const int MY_XML_PARSE_HUGE = 1 << 19;


		public Label open_file( string utf8_filename ) throws XmlError
		{

			string filename;
			try
			{
				filename = Filename.from_utf8( utf8_filename, -1, null, null );
			}
			catch ( ConvertError e )
			{
				throw new XmlError.OPEN_ERROR( "Utf8 filename conversion error." );
			}

			unowned Xml.Doc* doc = Xml.Parser.read_file( filename, null, MY_XML_PARSE_HUGE );
			if ( doc == null )
			{
				throw new XmlError.PARSE_ERROR( "xmlReadFile error." );
			}

			/* TODO:
			xmlXIncludeProcess (doc);
			xmlReconciliateNs (doc, xmlDocGetRootElement (doc));
			*/

			Label label = parse_doc( doc );
			label.filename = filename;

			return label;
		}


		public Label open_buffer( string buffer ) throws XmlError
		{

			unowned Xml.Doc* doc = Xml.Parser.read_doc( buffer, null, null, MY_XML_PARSE_HUGE );
			if ( doc == null )
			{
				throw new XmlError.PARSE_ERROR( "xmlReadDoc error." );
			}

			Label? label = parse_doc( doc );

			return label;
		}


		private Label parse_doc( Xml.Doc doc ) throws XmlError
		{
			unowned Xml.Node* root = doc.get_root_element();
			if ( (root == null) || (root->name == null) ) {
				throw new XmlError.PARSE_ERROR( "No document root." );
			}

#if TODO
			/* Try compatability mode 0.4 */
			if (xmlSearchNsByHref (doc, root, (xmlChar *)COMPAT04_NAME_SPACE))
			{
				message( "Importing from glabels 0.4 format" );
				return gl_xml_label_04_parse( root, status );
			}

			/* Test for current namespaces. */
			if ( !xmlSearchNsByHref (doc, root, (xmlChar *)COMPAT20_NAME_SPACE) &&
			     !xmlSearchNsByHref (doc, root, (xmlChar *)COMPAT22_NAME_SPACE) &&
			     !xmlSearchNsByHref (doc, root, (xmlChar *)LGL_XML_NAME_SPACE) )
			{
				message( "Unknown glabels Namespace -- Using %s", XML_NAME_SPACE );
			}
#endif

			if ( root->name != "Glabels-document" )
			{
				throw new XmlError.PARSE_ERROR( "Root node != \"Glabels-document\"." );
			}

			Label label = parse_glabels_document_node( root );
			label.compression = doc.get_compress_mode();

			return label;
		}


		private Label parse_glabels_document_node( Xml.Node node ) throws XmlError
		{
			Label label = new Label();

			/* Pass 1, extract data nodes to pre-load cache. */
			for ( unowned Xml.Node* child = node.children; child != null; child = child->next )
			{
				if ( child->name == "Data" )
				{
					/* TODO: xml_parse_data (child_node, label); */
				}
			}

			/* Pass 2, now extract everything else. */
			for ( unowned Xml.Node* child = node.children; child != null; child = child->next )
			{
				switch (child->name)
				{

				case "Template":
					Template? template = XmlTemplate.parse_template_node( child );
					if ( template == null )
					{
						throw new XmlError.PARSE_ERROR( "Bad template." );
					}
					label.template = template;
					break;

				case "Objects":
					parse_objects_node( child, label );
					break;

				case "Merge":
					parse_merge_node( child, label );
					break;

				case "Data":
					/* Handled in pass 1. */
					break;

				default:
					if ( child->is_text() == 0 )
					{
						message( "Unexpected %s child: \"%s\"", node.name, child->name );
					}
					break;
				}
			}

			return label;
		}


		private void parse_objects_node( Xml.Node node,
		                                 Label    label )
		{
			label.rotate = XmlUtil.get_prop_bool( node, "rotate", false );

			for ( unowned Xml.Node* child = node.children; child != null; child = child->next )
			{
				switch (child->name)
				{

				case "Object-text":
					/* TODO. */
					break;

				case "Object-box":
					parse_object_box_node( child, label );
					break;

				case "Object-ellipse":
					/* TODO. */
					break;

				case "Object-line":
					/* TODO. */
					break;

				case "Object-image":
					/* TODO. */
					break;

				case "Object-barcode":
					/* TODO. */
					break;

				default:
					if ( child->is_text() == 0 )
					{
						message( "Unexpected %s child: \"%s\"", node.name, child->name );
					}
					break;
				}
			}

		}


		private void parse_object_box_node( Xml.Node node,
		                                    Label    label )
		{
			LabelObjectBox object = new LabelObjectBox();

		
			/* position attrs */
			object.x0 = XmlUtil.get_prop_length( node, "x", 0.0 );
			object.y0 = XmlUtil.get_prop_length( node, "y", 0.0 );

			/* size attrs */
			object.w = XmlUtil.get_prop_length( node, "w", 0 );
			object.h = XmlUtil.get_prop_length( node, "h", 0 );

			/* line attrs */
			object.line_width = XmlUtil.get_prop_length( node, "line_width", 1.0 );
	
			{
				string key        = XmlUtil.get_prop_string( node, "line_color_field", null );
				bool   field_flag = key != null;
				Color  color      = Color.from_legacy_color( XmlUtil.get_prop_uint( node, "line_color", 0 ) );
				object.line_color_node = ColorNode( field_flag, color, key );
			}

			/* fill attrs */
			{
				string key        = XmlUtil.get_prop_string( node, "fill_color_field", null );
				bool   field_flag = key != null;
				Color  color      = Color.from_legacy_color( XmlUtil.get_prop_uint( node, "fill_color", 0 ) );
				object.fill_color_node = ColorNode( field_flag, color, key );
			}
	
			/* affine attrs */
			parse_affine_attrs( node, object );

			/* shadow attrs */
			parse_shadow_attrs( node, object );

			label.add_object( object );
		}


		private void parse_affine_attrs( Xml.Node    node,
		                                 LabelObject object )
		{
			double           a[6];

			a[0] = XmlUtil.get_prop_double( node, "a0", 0.0 );
			a[1] = XmlUtil.get_prop_double( node, "a1", 0.0 );
			a[2] = XmlUtil.get_prop_double( node, "a2", 0.0 );
			a[3] = XmlUtil.get_prop_double( node, "a3", 0.0 );
			a[4] = XmlUtil.get_prop_double( node, "a4", 0.0 );
			a[5] = XmlUtil.get_prop_double( node, "a5", 0.0 );

			object.matrix = Cairo.Matrix( a[0], a[1], a[2], a[3], a[4], a[5] );
		}


		private void parse_shadow_attrs( Xml.Node    node,
		                                 LabelObject object )
		{
			object.shadow_state = XmlUtil.get_prop_bool( node, "shadow", false );

			if (object.shadow_state)
			{
				object.shadow_x = XmlUtil.get_prop_length( node, "shadow_x", 0.0 );
				object.shadow_y = XmlUtil.get_prop_length( node, "shadow_y", 0.0 );
		
				string key        = XmlUtil.get_prop_string( node, "shadow_color_field", null );
				bool   field_flag = key != null;
				Color  color      = Color.from_legacy_color( XmlUtil.get_prop_uint( node, "shadow_color", 0 ) );
				object.shadow_color_node = ColorNode( field_flag, color, key );

				object.shadow_opacity = XmlUtil.get_prop_double( node, "shadow_opacity", 1.0 );
			}
		}


		private void parse_merge_node( Xml.Node  node,
		                               Label     label )
		{
			Merge merge = MergeFactory.create_merge( XmlUtil.get_prop_string( node, "type", null ) );

			merge.src  = XmlUtil.get_prop_string( node, "src", null );

			label.merge = merge;
		}


		public void save_file( Label label,
		                       string utf8_filename ) throws XmlError
		{
			Xml.Doc doc = create_doc( label );

			string filename;
			try
			{
				filename = Filename.from_utf8( utf8_filename, -1, null, null );
			}
			catch ( ConvertError e )
			{
				throw new XmlError.OPEN_ERROR( "Utf8 filename conversion error." );
			}

			if ( doc.save_format_file( filename, 1 ) < 0 )
			{
				throw new XmlError.SAVE_ERROR( "Problem saving xml file." );
			}

			label.filename = utf8_filename;
			label.modified = false;
		}


		public void save_buffer( Label      label,
		                         out string buffer ) throws XmlError
		{
			Xml.Doc doc = create_doc( label );

			int length;
			doc.dump_memory( out buffer, out length );
			if ( length <= 0 )
			{
				throw new XmlError.SAVE_ERROR( "Problem saving xml buffer." );
			}

			label.modified = false;
		}


		private Xml.Doc create_doc( Label label )
		{
			Xml.Doc doc = new Xml.Doc( "1.0" );
			unowned Xml.Node* root_node = new Xml.Node( null, "Glabels-document" );
			doc.set_root_element( root_node );
			unowned Xml.Ns *ns = new Xml.Ns( root_node, NAME_SPACE, null );
			root_node->ns = ns;

			XmlTemplate.create_template_node( label.template, root_node, ns );

			create_objects_node( root_node, ns, label );

			if ( !(label.merge is MergeNone) )
			{
				create_merge_node( root_node, ns, label );
			}

			create_data_node( doc, root_node, ns, label );

			return doc;
		}


		private void create_objects_node( Xml.Node root,
		                                  Xml.Ns   ns,
		                                  Label    label )
		{
			unowned Xml.Node *node = root.new_child( ns, "Objects" );

			XmlUtil.set_prop_string( node, "id", "0" );
			XmlUtil.set_prop_bool( node, "rotate", label.rotate );

			foreach ( LabelObject object in label.object_list )
			{
				if ( object is LabelObjectBox )
				{
					create_object_box_node( node, ns, (LabelObjectBox)object );
				}
				else /* TODO: other object types. */
				{
					message( "Unknown label object." );
				}
			}
		}


		private void create_object_box_node( Xml.Node       parent,
		                                     Xml.Ns         ns,
		                                     LabelObjectBox object )
		{
			unowned Xml.Node *node = parent.new_child( ns, "Object-box" );

			/* position attrs */
			XmlUtil.set_prop_length( node, "x", object.x0 );
			XmlUtil.set_prop_length( node, "y", object.y0 );

			/* size attrs */
			XmlUtil.set_prop_length( node, "w", object.w );
			XmlUtil.set_prop_length( node, "h", object.h );

			/* line attrs */
			XmlUtil.set_prop_length( node, "line_width", object.line_width );
			if ( object.line_color_node.field_flag )
			{
				XmlUtil.set_prop_string( node, "line_color_field", object.line_color_node.key );
			}
			else
			{
				XmlUtil.set_prop_uint_hex( node, "line_color", object.line_color_node.color.to_legacy_color() );
			}

			/* fill attrs */
			if ( object.fill_color_node.field_flag )
			{
				XmlUtil.set_prop_string( node, "fill_color_field", object.fill_color_node.key );
			}
			else
			{
				XmlUtil.set_prop_uint_hex( node, "fill_color", object.fill_color_node.color.to_legacy_color() );
			}

			/* affine attrs */
			create_affine_attrs( node, object );

			/* shadow attrs */
			create_shadow_attrs( node, object );
		}


		private void create_affine_attrs( Xml.Node    node,
		                                  LabelObject object )
		{
			XmlUtil.set_prop_double( node, "a0", object.matrix.xx );
			XmlUtil.set_prop_double( node, "a1", object.matrix.yx );
			XmlUtil.set_prop_double( node, "a2", object.matrix.xy );
			XmlUtil.set_prop_double( node, "a3", object.matrix.yy );
			XmlUtil.set_prop_double( node, "a4", object.matrix.x0 );
			XmlUtil.set_prop_double( node, "a5", object.matrix.y0 );
		}


		private void create_shadow_attrs( Xml.Node    node,
		                                  LabelObject object )
		{
			if ( object.shadow_state )
			{
				XmlUtil.set_prop_bool( node, "shadow", object.shadow_state );

				XmlUtil.set_prop_length( node, "shadow_x", object.shadow_x );
				XmlUtil.set_prop_length( node, "shadow_y", object.shadow_y );

				if ( object.shadow_color_node.field_flag )
				{
					XmlUtil.set_prop_string( node, "shadow_color_field", object.shadow_color_node.key );
				}
				else
				{
					XmlUtil.set_prop_uint_hex( node, "shadow_color", object.shadow_color_node.color.to_legacy_color() );
				}

				XmlUtil.set_prop_double( node, "shadow_opacity", object.shadow_opacity );

			}
		}


		private void create_merge_node( Xml.Node root,
		                                Xml.Ns   ns,
		                                Label    label )
		{
			unowned Xml.Node *node = root.new_child( ns, "Merge" );

			XmlUtil.set_prop_string( node, "type", label.merge.name );
			XmlUtil.set_prop_string( node, "src", label.merge.src );
		}


		private void create_data_node( Xml.Doc  doc,
		                               Xml.Node root,
		                               Xml.Ns   ns,
		                               Label    label )
		{
			unowned Xml.Node *node = root.new_child( ns, "Data" );

			/* TODO */
		}

	}

}

