/*
 *  svg-cache.h
 *  Copyright (C) 2010  Jim Evins <evins@snaught.com>.
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

#ifndef __SVG_CACHE_H__
#define __SVG_CACHE_H__

#include <glib.h>
#include <librsvg/rsvg.h>

G_BEGIN_DECLS

GHashTable *gl_svg_cache_new            (void);

void        gl_svg_cache_free           (GHashTable  *svg_cache);

void        gl_svg_cache_add_svg        (GHashTable  *svg_cache,
                                         gchar       *name,
                                         const gchar *contents);

RsvgHandle *gl_svg_cache_get_handle     (GHashTable  *svg_cache,
                                         gchar       *name);

gchar      *gl_svg_cache_get_contents   (GHashTable  *svg_cache,
                                         gchar       *name);

void        gl_svg_cache_remove_svg     (GHashTable  *svg_cache,
                                         gchar       *name);

GList      *gl_svg_cache_get_name_list  (GHashTable  *svg_cache);

void        gl_svg_cache_free_name_list (GList       *name_list);

G_END_DECLS

#endif /*__SVG_CACHE_H__ */




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
