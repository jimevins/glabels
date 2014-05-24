/*  font_sample.vala
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

	public class FontSample : Gtk.DrawingArea
	{
		private string sample_text;
		private string font_family;

		public FontSample( int       w,
		                   int       h,
		                   string    sample_text,
		                   string    font_family )
		{
			this.set_has_window( false );

			this.sample_text = sample_text;
			this.font_family = font_family;

			this.set_size_request( w, h );
		}

		public override bool draw( Cairo.Context cr )
		{
			double w = get_allocated_width();
			double h = get_allocated_height();

			Gtk.StyleContext style_context = this.get_style_context();
			Color  text_color, fill_color, border_color;
			if ( this.is_sensitive() )
			{
				text_color   = Color.black();
				fill_color   = Color.white();
				border_color = Color.from_gdk_rgba( style_context.get_border_color(Gtk.StateFlags.NORMAL) );
			}
			else
			{
				text_color   = Color( 0.75, 0.75, 0.75, 1.0 );
				fill_color   = Color.none();
				border_color = Color.from_gdk_rgba( style_context.get_border_color(Gtk.StateFlags.INSENSITIVE) );
			}

			cr.set_antialias( Cairo.Antialias.NONE );

			cr.rectangle( 1, 1, w-2, h-2 );

			cr.set_source_rgba( fill_color.r, fill_color.g, fill_color.b, fill_color.a );
			cr.fill_preserve();

			cr.set_source_rgb( border_color.r, border_color.g, border_color.b );
			cr.set_line_width( 1.0 );
			cr.stroke();

			cr.set_antialias( Cairo.Antialias.DEFAULT );

			Pango.Layout layout = Pango.cairo_create_layout( cr );

			Pango.FontDescription desc = new Pango.FontDescription();
			desc.set_family( font_family );
			desc.set_weight( Pango.Weight.NORMAL );
			desc.set_style( Pango.Style.NORMAL );
			desc.set_size( (int)(0.6 * (h-1) * Pango.SCALE) );

			layout.set_font_description( desc );
			layout.set_text( sample_text, -1 );
			layout.set_width( -1 );

			Pango.Rectangle ink_rect, logical_rect;
			layout.get_pixel_extents( out ink_rect, out logical_rect );
			double layout_width  = double.max( logical_rect.width, ink_rect.width );
			double layout_height = double.max( logical_rect.height, ink_rect.height );

			double layout_x = (w - layout_width) / 2.0;
			double layout_y = (h - layout_height) / 2.0;

			if (ink_rect.x < logical_rect.x)
			{
				layout_x += logical_rect.x - ink_rect.x;
			}

			if (ink_rect.y < logical_rect.y)
			{
				layout_y += logical_rect.y - ink_rect.y;
			}

			cr.set_source_rgb( text_color.r, text_color.g, text_color.b );
			cr.move_to( layout_x, layout_y );
			Pango.cairo_show_layout( cr, layout );

			return false;
		}

		public override void style_updated()
		{
			redraw_canvas();
		}

		private void redraw_canvas()
		{
			var window = get_window ();
			if (null == window)
			{
				return;
			}

			Cairo.Region region = window.get_clip_region ();
			// redraw the cairo canvas completely by exposing it
			window.invalidate_region (region, true);
			window.process_updates (true);
		}


	}

}
