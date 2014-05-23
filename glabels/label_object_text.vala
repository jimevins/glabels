/*  label_object_text.vala
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

	public class LabelObjectText : LabelObject
	{
		public  Gtk.TextBuffer   buffer;

		private Gtk.TextTagTable tag_table;
		private bool             size_changed;

		private const double FONT_SCALE = (72.0/96.0);
		private const double SELECTION_SLOP_PIXELS = 4.0;
		private const double TEXT_MARGIN = 3.0;

		private const Color  EMPTY_TEXT_COLOR = { 0.5, 0.5, 0.5, 0.5 };

		public bool auto_shrink { get; set; default = false; }


		public LabelObjectText.with_parent( Label parent )
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
			get
			{
				if (size_changed)
				{
					recalculate_size();
				}
				return _w;
			}

			set
			{
				if ( _w_raw != value )
				{
					_w_raw = value;
					size_changed = true;
					changed();
				}
			}
		}
		private double _w;


		/**
		 * Height of bounding box
		 */
		public override double h
		{
			get
			{
				if (size_changed)
				{
					recalculate_size();
				}
				return _h;
			}

			set
			{
				if ( _h_raw != value )
				{
					_h_raw = value;
					size_changed = true;
					changed();
				}
			}
		}
		private double _h;


		/**
		 * Font family
		 */
		public override string font_family
		{
			get { return _font_family; }

			set
			{
				if ( _font_family != value )
				{
					_font_family = value;
					size_changed = true;
					changed();
				}
			}
		}
		private string _font_family;


		/**
		 * Font size
		 */
		public override double font_size
		{
			get { return _font_size; }

			set
			{
				if ( _font_size != value )
				{
					_font_size = value;
					size_changed = true;
					changed();
				}
			}
		}
		private double _font_size;


		/**
		 * Font weight
		 */
		public override Pango.Weight font_weight
		{
			get { return _font_weight; }

			set
			{
				if ( _font_weight != value )
				{
					_font_weight = value;
					size_changed = true;
					changed();
				}
			}
		}
		private Pango.Weight _font_weight = Pango.Weight.NORMAL;


		/**
		 * Font italic flag
		 */
		public override bool font_italic_flag
		{
			get { return _font_italic_flag; }

			set
			{
				if ( _font_italic_flag != value )
				{
					_font_italic_flag = value;
					size_changed = true;
					changed();
				}
			}
		}
		private bool _font_italic_flag;


		/**
		 * Font underline flag
		 */
		public override bool font_underline_flag
		{
			get { return _font_underline_flag; }

			set
			{
				if ( _font_underline_flag != value )
				{
					_font_underline_flag = value;
					size_changed = true;
					changed();
				}
			}
		}
		private bool _font_underline_flag;


		/**
		 * Text color node
		 */
		public override ColorNode text_color_node
		{
			get { return _text_color_node; }

			set
			{
				if ( _text_color_node != value )
				{
					_text_color_node = value;
					changed();
				}
			}
		}
		private ColorNode _text_color_node;


		/**
		 * Text alignment
		 */
		public override Pango.Alignment text_alignment
		{
			get { return _text_alignment; }

			set
			{
				if ( _text_alignment != value )
				{
					_text_alignment = value;
					changed();
				}
			}
		}
		private Pango.Alignment _text_alignment;


		/**
		 * Text vertical alignment
		 */
		public override ValignType text_valignment
		{
			get { return _text_valignment; }

			set
			{
				if ( _text_valignment != value )
				{
					_text_valignment = value;
					changed();
				}
			}
		}
		private ValignType _text_valignment;


		/**
		 * Text line spacing
		 */
		public override double text_line_spacing
		{
			get { return _text_line_spacing; }

			set
			{
				if ( _text_line_spacing != value )
				{
					_text_line_spacing = value;
					size_changed = true;
					changed();
				}
			}
		}
		private double _text_line_spacing;



		public LabelObjectText()
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

			_font_family             = prefs.default_font_family;
			_font_size               = prefs.default_font_size;
			_font_weight             = prefs.default_font_weight;
			_font_italic_flag        = prefs.default_font_italic_flag;
			_font_underline_flag     = false;
			_text_color_node         = ColorNode.from_color( prefs.default_text_color );
			_text_alignment          = prefs.default_text_alignment;
			_text_valignment         = ValignType.TOP;
			_text_line_spacing       = prefs.default_text_line_spacing;

			tag_table    = new Gtk.TextTagTable();
			buffer       = new Gtk.TextBuffer( tag_table );
			size_changed = true;

			buffer.begin_user_action.connect( on_buffer_begin_user_action );
			buffer.changed.connect( on_buffer_changed );
		}


		public override bool can_text()
		{
			return true;
		}


		public override LabelObject dup()
		{
			LabelObjectText copy = new LabelObjectText();

			copy.set_common_properties_from_object( this );

			return copy;
		}


		public override void draw_object( Cairo.Context cr, bool in_editor, MergeRecord? record )
		{
			if ( in_editor && (buffer.get_char_count() == 0) )
			{
				draw_empty_text( cr );
			}
			else
			{
				Color text_color = text_color_node.expand( record );

				if ( in_editor && text_color_node.field_flag )
				{
					text_color = Color.from_rgba( 0, 0, 0, 0.5 );
				}

				draw_text_real( cr, in_editor, record, text_color );
			}
		}


		public override void draw_shadow( Cairo.Context cr, bool in_editor, MergeRecord? record )
		{
			Color shadow_color = shadow_color_node.expand( record );

			if ( in_editor && shadow_color_node.field_flag )
			{
				shadow_color = Color.black();
			}

			shadow_color.set_opacity( shadow_opacity );

			draw_text_real( cr, in_editor, record, shadow_color );
		}


		private void draw_empty_text( Cairo.Context cr )
		{
			set_empty_text_path( cr );

			cr.set_source_rgba( EMPTY_TEXT_COLOR.r, EMPTY_TEXT_COLOR.g, EMPTY_TEXT_COLOR.b, EMPTY_TEXT_COLOR.a );
			cr.fill();
		}


		private void draw_text_real( Cairo.Context cr, bool in_editor, MergeRecord? record, Color color )
		{
			cr.set_source_rgba( color.r, color.g, color.b, color.a );
			layout_text( cr, in_editor, record, false );
		}


		public override bool is_object_located_at( Cairo.Context cr, double x, double y )
		{
			if ( (x >= 0) && (x <= w) && (y >=0) && (y <= h) )
			{
				cr.new_path();
				if ( buffer.get_char_count() == 0 )
				{
					set_empty_text_path( cr );
				}
				else
				{
					layout_text( cr, true, null, true );
				}
				if ( cr.in_fill( x, y ) )
				{
					return true;
				}

				double scale_x = 1.0;
				double scale_y = 1.0;
				cr.device_to_user_distance( ref scale_x, ref scale_y );
				cr.set_line_width( 2*SELECTION_SLOP_PIXELS*scale_x );
				if (cr.in_stroke( x, y ))
				{
					return true;
				}
			}

			return false;
		}


		private void layout_text( Cairo.Context cr,
		                          bool in_editor,
		                          MergeRecord? record,
		                          bool path_only )
		{
			/*
			 * Workaround for pango Bug#700592, which is a regression of Bug#341481.
			 * Render font at device scale and scale font size accordingly.
			 */
			double scale_x = 1.0;
			double scale_y = 1.0;
			cr.device_to_user_distance( ref scale_x, ref scale_y );
			scale_x = Math.fabs( scale_x );
			scale_y = Math.fabs( scale_y );

			cr.save();
			cr.scale( scale_x, scale_y );

			TextLines lines = get_lines();
			string    text = lines.expand( record );

			Pango.Style style = font_italic_flag ? Pango.Style.ITALIC : Pango.Style.NORMAL;

			double scaled_font_size   = font_size * FONT_SCALE;

			if (!in_editor && (record != null) && auto_shrink)
			{
				scaled_font_size = auto_shrink_font_size( cr,
				                                          font_family, scaled_font_size, font_weight, style,
				                                          text, w, h );
			}


			Pango.Layout layout = Pango.cairo_create_layout( cr );

			Cairo.FontOptions font_options = new Cairo.FontOptions();
			font_options.set_hint_metrics( Cairo.HintMetrics.OFF );
			Pango.Context context = layout.get_context();
			Pango.cairo_context_set_font_options( context, font_options );

			Pango.FontDescription desc = new Pango.FontDescription();
			desc.set_family( font_family );
			desc.set_weight( font_weight );
			desc.set_size( (int)(scaled_font_size * Pango.SCALE/scale_x + 0.5) );
			desc.set_style( style );
			layout.set_font_description( desc );

			layout.set_text( text, -1 );
			layout.set_spacing( (int)(scaled_font_size * (text_line_spacing-1) * Pango.SCALE/scale_x + 0.5) );
			layout.set_width( (int)(w * Pango.SCALE/scale_x + 0.5) );
			layout.set_wrap( Pango.WrapMode.WORD );
			layout.set_alignment( text_alignment );

			int iw, ih;
			layout.get_pixel_size( out iw, out ih );

			double y;
			switch (text_valignment)
			{
			case ValignType.CENTER:
				y = (h/scale_x - ih) / 2;
				break;
			case ValignType.BOTTOM:
				y = h/scale_x - ih;
				break;
			default:
				y = 0;
				break;
			}

			cr.move_to( TEXT_MARGIN/scale_x, y );
			if ( path_only )
			{
				Pango.cairo_layout_path( cr, layout );
			}
			else
			{
				Pango.cairo_show_layout( cr, layout );
			}

			cr.restore();
		}


		private void set_empty_text_path( Cairo.Context cr )
		{
			/*
			 * Workaround for pango Bug#700592, which is a regression of Bug#341481.
			 * Render font at device scale and scale font size accordingly.
			 */
			double scale_x = 1.0;
			double scale_y = 1.0;
			cr.device_to_user_distance( ref scale_x, ref scale_y );
			scale_x = Math.fabs( scale_x );
			scale_y = Math.fabs( scale_y );

			cr.save();
			cr.scale( scale_x, scale_y );

			Pango.Layout layout = Pango.cairo_create_layout( cr );

			Cairo.FontOptions font_options = new Cairo.FontOptions();
			font_options.set_hint_metrics( Cairo.HintMetrics.OFF );
			Pango.Context context = layout.get_context();
			Pango.cairo_context_set_font_options( context, font_options );

			Pango.FontDescription desc = new Pango.FontDescription();
			desc.set_family( "Sans" );
			desc.set_weight( Pango.Weight.NORMAL );
			desc.set_size( (int)(12 * FONT_SCALE * Pango.SCALE/scale_x + 0.5) );
			desc.set_style( Pango.Style.NORMAL );
			layout.set_font_description( desc );

			layout.set_text( _("Text"), -1 );

			cr.move_to( TEXT_MARGIN/scale_x, 0 );
			Pango.cairo_layout_path( cr, layout );

			cr.restore();
		}


		private double auto_shrink_font_size( Cairo.Context cr,
		                                      string        family,
		                                      double        size,
		                                      Pango.Weight  weight,
		                                      Pango.Style   style,
		                                      string        text,
		                                      double        width,
		                                      double        height )
		{
			Pango.Layout layout = Pango.cairo_create_layout( cr );

			Pango.FontDescription desc = new Pango.FontDescription();
			desc.set_family( family );
			desc.set_weight( weight );
			desc.set_style( style );
			desc.set_size( (int)(size * Pango.SCALE + 0.5) );
			layout.set_font_description( desc );

			layout.set_text( text, -1 );
			layout.set_spacing( (int)(size * (text_line_spacing-1) * Pango.SCALE + 0.5) );
			layout.set_width( -1 );

			int iw, ih;
			layout.get_size( out iw, out ih );
			double layout_width  = iw / (double)Pango.SCALE;
			double layout_height = ih / (double)Pango.SCALE;

			double new_wsize = size;
			double new_hsize = size;
			if ( layout_width > width )
			{
				/* Scale down. */
				new_wsize = size * (width-2*TEXT_MARGIN) / layout_width;

				/* Round down to nearest 1/2 point */
				new_wsize = (int)(new_wsize*2.0) / 2.0;

				/* don't get ridiculously small. */
				if (new_wsize < 1.0)
				{
					new_wsize = 1.0;
				}
			}

			if ( layout_height > height )
			{
				/* Scale down. */
				new_hsize = size * height / layout_height;

				/* Round down to nearest 1/2 point */
				new_hsize = (int)(new_hsize*2.0) / 2.0;

				/* don't get ridiculously small. */
				if (new_hsize < 1.0)
				{
					new_hsize = 1.0;
				}
			}

			return (new_wsize < new_hsize ? new_wsize : new_hsize);
		}


		private void recalculate_size()
		{
			Pango.Context context = Gdk.pango_context_get();
			Cairo.FontOptions font_options = new Cairo.FontOptions();
			font_options.set_hint_metrics( Cairo.HintMetrics.OFF );
			Pango.cairo_context_set_font_options( context, font_options );

			Pango.Layout layout = new Pango.Layout( context );
			Pango.FontDescription desc = new Pango.FontDescription();

			if ( buffer.get_char_count() != 0 )
			{
				desc.set_family( font_family );
				desc.set_weight( font_weight );
				desc.set_style( font_italic_flag ? Pango.Style.ITALIC : Pango.Style.NORMAL );
				desc.set_size( (int)(font_size * FONT_SCALE * Pango.SCALE + 0.5) );
				layout.set_font_description( desc );
				layout.set_spacing( (int)(font_size * FONT_SCALE * (text_line_spacing-1) * Pango.SCALE + 0.5) );
				layout.set_text( get_lines().expand( null ), -1 );
			}
			else
			{
				desc.set_family( "Sans" );
				desc.set_weight( Pango.Weight.NORMAL );
				desc.set_style( Pango.Style.NORMAL );
				desc.set_size( (int)(12 * FONT_SCALE * Pango.SCALE + 0.5) );
				layout.set_font_description( desc );
				layout.set_text( _("Text"), -1 );
			}

			if ( _w_raw == 0 )
			{
				layout.set_width( -1 );
			}
			else
			{
				layout.set_width( (int)(_w_raw * Pango.SCALE + 0.5) );
			}

			int iw, ih;
			layout.get_size( out iw, out ih );

			if ( _w_raw != 0.0 )
			{
				_w = _w_raw;
			}
			else
			{
				_w = iw / Pango.SCALE + 2*TEXT_MARGIN;
			}

			double h_temp = ih / Pango.SCALE;
			if ( h_temp < _h_raw )
			{
				_h = h_raw;
			}
			else
			{
				_h = h_temp;
			}

			size_changed = false;
		}


		private void on_buffer_begin_user_action()
		{
		}


		private void on_buffer_changed()
		{
			size_changed = true;

			changed();
		}


		public TextLines get_lines()
		{
			Gtk.TextIter start, end;

			buffer.get_bounds( out start, out end );
			string text = buffer.get_text( start, end, false );
			TextLines lines = new TextLines.parse( text );

			return lines;
		}


		public void set_lines( TextLines lines )
		{
			buffer.set_text( lines.expand( null ) );
		}

	}

}
