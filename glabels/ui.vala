/*  ui.vala
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

	public class Ui : Gtk.UIManager
	{
		private weak Window window;
		private Prefs  prefs;

		private const Gtk.ActionEntry[] entries = {

			/* Menu entries. */
			{ "FileMenu",                null, N_("_File") },
			{ "EditMenu",                null, N_("_Edit") },
			{ "ViewMenu",                null, N_("_View") },
			{ "ViewMainToolBarMenu",     null, N_("Customize Main Toolbar") },
			{ "ViewDrawingToolBarMenu",  null, N_("Customize Drawing Toolbar") },
			{ "ViewPropertyToolBarMenu", null, N_("Customize Properties Toolbar") },
			{ "ObjectsMenu",             null, N_("_Objects") },
			{ "ObjectsCreateMenu",       null, N_("_Create") },
			{ "ObjectsOrderMenu",        null, N_("_Order") },
			{ "ObjectsRotateFlipMenu",   null, N_("_Rotate/Flip") },
			{ "ObjectsAlignMenu",        null, N_("_Alignment") },
			{ "ObjectsCenterMenu",       null, N_("C_enter") },
			{ "HelpMenu",                null, N_("_Help") },

			/* Popup entries. */
			{ "ContextMenu", null, N_("Context Menu") },
			{ "EmptySelectionContextMenu", null, N_("Context Menu") },

			/* File action entries. */
			{ "FileNew",
			  "gtk-new",
			  N_("_New"),
			  "<control>N",
			  N_("Create a new file"),
			  on_file_new },

			{ "FileOpen",
			  "gtk-open",
			  N_("_Open..."),
			  "<control>O",
			  N_("Open a file"),
			  on_file_open },

			{ "FileSave",
			  "gtk-save",
			  N_("_Save"),
			  "<control>S",
			  N_("Save current file"),
			  on_file_save },

			{ "FileSaveAs",
			  "gtk-save",
			  N_("Save _As..."),
			  "<shift><control>S",
			  N_("Save the current file to a different name"),
			  on_file_save_as },

			{ "FilePrint",
			  "gtk-print",
			  N_("_Print..."),
			  "<control>P",
			  N_("Print the current file"),
			  on_file_print },

			{ "FileProperties",
			  "gtk-properties",
			  N_("Properties..."),
			  null,
			  N_("Modify document properties"),
			  on_file_properties },

			{ "FileTemplateDesigner",
			  null,
			  N_("Template _Designer..."),
			  null,
			  N_("Create a custom template"),
			  on_file_template_designer },

			{ "FileClose",
			  "gtk-close",
			  N_("_Close"),
			  "<alt>F4",
			  N_("Close the current file"),
			  on_file_close },

			{ "FileQuit",
			  "gtk-quit",
			  N_("_Quit"),
			  "<control>Q",
			  N_("Quit the program"),
			  on_file_quit },


			/* Edit action entries. */
			{ "EditUndo",
			  "gtk-undo",
			  N_("Undo"),
			  "<control>Z",
			  N_("Undo"),
			  on_edit_undo },

			{ "EditRedo",
			  "gtk-redo",
			  N_("Redo"),
			  "<shift><control>Z",
			  N_("Redo"),
			  on_edit_redo },

			{ "EditCut",
			  "gtk-cut",
			  N_("Cut"),
			  "<control>X",
			  N_("Cut the selection"),
			  on_edit_cut },

			{ "EditCopy",
			  "gtk-copy",
			  N_("Copy"),
			  "<control>C",
			  N_("Copy the selection"),
			  on_edit_copy },

			{ "EditPaste",
			  "gtk-paste",
			  N_("Paste"),
			  "<control>V",
			  N_("Paste the clipboard"),
			  on_edit_paste },

			{ "EditDelete",
			  null,
			  N_("Delete"),
			  null,
			  N_("Delete the selected objects"),
			  on_edit_delete },

			{ "EditSelectAll",
			  null,
			  N_("Select All"),
			  "<control>A",
			  N_("Select all objects"),
			  on_edit_select_all },

			{ "EditUnSelectAll",
			  null,
			  N_("Un-select All"),
			  null,
			  N_("Remove all selections"),
			  on_edit_unselect_all },

			{ "EditPreferences",
			  "gtk-preferences",
			  N_("Preferences"),
			  null,
			  N_("Configure the application"),
			  on_edit_preferences },


			/* View action entries. */
			{ "ViewZoomIn",
			  "gtk-zoom-in",
			  N_("Zoom in"),
			  null,
			  N_("Increase magnification"),
			  on_view_zoomin },

			{ "ViewZoomOut",
			  "gtk-zoom-out",
			  N_("Zoom out"),
			  null,
			  N_("Decrease magnification"),
			  on_view_zoomout },

			{ "ViewZoom1to1",
			  "gtk-zoom-100",
			  N_("Zoom 1 to 1"),
			  null,
			  N_("Restore scale to 100%"),
			  on_view_zoom1to1 },

			{ "ViewZoomToFit",
			  "gtk-zoom-fit",
			  N_("Zoom to fit"),
			  null,
			  N_("Set scale to fit window"),
			  on_view_zoom_to_fit },


			/* Objects action entries. */
			{ "ObjectsArrowMode",
			  "glabels-arrow",
			  N_("Select Mode"),
			  null,
			  N_("Select, move and modify objects"),
			  on_objects_arrow_mode },

			{ "ObjectsCreateText",
			  "glabels-text",
			  N_("Text"),
			  null,
			  N_("Create text object"),
			  on_objects_create_text },

			{ "ObjectsCreateBox",
			  "glabels-box",
			  N_("Box"),
			  null,
			  N_("Create box/rectangle object"),
			  on_objects_create_box },

			{ "ObjectsCreateLine",
			  "glabels-line",
			  N_("Line"),
			  null,
			  N_("Create line object"),
			  on_objects_create_line },

			{ "ObjectsCreateEllipse",
			  "glabels-ellipse",
			  N_("Ellipse"),
			  null,
			  N_("Create ellipse/circle object"),
			  on_objects_create_ellipse },

			{ "ObjectsCreateImage",
			  "glabels-image",
			  N_("Image"),
			  null,
			  N_("Create image object"),
			  on_objects_create_image },

			{ "ObjectsCreateBarcode",
			  "glabels-barcode",
			  N_("Barcode"),
			  null,
			  N_("Create barcode object"),
			  on_objects_create_barcode },
	
			{ "ObjectsRaise",
			  "glabels-order-top",
			  N_("Bring to front"),
			  null,
			  N_("Raise object to top"),
			  on_objects_raise },

			{ "ObjectsLower",
			  "glabels-order-bottom",
			  N_("Send to back"),
			  null,
			  N_("Lower object to bottom"),
			  on_objects_lower },

			{ "ObjectsRotateLeft",
			  "glabels-rotate-left",
			  N_("Rotate left"),
			  null,
			  N_("Rotate object 90 degrees counter-clockwise"),
			  on_objects_rotate_left },

			{ "ObjectsRotateRight",
			  "glabels-rotate-right",
			  N_("Rotate right"),
			  null,
			  N_("Rotate object 90 degrees clockwise"),
			  on_objects_rotate_right },

			{ "ObjectsFlipHorizontal",
			  "glabels-flip-horiz",
			  N_("Flip horizontally"),
			  null,
			  N_("Flip object horizontally"),
			  on_objects_flip_horiz },

			{ "ObjectsFlipVertical",
			  "glabels-flip-vert",
			  N_("Flip vertically"),
			  null,
			  N_("Flip object vertically"),
			  on_objects_flip_vert },

			{ "ObjectsAlignLeft",
			  "glabels-align-left",
			  N_("Align left"),
			  null,
			  N_("Align objects to left edges"),
			  on_objects_align_left },

			{ "ObjectsAlignHCenter",
			  "glabels-align-hcenter",
			  N_("Align center"),
			  null,
			  N_("Align objects to horizontal centers"),
			  on_objects_align_hcenter },

			{ "ObjectsAlignRight",
			  "glabels-align-right",
			  N_("Align right"),
			  null,
			  N_("Align objects to right edges"),
			  on_objects_align_right },

			{ "ObjectsAlignTop",
			  "glabels-align-top",
			  N_("Align top"),
			  null,
			  N_("Align objects to top edges"),
			  on_objects_align_top },

			{ "ObjectsAlignVCenter",
			  "glabels-align-vcenter",
			  N_("Align middle"),
			  null,
			  N_("Align objects to vertical centers"),
			  on_objects_align_vcenter },

			{ "ObjectsAlignBottom",
			  "glabels-align-bottom",
			  N_("Align bottom"),
			  null,
			  N_("Align objects to bottom edges"),
			  on_objects_align_bottom },

			{ "ObjectsCenterHorizontal",
			  "glabels-center-horiz",
			  N_("Center horizontally"),
			  null,
			  N_("Center objects to horizontal label center"),
			  on_objects_center_horiz },

			{ "ObjectsCenterVertical",
			  "glabels-center-vert",
			  N_("Center vertically"),
			  null,
			  N_("Center objects to vertical label center"),
			  on_objects_center_vert },

			{ "ObjectsMergeProperties",
			  "glabels-merge",
			  N_("Merge properties"),
			  null,
			  N_("Edit merge properties"),
			  on_objects_merge_properties },


			/* Help actions entries. */
			{ "HelpContents",
			  "gtk-help",
			  N_("Contents"),
			  "F1",
			  N_("Open glabels manual"),
			  on_help_contents },

			{ "HelpAbout",
			  "gtk-about",
			  N_("About..."),
			  null,
			  N_("About glabels"),
			  on_help_about }

		};


		private const Gtk.ToggleActionEntry[] toggle_entries = {

			{ "ViewMainToolBar",
			  null,
			  N_("Main toolbar"),
			  null,
			  N_("Change the visibility of the main toolbar in the current window"),
			  on_view_main_toolbar_toggled,
			  true },

			{ "ViewDrawingToolBar",
			  null,
			  N_("Drawing toolbar"),
			  null,
			  N_("Change the visibility of the drawing toolbar in the current window"),
			  on_view_drawing_toolbar_toggled,
			  true },

			{ "ViewPropertyToolBar",
			  null,
			  N_("Property toolbar"),
			  null,
			  N_("Change the visibility of the property toolbar in the current window"),
			  on_view_property_bar_toggled,
			  true },

			{ "ViewGrid",
			  null,
			  N_("Grid"),
			  null,
			  N_("Change the visibility of the grid in the current window"),
			  on_view_grid_toggled,
			  true },

			{ "ViewMarkup",
			  null,
			  N_("Markup"),
			  null,
			  N_("Change the visibility of markup lines in the current window"),
			  on_view_markup_toggled,
			  true }

		};


		private static const string ui_info = """
			<ui>
			
				<menubar name='MenuBar'>
					<menu action='FileMenu'>
						<menuitem action='FileNew' />
						<menuitem action='FileOpen' />
						<separator />
						<menuitem action='FileSave' />
						<menuitem action='FileSaveAs' />
						<separator />
						<menuitem action='FilePrint' />
						<separator />
						<menuitem action='FileProperties' />
						<menuitem action='FileTemplateDesigner' />
						<separator />
						<menuitem action='FileClose' />
						<menuitem action='FileQuit' />
					</menu>
					<menu action='EditMenu'>
						<menuitem action='EditUndo' />
						<menuitem action='EditRedo' />
						<separator />
						<menuitem action='EditCut' />
						<menuitem action='EditCopy' />
						<menuitem action='EditPaste' />
						<menuitem action='EditDelete' />
						<separator />
						<menuitem action='EditSelectAll' />
						<menuitem action='EditUnSelectAll' />
						<separator />
						<menuitem action='EditPreferences' />
					</menu>
					<menu action='ViewMenu'>
						<menuitem action='ViewMainToolBar' />
						<menuitem action='ViewDrawingToolBar' />
						<menuitem action='ViewPropertyToolBar' />
						<separator />
						<menuitem action='ViewGrid' />
						<menuitem action='ViewMarkup' />
						<separator />
						<menuitem action='ViewZoomIn' />
						<menuitem action='ViewZoomOut' />
						<menuitem action='ViewZoom1to1' />
						<menuitem action='ViewZoomToFit' />
					</menu>
					<menu action='ObjectsMenu'>
						<menuitem action='ObjectsArrowMode' always-show-image='true' />
						<menu action='ObjectsCreateMenu'>
							<menuitem action='ObjectsCreateText' always-show-image='true' />
							<menuitem action='ObjectsCreateBox' always-show-image='true' />
							<menuitem action='ObjectsCreateLine' always-show-image='true' />
							<menuitem action='ObjectsCreateEllipse' always-show-image='true' />
							<menuitem action='ObjectsCreateImage' always-show-image='true' />
							<menuitem action='ObjectsCreateBarcode' always-show-image='true' />
						</menu>
						<separator />
						<menu action='ObjectsOrderMenu'>
							<menuitem action='ObjectsRaise' always-show-image='true' />
							<menuitem action='ObjectsLower' always-show-image='true' />
						</menu>
						<menu action='ObjectsRotateFlipMenu'>
							<menuitem action='ObjectsRotateLeft' always-show-image='true' />
							<menuitem action='ObjectsRotateRight' always-show-image='true' />
							<menuitem action='ObjectsFlipHorizontal' always-show-image='true' />
							<menuitem action='ObjectsFlipVertical' always-show-image='true' />
						</menu>
						<menu action='ObjectsAlignMenu'>
							<menuitem action='ObjectsAlignLeft' always-show-image='true' />
							<menuitem action='ObjectsAlignHCenter' always-show-image='true' />
							<menuitem action='ObjectsAlignRight' always-show-image='true' />
						        <separator />
							<menuitem action='ObjectsAlignTop' always-show-image='true' />
							<menuitem action='ObjectsAlignVCenter' always-show-image='true' />
							<menuitem action='ObjectsAlignBottom' always-show-image='true' />
						</menu>
						<menu action='ObjectsCenterMenu'>
							<menuitem action='ObjectsCenterHorizontal' always-show-image='true' />
							<menuitem action='ObjectsCenterVertical' always-show-image='true' />
						</menu>
						<separator />
						<menuitem action='ObjectsMergeProperties' />
					</menu>
					<menu action='HelpMenu'>
						<menuitem action='HelpContents' />
						<menuitem action='HelpAbout' />
					</menu>
				</menubar>
			
				<toolbar name='MainToolBar'>
					<toolitem action='FileNew' />
					<toolitem action='FileOpen' />
					<toolitem action='FileSave' />
					<separator />
					<toolitem action='FilePrint' />
					<separator />
					<toolitem action='EditCut' />
					<toolitem action='EditCopy' />
					<toolitem action='EditPaste' />
				</toolbar>
			
				<toolbar name='DrawingToolBar'>
					<toolitem action='ObjectsArrowMode' />
					<separator />
					<toolitem action='ObjectsCreateText' />
					<toolitem action='ObjectsCreateBox' />
					<toolitem action='ObjectsCreateLine' />
					<toolitem action='ObjectsCreateEllipse' />
					<toolitem action='ObjectsCreateImage' />
					<toolitem action='ObjectsCreateBarcode' />
					<separator />
					<toolitem action='ViewZoomIn' />
					<toolitem action='ViewZoomOut' />
					<toolitem action='ViewZoom1to1' />
					<toolitem action='ViewZoomToFit' />
					<separator />
					<toolitem action='ObjectsMergeProperties' />
				</toolbar>
			
				<popup action='ContextMenu'>
					<menu action='ObjectsOrderMenu'>
						<menuitem action='ObjectsRaise' always-show-image='true' />
						<menuitem action='ObjectsLower' always-show-image='true' />
					</menu>
					<menu action='ObjectsRotateFlipMenu'>
						<menuitem action='ObjectsRotateLeft' always-show-image='true' />
						<menuitem action='ObjectsRotateRight' always-show-image='true' />
						<menuitem action='ObjectsFlipHorizontal' always-show-image='true' />
						<menuitem action='ObjectsFlipVertical' always-show-image='true' />
					</menu>
					<menu action='ObjectsAlignMenu'>
						<menuitem action='ObjectsAlignLeft' always-show-image='true' />
						<menuitem action='ObjectsAlignHCenter' always-show-image='true' />
						<menuitem action='ObjectsAlignRight' always-show-image='true' />
						<separator />
						<menuitem action='ObjectsAlignTop' always-show-image='true' />
						<menuitem action='ObjectsAlignVCenter' always-show-image='true' />
						<menuitem action='ObjectsAlignBottom' always-show-image='true' />
					</menu>
					<menu action='ObjectsCenterMenu'>
						<menuitem action='ObjectsCenterHorizontal' always-show-image='true' />
						<menuitem action='ObjectsCenterVertical' always-show-image='true' />
					</menu>
					<separator />
					<menuitem action='EditCut' />
					<menuitem action='EditCopy' />
					<menuitem action='EditPaste' />
					<menuitem action='EditDelete' />
				</popup>
			
				<popup action='EmptySelectionContextMenu'>
					<menuitem action='EditPaste' />
				</popup>
			
			</ui>
			""";
		

		static string[] doc_verbs = {
			"/ui/MenuBar/FileMenu/FileProperties",
			"/ui/MenuBar/FileMenu/FileSave",
			"/ui/MenuBar/FileMenu/FileSaveAs",
			"/ui/MenuBar/FileMenu/FilePrint",
			"/ui/MenuBar/FileMenu/FileClose",
			"/ui/MenuBar/EditMenu/EditUndo",
			"/ui/MenuBar/EditMenu/EditRedo",
			"/ui/MenuBar/EditMenu/EditCut",
			"/ui/MenuBar/EditMenu/EditCopy",
			"/ui/MenuBar/EditMenu/EditDelete",
			"/ui/MenuBar/EditMenu/EditSelectAll",
			"/ui/MenuBar/EditMenu/EditUnSelectAll",
			"/ui/MenuBar/ViewMenu/ViewZoomIn",
			"/ui/MenuBar/ViewMenu/ViewZoomOut",
			"/ui/MenuBar/ViewMenu/ViewZoom1to1",
			"/ui/MenuBar/ViewMenu/ViewZoomToFit",
			"/ui/MenuBar/ViewMenu/ViewGrid",
			"/ui/MenuBar/ViewMenu/ViewMarkup",
			"/ui/MenuBar/ObjectsMenu/ObjectsArrowMode",
			"/ui/MenuBar/ObjectsMenu/ObjectsCreateMenu",
			"/ui/MenuBar/ObjectsMenu/ObjectsCreateMenu/ObjectsCreateText",
			"/ui/MenuBar/ObjectsMenu/ObjectsCreateMenu/ObjectsCreateLine",
			"/ui/MenuBar/ObjectsMenu/ObjectsCreateMenu/ObjectsCreateBox",
			"/ui/MenuBar/ObjectsMenu/ObjectsCreateMenu/ObjectsCreateEllipse",
			"/ui/MenuBar/ObjectsMenu/ObjectsCreateMenu/ObjectsCreateImage",
			"/ui/MenuBar/ObjectsMenu/ObjectsCreateMenu/ObjectsCreateBarcode",
			"/ui/MenuBar/ObjectsMenu/ObjectsOrderMenu",
			"/ui/MenuBar/ObjectsMenu/ObjectsOrderMenu/ObjectsRaise",
			"/ui/MenuBar/ObjectsMenu/ObjectsOrderMenu/ObjectsLower",
			"/ui/MenuBar/ObjectsMenu/ObjectsRotateFlipMenu",
			"/ui/MenuBar/ObjectsMenu/ObjectsRotateFlipMenu/ObjectsRotateLeft",
			"/ui/MenuBar/ObjectsMenu/ObjectsRotateFlipMenu/ObjectsRotateRight",
			"/ui/MenuBar/ObjectsMenu/ObjectsRotateFlipMenu/ObjectsFlipHorizontal",
			"/ui/MenuBar/ObjectsMenu/ObjectsRotateFlipMenu/ObjectsFlipVertical",
			"/ui/MenuBar/ObjectsMenu/ObjectsAlignMenu",
			"/ui/MenuBar/ObjectsMenu/ObjectsAlignMenu/ObjectsAlignLeft",
			"/ui/MenuBar/ObjectsMenu/ObjectsAlignMenu/ObjectsAlignRight",
			"/ui/MenuBar/ObjectsMenu/ObjectsAlignMenu/ObjectsAlignHCenter",
			"/ui/MenuBar/ObjectsMenu/ObjectsAlignMenu/ObjectsAlignTop",
			"/ui/MenuBar/ObjectsMenu/ObjectsAlignMenu/ObjectsAlignBottom",
			"/ui/MenuBar/ObjectsMenu/ObjectsAlignMenu/ObjectsAlignVCenter",
			"/ui/MenuBar/ObjectsMenu/ObjectsCenterMenu",
			"/ui/MenuBar/ObjectsMenu/ObjectsCenterMenu/ObjectsCenterHorizontal",
			"/ui/MenuBar/ObjectsMenu/ObjectsCenterMenu/ObjectsCenterVertical",
			"/ui/MenuBar/ObjectsMenu/ObjectsMergeProperties"
		};

		static string[] doc_modified_verbs = {
			"/ui/MenuBar/FileMenu/FileSave"
		};

		static string[] paste_verbs = {
			"/ui/MenuBar/EditMenu/EditPaste"
		};

		static string[] selection_verbs = {
			"/ui/MenuBar/EditMenu/EditCut",
			"/ui/MenuBar/EditMenu/EditCopy",
			"/ui/MenuBar/EditMenu/EditDelete",
			"/ui/MenuBar/EditMenu/EditUnSelectAll",
			"/ui/MenuBar/ObjectsMenu/ObjectsOrderMenu",
			"/ui/MenuBar/ObjectsMenu/ObjectsOrderMenu/ObjectsRaise",
			"/ui/MenuBar/ObjectsMenu/ObjectsOrderMenu/ObjectsLower",
			"/ui/MenuBar/ObjectsMenu/ObjectsRotateFlipMenu",
			"/ui/MenuBar/ObjectsMenu/ObjectsRotateFlipMenu/ObjectsRotateLeft",
			"/ui/MenuBar/ObjectsMenu/ObjectsRotateFlipMenu/ObjectsRotateRight",
			"/ui/MenuBar/ObjectsMenu/ObjectsRotateFlipMenu/ObjectsFlipHorizontal",
			"/ui/MenuBar/ObjectsMenu/ObjectsRotateFlipMenu/ObjectsFlipVertical",
			"/ui/MenuBar/ObjectsMenu/ObjectsCenterMenu",
			"/ui/MenuBar/ObjectsMenu/ObjectsCenterMenu/ObjectsCenterHorizontal",
			"/ui/MenuBar/ObjectsMenu/ObjectsCenterMenu/ObjectsCenterVertical"
		};

		static string[] atomic_selection_verbs = {
		};

		static string[] multi_selection_verbs = {
			"/ui/MenuBar/ObjectsMenu/ObjectsAlignMenu",
			"/ui/MenuBar/ObjectsMenu/ObjectsAlignMenu/ObjectsAlignLeft",
			"/ui/MenuBar/ObjectsMenu/ObjectsAlignMenu/ObjectsAlignRight",
			"/ui/MenuBar/ObjectsMenu/ObjectsAlignMenu/ObjectsAlignHCenter",
			"/ui/MenuBar/ObjectsMenu/ObjectsAlignMenu/ObjectsAlignTop",
			"/ui/MenuBar/ObjectsMenu/ObjectsAlignMenu/ObjectsAlignBottom",
			"/ui/MenuBar/ObjectsMenu/ObjectsAlignMenu/ObjectsAlignVCenter"
		};


		public Ui( Window window )
		{
			this.window = window;
			this.prefs = new Prefs();

			connect_proxy.connect( on_connect_proxy );
			disconnect_proxy.connect( on_disconnect_proxy );

			Gtk.ActionGroup actions = new Gtk.ActionGroup( "Actions" );
			actions.set_translation_domain( "" );
			actions.add_actions( entries, this );
			actions.add_toggle_actions( toggle_entries, this );

			insert_action_group( actions, 0 );
			window.add_accel_group( get_accel_group() );

			try
			{
				add_ui_from_string( ui_info, ui_info.length );
			}
			catch ( Error e )
			{
				message( "building menus failed: %s", e.message );
			}

			/* Set the toolbar styles according to prefs */
			set_app_main_toolbar_style();
			set_app_drawing_toolbar_style();
		
			/* Set view grid and markup visibility according to prefs */
			set_view_style();
		
			set_verb_list_sensitive( doc_verbs, false );
			set_verb_list_sensitive( paste_verbs, false );
		}


		public void update_all( View view )
		{
			Model model = view.model;

			set_verb_list_sensitive( doc_verbs, true );

			set_verb_sensitive( "/ui/MenuBar/EditMenu/EditUndo", model.undo_redo.can_undo() );
			set_verb_sensitive( "/ui/MenuBar/EditMenu/EditRedo", model.undo_redo.can_redo() );

			set_verb_list_sensitive( doc_modified_verbs, model.label.modified );

			set_verb_sensitive( "/ui/MenuBar/ViewMenu/ViewZoomIn", !view.is_zoom_max() );
			set_verb_sensitive( "/ui/MenuBar/ViewMenu/ViewZoomOut", !view.is_zoom_min() );

			set_verb_list_sensitive( selection_verbs, !model.label.is_selection_empty() );

			set_verb_list_sensitive( atomic_selection_verbs, model.label.is_selection_atomic() );

			set_verb_list_sensitive( multi_selection_verbs,
			                         !model.label.is_selection_empty() && !model.label.is_selection_atomic() );
		}


		public void update_modified_verbs( Model model )
		{
			set_verb_list_sensitive( doc_modified_verbs, model.label.modified );
		}


		public void update_selection_verbs( Model model, bool view_has_focus )
		{
			if ( view_has_focus )
			{
				set_verb_list_sensitive( selection_verbs, !model.label.is_selection_empty() );

				set_verb_list_sensitive( atomic_selection_verbs,
				                         model.label.is_selection_atomic() );

				set_verb_list_sensitive( multi_selection_verbs,
				                         !model.label.is_selection_empty() &&
				                         !model.label.is_selection_atomic() );
			}
			else
			{
				set_verb_list_sensitive( selection_verbs, false );
				set_verb_list_sensitive( atomic_selection_verbs, false );
				set_verb_list_sensitive( multi_selection_verbs, false );
			}
		}


		public void update_zoom_verbs( View view )
		{
			set_verb_sensitive( "/ui/MenuBar/ViewMenu/ViewZoomIn", !view.is_zoom_max() );
			set_verb_sensitive( "/ui/MenuBar/ViewMenu/ViewZoomOut", !view.is_zoom_min() );
		}


		public void update_paste_verbs( bool can_paste )
		{
			set_verb_list_sensitive( paste_verbs, can_paste );
		}


		public void update_undo_redo_verbs( Model model )
		{
			Gtk.MenuItem  menu_item;
			string        description;
			string        menu_label;

			menu_item = (Gtk.MenuItem)get_widget( "/MenuBar/EditMenu/EditUndo" );
			description = model.undo_redo.get_undo_description();
			menu_label = "%s: %s".printf( _("Undo"), description );
			menu_item.set_label( menu_label );

			menu_item = (Gtk.MenuItem)get_widget( "/MenuBar/EditMenu/EditRedo" );
			description = model.undo_redo.get_redo_description();
			menu_label = "%s: %s".printf( _("Redo"), description );
			menu_item.set_label( menu_label );

			set_verb_sensitive( "/ui/MenuBar/EditMenu/EditUndo", model.undo_redo.can_undo() );
			set_verb_sensitive( "/ui/MenuBar/EditMenu/EditRedo", model.undo_redo.can_redo() );
		}


		private void set_app_main_toolbar_style()
		{
			/* Updated view menu */
			set_verb_state( "/ui/ViewMenu/ViewMainToolBar", prefs.main_toolbar_visible );

			Gtk.Toolbar toolbar = (Gtk.Toolbar)get_widget( "/MainToolBar" );

			if ( prefs.main_toolbar_visible )
			{
				toolbar.show_all();
			}
			else
			{
				toolbar.hide();
			}
		}


		private void set_app_drawing_toolbar_style()
		{
			/* Updated view menu */
			set_verb_state( "/ui/MenuBar/ViewMenu/ViewDrawingToolBar",
			                prefs.drawing_toolbar_visible );

			Gtk.Toolbar toolbar = (Gtk.Toolbar)get_widget( "/DrawingToolBar" );

			toolbar.set_style( Gtk.ToolbarStyle.ICONS );

			if ( prefs.drawing_toolbar_visible )
			{
				toolbar.show_all();
			}
			else
			{
				toolbar.hide();
			}
		}


		private void set_view_style()
		{
			set_verb_state( "/ui/MenuBar/ViewMenu/ViewGrid", prefs.grid_visible );

			set_verb_state( "/ui/MenuBar/ViewMenu/ViewMarkup", prefs.markup_visible );
		}


		private void on_connect_proxy( Gtk.Action action,
		                               Gtk.Widget proxy )
		{
			if ( proxy is Gtk.MenuItem )
			{
				((Gtk.MenuItem)proxy).select.connect( on_menu_item_select );
				((Gtk.MenuItem)proxy).deselect.connect( on_menu_item_deselect );
			}
		}


		private void on_disconnect_proxy( Gtk.Action action,
		                                  Gtk.Widget proxy )
		{
			if ( proxy is Gtk.MenuItem )
			{
				((Gtk.MenuItem)proxy).select.disconnect( on_menu_item_select );
				((Gtk.MenuItem)proxy).deselect.disconnect( on_menu_item_deselect );
			}
		}


		private void on_menu_item_select( Gtk.MenuItem proxy )
		{
			Gtk.Action? action = proxy.get_data( "gtk-action" );
			return_if_fail( action != null );
	
			string? message = action.get_data( "tooltip" );
			if ( message != null )
			{
				window.statusbar.push( window.menu_tips_context_id, message );
			}
		}


		private void on_menu_item_deselect( Gtk.MenuItem proxy )
		{
			window.statusbar.pop( window.menu_tips_context_id );
		}


		private void set_verb_sensitive( string  cname,
		                                 bool    sensitive )
		{
			Gtk.Action action = get_action( cname );

			if ( action != null )
			{
				action.set_sensitive( sensitive );
			}
		}


		private void set_verb_list_sensitive(  string[] vlist,
		                                       bool     sensitive )
		{
			foreach ( string verb in vlist )
			{
				Gtk.Action action = get_action( verb );

				if ( action != null )
				{
					action.set_sensitive( sensitive );
				}
			}
		}


		private void set_verb_state( string cname,
		                             bool   state )
		{
			Gtk.ToggleAction action = (Gtk.ToggleAction)get_action( cname );

			if ( action != null )
			{
				action.set_active( state );
			}
		}



		/*****************************
		 * Actions
		 ****************************/

		private void on_file_new( Gtk.Action action )
		{
			File.new_label( window );
		}


		private void on_file_properties( Gtk.Action action )
		{
			stdout.printf( "gl_file_properties (GL_VIEW(window->view)->label, window)\n" );
		}


		private void on_file_template_designer( Gtk.Action action )
		{
			stdout.printf( """dialog = gl_template_designer_new (GTK_WINDOW(window));
			               gtk_widget_show (dialog);\n""" );
		}


		private void on_file_open( Gtk.Action action )
		{
			File.open( window );
		}


		private void on_file_save( Gtk.Action action )
		{
			File.save( window.model.label, window );
		}


		private void on_file_save_as( Gtk.Action action )
		{
			File.save_as( window.model.label, window );
		}


		private void on_file_print( Gtk.Action action )
		{
			File.print( window.model, window );
		}


		private void on_file_close( Gtk.Action action )
		{
			File.close( window );
		}


		private void on_file_quit( Gtk.Action action )
		{
			File.exit();
		}


		private void on_edit_undo( Gtk.Action action )
		{
			stdout.printf( "gl_label_undo (GL_LABEL (GL_VIEW (window->view)->label));\n" );
		}


		private void on_edit_redo( Gtk.Action action )
		{
			stdout.printf( "gl_label_redo (GL_LABEL (GL_VIEW (window->view)->label));\n" );
		}


		private void on_edit_cut( Gtk.Action action )
		{
			stdout.printf( "gl_label_cut_selection (window->label);\n" );
		}


		private void on_edit_copy( Gtk.Action action )
		{
			stdout.printf( "gl_label_copy_selection (window->label);\n" );
		}


		private void on_edit_paste( Gtk.Action action )
		{
			stdout.printf( "gl_label_paste (window->label);\n" );
		}


		private void on_edit_delete( Gtk.Action action )
		{
			stdout.printf( "gl_label_delete_selection (GL_VIEW(window->view)->label);\n" );
		}


		private void on_edit_select_all( Gtk.Action action )
		{
			window.model.label.select_all();
		}


		private void on_edit_unselect_all( Gtk.Action action )
		{
			window.model.label.unselect_all();
		}


		private void on_edit_preferences( Gtk.Action action )
		{
			/*
			static GtkWidget *dialog = NULL;

			if (dialog != NULL)
			{
				gtk_window_present (GTK_WINDOW (dialog));
				gtk_window_set_transient_for (GTK_WINDOW (dialog),        
				                              GTK_WINDOW(window ));

			} else {
                
				dialog = gl_prefs_dialog_new (GTK_WINDOW(window ));

				g_signal_connect (G_OBJECT (dialog), "destroy",
				                  G_CALLBACK (gtk_widget_destroyed), &dialog);
        
				gtk_widget_show (dialog);

			}
			*/
		}


		private void on_view_main_toolbar_toggled( Gtk.Action action )
		{
			Gtk.ToggleAction toggle_action = (Gtk.ToggleAction)action;
			
			prefs.main_toolbar_visible = toggle_action.active;
			set_app_main_toolbar_style();
		}


		private void on_view_drawing_toolbar_toggled( Gtk.Action action )
		{
			Gtk.ToggleAction toggle_action = (Gtk.ToggleAction)action;
			
			prefs.drawing_toolbar_visible = toggle_action.active;
			set_app_drawing_toolbar_style();
		}


		private void on_view_property_bar_toggled( Gtk.Action action )
		{
			/*
			gboolean     state;

			state =  gtk_toggle_action_get_active (action);

			gl_prefs_model_set_property_toolbar_visible (gl_prefs, state);
			if (state) {
				gtk_widget_show (GTK_WIDGET (window->property_bar));
			} else {
				gtk_widget_hide (GTK_WIDGET (window->property_bar));
			}
			*/
		}


		private void on_view_grid_toggled (Gtk.Action action )
		{
			Gtk.ToggleAction toggle_action = (Gtk.ToggleAction)action;

			bool state =  toggle_action.get_active();

			if ( window.view != null )
			{
				window.view.grid_visible = state;
			}
			prefs.grid_visible = state;
		}


		private void on_view_markup_toggled (Gtk.Action action )
		{
			Gtk.ToggleAction toggle_action = (Gtk.ToggleAction)action;

			bool state =  toggle_action.get_active();

			if ( window.view != null )
			{
				window.view.markup_visible = state;
			}
			prefs.markup_visible = state;
		}


		private void on_view_zoomin( Gtk.Action action )
		{
			window.view.zoom_in();
		}


		private void on_view_zoomout( Gtk.Action action )
		{
			window.view.zoom_out();
		}


		private void on_view_zoom1to1( Gtk.Action action )
		{
			window.view.zoom_1to1();
		}


		private void on_view_zoom_to_fit( Gtk.Action action )
		{
			window.view.zoom_to_fit();
		}


		private void on_objects_arrow_mode( Gtk.Action action )
		{
			window.view.arrow_mode();
		}


		private void on_objects_create_box( Gtk.Action action )
		{
			window.view.create_box_mode();
		}


		private void on_objects_create_ellipse( Gtk.Action action )
		{
			window.view.create_ellipse_mode();
		}


		private void on_objects_create_line( Gtk.Action action )
		{
			window.view.create_line_mode();
		}


		private void on_objects_create_image( Gtk.Action action )
		{
			window.view.create_image_mode();
		}


		private void on_objects_create_text( Gtk.Action action )
		{
			window.view.create_text_mode();
		}


		private void on_objects_create_barcode( Gtk.Action action )
		{
			/*
			if (window->view != NULL) {
				gl_view_object_create_mode (GL_VIEW(window->view),
				                            GL_LABEL_OBJECT_BARCODE);
			}
			*/
		}


		private void on_objects_raise( Gtk.Action action )
		{
			window.model.label.raise_selection_to_top();
		}


		private void on_objects_lower( Gtk.Action action )
		{
			window.model.label.lower_selection_to_bottom();
		}


		private void on_objects_rotate_left( Gtk.Action action )
		{
			window.model.label.rotate_selection_left();
		}


		private void on_objects_rotate_right( Gtk.Action action )
		{
			window.model.label.rotate_selection_right();
		}


		private void on_objects_flip_horiz( Gtk.Action action )
		{
			window.model.label.flip_selection_horiz();
		}


		private void on_objects_flip_vert( Gtk.Action action )
		{
			window.model.label.flip_selection_vert();
		}


		private void on_objects_align_left( Gtk.Action action )
		{
			window.model.label.align_selection_left();
		}


		private void on_objects_align_right( Gtk.Action action )
		{
			window.model.label.align_selection_right();
		}


		private void on_objects_align_hcenter( Gtk.Action action )
		{
			window.model.label.align_selection_hcenter();
		}


		private void on_objects_align_top( Gtk.Action action )
		{
			window.model.label.align_selection_top();
		}


		private void on_objects_align_bottom( Gtk.Action action )
		{
			window.model.label.align_selection_bottom();
		}


		private void on_objects_align_vcenter( Gtk.Action action )
		{
			window.model.label.align_selection_vcenter();
		}


		private void on_objects_center_horiz( Gtk.Action action )
		{
			window.model.label.center_selection_horiz();
		}


		private void on_objects_center_vert( Gtk.Action action )
		{
			window.model.label.center_selection_vert();
		}


		private void on_objects_merge_properties( Gtk.Action action )
		{
			/*
			if (window->merge_dialog) {

				gtk_window_present (GTK_WINDOW(window->merge_dialog));
				gtk_window_set_transient_for (GTK_WINDOW (window->merge_dialog),
				                              GTK_WINDOW (window ));

			} else {

				window->merge_dialog =
					g_object_ref (
						gl_merge_properties_dialog_new (GL_VIEW(window->view)->label,
						                                GTK_WINDOW(window )) );

				g_signal_connect (G_OBJECT(window->merge_dialog), "destroy",
				                  G_CALLBACK (gtk_widget_destroyed),
				                  &window->merge_dialog);

				gtk_widget_show (GTK_WIDGET (window->merge_dialog));

			}
			*/
		}


		private void on_help_contents( Gtk.Action action )
		{
			Help.display_contents( window );
		}


		private void on_help_about( Gtk.Action action )
		{
			Help.display_about_dialog( window );
		}

	}

}


