/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/**
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  ui.c:  GLabels ui module
 *
 *  Copyright (C) 2001-2002  Jim Evins <evins@snaught.com>.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */
#include <config.h>

#include "ui.h"

#include <glib/gi18n.h>
#include <gconf/gconf-client.h>

#include "ui-util.h"
#include "ui-commands.h"
#include "file.h"
#include "prefs.h"
#include "stock.h"
#include "window.h"

#include "debug.h"

/*==========================================================================*/
/* Private macros and constants.                                            */
/*==========================================================================*/

/*==========================================================================*/
/* Private types.                                                           */
/*==========================================================================*/


/*==========================================================================*/
/* Local function prototypes                                                */
/*==========================================================================*/

static void view_ui_item_toggled_cb        (GtkToggleAction *action,
					    GtkUIManager    *ui);

static void set_app_main_toolbar_style 	   (GtkUIManager    *ui);

static void set_app_drawing_toolbar_style  (GtkUIManager    *ui);

static void set_view_style                 (GtkUIManager    *ui);

/*==========================================================================*/
/* Private globals                                                          */
/*==========================================================================*/

static GtkActionEntry entries[] = {

	/* Menu entries. */
	{ "FileMenu",                NULL, "_File" },
	{ "FileRecentsMenu",         NULL, "Recent _Files" },
	{ "EditMenu",                NULL, "_Edit" },
	{ "ViewMenu",                NULL, "_View" },
	{ "ViewMainToolBarMenu",     NULL, "Customize Main Toolbar" },
	{ "ViewDrawingToolBarMenu",  NULL, "Customize Drawing Toolbar" },
	{ "ViewPropertyToolBarMenu", NULL, "Customize Properties Toolbar" },
	{ "ObjectsMenu",             NULL, "_Objects" },
	{ "ObjectsCreateMenu",       NULL, "_Create" },
	{ "ObjectsOrderMenu",        NULL, "_Order" },
	{ "ObjectsRotateFlipMenu",   NULL, "_Rotate/Flip" },
	{ "ObjectsAlignHorizMenu",   NULL, "Align _Horizontal" },
	{ "ObjectsAlignVertMenu",    NULL, "Align _Vertical" },
	{ "HelpMenu",                NULL, "_Help" },


	/* File action entries. */
	{ "FileNew",
	  GTK_STOCK_NEW,
	  "_New",
	  "<control>N",
	  "Create a new file",
	  G_CALLBACK (gl_ui_cmd_file_new) },

	{ "FileOpen",
	  GTK_STOCK_OPEN,
	  "_Open...",
	  "<control>O",
	  "Open a file",
	  G_CALLBACK (gl_ui_cmd_file_open) },

	{ "FileSave",
	  GTK_STOCK_SAVE,
	  "_Save",
	  "<control>S",
	  "Save current file",
	  G_CALLBACK (gl_ui_cmd_file_save) },

	{ "FileSaveAs",
	  GTK_STOCK_SAVE,
	  "Save _As...",
	  "<shift><control>S",
	  "Save the current file to a different name",
	  G_CALLBACK (gl_ui_cmd_file_save_as) },

	{ "FilePrint",
	  GTK_STOCK_PRINT,
	  "_Print...",
	  "<control>P",
	  "Print the current file",
	  G_CALLBACK (gl_ui_cmd_file_print) },

	{ "FileProperties",
	  GTK_STOCK_PROPERTIES,
	  "Properties...",
	  NULL,
	  "Modify document properties",
	  G_CALLBACK (gl_ui_cmd_file_properties) },

	{ "FileTemplateDesigner",
	  NULL,
	  "Template _Designer...",
	  NULL,
	  "Create a custom template",
	  G_CALLBACK (gl_ui_cmd_file_template_designer) },

	{ "FileClose",
	  GTK_STOCK_CLOSE,
	  "_Close",
	  "<alt>F4",
	  "Close the current file",
	  G_CALLBACK (gl_ui_cmd_file_close) },

	{ "FileQuit",
	  GTK_STOCK_QUIT,
	  "_Quit",
	  "<control>Q",
	  "Quit the program",
	  G_CALLBACK (gl_ui_cmd_file_quit) },


	/* Edit action entries. */
	{ "EditCut",
	  GTK_STOCK_CUT,
	  "Cut",
	  "<control>X",
	  "Cut the selection",
	  G_CALLBACK (gl_ui_cmd_edit_cut) },

	{ "EditCopy",
	  GTK_STOCK_COPY,
	  "Copy",
	  "<control>C",
	  "Copy the selection",
	  G_CALLBACK (gl_ui_cmd_edit_copy) },

	{ "EditPaste",
	  GTK_STOCK_PASTE,
	  "Paste",
	  "<control>V",
	  "Paste the clipboard",
	  G_CALLBACK (gl_ui_cmd_edit_paste) },

	{ "EditDelete",
	  NULL,
	  "Delete",
	  NULL,
	  "Delete the selected objects",
	  G_CALLBACK (gl_ui_cmd_edit_delete) },

	{ "EditSelectAll",
	  NULL,
	  "Select All",
	  "<control>A",
	  "Select all objects",
	  G_CALLBACK (gl_ui_cmd_edit_select_all) },

	{ "EditUnSelectAll",
	  NULL,
	  "Un-select All",
	  NULL,
	  "Remove all selections",
	  G_CALLBACK (gl_ui_cmd_edit_unselect_all) },

	{ "EditPreferences",
	  GTK_STOCK_PREFERENCES,
	  "Preferences",
	  NULL,
	  "Configure the application",
	  G_CALLBACK (gl_ui_cmd_edit_preferences) },


	/* View action entries. */
	{ "ViewZoomIn",
	  GTK_STOCK_ZOOM_IN,
	  "Zoom in",
	  NULL,
	  "Increase magnification",
	  G_CALLBACK (gl_ui_cmd_view_zoomin) },

	{ "ViewZoomOut",
	  GTK_STOCK_ZOOM_OUT,
	  "Zoom out",
	  NULL,
	  "Decrease magnification",
	  G_CALLBACK (gl_ui_cmd_view_zoomout) },

	{ "ViewZoom1to1",
	  GTK_STOCK_ZOOM_100,
	  "Zoom 1 to 1",
	  NULL,
	  "Restore scale to 100%",
	  G_CALLBACK (gl_ui_cmd_view_zoom1to1) },

	{ "ViewZoomToFit",
	  GTK_STOCK_ZOOM_FIT,
	  "Zoom to fit",
	  NULL,
	  "Set scale to fit window",
	  G_CALLBACK (gl_ui_cmd_view_zoom_to_fit) },


	/* Objects action entries. */
	{ "ObjectsArrowMode",
	  GL_STOCK_ARROW,
	  "Select Mode",
	  NULL,
	  "Select, move and modify objects",
	  G_CALLBACK (gl_ui_cmd_objects_arrow_mode) },

	{ "ObjectsCreateText",
	  GL_STOCK_TEXT,
	  "Text",
	  NULL,
	  "Create text object",
	  G_CALLBACK (gl_ui_cmd_objects_create_text) },

	{ "ObjectsCreateBox",
	  GL_STOCK_BOX,
	  "Box",
	  NULL,
	  "Create box/rectangle object",
	  G_CALLBACK (gl_ui_cmd_objects_create_box) },

	{ "ObjectsCreateLine",
	  GL_STOCK_LINE,
	  "Line",
	  NULL,
	  "Create line object",
	  G_CALLBACK (gl_ui_cmd_objects_create_line) },

	{ "ObjectsCreateEllipse",
	  GL_STOCK_ELLIPSE,
	  "Ellipse",
	  NULL,
	  "Create ellipse/circle object",
	  G_CALLBACK (gl_ui_cmd_objects_create_ellipse) },

	{ "ObjectsCreateImage",
	  GL_STOCK_IMAGE,
	  "Image",
	  NULL,
	  "Create image object",
	  G_CALLBACK (gl_ui_cmd_objects_create_image) },

	{ "ObjectsCreateBarcode",
	  GL_STOCK_BARCODE,
	  "Barcode",
	  NULL,
	  "Create barcode object",
	  G_CALLBACK (gl_ui_cmd_objects_create_barcode) },
	
	{ "ObjectsRaise",
	  GL_STOCK_ORDER_TOP,
	  "Bring to front", NULL,
	  "Raise object to top",
	  G_CALLBACK (gl_ui_cmd_objects_raise) },

	{ "ObjectsLower",
	  GL_STOCK_ORDER_BOTTOM,
	  "Send to back",
	  NULL,
	  "Lower object to bottom",
	  G_CALLBACK (gl_ui_cmd_objects_lower) },

	{ "ObjectsRotateLeft",
	  GL_STOCK_ROTATE_LEFT,
	  "Rotate left",
	  NULL,
	  "Rotate object 90 degrees counter-clockwise",
	  G_CALLBACK (gl_ui_cmd_objects_rotate_left) },

	{ "ObjectsRotateRight",
	  GL_STOCK_ROTATE_RIGHT,
	  "Rotate right",
	  NULL,
	  "Rotate object 90 degrees clockwise",
	  G_CALLBACK (gl_ui_cmd_objects_rotate_right) },

	{ "ObjectsFlipHorizontal",
	  GL_STOCK_FLIP_HORIZ,
	  "Flip horizontally",
	  NULL,
	  "Flip object horizontally",
	  G_CALLBACK (gl_ui_cmd_objects_flip_horiz) },

	{ "ObjectsFlipVertical",
	  GL_STOCK_FLIP_VERT,
	  "Flip vertically",
	  NULL,
	  "Flip object vertically",
	  G_CALLBACK (gl_ui_cmd_objects_flip_vert) },

	{ "ObjectsAlignLeft",
	  GL_STOCK_ALIGN_LEFT,
	  "Align left",
	  NULL,
	  "Align objects to left edges",
	  G_CALLBACK (gl_ui_cmd_objects_align_left) },

	{ "ObjectsAlignRight",
	  GL_STOCK_ALIGN_RIGHT,
	  "Align right",
	  NULL,
	  "Align objects to right edges",
	  G_CALLBACK (gl_ui_cmd_objects_align_right) },

	{ "ObjectsAlignHCenter",
	  GL_STOCK_ALIGN_HCENTER,
	  "Align horizontal center",
	  NULL,
	  "Align objects to horizontal centers",
	  G_CALLBACK (gl_ui_cmd_objects_align_hcenter) },

	{ "ObjectsAlignTop",
	  GL_STOCK_ALIGN_TOP,
	  "Align tops",
	  NULL,
	  "Align objects to top edges",
	  G_CALLBACK (gl_ui_cmd_objects_align_top) },

	{ "ObjectsAlignBottom",
	  GL_STOCK_ALIGN_BOTTOM,
	  "Align bottoms",
	  NULL,
	  "Align objects to bottom edges",
	  G_CALLBACK (gl_ui_cmd_objects_align_bottom) },

	{ "ObjectsAlignVCenter",
	  GL_STOCK_ALIGN_VCENTER,
	  "Align vertical center",
	  NULL,
	  "Align objects to vertical centers",
	  G_CALLBACK (gl_ui_cmd_objects_align_vcenter) },

	{ "ObjectsCenterHorizontal",
	  GL_STOCK_CENTER_HORIZ,
	  "Center horizontally",
	  NULL,
	  "Center objects to horizontal label center",
	  G_CALLBACK (gl_ui_cmd_objects_center_horiz) },

	{ "ObjectsCenterVertical",
	  GL_STOCK_CENTER_VERT,
	  "Center vertically",
	  NULL,
	  "Center objects to vertical label center",
	  G_CALLBACK (gl_ui_cmd_objects_center_vert) },

	{ "ObjectsMergeProperties",
	  GL_STOCK_MERGE,
	  "Merge properties",
	  NULL,
	  "Edit merge properties",
	  G_CALLBACK (gl_ui_cmd_objects_merge_properties) },


	/* Help actions entries. */
	{ "HelpContents",
	  GTK_STOCK_HELP,
	  "Contents",
	  "F1",
	  "Open glabels manual",
	  G_CALLBACK (gl_ui_cmd_help_contents) },

	{ "HelpAbout",
	  GTK_STOCK_ABOUT,
	  "About...",
	  NULL,
	  "About glabels",
	  G_CALLBACK (gl_ui_cmd_help_about) },

};
static guint n_entries = G_N_ELEMENTS (entries);

static GtkToggleActionEntry toggle_entries[] = {

	{ "ViewPropertyToolBar",
	  NULL,
	  "Property toolbar",
	  NULL,
	  "Change the visibility of the property toolbar in the current window",
	  G_CALLBACK (gl_ui_cmd_view_property_bar_toggle),
	  TRUE },

	{ "ViewPropertyToolBarToolTips",
	  NULL,
	  "Show tooltips",
	  NULL,
	  "Show tooltips for property toolbar",
	  G_CALLBACK (gl_ui_cmd_view_property_bar_tips_toggle),
	  TRUE },

	{ "ViewGrid",
	  NULL,
	  "Grid",
	  NULL,
	  "Change the visibility of the grid in the current window",
	  G_CALLBACK (gl_ui_cmd_view_grid_toggle),
	  TRUE },

	{ "ViewMarkup",
	  NULL,
	  "Markup",
	  NULL,
	  "Change the visibility of markup lines in the current window",
	  G_CALLBACK (gl_ui_cmd_view_markup_toggle),
	  TRUE },

};
static guint n_toggle_entries = G_N_ELEMENTS (toggle_entries);

static GtkToggleActionEntry ui_toggle_entries[] = {

	{ "ViewMainToolBar",
	  NULL,
	  "Main toolbar",
	  NULL,
	  "Change the visibility of the main toolbar in the current window",
	  G_CALLBACK (view_ui_item_toggled_cb),
	  TRUE },

	{ "ViewDrawingToolBar",
	  NULL,
	  "Drawing toolbar",
	  NULL,
	  "Change the visibility of the drawing toolbar in the current window",
	  G_CALLBACK (view_ui_item_toggled_cb),
	  TRUE },

	{ "ViewMainToolBarToolTips",
	  NULL,
	  "Show tooltips",
	  NULL,
	  "Show tooltips for main toolbar",
	  G_CALLBACK (view_ui_item_toggled_cb),
	  TRUE },

	{ "ViewDrawingToolBarToolTips",
	  NULL,
	  "Show tooltips",
	  NULL,
	  "Show tooltips for drawing toolbar",
	  G_CALLBACK (view_ui_item_toggled_cb),
	  TRUE },

};
static guint n_ui_toggle_entries = G_N_ELEMENTS (ui_toggle_entries);

static const gchar *ui_info = 
"<ui>"
""
"	<menubar name='MenuBar'>"
"		<menu action='FileMenu'>"
"			<menuitem action='FileNew' />"
"			<menuitem action='FileOpen' />"
"                       <menu action='FileRecentsMenu'>"
"			        <placeholder name='FileRecentsPlaceHolder' />"
"                       </menu>"
"			<separator />"
"			<menuitem action='FileSave' />"
"			<menuitem action='FileSaveAs' />"
"			<separator />"
"			<menuitem action='FilePrint' />"
"			<separator />"
"			<menuitem action='FileProperties' />"
"			<menuitem action='FileTemplateDesigner' />"
"			<separator />"
"			<menuitem action='FileClose' />"
"			<menuitem action='FileQuit' />"
"		</menu>"
"		<menu action='EditMenu'>"
"			<menuitem action='EditCut' />"
"			<menuitem action='EditCopy' />"
"			<menuitem action='EditPaste' />"
"			<menuitem action='EditDelete' />"
"			<separator />"
"			<menuitem action='EditSelectAll' />"
"			<menuitem action='EditUnSelectAll' />"
"			<separator />"
"			<menuitem action='EditPreferences' />"
"		</menu>"
"		<menu action='ViewMenu'>"
"			<menuitem action='ViewMainToolBar' />"
"			<menuitem action='ViewDrawingToolBar' />"
"			<menuitem action='ViewPropertyToolBar' />"
"			<separator />"
"			<menu action='ViewMainToolBarMenu'>"
"				<menuitem action='ViewMainToolBarToolTips' />"
"			</menu>"
"			<menu action='ViewDrawingToolBarMenu'>"
"				<menuitem action='ViewDrawingToolBarToolTips' />"
"			</menu>"
"			<menu action='ViewPropertyToolBarMenu'>"
"				<menuitem action='ViewPropertyToolBarToolTips' />"
"			</menu>"
"			<separator />"
"			<menuitem action='ViewGrid' />"
"			<menuitem action='ViewMarkup' />"
"			<separator />"
"			<menuitem action='ViewZoomIn' />"
"			<menuitem action='ViewZoomOut' />"
"			<menuitem action='ViewZoom1to1' />"
"			<menuitem action='ViewZoomToFit' />"
"		</menu>"
"		<menu action='ObjectsMenu'>"
"			<menuitem action='ObjectsArrowMode' />"
"			<menu action='ObjectsCreateMenu'>"
"				<menuitem action='ObjectsCreateText' />"
"				<menuitem action='ObjectsCreateBox' />"
"				<menuitem action='ObjectsCreateLine' />"
"				<menuitem action='ObjectsCreateEllipse' />"
"				<menuitem action='ObjectsCreateImage' />"
"				<menuitem action='ObjectsCreateBarcode' />"
"			</menu>"
"			<separator />"
"			<menu action='ObjectsOrderMenu'>"
"				<menuitem action='ObjectsRaise' />"
"				<menuitem action='ObjectsLower' />"
"			</menu>"
"			<menu action='ObjectsRotateFlipMenu'>"
"				<menuitem action='ObjectsRotateLeft' />"
"				<menuitem action='ObjectsRotateRight' />"
"				<menuitem action='ObjectsFlipHorizontal' />"
"				<menuitem action='ObjectsFlipVertical' />"
"			</menu>"
"			<menu action='ObjectsAlignHorizMenu'>"
"				<menuitem action='ObjectsAlignLeft' />"
"				<menuitem action='ObjectsAlignHCenter' />"
"				<menuitem action='ObjectsAlignRight' />"
"				<menuitem action='ObjectsCenterHorizontal' />"
"			</menu>"
"			<menu action='ObjectsAlignVertMenu'>"
"				<menuitem action='ObjectsAlignTop' />"
"				<menuitem action='ObjectsAlignVCenter' />"
"				<menuitem action='ObjectsAlignBottom' />"
"				<menuitem action='ObjectsCenterVertical' />"
"			</menu>"
"			<separator />"
"			<menuitem action='ObjectsMergeProperties' />"
"		</menu>"
"		<menu action='HelpMenu'>"
"			<menuitem action='HelpContents' />"
"			<menuitem action='HelpAbout' />"
"		</menu>"
"	</menubar>"
""
"	<toolbar name='MainToolBar'>"
"		<toolitem action='FileNew' />"
"		<toolitem action='FileOpen' />"
"		<toolitem action='FileSave' />"
"		<separator />"
"		<toolitem action='FilePrint' />"
"		<separator />"
"		<toolitem action='EditCut' />"
"		<toolitem action='EditCopy' />"
"		<toolitem action='EditPaste' />"
"	</toolbar>"
""
"	<toolbar name='DrawingToolBar'>"
"		<toolitem action='ObjectsArrowMode' />"
"		<separator />"
"		<toolitem action='ObjectsCreateText' />"
"		<toolitem action='ObjectsCreateBox' />"
"		<toolitem action='ObjectsCreateLine' />"
"		<toolitem action='ObjectsCreateEllipse' />"
"		<toolitem action='ObjectsCreateImage' />"
"		<toolitem action='ObjectsCreateBarcode' />"
"		<separator />"
"		<toolitem action='ViewZoomIn' />"
"		<toolitem action='ViewZoomOut' />"
"		<toolitem action='ViewZoom1to1' />"
"		<toolitem action='ViewZoomToFit' />"
"		<separator />"
"		<toolitem action='ObjectsMergeProperties' />"
"	</toolbar>"
""
"</ui>";


static gchar* doc_verbs [] = {
	"/ui/MenuBar/FileMenu/FileProperties",
	"/ui/MenuBar/FileMenu/FileSave",
	"/ui/MenuBar/FileMenu/FileSaveAs",
	"/ui/MenuBar/FileMenu/FilePrint",
	"/ui/MenuBar/FileMenu/FileClose",
	"/ui/MenuBar/EditMenu/EditCut",
	"/ui/MenuBar/EditMenu/EditCopy",
	"/ui/MenuBar/EditMenu/EditPaste",
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
	"/ui/MenuBar/ObjectsMenu/ObjectsCreateMenu/ObjectsCreateText",
	"/ui/MenuBar/ObjectsMenu/ObjectsCreateMenu/ObjectsCreateLine",
	"/ui/MenuBar/ObjectsMenu/ObjectsCreateMenu/ObjectsCreateBox",
	"/ui/MenuBar/ObjectsMenu/ObjectsCreateMenu/ObjectsCreateEllipse",
	"/ui/MenuBar/ObjectsMenu/ObjectsCreateMenu/ObjectsCreateImage",
	"/ui/MenuBar/ObjectsMenu/ObjectsCreateMenu/ObjectsCreateBarcode",
	"/ui/MenuBar/ObjectsMenu/ObjectsOrderMenu/ObjectsRaise",
	"/ui/MenuBar/ObjectsMenu/ObjectsOrderMenu/ObjectsLower",
	"/ui/MenuBar/ObjectsMenu/ObjectsRotateFlipMenu/ObjectsRotateLeft",
	"/ui/MenuBar/ObjectsMenu/ObjectsRotateFlipMenu/ObjectsRotateRight",
	"/ui/MenuBar/ObjectsMenu/ObjectsRotateFlipMenu/ObjectsFlipHorizontal",
	"/ui/MenuBar/ObjectsMenu/ObjectsRotateFlipMenu/ObjectsFlipVertical",
	"/ui/MenuBar/ObjectsMenu/ObjectsAlignHorizMenu/ObjectsAlignLeft",
	"/ui/MenuBar/ObjectsMenu/ObjectsAlignHorizMenu/ObjectsAlignRight",
	"/ui/MenuBar/ObjectsMenu/ObjectsAlignHorizMenu/ObjectsAlignHCenter",
	"/ui/MenuBar/ObjectsMenu/ObjectsAlignHorizMenu/ObjectsCenterHorizontal",
	"/ui/MenuBar/ObjectsMenu/ObjectsAlignVertMenu/ObjectsAlignTop",
	"/ui/MenuBar/ObjectsMenu/ObjectsAlignVertMenu/ObjectsAlignBottom",
	"/ui/MenuBar/ObjectsMenu/ObjectsAlignVertMenu/ObjectsAlignVCenter",
	"/ui/MenuBar/ObjectsMenu/ObjectsAlignVertMenu/ObjectsCenterVertical",
	"/ui/MenuBar/ObjectsMenu/ObjectsMergeProperties",

	NULL
};

static gchar* doc_modified_verbs [] = {
	"/ui/MenuBar/FileMenu/Save",

	NULL
};

static gchar* selection_verbs [] = {
	"/ui/MenuBar/EditMenu/EditCut",
	"/ui/MenuBar/EditMenu/EditCopy",
	"/ui/MenuBar/EditMenu/EditDelete",
	"/ui/MenuBar/EditMenu/EditUnSelectAll",
	"/ui/MenuBar/ObjectsMenu/ObjectsOrderMenu/ObjectsRaise",
	"/ui/MenuBar/ObjectsMenu/ObjectsOrderMenu/ObjectsLower",
	"/ui/MenuBar/ObjectsMenu/ObjectsRotateFlipMenu/ObjectsRotateLeft",
	"/ui/MenuBar/ObjectsMenu/ObjectsRotateFlipMenu/ObjectsRotateRight",
	"/ui/MenuBar/ObjectsMenu/ObjectsRotateFlipMenu/ObjectsFlipHorizontal",
	"/ui/MenuBar/ObjectsMenu/ObjectsRotateFlipMenu/ObjectsFlipVertical",
	"/ui/MenuBar/ObjectsMenu/ObjectsAlignHorizMenu/ObjectsCenterHorizontal",
	"/ui/MenuBar/ObjectsMenu/ObjectsAlignVertMenu/ObjectsCenterVertical",

	NULL
};

static gchar* atomic_selection_verbs [] = {

	NULL
};

static gchar* multi_selection_verbs [] = {
	"/ui/MenuBar/ObjectsMenu/ObjectsAlignHorizMenu/ObjectsAlignLeft",
	"/ui/MenuBar/ObjectsMenu/ObjectsAlignHorizMenu/ObjectsAlignRight",
	"/ui/MenuBar/ObjectsMenu/ObjectsAlignHorizMenu/ObjectsAlignHCenter",
	"/ui/MenuBar/ObjectsMenu/ObjectsAlignVertMenu/ObjectsAlignTop",
	"/ui/MenuBar/ObjectsMenu/ObjectsAlignVertMenu/ObjectsAlignBottom",
	"/ui/MenuBar/ObjectsMenu/ObjectsAlignVertMenu/ObjectsAlignVCenter",

	NULL
};


/*****************************************************************************/
/* Initialize UI component for given window.                                 */
/*****************************************************************************/
GtkUIManager *
gl_ui_new (GtkWindow *window)
{
	GtkUIManager            *ui;
	GtkActionGroup          *actions;
	GError                  *error = NULL;

	gl_debug (DEBUG_UI, "START");

	g_return_val_if_fail (window && GTK_IS_WINDOW (window), NULL);

	gl_debug (DEBUG_UI, "window = %p", window);

	ui = gtk_ui_manager_new ();

	actions = gtk_action_group_new ("Actions");
	gtk_action_group_add_actions (actions, entries, n_entries, window);
	gtk_action_group_add_toggle_actions (actions, 
					     toggle_entries, n_toggle_entries, 
					     window);
	gtk_action_group_add_toggle_actions (actions, 
					     ui_toggle_entries, n_ui_toggle_entries, 
					     ui);

	gtk_ui_manager_insert_action_group (ui, actions, 0);
	gtk_window_add_accel_group (window, gtk_ui_manager_get_accel_group (ui));

	if (!gtk_ui_manager_add_ui_from_string (ui, ui_info, strlen (ui_info), &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}

	/* Set the toolbar styles according to prefs */
	set_app_main_toolbar_style (ui);
	set_app_drawing_toolbar_style (ui);
		
	/* Set view grid and markup visibility according to prefs */
	set_view_style (ui);
		
	gl_ui_util_set_verb_list_sensitive (ui, doc_verbs, FALSE);

	gl_debug (DEBUG_UI, "END");

	return ui;
}

/*****************************************************************************/
/* Unref wrapper.                                                            */
/*****************************************************************************/
void
gl_ui_unref (GtkUIManager *ui)
{
	gl_debug (DEBUG_UI, "START");

	g_object_unref(ui);

	gl_debug (DEBUG_UI, "END");
}

/*****************************************************************************/
/* Update all verbs of given UI component.                                   */
/*****************************************************************************/
void
gl_ui_update_all (GtkUIManager *ui,
		  glView       *view)
{
	glLabel *label;

	gl_debug (DEBUG_UI, "START");

	gl_ui_util_set_verb_list_sensitive (ui, doc_verbs, TRUE);

	label = view->label;
	g_return_if_fail (label != NULL);

	gl_ui_util_set_verb_sensitive (ui, "/ui/MenuBar/EditMenu/EditUndo",
				       gl_label_can_undo (label));
	gl_ui_util_set_verb_sensitive (ui, "/ui/MenuBar/EditMenu/EditRedo",
				       gl_label_can_redo (label));

	gl_ui_util_set_verb_list_sensitive (ui, doc_modified_verbs,
					    gl_label_is_modified (label));

	gl_ui_util_set_verb_sensitive (ui, "/ui/MenuBar/ViewMenu/ViewZoomIn",
				       !gl_view_is_zoom_max (view));
	gl_ui_util_set_verb_sensitive (ui, "/ui/MenuBar/ViewMenu/ViewZoomOut",
				       !gl_view_is_zoom_min (view));

	gl_ui_util_set_verb_list_sensitive (ui, selection_verbs,
					    !gl_view_is_selection_empty (view));

	gl_ui_util_set_verb_list_sensitive (ui, atomic_selection_verbs,
					    gl_view_is_selection_atomic (view));

	gl_ui_util_set_verb_list_sensitive (ui, multi_selection_verbs,
					    !gl_view_is_selection_empty (view)
					    && !gl_view_is_selection_atomic (view));

	gl_debug (DEBUG_UI, "END");
}

/*****************************************************************************/
/* Update all verbs of given UI component to "no document" state.            */
/*****************************************************************************/
void
gl_ui_update_nodoc (GtkUIManager *ui)
{
	gl_debug (DEBUG_UI, "START");

	gl_ui_util_set_verb_list_sensitive (ui, doc_verbs, FALSE);

	gl_debug (DEBUG_UI, "END");
}

/*****************************************************************************/
/* Update label modified verbs of given UI component.                        */
/*****************************************************************************/
void
gl_ui_update_modified_verbs (GtkUIManager *ui,
			     glLabel      *label)
{
	gl_debug (DEBUG_UI, "START");

	gl_ui_util_set_verb_list_sensitive (ui, 
					    doc_modified_verbs,
					    gl_label_is_modified (label));

	gl_debug (DEBUG_UI, "END");
}

/*****************************************************************************/
/* Update verbs associated with selection state of given UI component.       */
/*****************************************************************************/
void
gl_ui_update_selection_verbs (GtkUIManager *ui,
			      glView       *view)
{
	gl_debug (DEBUG_UI, "START");

	gl_ui_util_set_verb_list_sensitive (ui, selection_verbs,
					    !gl_view_is_selection_empty (view));

	gl_ui_util_set_verb_list_sensitive (ui, atomic_selection_verbs,
					    gl_view_is_selection_atomic (view));

	gl_ui_util_set_verb_list_sensitive (ui, multi_selection_verbs,
					    !gl_view_is_selection_empty (view)
					    && !gl_view_is_selection_atomic (view));

	gl_debug (DEBUG_UI, "END");
}

/*****************************************************************************/
/* Update verbs associated with zoom level of given UI component.            */
/*****************************************************************************/
void
gl_ui_update_zoom_verbs (GtkUIManager *ui,
			 glView       *view)
{
	gl_debug (DEBUG_UI, "START");

	gl_ui_util_set_verb_sensitive (ui, "/ui/MenuBar/ViewMenu/ViewZoomIn",
				       !gl_view_is_zoom_max (view));
	gl_ui_util_set_verb_sensitive (ui, "/ui/MenuBar/ViewMenu/ViewZoomOut",
				       !gl_view_is_zoom_min (view));

	gl_debug (DEBUG_UI, "END");
}

/*****************************************************************************/
/* Update undo/redo verbs of given UI component.                             */
/*****************************************************************************/
void
gl_ui_update_undo_redo_verbs (GtkUIManager *ui,
			      glLabel      *label)
{
	gl_debug (DEBUG_UI, "START");

	gl_ui_util_set_verb_sensitive (ui, "/ui/MenuBar/EditMenu/EditUndo",
				       gl_label_can_undo (label));

	gl_ui_util_set_verb_sensitive (ui, "/ui/MenuBar/EditMenu/EditRedo",
				       gl_label_can_redo (label));

	gl_debug (DEBUG_UI, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  View menu item toggled callback.                                */
/*---------------------------------------------------------------------------*/
static void
view_ui_item_toggled_cb (GtkToggleAction *action,
			 GtkUIManager    *ui)
{
	const gchar *name;
	gboolean     state;

	gl_debug (DEBUG_UI, "START");

	g_return_if_fail (action && GTK_IS_TOGGLE_ACTION (action));

	name  = gtk_action_get_name (GTK_ACTION (action));
	state = gtk_toggle_action_get_active (action);

	gl_debug (DEBUG_UI, "Action = %s, State = %d", name, state);

	if (strcmp (name, "ViewMainToolBar") == 0)
	{
		gl_prefs->main_toolbar_visible = state;
		set_app_main_toolbar_style (ui);
		gl_prefs_model_save_settings (gl_prefs);
	}

	if (strcmp (name, "ViewMainToolBarToolTips") == 0)
	{
		gl_prefs->main_toolbar_view_tooltips = state;
		set_app_main_toolbar_style (ui);
		gl_prefs_model_save_settings (gl_prefs);
	}

	if (strcmp (name, "ViewDrawingToolBar") == 0)
	{
		gl_prefs->drawing_toolbar_visible = state;
		set_app_drawing_toolbar_style (ui);
		gl_prefs_model_save_settings (gl_prefs);
	}

	if (strcmp (name, "ViewDrawingToolBarToolTips") == 0)
	{
		gl_prefs->drawing_toolbar_view_tooltips = state;
		set_app_drawing_toolbar_style (ui);
		gl_prefs_model_save_settings (gl_prefs);
	}

	gl_debug (DEBUG_UI, "");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set main toolbar style.                                         */
/*---------------------------------------------------------------------------*/
static void
set_app_main_toolbar_style (GtkUIManager *ui)
{
	GtkWidget *toolbar;

	gl_debug (DEBUG_UI, "START");

	g_return_if_fail (ui && GTK_IS_UI_MANAGER (ui));
			
	/* Updated view menu */
	gl_ui_util_set_verb_state (ui, "/ui/ViewMenu/ViewMainToolBar",
				   gl_prefs->main_toolbar_visible);

	gl_ui_util_set_verb_sensitive (ui, "/ui/ViewMenu/ViewMainToolBarToolTips",
				       gl_prefs->main_toolbar_visible);

	gl_ui_util_set_verb_state (ui, "/ui/ViewMenu/ViewMainToolBarToolTips",
				   gl_prefs->main_toolbar_view_tooltips);

	
	toolbar = gtk_ui_manager_get_widget (ui, "/MainToolBar");

	gtk_toolbar_set_tooltips (GTK_TOOLBAR (toolbar),
				  gl_prefs->main_toolbar_view_tooltips);

	if (gl_prefs->main_toolbar_visible) {
		gtk_widget_show_all (toolbar);
	} else {
		gtk_widget_hide (toolbar);
	}
	
	gl_debug (DEBUG_UI, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set drawing toolbar style.                                      */
/*---------------------------------------------------------------------------*/
static void
set_app_drawing_toolbar_style (GtkUIManager *ui)
{
	GtkWidget *toolbar;

	gl_debug (DEBUG_UI, "START");

	g_return_if_fail (ui && GTK_IS_UI_MANAGER (ui));
			
	/* Updated view menu */
	gl_ui_util_set_verb_state (ui, "/ui/MenuBar/ViewMenu/ViewDrawingToolBar",
				   gl_prefs->drawing_toolbar_visible);

	gl_ui_util_set_verb_sensitive (ui, "/ui/MenuBar/ViewMenu/ViewDrawingToolBarToolTips",
				       gl_prefs->drawing_toolbar_visible);

	gl_ui_util_set_verb_state (ui, "/ui/MenuBar/ViewMenuDrawingToolBarToolTips",
				   gl_prefs->drawing_toolbar_view_tooltips);

	
	toolbar = gtk_ui_manager_get_widget (ui, "/DrawingToolBar");

	gtk_toolbar_set_tooltips (GTK_TOOLBAR (toolbar),
				  gl_prefs->drawing_toolbar_view_tooltips);

	gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);

	if (gl_prefs->drawing_toolbar_visible) {
		gtk_widget_show_all (toolbar);
	} else {
		gtk_widget_hide (toolbar);
	}
	
	gl_debug (DEBUG_UI, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set visibility of grid and markup.                              */
/*---------------------------------------------------------------------------*/
static void
set_view_style (GtkUIManager *ui)
{
	gl_debug (DEBUG_UI, "START");

	g_return_if_fail (ui && GTK_IS_UI_MANAGER(ui));
			
	gl_ui_util_set_verb_state (ui, "/ui/MenuBar/ViewMenu/ViewGrid",
				   gl_prefs->grid_visible);

	gl_ui_util_set_verb_state (ui, "/ui/MenuBar/ViewMenu/ViewMarkup",
				   gl_prefs->markup_visible);

	gl_debug (DEBUG_UI, "END");
}

