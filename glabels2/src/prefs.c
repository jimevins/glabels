/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prefs.c:  Application preferences module
 *
 *  Copyright (C) 2001-2003  Jim Evins <evins@snaught.com>.
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

#include "prefs.h"

#include <glib/gi18n.h>

#include "debug.h"

glPrefsModel      *gl_prefs     = NULL;

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/


/*========================================================*/
/* Private types.                                         */
/*========================================================*/


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/



/*****************************************************************************/
/* Initialize preferences module.                                            */
/*****************************************************************************/
void 
gl_prefs_init (void)
{
	gl_debug (DEBUG_PREFS, "START");

	gl_prefs = gl_prefs_model_new ();

	gl_prefs_model_load_settings (gl_prefs);

	gl_debug (DEBUG_PREFS, "END");
}


/****************************************************************************/
/* Get string representing desired units.                                   */
/****************************************************************************/
const gchar *
gl_prefs_get_page_size (void)
{
	return (gl_prefs->default_page_size);
}

/****************************************************************************/
/* Get desired units.                                                       */
/****************************************************************************/
glUnitsType
gl_prefs_get_units (void)
{
	return gl_prefs->units;
}

/****************************************************************************/
/* Get desired units per point.                                             */
/****************************************************************************/
gdouble
gl_prefs_get_units_per_point (void)
{

	switch (gl_prefs->units) {
	case GL_UNITS_POINT:
		return 1.0;	/* points */
	case GL_UNITS_INCH:
		return 1.0 / 72.0;	/* inches */
	case GL_UNITS_MM:
		return 0.35277778;	/* mm */
	default:
		g_message ("Illegal units");	/* Should not happen */
		return 1.0;
	}
}

/****************************************************************************/
/* Get precision for desired units.                                         */
/****************************************************************************/
gint
gl_prefs_get_units_precision (void)
{

	switch (gl_prefs->units) {
	case GL_UNITS_POINT:
		return 1;	/* points */
	case GL_UNITS_INCH:
		return 3;	/* inches */
	case GL_UNITS_MM:
		return 1;	/* mm */
	default:
		g_message ("Illegal units");	/* Should not happen */
		return 1.0;
	}
}

/****************************************************************************/
/* Get step size for desired units.                                         */
/****************************************************************************/
gdouble
gl_prefs_get_units_step_size (void)
{

	switch (gl_prefs->units) {
	case GL_UNITS_POINT:
		return 0.1;	/* points */
	case GL_UNITS_INCH:
		return 0.001;	/* inches */
	case GL_UNITS_MM:
		return 0.1;	/* mm */
	default:
		g_message ("Illegal units");	/* Should not happen */
		return 1.0;
	}
}

/****************************************************************************/
/* Get string representing desired units.                                   */
/****************************************************************************/
const gchar *
gl_prefs_get_units_string (void)
{

	switch (gl_prefs->units) {
	case GL_UNITS_POINT:
		return _("points");
	case GL_UNITS_INCH:
		return _("inches");
	case GL_UNITS_MM:
		return _("mm");
	default:
		g_message ("Illegal units");	/* Should not happen */
		return _("points");
	}
}

