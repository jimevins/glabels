/*  print.vala
 *
 *  Copyright (C) 2011  Jim Evins <evins@snaught.com>
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

	public class Print
	{
		private const double OUTLINE_WIDTH =  0.25;
		private const double TICK_OFFSET   =  2.25;
		private const double TICK_LENGTH   = 18.0;

		public Label label           { get; set; }
		public bool  outline_flag    { get; set; }
		public bool  reverse_flag    { get; set; }
		public bool  crop_marks_flag { get; set; }


		public void print_simple_sheet( Cairo.Context cr )
		{
			if ( crop_marks_flag )
			{
				print_crop_marks( cr );
			}

			TemplateFrame frame = label.template.frames.first().data;
			Gee.ArrayList<TemplateCoord?> origins = frame.get_origins();

			foreach ( TemplateCoord origin in origins )
			{
				print_label( cr, origin.x, origin.y, null );
			}
		}


		private void print_crop_marks( Cairo.Context cr )
		{
			TemplateFrame frame = label.template.frames.first().data;

			double w, h;
			frame.get_size( out w, out h );

			cr.save();

			cr.set_source_rgb( 0, 0, 0 );
			cr.set_line_width( OUTLINE_WIDTH );

			foreach ( TemplateLayout layout in frame.layouts )
			{

				double xmin = layout.x0;
				double ymin = layout.y0;
				double xmax = layout.x0 + layout.dx*(layout.nx - 1) + w;
				double ymax = layout.y0 + layout.dy*(layout.ny - 1) + h;

				for ( int ix=0; ix < layout.nx; ix++ )
				{
					double x1 = xmin + ix*layout.dx;
					double x2 = x1 + w;

					double y1 = double.max((ymin - TICK_OFFSET), 0.0);
					double y2 = double.max((y1 - TICK_LENGTH), 0.0);

					double y3 = double.min((ymax + TICK_OFFSET), label.template.page_height);
					double y4 = double.min((y3 + TICK_LENGTH), label.template.page_height);

					cr.move_to( x1, y1 );
					cr.line_to( x1, y2 );
					cr.stroke();

					cr.move_to( x2, y1 );
					cr.line_to( x2, y2 );
					cr.stroke();

					cr.move_to( x1, y3 );
					cr.line_to( x1, y4 );
					cr.stroke();

					cr.move_to( x2, y3 );
					cr.line_to( x2, y4 );
					cr.stroke();
				}

				for (int iy=0; iy < layout.ny; iy++ )
				{
					double y1 = ymin + iy*layout.dy;
					double y2 = y1 + h;

					double x1 = double.max((xmin - TICK_OFFSET), 0.0);
					double x2 = double.max((x1 - TICK_LENGTH), 0.0);

					double x3 = double.min((xmax + TICK_OFFSET), label.template.page_width);
					double x4 = double.min((x3 + TICK_LENGTH), label.template.page_width);

					cr.move_to( x1, y1 );
					cr.line_to( x2, y1 );
					cr.stroke();

					cr.move_to( x1, y2 );
					cr.line_to( x2, y2 );
					cr.stroke();

					cr.move_to( x3, y1 );
					cr.line_to( x4, y1 );
					cr.stroke();

					cr.move_to( x3, y2 );
					cr.line_to( x4, y2 );
					cr.stroke();
				}

			}

			cr.restore();
		}


		private void print_label( Cairo.Context cr,
		                          double        x,
		                          double        y,
		                          MergeRecord?  record )
		{

			double w, h;
			label.get_size( out w, out h );

			cr.save();

			/* Transform coordinate system to be relative to upper corner */
			/* of the current label */
			cr.translate( x, y );

			cr.save();

			clip_to_outline( cr );

			cr.save();

			/* Special transformations. */
			if ( label.rotate )
			{
				cr.rotate( Math.PI/2 );
				cr.translate( 0, -h );
			}
			if ( reverse_flag )
			{
                cr.translate( w, 0 );
                cr.scale( -1, 1 );
			}

			label.draw( cr, false, record );

			cr.restore(); /* From special transformations. */

			cr.restore(); /* From clip to outline. */

			if ( outline_flag )
			{
				draw_outline( cr );
			}

			cr.restore(); /* From translation. */
		}


		private void draw_outline( Cairo.Context cr )
		{
			cr.save();

			cr.set_source_rgb( 0, 0, 0 );
			cr.set_line_width( OUTLINE_WIDTH );

			TemplateFrame frame = label.template.frames.first().data;
			frame.cairo_path( cr, false );

			cr.stroke();

			cr.restore();
		}


		private void clip_to_outline( Cairo.Context cr )
		{
			TemplateFrame frame = label.template.frames.first().data;
			frame.cairo_path( cr, true );

			cr.set_fill_rule( Cairo.FillRule.EVEN_ODD );
			cr.clip();
		}



	}

}

