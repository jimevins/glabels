/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (LIBGLABELS) Template library for GLABELS
 *
 *  db.c:  template db module
 *
 *  Copyright (C) 2003-2007  Jim Evins <evins@snaught.com>.
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

#include "db.h"

#include <glib/gi18n.h>
#include <glib/gmem.h>
#include <glib/gstrfuncs.h>
#include <glib/gdir.h>
#include <glib/gmessages.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "libglabels-private.h"

#include "xml-paper.h"
#include "xml-category.h"
#include "xml-template.h"


/*===========================================*/
/* Private types                             */
/*===========================================*/

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GList *papers     = NULL;
static GList *categories = NULL;
static GList *templates  = NULL;

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static GList *read_papers                  (void);
static GList *read_paper_files_from_dir    (GList       *papers,
					    const gchar *dirname);

static GList *read_categories              (void);
static GList *read_category_files_from_dir (GList       *categories,
                                            const gchar *dirname);

static GList *read_templates               (void);
static GList *read_template_files_from_dir (GList       *templates,
                                            const gchar *dirname);

static lglTemplate *template_full_page (const gchar *page_size);



/*===========================================*/
/* Module initialization                     */
/*===========================================*/

/**
 * lgl_db_init:
 *
 * Initialize all libglabels subsystems.  It is not necessary for an application to call
 * lgl_db_init(), because libglabels will initialize on demand.  An application programmer may
 * choose to call lgl_db_init() at startup to minimize the impact of the first libglabels call
 * on GUI response time.
 *
 * This function initializes its paper definitions, category definitions, and its template
 * database.. It will search both system and user template directories to locate
 * this data.
 */
void
lgl_db_init (void)
{
	lglPaper    *paper_other;
        lglCategory *category_user_defined;
        GList       *page_sizes;
        GList       *p;

        /*
         * Paper definitions
         */
	if (!papers)
        {

                papers = read_papers ();

                /* Create and append an "Other" entry. */
                paper_other = lgl_paper_new ("Other", _("Other"), 0.0, 0.0);
                papers = g_list_append (papers, paper_other);

	}

        /*
         * Categories
         */
	if (!categories)
        {
                categories = read_categories ();

                /* Create and append a "User defined" entry. */
                category_user_defined = lgl_category_new ("user-defined", _("User defined"));
                categories = g_list_append (categories, category_user_defined);
	}

        /*
         * Templates
         */
	if (!templates)
        {

                templates = read_templates ();

                /* Create and append generic full page templates. */
                page_sizes = lgl_db_get_paper_id_list ();
                for ( p=page_sizes; p != NULL; p=p->next )
                {
                        if ( !lgl_db_is_paper_id_other (p->data) )
                        {
                                templates = g_list_append (templates,
                                                           template_full_page (p->data));
                        }
                }
                lgl_db_free_paper_id_list (page_sizes);

	}
}



/*===========================================*/
/* Paper db functions.                       */
/*===========================================*/

/**
 * lgl_db_get_paper_id_list:
 *
 * Get a list of all paper ids known to libglabels.
 *
 * Returns: a list of paper ids.
 *
 */
GList *
lgl_db_get_paper_id_list (void)
{
	GList           *ids = NULL;
	GList           *p;
	lglPaper        *paper;

	if (!papers)
        {
		lgl_db_init ();
	}

	for ( p=papers; p != NULL; p=p->next )
        {
		paper = (lglPaper *)p->data;
		ids = g_list_append (ids, g_strdup (paper->id));
	}

	return ids;
}


/**
 * lgl_db_free_paper_id_list:
 * @ids: List of id strings to be freed.
 *
 * Free up all storage associated with an id list obtained with
 * lgl_db_get_paper_id_list().
 *
 */
void
lgl_db_free_paper_id_list (GList *ids)
{
	GList *p;

	for (p = ids; p != NULL; p = p->next)
        {
		g_free (p->data);
		p->data = NULL;
	}

	g_list_free (ids);
}


/**
 * lgl_db_get_paper_name_list:
 *
 * Get a list of all localized paper names known to libglabels.
 *
 * Returns: a list of localized paper names.
 *
 */
GList *
lgl_db_get_paper_name_list (void)
{
	GList           *names = NULL;
	GList           *p;
	lglPaper        *paper;

	if (!papers)
        {
		lgl_db_init ();
	}

	for ( p=papers; p != NULL; p=p->next )
        {
		paper = (lglPaper *)p->data;
		names = g_list_append (names, g_strdup (paper->name));
	}

	return names;
}


/**
 * lgl_db_free_paper_name_list:
 * @names: List of localized paper name strings to be freed.
 *
 * Free up all storage associated with a name list obtained with
 * lgl_db_get_paper_name_list().
 *
 */
void
lgl_db_free_paper_name_list (GList *names)
{
	GList *p;

	for (p = names; p != NULL; p = p->next)
        {
		g_free (p->data);
		p->data = NULL;
	}

	g_list_free (names);
}


/**
 * lgl_db_lookup_paper_from_name:
 * @name: localized paper name string
 *
 * Lookup paper definition from localized paper name string.
 *
 * Returns: pointer to a newly allocated #lglPaper structure.
 *
 */
lglPaper *
lgl_db_lookup_paper_from_name (const gchar *name)
{
	GList       *p;
	lglPaper    *paper;

	if (!papers)
        {
		lgl_db_init ();
	}

	if (name == NULL)
        {
		/* If no name, return first paper as a default */
		return lgl_paper_dup ((lglPaper *) papers->data);
	}

	for (p = papers; p != NULL; p = p->next)
        {
		paper = (lglPaper *) p->data;
		if (UTF8_EQUAL (paper->name, name))
                {
			return lgl_paper_dup (paper);
		}
	}

	return NULL;
}


/**
 * lgl_db_lookup_paper_from_id:
 * @id: paper id string
 *
 * Lookup paper definition from id string.
 *
 * Returns: pointer to a newly allocated #lglPaper structure.
 *
 */
lglPaper *
lgl_db_lookup_paper_from_id (const gchar *id)
{
	GList       *p;
	lglPaper    *paper;

	if (!papers)
        {
		lgl_db_init ();
	}

	if (id == NULL)
        {
		/* If no id, return first paper as a default */
		return lgl_paper_dup ((lglPaper *) papers->data);
	}

	for (p = papers; p != NULL; p = p->next)
        {
		paper = (lglPaper *) p->data;
		if (ASCII_EQUAL (paper->id, id))
                {
			return lgl_paper_dup (paper);
		}
	}

	return NULL;
}


/**
 * lgl_db_lookup_paper_id_from_name:
 * @name: localized paper name stringp
 *
 * Lookup paper name string from localized paper name string.
 *
 * Returns: pointer to a newly allocated id string.
 *
 */
gchar *
lgl_db_lookup_paper_id_from_name (const gchar *name)
{
	lglPaper *paper = NULL;
	gchar    *id = NULL;

	if (name != NULL)
	{
		paper = lgl_db_lookup_paper_from_name (name);
		if ( paper != NULL )
		{
			id = g_strdup (paper->id);
			lgl_paper_free (paper);
			paper = NULL;
		}
	}

	return id;
}


/**
 * lgl_db_lookup_paper_name_from_id:
 * @id: paper id string
 *
 * Lookup localized paper name string from paper id string.
 *
 * Returns: pointer to a newly allocated localized paper name string.
 *
 */
gchar *
lgl_db_lookup_paper_name_from_id (const gchar         *id)
{
	lglPaper *paper = NULL;
	gchar    *name = NULL;

	if (id != NULL)
	{
		paper = lgl_db_lookup_paper_from_id (id);
		if ( paper != NULL )
		{
			name = g_strdup (paper->name);
			lgl_paper_free (paper);
			paper = NULL;
		}
	}

	return name;
}


/**
 * lgl_db_is_paper_id_known:
 * @id: paper id to test
 *
 * Determine if given paper id is known to libglabels.
 *
 * Returns: TRUE if id is known, otherwise FALSE.
 *
 */
gboolean
lgl_db_is_paper_id_known (const gchar *id)
{
	GList       *p;
	lglPaper    *paper;

	if (!papers)
        {
		lgl_db_init ();
	}

	if (id == NULL)
        {
		return FALSE;
	}

	for (p = papers; p != NULL; p = p->next)
        {
		paper = (lglPaper *) p->data;
		if (ASCII_EQUAL (paper->id, id))
                {
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * lgl_db_is_paper_id_other:
 * @id: paper id to test
 *
 * Determine if given paper id is the special id "Other."
 *
 * Returns: TRUE if id is "Other", otherwise FALSE.
 *
 */
gboolean
lgl_db_is_paper_id_other (const gchar *id)
{
	if (id == NULL)
        {
		return FALSE;
	}

	return (ASCII_EQUAL (id, "Other"));
}


static GList *
read_papers (void)
{
	gchar *data_dir;
	GList *papers = NULL;

	data_dir = LGL_SYSTEM_DATA_DIR;
	papers = read_paper_files_from_dir (papers, data_dir);
	g_free (data_dir);

	data_dir = LGL_USER_DATA_DIR;
	papers = read_paper_files_from_dir (papers, data_dir);
	g_free (data_dir);

	if (papers == NULL) {
		g_critical (_("Unable to locate paper size definitions.  Libglabels may not be installed correctly!"));
	}

	return papers;
}


static GList *
read_paper_files_from_dir (GList       *papers,
			   const gchar *dirname)
{
	GDir        *dp;
	const gchar *filename, *extension;
	gchar       *full_filename = NULL;
	GError      *gerror = NULL;
	GList       *new_papers = NULL;

	if (dirname == NULL) {
		return papers;
	}

	if (!g_file_test (dirname, G_FILE_TEST_EXISTS)) {
		return papers;
	}

	dp = g_dir_open (dirname, 0, &gerror);
	if (gerror != NULL) {
	        g_message ("cannot open data directory: %s", gerror->message );
		return papers;
	}

	while ((filename = g_dir_read_name (dp)) != NULL) {

		extension = strrchr (filename, '.');

		if (extension != NULL) {

			if ( ASCII_EQUAL (extension, ".paper") ||
			     ASCII_EQUAL (filename, "paper-sizes.xml") )
                        {

				full_filename =
				    g_build_filename (dirname, filename, NULL);
				new_papers =
				    lgl_xml_paper_read_papers_from_file (full_filename);
				g_free (full_filename);

				papers = g_list_concat (papers, new_papers);
				new_papers = NULL;

			}

		}

	}

	g_dir_close (dp);

	return papers;
}


/**
 * lgl_db_print_known_papers:
 *
 * For debugging purposes: print a list of all paper definitions known to
 * libglabels.
 *
 */
void
lgl_db_print_known_papers (void)
{
	GList       *p;
	lglPaper    *paper;

	if (!papers) {
		lgl_db_init ();
	}

	g_print ("%s():\n", __FUNCTION__);
	for (p = papers; p != NULL; p = p->next) {
		paper = (lglPaper *) p->data;

		g_print ("PAPER id=\"%s\", name=\"%s\", width=%gpts, height=%gpts\n",
			 paper->id, paper->name, paper->width, paper->height);

	}
	g_print ("\n");

}


/*===========================================*/
/* Category db functions.                    */
/*===========================================*/

/**
 * lgl_db_get_category_id_list:
 *
 * Get a list of all category ids known to libglabels.
 *
 * Returns: a list of category ids.
 *
 */
GList *
lgl_db_get_category_id_list (void)
{
	GList           *ids = NULL;
	GList           *p;
	lglCategory     *category;

	if (!categories)
        {
		lgl_db_init ();
	}

	for ( p=categories; p != NULL; p=p->next )
        {
		category = (lglCategory *)p->data;
		ids = g_list_append (ids, g_strdup (category->id));
	}

	return ids;
}


/**
 * lgl_db_free_category_id_list:
 * @ids: List of id strings to be freed.
 *
 * Free up all storage associated with an id list obtained with
 * lgl_db_get_category_id_list().
 *
 */
void
lgl_db_free_category_id_list (GList *ids)
{
	GList *p;

	for (p = ids; p != NULL; p = p->next)
        {
		g_free (p->data);
		p->data = NULL;
	}

	g_list_free (ids);
}


/**
 * lgl_db_get_category_name_list:
 *
 * Get a list of all localized category names known to libglabels.
 *
 * Returns: a list of localized category names.
 *
 */
GList *
lgl_db_get_category_name_list (void)
{
	GList           *names = NULL;
	GList           *p;
	lglCategory     *category;

	if (!categories)
        {
		lgl_db_init ();
	}

	for ( p=categories; p != NULL; p=p->next )
        {
		category = (lglCategory *)p->data;
		names = g_list_append (names, g_strdup (category->name));
	}

	return names;
}


/**
 * lgl_db_free_category_name_list:
 * @names: List of localized category name strings to be freed.
 *
 * Free up all storage associated with a name list obtained with
 * lgl_db_get_category_name_list().
 *
 */
void
lgl_db_free_category_name_list (GList *names)
{
	GList *p;

	for (p = names; p != NULL; p = p->next)
        {
		g_free (p->data);
		p->data = NULL;
	}

	g_list_free (names);
}


/**
 * lgl_db_lookup_category_from_name:
 * @name: localized category name string
 *
 * Lookup category definition from localized category name string.
 *
 * Returns: pointer to a newly allocated #lglCategory structure.
 *
 */
lglCategory *
lgl_db_lookup_category_from_name (const gchar *name)
{
	GList       *p;
	lglCategory *category;

	if (!categories)
        {
		lgl_db_init ();
	}

	if (name == NULL)
        {
		/* If no name, return first category as a default */
		return lgl_category_dup ((lglCategory *) categories->data);
	}

	for (p = categories; p != NULL; p = p->next)
        {
		category = (lglCategory *) p->data;
		if (UTF8_EQUAL (category->name, name))
                {
			return lgl_category_dup (category);
		}
	}

	return NULL;
}


/**
 * lgl_db_lookup_category_from_id:
 * @id: category id string
 *
 * Lookup category definition from id string.
 *
 * Returns: pointer to a newly allocated #lglCategory structure.
 *
 */
lglCategory *
lgl_db_lookup_category_from_id (const gchar *id)
{
	GList       *p;
	lglCategory *category;

	if (!categories)
        {
		lgl_db_init ();
	}

	if (id == NULL)
        {
		/* If no id, return first category as a default */
		return lgl_category_dup ((lglCategory *) categories->data);
	}

	for (p = categories; p != NULL; p = p->next)
        {
		category = (lglCategory *) p->data;
		if (ASCII_EQUAL (category->id, id))
                {
			return lgl_category_dup (category);
		}
	}

	return NULL;
}


/**
 * lgl_db_lookup_category_id_from_name:
 * @name: localized category name stringp
 *
 * Lookup category name string from localized category name string.
 *
 * Returns: pointer to a newly allocated id string.
 *
 */
gchar *
lgl_db_lookup_category_id_from_name (const gchar *name)
{
	lglCategory *category = NULL;
	gchar       *id = NULL;

	if (name != NULL)
	{
		category = lgl_db_lookup_category_from_name (name);
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
 * lgl_db_lookup_category_name_from_id:
 * @id: category id string
 *
 * Lookup localized category name string from category id string.
 *
 * Returns: pointer to a newly allocated localized category name string.
 *
 */
gchar *
lgl_db_lookup_category_name_from_id (const gchar         *id)
{
	lglCategory *category = NULL;
	gchar       *name = NULL;

	if (id != NULL)
	{
		category = lgl_db_lookup_category_from_id (id);
		if ( category != NULL )
		{
			name = g_strdup (category->name);
			lgl_category_free (category);
			category = NULL;
		}
	}

	return name;
}


/**
 * lgl_db_is_category_id_known:
 * @id: category id to test
 *
 * Determine if given category id is known to libglabels.
 *
 * Returns: TRUE if id is known, otherwise FALSE.
 *
 */
gboolean
lgl_db_is_category_id_known (const gchar *id)
{
	GList       *p;
	lglCategory *category;

	if (!categories)
        {
		lgl_db_init ();
	}

	if (id == NULL)
        {
		return FALSE;
	}

	for (p = categories; p != NULL; p = p->next)
        {
		category = (lglCategory *) p->data;
		if (ASCII_EQUAL (category->id, id))
                {
			return TRUE;
		}
	}

	return FALSE;
}


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

			if ( ASCII_EQUAL (extension, ".category") ||
			     ASCII_EQUAL (filename, "categories.xml") )
                        {

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


/**
 * lgl_db_print_known_categories:
 *
 * For debugging purposes: print a list of all category definitions known to
 * libglabels.
 *
 */
void
lgl_db_print_known_categories (void)
{
	GList       *p;
	lglCategory *category;

	if (!categories) {
		lgl_db_init ();
	}

	g_print ("%s():\n", __FUNCTION__);
	for (p = categories; p != NULL; p = p->next) {
		category = (lglCategory *) p->data;

		g_print ("CATEGORY id=\"%s\", name=\"%s\"\n", category->id, category->name);

	}
	g_print ("\n");

}



/*===========================================*/
/* Brand db functions.                       */
/*===========================================*/

/**
 * lgl_db_get_brand_list:
 * @paper_id: If non NULL, limit results to given page size.
 * @category_id: If non NULL, limit results to given template category.
 *
 * Get a list of all valid brands of templates in the template database.
 * Results can be filtered by page size and/or template category.  A list of valid page
 * sizes can be obtained using lgl_db_get_paper_id_list().  A list of valid template
 * categories can be obtained using lgl_db_get_category_id_list().
 *
 * Returns: a list of brands
 */
GList *
lgl_db_get_brand_list (const gchar *paper_id,
                       const gchar *category_id)
{
	GList            *p_tmplt, *p_alias;
	lglTemplate      *template;
	lglTemplateAlias *alias;
	GList            *brands = NULL;

	if (!templates)
        {
		lgl_db_init ();
	}

	for (p_tmplt = templates; p_tmplt != NULL; p_tmplt = p_tmplt->next)
        {
		template = (lglTemplate *) p_tmplt->data;
		if (lgl_template_does_page_size_match (template, paper_id) &&
                    lgl_template_does_category_match (template, category_id))
                {
			for (p_alias = template->aliases; p_alias != NULL;
			     p_alias = p_alias->next)
                        {
                                alias = (lglTemplateAlias *)p_alias->data;

                                if ( !g_list_find_custom (brands, alias->brand,
                                                          (GCompareFunc)lgl_str_utf8_casecmp) )
                                {
                                        brands = g_list_insert_sorted (brands,
                                                                       g_strdup (alias->brand),
                                                                       (GCompareFunc)lgl_str_utf8_casecmp);
                                }
			}
		}
	}

	return brands;
}


/**
 * lgl_db_free_brand_list:
 * @brands: List of template brand strings to be freed.
 *
 * Free up all storage associated with a list of template names obtained with
 * lgl_db_get_brand_list().
 *
 */
void
lgl_db_free_brand_list (GList *brands)
{
	GList *p_brand;

	for (p_brand = brands; p_brand != NULL; p_brand = p_brand->next)
        {
		g_free (p_brand->data);
		p_brand->data = NULL;
	}

	g_list_free (brands);
}



/*===========================================*/
/* Template db functions.                    */
/*===========================================*/

/**
 * lgl_db_register_template:
 * @template:  Pointer to a template structure to add to database.
 *
 * Register a template.  This function adds a template to the template database.
 * The template will be stored in an individual XML file in the user template directory.
 *
 * Returns: Status of registration attempt (#lglDbRegStatus)
 */
lglDbRegStatus
lgl_db_register_template (const lglTemplate *template)
{
        lglTemplate *template_copy;
        gchar       *dir, *filename, *abs_filename;
        gint         bytes_written;

	if (!templates)
        {
		lgl_db_init ();
	}

        if (lgl_db_does_template_exist (template->brand, template->part))
        {
                return LGL_DB_REG_BRAND_PART_EXISTS;
        }

	if (lgl_db_is_paper_id_known (template->paper_id))
        {
		dir = LGL_USER_DATA_DIR;
		mkdir (dir, 0775); /* Try to make sure directory exists. */
		filename = g_strdup_printf ("%s_%s.template", template->brand, template->part);
		abs_filename = g_build_filename (dir, filename, NULL);
		bytes_written = lgl_xml_template_write_template_to_file (template, abs_filename);
		g_free (dir);
		g_free (filename);
		g_free (abs_filename);

                if (bytes_written > 0)
                {
                        template_copy = lgl_template_dup (template);
                        lgl_template_add_category (template_copy, "user-defined");
                        templates = g_list_append (templates, template_copy);
                        return LGL_DB_REG_OK;
                }
                else
                {
                        lgl_template_free (template_copy);
                        return LGL_DB_REG_FILE_WRITE_ERROR;
                }
	}
        else
        {
		g_message ("Cannot register new template with unknown page size.");
                return LGL_DB_REG_BAD_PAPER_ID;
	}

}


/**
 * lgl_db_does_template_exist:
 * @brand: Brand name.
 * @part:  Part name/number.
 *
 * This function tests whether a template with the given brand and part name/number exists.
 *
 * Returns:  TRUE if such a template exists in the database.
 */
gboolean
lgl_db_does_template_exist (const gchar *brand,
                            const gchar *part)
{
	GList            *p_tmplt, *p_alias;
	lglTemplate      *template;
        lglTemplateAlias *alias;

	if (!templates)
        {
		lgl_db_init ();
	}

	if ((brand == NULL) || (part == NULL))
        {
		return FALSE;
	}

	for (p_tmplt = templates; p_tmplt != NULL; p_tmplt = p_tmplt->next)
        {
		template = (lglTemplate *) p_tmplt->data;
		for (p_alias = template->aliases; p_alias != NULL; p_alias = p_alias->next)
                {
                        alias = (lglTemplateAlias *)p_alias->data;

			if ( UTF8_EQUAL (brand, alias->brand) &&
                             UTF8_EQUAL (part, alias->part) )
                        {
				return TRUE;
			}
		}
	}

	return FALSE;
}


/**
 * lgl_db_get_template_name_list_unique:
 * @brand:     If non NULL, limit results to given brand
 * @paper_id: If non NULL, limit results to given page size.
 * @category_id: If non NULL, limit results to given template category.
 *
 * Get a list of valid names of unique templates in the template database.  Results
 * can be filtered by page size and/or template category.  A list of valid page sizes
 * can be obtained using lgl_db_get_paper_id_list().  A list of valid template categories
 * can be obtained using lgl_db_get_category_id_list().
 *
 * This function differs from lgl_db_get_template_name_list_all(), because it does not
 * return multiple names for the same template.
 *
 * Returns: a list of template names.
 */
GList *
lgl_db_get_template_name_list_unique (const gchar *brand,
                                      const gchar *paper_id,
                                      const gchar *category_id)
{
	GList       *p_tmplt;
	lglTemplate *template;
        gchar       *name;
	GList       *names = NULL;

	if (!templates)
        {
		lgl_db_init ();
	}

	for (p_tmplt = templates; p_tmplt != NULL; p_tmplt = p_tmplt->next)
        {
		template = (lglTemplate *) p_tmplt->data;

                if (lgl_template_does_brand_match (template, brand) &&
                    lgl_template_does_page_size_match (template, paper_id) &&
                    lgl_template_does_category_match (template, category_id))
                {
                        name = g_strdup_printf ("%s %s", template->brand, template->part);
                        names = g_list_insert_sorted (names, name,
                                                      (GCompareFunc)g_utf8_collate);
                }
	}

	return names;
}


/**
 * lgl_db_get_template_name_list_all:
 * @brand:     If non NULL, limit results to given brand
 * @paper_id: If non NULL, limit results to given page size.
 * @category_id: If non NULL, limit results to given template category.
 *
 * Get a list of all valid names and aliases of templates in the template database.
 * Results can be filtered by page size and/or template category.  A list of valid page
 * sizes can be obtained using lgl_db_get_paper_id_list().  A list of valid template
 * categories can be obtained using lgl_db_get_category_id_list().
 *
 * This function differs from lgl_db_get_template_name_list_unique(), because it will
 * return multiple names for the same template.
 *
 * Returns: a list of template names and aliases.
 */
GList *
lgl_db_get_template_name_list_all (const gchar *brand,
                                   const gchar *paper_id,
                                   const gchar *category_id)
{
	GList            *p_tmplt, *p_alias;
	lglTemplate      *template;
	lglTemplateAlias *alias;
        gchar            *name;
	GList            *names = NULL;

	if (!templates)
        {
		lgl_db_init ();
	}

	for (p_tmplt = templates; p_tmplt != NULL; p_tmplt = p_tmplt->next)
        {
		template = (lglTemplate *) p_tmplt->data;
		if (lgl_template_does_page_size_match (template, paper_id) &&
                    lgl_template_does_category_match (template, category_id))
                {
			for (p_alias = template->aliases; p_alias != NULL;
			     p_alias = p_alias->next)
                        {
                                alias = (lglTemplateAlias *)p_alias->data;

                                if ( !brand || UTF8_EQUAL( alias->brand, brand) )
                                {
                                        name = g_strdup_printf ("%s %s", alias->brand, alias->part);
                                        names = g_list_insert_sorted (names, name,
                                                                      (GCompareFunc)g_utf8_collate);
                                }
			}
		}
	}

	return names;
}


/**
 * lgl_db_free_template_name_list:
 * @names: List of template name strings to be freed.
 *
 * Free up all storage associated with a list of template names obtained with
 * lgl_db_get_template_name_list_all() or lgl_db_get_template_name_list_unique().
 *
 */
void
lgl_db_free_template_name_list (GList *names)
{
	GList *p_name;

	for (p_name = names; p_name != NULL; p_name = p_name->next)
        {
		g_free (p_name->data);
		p_name->data = NULL;
	}

	g_list_free (names);
}


/**
 * lgl_db_lookup_template_from_name:
 * @name: name string
 *
 * Lookup template in template database from name string.
 *
 * Returns: pointer to a newly allocated #lglTemplate structure.
 *
 */
lglTemplate *
lgl_db_lookup_template_from_name (const gchar *name)
{
	GList            *p_tmplt, *p_alias;
	lglTemplate      *template;
        lglTemplateAlias *alias;
        gchar            *candidate_name;

	if (!templates)
        {
		lgl_db_init ();
	}

	if (name == NULL)
        {
		/* If no name, return first template as a default */
		return lgl_template_dup ((lglTemplate *) templates->data);
	}

	for (p_tmplt = templates; p_tmplt != NULL; p_tmplt = p_tmplt->next)
        {
		template = (lglTemplate *) p_tmplt->data;
		for (p_alias = template->aliases; p_alias != NULL; p_alias = p_alias->next)
                {
                        alias = (lglTemplateAlias *)p_alias->data;
                        candidate_name = g_strdup_printf ("%s %s", alias->brand, alias->part);

			if ( UTF8_EQUAL (candidate_name, name) ) {
                                g_free (candidate_name);
				return lgl_template_dup (template);
			}
                        g_free (candidate_name);
		}
	}

	/* No matching template has been found so return the first template */
	return lgl_template_dup ((lglTemplate *) templates->data);
}


static GList *
read_templates (void)
{
	gchar       *data_dir;
	GList       *templates = NULL;
        GList       *p;
        lglTemplate *template;

        /*
         * User defined templates.  Add to user-defined category.
         */
	data_dir = LGL_USER_DATA_DIR;
	templates = read_template_files_from_dir (templates, data_dir);
	g_free (data_dir);
        for ( p=templates; p != NULL; p=p->next )
        {
                template = (lglTemplate *)p->data;
                lgl_template_add_category (template, "user-defined");
        }

        /*
         * System templates.
         */
	data_dir = LGL_SYSTEM_DATA_DIR;
	templates = read_template_files_from_dir (templates, data_dir);
	g_free (data_dir);

	if (templates == NULL)
        {
		g_critical (_("Unable to locate any template files.  Libglabels may not be installed correctly!"));
	}

	return templates;
}


static GList *
read_template_files_from_dir (GList       *templates,
			      const gchar *dirname)
{
	GDir        *dp;
	const gchar *filename, *extension, *extension2;
	gchar       *full_filename = NULL;
	GError      *gerror = NULL;
	GList       *new_templates = NULL;

	if (dirname == NULL)
		return templates;

	if (!g_file_test (dirname, G_FILE_TEST_EXISTS))
        {
		return templates;
	}

	dp = g_dir_open (dirname, 0, &gerror);
	if (gerror != NULL)
        {
	        g_message ("cannot open data directory: %s", gerror->message );
		return templates;
	}

	while ((filename = g_dir_read_name (dp)) != NULL)
        {

		extension = strrchr (filename, '.');
		extension2 = strrchr (filename, '-');

		if ( (extension && ASCII_EQUAL (extension, ".template")) ||
		     (extension2 && ASCII_EQUAL (extension2, "-templates.xml")) )
                {

			full_filename = g_build_filename (dirname, filename, NULL);
			new_templates =
				lgl_xml_template_read_templates_from_file (full_filename);
			g_free (full_filename);

			templates = g_list_concat (templates, new_templates);
			new_templates = NULL;
		}

	}

	g_dir_close (dp);

	return templates;
}


static lglTemplate *
template_full_page (const gchar *paper_id)
{
	lglPaper              *paper = NULL;
	lglTemplate           *template = NULL;
	lglTemplateFrame      *frame = NULL;
        gchar                 *part;
        gchar                 *desc;

	g_return_val_if_fail (paper_id, NULL);

	paper = lgl_db_lookup_paper_from_id (paper_id);
	if ( paper == NULL )
        {
		return NULL;
	}

	part = g_strdup_printf ("%s-Full-Page", paper->id);
	desc = g_strdup_printf (_("Generic %s full page template"), paper->name);

	template = lgl_template_new ("Generic", part, desc,
                                     paper_id, paper->width, paper->height);


	frame = lgl_template_frame_rect_new ("0",
                                             paper->width,
                                             paper->height,
                                             0.0,
                                             0.0,
                                             0.0);
	lgl_template_add_frame (template, frame);

	lgl_template_frame_add_layout (frame, lgl_template_layout_new (1, 1, 0., 0., 0., 0.));

	lgl_template_frame_add_markup (frame, lgl_template_markup_margin_new (9.0));

	g_free (desc);
	desc = NULL;
	lgl_paper_free (paper);
	paper = NULL;

	return template;
}


/**
 * lgl_db_print_known_templates:
 *
 * Print all known templates (for debugging purposes).
 *
 */
void
lgl_db_print_known_templates (void)
{
	GList       *p;
	lglTemplate *template;

	g_print ("%s():\n", __FUNCTION__);
	for (p=templates; p!=NULL; p=p->next)
        {
		template = (lglTemplate *)p->data;

		g_print("TEMPLATE brand=\"%s\", part=\"%s\", description=\"%s\"\n",
			template->brand, template->part, template->description);

	}
	g_print ("\n");

}

/**
 * lgl_db_print_aliases:
 *   @template: template
 *
 * Print all aliases of a template (for debugging purposes).
 *
 */
void
lgl_db_print_aliases (const lglTemplate *template)
{
	GList            *p;
        lglTemplateAlias *alias;

	g_print ("%s():\n", __FUNCTION__);
	for (p=template->aliases; p!=NULL; p=p->next)
        {
                alias = (lglTemplateAlias *)p->data;
		
		g_print("Alias: brand=\"%s\", part=\"%s\"\n", alias->brand, alias->part);

	}
	g_print ("\n");

}


