/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  glabels.c:  GLabels main module
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

#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnomeui/gnome-window-icon.h>

#include "splash.h"
#include "stock.h"
#include "merge.h"
#include "template.h"
#include "prefs.h"
#include "debug.h"
#include "window.h"

#define ICON_PIXMAP gnome_program_locate_file (NULL,\
					       GNOME_FILE_DOMAIN_APP_PIXMAP,\
					       "glabels/glabels-icon.png",\
					       FALSE, NULL)

static const struct poptOption options [] =
{
	{ "debug-view", '\0', POPT_ARG_NONE, &gl_debug_view, 0,
	  N_("Show view debugging messages."), NULL },

	{ "debug-item", '\0', POPT_ARG_NONE, &gl_debug_item, 0,
	  N_("Show item debugging messages."), NULL },

	{ "debug-print", '\0', POPT_ARG_NONE, &gl_debug_print, 0,
	  N_("Show printing debugging messages."), NULL },

	{ "debug-prefs", '\0', POPT_ARG_NONE, &gl_debug_prefs, 0,
	  N_("Show prefs debugging messages."), NULL },

	{ "debug-file", '\0', POPT_ARG_NONE, &gl_debug_file, 0,
	  N_("Show file debugging messages."), NULL },

	{ "debug-label", '\0', POPT_ARG_NONE, &gl_debug_label, 0,
	  N_("Show document debugging messages."), NULL },

	{ "debug-template", '\0', POPT_ARG_NONE, &gl_debug_template, 0,
	  N_("Show template debugging messages."), NULL },

	{ "debug-xml", '\0', POPT_ARG_NONE, &gl_debug_xml, 0,
	  N_("Show xml debugging messages."), NULL },

	{ "debug-merge", '\0', POPT_ARG_NONE, &gl_debug_merge, 0,
	  N_("Show document merge debugging messages."), NULL },

	{ "debug-commands", '\0', POPT_ARG_NONE, &gl_debug_commands, 0,
	  N_("Show commands debugging messages."), NULL },

	{ "debug-undo", '\0', POPT_ARG_NONE, &gl_debug_undo, 0,
	  N_("Show undo debugging messages."), NULL },

	{ "debug-recent", '\0', POPT_ARG_NONE, &gl_debug_recent, 0,
	  N_("Show recent debugging messages."), NULL },

	{ "debug-window", '\0', POPT_ARG_NONE, &gl_debug_window, 0,
	  N_("Show window debugging messages."), NULL },

	{ "debug-ui", '\0', POPT_ARG_NONE, &gl_debug_ui, 0,
	  N_("Show ui debugging messages."), NULL },

	{ "debug-media-select", '\0', POPT_ARG_NONE, &gl_debug_media_select, 0,
	  N_("Show media select widget debugging messages."), NULL },

	{ "debug-mini-preview", '\0', POPT_ARG_NONE, &gl_debug_mini_preview, 0,
	  N_("Show mini preview widget debugging messages."), NULL },

	{ "debug-wdgt", '\0', POPT_ARG_NONE, &gl_debug_wdgt, 0,
	  N_("Show widget debugging messages."), NULL },

	{ "debug", '\0', POPT_ARG_NONE, &gl_debug_all, 0,
	  N_("Turn on all debugging messages."), NULL },

	{NULL, '\0', 0, NULL, 0}
};


int
main (int argc, char **argv)
{
	GValue         value = { 0, };
    	GnomeProgram  *program;
	poptContext    ctx;
	char         **args;
	GList         *file_list = NULL, *p;
	gint           i;
	GtkWidget     *win;

	bindtextdomain (GETTEXT_PACKAGE, GLABELS_LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	/* Initialize gnome program */
	program = gnome_program_init ("glabels", VERSION,
				      LIBGNOMEUI_MODULE, argc, argv,
				      GNOME_PROGRAM_STANDARD_PROPERTIES,
				      GNOME_PARAM_POPT_TABLE, options,
				      NULL);

	/* Splash screen */
	gl_splash ();

	gl_stock_init();

	/* Set default icon */
	if (!g_file_test (ICON_PIXMAP, G_FILE_TEST_EXISTS))
	{
		g_warning ("Could not find %s", ICON_PIXMAP);
	}
	else
	{
		gnome_window_icon_set_default_from_file (ICON_PIXMAP);
	}
	
	/* Load user preferences */
	gl_prefs_init ();
	gl_prefs_load_settings ();

	gl_template_init();
	gl_merge_init();
	gl_merge_ui_init();
	gl_recent_init();

	if (bonobo_ui_init ("glabels", VERSION, &argc, argv) == FALSE) {
		g_error (_("Could not initialize Bonobo!\n"));
	}

	/* Parse args and build the list of files to be loaded at startup */
	g_value_init (&value, G_TYPE_POINTER);
    	g_object_get_property (G_OBJECT (program),
			       GNOME_PARAM_POPT_CONTEXT, &value);
    	ctx = g_value_get_pointer (&value);
    	g_value_unset (&value);
	args = (char**) poptGetArgs(ctx);
	for (i = 0; args && args[i]; i++) 
	{
		file_list = g_list_append (file_list, args[i]);
	}

	/* Open files or create empty top-level window. */
	for (p = file_list; p; p = p->next) {
		win = gl_window_new_from_file (p->data);
		gtk_widget_show_all (win);
	}
	if ( gl_window_get_window_list() == NULL ) {
		win = gl_window_new ();
		gtk_widget_show_all (win);
	}
	g_list_free (file_list);

	
	/* Begin main loop */
	bonobo_main();
		
	return 0;
}


