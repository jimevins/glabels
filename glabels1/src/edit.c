/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  edit.c:  Edit module
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

#include <gnome.h>

#include "edit.h"

#include "display.h"
#include "mdi.h"

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*============================================*/
/* Private function prototypes.               */
/*============================================*/

/*****************************************************************************/
/* "Cut"                                                                     */
/*****************************************************************************/
void
gl_edit_cut_cb (GtkWidget * widget,
		gpointer data)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (GNOME_MDI (data));
	GnomeApp *app = gnome_mdi_get_active_window (GNOME_MDI (data));
	glDisplay *display;

	g_return_if_fail (child != NULL);
	g_return_if_fail (app != NULL);

	display = gl_mdi_get_display (child);

	gl_display_cut (display);

}

/*****************************************************************************/
/* "Copy"                                                                    */
/*****************************************************************************/
void
gl_edit_copy_cb (GtkWidget * widget,
		 gpointer data)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (GNOME_MDI (data));
	GnomeApp *app = gnome_mdi_get_active_window (GNOME_MDI (data));
	glDisplay *display;

	g_return_if_fail (child != NULL);
	g_return_if_fail (app != NULL);

	display = gl_mdi_get_display (child);

	gl_display_copy (display);

}

/*****************************************************************************/
/* "Paste"                                                                   */
/*****************************************************************************/
void
gl_edit_paste_cb (GtkWidget * widget,
		  gpointer data)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (GNOME_MDI (data));
	glDisplay *display;

	g_return_if_fail (child != NULL);

	display = gl_mdi_get_display (child);

	gl_display_paste (display);
}

/*****************************************************************************/
/* "Select ALL"                                                              */
/*****************************************************************************/
void
gl_edit_select_all_cb (GtkWidget * widget,
		       gpointer data)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (GNOME_MDI (data));
	glDisplay *display;

	g_return_if_fail (child != NULL);

	display = gl_mdi_get_display (child);

	gl_display_select_all (display);
}

/*****************************************************************************/
/* "Un-select"                                                               */
/*****************************************************************************/
void
gl_edit_unselect_cb (GtkWidget * widget,
		     gpointer data)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (GNOME_MDI (data));
	glDisplay *display;

	g_return_if_fail (child != NULL);

	display = gl_mdi_get_display (child);

	gl_display_unselect_all (display);
}
