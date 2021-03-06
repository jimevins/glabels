/*
 *  ui-util.c
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

#include "ui-util.h"

#include <gtk/gtk.h>

#include "debug.h"


/*==========================================================================*/
/* Private macros and constants.                                            */
/*==========================================================================*/


/*==========================================================================*/
/* Private types.                                                           */
/*==========================================================================*/


/*==========================================================================*/
/* Private globals                                                          */
/*==========================================================================*/


/*==========================================================================*/
/* Local function prototypes                                                */
/*==========================================================================*/


/*****************************************************************************/
/** Set sensitivity of verb.                                                 */
/*****************************************************************************/
void
gl_ui_util_set_verb_sensitive (GtkUIManager  *ui,
			       gchar         *cname,
			       gboolean       sensitive)
{
	GtkAction *action;

	gl_debug (DEBUG_UI, "START");

	g_return_if_fail (cname != NULL);
	g_return_if_fail (GTK_IS_UI_MANAGER (ui));

	action = gtk_ui_manager_get_action (ui, cname);

	if (action) {
		gl_debug (DEBUG_UI, "Set action \"%s\" sensitive = %d", cname, sensitive);
		gtk_action_set_sensitive (action, sensitive);
	}

	gl_debug (DEBUG_UI, "END");
}


/*****************************************************************************/
/** Set sensitivity of a list of verbs.                                      */
/*****************************************************************************/
void
gl_ui_util_set_verb_list_sensitive (GtkUIManager  *ui,
				    gchar        **vlist,
				    gboolean       sensitive)
{
	GtkAction *action;

	gl_debug (DEBUG_UI, "START");

	g_return_if_fail (vlist != NULL);
	g_return_if_fail (GTK_IS_UI_MANAGER (ui));

	for ( ; *vlist; ++vlist)
	{
		action = gtk_ui_manager_get_action (ui, *vlist);

		if (action) {
			gtk_action_set_sensitive (action, sensitive);
		}
	}

	gl_debug (DEBUG_UI, "END");
}


/*****************************************************************************/
/** Set state of a verb.                                                     */
/*****************************************************************************/
void
gl_ui_util_set_verb_state (GtkUIManager  *ui,
			   gchar         *cname,
			   gboolean       state)
{
	GtkToggleAction *action;

	gl_debug (DEBUG_UI, "START");

	g_return_if_fail (cname != NULL);
	g_return_if_fail (GTK_IS_UI_MANAGER (ui));

	action = GTK_TOGGLE_ACTION (gtk_ui_manager_get_action (ui, cname));

	if (action) {
		gtk_toggle_action_set_active (action, state);
	}

	gl_debug (DEBUG_UI, "END");
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
