/*
 *  lgl-category.c
 *  Copyright (C) 2006-2010  Jim Evins <evins@snaught.com>.
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

#include <config.h>

#include "lgl-category.h"

#include <glib/gi18n.h>
#include <glib.h>
#include <string.h>

#include "libglabels-private.h"


/*===========================================*/
/* Private types                             */
/*===========================================*/


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/


/*===========================================*/
/* Functions.                                */
/*===========================================*/

/**
 * lgl_category_new:
 * @id:     Id of category definition. (E.g. label, card, etc.)  Should be
 *          unique.
 * @name:   Localized name of category.
 *
 * Allocates and constructs a new #lglCategory structure.
 *
 * Returns: a pointer to a newly allocated #lglCategory structure.
 *
 */
lglCategory *
lgl_category_new (gchar             *id,
                  gchar             *name)
{
	lglCategory *category;

	category         = g_new0 (lglCategory,1);
	category->id     = g_strdup (id);
	category->name   = g_strdup (name);

	return category;
}


/**
 * lgl_category_dup:
 * @orig:  #lglCategory structure to be duplicated.
 *
 * Duplicates an existing #lglCategory structure.
 *
 * Returns: a pointer to a newly allocated #lglCategory structure.
 *
 */
lglCategory *lgl_category_dup (const lglCategory *orig)
{
	lglCategory       *category;

	g_return_val_if_fail (orig, NULL);

	category = g_new0 (lglCategory,1);

	category->id     = g_strdup (orig->id);
	category->name   = g_strdup (orig->name);

	return category;
}


/**
 * lgl_category_free:
 * @category:  pointer to #lglCategory structure to be freed.
 *
 * Free all memory associated with an existing #lglCategory structure.
 *
 */
void lgl_category_free (lglCategory *category)
{

	if ( category != NULL ) {

		g_free (category->id);
		category->id = NULL;

		g_free (category->name);
		category->name = NULL;

		g_free (category);
	}

}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
