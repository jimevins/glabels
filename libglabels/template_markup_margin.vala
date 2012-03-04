/*  template_markup_margin.vala
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

	public class TemplateMarkupMargin : TemplateMarkup
	{
		private const int ARC_FINE = 2;


		public double size  { get; private set; } /* Margin size */


		public TemplateMarkupMargin( double size )
		{
			this.size = size;
		}


		public override TemplateMarkup dup()
		{
			TemplateMarkupMargin copy = new TemplateMarkupMargin( size );

			return (TemplateMarkup)copy;
		}


		public override void cairo_path( Cairo.Context cr,
		                                 TemplateFrame frame )
		{
			if ( frame is TemplateFrameRect )
			{
				rect_cairo_path( cr, (TemplateFrameRect)frame );
			}
			else if ( frame is TemplateFrameEllipse )
			{
				ellipse_cairo_path( cr, (TemplateFrameEllipse)frame );
			}
			else if ( frame is TemplateFrameRound )
			{
				round_cairo_path( cr, (TemplateFrameRound)frame );
			}
			else if ( frame is TemplateFrameCD )
			{
				cd_cairo_path( cr, (TemplateFrameCD)frame );
			}
			else
			{
				assert_not_reached();
			}
		}


		private void rect_cairo_path( Cairo.Context     cr,
		                              TemplateFrameRect frame )
		{
			double w, h;
			frame.get_size( out w, out h );

			w = w - 2*size;
			h = h - 2*size;
			double r = double.max( frame.r - size, 0.0);

			if ( r == 0 )
			{
				cr.rectangle( size, size, w, h );
			}
			else
			{
				cr.new_path();
				cr.arc_negative( size+r,   size+r,   r, 3*Math.PI/2, Math.PI );
				cr.arc_negative( size+r,   size+h-r, r, Math.PI,     Math.PI/2 );
				cr.arc_negative( size+w-r, size+h-r, r, Math.PI/2,   0 );
				cr.arc_negative( size+w-r, size+r,   r, 2*Math.PI,   3*Math.PI/2 );
				cr.close_path();
			}
		}


		private void ellipse_cairo_path( Cairo.Context        cr,
		                                 TemplateFrameEllipse frame )
		{
			double w, h;
			frame.get_size( out w, out h );

			w = w - 2*size;
			h = h - 2*size;

			cr.save();
			cr.translate(size, size);

			cr.new_path();
			cr.move_to( w, h/2 );
			for ( int i_theta = ARC_FINE; i_theta <= 360; i_theta += ARC_FINE )
			{
				double x = (w/2) + (w/2) * Math.cos( i_theta * Math.PI / 180 );
				double y = (h/2) + (h/2) * Math.sin( i_theta * Math.PI / 180 );

				cr.line_to( x, y );
			}
			cr.close_path();

			cr.restore();
		}


		private void round_cairo_path( Cairo.Context      cr,
		                               TemplateFrameRound frame )
		{
			cr.arc( frame.r, frame.r, frame.r-size, 0, 2*Math.PI );
			cr.close_path();
		}


		private void cd_cairo_path( Cairo.Context   cr,
		                            TemplateFrameCD frame )
		{
			double w, h;
			frame.get_size( out w, out h );

			double xc = w/2;
			double yc = h/2;

			double r1 = frame.r1 - size;
			double r2 = frame.r2 + size;

			/*
			 * Outer path (may be clipped)
			 */
			double theta1 = Math.acos( (w-2*size) / (2*r1) );
			double theta2 = Math.asin( (h-2*size) / (2*r1) );

			cr.new_path();
			cr.arc( xc, yc, r1, theta1,           theta2 );
			cr.arc( xc, yc, r1, Math.PI-theta2,   Math.PI-theta1 );
			cr.arc( xc, yc, r1, Math.PI+theta1,   Math.PI+theta2 );
			cr.arc( xc, yc, r1, 2*Math.PI-theta2, 2*Math.PI-theta1 );
			cr.close_path();


			/* Inner path (hole) */
			cr.new_sub_path();
			cr.arc( xc, yc, r2, 0.0, 2*Math.PI );
			cr.close_path();
		}


	}

}
