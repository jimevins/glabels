/*
 *  lgl-units.h
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

#ifndef __LGL_UNITS_H__
#define __LGL_UNITS_H__

#include <glib.h>

G_BEGIN_DECLS


/*
 * Units of distance
 */
typedef enum {
        LGL_UNITS_POINT, /* encoded as "pt" */
        LGL_UNITS_INCH,  /* encoded as "in" */
        LGL_UNITS_MM,    /* encoded as "mm" */
        LGL_UNITS_CM,    /* encoded as "cm" */
        LGL_UNITS_PICA,  /* encoded as "pc" */

        LGL_UNITS_FIRST = LGL_UNITS_POINT,
        LGL_UNITS_LAST  = LGL_UNITS_PICA,

        LGL_UNITS_INVALID = -1,
} lglUnits;


const gchar *lgl_units_get_id               (lglUnits     units);
lglUnits     lgl_units_from_id              (const gchar *id);

const gchar *lgl_units_get_name             (lglUnits     units);
lglUnits     lgl_units_from_name            (const gchar *name);

gdouble      lgl_units_get_points_per_unit  (lglUnits     units);
gdouble      lgl_units_get_units_per_point  (lglUnits     units);


G_END_DECLS


#endif /* __LGL_UNITS_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
