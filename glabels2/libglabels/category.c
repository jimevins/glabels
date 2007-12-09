/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (LIBGLABELS) Template library for GLABELS
 *
 *  category.c:  template category module
 *
 *  Copyright (C) 2001-2006  Jim Evins <evins@snaught.com>.
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
#include <config.h>

#include "category.h"

#include <glib/gi18n.h>
#include <glib/gmem.h>
#include <glib/gstrfuncs.h>
#include <glib/gmessages.h>
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


