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

		public bool auto_shrink { get; set; default = false; }


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
			Color text_color = text_color_node.expand( record );

			if ( in_editor && text_color_node.field_flag )
			{
				text_color = Color.from_rgba( 0, 0, 0, 0.5 );
			}

			draw_text_real( cr, in_editor, record, text_color );
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


		private void draw_text_real( Cairo.Context cr, bool in_editor, MergeRecord? record, Color color )
		{
			set_text_path( cr, in_editor, record);

			cr.set_source_rgba( color.r, color.g, color.b, color.a );
			cr.fill();
		}


		public override bool is_object_located_at( Cairo.Context cr, double x, double y )
		{
			if ( (x >= 0) && (x <= w) && (y >=0) && (y <= h) )
			{
				cr.new_path();
				set_text_path( cr, true, null);
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


		private void set_text_path( Cairo.Context cr, bool in_editor, MergeRecord? record )
		{
			cr.save();

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
			desc.set_size( (int)(scaled_font_size * Pango.SCALE + 0.5) );
			desc.set_style( style );
			layout.set_font_description( desc );

			layout.set_text( text, -1 );
			layout.set_spacing( (int)(scaled_font_size * (text_line_spacing-1) * Pango.SCALE + 0.5) );
			layout.set_width( (int)(w * Pango.SCALE + 0.5) );
			layout.set_wrap( Pango.WrapMode.WORD );
			layout.set_alignment( text_alignment );

			int iw, ih;
			layout.get_pixel_size( out iw, out ih );

			double y;
			switch (text_valignment)
			{
			case ValignType.CENTER:
				y = (h - ih) / 2;
				break;
			case ValignType.BOTTOM:
				y = h - ih;
				break;
			default:
				y = 0;
				break;
			}

			cr.move_to( TEXT_MARGIN, y );
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


		private void on_buffer_begin_user_action()
		{
		}


		private void on_buffer_changed()
		{
			size_changed = true;

			changed();
		}


		private TextLines get_lines()
		{
			Gtk.TextIter start, end;

			buffer.get_bounds( out start, out end );
			string text = buffer.get_text( start, end, false );
			TextLines lines = new TextLines( text );

			return lines;
		}

	}

}
