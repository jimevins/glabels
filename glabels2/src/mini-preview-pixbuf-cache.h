/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  mini-preview-pixbuf-cache.h:  GLabels mini-preview pixbuf cache module
 *
 *  Copyright (C) 2007  Jim Evins <evins@snaught.com>.
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
#ifndef __MINI_PREVIEW_PIXBUF_CACHE_H__
#define __MINI_PREVIEW_PIXBUF_CACHE_H__

#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

void        gl_mini_preview_pixbuf_cache_init           (void);

void        gl_mini_preview_pixbuf_cache_add_by_name    (gchar      *name);

GdkPixbuf  *gl_mini_preview_pixbuf_cache_get_pixbuf     (gchar      *name);


G_END_DECLS

#endif /*__MINI_PREVIEW_PIXBUF_CACHE_H__ */
