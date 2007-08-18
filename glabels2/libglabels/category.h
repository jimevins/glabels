/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (LIBGLABELS) Template library for GLABELS
 *
 *  category.h:  template category module header file
 *
 *  Copyright (C) 2006  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of the LIBGLABELS library.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 *  MA 02111-1307, USA
 */

#ifndef __CATEGORY_H__
#define __CATEGORY_H__

#include <glib/gtypes.h>
#include <glib/glist.h>

G_BEGIN_DECLS

/*
 *   Template class
 */
typedef struct _glCategory glCategory;

struct _glCategory {
	gchar               *id;     /* Unique ID of paper definition */
	gchar               *name;   /* Localized name of paper */
};


/*
 * Module initialization
 */
void              gl_category_init                (void);


/*
 * Category construction
 */
glCategory       *gl_category_new                 (gchar            *id,
                                                   gchar            *name);

glCategory       *gl_category_dup                 (const glCategory *orig);
void              gl_category_free                (glCategory       *category);


/*
 * Known category name lists
 */
GList            *gl_category_get_name_list       (void);
void              gl_category_free_name_list      (GList            *names);


/*
 * Query functions
 */
glCategory       *gl_category_from_name           (const gchar      *name);
glCategory       *gl_category_from_id             (const gchar      *id);
gchar            *gl_category_lookup_id_from_name (const gchar      *name);
gchar            *gl_category_lookup_name_from_id (const gchar      *id);

G_END_DECLS

#endif
