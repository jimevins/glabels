/*
 *  lgl-paper.h
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

#ifndef __LGL_PAPER_H__
#define __LGL_PAPER_H__

#include <glib.h>

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

#endif /* __LGL_PAPER_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
