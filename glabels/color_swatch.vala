/*  color_swatch.vala
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

	public class ColorSwatch : Gtk.DrawingArea
	{
		private Color color;


		public ColorSwatch( int       w,
		                    int       h,
		                    Color     color )
		{
			this.set_has_window( false );

			this.set_size_request( w, h );
			this.color = color;
		}


		public override bool draw( Cairo.Context cr )
		{
			cr.set_antialias( Cairo.Antialias.NONE );

			double w = get_allocated_width();
			double h = get_allocated_height();

			Gtk.StyleContext style_context = this.get_style_context();
			Color  fill_color, border_color;
			if ( this.is_sensitive() )
			{
				fill_color   = color;
				border_color = Color.from_gdk_rgba( style_context.get_border_color(Gtk.StateFlags.NORMAL) );
			}
			else
			{
				fill_color   = Color.none();
				border_color = Color.from_gdk_rgba( style_context.get_border_color(Gtk.StateFlags.INSENSITIVE) );
			}

			cr.rectangle( 1, 1, w-2, h-2 );

			if ( fill_color.has_alpha() )
			{
				cr.set_source_rgba( fill_color.r, fill_color.g, fill_color.b, fill_color.a );
				cr.fill_preserve();
			}

			cr.set_source_rgb( border_color.r, border_color.g, border_color.b );
			cr.set_line_width( 1.0 );
			cr.stroke();

			return false;
		}


		public override void style_updated()
		{
			redraw_canvas();
		}


		public void set_color( Color color )
		{
			if ( !this.color.equal( color ) )
			{
				this.color = color;
				redraw_canvas();
			}
		}


		private void redraw_canvas()
		{
			var window = get_window();
			if (null == window)
			{
				return;
			}

			Cairo.Region region = window.get_clip_region();
			// redraw the cairo canvas completely by exposing it
			window.invalidate_region(region, true);
			window.process_updates(true);
		}


	}

}
