/*  label_object_barcode.vala
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

	public class LabelObjectBarcode : LabelObject
	{

		private const double FONT_SCALE = (72.0/96.0);
		private const double SELECTION_SLOP_PIXELS = 4.0;
		private const double TEXT_MARGIN = 3.0;


		private glbarcode.Barcode? cached_bc;


		public LabelObjectBarcode.with_parent( Label parent )
		{
			this();
			parent.add_object( this );
		}


		/**
		 * Raw width of bounding box
		 */
		public double w_raw
		{
			get { return _w_raw; }
		}
		private double _w_raw;


		/**
		 * Raw height of bounding box
		 */
		public double h_raw
		{
			get { return _h_raw; }
		}
		private double _h_raw;


		/**
		 * Width of bounding box
		 */
		public override double w
		{
			get { return _w_actual; }

			set
			{
				if ( _w_raw != value )
				{
					_w_raw = value;
					update_cached_barcode();
					changed();
				}
			}
		}
		private double _w_actual;


		/**
		 * Height of bounding box
		 */
		public override double h
		{
			get { return _h_actual; }

			set
			{
				if ( _h_raw != value )
				{
					_h_raw = value;
					update_cached_barcode();
					changed();
				}
			}
		}
		private double _h_actual;


		/**
		 * Barcode data
		 */
		public override TextNode bc_data_node
		{
			get { return _bc_data_node; }

			set
			{
				if ( _bc_data_node != value )
				{
					_bc_data_node = value;
					update_cached_barcode();
					changed();
				}
			}
		}
		private TextNode _bc_data_node;


		/**
		 * Barcode type
		 */
		public override string bc_type
		{
			get { return _bc_type; }

			set
			{
				if ( _bc_type != value )
				{
					_bc_type = value;
					update_cached_barcode();
					changed();
				}
			}

		}
		private string _bc_type;


		/**
		 * Barcode show text flag
		 */
		public override bool bc_text_flag
		{
			get { return _bc_text_flag; }

			set
			{
				if ( _bc_text_flag != value )
				{
					_bc_text_flag = value;
					update_cached_barcode();
					changed();
				}
			}
		}
		private bool _bc_text_flag;


		/**
		 * Barcode checksum flag
		 */
		public override bool bc_checksum_flag
		{
			get { return _bc_checksum_flag; }

			set
			{
				if ( _bc_checksum_flag != value )
				{
					_bc_checksum_flag = value;
					update_cached_barcode();
					changed();
				}
			}
		}
		private bool _bc_checksum_flag;


		/**
		 * Barcode format digits
		 */
		public override int bc_format_digits
		{
			get { return _bc_format_digits; }

			set
			{
				if ( _bc_format_digits != value )
				{
					_bc_format_digits = value;
					update_cached_barcode();
					changed();
				}
			}
		}
		private int _bc_format_digits;


		/**
		 * Barcode color node
		 */
		public override ColorNode bc_color_node
		{
			get { return _bc_color_node; }

			set
			{
				if ( _bc_color_node != value )
				{
					_bc_color_node = value;
					update_cached_barcode();
					changed();
				}
			}
		}
		private ColorNode _bc_color_node;


		/**
		 * Style information
		 */
		public override BarcodeStyle bc_style { get; private set; }


		public LabelObjectBarcode()
		{
			handles.append( new HandleSouthEast( this ) );
			handles.append( new HandleSouthWest( this ) );
			handles.append( new HandleNorthEast( this ) );
			handles.append( new HandleNorthWest( this ) );
			handles.append( new HandleEast( this ) );
			handles.append( new HandleSouth( this ) );
			handles.append( new HandleWest( this ) );
			handles.append( new HandleNorth( this ) );

			outline = new Outline( this );

			Prefs prefs = new Prefs();

			_bc_style              = BarcodeBackends.lookup_style_from_id( "Code39" );
			_bc_type               = _bc_style.id;
			_bc_text_flag          = _bc_style.can_text;
			_bc_checksum_flag      = _bc_style.can_checksum;
			_bc_format_digits      = _bc_style.prefered_n;

			_bc_data_node          = new TextNode( false, _bc_style.default_digits );

			_bc_color_node         = ColorNode.from_color( prefs.default_line_color );

			update_cached_barcode();
		}


		public override LabelObject dup()
		{
			LabelObjectBarcode copy = new LabelObjectBarcode();

			copy.set_common_properties_from_object( this );

			return copy;
		}


		public override void draw_object( Cairo.Context cr, bool in_editor, MergeRecord? record )
		{
			Color bc_color = bc_color_node.expand( record );

			if ( in_editor && line_color_node.field_flag )
			{
				bc_color = Color.from_rgba( 0, 0, 0, 0.5 );
			}

			cr.set_source_rgba( bc_color.r, bc_color.g, bc_color.b, bc_color.a );

			if ( !in_editor && bc_data_node.field_flag )
			{

				string text = bc_data_node.expand( record );
				glbarcode.Barcode bc = glbarcode.Factory.create_barcode( bc_type, bc_text_flag, bc_checksum_flag, w, h, text );

				if ( bc != null )
				{
					bc.render( new glbarcode.CairoRenderer( cr ) );
				}

			}
			else
			{

				if ( cached_bc == null )
				{
					create_alt_msg_path( cr, _("Invalid barcode type") );
					cr.fill();
				}
				else
				{
					if ( cached_bc.is_empty )
					{
						create_alt_msg_path( cr, _("Barcode data empty") );
						cr.fill();
					}
					else if ( !cached_bc.is_data_valid )
					{
						create_alt_msg_path( cr, _("Invalid barcode data") );
						cr.fill();
					}
					else
					{
						cached_bc.render( new glbarcode.CairoRenderer( cr ) );
					}
				}

			}
		}


		public override void draw_shadow( Cairo.Context cr, bool in_editor, MergeRecord? record )
		{
			/* Shadows unsupported for barcodes.  Shadows would likely make the barcode unreadable. */
		}


		public override bool is_object_located_at( Cairo.Context cr, double x, double y )
		{
			if ( (x >= 0) && (x <= w) && (y >= 0) && (y <= h) )
			{

				if ( cached_bc == null )
				{
					create_alt_msg_path( cr, _("Invalid barcode type") );
				}
				else
				{
					if ( cached_bc.is_empty )
					{
						create_alt_msg_path( cr, _("Barcode data empty") );
					}
					else if ( !cached_bc.is_data_valid )
					{
						create_alt_msg_path( cr, _("Invalid barcode data") );
					}
					else
					{
						cached_bc.render( new glbarcode.CairoRenderer( cr, false ) );
					}
				}

				if ( cr.in_fill( x, y ) )
				{
					return true;
				}


				double scale_x = 1.0;
				double scale_y = 1.0;
				cr.device_to_user_distance( ref scale_x, ref scale_y );

				cr.set_line_width( 2*SELECTION_SLOP_PIXELS*scale_x );

				if ( cr.in_stroke( x, y ) )
				{
					return true;
				}

			}

			return false;
		}


		private void create_alt_msg_path( Cairo.Context cr, string msg )
		{
			cr.save();

			Pango.Layout layout = Pango.cairo_create_layout( cr );

			Cairo.FontOptions font_options = new Cairo.FontOptions();
			font_options.set_hint_metrics( Cairo.HintMetrics.OFF );
			Pango.Context context = layout.get_context();
			Pango.cairo_context_set_font_options( context, font_options );

			Pango.FontDescription desc = new Pango.FontDescription();
			desc.set_family( "Sans" );
			desc.set_weight( Pango.Weight.NORMAL );
			desc.set_size( (int)(12 * FONT_SCALE * Pango.SCALE) );
			desc.set_style( Pango.Style.NORMAL );
			layout.set_font_description( desc );

			layout.set_text( msg, -1 );

			cr.move_to( TEXT_MARGIN, 0 );
			Pango.cairo_layout_path( cr, layout );

			cr.restore();
		}


		private void update_cached_barcode()
		{
			string data;

			_bc_style = BarcodeBackends.lookup_style_from_id( bc_type );

			if ( bc_data_node.field_flag )
			{
				data = _bc_style.get_example_digits( bc_format_digits );
			}
			else
			{
				data = bc_data_node.expand( null );
			}

			cached_bc = glbarcode.Factory.create_barcode( bc_type, bc_text_flag, bc_checksum_flag, _w_raw, _h_raw, data );

			if ( (cached_bc == null) || !cached_bc.is_data_valid )
			{
				/* Try again with default digits, but don't save -- just extract size. */
				data = _bc_style.get_example_digits( bc_format_digits );

				glbarcode.Barcode bc = glbarcode.Factory.create_barcode( bc_type, bc_text_flag, bc_checksum_flag, _w_raw, _h_raw, data );

				if ( bc != null )
				{
					_w_actual = bc.w;
					_h_actual = bc.h;
				}
				else
				{
					/* If we still can't render, just set a default size. */
					_w_actual = 144;
					_h_actual = 72;
				}

			}
			else
			{
				_w_actual = cached_bc.w;
				_h_actual = cached_bc.h;
			}

			if ( !_bc_style.can_text )
			{
				_bc_text_flag = false;
			}
			else if ( !_bc_style.text_optional )
			{
				_bc_text_flag = true;
			}

			if ( !_bc_style.can_checksum )
			{
				_bc_checksum_flag = false;
			}
			else if ( !_bc_style.checksum_optional )
			{
				_bc_checksum_flag = true;
			}

			if ( !_bc_style.can_freeform )
			{
				_bc_format_digits = _bc_style.prefered_n;
			}

		}

	}

}
