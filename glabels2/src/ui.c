/*
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

#include "recent-files/egg-recent-view.h"
#include "recent-files/egg-recent-view-bonobo.h"
#include <gconf/gconf-client.h>

#include "ui.h"
#include "ui-util.h"
#include "commands.h"
#include "tools.h"
#include "recent.h" 
#include "file.h"
#include "prefs.h"

#include "debug.h"

/*==========================================================================*/
/* Private macros and constants.                                            */
/*==========================================================================*/
#define GLABELS_UI_XML GLABELS_UI_DIR "glabels-ui.xml"

/*==========================================================================*/
/* Private types.                                                           */
/*==========================================================================*/


/*==========================================================================*/
/* Private globals                                                          */
/*==========================================================================*/

static BonoboUIVerb gl_ui_verbs [] = {
	BONOBO_UI_VERB ("FileNew",               gl_cmd_file_new),
	BONOBO_UI_VERB ("FileProperties",        gl_cmd_file_properties),
	BONOBO_UI_VERB ("FileOpen",              gl_cmd_file_open),
	BONOBO_UI_VERB ("FileSave",              gl_cmd_file_save),
	BONOBO_UI_VERB ("FileSaveAs",            gl_cmd_file_save_as),
	BONOBO_UI_VERB ("FilePrint",             gl_cmd_file_print),
	BONOBO_UI_VERB ("FileClose",             gl_cmd_file_close),
	BONOBO_UI_VERB ("FileExit",              gl_cmd_file_exit),
	BONOBO_UI_VERB ("EditCut",               gl_cmd_edit_cut),
	BONOBO_UI_VERB ("EditCopy",              gl_cmd_edit_copy),
	BONOBO_UI_VERB ("EditPaste",             gl_cmd_edit_paste),
	BONOBO_UI_VERB ("EditDelete",            gl_cmd_edit_delete),
	BONOBO_UI_VERB ("EditSelectAll",         gl_cmd_edit_select_all),
	BONOBO_UI_VERB ("EditUnSelectAll",       gl_cmd_edit_unselect_all),
	BONOBO_UI_VERB ("ToolsArrow",            gl_tools_arrow),
	BONOBO_UI_VERB ("ToolsText",             gl_tools_text),
	BONOBO_UI_VERB ("ToolsBox",              gl_tools_box),
	BONOBO_UI_VERB ("ToolsLine",             gl_tools_line),
	BONOBO_UI_VERB ("ToolsEllipse",          gl_tools_ellipse),
	BONOBO_UI_VERB ("ToolsImage",            gl_tools_image),
	BONOBO_UI_VERB ("ToolsBarcode",          gl_tools_barcode),
	BONOBO_UI_VERB ("ToolsZoomIn",           gl_tools_zoomin),
	BONOBO_UI_VERB ("ToolsZoomOut",          gl_tools_zoomout),
	BONOBO_UI_VERB ("ToolsZoom1to1",         gl_tools_zoom1to1),
	BONOBO_UI_VERB ("ToolsZoomToFit",        gl_tools_zoom_to_fit),
	BONOBO_UI_VERB ("ToolsMergeProperties",  gl_tools_merge_properties),
	BONOBO_UI_VERB ("ToolsRaiseObjects",     gl_tools_raise_objects),
	BONOBO_UI_VERB ("ToolsLowerObjects",     gl_tools_lower_objects),
	BONOBO_UI_VERB ("ToolsRotateLeft",       gl_tools_rotate_objects_left),
	BONOBO_UI_VERB ("ToolsRotateRight",      gl_tools_rotate_objects_right),
	BONOBO_UI_VERB ("ToolsFlipHorizontal",   gl_tools_flip_objects_horiz),
	BONOBO_UI_VERB ("ToolsFlipVertical",     gl_tools_flip_objects_vert),
	BONOBO_UI_VERB ("ToolsAlignLeft",        gl_tools_align_objects_left),
	BONOBO_UI_VERB ("ToolsAlignRight",       gl_tools_align_objects_right),
	BONOBO_UI_VERB ("ToolsAlignHCenter",     gl_tools_align_objects_hcenter),
	BONOBO_UI_VERB ("ToolsAlignTop",         gl_tools_align_objects_top),
	BONOBO_UI_VERB ("ToolsAlignBottom",      gl_tools_align_objects_bottom),
	BONOBO_UI_VERB ("ToolsAlignVCenter",     gl_tools_align_objects_vcenter),
	BONOBO_UI_VERB ("ToolsCenterHorizontal", gl_tools_center_objects_horiz),
	BONOBO_UI_VERB ("ToolsCenterVertical",   gl_tools_center_objects_vert),
	BONOBO_UI_VERB ("SettingsPreferences",   gl_cmd_settings_preferences),
	BONOBO_UI_VERB ("HelpContents",          gl_cmd_help_contents),
	BONOBO_UI_VERB ("About",                 gl_cmd_help_about),

	BONOBO_UI_VERB_END
};

static gchar* doc_verbs [] = {
	"/commands/FileProperties",
	"/commands/FileSave",
	"/commands/FileSaveAs",
	"/commands/FilePrint",
	"/commands/FilePrintPreview",
	"/commands/FileClose",
	"/commands/FileCloseAll",
	"/commands/EditUndo",
	"/commands/EditRedo",
	"/commands/EditCut",
	"/commands/EditCopy",
	"/commands/EditPaste",
	"/commands/EditDelete",
	"/commands/EditSelectAll",
	"/commands/EditUnSelectAll",
	"/commands/ToolsArrow",
	"/commands/ToolsText",
	"/commands/ToolsLine",
	"/commands/ToolsBox",
	"/commands/ToolsEllipse",
	"/commands/ToolsImage",
	"/commands/ToolsBarcode",
	"/commands/ToolsZoomIn",
	"/commands/ToolsZoomOut",
	"/commands/ToolsZoom1to1",
	"/commands/ToolsZoomToFit",
	"/commands/ToolsMergeProperties",
	"/commands/ToolsRaiseObjects",
	"/commands/ToolsLowerObjects",
	"/commands/ToolsRotateLeft",
	"/commands/ToolsRotateRight",
	"/commands/ToolsFlipHorizontal",
	"/commands/ToolsFlipVertical",
	"/commands/ToolsAlignLeft",
	"/commands/ToolsAlignRight",
	"/commands/ToolsAlignHCenter",
	"/commands/ToolsAlignTop",
	"/commands/ToolsAlignBottom",
	"/commands/ToolsAlignVCenter",
	"/commands/ToolsCenterHorizontal",
	"/commands/ToolsCenterVertical",
	"/menu/Objects/CreateObjects",
	"/menu/Objects/Order",
	"/menu/Objects/RotateFlip",
	"/menu/Objects/AlignHoriz",
	"/menu/Objects/AlignVert",
	"/commands/ViewGrid",
	"/commands/ViewMarkup",

	NULL
};

static gchar* doc_modified_verbs [] = {
	"/commands/FileSave",

	NULL
};

static gchar* selection_verbs [] = {
	"/commands/EditCut",
	"/commands/EditCopy",
	"/commands/EditDelete",
	"/commands/EditUnSelectAll",
	"/commands/ToolsRaiseObjects",
	"/commands/ToolsLowerObjects",
	"/commands/ToolsRotateLeft",
	"/commands/ToolsRotateRight",
	"/commands/ToolsFlipHorizontal",
	"/commands/ToolsFlipVertical",
	"/commands/ToolsCenterHorizontal",
	"/commands/ToolsCenterVertical",
	"/menu/Objects/Order",
	"/menu/Objects/RotateFlip",
	"/menu/Objects/AlignHoriz",
	"/menu/Objects/AlignVert",

	NULL
};

static gchar* atomic_selection_verbs [] = {

	NULL
};

static gchar* multi_selection_verbs [] = {
	"/commands/ToolsAlignLeft",
	"/commands/ToolsAlignRight",
	"/commands/ToolsAlignHCenter",
	"/commands/ToolsAlignTop",
	"/commands/ToolsAlignBottom",
	"/commands/ToolsAlignVCenter",

	NULL
};


/*==========================================================================*/
/* Local function prototypes                                                */
/*==========================================================================*/

static void view_menu_item_toggled_cb     (BonoboUIComponent           *ui_component,
					   const char                  *path,
					   Bonobo_UIComponent_EventType type,
					   const char                  *state,
					   BonoboWindow                *win);

static void set_app_main_toolbar_style 	  (BonoboUIComponent           *ui_component);

static void set_app_drawing_toolbar_style (BonoboUIComponent           *ui_component);

static void set_view_style                (BonoboUIComponent           *ui_component);



/*****************************************************************************/
/* Initialize UI component for given window.                                 */
/*****************************************************************************/
void
gl_ui_init (BonoboUIComponent *ui_component,
	    BonoboWindow      *win,
	    GtkWidget         *cursor_info_frame,
	    GtkWidget         *zoom_info_frame)
{
        EggRecentView	*recent_view;
        EggRecentModel 	*recent_model;
	BonoboControl	*control;

	gl_debug (DEBUG_UI, "START");

	gl_debug (DEBUG_UI, "window = %p", win);

	g_return_if_fail (ui_component != NULL);

	bonobo_ui_engine_config_set_path (bonobo_window_get_ui_engine (win),
					  "/glabels/UIConfig/kvps");
	gl_debug (DEBUG_UI, "Path set");

	bonobo_ui_util_set_ui (ui_component,
			       "", GLABELS_UI_XML, "gLabels", NULL);
	gl_debug (DEBUG_UI, "UI set");

	bonobo_ui_component_add_verb_list_with_data(ui_component,
						    gl_ui_verbs, win);
	gl_debug (DEBUG_UI, "verb list added");

	/* Set the toolbar style according to prefs */
	set_app_main_toolbar_style (ui_component);
		
	/* Add listener for the view menu */
	bonobo_ui_component_add_listener (ui_component, "ViewMainToolbar", 
			(BonoboUIListenerFn)view_menu_item_toggled_cb, 
			(gpointer)win);

	bonobo_ui_component_add_listener (ui_component, "MainToolbarSystem", 
			(BonoboUIListenerFn)view_menu_item_toggled_cb, 
			(gpointer)win);
	bonobo_ui_component_add_listener (ui_component, "MainToolbarIcon", 
			(BonoboUIListenerFn)view_menu_item_toggled_cb, 
			(gpointer)win);
	bonobo_ui_component_add_listener (ui_component, "MainToolbarIconText", 
			(BonoboUIListenerFn)view_menu_item_toggled_cb, 
			(gpointer)win);
	bonobo_ui_component_add_listener (ui_component, "MainToolbarTooltips", 
			(BonoboUIListenerFn)view_menu_item_toggled_cb, 
			(gpointer)win);

	/* Set the toolbar style according to prefs */
	set_app_drawing_toolbar_style (ui_component);
		
	/* Add listener for the view menu */
	bonobo_ui_component_add_listener (ui_component, "ViewDrawingToolbar", 
			(BonoboUIListenerFn)view_menu_item_toggled_cb, 
			(gpointer)win);

	bonobo_ui_component_add_listener (ui_component, "DrawingToolbarSystem", 
			(BonoboUIListenerFn)view_menu_item_toggled_cb, 
			(gpointer)win);
	bonobo_ui_component_add_listener (ui_component, "DrawingToolbarIcon", 
			(BonoboUIListenerFn)view_menu_item_toggled_cb, 
			(gpointer)win);
	bonobo_ui_component_add_listener (ui_component, "DrawingToolbarIconText", 
			(BonoboUIListenerFn)view_menu_item_toggled_cb, 
			(gpointer)win);
	bonobo_ui_component_add_listener (ui_component, "DrawingToolbarTooltips", 
			(BonoboUIListenerFn)view_menu_item_toggled_cb, 
			(gpointer)win);


	/* Set view grid and markup visibility according to prefs */
	set_view_style (ui_component);
		
	/* Add listener for the view grid & markup */
	bonobo_ui_component_add_listener (ui_component, "ViewGrid", 
			(BonoboUIListenerFn)view_menu_item_toggled_cb, 
			(gpointer)win);
	bonobo_ui_component_add_listener (ui_component, "ViewMarkup", 
			(BonoboUIListenerFn)view_menu_item_toggled_cb, 
			(gpointer)win);

	gl_ui_util_set_verb_list_sensitive (ui_component, doc_verbs, FALSE);

	/* Status bar */
	gl_debug (DEBUG_UI, "START Setup status bar.");

        bonobo_ui_component_set_prop (ui_component,
				      "/status", "hidden", "0", NULL);

	control = bonobo_control_new (cursor_info_frame);
	bonobo_ui_component_object_set (ui_component,
					"/status/Cursor", BONOBO_OBJREF (control), NULL);
	bonobo_object_unref (BONOBO_OBJECT (control));
        bonobo_ui_component_set_prop (ui_component,
				      "/status/Cursor", "hidden", "0", NULL);

	control = bonobo_control_new (zoom_info_frame);
	bonobo_ui_component_object_set (ui_component,
					"/status/Zoom", BONOBO_OBJREF (control), NULL);
	bonobo_object_unref (BONOBO_OBJECT (control));
        bonobo_ui_component_set_prop (ui_component,
				      "/status/Zoom", "hidden", "0", NULL);

	gl_debug (DEBUG_UI, "END Setup status bar.");


	/* add an eggRecentView object */
        recent_model = gl_recent_get_model ();
        recent_view  =
		EGG_RECENT_VIEW (egg_recent_view_bonobo_new (ui_component,
							     "/menu/File/Recents"));
	egg_recent_view_set_model (recent_view, recent_model);

	g_signal_connect (G_OBJECT (recent_view), "activate",
			  G_CALLBACK (gl_file_open_recent), win);

	/* Squirrel away a copy to be unreferenced in gl_ui_unref() */
	g_object_set_data (G_OBJECT (ui_component), "recent-view", recent_view);

	gl_debug (DEBUG_UI, "END");
}

/*****************************************************************************/
/* Unref wrapper.                                                            */
/*****************************************************************************/
void
gl_ui_unref (BonoboUIComponent *ui_component)
{
	EggRecentView *recent_view;

	/* Pull out recent view to unreference. */
	recent_view = g_object_get_data (G_OBJECT(ui_component), "recent-view");
	if (recent_view) {
		g_object_unref (recent_view);
	}

	bonobo_object_unref(ui_component);
}

/*****************************************************************************/
/* Update all verbs of given UI component.                                   */
/*****************************************************************************/
void
gl_ui_update_all (BonoboUIComponent *ui_component,
		  glView            *view)
{
	glLabel *label;

	gl_debug (DEBUG_UI, "START");

	bonobo_ui_component_freeze (ui_component, NULL);

	gl_ui_util_set_verb_list_sensitive (ui_component, doc_verbs, TRUE);

	label = view->label;
	g_return_if_fail (label != NULL);

	gl_ui_util_set_verb_sensitive (ui_component, "/commands/EditUndo",
				       gl_label_can_undo (label));
	gl_ui_util_set_verb_sensitive (ui_component, "/commands/EditRedo",
				       gl_label_can_redo (label));

	gl_ui_util_set_verb_list_sensitive (ui_component, 
					    doc_modified_verbs,
					    gl_label_is_modified (label));

	gl_ui_util_set_verb_sensitive (ui_component, "/commands/ToolsZoomIn",
				       !gl_view_is_zoom_max (view));
	gl_ui_util_set_verb_sensitive (ui_component, "/commands/ToolsZoomOut",
				       !gl_view_is_zoom_min (view));

	gl_ui_util_set_verb_list_sensitive (ui_component,
					    selection_verbs,
					    !gl_view_is_selection_empty (view));

	gl_ui_util_set_verb_list_sensitive (ui_component,
					    atomic_selection_verbs,
					    gl_view_is_selection_atomic (view));

	gl_ui_util_set_verb_list_sensitive (ui_component,
					    multi_selection_verbs,
					    !gl_view_is_selection_empty (view)
					    && !gl_view_is_selection_atomic (view));

	bonobo_ui_component_thaw (ui_component, NULL);

	gl_debug (DEBUG_UI, "END");
}

/*****************************************************************************/
/* Update all verbs of given UI component to "no document" state.            */
/*****************************************************************************/
void
gl_ui_update_nodoc (BonoboUIComponent *ui_component)
{
	gl_debug (DEBUG_UI, "START");

	bonobo_ui_component_freeze (ui_component, NULL);
	
	gl_ui_util_set_verb_list_sensitive (ui_component, doc_verbs, FALSE);

	bonobo_ui_component_thaw (ui_component, NULL);

	gl_debug (DEBUG_UI, "END");
}

/*****************************************************************************/
/* Update label modified verbs of given UI component.                        */
/*****************************************************************************/
void
gl_ui_update_modified_verbs (BonoboUIComponent *ui_component,
			     glLabel           *label)
{
	gl_debug (DEBUG_UI, "START");

	bonobo_ui_component_freeze (ui_component, NULL);

	gl_ui_util_set_verb_list_sensitive (ui_component, 
					    doc_modified_verbs,
					    gl_label_is_modified (label));

	bonobo_ui_component_thaw (ui_component, NULL);

	gl_debug (DEBUG_UI, "END");
}

/*****************************************************************************/
/* Update verbs associated with selection state of given UI component.       */
/*****************************************************************************/
void
gl_ui_update_selection_verbs (BonoboUIComponent *ui_component,
			      glView            *view)
{
	gl_debug (DEBUG_UI, "START");

	bonobo_ui_component_freeze (ui_component, NULL);

	gl_ui_util_set_verb_list_sensitive (ui_component,
					    selection_verbs,
					    !gl_view_is_selection_empty (view));

	gl_ui_util_set_verb_list_sensitive (ui_component,
					    atomic_selection_verbs,
					    gl_view_is_selection_atomic (view));

	gl_ui_util_set_verb_list_sensitive (ui_component,
					    multi_selection_verbs,
					    !gl_view_is_selection_empty (view)
					    && !gl_view_is_selection_atomic (view));

	bonobo_ui_component_thaw (ui_component, NULL);

	gl_debug (DEBUG_UI, "END");
}

/*****************************************************************************/
/* Update verbs associated with zoom level of given UI component.            */
/*****************************************************************************/
void
gl_ui_update_zoom_verbs (BonoboUIComponent *ui_component,
			 glView            *view)
{
	gl_debug (DEBUG_UI, "START");

	bonobo_ui_component_freeze (ui_component, NULL);

	gl_ui_util_set_verb_sensitive (ui_component, "/commands/ToolsZoomIn",
				       !gl_view_is_zoom_max (view));
	gl_ui_util_set_verb_sensitive (ui_component, "/commands/ToolsZoomOut",
				       !gl_view_is_zoom_min (view));

	bonobo_ui_component_thaw (ui_component, NULL);

	gl_debug (DEBUG_UI, "END");
}

/*****************************************************************************/
/* Update undo/redo verbs of given UI component.                             */
/*****************************************************************************/
void
gl_ui_update_undo_redo_verbs (BonoboUIComponent *ui_component,
			      glLabel           *label)
{
	gl_debug (DEBUG_UI, "START");

	bonobo_ui_component_freeze (ui_component, NULL);

	gl_ui_util_set_verb_sensitive (ui_component,
				       "/commands/EditUndo",
				       gl_label_can_undo (label));

	gl_ui_util_set_verb_sensitive (ui_component,
				       "/commands/EditRedo",
				       gl_label_can_redo (label));

	bonobo_ui_component_thaw (ui_component, NULL);

	gl_debug (DEBUG_UI, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  View menu item toggled callback.                                */
/*---------------------------------------------------------------------------*/
static void
view_menu_item_toggled_cb (BonoboUIComponent           *ui_component,
			   const char                  *path,
			   Bonobo_UIComponent_EventType type,
			   const char                  *state,
			   BonoboWindow                *win)
{
	gboolean s;

	gl_debug (DEBUG_UI, "");

	s = (strcmp (state, "1") == 0);

	if (strcmp (path, "ViewMainToolbar") == 0)
	{
		gl_prefs->main_toolbar_visible = s;
		set_app_main_toolbar_style (ui_component);
		gl_prefs_model_save_settings (gl_prefs);

		return;
	}

	if (s && (strcmp (path, "MainToolbarSystem") == 0))
	{		
		gl_prefs->main_toolbar_buttons_style = GL_TOOLBAR_SYSTEM;
		set_app_main_toolbar_style (ui_component);
		gl_prefs_model_save_settings (gl_prefs);

		return;
	}

	if (s && (strcmp (path, "MainToolbarIcon") == 0))
	{		
		gl_prefs->main_toolbar_buttons_style = GL_TOOLBAR_ICONS;
		set_app_main_toolbar_style (ui_component);
		gl_prefs_model_save_settings (gl_prefs);

		return;
	}

	if (s && (strcmp (path, "MainToolbarIconText") == 0))
	{		
		gl_prefs->main_toolbar_buttons_style = GL_TOOLBAR_ICONS_AND_TEXT;
		set_app_main_toolbar_style (ui_component);
		gl_prefs_model_save_settings (gl_prefs);

		return;
	}

	if (strcmp (path, "MainToolbarTooltips") == 0)
	{
		gl_prefs->main_toolbar_view_tooltips = s;
		set_app_main_toolbar_style (ui_component);
		gl_prefs_model_save_settings (gl_prefs);

		return;
	}

	if (strcmp (path, "ViewDrawingToolbar") == 0)
	{
		gl_prefs->drawing_toolbar_visible = s;
		set_app_drawing_toolbar_style (ui_component);
		gl_prefs_model_save_settings (gl_prefs);

		return;
	}

	if (s && (strcmp (path, "DrawingToolbarSystem") == 0))
	{		
		gl_prefs->drawing_toolbar_buttons_style = GL_TOOLBAR_SYSTEM;
		set_app_drawing_toolbar_style (ui_component);
		gl_prefs_model_save_settings (gl_prefs);

		return;
	}

	if (s && (strcmp (path, "DrawingToolbarIcon") == 0))
	{		
		gl_prefs->drawing_toolbar_buttons_style = GL_TOOLBAR_ICONS;
		set_app_drawing_toolbar_style (ui_component);
		gl_prefs_model_save_settings (gl_prefs);

		return;
	}

	if (s && (strcmp (path, "DrawingToolbarIconText") == 0))
	{		
		gl_prefs->drawing_toolbar_buttons_style = GL_TOOLBAR_ICONS_AND_TEXT;
		set_app_drawing_toolbar_style (ui_component);
		gl_prefs_model_save_settings (gl_prefs);

		return;
	}

	if (strcmp (path, "DrawingToolbarTooltips") == 0)
	{
		gl_prefs->drawing_toolbar_view_tooltips = s;
		set_app_drawing_toolbar_style (ui_component);
		gl_prefs_model_save_settings (gl_prefs);

		return;
	}

	if (strcmp (path, "ViewGrid") == 0)
	{
		gl_prefs->grid_visible = s;
		if (s) {
			gl_view_show_grid (GL_VIEW(GL_WINDOW(win)->view));
		} else {
			gl_view_hide_grid (GL_VIEW(GL_WINDOW(win)->view));
		}
		gl_prefs_model_save_settings (gl_prefs);

		return;
	}

	if (strcmp (path, "ViewMarkup") == 0)
	{
		gl_prefs->markup_visible = s;
		if (s) {
			gl_view_show_markup (GL_VIEW(GL_WINDOW(win)->view));
		} else {
			gl_view_hide_markup (GL_VIEW(GL_WINDOW(win)->view));
		}
		gl_prefs_model_save_settings (gl_prefs);

		return;
	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set main toolbar style.                                         */
/*---------------------------------------------------------------------------*/
static void
set_app_main_toolbar_style (BonoboUIComponent *ui_component)
{
	GConfClient *client;
	gboolean labels;

	gl_debug (DEBUG_UI, "START");

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (ui_component));
			
	bonobo_ui_component_freeze (ui_component, NULL);

	/* Updated view menu */
	gl_ui_util_set_verb_state (ui_component, 
				   "/commands/ViewMainToolbar",
				   gl_prefs->main_toolbar_visible);

	gl_ui_util_set_verb_sensitive (ui_component, 
				       "/commands/MainToolbarSystem",
				       gl_prefs->main_toolbar_visible);
	gl_ui_util_set_verb_sensitive (ui_component, 
				       "/commands/MainToolbarIcon",
				       gl_prefs->main_toolbar_visible);
	gl_ui_util_set_verb_sensitive (ui_component, 
				       "/commands/MainToolbarIconText",
				       gl_prefs->main_toolbar_visible);
	gl_ui_util_set_verb_sensitive (ui_component, 
				       "/commands/MainToolbarTooltips",
				       gl_prefs->main_toolbar_visible);

	gl_ui_util_set_verb_state (ui_component, 
				   "/commands/MainToolbarSystem",
				   gl_prefs->main_toolbar_buttons_style == GL_TOOLBAR_SYSTEM);

	gl_ui_util_set_verb_state (ui_component, 
				   "/commands/MainToolbarIcon",
				   gl_prefs->main_toolbar_buttons_style == GL_TOOLBAR_ICONS);

	gl_ui_util_set_verb_state (ui_component, 
				   "/commands/MainToolbarIconText",
				   gl_prefs->main_toolbar_buttons_style == GL_TOOLBAR_ICONS_AND_TEXT);

	gl_ui_util_set_verb_state (ui_component, 
				   "/commands/MainToolbarTooltips",
				   gl_prefs->main_toolbar_view_tooltips);

	
	/* Actually update main_toolbar style */
	bonobo_ui_component_set_prop (
		ui_component, "/MainToolbar",
		"tips", gl_prefs->main_toolbar_view_tooltips ? "1" : "0",
		NULL);
	
	switch (gl_prefs->main_toolbar_buttons_style)
	{
		case GL_TOOLBAR_SYSTEM:
						
			client = gconf_client_get_default ();
			if (client == NULL) 
				goto error;

			labels = gconf_client_get_bool (client, 
					"/desktop/gnome/interface/toolbar-labels", NULL);

			g_object_unref (G_OBJECT (client));
			
			if (labels)
			{			
				bonobo_ui_component_set_prop (
					ui_component, "/MainToolbar", "look", "both", NULL);
			
			}
			else
			{
				bonobo_ui_component_set_prop (
					ui_component, "/MainToolbar", "look", "icons", NULL);
			}
	
			break;
			
		case GL_TOOLBAR_ICONS:
			bonobo_ui_component_set_prop (
				ui_component, "/MainToolbar", "look", "icon", NULL);
			
			break;
			
		case GL_TOOLBAR_ICONS_AND_TEXT:
			bonobo_ui_component_set_prop (
				ui_component, "/MainToolbar", "look", "both", NULL);
			
			break;
		default:
			goto error;
			break;
	}
	
	bonobo_ui_component_set_prop (
			ui_component, "/MainToolbar",
			"hidden", gl_prefs->main_toolbar_visible ? "0":"1", NULL);

 error:
	bonobo_ui_component_thaw (ui_component, NULL);

	gl_debug (DEBUG_UI, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set drawing toolbar style.                                      */
/*---------------------------------------------------------------------------*/
static void
set_app_drawing_toolbar_style (BonoboUIComponent *ui_component)
{
	GConfClient *client;
	gboolean labels;

	gl_debug (DEBUG_UI, "START");

	g_return_if_fail (BONOBO_IS_UI_COMPONENT(ui_component));
			
	bonobo_ui_component_freeze (ui_component, NULL);

	/* Updated view menu */
	gl_ui_util_set_verb_state (ui_component, 
				   "/commands/ViewDrawingToolbar",
				   gl_prefs->drawing_toolbar_visible);

	gl_ui_util_set_verb_sensitive (ui_component, 
				       "/commands/DrawingToolbarSystem",
				       gl_prefs->drawing_toolbar_visible);
	gl_ui_util_set_verb_sensitive (ui_component, 
				       "/commands/DrawingToolbarIcon",
				       gl_prefs->drawing_toolbar_visible);
	gl_ui_util_set_verb_sensitive (ui_component, 
				       "/commands/DrawingToolbarIconText",
				       gl_prefs->drawing_toolbar_visible);
	gl_ui_util_set_verb_sensitive (ui_component, 
				       "/commands/DrawingToolbarTooltips",
				       gl_prefs->drawing_toolbar_visible);

	gl_ui_util_set_verb_state (
		ui_component, 
		"/commands/DrawingToolbarSystem",
		gl_prefs->drawing_toolbar_buttons_style == GL_TOOLBAR_SYSTEM);

	gl_ui_util_set_verb_state (
		ui_component, 
		"/commands/DrawingToolbarIcon",
		gl_prefs->drawing_toolbar_buttons_style == GL_TOOLBAR_ICONS);

	gl_ui_util_set_verb_state (
		ui_component, 
		"/commands/DrawingToolbarIconText",
		gl_prefs->drawing_toolbar_buttons_style == GL_TOOLBAR_ICONS_AND_TEXT);

	gl_ui_util_set_verb_state (ui_component, 
			"/commands/DrawingToolbarTooltips",
			gl_prefs->drawing_toolbar_view_tooltips);

	
	/* Actually update drawing_toolbar style */
	bonobo_ui_component_set_prop (
		ui_component, "/DrawingToolbar",
		"tips", gl_prefs->drawing_toolbar_view_tooltips ? "1" : "0",
		NULL);
	
	switch (gl_prefs->drawing_toolbar_buttons_style)
	{
		case GL_TOOLBAR_SYSTEM:
						
			client = gconf_client_get_default ();
			if (client == NULL) 
				goto error;

			labels = gconf_client_get_bool (client, 
					"/desktop/gnome/interface/toolbar-labels", NULL);

			g_object_unref (G_OBJECT (client));
			
			if (labels)
			{			
				bonobo_ui_component_set_prop (
					ui_component, "/DrawingToolbar", "look", "both", NULL);
			
			}
			else
			{
				bonobo_ui_component_set_prop (
					ui_component, "/DrawingToolbar", "look", "icons", NULL);
			}
	
			break;
			
		case GL_TOOLBAR_ICONS:
			bonobo_ui_component_set_prop (
				ui_component, "/DrawingToolbar", "look", "icon", NULL);
			
			break;
			
		case GL_TOOLBAR_ICONS_AND_TEXT:
			bonobo_ui_component_set_prop (
				ui_component, "/DrawingToolbar", "look", "both", NULL);
			
			break;
		default:
			goto error;
			break;
	}
	
	bonobo_ui_component_set_prop (
			ui_component, "/DrawingToolbar",
			"hidden", gl_prefs->drawing_toolbar_visible ? "0":"1", NULL);

 error:
	bonobo_ui_component_thaw (ui_component, NULL);

	gl_debug (DEBUG_UI, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set visibility of grid and markup.                              */
/*---------------------------------------------------------------------------*/
static void
set_view_style (BonoboUIComponent *ui_component)
{
	GConfClient *client;
	gboolean labels;

	gl_debug (DEBUG_UI, "START");

	g_return_if_fail (BONOBO_IS_UI_COMPONENT(ui_component));
			
	bonobo_ui_component_freeze (ui_component, NULL);

	gl_ui_util_set_verb_state (ui_component, 
				   "/commands/ViewGrid",
				   gl_prefs->grid_visible);

	gl_ui_util_set_verb_state (ui_component, 
				   "/commands/ViewMarkup",
				   gl_prefs->markup_visible);

 error:
	bonobo_ui_component_thaw (ui_component, NULL);

	gl_debug (DEBUG_UI, "END");
}


