/*
 *  str-util.h
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

#ifndef __STR_UTIL_H__
#define __STR_UTIL_H__

#include <glib.h>
#include <pango/pango.h>

G_BEGIN_DECLS

gchar              *gl_str_util_fraction_to_string    (gdouble            x);

const gchar        *gl_str_util_align_to_string       (PangoAlignment     align);
PangoAlignment      gl_str_util_string_to_align       (const gchar       *string);

const gchar        *gl_str_util_weight_to_string      (PangoWeight        weight);
PangoWeight         gl_str_util_string_to_weight      (const gchar       *string);

G_END_DECLS

#endif /* __STR_UTIL_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
