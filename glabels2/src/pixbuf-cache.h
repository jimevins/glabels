/*
 *  pixbuf-cache.h
 *  Copyright (C) 2003-2009  Jim Evins <evins@snaught.com>.
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

#ifndef __PIXBUF_CACHE_H__
#define __PIXBUF_CACHE_H__

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

GHashTable *gl_pixbuf_cache_new            (void);

void        gl_pixbuf_cache_free           (GHashTable *pixbuf_cache);

void        gl_pixbuf_cache_add_pixbuf     (GHashTable *pixbuf_cache,
					    gchar      *name,
					    GdkPixbuf  *pixbuf);

GdkPixbuf  *gl_pixbuf_cache_get_pixbuf     (GHashTable *pixbuf_cache,
					    gchar      *name);

void        gl_pixbuf_cache_remove_pixbuf  (GHashTable *pixbuf_cache,
					    gchar      *name);

GList      *gl_pixbuf_cache_get_name_list  (GHashTable *pixbuf_cache);

void        gl_pixbuf_cache_free_name_list (GList      *name_list);

G_END_DECLS

#endif /*__PIXBUF_CACHE_H__ */




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
