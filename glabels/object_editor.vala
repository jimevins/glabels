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

namespace glabels
{

	class ObjectEditor : Gtk.Box
	{
		private Prefs prefs;

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
		/* TODO: aspect lock, and image reset */

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
			prefs = new Prefs();

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

			/* Line widgets. */
			line_width_spin = builder.get_object( "line_width_spin" ) as Gtk.SpinButton;
			line_color_box  = builder.get_object( "line_color_box" )  as Gtk.Box;
			line_color_button = new ColorButton( _("No line"), Color.none(), Color.black() );
			line_color_box.pack_start( line_color_button, true, true, 0 );

			/* Fill widgets. */
			fill_color_box  = builder.get_object( "fill_color_box" )  as Gtk.Box;
			fill_color_button = new ColorButton( _("No fill"), Color.none(), Color.black() );
			fill_color_box.pack_start( fill_color_button, true, true, 0 );

			/* Position widgets. */
			pos_x_spin        = builder.get_object( "pos_x_spin" )        as Gtk.SpinButton;
			pos_y_spin        = builder.get_object( "pos_y_spin" )        as Gtk.SpinButton;
			pos_x_units_label = builder.get_object( "pos_x_units_label" ) as Gtk.Label;
			pos_y_units_label = builder.get_object( "pos_y_units_label" ) as Gtk.Label;

			/* Size widgets. */
			size_w_spin        = builder.get_object( "size_w_spin" )        as Gtk.SpinButton;
			size_h_spin        = builder.get_object( "size_h_spin" )        as Gtk.SpinButton;
			size_w_units_label = builder.get_object( "size_w_units_label" ) as Gtk.Label;
			size_h_units_label = builder.get_object( "size_h_units_label" ) as Gtk.Label;

			/* Shadow widgets. */
			shadow_enable_check  = builder.get_object( "shadow_enable_check" )  as Gtk.CheckButton;
			shadow_controls_grid = builder.get_object( "shadow_controls_grid" ) as Gtk.Grid;
			shadow_x_spin        = builder.get_object( "shadow_x_spin" )        as Gtk.SpinButton;
			shadow_y_spin        = builder.get_object( "shadow_y_spin" )        as Gtk.SpinButton;
			shadow_x_units_label = builder.get_object( "shadow_x_units_label" ) as Gtk.Label;
			shadow_y_units_label = builder.get_object( "shadow_y_units_label" ) as Gtk.Label;
			shadow_color_box     = builder.get_object( "shadow_color_box" )  as Gtk.Box;
			shadow_opacity_spin  = builder.get_object( "shadow_opacity_spin" )  as Gtk.SpinButton;
			shadow_color_button  = new ColorButton( _("Default"), Color.black(), Color.black() );
			shadow_color_box.pack_start(shadow_color_button, true, true, 0 );

		}


	}

}

