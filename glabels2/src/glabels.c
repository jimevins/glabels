/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/**
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

#include <glib/gi18n.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnomeui/gnome-window-icon.h>

#include "critical-error-handler.h"
#include "splash.h"
#include "stock.h"
#include "merge-init.h"
#include "recent.h"
#include <libglabels/paper.h>
#include <libglabels/template.h>
#include "prefs.h"
#include "debug.h"
#include "window.h"
#include "file.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/
#define ICON_PIXMAP_FILE "glabels.png"

/*========================================================*/
/* Private globals                                        */
/*========================================================*/

/*========================================================*/
/* Local function prototypes                              */
/*========================================================*/
gboolean save_session_cb (GnomeClient        *client,
			  gint                phase,
			  GnomeRestartStyle   save_style,
			  gint                shutdown,
			  GnomeInteractStyle  interact_style,
			  gint                fast,
			  gpointer            client_data);

void client_die_cb       (GnomeClient        *client,
			  gpointer            client_data);

/****************************************************************************/
/* main program                                                             */
/****************************************************************************/
int
main (int argc, char **argv)
{
	GValue         value = { 0, };
    	GnomeProgram  *program;
	gchar         *icon_file;
	GnomeClient   *client;
	poptContext    ctx;
	char         **args;
	GList         *file_list = NULL, *p;
	gint           i;
	GtkWidget     *win;
	gchar	      *utf8_filename;

	bindtextdomain (GETTEXT_PACKAGE, GLABELS_LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	/* Initialize gnome program */
	program = gnome_program_init ("glabels", VERSION,
				      LIBGNOMEUI_MODULE, argc, argv,
				      GNOME_PROGRAM_STANDARD_PROPERTIES,
				      NULL);

	/* Install GUI handlers for critical error and warning messages */
	gl_critical_error_handler_init();
	gl_warning_handler_init();

	/* Splash screen */
	gl_splash ();

	/* Set default icon */
	icon_file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_APP_PIXMAP,
					       ICON_PIXMAP_FILE,  FALSE, NULL);
	if (!g_file_test (icon_file, G_FILE_TEST_EXISTS))
	{
		g_message ("Could not find %s", ICON_PIXMAP_FILE);
	}
	else
	{
		gnome_window_icon_set_default_from_file (icon_file);
	}
	g_free (icon_file);
	
	/* Initialize subsystems */
	gl_debug_init ();
	gl_stock_init ();
	gl_paper_init ();
	gl_prefs_init ();
	gl_template_init ();
	gl_merge_init ();
	gl_recent_init ();
	
	client = gnome_master_client();

	g_signal_connect (G_OBJECT (client), "save_yourself",
			  G_CALLBACK (save_session_cb),
			  (gpointer)argv[0]);

	g_signal_connect (G_OBJECT (client), "die",
			  G_CALLBACK (client_die_cb), NULL);

	/* Parse args and build the list of files to be loaded at startup */
	g_value_init (&value, G_TYPE_POINTER);
    	g_object_get_property (G_OBJECT (program),
			       GNOME_PARAM_POPT_CONTEXT, &value);
    	ctx = g_value_get_pointer (&value);
    	g_value_unset (&value);
	args = (char**) poptGetArgs(ctx);
	for (i = 0; args && args[i]; i++) 
	{
		utf8_filename = g_filename_to_utf8 (args[i], -1, NULL, NULL, NULL);
		if (utf8_filename)
			file_list = g_list_append (file_list, utf8_filename);
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

	g_object_unref (G_OBJECT (program));

	return 0;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Save session" callback.                                        */
/*---------------------------------------------------------------------------*/
gboolean save_session_cb (GnomeClient        *client,
			  gint                phase,
			  GnomeRestartStyle   save_style,
			  gint                shutdown,
			  GnomeInteractStyle  interact_style,
			  gint                fast,
			  gpointer            client_data)
{
	gchar       *argv[128];
	gint         argc;
	const GList *window_list;
	GList       *p;
	glWindow    *window;
	glLabel     *label;

	argv[0] = (gchar *)client_data;
	argc = 1;

	window_list = gl_window_get_window_list();
	for ( p=(GList *)window_list; p != NULL; p=p->next ) {
		window = GL_WINDOW(p->data);
		if ( !gl_window_is_empty (window) ) {
			label = GL_VIEW(window->view)->label;
			argv[argc++] = gl_label_get_filename (label);
		}
	}
	gnome_client_set_clone_command(client, argc, argv);
	gnome_client_set_restart_command(client, argc, argv);
	
	return TRUE;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  "Die" callback.                                                 */
/*---------------------------------------------------------------------------*/
void client_die_cb (GnomeClient *client,
		    gpointer     client_data)
{
	gl_file_exit ();
}


