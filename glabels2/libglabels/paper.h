/*
 *  (LIBGLABELS) Template library for GLABELS
 *
 *  paper.h:  paper module header file
 *
 *  Copyright (C) 2003, 2004  Jim Evins <evins@snaught.com>.
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

#ifndef __PAPER_H__
#define __PAPER_H__

#include <glib.h>

G_BEGIN_DECLS


/*
 *   glPaper structure
 */
typedef struct _glPaper glPaper;

struct _glPaper {
	gchar               *id;
	gchar               *name;
	gdouble              width;
	gdouble              height;
};


/*
 * Module initialization
 */
void                 gl_paper_init                (void);


/*
 * Paper construction
 */
glPaper             *gl_paper_new                 (gchar           *id,
						   gchar           *name,
						   gdouble          width,
						   gdouble          height);

glPaper             *gl_paper_dup                 (const glPaper   *orig);

void                 gl_paper_free                (glPaper         *paper);


/*
 * ID lists
 */
GList               *gl_paper_get_id_list         (void);
void                 gl_paper_free_id_list        (GList           *ids);


/*
 * Name lists
 */
GList               *gl_paper_get_name_list       (void);
void                 gl_paper_free_name_list      (GList           *names);


/*
 * Query functions
 */
gboolean             gl_paper_is_id_known         (const gchar     *id);
gboolean             gl_paper_is_id_other         (const gchar     *id);

glPaper             *gl_paper_from_id             (const gchar     *id);
glPaper             *gl_paper_from_name           (const gchar     *name);

gchar               *gl_paper_lookup_id_from_name (const gchar     *name);
gchar               *gl_paper_lookup_name_from_id (const gchar     *id);


/*
 * Debugging
 */
void                 gl_paper_print_known_papers  (void);


G_END_DECLS

#endif
