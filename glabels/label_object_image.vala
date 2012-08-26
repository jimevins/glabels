/*  label_object_image.vala
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

	public class LabelObjectImage : LabelObject
	{
		private const double MIN_IMAGE_SIZE = 1.0;

		private enum FileType { NONE, PIXBUF, SVG }


		private static Gdk.Pixbuf? default_pixbuf = null;


		public LabelObjectImage.with_parent( Label parent )
		{
			this();
			parent.add_object( this );
		}


		/**
		 * Filename
		 */
		public override TextNode filename_node
		{
			get { return _filename_node; }

			set
			{
				if ( (_filename_node == null) || !_filename_node.equal( value ) )
				{
					update_filename_node( value );
					changed();
				}
			}
		}
		private TextNode _filename_node;


		/**
		 * Width
		 */
		public override double w
		{
			get { return base.w; }

			set
			{
				if ( value < MIN_IMAGE_SIZE )
				{
					base.w = MIN_IMAGE_SIZE;
				}
				else
				{
					base.w = value;
				}
			}
		}


		/**
		 * Height
		 */
		public override double h
		{
			get { return base.h; }

			set
			{
				if ( value < MIN_IMAGE_SIZE )
				{
					base.h = MIN_IMAGE_SIZE;
				}
				else
				{
					base.h = value;
				}
			}
		}


		private FileType type;

		private Gdk.Pixbuf?  pixbuf;
		private Rsvg.Handle? svg_handle;
		

		public LabelObjectImage()
		{
			handles.append( new HandleSouthEast( this ) );
			handles.append( new HandleSouthWest( this ) );
			handles.append( new HandleNorthEast( this ) );
			handles.append( new HandleNorthWest( this ) );
			handles.append( new HandleEast( this ) );
			handles.append( new HandleSouth( this ) );
			handles.append( new HandleWest( this ) );
			handles.append( new HandleNorth( this ) );

			type       = FileType.NONE;
			pixbuf     = null;
			svg_handle = null;

			if ( default_pixbuf == null )
			{
				Gdk.Pixbuf pixbuf = new Gdk.Pixbuf.from_xpm_data( Pixmaps.checkerboard_xpm );
				default_pixbuf = pixbuf.scale_simple( 128, 128, Gdk.InterpType.NEAREST );
			}
		}


		public override LabelObject dup()
		{
			LabelObjectImage copy = new LabelObjectImage();

			copy.set_common_properties_from_object( this );

			return copy;
		}


		public override void draw_object( Cairo.Context cr, bool in_editor, MergeRecord? record )
		{
			cr.save();

			switch (type)
			{

			case FileType.PIXBUF:
				double image_w = pixbuf.width;
				double image_h = pixbuf.height;
				cr.rectangle( 0, 0, w, h );
				cr.scale( w/image_w, h/image_h );
				Gdk.cairo_set_source_pixbuf( cr, pixbuf, 0, 0 );
				cr.fill();
				break;

			case FileType.SVG:
				double image_w = svg_handle.width;
				double image_h = svg_handle.height;
				cr.scale( w/image_w, h/image_h );
				svg_handle.render_cairo( cr );
				break;

			default:
				double image_w = default_pixbuf.width;
				double image_h = default_pixbuf.height;
				cr.rectangle( 0, 0, w, h );
				cr.scale( w/image_w, h/image_h );
				Gdk.cairo_set_source_pixbuf( cr, default_pixbuf, 0, 0 );
				cr.fill();
				break;

			}

			cr.restore();
		}


		public override void draw_shadow( Cairo.Context cr, bool in_editor, MergeRecord? record )
		{
			Color shadow_color = shadow_color_node.expand( record );
			if ( shadow_color_node.field_flag && in_editor )
			{
				shadow_color = Color.black();
			}
			shadow_color.set_opacity( shadow_opacity );

			cr.save();

			switch (type)
			{

			case FileType.PIXBUF:
				Gdk.Pixbuf shadow_pixbuf = shadow_pixbuf( pixbuf, shadow_color );
				double image_w = shadow_pixbuf.width;
				double image_h = shadow_pixbuf.height;
				cr.rectangle( 0, 0, w, h );
				cr.scale( w/image_w, h/image_h );
				Gdk.cairo_set_source_pixbuf( cr, shadow_pixbuf, 0, 0 );
				cr.fill();
				break;

			case FileType.SVG:
				/* TODO: handle SVG shadows properly. */
				cr.rectangle( 0, 0, w, h );
				cr.set_source_rgba( shadow_color.r, shadow_color.g, shadow_color.b, shadow_color.a );
				cr.fill();
				break;

			default:
				cr.rectangle( 0, 0, w, h );
				cr.set_source_rgba( shadow_color.r, shadow_color.g, shadow_color.b, shadow_color.a );
				cr.fill();
				break;

			}

			cr.restore();
		}


		public override bool is_object_located_at( Cairo.Context cr, double x, double y )
		{
			cr.new_path();
			cr.rectangle( 0, 0, w, h );

			if ( cr.in_fill( x, y ) )
			{
				return true;
			}

			return false;
		}


		private void update_filename_node( TextNode new_node )
		{
			/* Remove reference to previous image. */
			switch (type)
			{
			case FileType.NONE:
				break;

			case FileType.PIXBUF:
				parent.pixbuf_cache.remove_pixbuf( _filename_node.data );
				break;

			case FileType.SVG:
				parent.svg_cache.remove_svg( _filename_node.data );
				break;

			default:
				assert_not_reached();
			}

			/* Update */
			_filename_node = new_node;

			/* Set the new file type and load pixbuf or SVG. */
			if ( !_filename_node.field_flag && (_filename_node.data != null) )
			{
				if ( _filename_node.data.has_suffix( ".svg" ) || _filename_node.data.has_suffix( ".SVG" ) )
				{
					svg_handle = parent.svg_cache.get_handle( _filename_node.data );
					if ( svg_handle != null )
					{
						type       = FileType.SVG;
						pixbuf     = null;
					}
					else
					{
						type       = FileType.NONE;
						pixbuf     = null;
					}
				}
				else
				{
					pixbuf     = parent.pixbuf_cache.get_pixbuf( _filename_node.data );
					if ( pixbuf != null )
					{
						type       = FileType.PIXBUF;
						svg_handle = null;
					}
					else
					{
						type       = FileType.NONE;
						svg_handle = null;
					}
				}
			}
			else
			{
				type       = FileType.NONE;
				svg_handle = null;
				pixbuf     = null;
			}

			/* Treat current size as a bounding box, scale image to maintain aspect
			 * ratio while fitting it in this bounding box. */
			double image_w, image_h;
			switch ( type )
			{
			case FileType.PIXBUF:
				image_w = pixbuf.width;
				image_h = pixbuf.height;
				break;
			case FileType.SVG:
				image_w = svg_handle.width;
				image_h = svg_handle.height;
				break;
			default:
				image_w = default_pixbuf.width;
				image_h = default_pixbuf.height;
				break;
			}
			double aspect_ratio = image_h / image_w;
			if ( h > w*aspect_ratio )
			{
				h = w * aspect_ratio;
			}
			else
			{
				w = h / aspect_ratio;
			}

		}

	}


	Gdk.Pixbuf? shadow_pixbuf ( Gdk.Pixbuf pixbuf, Color shadow_color )
	{
		uchar shadow_r = (uchar)(shadow_color.r * 255.0);
		uchar shadow_g = (uchar)(shadow_color.g * 255.0);
		uchar shadow_b = (uchar)(shadow_color.b * 255.0);

		/* extract pixels and parameters from source pixbuf. */
		uchar* buf_src         = pixbuf.pixels;
		int    bits_per_sample = pixbuf.bits_per_sample;
		int    n_channels      = pixbuf.n_channels;
		bool   src_has_alpha   = pixbuf.has_alpha;
		int    width           = pixbuf.width;
		int    height          = pixbuf.height;
		int    src_rowstride   = pixbuf.rowstride;

		/* validate assumptions about source pixbuf. */
		if ( buf_src == null) return null;
		if ( bits_per_sample != 8 ) return null;
		if ( (n_channels < 3) || (n_channels > 4) ) return null;
		if ( width <= 0 ) return null;
		if ( height <= 0 ) return null;
		if ( src_rowstride <= 0 ) return null;

		/* Allocate a destination pixbuf */
		Gdk.Pixbuf dest_pixbuf = new Gdk.Pixbuf( Gdk.Colorspace.RGB, true, bits_per_sample, width, height );
		int        dest_rowstride = dest_pixbuf.rowstride;
		uchar*     buf_dest       = dest_pixbuf.pixels;
		if ( buf_dest == null ) {
			return null;
		}

		/* Process pixels: set rgb components and composite alpha with shadow_opacity. */
		uchar* p_src  = buf_src;
		uchar* p_dest = buf_dest;
		for ( int iy=0; iy < height; iy++ )
		{
        
			p_src  = buf_src + iy*src_rowstride;
			p_dest = buf_dest + iy*dest_rowstride;

			for ( int ix=0; ix < width; ix++ )
			{

				p_src += 3; /* skip RGB */

				*p_dest++ = shadow_r;
				*p_dest++ = shadow_g;
				*p_dest++ = shadow_b;

				if ( src_has_alpha )
				{
					*p_dest++ = *p_src++ * shadow_color.a;
				}
				else
				{
					*p_dest++ = shadow_color.a * 255.0;
				}


			}

		}

		return dest_pixbuf;
	}


}
