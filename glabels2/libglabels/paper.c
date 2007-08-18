/*
 *  (LIBGLABELS) Template library for GLABELS
 *
 *  paper.c:  paper module
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
#include <config.h>

#include "paper.h"

#include <glib/gi18n.h>
#include <glib/gmem.h>
#include <glib/gstrfuncs.h>
#include <glib/gdir.h>
#include <glib/gmessages.h>
#include <string.h>

#include "libglabels-private.h"

#include "xml-paper.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GList *papers = NULL;

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static GList *read_papers (void);
static GList *read_paper_files_from_dir (GList       *papers,
					 const gchar *dirname);

/**
 * gl_paper_init:
 *
 * Initialize libglabels paper module by reading all paper definition
 * files located in system and user template directories.
 */
void
gl_paper_init (void)
{
	glPaper *other;

	if (papers) {
		return; /* Already initialized. */
	}

	papers = read_papers ();

	/* Create and append an "Other" entry. */
	other = gl_paper_new ("Other", _("Other"), 0.0, 0.0);
	papers = g_list_append (papers, other);
}


/**
 * gl_paper_new:
 * @id:     Id of paper definition. (E.g. US-Letter, A4, etc.)  Should be
 *          unique.
 * @name:   Localized name of paper.
 * @width:  Width of paper in points.
 * @height: Height of paper in points.
 *
 * Allocates and constructs a new #glPaper structure.
 *
 * Returns: a pointer to a newly allocated #glPaper structure.
 *
 */
glPaper *
gl_paper_new (gchar             *id,
	      gchar             *name,
	      gdouble            width,
	      gdouble            height)
{
	glPaper *paper;

	paper         = g_new0 (glPaper,1);
	paper->id     = g_strdup (id);
	paper->name   = g_strdup (name);
	paper->width  = width;
	paper->height = height;

	return paper;
}


/**
 * gl_paper_dup:
 * @orig:  #glPaper structure to be duplicated.
 *
 * Duplicates an existing #glPaper structure.
 *
 * Returns: a pointer to a newly allocated #glPaper structure.
 *
 */
glPaper *gl_paper_dup (const glPaper *orig)
{
	glPaper       *paper;

	g_return_val_if_fail (orig, NULL);

	paper = g_new0 (glPaper,1);

	paper->id     = g_strdup (orig->id);
	paper->name   = g_strdup (orig->name);
	paper->width  = orig->width;
	paper->height = orig->height;

	return paper;
}


/**
 * gl_paper_free:
 * @paper:  pointer to #glPaper structure to be freed.
 *
 * Free all memory associated with an existing #glPaper structure.
 *
 */
void gl_paper_free (glPaper *paper)
{

	if ( paper != NULL ) {

		g_free (paper->id);
		paper->id = NULL;

		g_free (paper->name);
		paper->name = NULL;

		g_free (paper);
	}

}


/**
 * gl_paper_get_id_list:
 *
 * Get a list of all paper ids known to libglabels.
 *
 * Returns: a list of paper ids.
 *
 */
GList *
gl_paper_get_id_list (void)
{
	GList           *ids = NULL;
	GList           *p;
	glPaper         *paper;

	if (!papers) {
		gl_paper_init ();
	}

	for ( p=papers; p != NULL; p=p->next ) {
		paper = (glPaper *)p->data;
		ids = g_list_append (ids, g_strdup (paper->id));
	}

	return ids;
}

/**
 * gl_paper_free_id_list:
 * @ids: List of id strings to be freed.
 *
 * Free up all storage associated with an id list obtained with
 * gl_paper_get_id_list().
 *
 */
void
gl_paper_free_id_list (GList *ids)
{
	GList *p;

	for (p = ids; p != NULL; p = p->next) {
		g_free (p->data);
		p->data = NULL;
	}

	g_list_free (ids);
}


/**
 * gl_paper_get_name_list:
 *
 * Get a list of all localized paper names known to libglabels.
 *
 * Returns: a list of localized paper names.
 *
 */
GList *
gl_paper_get_name_list (void)
{
	GList           *names = NULL;
	GList           *p;
	glPaper         *paper;

	if (!papers) {
		gl_paper_init ();
	}

	for ( p=papers; p != NULL; p=p->next ) {
		paper = (glPaper *)p->data;
		names = g_list_append (names, g_strdup (paper->name));
	}

	return names;
}


/**
 * gl_paper_free_name_list:
 * @names: List of localized paper name strings to be freed.
 *
 * Free up all storage associated with a name list obtained with
 * gl_paper_get_name_list().
 *
 */
void
gl_paper_free_name_list (GList *names)
{
	GList *p;

	for (p = names; p != NULL; p = p->next) {
		g_free (p->data);
		p->data = NULL;
	}

	g_list_free (names);
}


/**
 * gl_paper_is_id_known:
 * @id: paper id to test
 *
 * Determine if given paper id is known to libglabels.
 *
 * Returns: TRUE if id is known, otherwise FALSE.
 *
 */
gboolean
gl_paper_is_id_known (const gchar *id)
{
	GList       *p;
	glPaper     *paper;

	if (!papers) {
		gl_paper_init ();
	}

	if (id == NULL) {
		return FALSE;
	}

	for (p = papers; p != NULL; p = p->next) {
		paper = (glPaper *) p->data;
		if (g_strcasecmp (paper->id, id) == 0) {
			return TRUE;
		}
	}

	return FALSE;
}


/**
 * gl_paper_is_id_other:
 * @id: paper id to test
 *
 * Determine if given paper id is the special id "Other."
 *
 * Returns: TRUE if id is "Other", otherwise FALSE.
 *
 */
gboolean
gl_paper_is_id_other (const gchar *id)
{
	if (id == NULL) {
		return FALSE;
	}

	return (g_strcasecmp (id, "Other") == 0);
}


/**
 * gl_paper_from_id:
 * @id: paper id string
 *
 * Lookup paper definition from id string.
 *
 * Returns: pointer to a newly allocated #glPaper structure.
 *
 */
glPaper *
gl_paper_from_id (const gchar *id)
{
	GList       *p;
	glPaper     *paper;

	if (!papers) {
		gl_paper_init ();
	}

	if (id == NULL) {
		/* If no id, return first paper as a default */
		return gl_paper_dup ((glPaper *) papers->data);
	}

	for (p = papers; p != NULL; p = p->next) {
		paper = (glPaper *) p->data;
		if (g_strcasecmp (paper->id, id) == 0) {
			return gl_paper_dup (paper);
		}
	}

	return NULL;
}


/**
 * gl_paper_from_name:
 * @name: localized paper name string
 *
 * Lookup paper definition from localized paper name string.
 *
 * Returns: pointer to a newly allocated #glPaper structure.
 *
 */
glPaper *
gl_paper_from_name (const gchar *name)
{
	GList       *p;
	glPaper     *paper;

	if (!papers) {
		gl_paper_init ();
	}

	if (name == NULL) {
		/* If no name, return first paper as a default */
		return gl_paper_dup ((glPaper *) papers->data);
	}

	for (p = papers; p != NULL; p = p->next) {
		paper = (glPaper *) p->data;
		if (g_strcasecmp (paper->name, name) == 0) {
			return gl_paper_dup (paper);
		}
	}

	return NULL;
}


/**
 * gl_paper_lookup_id_from_name:
 * @name: localized paper name stringp
 *
 * Lookup paper name string from localized paper name string.
 *
 * Returns: pointer to a newly allocated id string.
 *
 */
gchar *
gl_paper_lookup_id_from_name (const gchar       *name)
{
	glPaper *paper = NULL;
	gchar   *id = NULL;

	if (name != NULL)
	{
		paper = gl_paper_from_name (name);
		if ( paper != NULL )
		{
			id = g_strdup (paper->id);
			gl_paper_free (paper);
			paper = NULL;
		}
	}

	return id;
}

/**
 * gl_paper_lookup_name_from_id:
 * @id: paper id string
 *
 * Lookup localized paper name string from paper id string.
 *
 * Returns: pointer to a newly allocated localized paper name string.
 *
 */
gchar *
gl_paper_lookup_name_from_id (const gchar       *id)
{
	glPaper *paper = NULL;
	gchar   *name = NULL;

	if (id != NULL)
	{
		paper = gl_paper_from_id (id);
		if ( paper != NULL )
		{
			name = g_strdup (paper->name);
			gl_paper_free (paper);
			paper = NULL;
		}
	}

	return name;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Read papers from various  files.                               */
/*--------------------------------------------------------------------------*/
static GList *
read_papers (void)
{
	gchar *data_dir;
	GList *papers = NULL;

	data_dir = GL_SYSTEM_DATA_DIR;
	papers = read_paper_files_from_dir (papers, data_dir);
	g_free (data_dir);

	data_dir = GL_USER_DATA_DIR;
	papers = read_paper_files_from_dir (papers, data_dir);
	g_free (data_dir);

	if (papers == NULL) {
		g_critical (_("Unable to locate paper size definitions.  Libglabels may not be installed correctly!"));
	}

	return papers;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Read all paper files from given directory.  Append to list.    */
/*--------------------------------------------------------------------------*/
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

			if ( (g_strcasecmp (extension, ".paper") == 0)
			     || (g_strcasecmp (filename, "paper-sizes.xml") == 0) ) {

				full_filename =
				    g_build_filename (dirname, filename, NULL);
				new_papers =
				    gl_xml_paper_read_papers_from_file (full_filename);
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
 * gl_paper_print_known_papers:
 *
 * For debugging purposes: print a list of all paper definitions known to
 * libglabels.
 *
 */
void
gl_paper_print_known_papers (void)
{
	GList       *p;
	glPaper     *paper;

	if (!papers) {
		gl_paper_init ();
	}

	g_print ("%s():\n", __FUNCTION__);
	for (p = papers; p != NULL; p = p->next) {
		paper = (glPaper *) p->data;

		g_print ("PAPER id=\"%s\", name=\"%s\", width=%gpts, height=%gpts\n",
			 paper->id, paper->name, paper->width, paper->height);

	}
	g_print ("\n");

}


