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

/*
 * This file is based on gedit-menus.c from gedit2:
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi 
 *
 */
#include <config.h>

#include "gnome-recent-view.h"
#include <gconf/gconf-client.h>

#include "ui.h"
#include "commands.h"
#include "tools.h"
#include "glabels.h"
#include "recent.h" 
#include "file.h"
#include "prefs.h"

/*===========================================*/
/* Public globals                            */
/*===========================================*/
BonoboUIVerb gl_ui_verbs [] = {
	BONOBO_UI_VERB ("FileNew", gl_cmd_file_new),
	BONOBO_UI_VERB ("FileOpen", gl_cmd_file_open),
	BONOBO_UI_VERB ("FileSave", gl_cmd_file_save),
	BONOBO_UI_VERB ("FileSaveAs", gl_cmd_file_save_as),
	BONOBO_UI_VERB ("FilePrint", gl_cmd_file_print),
	BONOBO_UI_VERB ("FileClose", gl_cmd_file_close),
	BONOBO_UI_VERB ("FileCloseAll", gl_cmd_file_close_all),
	BONOBO_UI_VERB ("FileExit", gl_cmd_file_exit),
	BONOBO_UI_VERB ("EditCut", gl_cmd_edit_cut),
	BONOBO_UI_VERB ("EditCopy", gl_cmd_edit_copy),
	BONOBO_UI_VERB ("EditPaste", gl_cmd_edit_paste),
	BONOBO_UI_VERB ("EditDelete", gl_cmd_edit_delete),
	BONOBO_UI_VERB ("EditSelectAll", gl_cmd_edit_select_all),
	BONOBO_UI_VERB ("EditUnSelectAll", gl_cmd_edit_unselect_all),
	BONOBO_UI_VERB ("ToolsArrow", gl_tools_arrow),
	BONOBO_UI_VERB ("ToolsText", gl_tools_text),
	BONOBO_UI_VERB ("ToolsBox", gl_tools_box),
	BONOBO_UI_VERB ("ToolsLine", gl_tools_line),
	BONOBO_UI_VERB ("ToolsEllipse", gl_tools_ellipse),
	BONOBO_UI_VERB ("ToolsImage", gl_tools_image),
	BONOBO_UI_VERB ("ToolsBarcode", gl_tools_barcode),
	BONOBO_UI_VERB ("ToolsZoomIn", gl_tools_zoomin),
	BONOBO_UI_VERB ("ToolsZoomOut", gl_tools_zoomout),
	BONOBO_UI_VERB ("ToolsZoom1to1", gl_tools_zoom1to1),
	BONOBO_UI_VERB ("ToolsMergeProperties", gl_tools_merge_properties),
	BONOBO_UI_VERB ("ToolsObjectProperties", gl_tools_object_properties),
	BONOBO_UI_VERB ("ToolsRaiseObjects", gl_tools_raise_objects),
	BONOBO_UI_VERB ("ToolsLowerObjects", gl_tools_lower_objects),
	BONOBO_UI_VERB ("SettingsPreferences", gl_cmd_settings_preferences),
	BONOBO_UI_VERB ("HelpContents", gl_cmd_help_contents),
	BONOBO_UI_VERB ("About", gl_cmd_help_about),

	BONOBO_UI_VERB_END
};

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/


/*========================================================*/
/* Private types.                                         */
/*========================================================*/


/*===========================================*/
/* Private globals                           */
/*===========================================*/

static gchar* doc_verbs [] = {
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
	"/commands/ToolsMergeProperties",
	"/commands/ToolsObjectProperties",
	"/commands/ToolsRaiseObjects",
	"/commands/ToolsLowerObjects",

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

	NULL
};

static gchar* atomic_selection_verbs [] = {
	"/commands/ToolsObjectProperties",

	NULL
};


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void view_menu_item_toggled_cb (
			BonoboUIComponent           *ui_component,
			const char                  *path,
			Bonobo_UIComponent_EventType type,
			const char                  *state,
			BonoboWindow                *win);

static void set_app_main_toolbar_style 	 (BonoboWindow *win);
static void set_app_drawing_toolbar_style (BonoboWindow *win);

static void
set_verb_sensitive (BonoboUIComponent  *ui_component,
		    gchar              *cname,
		    gboolean            sensitive);
static void
set_verb_list_sensitive (BonoboUIComponent   *ui_component,
			 gchar              **vlist,
			 gboolean             sensitive);
static void
set_verb_state (BonoboUIComponent   *ui_component,
		gchar               *cname,
		gboolean             state);



/*****************************************************************************/
/* Initialize UI component for given window.                                 */
/*****************************************************************************/
void gl_ui_init   (BonoboUIComponent *ui_component,
		   BonoboWindow      *win)
{
        GnomeRecentView *recent_view;
        GnomeRecentModel *recent_model;

	g_return_if_fail (ui_component != NULL);

	/* Set the toolbar style according to prefs */
	set_app_main_toolbar_style (win);
		
	/* Add listener fo the view menu */
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
	set_app_drawing_toolbar_style (win);
		
	/* Add listener fo the view menu */
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


	set_verb_list_sensitive (ui_component, doc_verbs, FALSE);

        /* add a GeditRecentView object */
        recent_model = gl_recent_get_model ();
        recent_view  =
		GNOME_RECENT_VIEW (gnome_recent_view_bonobo_new (ui_component,
								 "/menu/File/Recents"));
        gnome_recent_view_set_model (recent_view, recent_model);
        
        g_signal_connect (G_OBJECT (recent_view), "activate",
                          G_CALLBACK (gl_file_open_recent), win);

}

/*****************************************************************************/
/* Update all verbs of given UI component.                                   */
/*****************************************************************************/
void gl_ui_update_all (BonoboUIComponent *ui_component,
		       glView            *view)
{
	glLabel *label;

	bonobo_ui_component_freeze (ui_component, NULL);

	set_verb_list_sensitive (ui_component, doc_verbs, TRUE);

	label = view->label;
	g_return_if_fail (label != NULL);

	set_verb_sensitive (ui_component,
			    "/commands/EditUndo", gl_label_can_undo (label));

	set_verb_sensitive (ui_component,
			    "/commands/EditRedo", gl_label_can_redo (label));

	set_verb_list_sensitive (ui_component, 
				 doc_modified_verbs,
				 gl_label_is_modified (label));

	set_verb_list_sensitive (ui_component,
				 selection_verbs,
				 !gl_view_is_selection_empty (view));

	set_verb_list_sensitive (ui_component,
				 atomic_selection_verbs,
				 gl_view_is_selection_atomic (view));

	bonobo_ui_component_thaw (ui_component, NULL);
}

/*****************************************************************************/
/* Update all verbs of given UI component to "no document" state.            */
/*****************************************************************************/
void gl_ui_update_nodoc           (BonoboUIComponent *ui_component)
{
	bonobo_ui_component_freeze (ui_component, NULL);
	
	set_verb_list_sensitive (ui_component, doc_verbs, FALSE);

	bonobo_ui_component_thaw (ui_component, NULL);
}

/*****************************************************************************/
/* Update undo/redo verbs of given UI component.                             */
/*****************************************************************************/
void gl_ui_update_undo_redo_verbs (BonoboUIComponent *ui_component,
				   glLabel           *label)
{
	bonobo_ui_component_freeze (ui_component, NULL);

	set_verb_sensitive (ui_component,
			    "/commands/EditUndo", gl_label_can_undo (label));

	set_verb_sensitive (ui_component,
			    "/commands/EditRedo", gl_label_can_redo (label));

	bonobo_ui_component_thaw (ui_component, NULL);
}

/*****************************************************************************/
/* Update verbs associated with selection state of given UI component.       */
/*****************************************************************************/
void gl_ui_update_selection_verbs (BonoboUIComponent *ui_component,
				   glView            *view)
{
	bonobo_ui_component_freeze (ui_component, NULL);

	set_verb_list_sensitive (ui_component,
				 selection_verbs,
				 !gl_view_is_selection_empty (view));

	set_verb_list_sensitive (ui_component,
				 atomic_selection_verbs,
				 gl_view_is_selection_atomic (view));

	bonobo_ui_component_thaw (ui_component, NULL);
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

	s = (strcmp (state, "1") == 0);

	if ((strcmp (path, "ViewMainToolbar") == 0) &&
	    (s != gl_prefs->main_toolbar_visible))
	{
		gl_prefs->main_toolbar_visible = s;
		set_app_main_toolbar_style (win);

		return;
	}

	if (s && (strcmp (path, "MainToolbarSystem") == 0) &&
	    (gl_prefs->main_toolbar_buttons_style != GL_TOOLBAR_SYSTEM))
	{		
		gl_prefs->main_toolbar_buttons_style = GL_TOOLBAR_SYSTEM;
		set_app_main_toolbar_style (win);

		return;
	}

	if (s && (strcmp (path, "MainToolbarIcon") == 0) &&
	    (gl_prefs->main_toolbar_buttons_style != GL_TOOLBAR_ICONS))
	{		
		gl_prefs->main_toolbar_buttons_style = GL_TOOLBAR_ICONS;
		set_app_main_toolbar_style (win);

		return;
	}

	if (s && (strcmp (path, "MainToolbarIconText") == 0) &&
	    (gl_prefs->main_toolbar_buttons_style != GL_TOOLBAR_ICONS_AND_TEXT))
	{		
		gl_prefs->main_toolbar_buttons_style = GL_TOOLBAR_ICONS_AND_TEXT;
		set_app_main_toolbar_style (win);

		return;
	}

	if ((strcmp (path, "MainToolbarTooltips") == 0) &&
	    (s != gl_prefs->main_toolbar_view_tooltips))
	{
		gl_prefs->main_toolbar_view_tooltips = s;
		set_app_main_toolbar_style (win);

		return;
	}

	if ((strcmp (path, "ViewDrawingToolbar") == 0) &&
	    (s != gl_prefs->drawing_toolbar_visible))
	{
		gl_prefs->drawing_toolbar_visible = s;
		set_app_drawing_toolbar_style (win);

		return;
	}

	if (s && (strcmp (path, "DrawingToolbarSystem") == 0) &&
	    (gl_prefs->drawing_toolbar_buttons_style != GL_TOOLBAR_SYSTEM))
	{		
		gl_prefs->drawing_toolbar_buttons_style = GL_TOOLBAR_SYSTEM;
		set_app_drawing_toolbar_style (win);

		return;
	}

	if (s && (strcmp (path, "DrawingToolbarIcon") == 0) &&
	    (gl_prefs->drawing_toolbar_buttons_style != GL_TOOLBAR_ICONS))
	{		
		gl_prefs->drawing_toolbar_buttons_style = GL_TOOLBAR_ICONS;
		set_app_drawing_toolbar_style (win);

		return;
	}

	if (s && (strcmp (path, "DrawingToolbarIconText") == 0) &&
	    (gl_prefs->drawing_toolbar_buttons_style != GL_TOOLBAR_ICONS_AND_TEXT))
	{		
		gl_prefs->drawing_toolbar_buttons_style = GL_TOOLBAR_ICONS_AND_TEXT;
		set_app_drawing_toolbar_style (win);

		return;
	}

	if ((strcmp (path, "DrawingToolbarTooltips") == 0) &&
	    (s != gl_prefs->drawing_toolbar_view_tooltips))
	{
		gl_prefs->drawing_toolbar_view_tooltips = s;
		set_app_drawing_toolbar_style (win);

		return;
	}

}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set main toolbar style.                                         */
/*---------------------------------------------------------------------------*/
static void
set_app_main_toolbar_style (BonoboWindow *win)
{
	BonoboUIComponent *ui_component;
	GConfClient *client;
	gboolean labels;

	g_return_if_fail (BONOBO_IS_WINDOW (win));
			
	ui_component = bonobo_mdi_get_ui_component_from_window (win);
	g_return_if_fail (ui_component != NULL);
			
	bonobo_ui_component_freeze (ui_component, NULL);

	/* Updated view menu */
	set_verb_state (ui_component, 
			"/commands/ViewMainToolbar",
			gl_prefs->main_toolbar_visible);

	set_verb_sensitive (ui_component, 
			    "/commands/MainToolbarSystem",
			    gl_prefs->main_toolbar_visible);
	set_verb_sensitive (ui_component, 
			    "/commands/MainToolbarIcon",
			    gl_prefs->main_toolbar_visible);
	set_verb_sensitive (ui_component, 
			    "/commands/MainToolbarIconText",
			    gl_prefs->main_toolbar_visible);
	set_verb_sensitive (ui_component, 
			    "/commands/MainToolbarTooltips",
			    gl_prefs->main_toolbar_visible);

	set_verb_state (ui_component, 
			"/commands/MainToolbarSystem",
			gl_prefs->main_toolbar_buttons_style == GL_TOOLBAR_SYSTEM);

	set_verb_state (ui_component, 
			"/commands/MainToolbarIcon",
			gl_prefs->main_toolbar_buttons_style == GL_TOOLBAR_ICONS);

	set_verb_state (ui_component, 
			"/commands/MainToolbarIconText",
			gl_prefs->main_toolbar_buttons_style == GL_TOOLBAR_ICONS_AND_TEXT);

	set_verb_state (ui_component, 
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

}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set drawing toolbar style.                                      */
/*---------------------------------------------------------------------------*/
static void
set_app_drawing_toolbar_style (BonoboWindow *win)
{
	BonoboUIComponent *ui_component;
	GConfClient *client;
	gboolean labels;

	
	g_return_if_fail (BONOBO_IS_WINDOW (win));
			
	ui_component = bonobo_mdi_get_ui_component_from_window (win);
	g_return_if_fail (ui_component != NULL);
			
	bonobo_ui_component_freeze (ui_component, NULL);

	/* Updated view menu */
	set_verb_state (ui_component, 
			"/commands/ViewDrawingToolbar",
			gl_prefs->drawing_toolbar_visible);

	set_verb_sensitive (ui_component, 
			    "/commands/DrawingToolbarSystem",
			    gl_prefs->drawing_toolbar_visible);
	set_verb_sensitive (ui_component, 
			    "/commands/DrawingToolbarIcon",
			    gl_prefs->drawing_toolbar_visible);
	set_verb_sensitive (ui_component, 
			    "/commands/DrawingToolbarIconText",
			    gl_prefs->drawing_toolbar_visible);
	set_verb_sensitive (ui_component, 
			    "/commands/DrawingToolbarTooltips",
			    gl_prefs->drawing_toolbar_visible);

	set_verb_state (
		ui_component, 
		"/commands/DrawingToolbarSystem",
		gl_prefs->drawing_toolbar_buttons_style == GL_TOOLBAR_SYSTEM);

	set_verb_state (
		ui_component, 
		"/commands/DrawingToolbarIcon",
		gl_prefs->drawing_toolbar_buttons_style == GL_TOOLBAR_ICONS);

	set_verb_state (
		ui_component, 
		"/commands/DrawingToolbarIconText",
		gl_prefs->drawing_toolbar_buttons_style == GL_TOOLBAR_ICONS_AND_TEXT);

	set_verb_state (ui_component, 
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

}


/*---------------------------------------------------------------------------*/
/* Set sensitivity of verb.                                                  */
/*---------------------------------------------------------------------------*/
static void
set_verb_sensitive (BonoboUIComponent  *ui_component,
		    gchar              *cname,
		    gboolean            sensitive)
{
	g_return_if_fail (cname != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (ui_component));

	bonobo_ui_component_set_prop (ui_component,
				      cname,
				      "sensitive",
				      sensitive ? "1" : "0",
				      NULL);
}

/*---------------------------------------------------------------------------*/
/* Set sensitivity of a list of verbs.                                       */
/*---------------------------------------------------------------------------*/
static void
set_verb_list_sensitive (BonoboUIComponent   *ui_component,
			 gchar              **vlist,
			 gboolean             sensitive)
{
	g_return_if_fail (vlist != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (ui_component));

	for ( ; *vlist; ++vlist)
	{
		bonobo_ui_component_set_prop (ui_component,
					      *vlist,
					      "sensitive",
					      sensitive ? "1" : "0",
					      NULL);
	}
}

/*---------------------------------------------------------------------------*/
/* Set state of a verb.                                                      */
/*---------------------------------------------------------------------------*/
static void
set_verb_state (BonoboUIComponent   *ui_component,
		gchar               *cname,
		gboolean             state)
{
	g_return_if_fail (cname != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (ui_component));

	bonobo_ui_component_set_prop (ui_component,
				      cname,
				      "state",
				      state ? "1" : "0",
				      NULL);
}

