/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  glabels.c: main program module
 *
 *  Copyright (C) 2001  Jim Evins <evins@snaught.com>.
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

#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>

#include "splash.h"
#include "stock.h"
#include "merge.h"
#include "merge_ui.h"
#include "mdi.h"
#include "template.h"
#include "debug.h"

#ifdef PACKAGE_DATA_DIR
#define ICON_PIXMAP (PACKAGE_DATA_DIR "/pixmaps/glabels/glabels-icon.png")
#else
#define ICON_PIXMAP gnome_pixmap_file("glabels/glabels-icon.png")
#endif

/*============================================*/
/* Private globals                            */
/*============================================*/
static struct poptOption options[] = {
	{NULL, '\0', 0, NULL, 0, NULL, NULL}
};

/*============================================*/
/* Private function prototypes                */
/*============================================*/
static void session_die (GnomeClient * client,
			 gpointer client_data);
static gint save_session (GnomeClient * client,
			  gint phase,
			  GnomeSaveStyle save_style,
			  gint is_shutdown,
			  GnomeInteractStyle interact_style,
			  gint is_fast,
			  gpointer client_data);


/*****************************************************************************/
/* Main                                       */
/*****************************************************************************/
int
main (int argc,
      char *argv[])
{
	poptContext pctx;
	gchar **args;
	GnomeMDI *mdi;
	GnomeClient *client;
	GSList *p, *file_list = NULL;
	gint i, opened;

	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
	textdomain (PACKAGE);

	gnome_init_with_popt_table (PACKAGE, VERSION,
				    argc, argv, options, 0, &pctx);

	/* Splash screen */
	gl_splash ();

	if (!g_file_exists (ICON_PIXMAP)) {
		WARN ("Could not find %s", ICON_PIXMAP);
	}
	gnome_window_icon_set_default_from_file (ICON_PIXMAP);

	/* argument parsing */
	args = (char **) poptGetArgs (pctx);
	for (i = 0; args && args[i]; i++) {
		file_list = g_slist_append (file_list, args[i]);
	}
	poptFreeContext (pctx);

	/* session management */
	client = gnome_master_client ();
	gtk_signal_connect (GTK_OBJECT (client), "save_yourself",
			    GTK_SIGNAL_FUNC (save_session), argv[0]);
	gtk_signal_connect (GTK_OBJECT (client), "die",
			    GTK_SIGNAL_FUNC (session_die), NULL);

	gl_stock_init ();
	gl_merge_init ();
	gl_merge_ui_init ();
	gl_template_init ();

	mdi = gl_mdi_init ();

	/* Any files on command line? */
	opened = 0;
	for (p = file_list; p; p = p->next) {
		if (gl_mdi_new_child_from_xml_file (p->data))
			opened++;
	}
	g_slist_free (file_list);

	if (!opened) {
		gnome_mdi_open_toplevel (mdi);
	}

	/* Now start main loop */
	gtk_main ();

	return 0;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Save Session signal callback.                                   */
/*---------------------------------------------------------------------------*/
static gint
save_session (GnomeClient * client,
	      gint phase,
	      GnomeSaveStyle save_style,
	      gint is_shutdown,
	      GnomeInteractStyle interact_style,
	      gint is_fast,
	      gpointer client_data)
{
	gchar **argv;
	guint argc;

	argv = g_malloc0 (4 * sizeof (gchar *));
	argc = 1;

	argv[0] = client_data;

	gnome_client_set_clone_command (client, argc, argv);
	gnome_client_set_restart_command (client, argc, argv);

	return TRUE;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Session Die signal callback.                                    */
/*---------------------------------------------------------------------------*/
static void
session_die (GnomeClient * client,
	     gpointer client_data)
{
	gtk_main_quit ();
}
