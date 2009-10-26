/*
 *  recent.c
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

#include "recent.h"

#include <gtk/gtk.h>
#include <string.h>

#include "prefs.h"

#include "debug.h"

#define GLABELS_MIME_TYPE "application/x-glabels"

static GtkRecentManager *model;


/*****************************************************************************/
/* Initialize recent files model.                                            */
/*****************************************************************************/
void
gl_recent_init (void)
{
        gl_debug (DEBUG_RECENT, "START");

        model = gtk_recent_manager_get_default ();

        gl_debug (DEBUG_RECENT, "END");
}


/*****************************************************************************/
/* Get UTF8 filename from GtkRecentInfo structure.                           */
/*****************************************************************************/
gchar *
gl_recent_get_utf8_filename (GtkRecentInfo *item)
{
        const gchar *uri;
        gchar       *filename;
        gchar       *utf8_filename = NULL;

        gl_debug (DEBUG_RECENT, "START");

        uri = gtk_recent_info_get_uri (item);

        filename = g_filename_from_uri (uri, NULL, NULL);
        if ( filename != NULL )
        {
                utf8_filename = g_filename_to_utf8 (filename, -1, NULL, NULL, NULL);
                g_free (filename);
        }

        return utf8_filename;
        gl_debug (DEBUG_RECENT, "END");
}


/*****************************************************************************/
/* Add file by UTF8 filename to recent model.                                */
/*****************************************************************************/
void
gl_recent_add_utf8_filename (gchar *utf8_filename)
{
        GtkRecentData *recent_data;
        gchar         *filename;
        gchar         *uri;

        static gchar *groups[2] = {
                "glabels",
                NULL
        };

        gl_debug (DEBUG_RECENT, "START");

        recent_data = g_slice_new (GtkRecentData);

        recent_data->display_name = NULL;
        recent_data->description  = NULL;
        recent_data->mime_type    = GLABELS_MIME_TYPE;
        recent_data->app_name     = (gchar *) g_get_application_name ();
        recent_data->app_exec     = g_strjoin (" ", g_get_prgname (), "%f", NULL);
        recent_data->groups       = groups;
        recent_data->is_private = FALSE;

        filename = g_filename_from_utf8 (utf8_filename, -1, NULL, NULL, NULL);
        if ( filename != NULL )
        {

                uri = g_filename_to_uri (filename, NULL, NULL);
                if ( uri != NULL )
                {

                        gtk_recent_manager_add_full (model, uri, recent_data);
                        g_free (uri);

                }
                g_free (filename);

        }

        g_free (recent_data->app_exec);
        g_slice_free (GtkRecentData, recent_data);

        gl_debug (DEBUG_RECENT, "END");
}


/*****************************************************************************/
/* Create a menu of recent files.                                            */
/*****************************************************************************/
GtkWidget *
gl_recent_create_menu (void)
{
        GtkWidget               *recent_menu;
        GtkRecentFilter         *recent_filter;

        gl_debug (DEBUG_RECENT, "START");

        recent_menu  =
                gtk_recent_chooser_menu_new_for_manager (model);
        gtk_recent_chooser_menu_set_show_numbers (GTK_RECENT_CHOOSER_MENU (recent_menu), FALSE);
        gtk_recent_chooser_set_show_icons (GTK_RECENT_CHOOSER (recent_menu), TRUE);
        gtk_recent_chooser_set_limit (GTK_RECENT_CHOOSER (recent_menu),
                                      gl_prefs_model_get_max_recents (gl_prefs));
        gtk_recent_chooser_set_sort_type (GTK_RECENT_CHOOSER (recent_menu), GTK_RECENT_SORT_MRU);
        gtk_recent_chooser_set_local_only (GTK_RECENT_CHOOSER (recent_menu), TRUE);

        recent_filter = gtk_recent_filter_new ();
        gtk_recent_filter_add_mime_type (recent_filter, GLABELS_MIME_TYPE);
        gtk_recent_chooser_set_filter (GTK_RECENT_CHOOSER (recent_menu), recent_filter);

        gl_debug (DEBUG_RECENT, "END");
        return recent_menu;
}




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
