/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  menu.c:  Menu module
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

#include "menu.h"
#include "stock.h"
#include "file.h"
#include "print_dialog.h"
#include "edit.h"
#include "prefs.h"
#include "tools.h"

#ifdef PACKAGE_DATA_DIR
#define LOGO_PIXMAP (PACKAGE_DATA_DIR "/pixmaps/glabels/glabels-about-logo.png")
#else
#define LOGO_PIXMAP gnome_pixmap_file("glabels/glabels-about-logo.png")
#endif

/*===========================================*/
/* Private function prototypes.              */
/*===========================================*/
static void about_cb (GtkWidget * widget, gpointer data);
static void nothing_cb (GtkWidget * widget, gpointer data);

/*===========================================*/
/* Menu definitions                          */
/*===========================================*/

static GnomeUIInfo menu_file[] = {
	GNOMEUIINFO_MENU_NEW_ITEM (N_("_New"),
				   N_("Make a new, empty label"),
				   gl_file_new_cb, NULL),

	GNOMEUIINFO_MENU_OPEN_ITEM (gl_file_open_cb, NULL),

	GNOMEUIINFO_MENU_SAVE_ITEM (gl_file_save_cb, NULL),

	GNOMEUIINFO_MENU_SAVE_AS_ITEM (gl_file_save_as_cb, NULL),

	GNOMEUIINFO_SEPARATOR,

	GNOMEUIINFO_MENU_PRINT_ITEM (gl_print_dialog_cb, NULL),

	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_MENU_CLOSE_ITEM (gl_file_close_cb, NULL),
	GNOMEUIINFO_MENU_EXIT_ITEM (gl_file_exit_cb, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo menu_edit[] = {
#ifdef UNDO_REDO
	GNOMEUIINFO_MENU_UNDO_ITEM (nothing_cb, NULL),
	GNOMEUIINFO_MENU_REDO_ITEM (nothing_cb, NULL),

	GNOMEUIINFO_SEPARATOR,
#endif

	GNOMEUIINFO_MENU_CUT_ITEM (gl_edit_cut_cb, NULL),
	GNOMEUIINFO_MENU_COPY_ITEM (gl_edit_copy_cb, NULL),
	GNOMEUIINFO_MENU_PASTE_ITEM (gl_edit_paste_cb, NULL),

	GNOMEUIINFO_SEPARATOR,

	GNOMEUIINFO_MENU_SELECT_ALL_ITEM (gl_edit_select_all_cb, NULL),
	GNOMEUIINFO_ITEM_NONE (N_("U_nselect All "),
			       N_("Remove all selections"),
			       gl_edit_unselect_cb),

	GNOMEUIINFO_END
};

static GnomeUIInfo menu_tools[] = {

	GNOMEUIINFO_ITEM_STOCK (N_("Select"),
				N_("Select, move and modify objects"),
				gl_tools_arrow_cb, GL_STOCK_PIXMAP_ARROW),

	GNOMEUIINFO_SEPARATOR,

	GNOMEUIINFO_ITEM_NONE (N_("Text"), N_("Create text object"),
				gl_tools_text_cb),

	GNOMEUIINFO_ITEM_NONE (N_("Box"), N_("Create box/rectangle object"),
				gl_tools_box_cb),

	GNOMEUIINFO_ITEM_NONE (N_("Line"), N_("Create line object"),
				gl_tools_line_cb),

	GNOMEUIINFO_ITEM_NONE (N_("Ellipse"),
				N_("Create ellipse/circle object"),
				gl_tools_ellipse_cb),

	GNOMEUIINFO_ITEM_NONE (N_("Image"), N_("Create image object"),
				gl_tools_image_cb),

	GNOMEUIINFO_ITEM_NONE (N_("Barcode"), N_("Create barcode object"),
				gl_tools_barcode_cb),

	GNOMEUIINFO_SEPARATOR,

	GNOMEUIINFO_ITEM_NONE (N_("Zoom in"), N_("Zoom in"),
				gl_tools_zoomin_cb),

	GNOMEUIINFO_ITEM_NONE (N_("Zoom out"), N_("Zoom out"),
				gl_tools_zoomout_cb),

	GNOMEUIINFO_ITEM_NONE (N_("Zoom 1:1"), N_("Zoom to 1:1"),
				gl_tools_zoom1to1_cb),

	GNOMEUIINFO_SEPARATOR,

	GNOMEUIINFO_ITEM_NONE (N_("Merge properties"),
				N_("Edit merge properties"),
				gl_tools_merge_properties_cb),

	GNOMEUIINFO_END
};

static GnomeUIInfo menu_settings[] = {
	GNOMEUIINFO_MENU_PREFERENCES_ITEM (gl_prefs_cb, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo menu_windows[] = {
	GNOMEUIINFO_END
};

static GnomeUIInfo menu_help[] = {
	GNOMEUIINFO_HELP ("glabels"),
	GNOMEUIINFO_MENU_ABOUT_ITEM (about_cb, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo menu_main[] = {
	GNOMEUIINFO_MENU_FILE_TREE (menu_file),
	GNOMEUIINFO_MENU_EDIT_TREE (menu_edit),
	GNOMEUIINFO_SUBTREE (N_("_Tools"), menu_tools),
	GNOMEUIINFO_MENU_SETTINGS_TREE (menu_settings),
	GNOMEUIINFO_MENU_WINDOWS_TREE (menu_windows),
	GNOMEUIINFO_MENU_HELP_TREE (menu_help),
	GNOMEUIINFO_END
};

/*===========================================*/
/* Toolbar definitions                       */
/*===========================================*/
static GnomeUIInfo toolbar[] = {
	GNOMEUIINFO_ITEM_STOCK (N_("  New   "), N_("New Label/Card"),
				gl_file_new_cb, GNOME_STOCK_PIXMAP_NEW),
	GNOMEUIINFO_ITEM_STOCK (N_("  Open  "), N_("Open a file"),
				gl_file_open_cb, GNOME_STOCK_PIXMAP_OPEN),
	GNOMEUIINFO_ITEM_STOCK (N_("  Save  "), N_("Save current file"),
				gl_file_save_cb, GNOME_STOCK_PIXMAP_SAVE),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_ITEM_STOCK (N_(" Print  "), N_("Print current file"),
				gl_print_dialog_cb, GNOME_STOCK_PIXMAP_PRINT),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_END
};

/*****************************************************************************/
/* Drawing tools toolbar definitions                                         */
/*****************************************************************************/
static GnomeUIInfo tools_toolbar[] = {

	GNOMEUIINFO_ITEM_STOCK (NULL, N_("Select, move and modify objects"),
				gl_tools_arrow_cb, GL_STOCK_PIXMAP_ARROW),

	GNOMEUIINFO_SEPARATOR,

	GNOMEUIINFO_ITEM_STOCK (NULL, N_("Create text object"),
				gl_tools_text_cb, GL_STOCK_PIXMAP_TEXT),

	GNOMEUIINFO_ITEM_STOCK (NULL, N_("Create box/rectangle object"),
				gl_tools_box_cb, GL_STOCK_PIXMAP_BOX),

	GNOMEUIINFO_ITEM_STOCK (NULL, N_("Create line object"),
				gl_tools_line_cb, GL_STOCK_PIXMAP_LINE),

	GNOMEUIINFO_ITEM_STOCK (NULL, N_("Create ellipse/circle object"),
				gl_tools_ellipse_cb, GL_STOCK_PIXMAP_ELLIPSE),

	GNOMEUIINFO_ITEM_STOCK (NULL, N_("Create image object"),
				gl_tools_image_cb, GL_STOCK_PIXMAP_IMAGE),

	GNOMEUIINFO_ITEM_STOCK (NULL, N_("Create barcode object"),
				gl_tools_barcode_cb, GL_STOCK_PIXMAP_BARCODE),

	GNOMEUIINFO_SEPARATOR,

	GNOMEUIINFO_ITEM_STOCK (NULL, N_("Zoom in"),
				gl_tools_zoomin_cb, GL_STOCK_PIXMAP_ZOOMIN),

	GNOMEUIINFO_ITEM_STOCK (NULL, N_("Zoom out"),
				gl_tools_zoomout_cb, GL_STOCK_PIXMAP_ZOOMOUT),

	GNOMEUIINFO_ITEM_STOCK (NULL, N_("Zoom to 1:1"),
				gl_tools_zoom1to1_cb,
				GL_STOCK_PIXMAP_ZOOM1TO1),

	GNOMEUIINFO_SEPARATOR,

	GNOMEUIINFO_ITEM_STOCK (NULL, N_("Merge properties"),
				gl_tools_merge_properties_cb,
				GL_STOCK_PIXMAP_MERGE),

	GNOMEUIINFO_END
};


/****************************************************************************/
/* Install menus templates in mdi.                                          */
/****************************************************************************/
void
gl_menu_install_menus (GnomeMDI * mdi)
{
	gnome_mdi_set_menubar_template (mdi, menu_main);
	gnome_mdi_set_child_list_path (mdi, GNOME_MENU_WINDOWS_PATH);
}

/****************************************************************************/
/* Install toolbar in application window.                                   */
/****************************************************************************/
void
gl_menu_install_toolbar (GnomeMDI * mdi)
{
	gnome_mdi_set_toolbar_template (mdi, toolbar);
}

/*****************************************************************************/
/* Install drawing tools toolbar in application window.                      */
/*****************************************************************************/
void
gl_menu_install_tools_toolbar (GnomeMDI * mdi,
			       GnomeApp * app)
{
	GtkWidget *toolbar;

	toolbar = gtk_toolbar_new (GTK_ORIENTATION_VERTICAL,
				   GTK_TOOLBAR_ICONS);

	gnome_app_fill_toolbar_with_data (GTK_TOOLBAR (toolbar), tools_toolbar,
					  NULL, mdi);
	gnome_app_add_toolbar (GNOME_APP (app), GTK_TOOLBAR (toolbar),
			       "ToolsToolbar", GNOME_DOCK_ITEM_BEH_NORMAL,
			       GNOME_DOCK_LEFT, 1, 1, 0);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE about menu callback.                                             */
/*--------------------------------------------------------------------------*/
static void
about_cb (GtkWidget * widget,
	  gpointer data)
{
	static GtkWidget *dialog = NULL;
	GnomeApp *app = gnome_mdi_get_active_window (GNOME_MDI (data));

	if (dialog != NULL) {
		g_assert (GTK_WIDGET_REALIZED (dialog));
		gdk_window_show (dialog->window);
		gdk_window_raise (dialog->window);
	} else {
		const gchar *authors[] = {
			"Jim Evins <evins@snaught.com>",
			NULL
		};
		gchar *copy_text = "Copyright 2001 Jim Evins";
		gchar *about_text =
		    _("A label and business card creation program for GNOME.\n"
		      " \n"
		      "Glabels is free software; you can redistribute it and/or modify it "
		      "under the terms of the GNU General Public License as published by "
		      "the Free Software Foundation; either version 2 of the License, or "
		      "(at your option) any later version.\n" " \n"
		      "This program is distributed in the hope that it will be useful, but "
		      "WITHOUT ANY WARRANTY; without even the implied warranty of "
		      "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU "
		      "General Public License for more details.\n");

		dialog = gnome_about_new ("glabels", VERSION,
					  copy_text, authors, about_text,
					  LOGO_PIXMAP);
		gtk_window_set_transient_for (GTK_WINDOW (dialog),
					      GTK_WINDOW (app));

		gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
				    GTK_SIGNAL_FUNC (gtk_widget_destroyed),
				    &dialog);

		gtk_widget_show (dialog);
	}

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  place-holder callback.                                         */
/*--------------------------------------------------------------------------*/
static void
nothing_cb (GtkWidget * widget,
	    gpointer data)
{
	GtkWidget *dialog;
	GnomeApp *app = gnome_mdi_get_active_window (GNOME_MDI (data));

	dialog = gnome_ok_dialog (_("Function is not implemented!"));
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (app));
}
