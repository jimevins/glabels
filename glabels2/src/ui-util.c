/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  ui-util.c:  GLabels ui utilities module
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

#include "ui-util.h"

#include <bonobo/bonobo-control.h>

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
/* Set sensitivity of verb.                                                  */
/*****************************************************************************/
void
gl_ui_util_set_verb_sensitive (BonoboUIComponent  *ui_component,
			       gchar              *cname,
			       gboolean            sensitive)
{
	gl_debug (DEBUG_UI, "START");

	g_return_if_fail (cname != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (ui_component));

	bonobo_ui_component_set_prop (ui_component,
				      cname,
				      "sensitive",
				      sensitive ? "1" : "0",
				      NULL);

	gl_debug (DEBUG_UI, "END");
}

/*****************************************************************************/
/* Set sensitivity of a list of verbs.                                       */
/*****************************************************************************/
void
gl_ui_util_set_verb_list_sensitive (BonoboUIComponent   *ui_component,
				    gchar              **vlist,
				    gboolean             sensitive)
{
	gl_debug (DEBUG_UI, "START");

	g_return_if_fail (vlist != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (ui_component));

	for ( ; *vlist; ++vlist)
	{
		bonobo_ui_component_set_prop (ui_component,
					      *vlist,
					      "sensitive",
					      sensitive ? "1" : "0",
					      NULL);
	}

	gl_debug (DEBUG_UI, "END");
}

/*****************************************************************************/
/* Set state of a verb.                                                      */
/*****************************************************************************/
void
gl_ui_util_set_verb_state (BonoboUIComponent   *ui_component,
			   gchar               *cname,
			   gboolean             state)
{
	gl_debug (DEBUG_UI, "START");

	g_return_if_fail (cname != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (ui_component));

	bonobo_ui_component_set_prop (ui_component,
				      cname,
				      "state",
				      state ? "1" : "0",
				      NULL);

	gl_debug (DEBUG_UI, "END");
}


/*****************************************************************************/
/* Insert widget at path.                                                    */
/*****************************************************************************/
void
gl_ui_util_insert_widget (BonoboUIComponent *ui_component,
			  GtkWidget         *widget,
			  const char        *path)
{
        BonoboControl *control;
 
	gl_debug (DEBUG_UI, "START");

        gtk_widget_show_all (widget);
        control = bonobo_control_new (widget);
        bonobo_ui_component_object_set (ui_component,
					path,
					BONOBO_OBJREF (control),
					NULL);
        bonobo_object_unref (BONOBO_OBJECT (control));

	gl_debug (DEBUG_UI, "END");
}

