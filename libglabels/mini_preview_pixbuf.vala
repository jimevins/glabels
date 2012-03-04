/*  mini_preview_pixbuf.vala
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

	private struct ColorStruct
	{
		double r;
		double g;
		double b;
	}

	private const ColorStruct paper         = { 0.85,  0.85,  0.85 };
	private const ColorStruct paper_outline = { 0.0,   0.0,   0.0  };
	private const ColorStruct label         = { 0.95,  0.95,  0.95  };
	private const ColorStruct label_outline = { 0.25,  0.25,  0.25 };

	private const double paper_outline_width_pixels = 1.0;
	private const double label_outline_width_pixels = 1.0;


	public class MiniPreviewPixbuf
	{
		public Gdk.Pixbuf pixbuf { get; private set; }

		public MiniPreviewPixbuf( Template template,
		                          int      width,
		                          int      height )
		{
			pixbuf = new Gdk.Pixbuf( Gdk.Colorspace.RGB, true, 8, width, height );

			Cairo.ImageSurface surface = new Cairo.ImageSurface.for_data( pixbuf.get_pixels(),
			                                                              Cairo.Format.RGB24,
			                                                              pixbuf.get_width(),
			                                                              pixbuf.get_height(),
			                                                              pixbuf.get_rowstride() );
			Cairo.Context cr = new Cairo.Context( surface );

			/* Clear pixbuf */
			cr.save();
			cr.set_operator( Cairo.Operator.CLEAR );
			cr.paint();
			cr.restore();

			cr.set_antialias( Cairo.Antialias.GRAY );

			/* Set scale and offset */
			double w = width - 1;
			double h = height - 1;
			double scale;
			if ( (w/template.page_width) > (h/template.page_height) )
			{
				scale = h / template.page_height;
			}
			else
			{
				scale = w / template.page_width;
			}
			double offset_x = (width/scale - template.page_width) / 2.0;
			double offset_y = (height/scale - template.page_height) / 2.0;
			cr.identity_matrix();
			cr.scale( scale, scale );
			cr.translate( offset_x, offset_y );

			/* Draw paper and label outlines */
			draw_paper( cr, template, scale );
			draw_label_outlines( cr, template, scale );
		}


		private void draw_paper( Cairo.Context cr,
		                         Template      template,
		                         double        scale )
		{
			cr.save();

			cr.rectangle( 0, 0, template.page_width, template.page_height );

			cr.set_source_rgb( paper.r, paper.g, paper.b );
			cr.fill_preserve();

			cr.set_line_width( paper_outline_width_pixels/scale );
			cr.set_source_rgb( paper_outline.r, paper_outline.g, paper_outline.b );
			cr.stroke();

			cr.restore();
		}


		private void draw_label_outlines( Cairo.Context cr,
		                                  Template      template,
		                                  double        scale )
		{
			cr.save();

			cr.set_line_width( label_outline_width_pixels/scale );

			TemplateFrame frame = template.frames.first().data;

			Gee.ArrayList<TemplateCoord?> origins = frame.get_origins();

			foreach ( TemplateCoord origin in origins )
			{
				draw_label_outline( cr, frame, origin.x, origin.y );
			}

			cr.restore();
		}


		private void draw_label_outline( Cairo.Context  cr,
		                                 TemplateFrame  frame,
		                                 double         x0,
		                                 double         y0 )
		{
			cr.save();

			cr.translate( x0, y0 );

			frame.cairo_path( cr, false );

			cr.set_source_rgb( label.r, label.g, label.b );
			cr.set_fill_rule( Cairo.FillRule.EVEN_ODD );
			cr.fill_preserve();

			cr.set_source_rgb( label_outline.r, label_outline.g, label_outline.b );
			cr.stroke();

			cr.restore();
		}


	}

}
