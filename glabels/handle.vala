/*  handle.vala
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

namespace glabels
{

	private const double HANDLE_PIXELS               = 7;
	private const double HANDLE_OUTLINE_WIDTH_PIXELS = 1;

	private const Color HANDLE_FILL_COLOR    = { 0.0,  0.75,  0.0,  0.4 };
	private const Color HANDLE_OUTLINE_COLOR = { 0.0,  0.0,   0.0,  0.8 };


	public abstract class Handle
	{
		public weak LabelObject owner { get; protected set; }

		public abstract void draw( Cairo.Context cr );
		public abstract void cairo_path( Cairo.Context cr );


		protected static void draw_at( Cairo.Context cr,
		                               double        x_handle,
		                               double        y_handle )
		{
			cr.save();

			cr.translate( x_handle, y_handle );

			double scale_x = 1;
			double scale_y = 1;
			cr.device_to_user_distance( ref scale_x, ref scale_y );
			cr.scale( scale_x, scale_y );

			cr.rectangle( -HANDLE_PIXELS/2, -HANDLE_PIXELS/2, HANDLE_PIXELS, HANDLE_PIXELS );

			cr.set_source_rgba( HANDLE_FILL_COLOR.r, HANDLE_FILL_COLOR.g, HANDLE_FILL_COLOR.b, HANDLE_FILL_COLOR.a );
			cr.fill_preserve();
                               
			cr.set_line_width( HANDLE_OUTLINE_WIDTH_PIXELS );
			cr.set_source_rgba( HANDLE_OUTLINE_COLOR.r, HANDLE_OUTLINE_COLOR.g, HANDLE_OUTLINE_COLOR.b,
			                    HANDLE_OUTLINE_COLOR.a );
			cr.stroke();

			cr.restore();
		}


		protected static void cairo_path_at( Cairo.Context cr,
		                                     double        x_handle,
		                                     double        y_handle )
		{
			cr.save();

			cr.translate( x_handle, y_handle );

			double scale_x = 1;
			double scale_y = 1;
			cr.device_to_user_distance( ref scale_x, ref scale_y );
			cr.scale( scale_x, scale_y );

			cr.rectangle( -HANDLE_PIXELS/2, -HANDLE_PIXELS/2, HANDLE_PIXELS, HANDLE_PIXELS );

			cr.restore();
		}

	}


	public class HandleNorth : Handle
	{
		public HandleNorth( LabelObject owner )
		{
			this.owner = owner;
		}

		public override void draw( Cairo.Context cr )
		{
			draw_at( cr, owner.w/2, 0 );
		}

		public override void cairo_path( Cairo.Context cr )
		{
			cairo_path_at( cr, owner.w/2, 0 );
		}
	}


	public class HandleNorthEast : Handle
	{
		public HandleNorthEast( LabelObject owner )
		{
			this.owner = owner;
		}

		public override void draw( Cairo.Context cr )
		{
			draw_at( cr, owner.w, 0 );
		}

		public override void cairo_path( Cairo.Context cr )
		{
			cairo_path_at( cr, owner.w, 0 );
		}
	}


	public class HandleEast : Handle
	{
		public HandleEast( LabelObject owner )
		{
			this.owner = owner;
		}

		public override void draw( Cairo.Context cr )
		{
			draw_at( cr, owner.w, owner.h/2 );
		}

		public override void cairo_path( Cairo.Context cr )
		{
			cairo_path_at( cr, owner.w, owner.h/2 );
		}
	}


	public class HandleSouthEast : Handle
	{
		public HandleSouthEast( LabelObject owner )
		{
			this.owner = owner;
		}

		public override void draw( Cairo.Context cr )
		{
			draw_at( cr, owner.w, owner.h );
		}

		public override void cairo_path( Cairo.Context cr )
		{
			cairo_path_at( cr, owner.w, owner.h );
		}
	}


	public class HandleSouth : Handle
	{
		public HandleSouth( LabelObject owner )
		{
			this.owner = owner;
		}

		public override void draw( Cairo.Context cr )
		{
			draw_at( cr, owner.w/2, owner.h );
		}

		public override void cairo_path( Cairo.Context cr )
		{
			cairo_path_at( cr, owner.w/2, owner.h );
		}
	}


	public class HandleSouthWest : Handle
	{
		public HandleSouthWest( LabelObject owner )
		{
			this.owner = owner;
		}

		public override void draw( Cairo.Context cr )
		{
			draw_at( cr, 0, owner.h );
		}

		public override void cairo_path( Cairo.Context cr )
		{
			cairo_path_at( cr, 0, owner.h );
		}
	}


	public class HandleWest : Handle
	{
		public HandleWest( LabelObject owner )
		{
			this.owner = owner;
		}

		public override void draw( Cairo.Context cr )
		{
			draw_at( cr, 0, owner.h/2 );
		}

		public override void cairo_path( Cairo.Context cr )
		{
			cairo_path_at( cr, 0, owner.h/2 );
		}
	}


	public class HandleNorthWest : Handle
	{
		public HandleNorthWest( LabelObject owner )
		{
			this.owner = owner;
		}

		public override void draw( Cairo.Context cr )
		{
			draw_at( cr, 0, 0 );
		}

		public override void cairo_path( Cairo.Context cr )
		{
			cairo_path_at( cr, 0, 0 );
		}
	}


}

