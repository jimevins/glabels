/*  object_editor.vala
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

	class ObjectEditor : Gtk.Box
	{
		private Prefs           prefs;
		private Units           units;

		private Label           label;
		private LabelObject?    object;


		private double          line_width;
		private ColorNode       line_color_node;

		private ColorNode       fill_color_node;

		private double          x;
		private double          y;

		private double          w;
		private double          h;

		private bool            shadow_enable;
		private double          shadow_x;
		private double          shadow_y;
		private ColorNode       shadow_color_node;
		private double          shadow_opacity;


		private double          w_max;
		private double          h_max;


		private Gtk.Image       title_image;
		private Gtk.Label       title_label;
		private Gtk.Notebook    notebook;

		private Gtk.Box         line_fill_page_box;
		private Gtk.Box         pos_size_page_box;
		private Gtk.Box         shadow_page_box;

		private Gtk.SpinButton  line_width_spin;
		private Gtk.Box         line_color_box;
		private ColorButton     line_color_button;

		private Gtk.Box         fill_color_box;
		private ColorButton     fill_color_button;

		private Gtk.SpinButton  pos_x_spin;
		private Gtk.SpinButton  pos_y_spin;
		private Gtk.Label       pos_x_units_label;
		private Gtk.Label       pos_y_units_label;

		private Gtk.SpinButton  size_w_spin;
		private Gtk.SpinButton  size_h_spin;
		private Gtk.Label       size_w_units_label;
		private Gtk.Label       size_h_units_label;
		private Gtk.CheckButton size_aspect_check;
		private Gtk.Button      size_reset_image_button;

		private Gtk.CheckButton shadow_enable_check;
		private Gtk.Grid        shadow_controls_grid;
		private Gtk.SpinButton  shadow_x_spin;
		private Gtk.SpinButton  shadow_y_spin;
		private Gtk.Label       shadow_x_units_label;
		private Gtk.Label       shadow_y_units_label;
		private Gtk.Box         shadow_color_box;
		private ColorButton     shadow_color_button;
		private Gtk.SpinButton  shadow_opacity_spin;


		public ObjectEditor()
		{
			/* Load the user interface. */
			Gtk.Builder builder = new Gtk.Builder();
			try
			{
				string file = GLib.Path.build_filename( Config.DATADIR, Config.GLABELS_BRANCH, "ui", "object_editor.ui" );
				string[] objects = { "object_editor_vbox",
				                     "line_width_adjustment",
				                     "size_w_adjustment", "size_h_adjustment",
				                     "pos_x_adjustment", "pos_y_adjustment",
				                     "shadow_x_adjustment", "shadow_y_adjustment", "shadow_opacity_adjustment",
				                     "page_sizegroup", "width_sizegroup", "label_sizegroup", "color_box_sizegroup",
				                     null };
				builder.add_objects_from_file( file, objects );
			}
			catch ( Error err )
			{
				error( "Error: %s", err.message );
			}

			Gtk.VBox object_editor_vbox = builder.get_object( "object_editor_vbox" ) as Gtk.VBox;
			pack_start( object_editor_vbox );

			title_image  = builder.get_object( "title_image" ) as Gtk.Image;
			title_label  = builder.get_object( "title_label" ) as Gtk.Label;
			notebook     = builder.get_object( "notebook" )    as Gtk.Notebook;

			title_image.set_from_icon_name( "glabels-object-properties", Gtk.IconSize.LARGE_TOOLBAR );
			title_image.set_sensitive( false );

			title_label.set_text( "<b>%s</b>".printf( _("Object properties") ) );
			title_label.set_use_markup( true );
			title_label.set_sensitive( false );

			/* Notebook pages. */
			line_fill_page_box = builder.get_object( "line_fill_page_box" ) as Gtk.Box;
			pos_size_page_box  = builder.get_object( "pos_size_page_box" )  as Gtk.Box;
			shadow_page_box    = builder.get_object( "shadow_page_box" )    as Gtk.Box;


			/* Line widgets. */
			line_width_spin         = builder.get_object( "line_width_spin" )         as Gtk.SpinButton;
			line_color_box          = builder.get_object( "line_color_box" )          as Gtk.Box;

			line_color_button = new ColorButton( _("No line"), Color.none(), Color.black() );
			line_color_box.pack_start( line_color_button, true, true, 0 );

			line_width_spin.value_changed.connect( on_line_width_spin_changed );
			line_color_button.color_changed.connect( on_line_color_button_changed );


			/* Fill widgets. */
			fill_color_box          = builder.get_object( "fill_color_box" )          as Gtk.Box;

			fill_color_button       = new ColorButton( _("No fill"), Color.none(), Color.black() );
			fill_color_box.pack_start( fill_color_button, true, true, 0 );

			fill_color_button.color_changed.connect( on_fill_color_button_changed );


			/* Position widgets. */
			pos_x_spin              = builder.get_object( "pos_x_spin" )              as Gtk.SpinButton;
			pos_y_spin              = builder.get_object( "pos_y_spin" )              as Gtk.SpinButton;
			pos_x_units_label       = builder.get_object( "pos_x_units_label" )       as Gtk.Label;
			pos_y_units_label       = builder.get_object( "pos_y_units_label" )       as Gtk.Label;

			pos_x_spin.value_changed.connect( on_pos_x_spin_changed );
			pos_y_spin.value_changed.connect( on_pos_y_spin_changed );


			/* Size widgets. */
			size_w_spin             = builder.get_object( "size_w_spin" )             as Gtk.SpinButton;
			size_h_spin             = builder.get_object( "size_h_spin" )             as Gtk.SpinButton;
			size_w_units_label      = builder.get_object( "size_w_units_label" )      as Gtk.Label;
			size_h_units_label      = builder.get_object( "size_h_units_label" )      as Gtk.Label;
			size_aspect_check       = builder.get_object( "size_aspect_check" )       as Gtk.CheckButton;
			size_reset_image_button = builder.get_object( "size_reset_image_button" ) as Gtk.Button;

			size_w_spin.value_changed.connect( on_size_w_spin_changed );
			size_h_spin.value_changed.connect( on_size_h_spin_changed );


			/* Shadow widgets. */
			shadow_enable_check     = builder.get_object( "shadow_enable_check" )     as Gtk.CheckButton;
			shadow_controls_grid    = builder.get_object( "shadow_controls_grid" )    as Gtk.Grid;
			shadow_x_spin           = builder.get_object( "shadow_x_spin" )           as Gtk.SpinButton;
			shadow_y_spin           = builder.get_object( "shadow_y_spin" )           as Gtk.SpinButton;
			shadow_x_units_label    = builder.get_object( "shadow_x_units_label" )    as Gtk.Label;
			shadow_y_units_label    = builder.get_object( "shadow_y_units_label" )    as Gtk.Label;
			shadow_color_box        = builder.get_object( "shadow_color_box" )        as Gtk.Box;
			shadow_opacity_spin     = builder.get_object( "shadow_opacity_spin" )     as Gtk.SpinButton;

			shadow_color_button = new ColorButton( _("Default"), Color.black(), Color.black() );
			shadow_color_box.pack_start(shadow_color_button, true, true, 0 );

			shadow_enable_check.toggled.connect( on_shadow_enable_check_changed );
			shadow_x_spin.value_changed.connect( on_shadow_x_spin_changed );
			shadow_y_spin.value_changed.connect( on_shadow_y_spin_changed );
			shadow_color_button.color_changed.connect( on_shadow_color_button_changed );
			shadow_opacity_spin.value_changed.connect( on_shadow_opacity_spin_changed );


			notebook.hide();
			notebook.set_no_show_all( true );

			prefs = new Prefs();
			on_prefs_changed();
			prefs.changed.connect( on_prefs_changed );
		}


		public void set_label( Label label )
		{
			this.label = label;

			on_label_size_changed();
			on_merge_changed();
			on_selection_changed();

			label.size_changed.connect( on_label_size_changed );
			label.merge_changed.connect( on_merge_changed );
			label.selection_changed.connect( on_selection_changed );
		}


		private void on_prefs_changed()
		{
			units = prefs.units;

			pos_x_units_label.set_text( units.name );
			pos_y_units_label.set_text( units.name );
			size_w_units_label.set_text( units.name );
			size_h_units_label.set_text( units.name );
			shadow_x_units_label.set_text( units.name );
			shadow_y_units_label.set_text( units.name );

			int    precision = UnitsUtil.get_precision( units );
			pos_x_spin.set_digits( precision );
			pos_y_spin.set_digits( precision );
			size_w_spin.set_digits( precision );
			size_h_spin.set_digits( precision );
			shadow_x_spin.set_digits( precision );
			shadow_y_spin.set_digits( precision );

			double step_size = UnitsUtil.get_step_size( units );
			pos_x_spin.set_increments( step_size, 10*step_size );
			pos_y_spin.set_increments( step_size, 10*step_size );
			size_w_spin.set_increments( step_size, 10*step_size );
			size_h_spin.set_increments( step_size, 10*step_size );
			shadow_x_spin.set_increments( step_size, 10*step_size );
			shadow_y_spin.set_increments( step_size, 10*step_size );

			double wh_max = double.max( w_max*units.units_per_point, h_max*units.units_per_point );
			pos_x_spin.set_range( 0, 2*wh_max );
			pos_y_spin.set_range( 0, 2*wh_max );
			size_w_spin.set_range( 0, 2*wh_max );
			size_h_spin.set_range( 0, 2*wh_max );
			shadow_x_spin.set_range( 0, 2*wh_max );
			shadow_y_spin.set_range( 0, 2*wh_max );

			pos_x_spin.set_value( x * units.units_per_point );
			pos_y_spin.set_value( y * units.units_per_point );
			size_w_spin.set_value( w * units.units_per_point );
			size_h_spin.set_value( h * units.units_per_point );
			shadow_x_spin.set_value( shadow_x * units.units_per_point );
			shadow_y_spin.set_value( shadow_y * units.units_per_point );

		}


		private void on_selection_changed()
		{
			if ( label.is_selection_atomic() )
			{
				object = label.get_1st_selected_object();

				if ( object is LabelObjectBox  )
				{
					title_image.set_from_icon_name( "glabels-box", Gtk.IconSize.LARGE_TOOLBAR );
					title_label.set_text( "<b>%s</b>".printf( _("Box object properties") ) );

					line_fill_page_box.show_all();
					pos_size_page_box.show_all();
					shadow_page_box.show_all();

					size_reset_image_button.hide();
				}
				else
				{
					assert_not_reached();
				}

				load_line_width_spin();
				load_line_color_button();
				load_fill_color_button();
				load_pos_x_spin();
				load_pos_y_spin();
				load_size_w_spin();
				load_size_h_spin();
				load_shadow_enable_check();
				load_shadow_x_spin();
				load_shadow_y_spin();
				load_shadow_color_button();
				load_shadow_opacity_spin();

				title_image.set_sensitive( true );
				title_label.set_use_markup( true );
				title_label.set_sensitive( true );
					
				notebook.show();
			}
			else
			{
				object = null;

				title_image.set_from_icon_name( "glabels-object-properties", Gtk.IconSize.LARGE_TOOLBAR );
				title_image.set_sensitive( false );

				title_label.set_text( "<b>%s</b>".printf( _("Object properties") ) );
				title_label.set_use_markup( true );
				title_label.set_sensitive( false );

				notebook.hide();
			}
		}


		private void on_label_size_changed()
		{
			label.get_size( out w_max, out h_max );

			double wh_max = double.max( w_max*units.units_per_point, h_max*units.units_per_point );

			pos_x_spin.set_range( 0, 2*wh_max );
			pos_y_spin.set_range( 0, 2*wh_max );
			size_w_spin.set_range( 0, 2*wh_max );
			size_h_spin.set_range( 0, 2*wh_max );
			shadow_x_spin.set_range( 0, 2*wh_max );
			shadow_y_spin.set_range( 0, 2*wh_max );
		}


		private void on_merge_changed()
		{
			if ( label.merge is MergeNone )
			{
				line_color_button.clear_keys();
				fill_color_button.clear_keys();
				shadow_color_button.clear_keys();
			}
			else
			{
				List<string> key_list = label.merge.get_key_list();
				line_color_button.set_keys( key_list );
				fill_color_button.set_keys( key_list );
				shadow_color_button.set_keys( key_list );
			}
		}


		private void on_line_width_spin_changed()
		{
			if ( object != null )
			{
				line_width = line_width_spin.get_value();
				object.line_width = line_width;
			}
		}


		private void load_line_width_spin()
		{
			GLib.SignalHandler.block_by_func( (void*)line_width_spin, (void*)on_line_width_spin_changed, this );

			line_width = object.line_width;
			line_width_spin.set_value( line_width );

			GLib.SignalHandler.unblock_by_func( (void*)line_width_spin, (void*)on_line_width_spin_changed, this );
		}


		private void on_line_color_button_changed()
		{
			if ( object != null )
			{
				bool is_default;

				line_color_node = line_color_button.get_color_node( out is_default );
				object.line_color_node = line_color_node;
			}
		}


		private void load_line_color_button()
		{
			GLib.SignalHandler.block_by_func( (void*)line_color_button, (void*)on_line_color_button_changed, this );

			line_color_node = object.line_color_node;
			line_color_button.set_color_node( line_color_node );

			GLib.SignalHandler.unblock_by_func( (void*)line_color_button, (void*)on_line_color_button_changed, this );
		}


		private void on_fill_color_button_changed()
		{
			if ( object != null )
			{
				bool is_default;

				fill_color_node   = fill_color_button.get_color_node( out is_default );
				object.fill_color_node = fill_color_node;
			}
		}


		private void load_fill_color_button()
		{
			GLib.SignalHandler.block_by_func( (void*)fill_color_button, (void*)on_fill_color_button_changed, this );

			fill_color_node = object.fill_color_node;
			fill_color_button.set_color_node( fill_color_node );

			GLib.SignalHandler.unblock_by_func( (void*)fill_color_button, (void*)on_fill_color_button_changed, this );
		}


		private void on_pos_x_spin_changed()
		{
			if ( object != null )
			{
				x = pos_x_spin.get_value() * units.points_per_unit;
				object.x0 = x;
			}
		}


		private void load_pos_x_spin()
		{
			GLib.SignalHandler.block_by_func( (void*)pos_x_spin, (void*)on_pos_x_spin_changed, this );

			x = object.x0;
			pos_x_spin.set_value( x * units.units_per_point );

			GLib.SignalHandler.unblock_by_func( (void*)pos_x_spin, (void*)on_pos_x_spin_changed, this );
		}


		private void on_pos_y_spin_changed()
		{
			if ( object != null )
			{
				y = pos_y_spin.get_value() * units.points_per_unit;
				object.y0 = y;
			}
		}


		private void load_pos_y_spin()
		{
			GLib.SignalHandler.block_by_func( (void*)pos_y_spin, (void*)on_pos_y_spin_changed, this );

			y = object.y0;
			pos_y_spin.set_value( y * units.units_per_point );

			GLib.SignalHandler.unblock_by_func( (void*)pos_y_spin, (void*)on_pos_y_spin_changed, this );
		}


		private void on_size_w_spin_changed()
		{
			if ( object != null )
			{
				w = size_w_spin.get_value() * units.points_per_unit;
				object.w = w;
			}
		}


		private void load_size_w_spin()
		{
			GLib.SignalHandler.block_by_func( (void*)size_w_spin, (void*)on_size_w_spin_changed, this );

			w = object.w;
			size_w_spin.set_value( w * units.units_per_point );

			GLib.SignalHandler.unblock_by_func( (void*)size_w_spin, (void*)on_size_w_spin_changed, this );
		}


		private void on_size_h_spin_changed()
		{
			if ( object != null )
			{
				h = size_h_spin.get_value() * units.points_per_unit;
				object.h = h;
			}
		}


		private void load_size_h_spin()
		{
			GLib.SignalHandler.block_by_func( (void*)size_h_spin, (void*)on_size_h_spin_changed, this );

			h = object.h;
			size_h_spin.set_value( h * units.units_per_point );

			GLib.SignalHandler.unblock_by_func( (void*)size_h_spin, (void*)on_size_h_spin_changed, this );
		}


		private void on_shadow_enable_check_changed()
		{
			if ( object != null )
			{
				shadow_enable     = shadow_enable_check.get_active();
				shadow_controls_grid.set_sensitive( shadow_enable );

				object.shadow_state = shadow_enable;
			}
		}


		private void load_shadow_enable_check()
		{
			GLib.SignalHandler.block_by_func( (void*)shadow_enable_check, (void*)on_shadow_enable_check_changed, this );

			shadow_enable = object.shadow_state;
			shadow_enable_check.set_active( shadow_enable );
			shadow_controls_grid.set_sensitive( shadow_enable );

			GLib.SignalHandler.unblock_by_func( (void*)shadow_enable_check, (void*)on_shadow_enable_check_changed, this );
		}


		private void on_shadow_x_spin_changed()
		{
			if ( object != null )
			{
				shadow_x  = shadow_x_spin.get_value() * units.points_per_unit;
				object.shadow_x = shadow_x;
			}
		}


		private void load_shadow_x_spin()
		{
			GLib.SignalHandler.block_by_func( (void*)shadow_x_spin, (void*)on_shadow_x_spin_changed, this );

			shadow_x = object.shadow_x;
			shadow_x_spin.set_value( shadow_x * units.units_per_point );

			GLib.SignalHandler.unblock_by_func( (void*)shadow_x_spin, (void*)on_shadow_x_spin_changed, this );
		}


		private void on_shadow_y_spin_changed()
		{
			if ( object != null )
			{
				shadow_y = shadow_y_spin.get_value() * units.points_per_unit;
				object.shadow_y = shadow_y;
			}
		}


		private void load_shadow_y_spin()
		{
			GLib.SignalHandler.block_by_func( (void*)shadow_y_spin, (void*)on_shadow_y_spin_changed, this );

			shadow_y = object.shadow_y;
			shadow_y_spin.set_value( shadow_y * units.units_per_point );

			GLib.SignalHandler.unblock_by_func( (void*)shadow_y_spin, (void*)on_shadow_y_spin_changed, this );
		}


		private void on_shadow_color_button_changed()
		{
			if ( object != null )
			{
				bool is_default;

				shadow_color_node = fill_color_button.get_color_node( out is_default );
				object.shadow_color_node = shadow_color_node;
			}
		}


		private void load_shadow_color_button()
		{
			GLib.SignalHandler.block_by_func( (void*)shadow_color_button, (void*)on_shadow_color_button_changed, this );

			shadow_color_node = object.shadow_color_node;
			shadow_color_button.set_color_node( shadow_color_node );

			GLib.SignalHandler.unblock_by_func( (void*)shadow_color_button, (void*)on_shadow_color_button_changed, this );
		}


		private void on_shadow_opacity_spin_changed()
		{
			if ( object != null )
			{
				shadow_opacity = shadow_opacity_spin.get_value();
				object.shadow_opacity = shadow_opacity;
			}
		}


		private void load_shadow_opacity_spin()
		{
			GLib.SignalHandler.block_by_func( (void*)shadow_opacity_spin, (void*)on_shadow_opacity_spin_changed, this );

			shadow_opacity = object.shadow_opacity;
			shadow_opacity_spin.set_value( shadow_opacity );

			GLib.SignalHandler.unblock_by_func( (void*)shadow_opacity_spin, (void*)on_shadow_opacity_spin_changed, this );
		}


	}

}

