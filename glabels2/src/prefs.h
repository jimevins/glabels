/*
 *  prefs.h
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

#ifndef __PREFS_H__
#define __PREFS_H__

#include "prefs-model.h"

G_BEGIN_DECLS



extern glPrefsModel *gl_prefs;



void          gl_prefs_init                (void);

const gchar  *gl_prefs_get_page_size       (void);

lglUnitsType  gl_prefs_get_units           (void);

const gchar  *gl_prefs_get_units_string    (void);

gdouble       gl_prefs_get_units_per_point (void);

gdouble       gl_prefs_get_units_step_size (void);

gint          gl_prefs_get_units_precision (void);

void          gl_prefs_add_recent_template (const gchar *name);

G_END_DECLS

#endif /* __PREFS_H__ */




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
