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

/*
 * This file is based on gedit2.c from gedit2:
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi 
 *
 */
#include <config.h>

#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnomeui/gnome-window-icon.h>

#include "glabels.h"
#include "splash.h"
#include "stock.h"
#include "merge.h"
#include "template.h"
#include "mdi.h"
#include "prefs.h"
#include "file.h"
#include "debug.h"

#define ICON_PIXMAP gnome_program_locate_file (NULL,\
					       GNOME_FILE_DOMAIN_APP_PIXMAP,\
					       "glabels/glabels-icon.png",\
					       FALSE, NULL)

glMDI *glabels_mdi = NULL;
gboolean glabels_close_x_button_pressed = FALSE;
gboolean glabels_exit_button_pressed = FALSE; 

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

	{ "debug-mdi", '\0', POPT_ARG_NONE, &gl_debug_mdi, 0,
	  N_("Show mdi debugging messages."), NULL },

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

	/* Create glabels_mdi and open the first top level window */
	glabels_mdi = gl_mdi_new ();
	bonobo_mdi_open_toplevel (BONOBO_MDI (glabels_mdi), NULL); 

	for (p = file_list; p; p = p->next) {
		gl_file_open_real (p->data,
				   GTK_WINDOW(glabels_get_active_window()));
	}
	g_list_free (file_list);

	gtk_main();
		
	return 0;
}


BonoboWindow*
glabels_get_active_window (void)
{
	g_return_val_if_fail (glabels_mdi != NULL, NULL);

	return	bonobo_mdi_get_active_window (BONOBO_MDI (glabels_mdi));
}

glLabel*
glabels_get_active_label (void)
{
	BonoboMDIChild *active_child;

	g_return_val_if_fail (glabels_mdi != NULL, NULL);

	active_child = bonobo_mdi_get_active_child (BONOBO_MDI (glabels_mdi));

	if (active_child == NULL)
		return NULL;

	return GL_MDI_CHILD (active_child)->label;
}

glView*
glabels_get_active_view (void)
{
	GtkWidget *active_view;

	g_return_val_if_fail (glabels_mdi != NULL, NULL);

	active_view = bonobo_mdi_get_active_view (BONOBO_MDI (glabels_mdi));
	
	if (active_view == NULL)
		return NULL;

	return GL_VIEW (active_view);
}

GList* 
glabels_get_top_windows (void)
{
	g_return_val_if_fail (glabels_mdi != NULL, NULL);

	return	bonobo_mdi_get_windows (BONOBO_MDI (glabels_mdi));
}

