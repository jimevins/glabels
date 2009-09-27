/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  pixbuf-cache.h:  GLabels pixbuf cache module
 *
 *  Copyright (C) 2003  Jim Evins <evins@snaught.com>.
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
#ifndef __PIXBUF_CACHE_H__
#define __PIXBUF_CACHE_H__

#include <glib/gmem.h>
#include <glib/ghash.h>
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
