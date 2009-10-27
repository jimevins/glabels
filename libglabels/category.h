/*
 *  category.h
 *  Copyright (C) 2006-2009  Jim Evins <evins@snaught.com>.
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

#ifndef __LGL_CATEGORY_H__
#define __LGL_CATEGORY_H__

#include <glib.h>

G_BEGIN_DECLS

/*
 *   Template class
 */
typedef struct _lglCategory lglCategory;

struct _lglCategory {
	gchar               *id;     /* Unique ID of category */
	gchar               *name;   /* Localized name of category */
};


/*
 * Category construction
 */
lglCategory      *lgl_category_new                 (gchar             *id,
                                                    gchar             *name);

lglCategory      *lgl_category_dup                 (const lglCategory *orig);
void              lgl_category_free                (lglCategory       *category);


G_END_DECLS

#endif /* __LGL_CATEGORY_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
