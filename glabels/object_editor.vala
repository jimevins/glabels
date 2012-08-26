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
		private Prefs            prefs;
		private Units            units;

		private Model            model;
		private LabelObject?     object;


		/* Widgets */
		private Gtk.Image        title_image;
		private Gtk.Label        title_label;
		private Gtk.Notebook     notebook;

		private Gtk.Box          text_page_box;
		private Gtk.Box          image_page_box;
		private Gtk.Box          line_fill_page_box;
		private Gtk.Box          pos_size_page_box;
		private Gtk.Box          shadow_page_box;

		private Gtk.Box          text_font_family_box;
		private FontButton       text_font_family_button;
		private Gtk.SpinButton   text_font_size_spin;
		private Gtk.ToggleButton text_font_bold_toggle;
		private Gtk.ToggleButton text_font_italic_toggle;
		private Gtk.ToggleButton text_font_underline_toggle;
		private Gtk.Box          text_color_box;
		private ColorButton      text_color_button;
		private Gtk.ToggleButton text_halign_left_toggle;
		private Gtk.ToggleButton text_halign_center_toggle;
		private Gtk.ToggleButton text_halign_right_toggle;
		private Gtk.ToggleButton text_valign_top_toggle;
		private Gtk.ToggleButton text_valign_middle_toggle;
		private Gtk.ToggleButton text_valign_bottom_toggle;
		private Gtk.SpinButton   text_line_spacing_spin;
		private Gtk.TextView     text_textview;
		private Gtk.Box          text_insert_field_box;
		private FieldButton      text_insert_field_button;

		private Gtk.RadioButton  image_file_radio;
		private Gtk.RadioButton  image_key_radio;
		private Gtk.FileChooserButton   image_filebutton;
		private Gtk.Box          image_key_box;
		private FieldButton      image_key_button;

		private Gtk.SpinButton   line_width_spin;
		private Gtk.Box          line_color_box;
		private ColorButton      line_color_button;

		private Gtk.Frame        fill_frame;
		private Gtk.Box          fill_color_box;
		private ColorButton      fill_color_button;

		private Gtk.SpinButton   pos_x_spin;
		private Gtk.SpinButton   pos_y_spin;
		private Gtk.Label        pos_x_units_label;
		private Gtk.Label        pos_y_units_label;

		private Gtk.Frame        rect_size_frame;
		private Gtk.SpinButton   size_w_spin;
		private Gtk.SpinButton   size_h_spin;
		private Gtk.Label        size_w_units_label;
		private Gtk.Label        size_h_units_label;
		private Gtk.CheckButton  size_aspect_check;
		private Gtk.Button       size_reset_image_button;

		private Gtk.Frame        line_size_frame;
		private Gtk.SpinButton   line_length_spin;
		private Gtk.SpinButton   line_angle_spin;
		private Gtk.Label        line_length_units_label;

		private Gtk.CheckButton  shadow_enable_check;
		private Gtk.Grid         shadow_controls_grid;
		private Gtk.SpinButton   shadow_x_spin;
		private Gtk.SpinButton   shadow_y_spin;
		private Gtk.Label        shadow_x_units_label;
		private Gtk.Label        shadow_y_units_label;
		private Gtk.Box          shadow_color_box;
		private ColorButton      shadow_color_button;
		private Gtk.SpinButton   shadow_opacity_spin;


		/* Signal IDs */
		private ulong sigid_text_font_family_button_changed;
		private ulong sigid_text_font_size_spin_changed;
		private ulong sigid_text_font_bold_toggle_toggled;
		private ulong sigid_text_font_italic_toggle_toggled;
		private ulong sigid_text_font_underline_toggle_toggled;
		private ulong sigid_text_color_button_changed;
		private ulong sigid_text_halign_left_toggle_toggled;
		private ulong sigid_text_halign_center_toggle_toggled;
		private ulong sigid_text_halign_right_toggle_toggled;
		private ulong sigid_text_valign_top_toggle_toggled;
		private ulong sigid_text_valign_middle_toggle_toggled;
		private ulong sigid_text_valign_bottom_toggle_toggled;
		private ulong sigid_text_line_spacing_spin_changed;
		private ulong sigid_text_insert_field_button_key_selected;

		private ulong sigid_image_filebutton_selection_changed;
		private ulong sigid_image_key_button_changed;

		private ulong sigid_line_width_spin_changed;
		private ulong sigid_line_color_button_changed;

		private ulong sigid_fill_color_button_changed;

		private ulong sigid_pos_x_spin_changed;
		private ulong sigid_pos_y_spin_changed;

		private ulong sigid_size_w_spin_changed;
		private ulong sigid_size_h_spin_changed;

		private ulong sigid_line_length_spin_changed;
		private ulong sigid_line_angle_spin_changed;

		private ulong sigid_shadow_enable_check_changed;
		private ulong sigid_shadow_x_spin_changed;
		private ulong sigid_shadow_y_spin_changed;
		private ulong sigid_shadow_color_button_changed;
		private ulong sigid_shadow_opacity_spin_changed;



		public ObjectEditor()
		{
			/* Load the user interface. */
			Gtk.Builder builder = new Gtk.Builder();
			try
			{
				string file = GLib.Path.build_filename( Config.DATADIR, Config.GLABELS_BRANCH, "ui", "object_editor.ui" );
				string[] objects = { "object_editor_vbox",
				                     "font_size_adjustment", "line_spacing_adjustment",
				                     "line_width_adjustment",
				                     "size_w_adjustment", "size_h_adjustment",
				                     "line_length_adjustment", "line_angle_adjustment",
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
			text_page_box      = builder.get_object( "text_page_box" )      as Gtk.Box;
			image_page_box     = builder.get_object( "image_page_box" )      as Gtk.Box;
			line_fill_page_box = builder.get_object( "line_fill_page_box" ) as Gtk.Box;
			pos_size_page_box  = builder.get_object( "pos_size_page_box" )  as Gtk.Box;
			shadow_page_box    = builder.get_object( "shadow_page_box" )    as Gtk.Box;


			/* Text widgets. */
			text_font_family_box       = builder.get_object( "text_font_family_box" )       as Gtk.Box;
			text_font_size_spin        = builder.get_object( "text_font_size_spin" )        as Gtk.SpinButton;
			text_font_bold_toggle      = builder.get_object( "text_font_bold_toggle" )      as Gtk.ToggleButton;
			text_font_italic_toggle    = builder.get_object( "text_font_italic_toggle" )    as Gtk.ToggleButton;
			text_font_underline_toggle = builder.get_object( "text_font_underline_toggle" ) as Gtk.ToggleButton;
			text_color_box             = builder.get_object( "text_color_box" )             as Gtk.Box;
			text_halign_left_toggle    = builder.get_object( "text_halign_left_toggle" )    as Gtk.ToggleButton;
			text_halign_center_toggle  = builder.get_object( "text_halign_center_toggle" )  as Gtk.ToggleButton;
			text_halign_right_toggle   = builder.get_object( "text_halign_right_toggle" )   as Gtk.ToggleButton;
			text_valign_top_toggle     = builder.get_object( "text_valign_top_toggle" )     as Gtk.ToggleButton;
			text_valign_middle_toggle  = builder.get_object( "text_valign_middle_toggle" )  as Gtk.ToggleButton;
			text_valign_bottom_toggle  = builder.get_object( "text_valign_bottom_toggle" )  as Gtk.ToggleButton;
			text_line_spacing_spin     = builder.get_object( "text_line_spacing_spin" )     as Gtk.SpinButton;
			text_textview              = builder.get_object( "text_textview" )              as Gtk.TextView;
			text_insert_field_box      = builder.get_object( "text_insert_field_box" )      as Gtk.Box;

			text_font_family_button = new FontButton( null );
			text_font_family_box.pack_start( text_font_family_button, false, false, 0 );

			text_color_button = new ColorButton( _("Default"), Color.black(), Color.black() );
			text_color_box.pack_start( text_color_button, true, true, 0 );

			text_insert_field_button = new FieldButton( _("Insert merge field") );
			text_insert_field_box.pack_start( text_insert_field_button, true, true, 0 );

			sigid_text_font_family_button_changed =
				text_font_family_button.changed.connect( on_text_font_family_button_changed );
			sigid_text_font_size_spin_changed =
				text_font_size_spin.value_changed.connect( on_text_font_size_spin_changed );
			sigid_text_font_bold_toggle_toggled =
				text_font_bold_toggle.toggled.connect( on_text_font_bold_toggle_toggled );
			sigid_text_font_italic_toggle_toggled =
				text_font_italic_toggle.toggled.connect( on_text_font_italic_toggle_toggled );
			sigid_text_font_underline_toggle_toggled =
				text_font_underline_toggle.toggled.connect( on_text_font_underline_toggle_toggled );
			sigid_text_color_button_changed =
				text_color_button.color_changed.connect( on_text_color_button_changed );
			sigid_text_halign_left_toggle_toggled =
				text_halign_left_toggle.toggled.connect( on_text_halign_left_toggle_toggled );
			sigid_text_halign_center_toggle_toggled =
				text_halign_center_toggle.toggled.connect( on_text_halign_center_toggle_toggled );
			sigid_text_halign_right_toggle_toggled =
				text_halign_right_toggle.toggled.connect( on_text_halign_right_toggle_toggled );
			sigid_text_valign_top_toggle_toggled =
				text_valign_top_toggle.toggled.connect( on_text_valign_top_toggle_toggled );
			sigid_text_valign_middle_toggle_toggled =
				text_valign_middle_toggle.toggled.connect( on_text_valign_middle_toggle_toggled );
			sigid_text_valign_bottom_toggle_toggled =
				text_valign_bottom_toggle.toggled.connect( on_text_valign_bottom_toggle_toggled );
			sigid_text_line_spacing_spin_changed =
				text_line_spacing_spin.value_changed.connect( on_text_line_spacing_spin_changed );
			sigid_text_insert_field_button_key_selected =
				text_insert_field_button.key_selected.connect( on_text_insert_field_button_key_selected );


			/* Image widgets. */
			image_file_radio        = builder.get_object( "image_file_radio" )        as Gtk.RadioButton;
			image_key_radio         = builder.get_object( "image_key_radio" )         as Gtk.RadioButton;
			image_filebutton        = builder.get_object( "image_filebutton" )        as Gtk.FileChooserButton;
			image_key_box           = builder.get_object( "image_key_box" )           as Gtk.Box;

			image_key_button = new FieldButton( null );
			image_key_box.pack_start( image_key_button, true, true, 0 );

			sigid_image_filebutton_selection_changed =
				image_filebutton.selection_changed.connect( on_image_filebutton_selection_changed );
			sigid_image_key_button_changed = image_key_button.changed.connect( on_image_key_button_changed );


			/* Line widgets. */
			line_width_spin         = builder.get_object( "line_width_spin" )         as Gtk.SpinButton;
			line_color_box          = builder.get_object( "line_color_box" )          as Gtk.Box;

			line_color_button = new ColorButton( _("No line"), Color.none(), Color.black() );
			line_color_box.pack_start( line_color_button, true, true, 0 );

			sigid_line_width_spin_changed =
			    line_width_spin.value_changed.connect( on_line_width_spin_changed );
			sigid_line_color_button_changed =
			    line_color_button.color_changed.connect( on_line_color_button_changed );


			/* Fill widgets. */
			fill_frame              = builder.get_object( "fill_frame" )              as Gtk.Frame;
			fill_color_box          = builder.get_object( "fill_color_box" )          as Gtk.Box;

			fill_color_button       = new ColorButton( _("No fill"), Color.none(), Color.black() );
			fill_color_box.pack_start( fill_color_button, true, true, 0 );

			sigid_fill_color_button_changed =
			    fill_color_button.color_changed.connect( on_fill_color_button_changed );


			/* Position widgets. */
			pos_x_spin              = builder.get_object( "pos_x_spin" )              as Gtk.SpinButton;
			pos_y_spin              = builder.get_object( "pos_y_spin" )              as Gtk.SpinButton;
			pos_x_units_label       = builder.get_object( "pos_x_units_label" )       as Gtk.Label;
			pos_y_units_label       = builder.get_object( "pos_y_units_label" )       as Gtk.Label;

			sigid_pos_x_spin_changed = pos_x_spin.value_changed.connect( on_pos_x_spin_changed );
			sigid_pos_y_spin_changed = pos_y_spin.value_changed.connect( on_pos_y_spin_changed );


			/* Rectangle size widgets. */
			rect_size_frame         = builder.get_object( "rect_size_frame" )         as Gtk.Frame;
			size_w_spin             = builder.get_object( "size_w_spin" )             as Gtk.SpinButton;
			size_h_spin             = builder.get_object( "size_h_spin" )             as Gtk.SpinButton;
			size_w_units_label      = builder.get_object( "size_w_units_label" )      as Gtk.Label;
			size_h_units_label      = builder.get_object( "size_h_units_label" )      as Gtk.Label;
			size_aspect_check       = builder.get_object( "size_aspect_check" )       as Gtk.CheckButton;
			size_reset_image_button = builder.get_object( "size_reset_image_button" ) as Gtk.Button;

			sigid_size_w_spin_changed = size_w_spin.value_changed.connect( on_size_w_spin_changed );
			sigid_size_h_spin_changed = size_h_spin.value_changed.connect( on_size_h_spin_changed );


			/* Line size widgets. */
			line_size_frame         = builder.get_object( "line_size_frame" )         as Gtk.Frame;
			line_length_spin        = builder.get_object( "line_length_spin" )        as Gtk.SpinButton;
			line_angle_spin         = builder.get_object( "line_angle_spin" )         as Gtk.SpinButton;
			line_length_units_label = builder.get_object( "line_length_units_label" ) as Gtk.Label;

			sigid_line_length_spin_changed = line_length_spin.value_changed.connect( on_line_length_spin_changed );
			sigid_line_angle_spin_changed  = line_angle_spin.value_changed.connect( on_line_angle_spin_changed );


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

			sigid_shadow_enable_check_changed =
				shadow_enable_check.toggled.connect( on_shadow_enable_check_changed );
			sigid_shadow_x_spin_changed =
				shadow_x_spin.value_changed.connect( on_shadow_x_spin_changed );
			sigid_shadow_y_spin_changed =
				shadow_y_spin.value_changed.connect( on_shadow_y_spin_changed );
			sigid_shadow_color_button_changed =
				shadow_color_button.color_changed.connect( on_shadow_color_button_changed );
			sigid_shadow_opacity_spin_changed =
				shadow_opacity_spin.value_changed.connect( on_shadow_opacity_spin_changed );


			notebook.hide();
			notebook.set_no_show_all( true );

			prefs = new Prefs();
			on_prefs_changed();
			prefs.changed.connect( on_prefs_changed );
		}


		public void set_model( Model model )
		{
			this.model = model;

			on_label_size_changed();
			on_merge_changed();
			on_selection_changed();

			model.label.size_changed.connect( on_label_size_changed );
			model.label.merge_changed.connect( on_merge_changed );
			model.label.selection_changed.connect( on_selection_changed );
		}


		private void on_prefs_changed()
		{
			units = prefs.units;

			pos_x_units_label.set_text( units.name );
			pos_y_units_label.set_text( units.name );
			size_w_units_label.set_text( units.name );
			size_h_units_label.set_text( units.name );
			line_length_units_label.set_text( units.name );
			shadow_x_units_label.set_text( units.name );
			shadow_y_units_label.set_text( units.name );

			int    precision = UnitsUtil.get_precision( units );
			pos_x_spin.set_digits( precision );
			pos_y_spin.set_digits( precision );
			size_w_spin.set_digits( precision );
			size_h_spin.set_digits( precision );
			line_length_spin.set_digits( precision );
			shadow_x_spin.set_digits( precision );
			shadow_y_spin.set_digits( precision );

			double step_size = UnitsUtil.get_step_size( units );
			pos_x_spin.set_increments( step_size, 10*step_size );
			pos_y_spin.set_increments( step_size, 10*step_size );
			size_w_spin.set_increments( step_size, 10*step_size );
			size_h_spin.set_increments( step_size, 10*step_size );
			line_length_spin.set_increments( step_size, 10*step_size );
			shadow_x_spin.set_increments( step_size, 10*step_size );
			shadow_y_spin.set_increments( step_size, 10*step_size );

			on_label_size_changed();

			load_pos_x_spin();
			load_pos_y_spin();
			load_size_w_spin();
			load_size_h_spin();
			load_line_length_spin();
			load_shadow_x_spin();
			load_shadow_y_spin();
		}


		private void on_selection_changed()
		{
			if ( object != null )
			{
				object.moved.disconnect( on_object_moved );
				object.changed.disconnect( on_object_changed );
			}

			if ( model.label.is_selection_atomic() )
			{
				object = model.label.get_1st_selected_object();

				if ( object is LabelObjectText )
				{
					title_image.set_from_icon_name( "glabels-text", Gtk.IconSize.LARGE_TOOLBAR );
					title_label.set_text( "<b>%s</b>".printf( _("Text object properties") ) );

					text_page_box.show_all();
					image_page_box.hide();
					line_fill_page_box.hide();
					pos_size_page_box.show_all();
					shadow_page_box.show_all();

					line_size_frame.hide();
					size_reset_image_button.hide();
				}
				else if ( object is LabelObjectBox )
				{
					title_image.set_from_icon_name( "glabels-box", Gtk.IconSize.LARGE_TOOLBAR );
					title_label.set_text( "<b>%s</b>".printf( _("Box object properties") ) );

					text_page_box.hide();
					image_page_box.hide();
					line_fill_page_box.show_all();
					pos_size_page_box.show_all();
					shadow_page_box.show_all();

					line_size_frame.hide();
					size_reset_image_button.hide();
				}
				else if ( object is LabelObjectEllipse )
				{
					title_image.set_from_icon_name( "glabels-ellipse", Gtk.IconSize.LARGE_TOOLBAR );
					title_label.set_text( "<b>%s</b>".printf( _("Ellipse object properties") ) );

					text_page_box.hide();
					image_page_box.hide();
					line_fill_page_box.show_all();
					pos_size_page_box.show_all();
					shadow_page_box.show_all();

					line_size_frame.hide();
					size_reset_image_button.hide();
				}
				else if ( object is LabelObjectLine )
				{
					title_image.set_from_icon_name( "glabels-line", Gtk.IconSize.LARGE_TOOLBAR );
					title_label.set_text( "<b>%s</b>".printf( _("Line object properties") ) );

					text_page_box.hide();
					image_page_box.hide();
					line_fill_page_box.show_all();
					pos_size_page_box.show_all();
					shadow_page_box.show_all();

					fill_frame.hide();
					rect_size_frame.hide();
					size_reset_image_button.hide();
				}
				else if ( object is LabelObjectImage )
				{
					title_image.set_from_icon_name( "glabels-image", Gtk.IconSize.LARGE_TOOLBAR );
					title_label.set_text( "<b>%s</b>".printf( _("Image object properties") ) );

					text_page_box.hide();
					image_page_box.show_all();
					line_fill_page_box.hide();
					pos_size_page_box.show_all();
					shadow_page_box.show_all();

					line_size_frame.hide();
					size_reset_image_button.hide();
				}
				else
				{
					assert_not_reached();
				}

				load_text_font_family_button();
				load_text_font_size_spin();
				load_text_font_bold_toggle();
				load_text_font_italic_toggle();
				load_text_font_underline_toggle();
				load_text_color_button();
				load_text_halign_toggles();
				load_text_valign_toggles();
				load_text_line_spacing_spin();
				load_text_textview();
				load_image_filebutton();
				load_image_key_button();
				load_line_width_spin();
				load_line_color_button();
				load_fill_color_button();
				load_pos_x_spin();
				load_pos_y_spin();
				load_size_w_spin();
				load_size_h_spin();
				load_line_length_spin();
				load_line_angle_spin();
				load_shadow_enable_check();
				load_shadow_x_spin();
				load_shadow_y_spin();
				load_shadow_color_button();
				load_shadow_opacity_spin();

				title_image.set_sensitive( true );
				title_label.set_use_markup( true );
				title_label.set_sensitive( true );
					
				notebook.show();

				object.moved.connect( on_object_moved );
				object.changed.connect( on_object_changed );
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
			if ( model != null )
			{
				double w_max, h_max;

				model.label.get_size( out w_max, out h_max );

				double wh_max = double.max( w_max*units.units_per_point, h_max*units.units_per_point );

				pos_x_spin.set_range( -2*wh_max, 2*wh_max );
				pos_y_spin.set_range( -2*wh_max, 2*wh_max );
				size_w_spin.set_range( 0, 2*wh_max );
				size_h_spin.set_range( 0, 2*wh_max );
				shadow_x_spin.set_range( -wh_max, wh_max );
				shadow_y_spin.set_range( -wh_max, wh_max );
			}
		}


		private void on_merge_changed()
		{
			if ( model.label.merge is MergeNone )
			{
				text_color_button.clear_keys();
				text_insert_field_button.clear_keys();
				image_key_button.clear_keys();
				line_color_button.clear_keys();
				fill_color_button.clear_keys();
				shadow_color_button.clear_keys();

				image_file_radio.set_active( true );
				image_key_radio.set_sensitive( false );
			}
			else
			{
				List<string> key_list = model.label.merge.get_key_list();
				text_color_button.set_keys( key_list );
				text_insert_field_button.set_keys( key_list );
				image_key_button.set_keys( key_list );
				line_color_button.set_keys( key_list );
				fill_color_button.set_keys( key_list );
				shadow_color_button.set_keys( key_list );

				image_key_radio.set_sensitive( true );
			}
		}


		private void on_object_moved()
		{
			load_pos_x_spin();
			load_pos_y_spin();
		}


		private void on_object_changed()
		{
			if ( object is LabelObjectLine )
			{
				load_line_length_spin();
				load_line_angle_spin();
			}
			else
			{
				load_size_w_spin();
				load_size_h_spin();
			}
		}


		/***************************
		 * text_font_family_button
		 ***************************/
		private void on_text_font_family_button_changed()
		{
			if ( object != null )
			{
				object.font_family = text_font_family_button.get_family();
			}
		}


		private void load_text_font_family_button()
		{
			if ( (object != null) && object.can_text() )
			{
				GLib.SignalHandler.block( (void*)text_font_family_button, sigid_text_font_family_button_changed );

				text_font_family_button.set_family( object.font_family );

				GLib.SignalHandler.unblock( (void*)text_font_family_button, sigid_text_font_family_button_changed );
			}
		}


		/***************************
		 * text_font_size_spin
		 ***************************/
		private void on_text_font_size_spin_changed()
		{
			if ( object != null )
			{
				object.font_size = text_font_size_spin.get_value();
			}
		}


		private void load_text_font_size_spin()
		{
			if ( object != null )
			{
				GLib.SignalHandler.block( (void*)text_font_size_spin, sigid_text_font_size_spin_changed );

				text_font_size_spin.set_value( object.font_size );

				GLib.SignalHandler.unblock( (void*)text_font_size_spin, sigid_text_font_size_spin_changed );
			}
		}


		/***************************
		 * text_font_bold_toggle
		 ***************************/
		private void on_text_font_bold_toggle_toggled()
		{
			if ( object != null )
			{
				object.font_weight = text_font_bold_toggle.get_active() ? Pango.Weight.BOLD : Pango.Weight.NORMAL;
			}
		}


		private void load_text_font_bold_toggle()
		{
			if ( object != null )
			{
				GLib.SignalHandler.block( (void*)text_font_bold_toggle, sigid_text_font_bold_toggle_toggled );

				text_font_bold_toggle.set_active( object.font_weight == Pango.Weight.BOLD );

				GLib.SignalHandler.unblock( (void*)text_font_bold_toggle, sigid_text_font_bold_toggle_toggled );
			}
		}


		/***************************
		 * text_font_italic_toggle
		 ***************************/
		private void on_text_font_italic_toggle_toggled()
		{
			if ( object != null )
			{
				object.font_italic_flag = text_font_italic_toggle.get_active();
			}
		}


		private void load_text_font_italic_toggle()
		{
			if ( object != null )
			{
				GLib.SignalHandler.block( (void*)text_font_italic_toggle, sigid_text_font_italic_toggle_toggled );

				text_font_italic_toggle.set_active( object.font_italic_flag );

				GLib.SignalHandler.unblock( (void*)text_font_italic_toggle, sigid_text_font_italic_toggle_toggled );
			}
		}


		/******************************
		 * text_font_underline_toggle
		 ******************************/
		private void on_text_font_underline_toggle_toggled()
		{
			if ( object != null )
			{
				object.font_underline_flag = text_font_underline_toggle.get_active();
			}
		}


		private void load_text_font_underline_toggle()
		{
			if ( object != null )
			{
				GLib.SignalHandler.block( (void*)text_font_underline_toggle, sigid_text_font_underline_toggle_toggled );

				text_font_underline_toggle.set_active( object.font_underline_flag );

				GLib.SignalHandler.unblock( (void*)text_font_underline_toggle, sigid_text_font_underline_toggle_toggled );
			}
		}


		/******************************
		 * text_color_button
		 ******************************/
		private void on_text_color_button_changed()
		{
			if ( object != null )
			{
				bool is_default;

				object.text_color_node = text_color_button.get_color_node( out is_default );
			}
		}


		private void load_text_color_button()
		{
			if ( object != null )
			{
				GLib.SignalHandler.block( (void*)text_color_button, sigid_text_color_button_changed );

				text_color_button.set_color_node( object.text_color_node );

				GLib.SignalHandler.unblock( (void*)text_color_button, sigid_text_color_button_changed );
			}
		}


		/******************************
		 * text_halign_*_toggle
		 ******************************/
		private void on_text_halign_left_toggle_toggled()
		{
			if ( object != null )
			{
				if ( text_halign_left_toggle.get_active() )
				{
					object.text_alignment = Pango.Alignment.LEFT;

					text_halign_center_toggle.set_active( false );
					text_halign_right_toggle.set_active( false );
				}
			}
		}


		private void on_text_halign_center_toggle_toggled()
		{
			if ( object != null )
			{
				if ( text_halign_center_toggle.get_active() )
				{
					object.text_alignment = Pango.Alignment.CENTER;

					text_halign_left_toggle.set_active( false );
					text_halign_right_toggle.set_active( false );
				}
			}
		}


		private void on_text_halign_right_toggle_toggled()
		{
			if ( object != null )
			{
				if ( text_halign_right_toggle.get_active() )
				{
					object.text_alignment = Pango.Alignment.RIGHT;

					text_halign_left_toggle.set_active( false );
					text_halign_center_toggle.set_active( false );
				}
			}
		}


		private void load_text_halign_toggles()
		{
			if ( object != null )
			{
				GLib.SignalHandler.block( (void*)text_halign_left_toggle, sigid_text_halign_left_toggle_toggled );
				GLib.SignalHandler.block( (void*)text_halign_center_toggle, sigid_text_halign_center_toggle_toggled );
				GLib.SignalHandler.block( (void*)text_halign_right_toggle, sigid_text_halign_right_toggle_toggled );

				switch ( object.text_alignment )
				{
				case Pango.Alignment.LEFT:
					text_halign_left_toggle.set_active( true );
					text_halign_center_toggle.set_active( false );
					text_halign_right_toggle.set_active( false );
					break;
				case Pango.Alignment.CENTER:
					text_halign_left_toggle.set_active( false );
					text_halign_center_toggle.set_active( true );
					text_halign_right_toggle.set_active( false );
					break;
				case Pango.Alignment.RIGHT:
					text_halign_left_toggle.set_active( false );
					text_halign_center_toggle.set_active( false );
					text_halign_right_toggle.set_active( true );
					break;
				default:
					assert_not_reached();
				}

				GLib.SignalHandler.unblock( (void*)text_halign_left_toggle, sigid_text_halign_left_toggle_toggled );
				GLib.SignalHandler.unblock( (void*)text_halign_center_toggle, sigid_text_halign_center_toggle_toggled );
				GLib.SignalHandler.unblock( (void*)text_halign_right_toggle, sigid_text_halign_right_toggle_toggled );
			}
		}


		/******************************
		 * text_valign_*_toggle
		 ******************************/
		private void on_text_valign_top_toggle_toggled()
		{
			if ( object != null )
			{
				if ( text_valign_top_toggle.get_active() )
				{
					object.text_valignment = ValignType.TOP;

					text_valign_middle_toggle.set_active( false );
					text_valign_bottom_toggle.set_active( false );
				}
			}
		}


		private void on_text_valign_middle_toggle_toggled()
		{
			if ( object != null )
			{
				if ( text_valign_middle_toggle.get_active() )
				{
					object.text_valignment = ValignType.CENTER;

					text_valign_top_toggle.set_active( false );
					text_valign_bottom_toggle.set_active( false );
				}
			}
		}


		private void on_text_valign_bottom_toggle_toggled()
		{
			if ( object != null )
			{
				if ( text_valign_bottom_toggle.get_active() )
				{
					object.text_valignment = ValignType.BOTTOM;

					text_valign_top_toggle.set_active( false );
					text_valign_middle_toggle.set_active( false );
				}
			}
		}


		private void load_text_valign_toggles()
		{
			if ( object != null )
			{
				GLib.SignalHandler.block( (void*)text_valign_top_toggle, sigid_text_valign_top_toggle_toggled );
				GLib.SignalHandler.block( (void*)text_valign_middle_toggle, sigid_text_valign_middle_toggle_toggled );
				GLib.SignalHandler.block( (void*)text_valign_bottom_toggle, sigid_text_valign_bottom_toggle_toggled );

				switch ( object.text_valignment )
				{
				case ValignType.TOP:
					text_valign_top_toggle.set_active( true );
					text_valign_middle_toggle.set_active( false );
					text_valign_bottom_toggle.set_active( false );
					break;
				case ValignType.CENTER:
					text_valign_top_toggle.set_active( false );
					text_valign_middle_toggle.set_active( true );
					text_valign_bottom_toggle.set_active( false );
					break;
				case ValignType.BOTTOM:
					text_valign_top_toggle.set_active( false );
					text_valign_middle_toggle.set_active( false );
					text_valign_bottom_toggle.set_active( true );
					break;
				default:
					assert_not_reached();
				}

				GLib.SignalHandler.unblock( (void*)text_valign_top_toggle, sigid_text_valign_top_toggle_toggled );
				GLib.SignalHandler.unblock( (void*)text_valign_middle_toggle, sigid_text_valign_middle_toggle_toggled );
				GLib.SignalHandler.unblock( (void*)text_valign_bottom_toggle, sigid_text_valign_bottom_toggle_toggled );
			}
		}


		/***************************
		 * text_line_spacing_spin
		 ***************************/
		private void on_text_line_spacing_spin_changed()
		{
			if ( object != null )
			{
				object.text_line_spacing = text_line_spacing_spin.get_value();
			}
		}


		private void load_text_line_spacing_spin()
		{
			if ( object != null )
			{
				GLib.SignalHandler.block( (void*)text_line_spacing_spin, sigid_text_line_spacing_spin_changed );

				text_line_spacing_spin.set_value( object.text_line_spacing );

				GLib.SignalHandler.unblock( (void*)text_line_spacing_spin, sigid_text_line_spacing_spin_changed );
			}
		}


		/***************************
		 * text_textview
		 ***************************/
		private void load_text_textview()
		{
			if ( object != null )
			{
				if ( object is LabelObjectText  )
				{
					LabelObjectText tobject = object as LabelObjectText;

					text_textview.set_buffer( tobject.buffer );
				}
			}
		}


		/***************************
		 * text_insert_field_button
		 ***************************/
		private void on_text_insert_field_button_key_selected( string key )
		{
			if ( object != null )
			{
				if ( object is LabelObjectText  )
				{
					string field_string = "${%s}".printf( key );
					text_textview.get_buffer().insert_at_cursor( field_string, -1 );
				}
			}
		}


		/******************************
		 * image_filebutton
		 ******************************/
		private void on_image_filebutton_selection_changed()
		{
			if ( object != null )
			{
				string filename = image_filebutton.get_filename();
				if ( filename != null )
				{
					object.filename_node = new TextNode( false, filename );
				}
			}
		}

		private void load_image_filebutton()
		{
			if ( (object != null) && (object.filename_node != null) )
			{
				if ( !object.filename_node.field_flag )
				{
					image_file_radio.set_active( true );

					GLib.SignalHandler.block( (void*)image_filebutton, sigid_image_filebutton_selection_changed );

					if ( object.filename_node.data != null )
					{
						image_filebutton.set_filename( object.filename_node.data );
					}
					else
					{
						image_filebutton.unselect_all();
					}

					GLib.SignalHandler.unblock( (void*)image_filebutton, sigid_image_filebutton_selection_changed );
				}
			}
		}


		/******************************
		 * image_key_button
		 ******************************/
		private void on_image_key_button_changed()
		{
			if ( object != null )
			{
				object.filename_node = new TextNode( true, image_key_button.get_key() );
			}
		}

		private void load_image_key_button()
		{
			if ( (object != null) && (object.filename_node != null) )
			{
				if ( object.filename_node.field_flag )
				{
					image_key_radio.set_active( true );

					GLib.SignalHandler.block( (void*)image_key_button, sigid_image_key_button_changed );

					image_key_button.set_key( object.filename_node.data );

					GLib.SignalHandler.unblock( (void*)image_key_button, sigid_image_key_button_changed );
				}
			}
		}

		/******************************
		 * line_width_spin
		 ******************************/
		private void on_line_width_spin_changed()
		{
			if ( object != null )
			{
				object.line_width = line_width_spin.get_value();
			}
		}


		private void load_line_width_spin()
		{
			if ( object != null )
			{
				GLib.SignalHandler.block( (void*)line_width_spin, sigid_line_width_spin_changed );

				line_width_spin.set_value( object.line_width );

				GLib.SignalHandler.unblock( (void*)line_width_spin, sigid_line_width_spin_changed );
			}
		}


		/******************************
		 * line_color_button
		 ******************************/
		private void on_line_color_button_changed()
		{
			if ( object != null )
			{
				bool is_default;

				object.line_color_node = line_color_button.get_color_node( out is_default );
			}
		}


		private void load_line_color_button()
		{
			if ( object != null )
			{
				GLib.SignalHandler.block( (void*)line_color_button, sigid_line_color_button_changed );

				line_color_button.set_color_node( object.line_color_node );

				GLib.SignalHandler.unblock( (void*)line_color_button, sigid_line_color_button_changed );
			}
		}


		/******************************
		 * fill_color_button
		 ******************************/
		private void on_fill_color_button_changed()
		{
			if ( object != null )
			{
				bool is_default;

				object.fill_color_node = fill_color_button.get_color_node( out is_default );
			}
		}


		private void load_fill_color_button()
		{
			if ( object != null )
			{
				GLib.SignalHandler.block( (void*)fill_color_button, sigid_fill_color_button_changed );

				fill_color_button.set_color_node( object.fill_color_node );

				GLib.SignalHandler.unblock( (void*)fill_color_button, sigid_fill_color_button_changed );
			}
		}


		/******************************
		 * pos_x_spin
		 ******************************/
		private void on_pos_x_spin_changed()
		{
			if ( object != null )
			{
				object.x0 = pos_x_spin.get_value() * units.points_per_unit;
			}
		}


		private void load_pos_x_spin()
		{
			if ( object != null )
			{
				GLib.SignalHandler.block( (void*)pos_x_spin, sigid_pos_x_spin_changed );

				pos_x_spin.set_value( object.x0 * units.units_per_point );

				GLib.SignalHandler.unblock( (void*)pos_x_spin, sigid_pos_x_spin_changed );
			}
		}


		/******************************
		 * pos_y_spin
		 ******************************/
		private void on_pos_y_spin_changed()
		{
			if ( object != null )
			{
				object.y0 = pos_y_spin.get_value() * units.points_per_unit;
			}
		}


		private void load_pos_y_spin()
		{
			if ( object != null )
			{
				GLib.SignalHandler.block( (void*)pos_y_spin, sigid_pos_y_spin_changed );

				pos_y_spin.set_value( object.y0 * units.units_per_point );

				GLib.SignalHandler.unblock( (void*)pos_y_spin, sigid_pos_y_spin_changed );
			}
		}


		/******************************
		 * size_w_spin
		 ******************************/
		private void on_size_w_spin_changed()
		{
			if ( object != null )
			{
				double w = size_w_spin.get_value() * units.points_per_unit;

				if ( size_aspect_check.get_active() )
				{
					object.set_w_honor_aspect( w );
				}
				else
				{
					object.w = w;
				}
			}
		}


		private void load_size_w_spin()
		{
			if ( (object != null) && !(object is LabelObjectLine) )
			{
				GLib.SignalHandler.block( (void*)size_w_spin, sigid_size_w_spin_changed );

				size_w_spin.set_value( object.w * units.units_per_point );

				GLib.SignalHandler.unblock( (void*)size_w_spin, sigid_size_w_spin_changed );
			}
		}


		/******************************
		 * size_h_spin
		 ******************************/
		private void on_size_h_spin_changed()
		{
			if ( object != null )
			{
				double h = size_h_spin.get_value() * units.points_per_unit;

				if ( size_aspect_check.get_active() )
				{
					object.set_h_honor_aspect( h );
				}
				else
				{
					object.h = h;
				}
			}
		}


		private void load_size_h_spin()
		{
			if ( (object != null) && !(object is LabelObjectLine) )
			{
				GLib.SignalHandler.block( (void*)size_h_spin, sigid_size_h_spin_changed );

				size_h_spin.set_value( object.h * units.units_per_point );

				GLib.SignalHandler.unblock( (void*)size_h_spin, sigid_size_h_spin_changed );
			}
		}


		/******************************
		 * line_length_spin
		 ******************************/
		private void on_line_length_spin_changed()
		{
			if ( object != null )
			{
				double length = line_length_spin.get_value() * units.points_per_unit;
				double angle  = line_angle_spin.get_value() * Math.PI/180;

				object.w = length * Math.cos( -angle );
				object.h = length * Math.sin( -angle );
			}
		}


		private void load_line_length_spin()
		{
			if ( (object != null) && object is LabelObjectLine )
			{
				GLib.SignalHandler.block( (void*)line_length_spin, sigid_line_length_spin_changed );

				double length = Math.sqrt( object.w*object.w + object.h*object.h );

				line_length_spin.set_value( length * units.units_per_point );

				GLib.SignalHandler.unblock( (void*)line_length_spin, sigid_line_length_spin_changed );
			}
		}


		/******************************
		 * line_angle_spin
		 ******************************/
		private void on_line_angle_spin_changed()
		{
			if ( object != null )
			{
				double length = line_length_spin.get_value() * units.points_per_unit;
				double angle  = line_angle_spin.get_value() * Math.PI/180;

				object.w = length * Math.cos( angle );
				object.h = length * Math.sin( angle );
			}
		}


		private void load_line_angle_spin()
		{
			if ( (object != null) && object is LabelObjectLine )
			{
				GLib.SignalHandler.block( (void*)line_angle_spin, sigid_line_angle_spin_changed );

				double angle = Math.atan2( object.h, object.w );

				line_angle_spin.set_value( angle * 180/Math.PI );

				GLib.SignalHandler.unblock( (void*)line_angle_spin, sigid_line_angle_spin_changed );
			}
		}


		/******************************
		 * shadow_enable_check
		 ******************************/
		private void on_shadow_enable_check_changed()
		{
			if ( object != null )
			{
				object.shadow_state = shadow_enable_check.get_active();
				shadow_controls_grid.set_sensitive( object.shadow_state );
			}
		}


		private void load_shadow_enable_check()
		{
			if ( object != null )
			{
				GLib.SignalHandler.block( (void*)shadow_enable_check, sigid_shadow_enable_check_changed );

				shadow_enable_check.set_active( object.shadow_state );
				shadow_controls_grid.set_sensitive( object.shadow_state );

				GLib.SignalHandler.unblock( (void*)shadow_enable_check, sigid_shadow_enable_check_changed );
			}
		}


		/******************************
		 * shadow_x_spin
		 ******************************/
		private void on_shadow_x_spin_changed()
		{
			if ( object != null )
			{
				object.shadow_x  = shadow_x_spin.get_value() * units.points_per_unit;
			}
		}


		private void load_shadow_x_spin()
		{
			if ( object != null )
			{
				GLib.SignalHandler.block( (void*)shadow_x_spin, sigid_shadow_x_spin_changed );

				shadow_x_spin.set_value( object.shadow_x * units.units_per_point );

				GLib.SignalHandler.unblock( (void*)shadow_x_spin, sigid_shadow_x_spin_changed );
			}
		}


		/******************************
		 * shadow_y_spin
		 ******************************/
		private void on_shadow_y_spin_changed()
		{
			if ( object != null )
			{
				object.shadow_y = shadow_y_spin.get_value() * units.points_per_unit;
			}
		}


		private void load_shadow_y_spin()
		{
			if ( object != null )
			{
				GLib.SignalHandler.block( (void*)shadow_y_spin, sigid_shadow_y_spin_changed );

				shadow_y_spin.set_value( object.shadow_y * units.units_per_point );

				GLib.SignalHandler.unblock( (void*)shadow_y_spin, sigid_shadow_y_spin_changed );
			}
		}


		/******************************
		 * shadow_color_button
		 ******************************/
		private void on_shadow_color_button_changed()
		{
			if ( object != null )
			{
				bool is_default;

				object.shadow_color_node = shadow_color_button.get_color_node( out is_default );
			}
		}


		private void load_shadow_color_button()
		{
			if ( object != null )
			{
				GLib.SignalHandler.block( (void*)shadow_color_button, sigid_shadow_color_button_changed );

				shadow_color_button.set_color_node( object.shadow_color_node );

				GLib.SignalHandler.unblock( (void*)shadow_color_button, sigid_shadow_color_button_changed );
			}
		}


		/******************************
		 * shadow_opacity_spin
		 ******************************/
		private void on_shadow_opacity_spin_changed()
		{
			if ( object != null )
			{
				object.shadow_opacity = shadow_opacity_spin.get_value() / 100;
			}
		}


		private void load_shadow_opacity_spin()
		{
			if ( object != null )
			{
				GLib.SignalHandler.block( (void*)shadow_opacity_spin, sigid_shadow_opacity_spin_changed );

				shadow_opacity_spin.set_value( object.shadow_opacity * 100 );

				GLib.SignalHandler.unblock( (void*)shadow_opacity_spin, sigid_shadow_opacity_spin_changed );
			}
		}


	}

}

