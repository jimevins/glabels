/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  splash.c:  Splash screen module
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

#include "config.h"

#include <gnome.h>

#include "debug.h"
#include "splash.h"

#ifdef PACKAGE_DATA_DIR
#define SPLASH_PIXMAP (PACKAGE_DATA_DIR "/pixmaps/glabels/glabels-logo.png")
#else
#define SPLASH_PIXMAP gnome_pixmap_file("glabels/glabels-logo.png")
#endif

#define SPLASH_TIMEOUT 2000

static GtkWidget *splash = NULL;

static gint splash_timeout (gpointer not_used);


/***************************************************************************/
/* Create splash screen.                                                   */
/***************************************************************************/
void
gl_splash (void)
{
	GtkWidget *wpixmap, *wvbox, *whbox, *wframe1, *wframe2;
	gchar *label;

	if (splash)
		return;

	splash = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_window_set_position (GTK_WINDOW (splash), GTK_WIN_POS_CENTER);

	wframe1 = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (wframe1), GTK_SHADOW_OUT);
	wframe2 = gtk_frame_new (NULL);

	wvbox = gtk_vbox_new (FALSE, 5);
	gtk_container_set_border_width (GTK_CONTAINER (wvbox), 2);

	whbox = gtk_hbox_new (FALSE, 5);

	if (!g_file_exists (SPLASH_PIXMAP)) {
		WARN ("Could not find %s", SPLASH_PIXMAP);
	}
	wpixmap = gnome_pixmap_new_from_file (SPLASH_PIXMAP);
	if (!wpixmap) {
		WARN ("can't find splash pixmap");
		gtk_widget_destroy (splash);
		return;
	}

	gtk_container_add (GTK_CONTAINER (splash), wframe1);
	gtk_container_add (GTK_CONTAINER (wframe1), wframe2);
	gtk_container_add (GTK_CONTAINER (wframe2), wvbox);

	gtk_box_pack_start (GTK_BOX (wvbox), wpixmap, TRUE, TRUE, 0);

	label = g_strdup_printf ("Version %s", VERSION);
	gtk_box_pack_start (GTK_BOX (wvbox),
			    gtk_label_new (label), TRUE, TRUE, 0);
	g_free (label);

	gtk_widget_show_all (splash);

	gtk_timeout_add (SPLASH_TIMEOUT, splash_timeout, NULL);

	while (gtk_events_pending ()) {
		gtk_main_iteration ();
	}
}

/*-------------------------------------------------------------------------*/
/* PRIVATE.  Callback to tear-down splash screen once timer has expired.   */
/*-------------------------------------------------------------------------*/
static gint
splash_timeout (gpointer not_used)
{
	if (splash) {
		gtk_widget_destroy (splash);
		splash = NULL;
	}

	return FALSE;
}

