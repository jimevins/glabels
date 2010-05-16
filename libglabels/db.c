/*
 *  db.c
 *  Copyright (C) 2003-2009  Jim Evins <evins@snaught.com>.
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

#include "db.h"

#include <glib/gi18n.h>
#include <glib.h>
#include <glib-object.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "libglabels-private.h"

#include "xml-paper.h"
#include "xml-category.h"
#include "xml-vendor.h"
#include "xml-template.h"

/*===========================================*/
/* Private macros and constants.             */
/*===========================================*/

/* Data system and user data directories.  (must free w/ g_free()) */
#define SYSTEM_CONFIG_DIR     g_build_filename (LIBGLABELS_CONFIG_DIR, "templates", NULL)
#define USER_CONFIG_DIR       g_build_filename (g_get_user_config_dir (), "libglabels", "templates" , NULL)
#define USER_CONFIG_DIR_OLD22 g_build_filename (g_get_home_dir (), ".glabels", NULL)


/*===========================================*/
/* Private types                             */
/*===========================================*/

#define TYPE_LGL_DB_MODEL              (lgl_db_model_get_type ())
#define LGL_DB_MODEL(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_LGL_DB_MODEL, lglDbModel))
#define LGL_DB_MODEL_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_LGL_DB_MODEL, lglDbModelClass))
#define IS_LGL_DB_MODEL(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_LGL_DB_MODEL))
#define IS_LGL_DB_MODEL_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_LGL_DB_MODEL))
#define LGL_DB_MODEL_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), TYPE_LGL_DB_MODEL, lglDbModelClass))


typedef struct _lglDbModel          lglDbModel;
typedef struct _lglDbModelClass     lglDbModelClass;


struct _lglDbModel {
        GObject     parent;

        GList      *papers;
        GList      *categories;
        GList      *vendors;
        GList      *templates;

        GHashTable *template_cache;
};


struct _lglDbModelClass {
        GObjectClass  parent_class;

        /*
         * Signals
         */
        void (*changed)     (lglDbModel *this,
                             gpointer    user_data);

};


enum {
        CHANGED,
        LAST_SIGNAL
};


/*===========================================*/
/* Private globals                           */
/*===========================================*/

static guint signals[LAST_SIGNAL] = {0};

static lglDbModel *model = NULL;


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void   lgl_db_model_finalize        (GObject     *object);

static void   add_to_template_cache        (lglTemplate *template);

static GList *read_papers                  (void);
static GList *read_paper_files_from_dir    (GList       *papers,
					    const gchar *dirname);

static GList *read_categories              (void);
static GList *read_category_files_from_dir (GList       *categories,
                                            const gchar *dirname);

static GList *read_vendors                 (void);
static GList *read_vendor_files_from_dir   (GList       *vendors,
					    const gchar *dirname);

static void   read_templates               (void);
static void   read_template_files_from_dir (const gchar *dirname);

static lglTemplate *template_full_page     (const gchar *page_size);

static void   copy_old_custom_files        (void);


/*****************************************************************************/
/* Object infrastructure.                                                    */
/*****************************************************************************/
G_DEFINE_TYPE (lglDbModel, lgl_db_model, G_TYPE_OBJECT);


/*****************************************************************************/
/* Class Init Function.                                                      */
/*****************************************************************************/
static void
lgl_db_model_class_init (lglDbModelClass *class)
{
        GObjectClass  *gobject_class = (GObjectClass *) class;

        lgl_db_model_parent_class = g_type_class_peek_parent (class);

        gobject_class->finalize = lgl_db_model_finalize;

        signals[CHANGED] =
                g_signal_new ("changed",
                              G_OBJECT_CLASS_TYPE (gobject_class),
                              G_SIGNAL_RUN_LAST,
                              G_STRUCT_OFFSET (lglDbModelClass, changed),
                              NULL, NULL,
                              g_cclosure_marshal_VOID__VOID,
                              G_TYPE_NONE,
                              0);
}


/*****************************************************************************/
/* Object Instance Init Function.                                            */
/*****************************************************************************/
static void
lgl_db_model_init (lglDbModel *this)
{
        this->template_cache = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify)lgl_template_free);
}


/*****************************************************************************/
/* Finalize Method.                                                          */
/*****************************************************************************/
static void
lgl_db_model_finalize (GObject *object)
{
        lglDbModel    *this;
        GList         *p;

        g_return_if_fail (object && IS_LGL_DB_MODEL (object));
        this = LGL_DB_MODEL (object);

        g_hash_table_unref (this->template_cache);

	for (p = this->papers; p != NULL; p = p->next)
        {
		g_free (p->data);
		p->data = NULL;
	}
	g_list_free (this->papers);

	for (p = this->categories; p != NULL; p = p->next)
        {
		g_free (p->data);
		p->data = NULL;
	}
	g_list_free (this->categories);

	for (p = this->vendors; p != NULL; p = p->next)
        {
		g_free (p->data);
		p->data = NULL;
	}
	g_list_free (this->vendors);

	for (p = this->templates; p != NULL; p = p->next)
        {
		lgl_template_free ((lglTemplate *)p->data);
		p->data = NULL;
	}
	g_list_free (this->templates);

        G_OBJECT_CLASS (lgl_db_model_parent_class)->finalize (object);
}


/*****************************************************************************/
/** New Object Generator.                                                    */
/*****************************************************************************/
lglDbModel *
lgl_db_model_new (void)
{
        lglDbModel *this;

        this = g_object_new (TYPE_LGL_DB_MODEL, NULL);

        return this;
}


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
 * This function initializes its paper definitions, category definitions, vendor definitions,
 * and its template database. It will search both system and user template directories to locate
 * this data.
 */
void
lgl_db_init (void)
{
	lglPaper    *paper_other;
        lglCategory *category_user_defined;
        lglTemplate *template;
        GList       *page_sizes;
        GList       *p;

        copy_old_custom_files ();

        model = lgl_db_model_new ();

        /*
         * Paper definitions
         */
        model->papers = read_papers ();

        /* Create and append an "Other" entry. */
        paper_other = lgl_paper_new ("Other", _("Other"), 0.0, 0.0, NULL);
        model->papers = g_list_append (model->papers, paper_other);

        /*
         * Categories
         */
        model->categories = read_categories ();

        /* Create and append a "User defined" entry. */
        category_user_defined = lgl_category_new ("user-defined", _("User defined"));
        model->categories = g_list_append (model->categories, category_user_defined);

        /*
         * Vendors
         */
        model->vendors = read_vendors ();

        /*
         * Templates
         */
        read_templates ();

        /* Create and append generic full page templates. */
        page_sizes = lgl_db_get_paper_id_list ();
        for ( p=page_sizes; p != NULL; p=p->next )
        {
                if ( !lgl_db_is_paper_id_other (p->data) )
                {
                        template = template_full_page (p->data);
                        _lgl_db_register_template_internal (template);
                        lgl_template_free (template);
                }
        }
        lgl_db_free_paper_id_list (page_sizes);

}


gulong
lgl_db_notify_add (lglDbNotifyFunc func,
                   gpointer        user_data)
{
	if (!model)
        {
		lgl_db_init ();
	}

        return g_signal_connect_swapped (G_OBJECT (model), "changed", G_CALLBACK (func), user_data);
}


void
lgl_db_notify_remove  (gulong id)
{
        g_signal_handler_disconnect (G_OBJECT (model), id);
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

	if (!model)
        {
		lgl_db_init ();
	}

	for ( p=model->papers; p != NULL; p=p->next )
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

	if (!model)
        {
		lgl_db_init ();
	}

	for ( p=model->papers; p != NULL; p=p->next )
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

	if (!model)
        {
		lgl_db_init ();
	}

	if (name == NULL)
        {
		/* If no name, return first paper as a default */
		return lgl_paper_dup ((lglPaper *) model->papers->data);
	}

	for (p = model->papers; p != NULL; p = p->next)
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

	if (!model)
        {
		lgl_db_init ();
	}

	if (id == NULL)
        {
		/* If no id, return first paper as a default */
		return lgl_paper_dup ((lglPaper *) model->papers->data);
	}

	for (p = model->papers; p != NULL; p = p->next)
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

	if (!model)
        {
		lgl_db_init ();
	}

	if (id == NULL)
        {
		return FALSE;
	}

	for (p = model->papers; p != NULL; p = p->next)
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

	data_dir = SYSTEM_CONFIG_DIR;
	papers = read_paper_files_from_dir (papers, data_dir);
	g_free (data_dir);

	data_dir = USER_CONFIG_DIR;
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

			if ( ASCII_EQUAL (filename, "paper-sizes.xml") )
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

	if (!model) {
		lgl_db_init ();
	}

	g_print ("%s():\n", __FUNCTION__);
	for (p = model->papers; p != NULL; p = p->next) {
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

	if (!model)
        {
		lgl_db_init ();
	}

	for ( p=model->categories; p != NULL; p=p->next )
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

	if (!model)
        {
		lgl_db_init ();
	}

	for ( p=model->categories; p != NULL; p=p->next )
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

	if (!model)
        {
		lgl_db_init ();
	}

	if (name == NULL)
        {
		/* If no name, return first category as a default */
		return lgl_category_dup ((lglCategory *) model->categories->data);
	}

	for (p = model->categories; p != NULL; p = p->next)
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

	if (!model)
        {
		lgl_db_init ();
	}

	if (id == NULL)
        {
		/* If no id, return first category as a default */
		return lgl_category_dup ((lglCategory *) model->categories->data);
	}

	for (p = model->categories; p != NULL; p = p->next)
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

	if (!model)
        {
		lgl_db_init ();
	}

	if (id == NULL)
        {
		return FALSE;
	}

	for (p = model->categories; p != NULL; p = p->next)
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

	data_dir = SYSTEM_CONFIG_DIR;
	categories = read_category_files_from_dir (categories, data_dir);
	g_free (data_dir);

	data_dir = USER_CONFIG_DIR;
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

			if ( ASCII_EQUAL (filename, "categories.xml") )
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

	if (!model) {
		lgl_db_init ();
	}

	g_print ("%s():\n", __FUNCTION__);
	for (p = model->categories; p != NULL; p = p->next) {
		category = (lglCategory *) p->data;

		g_print ("CATEGORY id=\"%s\", name=\"%s\"\n", category->id, category->name);

	}
	g_print ("\n");

}


/*===========================================*/
/* Vendor db functions.                       */
/*===========================================*/

/**
 * lgl_db_get_vendor_name_list:
 *
 * Get a list of all localized vendor names known to libglabels.
 *
 * Returns: a list of localized vendor names.
 *
 */
GList *
lgl_db_get_vendor_name_list (void)
{
	GList           *names = NULL;
	GList           *p;
	lglVendor       *vendor;

	if (!model)
        {
		lgl_db_init ();
	}

	for ( p=model->vendors; p != NULL; p=p->next )
        {
		vendor = (lglVendor *)p->data;
		names = g_list_append (names, g_strdup (vendor->name));
	}

	return names;
}


/**
 * lgl_db_free_vendor_name_list:
 * @names: List of localized vendor name strings to be freed.
 *
 * Free up all storage associated with a name list obtained with
 * lgl_db_get_vendor_name_list().
 *
 */
void
lgl_db_free_vendor_name_list (GList *names)
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
 * lgl_db_lookup_vendor_from_name:
 * @name: localized vendor name string
 *
 * Lookup vendor definition from localized vendor name string.
 *
 * Returns: pointer to a newly allocated #lglVendor structure.
 *
 */
lglVendor *
lgl_db_lookup_vendor_from_name (const gchar *name)
{
	GList       *p;
	lglVendor   *vendor;

	if (!model)
        {
		lgl_db_init ();
	}

	if (name == NULL)
        {
		/* If no name, return first vendor as a default */
		return lgl_vendor_dup ((lglVendor *) model->vendors->data);
	}

	for (p = model->vendors; p != NULL; p = p->next)
        {
		vendor = (lglVendor *) p->data;
		if (UTF8_EQUAL (vendor->name, name))
                {
			return lgl_vendor_dup (vendor);
		}
	}

	return NULL;
}


/**
 * lgl_db_is_vendor_name_known:
 * @name: vendor name to test
 *
 * Determine if given vendor id is known to libglabels.
 *
 * Returns: TRUE if id is known, otherwise FALSE.
 *
 */
gboolean
lgl_db_is_vendor_name_known (const gchar *name)
{
	GList       *p;
	lglVendor   *vendor;

	if (!model)
        {
		lgl_db_init ();
	}

	if (name == NULL)
        {
		return FALSE;
	}

	for (p = model->vendors; p != NULL; p = p->next)
        {
		vendor = (lglVendor *) p->data;
		if (UTF8_EQUAL (vendor->name, name))
                {
			return TRUE;
		}
	}

	return FALSE;
}


static GList *
read_vendors (void)
{
	gchar *data_dir;
	GList *vendors = NULL;

	data_dir = SYSTEM_CONFIG_DIR;
	vendors = read_vendor_files_from_dir (vendors, data_dir);
	g_free (data_dir);

	data_dir = USER_CONFIG_DIR;
	vendors = read_vendor_files_from_dir (vendors, data_dir);
	g_free (data_dir);

	return vendors;
}


static GList *
read_vendor_files_from_dir (GList      *vendors,
			   const gchar *dirname)
{
	GDir        *dp;
	const gchar *filename, *extension;
	gchar       *full_filename = NULL;
	GError      *gerror = NULL;
	GList       *new_vendors = NULL;

	if (dirname == NULL) {
		return vendors;
	}

	if (!g_file_test (dirname, G_FILE_TEST_EXISTS)) {
		return vendors;
	}

	dp = g_dir_open (dirname, 0, &gerror);
	if (gerror != NULL) {
	        g_message ("cannot open data directory: %s", gerror->message );
		return vendors;
	}

	while ((filename = g_dir_read_name (dp)) != NULL) {

		extension = strrchr (filename, '.');

		if (extension != NULL) {

			if ( ASCII_EQUAL (filename, "vendors.xml") )
                        {

				full_filename =
				    g_build_filename (dirname, filename, NULL);
				new_vendors =
				    lgl_xml_vendor_read_vendors_from_file (full_filename);
				g_free (full_filename);

				vendors = g_list_concat (vendors, new_vendors);
				new_vendors = NULL;

			}

		}

	}

	g_dir_close (dp);

	return vendors;
}


/**
 * lgl_db_print_known_vendors:
 *
 * For debugging purposes: print a list of all vendor definitions known to
 * libglabels.
 *
 */
void
lgl_db_print_known_vendors (void)
{
	GList       *p;
	lglVendor   *vendor;

	if (!model) {
		lgl_db_init ();
	}

	g_print ("%s():\n", __FUNCTION__);
	for (p = model->vendors; p != NULL; p = p->next) {
		vendor = (lglVendor *) p->data;

		g_print ("VENDOR name=\"%s\", url=\"%s\"\n",
			 vendor->name, vendor->url);

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

	if (!model)
        {
		lgl_db_init ();
	}

	for (p_tmplt = model->templates; p_tmplt != NULL; p_tmplt = p_tmplt->next)
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

void
_lgl_db_register_template_internal (const lglTemplate   *template)
{
        lglTemplate *template_copy;

        if (!lgl_db_does_template_exist (template->brand, template->part))
        {
                template_copy = lgl_template_dup (template);
                model->templates = g_list_append (model->templates, template_copy);
                add_to_template_cache (template_copy);
        }
        else
        {
		g_message ("Duplicate template: %s %s.", template->brand, template->part);
        }
}


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

	if (!model)
        {
		lgl_db_init ();
	}

        if (lgl_db_does_template_exist (template->brand, template->part))
        {
                return LGL_DB_REG_BRAND_PART_EXISTS;
        }

	if (lgl_db_is_paper_id_known (template->paper_id))
        {
		dir = USER_CONFIG_DIR;
		g_mkdir_with_parents (dir, 0775); /* Try to make sure directory exists. */
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
                        model->templates = g_list_append (model->templates, template_copy);
                        add_to_template_cache (template_copy);
                        g_signal_emit (G_OBJECT (model), signals[CHANGED], 0);
                        return LGL_DB_REG_OK;
                }
                else
                {
                        return LGL_DB_REG_FILE_WRITE_ERROR;
                }
	}
        else
        {
		g_message ("Cannot register new template with unknown page size.");
                return LGL_DB_REG_BAD_PAPER_ID;
	}

}


lglDbDeleteStatus
lgl_db_delete_template_by_name (const gchar *name)
{
        lglTemplate *template, *template1;
        gchar       *dir, *filename, *abs_filename;
        GList       *p;

	if (!model)
        {
		lgl_db_init ();
	}

        if (!lgl_db_does_template_name_exist (name))
        {
                return LGL_DB_DELETE_DOES_NOT_EXIST;
        }

        template = lgl_db_lookup_template_from_name (name);
        if ( lgl_template_does_category_match (template, "user-defined") )
        {
		dir = USER_CONFIG_DIR;
		filename = g_strdup_printf ("%s_%s.template", template->brand, template->part);
		abs_filename = g_build_filename (dir, filename, NULL);

                g_free (abs_filename);

		g_free (dir);
		g_free (filename);
		g_free (abs_filename);

                for ( p=model->templates; p != NULL; p=p->next )
                {
                        template1 = (lglTemplate *)p->data;

                        if ( lgl_template_do_templates_match (template, template1) )
                        {
                                model->templates = g_list_delete_link (model->templates, p);
                                g_hash_table_remove (model->template_cache, name);
                                break;
                        }
                }

                lgl_template_free (template);

                g_signal_emit (G_OBJECT (model), signals[CHANGED], 0);
                return LGL_DB_DELETE_OK;
        }
        else
        {
                return LGL_DB_DELETE_NOT_USER_DEFINED;
        }

}


lglDbDeleteStatus
lgl_db_delete_template_by_brand_part (const gchar  *brand,
                                      const gchar  *part)
{
        gchar             *name;
        lglDbDeleteStatus  status;

        name = g_strdup_printf ("%s %s", brand, part);

        status = lgl_db_delete_template_by_name (name);

        g_free (name);

        return status;
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

	if (!model)
        {
		lgl_db_init ();
	}

	if ((brand == NULL) || (part == NULL))
        {
		return FALSE;
	}

	for (p_tmplt = model->templates; p_tmplt != NULL; p_tmplt = p_tmplt->next)
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
 * lgl_db_does_template_name_exist:
 * @name: name string
 *
 * This function test whether a template with the given name exists.
 *
 * Returns: TRUE if such a template exists in the database.
 *
 */
gboolean
lgl_db_does_template_name_exist (const gchar *name)
{
	GList            *p_tmplt, *p_alias;
        lglTemplate      *template;
        lglTemplateAlias *alias;
        gchar            *candidate_name;

	if (!model)
        {
		lgl_db_init ();
	}

	if (name == NULL)
        {
		return FALSE;
	}

	for (p_tmplt = model->templates; p_tmplt != NULL; p_tmplt = p_tmplt->next)
        {
		template = (lglTemplate *) p_tmplt->data;
		for (p_alias = template->aliases; p_alias != NULL; p_alias = p_alias->next)
                {
                        alias = (lglTemplateAlias *)p_alias->data;
                        candidate_name = g_strdup_printf ("%s %s", alias->brand, alias->part);

			if ( UTF8_EQUAL (candidate_name, name) ) {
                                g_free (candidate_name);
                                return TRUE;
			}
                        g_free (candidate_name);
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

	if (!model)
        {
		lgl_db_init ();
	}

	for (p_tmplt = model->templates; p_tmplt != NULL; p_tmplt = p_tmplt->next)
        {
		template = (lglTemplate *) p_tmplt->data;

                if (lgl_template_does_brand_match (template, brand) &&
                    lgl_template_does_page_size_match (template, paper_id) &&
                    lgl_template_does_category_match (template, category_id))
                {
                        name = g_strdup_printf ("%s %s", template->brand, template->part);
                        names = g_list_insert_sorted (names, name,
                                                      (GCompareFunc)lgl_str_part_name_cmp);
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

	if (!model)
        {
		lgl_db_init ();
	}

	for (p_tmplt = model->templates; p_tmplt != NULL; p_tmplt = p_tmplt->next)
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
                                                                      (GCompareFunc)lgl_str_part_name_cmp);
                                }
			}
		}
	}

	return names;
}


/**
 * lgl_db_get_similar_template_name_list:
 * @name:     Name of template under test.
 *
 * Get a list of all valid names and aliases of templates in the template database that
 * have the same size and layout characteristics as the given template.
 *
 * Returns: a list of template names and aliases.
 */
GList *
lgl_db_get_similar_template_name_list (const gchar  *name)
{
	GList            *p_tmplt, *p_alias;
	lglTemplate      *template1;
	lglTemplate      *template2;
	lglTemplateAlias *alias;
        gchar            *name2;
	GList            *names = NULL;

	if (!model)
        {
		lgl_db_init ();
	}

        if ( !name )
        {
                return NULL;
        }

        template1 = lgl_db_lookup_template_from_name (name);
        if ( !template1 )
        {
                return NULL;
        }

        for (p_alias = template1->aliases; p_alias != NULL; p_alias = p_alias->next)
        {
                alias = (lglTemplateAlias *)p_alias->data;

                name2 = g_strdup_printf ("%s %s", alias->brand, alias->part);
                if ( !UTF8_EQUAL (name2, name) )
                {
                        names = g_list_insert_sorted (names, name2,
                                                      (GCompareFunc)lgl_str_part_name_cmp);
                }
        }

	for (p_tmplt = model->templates; p_tmplt != NULL; p_tmplt = p_tmplt->next)
        {
		template2 = (lglTemplate *) p_tmplt->data;

                if ( lgl_template_are_templates_identical (template1, template2) )
                {
			for (p_alias = template2->aliases; p_alias != NULL; p_alias = p_alias->next)
                        {
                                alias = (lglTemplateAlias *)p_alias->data;

                                name2 = g_strdup_printf ("%s %s", alias->brand, alias->part);
                                if ( !UTF8_EQUAL (name2, name) )
                                {
                                        names = g_list_insert_sorted (names, name2,
                                                                      (GCompareFunc)lgl_str_part_name_cmp);
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
	GList            *p_alias;
	lglTemplate      *template;
        lglTemplateAlias *alias;
        gchar            *candidate_name;
	lglTemplate      *new_template;

	if (!model)
        {
		lgl_db_init ();
	}

	if (name == NULL)
        {
		/* If no name, return first template as a default */
		return lgl_template_dup ((lglTemplate *) model->templates->data);
	}

        template = g_hash_table_lookup (model->template_cache, name);

        if (template)
        {
                for (p_alias = template->aliases; p_alias != NULL; p_alias = p_alias->next)
                {
                        alias = (lglTemplateAlias *)p_alias->data;
                        candidate_name = g_strdup_printf ("%s %s", alias->brand, alias->part);

                        if ( UTF8_EQUAL (candidate_name, name) )
                        {
                                g_free (candidate_name);
                                new_template = lgl_template_dup (template);
                                g_free (new_template->brand);
                                new_template->brand = g_strdup (alias->brand);
                                g_free (new_template->part);
                                new_template->part = g_strdup (alias->part);
                                return new_template;
                        }

                        g_free (candidate_name);
                }
        }

	/* No matching template has been found so return the first template */
	return lgl_template_dup ((lglTemplate *) model->templates->data);
}


/**
 * lgl_db_lookup_template_from_brand_part:
 * @brand: brand name string
 * @part:  part name string
 *
 * Lookup template in template database from brand and part strings.
 *
 * Returns: pointer to a newly allocated #lglTemplate structure.
 *
 */
lglTemplate *
lgl_db_lookup_template_from_brand_part(const gchar *brand,
                                       const gchar *part)
{
        gchar            *name;
	GList            *p_alias;
	lglTemplate      *template;
        lglTemplateAlias *alias;
        gchar            *candidate_name;
	lglTemplate      *new_template;

	if (!model)
        {
		lgl_db_init ();
	}

	if ((brand == NULL) || (part == NULL))
        {
		/* If no name, return first template as a default */
		return lgl_template_dup ((lglTemplate *) model->templates->data);
	}

        name = g_strdup_printf ("%s %s", brand, part);
        template = g_hash_table_lookup (model->template_cache, name);

        if (template)
        {
                for (p_alias = template->aliases; p_alias != NULL; p_alias = p_alias->next)
                {
                        alias = (lglTemplateAlias *)p_alias->data;
                        candidate_name = g_strdup_printf ("%s %s", alias->brand, alias->part);

                        if ( UTF8_EQUAL (candidate_name, name) )
                        {
                                g_free (candidate_name);
                                new_template = lgl_template_dup (template);
                                g_free (new_template->brand);
                                new_template->brand = g_strdup (alias->brand);
                                g_free (new_template->part);
                                new_template->part = g_strdup (alias->part);
                                g_free (name);
                                return new_template;
                        }

                        g_free (candidate_name);
                }
        }

	/* No matching template has been found so return the first template */
        g_free (name);
	return lgl_template_dup ((lglTemplate *) model->templates->data);
}


static void
add_to_template_cache (lglTemplate *template)
{
	GList            *p_alias;
        lglTemplateAlias *alias;
        gchar            *name;

        for ( p_alias=template->aliases; p_alias != NULL; p_alias=p_alias->next )
        {
                alias = (lglTemplateAlias *)p_alias->data;
                name = g_strdup_printf ("%s %s", alias->brand, alias->part);

                g_hash_table_insert (model->template_cache, name, template);
        }
}


void
read_templates (void)
{
	gchar       *data_dir;
        GList       *p;
        lglTemplate *template;

        /*
         * User defined templates.  Add to user-defined category.
         */
	data_dir = USER_CONFIG_DIR;
	read_template_files_from_dir (data_dir);
	g_free (data_dir);
        for ( p=model->templates; p != NULL; p=p->next )
        {
                template = (lglTemplate *)p->data;
                lgl_template_add_category (template, "user-defined");
        }

        /*
         * System templates.
         */
	data_dir = SYSTEM_CONFIG_DIR;
	read_template_files_from_dir (data_dir);
	g_free (data_dir);

	if (model->templates == NULL)
        {
		g_critical (_("Unable to locate any template files.  Libglabels may not be installed correctly!"));
	}
}


void
read_template_files_from_dir (const gchar *dirname)
{
	GDir        *dp;
	const gchar *filename, *extension, *extension2;
	gchar       *full_filename = NULL;
	GError      *gerror = NULL;

	if (dirname == NULL)
		return;

	if (!g_file_test (dirname, G_FILE_TEST_EXISTS))
        {
		return;
	}

	dp = g_dir_open (dirname, 0, &gerror);
	if (gerror != NULL)
        {
	        g_message ("cannot open data directory: %s", gerror->message );
		return;
	}

	while ((filename = g_dir_read_name (dp)) != NULL)
        {

		extension = strrchr (filename, '.');
		extension2 = strrchr (filename, '-');

		if ( (extension && ASCII_EQUAL (extension, ".template")) ||
		     (extension2 && ASCII_EQUAL (extension2, "-templates.xml")) )
                {

			full_filename = g_build_filename (dirname, filename, NULL);
                        lgl_xml_template_read_templates_from_file (full_filename);
			g_free (full_filename);
		}

	}

	g_dir_close (dp);
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
	desc = g_strdup_printf (_("%s full page label"), paper->name);

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


/*
 * Migrate custom files from the old ~/.glabels location to the new
 * ~/.config/libglabels/templates directory.  Place a timestamp file
 * in the old directory to prevent future copies.
 */
static void
copy_old_custom_files (void)
{
        gchar       *old_dir_name;
        gchar       *new_dir_name;
        gchar       *timestamp_file_name;
        GDir        *dir;
        const gchar *name;
        gchar       *old_full_name, *new_full_name;
        gchar       *contents;
        gsize        length;

        old_dir_name = USER_CONFIG_DIR_OLD22;
        timestamp_file_name = g_build_filename (old_dir_name, ".copied_to_30", NULL);

        if ( g_file_test (old_dir_name, G_FILE_TEST_EXISTS) &&
             !g_file_test (timestamp_file_name, G_FILE_TEST_EXISTS) )
        {

                new_dir_name = USER_CONFIG_DIR;
		g_mkdir_with_parents (new_dir_name, 0775); /* Try to make sure directory exists. */

                dir = g_dir_open (old_dir_name, 0, NULL);

                if (dir)
                {
                        while (name = g_dir_read_name (dir))
                        {
                                old_full_name = g_build_filename (old_dir_name, name, NULL);
                                new_full_name = g_build_filename (new_dir_name, name, NULL);

                                if ( g_file_get_contents (old_full_name, &contents, &length, NULL) )
                                {
                                        g_file_set_contents (new_full_name, contents, length, NULL);
                                        g_free (contents);
                                }

                                g_free (old_full_name);
                                g_free (new_full_name);
                        }

                        g_dir_close (dir);

                        g_file_set_contents (timestamp_file_name, NULL, 0, NULL);
                }

                g_free (new_dir_name);
        }

        g_free (timestamp_file_name);
        g_free (old_dir_name);
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

        if (!model)
        {
                lgl_db_init ();
        }

	g_print ("%s():\n", __FUNCTION__);
	for (p=model->templates; p!=NULL; p=p->next)
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

        if (!model)
        {
                lgl_db_init ();
        }

	g_print ("%s():\n", __FUNCTION__);
	for (p=template->aliases; p!=NULL; p=p->next)
        {
                alias = (lglTemplateAlias *)p->data;
		
		g_print("Alias: brand=\"%s\", part=\"%s\"\n", alias->brand, alias->part);

	}
	g_print ("\n");

}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
