/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  menus.c:  GLabels menus module
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

#include "menus.h"
#include "commands.h"
#include "tools.h"
#include "glabels.h"

BonoboUIVerb gl_verbs [] = {
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

gchar* gl_menus_no_docs_sensible_verbs [] = {
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

gchar* gl_menus_not_modified_doc_sensible_verbs [] = {
	"/commands/FileSave",

	NULL
};


void
gl_menus_set_verb_sensitive (BonoboUIComponent *ui_component, gchar* cname, gboolean sensitive)
{
	g_return_if_fail (cname != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (ui_component));

	bonobo_ui_component_set_prop (
		ui_component, cname, "sensitive", sensitive ? "1" : "0", NULL);
}

void
gl_menus_set_verb_list_sensitive (BonoboUIComponent *ui_component, gchar** vlist, gboolean sensitive)
{
	g_return_if_fail (vlist != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (ui_component));

	for ( ; *vlist; ++vlist)
	{
		bonobo_ui_component_set_prop (
			ui_component, *vlist, "sensitive", sensitive ? "1" : "0", NULL);
	}
}

void
gl_menus_set_verb_state (BonoboUIComponent *ui_component, gchar* cname, gboolean state)
{
	g_return_if_fail (cname != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (ui_component));

	bonobo_ui_component_set_prop (
		ui_component, cname, "state", state ? "1" : "0", NULL);
}

void
gl_menus_add_menu_item (BonoboWindow *window, const gchar *path,
		     const gchar *name, const gchar *label,
		     const gchar *tooltip, const gchar *stock_pixmap,
		     BonoboUIVerbFn cb)
{
	BonoboUIComponent *ui_component;
	gchar *item_path;
	gchar *cmd;

	g_return_if_fail (window != NULL);
	g_return_if_fail (path != NULL);
	g_return_if_fail (label != NULL);
	g_return_if_fail (cb != NULL);
	
	item_path = g_strconcat (path, name, NULL);
	ui_component = bonobo_mdi_get_ui_component_from_window (BONOBO_WINDOW (window));
	if (!bonobo_ui_component_path_exists (ui_component, item_path, NULL)) {
		gchar *xml;

		xml = g_strdup_printf ("<menuitem name=\"%s\" verb=\"\""
				       " _label=\"%s\""
				       " _tip=\"%s\" hident=\"0\" />", name,
				       label, tooltip);


		if (stock_pixmap != NULL) {
			cmd = g_strdup_printf ("<cmd name=\"%s\""
				" pixtype=\"stock\" pixname=\"%s\" />",
				name, stock_pixmap);
		}
		else {
			cmd = g_strdup_printf ("<cmd name=\"%s\" />", name);
		}


		bonobo_ui_component_set_translate (ui_component, path,
						   xml, NULL);

		bonobo_ui_component_set_translate (ui_component, "/commands/",
						   cmd, NULL);
						   
		bonobo_ui_component_add_verb (ui_component, name, cb, NULL);

		g_free (xml);
		g_free (cmd);
	}

	g_free (item_path);
}

void
gl_menus_remove_menu_item (BonoboWindow *window, const gchar *path,
			const gchar *name)
{
	BonoboUIComponent *ui_component;
	gchar *item_path;

	g_return_if_fail (window != NULL);
	g_return_if_fail (path != NULL);
	g_return_if_fail (name != NULL);

	item_path = g_strconcat (path, name, NULL);
	ui_component = bonobo_mdi_get_ui_component_from_window (BONOBO_WINDOW (window));

	if (bonobo_ui_component_path_exists (ui_component, item_path, NULL)) {
		gchar *cmd;

		cmd = g_strdup_printf ("/commands/%s", name);
		
		bonobo_ui_component_rm (ui_component, item_path, NULL);
		bonobo_ui_component_rm (ui_component, cmd, NULL);
		
		g_free (cmd);
	}

	g_free (item_path);
}

void
gl_menus_add_menu_item_all (const gchar *path, const gchar *name,
			 const gchar *label, const gchar *tooltip,
			 const gchar *stock_pixmap,
			 BonoboUIVerbFn cb)
{
	GList* top_windows;
	
	top_windows = glabels_get_top_windows ();
	g_return_if_fail (top_windows != NULL);
       
	while (top_windows)
	{
		BonoboWindow* window = BONOBO_WINDOW (top_windows->data);


		gl_menus_add_menu_item (window, path, name, label, tooltip,
				     stock_pixmap, cb);
		
		top_windows = g_list_next (top_windows);
	}
}

void
gl_menus_remove_menu_item_all (const gchar *path, const gchar *name)
{
	GList* top_windows;
	
	top_windows = glabels_get_top_windows ();
	g_return_if_fail (top_windows != NULL);
       
	while (top_windows)
	{
		BonoboWindow* window = BONOBO_WINDOW (top_windows->data);


		gl_menus_remove_menu_item (window, path, name);

		
		top_windows = g_list_next (top_windows);
	}
}
