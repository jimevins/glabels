/*  label_object.vala
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

	public abstract class LabelObject : Object
	{
		public signal void changed();
		public signal void moved();


		private   bool         selected;
		protected List<Handle> handles;
		protected Outline?     outline;

		private   double       aspect_ratio;



		/**
		 * Parent label
		 */
		public weak Label parent { get; set; }


		/**
		 * X coordinate of origin
		 */
		public double x0
		{
			get { return _x0; }

			set
			{
				if ( _x0 != value )
				{
					_x0 = value;
					moved();
				}
			}
		}
		private double _x0;


		/**
		 * Y coordinate of origin
		 */
		public double y0
		{
			get { return _y0; }

			set
			{
				if ( _y0 != value )
				{
					_y0 = value;
					moved();
				}
			}
		}
		private double _y0;


		/**
		 * Width of bounding box
		 */
		public virtual double w
		{
			get { return _w; }

			set
			{
				if ( _w != value )
				{
					_w = value;
					aspect_ratio = _h / _w;
					changed();
				}
			}
		}
		private double _w;


		/**
		 * Height of bounding box
		 */
		public virtual double h
		{
			get { return _h; }

			set
			{
				if ( _h != value )
				{
					_h = value;
					aspect_ratio = _h / _w;
					changed();
				}
			}
		}
		private double _h;


		/**
		 * Transformation matrix
		 */
		public Cairo.Matrix matrix
		{
			get { return _matrix; }

			set
			{
				if ( _matrix != value )
				{
					_matrix = value;
					changed();
				}
			}
		}
		private Cairo.Matrix _matrix;


		/**
		 * Text parameters interface
		 */
		public virtual string          font_family         { get; set; }
		public virtual double          font_size           { get; set; }
		public virtual Pango.Weight    font_weight         { get; set; }
		public virtual bool            font_italic_flag    { get; set; }
		public virtual bool            font_underline_flag { get; set; }
		public virtual ColorNode       text_color_node     { get; set; }
		public virtual Pango.Alignment text_alignment      { get; set; }
		public virtual ValignType      text_valignment     { get; set; }
		public virtual double          text_line_spacing   { get; set; }


		/**
		 * Shape parameters interface
		 */
		public virtual double          line_width          { get; set; }
		public virtual ColorNode       line_color_node     { get; set; }
		public virtual ColorNode       fill_color_node     { get; set; }


		/**
		 * Shadow state
		 */
		public bool shadow_state
		{
			get { return _shadow_state; }

			set
			{
				if ( _shadow_state != value )
				{
					_shadow_state = value;
					changed();
				}
			}
		}
		private bool _shadow_state;


		/**
		 * Shadow x offset
		 */
		public double shadow_x
		{
			get { return _shadow_x; }

			set
			{
				if ( _shadow_x != value )
				{
					_shadow_x = value;
					changed();
				}
			}
		}
		private double _shadow_x;


		/**
		 * Shadow y offset
		 */
		public double shadow_y
		{
			get { return _shadow_y; }

			set
			{
				if ( _shadow_y != value )
				{
					_shadow_y = value;
					changed();
				}
			}
		}
		private double _shadow_y;


		/**
		 * Shadow color node
		 */
		public ColorNode shadow_color_node
		{
			get { return _shadow_color_node; }

			set
			{
				if ( _shadow_color_node != value )
				{
					_shadow_color_node = value;
					changed();
				}
			}
		}
		private ColorNode _shadow_color_node;


		/**
		 * Shadow opacity
		 */
		public double shadow_opacity
		{
			get { return _shadow_opacity; }

			set
			{
				if ( _shadow_opacity != value )
				{
					_shadow_opacity = value;
					changed();
				}
			}
		}
		private double _shadow_opacity;



		/*
		 * Methods that concrete LabelObjects must implement.
		 */

		public abstract LabelObject dup();

		protected abstract void draw_object( Cairo.Context cr, bool in_editor, MergeRecord? record );
		protected abstract void draw_shadow( Cairo.Context cr, bool in_editor, MergeRecord? record );

		protected abstract bool is_object_located_at( Cairo.Context cr, double x, double y );



		public LabelObject()
		{
			_x0 = 0;
			_y0 = 0;
			_matrix = Cairo.Matrix.identity();

			_shadow_state            = false;
			_shadow_x                = 3.6;
			_shadow_y                = 3.6;
			_shadow_color_node       = ColorNode.from_color( Color.black() );
			_shadow_opacity          = 0.5;

			selected = false;
		}


		protected void set_common_properties_from_object( LabelObject src_object )
		{
			parent            = src_object.parent;
			x0                = src_object.x0;
			y0                = src_object.y0;
			matrix            = src_object.matrix;

			w                 = src_object.w;
			h                 = src_object.h;

			shadow_state      = src_object.shadow_state;
			shadow_x          = src_object.shadow_x;
			shadow_y          = src_object.shadow_y;
			shadow_color_node = src_object.shadow_color_node;
			shadow_opacity    = src_object.shadow_opacity;
		}


		public void select()
		{
			selected = true;
		}


		public void unselect()
		{
			selected = false;
		}


		public bool is_selected()
		{
			return selected;
		}


		/*
		 * Virtual method defaults, these methods are used to determine if generic properties, such as those
		 * controlled by the PropertyBar, are available for the object.
		 */

		public virtual bool can_text()
		{
			return false;
		}

		public virtual bool can_fill()
		{
			return false;
		}

		public virtual bool can_line_color()
		{
			return false;
		}

		public virtual bool can_line_width()
		{
			return false;
		}


		public void set_position( double x0,
		                          double y0 )
		{
			if ( ( _x0 != x0 ) || ( _y0 != y0 ) )
			{
				_x0 = x0;
				_y0 = y0;

				moved();
			}
		}


		public void set_position_relative( double dx,
		                                   double dy )
		{
			if ( ( dx != 0 ) || ( dy != 0 ) )
			{
				_x0 += dx;
				_y0 += dy;

				moved();
			}
		}


		public void set_size( double w,
		                      double h )
		{
			this.w = w;
			this.h = h;

			aspect_ratio = h / w;
		}


		public void set_size_honor_aspect( double w,
		                                   double h )
		{
			if ( h > (w * aspect_ratio) )
			{
				h = w * aspect_ratio;
			}
			else
			{
				w = h / aspect_ratio;
			}

			if ( ( _w != w ) || ( _h != h ) )
			{
				_w = w;
				_h = h;

				changed();
			}
		}


		public void set_w_honor_aspect( double w )
		{
			double h = w * aspect_ratio;

			if ( ( _w != w ) || ( _h != h ) )
			{
				_w = w;
				_h = h;

				changed();
			}
		}


		public void set_h_honor_aspect( double h )
		{
			double w = h / aspect_ratio;

			if ( ( _w != w ) || ( _h != h ) )
			{
				_w = w;
				_h = h;

				changed();
			}
		}


		public LabelRegion get_extent()
		{
			double xa1 =   - line_width/2;
			double ya1 =   - line_width/2;
			double xa2 = w + line_width/2;
			double ya2 =   - line_width/2;
			double xa3 = w + line_width/2;
			double ya3 = h + line_width/2;
			double xa4 =   - line_width/2;
			double ya4 = h + line_width/2;

			matrix.transform_point( ref xa1, ref ya1 );
			matrix.transform_point( ref xa2, ref ya2 );
			matrix.transform_point( ref xa3, ref ya3 );
			matrix.transform_point( ref xa4, ref ya4 );

			LabelRegion region = LabelRegion();
			region.x1 = double.min( xa1, double.min( xa2, double.min( xa3, xa4 ) ) ) + x0;
			region.y1 = double.min( ya1, double.min( ya2, double.min( ya3, ya4 ) ) ) + y0;
			region.x2 = double.max( xa1, double.max( xa2, double.max( xa3, xa4 ) ) ) + x0;
			region.y2 = double.max( ya1, double.max( ya2, double.max( ya3, ya4 ) ) ) + y0;

			return region;
		}


		public void rotate( double theta_degs )
		{
			if ( theta_degs != 0 )
			{
				_matrix.rotate( theta_degs * (Math.PI / 180) );
				changed();
			}
		}


		public void flip_horiz()
		{
			_matrix.scale( -1, 1 );
			changed();
		}


		public void flip_vert()
		{
			_matrix.scale( 1, -1 );
			changed();
		}


		public void draw( Cairo.Context cr,
		                  bool          in_editor,
		                  MergeRecord?  record )
		{
			cr.save();
			cr.translate( x0, y0 );

			if ( shadow_state )
			{
				cr.save();
				cr.translate( shadow_x, shadow_y );
				cr.transform( matrix );
				draw_shadow( cr, in_editor, record );
				cr.restore();
			}

			cr.transform( matrix );
			draw_object( cr, in_editor, record );

			cr.restore();
		}


		public void draw_selection_highlight( Cairo.Context cr )
		{
			cr.save();
			cr.translate( x0, y0 );
			cr.transform( matrix );

			if ( outline != null )
			{
				outline.draw( cr );
			}

			foreach( Handle handle in handles )
			{
				handle.draw( cr );
			}

			cr.restore();
		}


		public bool is_located_at( Cairo.Context cr,
		                           double        x_pixels,
		                           double        y_pixels )
		{
			cr.save();
			cr.translate( x0, y0 );
			cr.transform( matrix );

			double x = x_pixels;
			double y = y_pixels;
			cr.device_to_user( ref x, ref y );

			bool ret_val = is_object_located_at( cr, x, y );

			if ( (outline != null) && is_selected() )
			{
				if ( outline.in_stroke( cr, x, y ) )
				{
					ret_val = true;
				}
			}

			cr.restore();

			return ret_val;
		}


		public Handle? handle_at(  Cairo.Context cr,
		                           double        x_pixels,
		                           double        y_pixels )
		{
			Handle ret_val = null;

			cr.save();
			cr.translate( x0, y0 );
			cr.transform( matrix );

			double x = x_pixels;
			double y = y_pixels;
			cr.device_to_user( ref x, ref y );

			foreach( Handle handle in handles )
			{
				handle.cairo_path( cr );

				if ( cr.in_fill( x, y ) )
				{
					ret_val = handle;
					break;
				}
			}

			cr.restore();

			return ret_val;
		}


	}

}
