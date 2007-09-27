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
#include <glib/gdir.h>
#include <string.h>

#include "libglabels-private.h"

#include "xml-category.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/


/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GList *categories = NULL;


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static GList *read_categories (void);

static GList *read_category_files_from_dir (GList       *categories,
                                            const gchar *dirname);


/**
 * lgl_category_init:
 *
 * Initialize libglabels category module by reading all category definition
 * files located in system and user template directories.
 *
 * The end user would typically call lgl_init() instead.
 */
void
lgl_category_init (void)
{
	if (categories) {
		return; /* Already initialized. */
	}

	categories = read_categories ();
}


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


/**
 * lgl_category_get_name_list:
 *
 * Get a list of all localized category names known to liblglabels.
 *
 * Returns: a list of localized category names.
 *
 */
GList *
lgl_category_get_name_list (void)
{
	GList           *names = NULL;
	GList           *p;
	lglCategory     *category;

	if (!categories) {
		lgl_category_init ();
	}

	for ( p=categories; p != NULL; p=p->next ) {
		category = (lglCategory *)p->data;
		names = g_list_append (names, g_strdup (category->name));
	}

	return names;
}


/**
 * lgl_category_free_name_list:
 * @names: List of localized category name strings to be freed.
 *
 * Free up all storage associated with a name list obtained with
 * lgl_category_get_name_list().
 *
 */
void
lgl_category_free_name_list (GList *names)
{
	GList *p;

	for (p = names; p != NULL; p = p->next) {
		g_free (p->data);
		p->data = NULL;
	}

	g_list_free (names);
}


/**
 * lgl_category_from_id:
 * @id: category id string
 *
 * Lookup category definition from id string.
 *
 * Returns: pointer to a newly allocated #lglCategory structure.
 *
 */
lglCategory *
lgl_category_from_id (const gchar *id)
{
	GList        *p;
	lglCategory  *category;

	if (!categories) {
		lgl_category_init ();
	}

	if (id == NULL) {
		/* If no id, return first category as a default */
		return lgl_category_dup ((lglCategory *) categories->data);
	}

	for (p = categories; p != NULL; p = p->next) {
		category = (lglCategory *) p->data;
		if (g_strcasecmp (category->id, id) == 0) {
			return lgl_category_dup (category);
		}
	}

	return NULL;
}


/**
 * lgl_category_from_name:
 * @name: localized category name string
 *
 * Lookup category definition from localized category name string.
 *
 * Returns: pointer to a newly allocated #lglCategory structure.
 *
 */
lglCategory *
lgl_category_from_name (const gchar *name)
{
	GList        *p;
	lglCategory  *category;

	if (!categories) {
		lgl_category_init ();
	}

	if (name == NULL) {
		/* If no name, return first category as a default */
		return lgl_category_dup ((lglCategory *) categories->data);
	}

	for (p = categories; p != NULL; p = p->next) {
		category = (lglCategory *) p->data;
		if (g_strcasecmp (category->name, name) == 0) {
			return lgl_category_dup (category);
		}
	}

	return NULL;
}


/**
 * lgl_category_lookup_id_from_name:
 * @name: localized category name stringp
 *
 * Lookup category name string from localized category name string.
 *
 * Returns: pointer to a newly allocated id string.
 *
 */
gchar *
lgl_category_lookup_id_from_name (const gchar       *name)
{
	lglCategory *category = NULL;
	gchar       *id = NULL;

        if (name != NULL)
        {
                category = lgl_category_from_name (name);
                if ( category != NULL )
                {
                        id = g_strdup (category->id);
                        lgl_category_free (category);
                        category = NULL;
                }
        }

	return id;
}

/**
 * lgl_category_lookup_name_from_id:
 * @id: category id string
 *
 * Lookup localized category name string from category id string.
 *
 * Returns: pointer to a newly allocated localized category name string.
 *
 */
gchar *
lgl_category_lookup_name_from_id (const gchar       *id)
{
	lglCategory *category = NULL;
	gchar       *name = NULL;

        if (id != NULL)
        {
                category = lgl_category_from_id (id);
                if ( category != NULL )
                {
                        name = g_strdup (category->name);
                        lgl_category_free (category);
                        category = NULL;
                }
        }

	return name;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Read categories from various files.                            */
/*--------------------------------------------------------------------------*/
static GList *
read_categories (void)
{
	gchar *data_dir;
	GList *categories = NULL;

	data_dir = LGL_SYSTEM_DATA_DIR;
	categories = read_category_files_from_dir (categories, data_dir);
	g_free (data_dir);

	data_dir = LGL_USER_DATA_DIR;
	categories = read_category_files_from_dir (categories, data_dir);
	g_free (data_dir);

	if (categories == NULL) {
		g_critical (_("Unable to locate category definitions.  Libglabels may not be installed correctly!"));
	}

	return categories;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Read all category files from given directory.  Append to list.    */
/*--------------------------------------------------------------------------*/
static GList *
read_category_files_from_dir (GList       *categories,
                              const gchar *dirname)
{
	GDir        *dp;
	const gchar *filename, *extension;
	gchar       *full_filename = NULL;
	GError      *gerror = NULL;
	GList       *new_categories = NULL;

	if (dirname == NULL) {
		return categories;
	}

	if (!g_file_test (dirname, G_FILE_TEST_EXISTS)) {
		return categories;
	}

	dp = g_dir_open (dirname, 0, &gerror);
	if (gerror != NULL) {
	        g_message ("cannot open data directory: %s", gerror->message );
		return categories;
	}

	while ((filename = g_dir_read_name (dp)) != NULL) {

		extension = strrchr (filename, '.');

		if (extension != NULL) {

			if ( (g_strcasecmp (extension, ".categories") == 0)
			     || (g_strcasecmp (filename, "categories.xml") == 0) ) {

				full_filename =
				    g_build_filename (dirname, filename, NULL);
				new_categories =
				    lgl_xml_category_read_categories_from_file (full_filename);
				g_free (full_filename);

				categories = g_list_concat (categories, new_categories);
				new_categories = NULL;

			}

		}

	}

	g_dir_close (dp);

	return categories;
}




