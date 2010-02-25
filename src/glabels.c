/*
 *  glabels.c
 *  Copyright (C) 2001-2009  Jim Evins <evins@snaught.com>.
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

#include <config.h>

#include <glib/gi18n.h>

#include <libglabels.h>
#include "warning-handler.h"
#include "critical-error-handler.h"
#include "stock.h"
#include "merge-init.h"
#include "recent.h"
#include "mini-preview-pixbuf-cache.h"
#include "prefs.h"
#include "font-history.h"
#include "template-history.h"
#include "debug.h"
#include "window.h"
#include "file.h"


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/


/*========================================================*/
/* Private globals                                        */
/*========================================================*/


/*========================================================*/
/* Local function prototypes                              */
/*========================================================*/


/****************************************************************************/
/* main program                                                             */
/****************************************************************************/
int
main (int argc, char **argv)
{
	gchar **remaining_args = NULL;
	GOptionEntry option_entries[] = {
		{ G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY,
		  &remaining_args, NULL, N_("[FILE...]") },
		{ NULL }
	};

	GOptionContext *option_context;
	gchar          *icon_file;
	GList          *file_list = NULL, *p;
	GtkWidget      *win;
	gchar	       *utf8_filename;
        GError         *error = NULL;

	bindtextdomain (GETTEXT_PACKAGE, GLABELS_LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	option_context = g_option_context_new (NULL);
        g_option_context_set_summary (option_context,
                                      _("Launch gLabels label and business card designer."));
	g_option_context_add_main_entries (option_context, option_entries, GETTEXT_PACKAGE);


	/* Initialize program */
        gtk_init( &argc, &argv );
        if (!g_option_context_parse (option_context, &argc, &argv, &error))
	{
	        g_print(_("%s\nRun '%s --help' to see a full list of available command line options.\n"),
			error->message, argv[0]);
		g_error_free (error);
		return 1;
	}


	/* Install GUI handlers for critical error and warning messages */
	gl_critical_error_handler_init();
	gl_warning_handler_init();

	/* Set default icon */
	icon_file = g_build_filename (GLABELS_ICON_DIR, GLABELS_ICON, NULL);
	if (!g_file_test (icon_file, G_FILE_TEST_EXISTS))
	{
		g_message ("Could not find %s", icon_file);
	}
	else
	{
		gtk_window_set_default_icon_from_file (icon_file, NULL);
	}
        g_free (icon_file);

	
	/* Initialize subsystems */
	gl_debug_init ();
	gl_stock_init ();
	lgl_db_init ();
	gl_prefs_init ();
	gl_mini_preview_pixbuf_cache_init ();
	gl_merge_init ();
	gl_recent_init ();
        gl_template_history_init ();
        gl_font_history_init ();
	

	/* Parse args and build the list of files to be loaded at startup */
	if (remaining_args != NULL) {
		gint i, num_args;

		num_args = g_strv_length (remaining_args);
		for (i = 0; i < num_args; ++i) {
			utf8_filename = g_filename_to_utf8 (remaining_args[i], -1, NULL, NULL, NULL);
			if (utf8_filename)
				file_list = g_list_append (file_list, utf8_filename);
		}
		g_strfreev (remaining_args);
		remaining_args = NULL;
	}


	/* Open files or create empty top-level window. */
	for (p = file_list; p; p = p->next) {
		win = gl_window_new_from_file (p->data);
		gtk_widget_show_all (win);
		g_free (p->data);
	}
	if ( gl_window_get_window_list() == NULL ) {
		win = gl_window_new ();
		gtk_widget_show_all (win);
	}
	g_list_free (file_list);

	
	/* Begin main loop */
	gtk_main();

	return 0;
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
