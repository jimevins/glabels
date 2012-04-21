/*  mini_preview.vala
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

	public class MiniPreview : Gtk.EventBox
	{

		public signal void clicked( int index );

		public signal void pressed( int index1,
		                            int index2 );

		public signal void released( int index1,
		                             int index2 );


		private const double MARGIN = 2;
		private const double SHADOW_OFFSET = 3;


		private Gtk.DrawingArea canvas;

		private Template?                     template;
		private Gee.ArrayList<TemplateCoord?> origins;
		private Gee.ArrayList<TemplateCoord?> centers;

		private int highlight_first;
		private int highlight_last;

		private bool dragging;
		private int  first_i;
		private int  last_i;
		private int  prev_i;

		private bool update_scheduled_flag;

		private Model? model;



		public MiniPreview( int width,
		                    int height )
		{
			add_events( Gdk.EventMask.BUTTON_PRESS_MASK   |
			            Gdk.EventMask.BUTTON_RELEASE_MASK |
			            Gdk.EventMask.POINTER_MOTION_MASK );

			set_visible_window( false );

			canvas = new Gtk.DrawingArea();
			canvas.set_has_window( false );
			add( canvas );

			canvas.set_size_request( width, height );

			canvas.draw.connect( on_draw );
			canvas.style_set.connect( on_style_set );
		}


		public void set_model( Model model )
		{
			this.model = model;
			set_template( model.label.template );

			model.label.changed.connect( on_label_changed );
		}


		public void set_template_by_name( string name )
		{
			Template? template = Db.lookup_template_from_name( name );
			set_template( template );
		}


		public void set_template( Template? template )
		{
			this.template = template;

			if ( template != null )
			{
				TemplateFrame frame = template.frames.first().data;

				/*
				 * Cache list of origins.
				 */
				origins = frame.get_origins();

				/*
				 * Also build list of label centers from origins and cache.
				 */
				double w, h;
				frame.get_size( out w, out h );

				centers = frame.get_origins();

				foreach ( TemplateCoord center in centers )
				{
					center.x += w/2;
					center.y += h/2;
				}


				redraw_canvas();
			}
		}


		public void highlight_range( int first_label,
		                             int last_label )
		{
			if ( ( first_label != highlight_first ) ||
			     ( last_label  != highlight_last ) )
			{
				highlight_first = first_label;
				highlight_last  = last_label;

				redraw_canvas();
			}
		}


		public override bool button_press_event( Gdk.EventButton event )
		{
			if ( event.button == 1 )
			{
				Cairo.Context cr = Gdk.cairo_create( canvas.get_window() );

				set_transform_and_get_scale( cr );

				double x = event.x;
				double y = event.y;
				cr.device_to_user( ref x, ref y );

				int i = find_closest_label( x, y );
				clicked( i );

				first_i = i;
				last_i  = i;
				pressed( first_i, last_i );

				dragging = true;
				prev_i   = i;
			}

			return false;
		}


		public override bool button_release_event( Gdk.EventButton event )
		{
			if ( event.button == 1 )
			{
				dragging = false;
			}
			released( first_i, last_i );

			return false;
		}

		
		public override bool motion_notify_event( Gdk.EventMotion event )
		{
			if ( dragging )
			{
				Cairo.Context cr = Gdk.cairo_create( canvas.get_window() );

				set_transform_and_get_scale( cr );

				double x = event.x;
				double y = event.y;
				cr.device_to_user( ref x, ref y );

				int i = find_closest_label( x, y );

				if ( i != prev_i )
				{
					last_i = prev_i = i;
					pressed( int.min( first_i, last_i ), int.max( first_i, last_i ) );
				}
			}

			return false;
		}


		private void on_style_set( Gtk.Style? previous_style )
		{
			redraw_canvas();
		}


		private void on_label_changed()
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

			if ( !update_scheduled_flag )
			{
				update_scheduled_flag = true;

				unowned Cairo.Region region = window.get_clip_region ();
				// redraw the cairo canvas completely by exposing it
				window.invalidate_region (region, true);
			}

		}


		private double set_transform_and_get_scale(Cairo.Context cr )
		{

			/* Establish scale and origin. */
			double w = get_allocated_width();
			double h = get_allocated_height();

			/* establish scale. */
			double scale = double.min( (w - 2*MARGIN - 2*SHADOW_OFFSET)/template.page_width,
			                           (h - 2*MARGIN - 2*SHADOW_OFFSET)/template.page_height );

			/* Find offset to center preview. */
			double offset_x = (w/scale - template.page_width) / 2.0;
			double offset_y = MARGIN;

			/* Set transformation. */
			cr.scale( scale, scale );
			cr.translate( offset_x, offset_y );

			return scale;
		}


		private int find_closest_label( double x,
		                                double y )
		{
			double dx = x - centers[0].x;
			double dy = y - centers[0].y;
			double min_d2 = dx*dx + dy*dy;

			int i = 0;
			int min_i = 0;

			foreach ( TemplateCoord center in centers )
			{
				dx = x - center.x;
				dy = y - center.y;
				double d2 = dx*dx + dy*dy;

				if ( d2 < min_d2 )
				{
					min_d2 = d2;
					min_i  = i;
				}
				i++;
			}

			return min_i + 1;
		}


		private bool on_draw( Cairo.Context cr )
		{
			update_scheduled_flag = false;

			if ( template != null )
			{
				double scale = set_transform_and_get_scale( cr );

				/* update shadow */
				double shadow_x = SHADOW_OFFSET/scale;
				double shadow_y = SHADOW_OFFSET/scale;

				draw_shadow( cr, shadow_x, shadow_y );

				draw_paper( cr,	1.0/scale );

				draw_labels( cr, 2.0/scale );

				if ( model != null )
				{
					draw_rich_preview( cr );
				}

			}

			return false;
		}


		private void draw_shadow( Cairo.Context cr,
		                          double        x,
		                          double        y )
		{
			cr.save();

			cr.rectangle( x, y, template.page_width, template.page_height );

			Gtk.Style style = get_style();
			Color shadow_color = Color.from_gdk_color( style.dark[Gtk.StateType.NORMAL] );
			cr.set_source_rgb( shadow_color.r, shadow_color.g, shadow_color.b );

			cr.fill();

			cr.restore();
		}


		private void draw_paper( Cairo.Context cr,
		                         double        line_width )
		{
			cr.save();

			cr.rectangle( 0, 0, template.page_width, template.page_height );

			Gtk.Style style = get_style();
			Color paper_color = Color.from_gdk_color( style.light[Gtk.StateType.NORMAL] );
			Color outline_color = Color.from_gdk_color( style.fg[Gtk.StateType.NORMAL] );

			cr.set_source_rgb( paper_color.r, paper_color.g, paper_color.b );
			cr.fill_preserve();

			cr.set_source_rgb( outline_color.r, outline_color.g, outline_color.b );
			cr.set_line_width( line_width );
			cr.stroke();

			cr.restore();
		}


		private void draw_labels( Cairo.Context cr,
		                          double        line_width )
		{
			TemplateFrame frame = template.frames.first().data;

			int n_labels = frame.get_n_labels();

			Gtk.Style style = get_style();
			Color base_color = Color.from_gdk_color( style.base[Gtk.StateType.SELECTED] );
			Color highlight_color = Color.from_color_and_opacity( base_color, 0.10 );

			Color outline_color;
			if ( model != null )
			{
				/* Outlines are more subtle when doing a rich preview. */
				outline_color = Color.from_color_and_opacity( base_color, 0.25 );
			}
			else
			{
				outline_color = Color.from_color_and_opacity( base_color, 1.00 );
			}

			for ( int i = 0; i < n_labels; i++ )
			{
				cr.save();

				cr.translate( origins[i].x, origins[i].y );
				frame.cairo_path( cr, false );

				if ( ((i+1) >= highlight_first) &&
				     ((i+1) <= highlight_last) )
				{
					cr.set_source_rgba( highlight_color.r, highlight_color.g, highlight_color.b, highlight_color.a );
					cr.set_fill_rule( Cairo.FillRule.EVEN_ODD );
					cr.fill_preserve();
				}

				cr.set_line_width( line_width );
				cr.set_source_rgba( outline_color.r, outline_color.g, outline_color.b, outline_color.a );
				cr.stroke();

				cr.restore();
			}
		}


		private void draw_rich_preview( Cairo.Context cr )
		{
			/* TODO: test for merge. */
			model.print.print_simple_sheet( cr, 0 );
		}

		
	}

}