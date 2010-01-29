/*
 *  units-util.c
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

#include "units-util.h"

#include "debug.h"


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


/****************************************************************************/
/* Get step size for desired units.                                         */
/****************************************************************************/
gdouble
gl_units_util_get_step_size (lglUnits units)
{

	switch (units) {
	case LGL_UNITS_POINT:
		return 0.1;	/* points */
	case LGL_UNITS_INCH:
		return 0.001;	/* inches */
	case LGL_UNITS_MM:
		return 0.1;	/* mm */
	default:
		g_message ("Illegal units");	/* Should not happen */
		return 1.0;
	}
}


/****************************************************************************/
/* Get precision for desired units.                                         */
/****************************************************************************/
gint
gl_units_util_get_precision (lglUnits units)
{

	switch (units) {
	case LGL_UNITS_POINT:
		return 1;	/* points */
	case LGL_UNITS_INCH:
		return 3;	/* inches */
	case LGL_UNITS_MM:
		return 1;	/* mm */
	default:
		g_message ("Illegal units");	/* Should not happen */
		return 1.0;
	}
}


/****************************************************************************/
/* Get grid size for desired units.                                         */
/****************************************************************************/
gdouble
gl_units_util_get_grid_size (lglUnits units)
{

	switch (units) {
	case LGL_UNITS_POINT:
		return 10.0;
	case LGL_UNITS_INCH:
		return 0.125 * lgl_units_get_points_per_unit (units);
	case LGL_UNITS_MM:
		return 5     * lgl_units_get_points_per_unit (units);
	default:
		g_message ("Illegal units");	/* Should not happen */
		return 10;
	}
}




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
