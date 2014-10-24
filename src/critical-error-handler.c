/*
 *  critical-error-handler.h
 *  Copyright (C) 2005-2009  Jim Evins <evins@snaught.com>.
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

#include "critical-error-handler.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <stdlib.h>


static void critical_error_handler (const gchar    *log_domain,
                                    GLogLevelFlags  log_level,
                                    const gchar    *message,
                                    gpointer        user_data);


/***************************************************************************/
/* Initialize error handler.                                               */
/***************************************************************************/
void
gl_critical_error_handler_init (void)
{
        g_log_set_handler ("LibGlabels",
                           G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,
                           critical_error_handler,
                           "libglabels");

        g_log_set_handler (G_LOG_DOMAIN,
                           G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,
                           critical_error_handler,
                           "glabels");
}


/*-------------------------------------------------------------------------*/
/* PRIVATE.  Actual error handler.                                         */
/*-------------------------------------------------------------------------*/
static void
critical_error_handler (const gchar    *log_domain,
                        GLogLevelFlags  log_level,
                        const gchar    *message,
                        gpointer        user_data)
{
        GtkWidget *dummy_window;
        GtkWidget *dialog;

        dummy_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        dialog = gtk_message_dialog_new (GTK_WINDOW (dummy_window),
                                         GTK_DIALOG_MODAL,
                                         GTK_MESSAGE_ERROR,
                                         GTK_BUTTONS_NONE,
                                         _("gLabels Fatal Error!"));
        gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                                  "%s", message);
        gtk_dialog_add_button (GTK_DIALOG (dialog),
                               GTK_STOCK_QUIT, 0);

        gtk_dialog_run (GTK_DIALOG (dialog));

        abort ();
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
