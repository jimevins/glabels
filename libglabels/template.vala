/*  template.vala
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
using Math;

namespace libglabels
{

	private const double EPSILON = 0.5;	 /* Allowed error when comparing dimensions. (0.5pts ~= .007in ~= .2mm) */

	private const int PREVIEW_PIXBUF_SIZE = 72;


	public int compare_template_names( Template a, Template b )
	{
		return ( StrUtil.compare_part_names( a.name, b.name ) );
	}


	public class Template
	{
		public string    brand       { get; private set; }
		public string    part        { get; private set; }
		public string    equiv_part  { get; set; }

		public string    name        { get; private set; }

		public string    description { get; private set; }

		public string    paper_id    { get; private set; }
		public double    page_width  { get; private set; }
		public double    page_height { get; private set; }

		public Gdk.Pixbuf preview_pixbuf { get; private set; }


		/* Meta information */
		public string               product_url  { get; set; }
		public unowned List<string> category_ids { get; private set; }

		/* List of label frames.  Currently glabels only supports a single label frame per template. */
		public unowned List<TemplateFrame> frames { get; private set; }


		public Template( string   brand,
						 string   part,
						 string   description,
						 string   paper_id,
						 double   page_width,
						 double   page_height )
		{
			this.brand       = brand;
			this.part        = part;
			this.description = description;
			this.paper_id    = paper_id;
			this.page_width  = page_width;
			this.page_height = page_height;

			this.name       = "%s %s".printf( brand, part );
		}


		public Template.full_page( string paper_id )
		{
			Paper? paper = Db.lookup_paper_from_id( paper_id );
			if ( paper != null )
			{
				string part = "%s-Full-Page".printf( paper.id );
				string desc = _("%s full page").printf( paper.name );

				this( "Generic", part, desc, paper.id, paper.width, paper.height );

				TemplateFrame frame = new TemplateFrameRect( "0",
															 paper.width,
															 paper.height,
															 0, 0, 0 );
				frame.add_layout( new TemplateLayout( 1, 1, 0, 0, 0, 0 ) );
				frame.add_markup( new TemplateMarkupMargin( 9 ) );

				this.add_frame( frame );

				this.init_preview_pixbuf();
								  
			}
			else
			{
				error( "Cannot create full page template for \"%s\"", paper_id );
			}
		}


		public Template.from_equiv( string brand,
									string part,
									string equiv_part )
		{
			Template? template = Db.lookup_template_from_brand_part( brand, equiv_part );
			if ( template != null )
			{
				this( brand, part, template.description,
					  template.paper_id, template.page_width, template.page_height );

				this.equiv_part     = equiv_part;
				this.product_url    = template.product_url;
				this.preview_pixbuf = template.preview_pixbuf;

				foreach (string category_id in template.category_ids)
				{
					add_category( category_id );
				}

				foreach (TemplateFrame frame in template.frames)
				{
					add_frame( frame.dup() );
				}
			}
			else
			{
				error( "Cannot create equivalent template for \"%s\", \"%s\". Forward references not supported.",
					   brand, equiv_part );
			}
		}


		public Template dup()
		{
			Template copy = new Template( brand, part, description,
										  paper_id, page_width, page_height );

			copy.equiv_part     = equiv_part;
			copy.product_url    = product_url;
			copy.preview_pixbuf = preview_pixbuf;

			foreach (string category_id in category_ids)
			{
				copy.add_category( category_id );
			}

			foreach (TemplateFrame frame in frames)
			{
				copy.add_frame( frame.dup() );
			}

			return copy;
		}


		public bool is_match( Template template2 )
		{
			return ( (template2.brand == brand) && (template2.part == part) );
		}


		public bool does_category_match( string? category_id )
		{
			if ( category_id == null )
			{
				return true;
			}

			foreach ( string my_category_id in category_ids )
			{
				if ( my_category_id == category_id )
				{
					return true;
				}
			}

			return false;
		}


		public bool is_similar( Template template2 )
		{
			if ( (template2.paper_id    != paper_id)   ||
				 (template2.page_width  != page_width) ||
				 (template2.page_height != page_height) )
			{
				return false;
			}

			TemplateFrame frame1 = frames.first().data;
			TemplateFrame frame2 = template2.frames.first().data;

			if ( !frame1.is_similar( frame2 ) )
			{
				return false;
			}

			foreach ( TemplateLayout layout1 in frame1.layouts )
			{
				bool match_found = false;
				foreach ( TemplateLayout layout2 in frame2.layouts )
				{
					if ( layout1.is_similar( layout2 ) )
					{
						match_found = true;
						break;
					}
				}
				if ( !match_found )
				{
					return false;
				}

			}

			return true;
		}


		public void add_frame( TemplateFrame frame )
		{
			frames.append( frame );
		}


		public void add_category( string category_id )
		{
			category_ids.append( category_id );
		}


		public void init_preview_pixbuf()
		{
			MiniPreviewPixbuf pb = new MiniPreviewPixbuf( this, PREVIEW_PIXBUF_SIZE, PREVIEW_PIXBUF_SIZE );

			preview_pixbuf = pb.pixbuf;
		}


	}

}
