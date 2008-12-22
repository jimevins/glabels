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

#include <glib/gtypes.h>
#include <glib/glist.h>

G_BEGIN_DECLS


/*
 *   lglPaper structure
 */
typedef struct _lglPaper lglPaper;

struct _lglPaper {
	gchar               *id;       /* Unique ID of paper definition */
	gchar               *name;     /* Localized name of paper */
	gdouble              width;    /* Width (in points) */
	gdouble              height;   /* Height (in points) */
	gchar               *pwg_size; /* PWG 5101.1-2002 size name */
};


/*
 * Paper construction
 */
lglPaper            *lgl_paper_new                 (gchar           *id,
						    gchar           *name,
						    gdouble          width,
						    gdouble          height,
						    gchar           *pwg_size);

lglPaper             *lgl_paper_dup                (const lglPaper   *orig);

void                 lgl_paper_free                (lglPaper         *paper);


G_END_DECLS

#endif
