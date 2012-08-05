/*  view.vala
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

	public class View : Gtk.ScrolledWindow
	{
		private const Color BG_COLOR          = { 0.8,   0.8,   0.8,   1.0 };
		private const Color SHADOW_COLOR      = { 0.2,   0.2,   0.2,   1.0 };
		private const Color PAPER_COLOR       = { 1.0,   1.0,   1.0,   1.0 };
		private const Color GRID_COLOR        = { 0.753, 0.753, 0.753, 1.0 };
		private const Color MARKUP_COLOR      = { 0.94,  0.39,  0.39,  1.0 };
		private const Color OUTLINE_COLOR     = { 0,     0,     0,     1.0 };
		private const Color SELECT_LINE_COLOR = { 0,     0,     1,     0.5 };
		private const Color SELECT_FILL_COLOR = { 0.75,  0.75,  1,     0.5 };

		private const double POINTS_PER_MM = 2.8346456692;

		private const double GRID_LINE_WIDTH_PIXELS   = 1;
		private const double MARKUP_LINE_WIDTH_PIXELS = 1;
		private const double OUTLINE_WIDTH_PIXELS     = 1;
		private const double SELECT_LINE_WIDTH_PIXELS = 3;

		private const int CURSOR_X_HOTSPOT = 7;
		private const int CURSOR_Y_HOTSPOT = 7;

		private const int ZOOMTOFIT_PAD = 16;
		private const int SHADOW_OFFSET_PIXELS = ZOOMTOFIT_PAD/4;

		private const double zooms[] = { 8, 6, 4, 3, 2, 1.5, 1, 0.75, 0.67, 0.50, 0.33, 0.25, 0.15, 0.10 };


		private enum State { IDLE, ARROW_SELECT_REGION, ARROW_MOVE, ARROW_RESIZE, CREATE_DRAG }

		private enum CreateType { BOX, ELLIPSE, LINE, IMAGE, TEXT, BARCODE }


		public signal void context_menu_activate( uint event_button, uint event_time );
		public signal void zoom_changed( double zoom );
		public signal void pointer_moved( double x, double y );
		public signal void pointer_exit();
		public signal void mode_changed();


		private Prefs        prefs;

		private Gtk.Layout   canvas;

		private double       grid_spacing;
		private bool         zoom_to_fit_flag;
		private double       home_scale;

		private bool         update_scheduled_flag;
		private double       x0;
		private double       y0;
		private double       scale;

		private bool         in_object_create_mode;

		private State        state;

		/* ARROW_SELECT_REGION state */
		private bool         select_region_visible;
		private LabelRegion  select_region;

		/* ARROW_MOVE state */
		private double       move_last_x;
		private double       move_last_y;

		/* ARROW_RESIZE state */
		private LabelObject? resize_object;
		private Handle?      resize_handle;
		private bool         resize_honor_aspect;

		/* CREATE_DRAG state */
		private CreateType   create_object_type;
		private LabelObject? create_object;
		private double       create_x0;
		private double       create_y0;


		public double zoom         { get; private set; }

		private bool _markup_visible;
		public bool markup_visible
		{
			get
			{
				return _markup_visible;
			}

			set
			{
				_markup_visible = value;
				update();
			}
		}

		private bool _grid_visible;
		public bool grid_visible
		{
			get
			{
				return _grid_visible;
			}

			set
			{
				_grid_visible = value;
				update();
			}
		}


		private Model? _model;
		public Model? model
		{
			get
			{
				return _model;
			}

			set
			{
				_model = value;
				if ( _model != null )
				{
					model.label.changed.connect( on_label_changed );
					model.label.selection_changed.connect( on_label_changed );
					model.label.size_changed.connect( on_label_size_changed );

					canvas.focus_in_event.connect( on_focus_in_event );
					canvas.focus_out_event.connect( on_focus_out_event );
					canvas.enter_notify_event.connect( on_enter_notify_event );
					canvas.leave_notify_event.connect( on_leave_notify_event );
					canvas.motion_notify_event.connect( on_motion_notify_event );
					canvas.button_press_event.connect( on_button_press_event );
					canvas.button_release_event.connect( on_button_release_event );
					canvas.key_press_event.connect( on_key_press_event );
				}
			}
		}


		public View()
		{
			prefs = new Prefs();

			canvas = new Gtk.Layout( null, null );
			this.set_policy( Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC );
			this.add( canvas );

			canvas.modify_bg( Gtk.StateType.NORMAL, BG_COLOR.to_gdk_color() );
			canvas.set_can_focus( true );

			canvas.add_events( Gdk.EventMask.FOCUS_CHANGE_MASK   |
			                   Gdk.EventMask.ENTER_NOTIFY_MASK   |
			                   Gdk.EventMask.LEAVE_NOTIFY_MASK   |
			                   Gdk.EventMask.POINTER_MOTION_MASK |
			                   Gdk.EventMask.BUTTON_PRESS_MASK   |
			                   Gdk.EventMask.BUTTON_RELEASE_MASK |
			                   Gdk.EventMask.KEY_PRESS_MASK );

			grid_visible          = true;
			grid_spacing          = UnitsUtil.get_grid_size( prefs.units );
			markup_visible        = true;
			in_object_create_mode = false;
			zoom                  = 1.0;
			zoom_to_fit_flag      = false;
			home_scale            = determine_home_scale();

			prefs.changed.connect( on_prefs_changed );
			canvas.draw.connect( on_draw );
			canvas.realize.connect( on_realize );
			canvas.size_allocate.connect( on_size_allocate );
			canvas.screen_changed.connect( on_screen_changed );
		}


		private double determine_home_scale()
		{
			if ( !canvas.has_screen() )
			{
				return 1.0;
			}

			Gdk.Screen screen = canvas.get_screen();

			double screen_width_pixels  = screen.get_width();
			double screen_width_mm      = screen.get_width_mm();
			double screen_height_pixels = screen.get_height();
			double screen_height_mm     = screen.get_width_mm();

			double x_pixels_per_mm = screen_width_pixels / screen_width_mm;
			double y_pixels_per_mm = screen_height_pixels / screen_height_mm;

			double scale = ( x_pixels_per_mm + y_pixels_per_mm ) / (2 * POINTS_PER_MM);

			/* Make sure scale is somewhat sane. */
			if ( (scale < 0.25) || (scale > 4.0) )
			{
				scale = 1.0;
			}

			return scale;
		}


		/**
		 * Zoom in one "notch"
		 */
		public void zoom_in()
		{
			/* find closest standard zoom. */

			/* start with 2nd largest scale */
			int i_min = 1;
			double dist_min = Math.fabs( zooms[1] - zoom );

			for ( int i = 2; i < zooms.length; i++ )
			{
				double dist = Math.fabs( zooms[i] - zoom );
				if ( dist < dist_min )
				{
					i_min = i;
					dist_min = dist;
				}
			}

			/* zoom in one notch */
			set_zoom_real( zooms[i_min-1], false );
		}


		/**
		 * Zoom out one "notch"
		 */
		public void zoom_out()
		{
			/* find closest standard zoom. */

			/* start with largest scale, end on 2nd smallest */
			int i_min = 0;
			double dist_min = Math.fabs( zooms[0] - zoom );

			for ( int i = 1; i < (zooms.length-1); i++ )
			{
				double dist = Math.fabs( zooms[i] - zoom );
				if ( dist < dist_min )
				{
					i_min = i;
					dist_min = dist;
				}
			}

			/* zoom in one notch */
			set_zoom_real( zooms[i_min+1], false );
		}


		/**
		 * Set zoom to 1 to 1
		 */
		public void zoom_1to1()
		{
			set_zoom_real( 1.0, false );
		}


		/**
		 * Zoom to fit
		 */
		public void zoom_to_fit()
		{
			if ( !has_screen() )
			{
				/* Delay until realized. */
				zoom_to_fit_flag = true;
				return;
			}

			int w_view = get_allocated_width();
			int h_view = get_allocated_height();

			double w_label, h_label;
			model.label.get_size( out w_label, out h_label );

			/* Calculate best scale */
			double x_scale = (double)( w_view - ZOOMTOFIT_PAD ) / w_label;
			double y_scale = (double)( h_view - ZOOMTOFIT_PAD ) / h_label;
			double scale = double.min( x_scale, y_scale );

			/* Limit */
			scale = double.min( scale, zooms[0]*home_scale );
			scale = double.max( scale, zooms[zooms.length - 1]*home_scale );

			set_zoom_real( scale/home_scale, true );
		}


		private void set_zoom_real( double zoom,
		                            bool   zoom_to_fit_flag )
		{
			/* Limit, if needed. */
			zoom = double.min( zoom, zooms[0] );
			zoom = double.max( zoom, zooms[ zooms.length-1 ] );

			if ( this.zoom != zoom )
			{
				this.zoom             = zoom;
				this.zoom_to_fit_flag = zoom_to_fit_flag;

				update();

				zoom_changed( zoom );
			}
		}


		public bool is_zoom_max()
		{
			return ( zoom >= zooms[0] );
		}


		public bool is_zoom_min()
		{
			return ( zoom <= zooms[zooms.length-1] );
		}


		public void arrow_mode()
		{
			Gdk.Window window = canvas.get_window();

			Gdk.Cursor cursor = new Gdk.Cursor( Gdk.CursorType.LEFT_PTR );
			window.set_cursor( cursor );

			in_object_create_mode = false;
			state = State.IDLE;
		}


		public void create_box_mode()
		{
			Gdk.Window window = canvas.get_window();

			try
			{
				Gdk.Pixbuf pixbuf = Gdk.Pixbuf.from_pixdata( Cursor.box_pixdata, false );
				Gdk.Cursor cursor = new Gdk.Cursor.from_pixbuf( Gdk.Display.get_default(),
				                                                pixbuf, CURSOR_X_HOTSPOT, CURSOR_Y_HOTSPOT );
				window.set_cursor( cursor );
			}
			catch ( Error err )
			{
				error( "%s\n", err.message );
			}

			in_object_create_mode = true;
			create_object_type    = CreateType.BOX;
			state                 = State.IDLE;
		}


		public void create_text_mode()
		{
			Gdk.Window window = canvas.get_window();

			try
			{
				Gdk.Pixbuf pixbuf = Gdk.Pixbuf.from_pixdata( Cursor.text_pixdata, false );
				Gdk.Cursor cursor = new Gdk.Cursor.from_pixbuf( Gdk.Display.get_default(),
				                                                pixbuf, CURSOR_X_HOTSPOT, CURSOR_Y_HOTSPOT );
				window.set_cursor( cursor );
			}
			catch ( Error err )
			{
				error( "%s\n", err.message );
			}

			in_object_create_mode = true;
			create_object_type    = CreateType.TEXT;
			state                 = State.IDLE;
		}


		private void on_prefs_changed()
		{
			grid_spacing = UnitsUtil.get_grid_size( prefs.units );
			update();
		}


		private void update()
		{
			var window = canvas.get_window();
			if (null == window)
			{
				return;
			}

			if ( !update_scheduled_flag )
			{
				update_scheduled_flag = true;

				Gdk.Rectangle rect = Gdk.Rectangle();

				rect.x      = 0;
				rect.y      = 0;
				rect.width  = canvas.get_allocated_width();
				rect.height = canvas.get_allocated_height();

				window.invalidate_rect( rect, true );
			}
		}


		private bool on_draw( Cairo.Context cr )
		{
			update_scheduled_flag = false;

			Gdk.Window bin_window = canvas.get_bin_window();
			Cairo.Context bin_cr = Gdk.cairo_create( bin_window );

			draw_layers( bin_window, bin_cr );

			return false;
		}


		private void draw_layers( Gdk.Window    window,
		                          Cairo.Context cr )
		{
			if ( model != null )
			{
				this.scale = zoom * home_scale;

				double w, h;
				model.label.get_size( out w, out h );

				canvas.set_size( (int)(w*scale + 8), (int)(h*scale + 8) );

				int canvas_w = window.get_width();
				int canvas_h = window.get_height();

				this.x0 = (canvas_w/scale - w) / 2;
				this.y0 = (canvas_h/scale - h) / 2;

				cr.save();

				cr.scale( scale, scale );
				cr.translate( x0, y0 );

				draw_bg_layer( cr );
				draw_grid_layer( cr );
				draw_markup_layer( cr );
				draw_objects_layer( cr );
				draw_fg_layer( cr );
				draw_highlight_layer( cr );
				draw_select_region_layer( cr );

				cr.restore();
			}
		}


		private void set_frame_path( Cairo.Context cr,
		                             TemplateFrame frame )
		{
			cr.save();

			if ( model.label.rotate )
			{
				double w, h;
				frame.get_size( out w, out h );

				/* Canvas coordinates are relative to content, so we must rotate the frame. */
				cr.rotate( -Math.PI / 2 );
				cr.translate( -w, 0 );
			}

			frame.cairo_path( cr, false );

			cr.restore();
		}

		private void draw_bg_layer( Cairo.Context cr )
		{
			TemplateFrame frame = model.label.template.frames.first().data;

			double w, h;
			frame.get_size( out w, out h );

			cr.save();
			cr.translate( SHADOW_OFFSET_PIXELS/scale, SHADOW_OFFSET_PIXELS/scale );

			set_frame_path( cr, frame );
			cr.set_source_rgb( SHADOW_COLOR.r, SHADOW_COLOR.g, SHADOW_COLOR.b );
			cr.set_fill_rule( Cairo.FillRule.EVEN_ODD );
			cr.fill();
			cr.restore();

			cr.save();
			set_frame_path( cr, frame );
			cr.set_source_rgb( PAPER_COLOR.r, PAPER_COLOR.g, PAPER_COLOR.b );
			cr.set_fill_rule( Cairo.FillRule.EVEN_ODD );
			cr.fill();
			cr.restore();
		}


		private void draw_grid_layer( Cairo.Context cr )
		{
			if ( grid_visible )
			{
				TemplateFrame frame = model.label.template.frames.first().data;

				double w, h;
				model.label.get_size( out w, out h );
        
				double x0, y0;
				if ( frame is TemplateFrameRect )
				{
					x0 = grid_spacing;
					y0 = grid_spacing;
				}
				else
				{
					/* round labels, adjust grid to line up with center of label. */
					x0 = Math.fmod( w/2, grid_spacing );
					y0 = Math.fmod( h/2, grid_spacing );
				}

				cr.save();

				set_frame_path( cr, frame );
				cr.clip();

				cr.set_antialias( Cairo.Antialias.NONE );
				cr.set_line_width( GRID_LINE_WIDTH_PIXELS/scale );
				cr.set_source_rgb( GRID_COLOR.r, GRID_COLOR.g, GRID_COLOR.b );

				for ( double x = x0; x < w; x += grid_spacing )
				{
					cr.move_to( x, 0 );
					cr.line_to( x, h );
					cr.stroke();
				}

				for ( double y = y0; y < h; y += grid_spacing )
				{
					cr.move_to( 0, y );
					cr.line_to( w, y );
					cr.stroke();
				}

				cr.restore();
			}
		}


		private void draw_markup_layer( Cairo.Context cr )
		{
			if ( markup_visible )
			{
				TemplateFrame frame = model.label.template.frames.first().data;

				cr.save();

				if ( model.label.rotate )
				{
					double w, h;
					frame.get_size( out w, out h );

					/* Markup is relative to the normal orientation of label. */
					cr.rotate( -Math.PI / 2 );
					cr.translate( -w, 0 );
				}

				cr.set_line_width( MARKUP_LINE_WIDTH_PIXELS / scale );
				cr.set_source_rgb( MARKUP_COLOR.r, MARKUP_COLOR.g, MARKUP_COLOR.b );

				foreach ( TemplateMarkup markup in frame.markups )
				{
					markup.cairo_path( cr, frame );
					cr.stroke();
				}

				cr.restore();
			}
		}


		private void draw_objects_layer( Cairo.Context cr )
		{
			model.label.draw( cr, true, null );
		}


		private void draw_fg_layer( Cairo.Context cr )
		{
			TemplateFrame frame = model.label.template.frames.first().data;

			set_frame_path( cr, frame );

			cr.set_line_width( OUTLINE_WIDTH_PIXELS/scale );
			cr.set_source_rgb( OUTLINE_COLOR.r, OUTLINE_COLOR.g, OUTLINE_COLOR.b );
			cr.stroke();
		}


		private void draw_highlight_layer( Cairo.Context cr )
		{
			cr.save();
			cr.set_antialias( Cairo.Antialias.NONE );

			foreach ( LabelObject object in model.label.object_list )
			{
				if ( object.is_selected() )
				{
					object.draw_selection_highlight( cr );
				}
			}

			cr.restore();
		}


		private void draw_select_region_layer( Cairo.Context cr )
		{
			if ( select_region_visible )
			{
				double x1 = double.min( select_region.x1, select_region.x2 );
				double y1 = double.min( select_region.y1, select_region.y2 );
				double w  = Math.fabs( select_region.x2 - select_region.x1 );
				double h  = Math.fabs( select_region.y2 - select_region.y1 );

				cr.save();

				cr.set_antialias( Cairo.Antialias.NONE );

				cr.rectangle( x1, y1, w, h );

				cr.set_source_rgba( SELECT_FILL_COLOR.r, SELECT_FILL_COLOR.g, SELECT_FILL_COLOR.b,
				                    SELECT_FILL_COLOR.a );
				cr.fill_preserve();

				cr.set_line_width( SELECT_LINE_WIDTH_PIXELS/scale );
				cr.set_source_rgba( SELECT_LINE_COLOR.r, SELECT_LINE_COLOR.g, SELECT_LINE_COLOR.b,
				                    SELECT_LINE_COLOR.a );
				cr.stroke();

				cr.restore();
			}
		}


		private void on_realize()
		{
			if ( zoom_to_fit_flag )
			{
				/* Maintain best fit zoom */
				zoom_to_fit();
			}
		}


		private void on_size_allocate( Gtk.Allocation allocation )
		{
			Gtk.Adjustment hadjustment = canvas.get_hadjustment();
			Gtk.Adjustment vadjustment = canvas.get_vadjustment();

			hadjustment.set_page_size( allocation.width );
			hadjustment.set_page_increment( allocation.width / 2 );

			vadjustment.set_page_size( allocation.height );
			vadjustment.set_page_increment( allocation.height / 2 );

			hadjustment.changed();
			vadjustment.changed();

			if ( zoom_to_fit_flag )
			{
				/* Maintain best fit zoom */
				zoom_to_fit();
			}
		}


		private void on_screen_changed()
		{
			if ( canvas.has_screen() )
			{

				home_scale = determine_home_scale();

				if ( zoom_to_fit_flag )
				{
					/* Maintain best fit zoom */
					zoom_to_fit();
				}
			}
		}


		private void on_label_changed()
		{
			update();
		}


		private void on_label_size_changed()
		{
			Gtk.Adjustment hadjustment = canvas.get_hadjustment();
			Gtk.Adjustment vadjustment = canvas.get_vadjustment();

			hadjustment.changed();
			vadjustment.changed();

			update();
		}


		private bool on_focus_in_event( Gdk.EventFocus event )
		{
			return false;
		}


		private bool on_focus_out_event( Gdk.EventFocus event )
		{
			return false;
		}


		private bool on_enter_notify_event( Gdk.EventCrossing event )
		{
			return false;
		}


		private bool on_leave_notify_event( Gdk.EventCrossing event )
		{
			pointer_exit();
			return false;
		}


		private bool on_motion_notify_event( Gdk.EventMotion event )
		{
			bool return_value = false;

			Gdk.Window bin_window = canvas.get_bin_window();
			Gdk.Window window = canvas.get_window();

			Cairo.Context cr = Gdk.cairo_create( bin_window );

			/*
			 * Translate to label coordinates
			 */
			cr.scale( scale, scale );
			cr.translate( x0, y0 );

			double x = event.x;
			double y = event.y;
			cr.device_to_user( ref x, ref y );

			/*
			 * Emit signal regardless of mode
			 */
			pointer_moved( x, y );

			/*
			 * Handle event as appropriate for mode
			 */
			if ( !in_object_create_mode )
			{
				switch (state)
				{

				case State.IDLE:
					Gdk.Cursor cursor;
					Handle? handle;
					if ( model.label.is_selection_atomic() &&
					     (handle = model.label.handle_at( cr, event.x, event.y )) != null )
					{
					     cursor = new Gdk.Cursor( Gdk.CursorType.CROSSHAIR );
					}
					else if ( model.label.object_at( cr, event.x, event.y) != null )
					{
						cursor = new Gdk.Cursor( Gdk.CursorType.FLEUR );
					}
					else
					{
						cursor = new Gdk.Cursor( Gdk.CursorType.LEFT_PTR );
					}
					window.set_cursor( cursor );
					break;

				case State.ARROW_SELECT_REGION:
					select_region.x2 = x;
					select_region.y2 = y;
					update();
					break;

				case State.ARROW_MOVE:
					model.label.move_selection( (x - move_last_x), (y - move_last_y) );
					move_last_x = x;
					move_last_y = y;
					break;

				case State.ARROW_RESIZE:
					handle_resize_motion( cr, event.x, event.y );
					break;

				default:
					warning( "Invalid arrow state." );      /*Should not happen!*/
					break;

				}
				return_value = true;

			}
			else
			{

				if ( state != State.IDLE )
				{
					switch ( create_object_type )
					{
					case CreateType.BOX:
						create_object.set_position( double.min( x, create_x0 ), double.min( y, create_y0 ) );
						create_object.set_size( double.max( x, create_x0 ) - double.min( x, create_x0 ),
						                        double.max( y, create_y0 ) - double.min( y, create_y0 ) );
						break;
					case CreateType.ELLIPSE:
						/* TODO */
						break;
					case CreateType.LINE: 
						/* TODO */
						break;
					case CreateType.IMAGE:
						/* TODO */
						break;
					case CreateType.TEXT:
						create_object.set_position( double.min( x, create_x0 ), double.min( y, create_y0 ) );
						create_object.set_size( double.max( x, create_x0 ) - double.min( x, create_x0 ),
						                        double.max( y, create_y0 ) - double.min( y, create_y0 ) );
						break;
					case CreateType.BARCODE:
						/* TODO */
						break;
					default:
						warning( "Invalid create type." );   /* Should not happen! */
						break;
					}
				}

			}

			return return_value;
		}


		private bool on_button_press_event( Gdk.EventButton event )
		{
			bool return_value = false;

			canvas.grab_focus();

			Gdk.Window bin_window = canvas.get_bin_window();

			Cairo.Context cr = Gdk.cairo_create( bin_window );

			/*
			 * Translate to label coordinates
			 */
			cr.scale( scale, scale );
			cr.translate( x0, y0 );

			double x = event.x;
			double y = event.y;
			cr.device_to_user( ref x, ref y );

			switch (event.button)
			{

			case 1:
				/*
				 * Handle event as appropriate for mode
				 */
				if ( !in_object_create_mode )
				{
					LabelObject object;
					Handle? handle = null;
					if ( model.label.is_selection_atomic() &&
					     (handle = model.label.handle_at( cr, event.x, event.y )) != null )
					{
						resize_object = handle.owner;
						resize_handle = handle;
						resize_honor_aspect = (event.state & Gdk.ModifierType.CONTROL_MASK) != 0;

						state = State.ARROW_RESIZE;
					}
					else if ( (object = model.label.object_at( cr, event.x, event.y)) != null )
					{
						if ( (event.state & Gdk.ModifierType.CONTROL_MASK) != 0 )
						{
							if ( object.is_selected() )
							{
								/* Un-selecting a selected item */
								model.label.unselect_object( object );
							}
							else
							{
								/* Add to current selection */
								model.label.select_object( object );
							}
						}
						else
						{
							if ( !object.is_selected() )
							{
								/* remove any selections before adding */
								model.label.unselect_all();
								/* Add to current selection */
								model.label.select_object( object );
							}
						}

						move_last_x = x;
						move_last_y = y;

						state = State.ARROW_MOVE;
					}
					else
					{
						if ( (event.state & Gdk.ModifierType.CONTROL_MASK) == 0 )
						{
							model.label.unselect_all();
						}

						select_region_visible = true;
						select_region.x1 = x;
						select_region.y1 = y;
						select_region.x2 = x;
						select_region.y2 = y;

						state = State.ARROW_SELECT_REGION;
					}

					return_value = true;
				}
				else
				{

					if ( state == State.IDLE )
					{
						switch ( create_object_type )
						{
						case CreateType.BOX:
							create_object = new LabelObjectBox() as LabelObject;
							break;
						case CreateType.ELLIPSE:
							/* TODO */
							break;
						case CreateType.LINE: 
							/* TODO */
							break;
						case CreateType.IMAGE:
							/* TODO */
							break;
						case CreateType.TEXT:
							create_object = new LabelObjectText() as LabelObject;
							break;
						case CreateType.BARCODE:
							/* TODO */
							break;
						default:
							warning( "Invalid create type." );   /* Should not happen! */
							break;
						}

						create_object.set_position( x, y );
						create_object.set_size( 0, 0 );
						model.label.add_object( create_object );

						model.label.unselect_all();
						model.label.select_object( create_object );

						create_x0 = x;
						create_y0 = y;

						state = State.CREATE_DRAG;

						return_value = true;
					}

				}
				event.device.grab( bin_window, Gdk.GrabOwnership.APPLICATION, false,
				                   (Gdk.EventMask.BUTTON1_MOTION_MASK | Gdk.EventMask.BUTTON_RELEASE_MASK),
				                   null, event.time );
				break;

			case 3:
				context_menu_activate( event.button, event.time );
				return_value = true;
				break;

			default:
				break;

			}

			return return_value;
		}


		private bool on_button_release_event( Gdk.EventButton event )
		{
			bool return_value = false;

			canvas.grab_focus();

			Gdk.Window bin_window = canvas.get_bin_window();
			Gdk.Window window = canvas.get_window();

			Cairo.Context cr = Gdk.cairo_create( bin_window );

			/*
			 * Translate to label coordinates
			 */
			cr.scale( scale, scale );
			cr.translate( x0, y0 );

			double x = event.x;
			double y = event.y;
			cr.device_to_user( ref x, ref y );

			switch (event.button)
			{

			case 1:
				event.device.ungrab( event.time );
				/*
				 * Handle event as appropriate for mode
				 */
				if ( !in_object_create_mode )
				{

					switch (state)
					{
					case State.ARROW_RESIZE:
						resize_object = null;
						state = State.IDLE;
						break;

					case State.ARROW_SELECT_REGION:
						select_region_visible = false;
						select_region.x2 = x;
						select_region.y2 = y;
						model.label.select_region( select_region );
						update();
						state = State.IDLE;
						break;

					default:
						state = State.IDLE;
						break;
					}

					return_value = true;

				}
				else
				{

					Gdk.Cursor cursor = new Gdk.Cursor( Gdk.CursorType.LEFT_PTR );
					window.set_cursor( cursor );

					if ( (create_object.w < 4) && (create_object.h < 4) )
					{
						if ( create_object is LabelObjectText )
						{
							create_object.set_size( 0, 0 );
						}
						else
						{
							create_object.set_size( 72, 72 );
						}
					}

					in_object_create_mode = false;
					state = State.IDLE;
					
				}
				break;

			default:
				break;

			}

			return return_value;
		}


		private bool on_key_press_event( Gdk.EventKey event )
		{
			if ( !in_object_create_mode && (state == State.IDLE) )
			{
				switch (event.keyval)
				{

				case Gdk.Key.Left:
				case Gdk.Key.KP_Left:
					model.label.move_selection( (-1 / zoom), 0 );
					break;

				case Gdk.Key.Up:
				case Gdk.Key.KP_Up:
					model.label.move_selection( 0, (-1 / zoom) );
					break;

				case Gdk.Key.Right:
				case Gdk.Key.KP_Right:
					model.label.move_selection( (1 / zoom), 0 );
					break;

				case Gdk.Key.Down:
				case Gdk.Key.KP_Down:
					model.label.move_selection( 0, (1 / zoom) );
					break;

				case Gdk.Key.Delete:
				case Gdk.Key.KP_Delete:
					model.label.delete_selection();
					Gdk.Window window = canvas.get_window();
					Gdk.Cursor cursor = new Gdk.Cursor( Gdk.CursorType.LEFT_PTR );
					window.set_cursor( cursor );
					break;

				default:
					return false;

				}
			}

			return true;
		}


		private void handle_resize_motion( Cairo.Context cr,
		                                   double        x_pixels,
		                                   double        y_pixels )
		{
			cr.save();

			/*
			 * Change to item relative coordinates
			 */
			cr.translate( resize_object.x0, resize_object.y0 );
			cr.transform( resize_object.matrix );

			/*
			 * Initialize origin and 2 corners in object relative coordinates.
			 */
			double x0 = 0.0;
			double y0 = 0.0;

			double x1 = 0.0;
			double y1 = 0.0;

			double x2 = resize_object.w;
			double y2 = resize_object.h;

			/*
			 * Translate x,y into object relative coordinates.
			 */
			double x = x_pixels;
			double y = y_pixels;
			cr.device_to_user( ref x, ref y );

			/*
			 * Calculate new size
			 */
			double w, h;
			if ( resize_handle is HandleNorthWest )
			{
				w = double.max( x2 - x, 0 );
				h = double.max( y2 - y, 0 );
			}
			else if ( resize_handle is HandleNorth )
			{
				w = x2 - x1;
				h = double.max( y2 - y, 0 );
			}
			else if ( resize_handle is HandleNorthEast )
			{
				w = double.max( x - x1, 0 );
				h = double.max( y2 - y, 0 );
			}
			else if ( resize_handle is HandleEast )
			{
				w = double.max( x - x1, 0 );
				h = y2 - y1;
			}
			else if ( resize_handle is HandleSouthEast )
			{
				w = double.max( x - x1, 0 );
				h = double.max( y - y1, 0 );
			}
			else if ( resize_handle is HandleSouth )
			{
				w = x2 - x1;
				h = double.max( y - y1, 0 );
			}
			else if ( resize_handle is HandleSouthWest )
			{
				w = double.max( x2 - x, 0 );
				h = double.max( y - y1, 0 );
			}
			else if ( resize_handle is HandleWest )
			{
				w = double.max( x2 - x, 0 );
				h = y2 - y1;
			}
			else
			{
				assert_not_reached();
			}

			/*
			 * Set size
			 */
			if ( resize_honor_aspect )
			{
				resize_object.set_size_honor_aspect( w, h );
			}
			else
			{
				resize_object.set_size( w, h );
			}

			/*
			 * Adjust origin, if needed.
			 */
			if ( resize_handle is HandleNorthWest )
			{
				x0 += x2 - resize_object.w;
				y0 += y2 - resize_object.h;
			}
			else if ( (resize_handle is HandleNorth) || (resize_handle is HandleNorthEast) )
			{
				y0 += y2 - resize_object.h;
			}
			else if ( (resize_handle is HandleWest) || (resize_handle is HandleSouthWest) )
			{
				x0 += x2 - resize_object.w;
			}

			/*
			 * Put new origin back into world coordinates and set.
			 */
			cr.user_to_device( ref x0, ref y0 );
			cr.restore();
			cr.device_to_user( ref x0, ref y0 );
			resize_object.set_position( x0, y0 );
		}


	}

}
