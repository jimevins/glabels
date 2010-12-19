/*
 *  lgl-units.c
 *  Copyright (C) 2003-2010  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of libglabels.
 *
 *  libglabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libglabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libglabels.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "lgl-units.h"

#include <glib/gi18n.h>
#include <glib.h>
#include <string.h>

#include "libglabels-private.h"


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

#define POINTS_PER_POINT    1.0 /* internal units are points. */
#define POINTS_PER_INCH    72.0
#define POINTS_PER_MM       2.83464566929
#define POINTS_PER_CM       (10.0*POINTS_PER_MM)
#define POINTS_PER_PICA     (1.0/12.0)


/*========================================================*/
/* Private types.                                         */
/*========================================================*/

typedef struct {
	gchar       *id;
	gchar       *name;
	gdouble      points_per_unit;
} UnitTableEntry;


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static UnitTableEntry unit_table[] = {

	/* The ids are identical to the absolute length units supported in
	   the CSS2 Specification (Section 4.3.2) */

	/* This table must be sorted exactly as the enumerations in lglUnits */

	/* [LGL_UNITS_POINT] */   {"pt", N_("points"), POINTS_PER_POINT},
	/* [LGL_UNITS_INCH]  */   {"in", N_("inches"), POINTS_PER_INCH},
	/* [LGL_UNITS_MM]    */   {"mm", N_("mm"),     POINTS_PER_MM},
	/* [LGL_UNITS_CM]    */   {"cm", N_("cm"),     POINTS_PER_CM},
	/* [LGL_UNITS_PICA]  */   {"pc", N_("picas"),  POINTS_PER_PICA},

};



/**
 * lgl_units_get_id:
 * @units:       Units (#lglUnits)
 *
 * Return a unique ID string for the given units.  This ID is how units
 * are encoded in libglabels XML files and will remain constant across
 * all locales.  IDs are identical to the absolute length units supported
 * in the CSS2 Specification (Section 4.3.2). 
 *
 * Returns: ID string.
 *
 */
const gchar *
lgl_units_get_id (lglUnits     units)
{
        if ( (units >= LGL_UNITS_FIRST) && (units <= LGL_UNITS_LAST) )
        {
                return unit_table[units].id;
        }
        else
        {
                /* Default to "pt", if invalid. */
                return unit_table[LGL_UNITS_POINT].id;
        }
}


/**
 * lgl_units_from_id:
 * @id:       ID string
 *
 * Return the unique #lglUnits for the given ID string.
 * This ID is how units are encoded in libglabels XML files and will remain
 * constant across all locales.  IDs are identical to the absolute length
 * units supported in the CSS2 Specification (Section 4.3.2). 
 *
 * Returns: units (#lglUnits).
 *
 */
lglUnits
lgl_units_from_id (const gchar *id)
{
        lglUnits units;

        /* An empty or missing id defaults to points. */
        if ( (id == NULL) || (strlen (id) == 0) )
        {
                return LGL_UNITS_POINT;
        }

        for ( units = LGL_UNITS_FIRST; units <= LGL_UNITS_LAST; units++) {
                if (g_ascii_strcasecmp (id, unit_table[units].id) == 0) {
                        return units;
                }
        }

        /* Try name as a fallback. (Will catch some legacy preferences.) */
        for ( units = LGL_UNITS_FIRST; units <= LGL_UNITS_LAST; units++) {
                if (g_ascii_strcasecmp (id, unit_table[units].name) == 0) {
                        return units;
                }
        }

        /* For compatibility with old preferences. */
        if (g_ascii_strcasecmp (id, "Millimeters") == 0) {
                return LGL_UNITS_MM;
        }

        return LGL_UNITS_INVALID;
}


/**
 * lgl_units_get_name:
 * @units:       Units (#lglUnits)
 *
 * Return a unique name string for the given units.  This name is human
 * readable and will be translated to the current locale.
 *
 * Returns: name string.
 *
 */
const gchar *
lgl_units_get_name (lglUnits     units)
{
        if ( (units >= LGL_UNITS_FIRST) && (units <= LGL_UNITS_LAST) )
        {
                return gettext ((char *)unit_table[units].name);
        }
        else
        {
                /* Default to "points", if invalid. */
                return gettext ((char *)unit_table[LGL_UNITS_POINT].name);
        }
}


/**
 * lgl_units_from_name:
 * @name:       NAME string
 *
 * Return the unique #lglUnits for the given name string.  This name is
 * human readable and is expected to be translated to the current locale.
 *
 * Returns: units (#lglUnits).
 *
 */
lglUnits
lgl_units_from_name (const gchar *name)
{
        lglUnits units;

        for ( units = LGL_UNITS_FIRST; units <= LGL_UNITS_LAST; units++) {
                if (g_ascii_strcasecmp (name, gettext ((char *)unit_table[units].name) ) == 0) {
                        return units;
                }
        }

        return LGL_UNITS_INVALID;
}


/**
 * lgl_units_get_points_per_unit:
 * @units:       Units (#lglUnits)
 *
 * Return a scale factor for the given units in points/unit.
 *
 * Returns: scale factor.
 *
 */
gdouble
lgl_units_get_points_per_unit (lglUnits     units)
{
        if ( (units >= LGL_UNITS_FIRST) && (units <= LGL_UNITS_LAST) )
        {
                return unit_table[units].points_per_unit;
        }
        else
        {
                /* Default to "points", if invalid. */
                return 1.0;
        }
}


/**
 * lgl_units_get_units_per_point:
 * @units:       Units (#lglUnits)
 *
 * Return a scale factor for the given units in units/point.
 *
 * Returns: scale factor.
 *
 */
gdouble
lgl_units_get_units_per_point (lglUnits     units)
{
        if ( (units >= LGL_UNITS_FIRST) && (units <= LGL_UNITS_LAST) )
        {
                return 1.0 / unit_table[units].points_per_unit;
        }
        else
        {
                /* Default to "points", if invalid. */
                return 1.0;
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
