/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  paper.c:  paper module
 *
 *  Copyright (C) 2003  Jim Evins <evins@snaught.com>.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <config.h>

#include <string.h>
#include <libgnome/libgnome.h>

#include "paper.h"
#include "xml-paper.h"
#include "util.h"

#include "debug.h"

#define GL_DATA_DIR gnome_program_locate_file (NULL,\
					 GNOME_FILE_DOMAIN_APP_DATADIR,\
					 "glabels",\
					 FALSE, NULL)


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


/*****************************************************************************/
/* Initialize module.                                                        */
/*****************************************************************************/
void
gl_paper_init (void)
{
	glPaper *other;

	gl_debug (DEBUG_PAPER, "START");

	papers = read_papers ();

	/* Create and append an "Other" entry. */
	other = g_new0 (glPaper,1);
	other->id   = g_strdup ("Other");
	other->name = g_strdup (_("Other"));
	papers = g_list_append (papers, other);

	gl_debug (DEBUG_PAPER, "END");
}

/*****************************************************************************/
/* Get a list of valid page size ids                                         */
/*****************************************************************************/
GList *
gl_paper_get_id_list (void)
{
	GList           *ids = NULL;
	GList           *p;
	glPaper         *paper;

	gl_debug (DEBUG_PAPER, "START");

	for ( p=papers; p != NULL; p=p->next ) {
		paper = (glPaper *)p->data;
		ids = g_list_append (ids, g_strdup (paper->id));
	}

	gl_debug (DEBUG_PAPER, "END");
	return ids;
}

/*****************************************************************************/
/* Free a list of page size ids.                                             */
/*****************************************************************************/
void
gl_paper_free_id_list (GList **ids)
{
	GList *p;

	gl_debug (DEBUG_PAPER, "START");

	for (p = *ids; p != NULL; p = p->next) {
		g_free (p->data);
		p->data = NULL;
	}

	g_list_free (*ids);
	*ids = NULL;

	gl_debug (DEBUG_PAPER, "END");
}

/*****************************************************************************/
/* Get a list of valid page size names                                       */
/*****************************************************************************/
GList *
gl_paper_get_name_list (void)
{
	GList           *names = NULL;
	GList           *p;
	glPaper         *paper;

	gl_debug (DEBUG_PAPER, "START");

	for ( p=papers; p != NULL; p=p->next ) {
		paper = (glPaper *)p->data;
		names = g_list_append (names, g_strdup (paper->name));
	}

	gl_debug (DEBUG_PAPER, "END");
	return names;
}

/*****************************************************************************/
/* Free a list of page size names.                                           */
/*****************************************************************************/
void
gl_paper_free_name_list (GList **names)
{
	GList *p;

	gl_debug (DEBUG_PAPER, "START");

	for (p = *names; p != NULL; p = p->next) {
		g_free (p->data);
		p->data = NULL;
	}

	g_list_free (*names);
	*names = NULL;

	gl_debug (DEBUG_PAPER, "END");
}

/*****************************************************************************/
/* Is page size id known?                                                    */
/*****************************************************************************/
gboolean
gl_paper_is_id_known (const gchar *id)
{
	GList       *p;
	glPaper     *paper;

	gl_debug (DEBUG_PAPER, "START");

	if (id == NULL) {
		gl_debug (DEBUG_PAPER, "END (false, id=NULL)");
		return FALSE;
	}

	for (p = papers; p != NULL; p = p->next) {
		paper = (glPaper *) p->data;
		if (g_strcasecmp (paper->id, id) == 0) {
			gl_debug (DEBUG_PAPER, "END (true)");
			return TRUE;
		}
	}

	gl_debug (DEBUG_PAPER, "END (false)");
	return FALSE;
}

/*****************************************************************************/
/* Return a paper structure from id.                                         */
/*****************************************************************************/
glPaper *
gl_paper_from_id (const gchar *id)
{
	GList       *p;
	glPaper     *paper;

	gl_debug (DEBUG_PAPER, "START");

	if (id == NULL) {
		/* If no id, return first paper as a default */
		return gl_paper_dup ((glPaper *) papers->data);
	}

	for (p = papers; p != NULL; p = p->next) {
		paper = (glPaper *) p->data;
		if (g_strcasecmp (paper->id, id) == 0) {
			gl_debug (DEBUG_PAPER, "END");
			return gl_paper_dup (paper);
		}
	}

	gl_debug (DEBUG_PAPER, "END");
	return NULL;
}

/*****************************************************************************/
/* Return a paper structure from name.                                       */
/*****************************************************************************/
glPaper *
gl_paper_from_name (const gchar *name)
{
	GList       *p;
	glPaper     *paper;

	gl_debug (DEBUG_PAPER, "START");

	if (name == NULL) {
		/* If no name, return first paper as a default */
		return gl_paper_dup ((glPaper *) papers->data);
	}

	for (p = papers; p != NULL; p = p->next) {
		paper = (glPaper *) p->data;
		if (g_strcasecmp (paper->name, name) == 0) {
			gl_debug (DEBUG_PAPER, "END");
			return gl_paper_dup (paper);
		}
	}

	gl_debug (DEBUG_PAPER, "END");
	return NULL;
}

/*****************************************************************************/
/* Lookup paper id from name.                                                */
/*****************************************************************************/
gchar *
gl_paper_lookup_id_from_name (const gchar       *name)
{
	glPaper *paper;
	gchar   *id = NULL;

	gl_debug (DEBUG_PAPER, "START");

	paper = gl_paper_from_name (name);
	if ( paper != NULL ) {
		id = g_strdup (paper->id);
		gl_paper_free (&paper);
	}

	gl_debug (DEBUG_PAPER, "END");
	return id;
}

/*****************************************************************************/
/* Lookup paper name from id.                                                */
/*****************************************************************************/
gchar *
gl_paper_lookup_name_from_id (const gchar       *id)
{
	glPaper *paper;
	gchar   *name = NULL;

	gl_debug (DEBUG_PAPER, "START");

	paper = gl_paper_from_id (id);
	if ( paper != NULL ) {
		name = g_strdup (paper->name);
		gl_paper_free (&paper);
	}

	gl_debug (DEBUG_PAPER, "END");
	return name;
}

/*****************************************************************************/
/* Copy a paper.                                                          */
/*****************************************************************************/
glPaper *gl_paper_dup (const glPaper *orig_paper)
{
	glPaper       *paper;

	gl_debug (DEBUG_PAPER, "START");

	paper = g_new0 (glPaper,1);

	paper->id     = g_strdup (orig_paper->id);
	paper->name   = g_strdup (orig_paper->name);
	paper->width  = orig_paper->width;
	paper->height = orig_paper->height;

	gl_debug (DEBUG_PAPER, "END");
	return paper;
}

/*****************************************************************************/
/* Free up a paper.                                                       */
/*****************************************************************************/
void gl_paper_free (glPaper **paper)
{
	GList *p;

	gl_debug (DEBUG_PAPER, "START");

	if ( *paper != NULL ) {

		g_free ((*paper)->id);
		(*paper)->name = NULL;

		g_free ((*paper)->name);
		(*paper)->name = NULL;

		g_free (*paper);
		*paper = NULL;

	}

	gl_debug (DEBUG_PAPER, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Read papers from various  files.                            */
/*--------------------------------------------------------------------------*/
static GList *
read_papers (void)
{
	gchar *home_data_dir = gl_util_get_home_data_dir ();
	GList *papers = NULL;

	gl_debug (DEBUG_PAPER, "START");

	papers = read_paper_files_from_dir (papers, GL_DATA_DIR);
	papers = read_paper_files_from_dir (papers, home_data_dir);

	g_free (home_data_dir);

	if (papers == NULL) {
		g_warning (_("No paper files found!"));
	}

	gl_debug (DEBUG_PAPER, "END");
	return papers;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Read all paper files from given directory.  Append to list. */
/*--------------------------------------------------------------------------*/
static GList *
read_paper_files_from_dir (GList       *papers,
			   const gchar *dirname)
{
	GDir        *dp;
	const gchar *filename, *extension;
	gchar       *full_filename = NULL;
	GError      *gerror = NULL;

	gl_debug (DEBUG_PAPER, "START");

	if (dirname == NULL)
		return papers;

	dp = g_dir_open (dirname, 0, &gerror);
	if (gerror != NULL) {
	        g_warning ("cannot open data directory: %s", gerror->message );
		gl_debug (DEBUG_PAPER, "END");
		return papers;
	}

	while ((filename = g_dir_read_name (dp)) != NULL) {

		extension = strrchr (filename, '.');

		if (extension != NULL) {

			if ( (g_strcasecmp (extension, ".paper") == 0)
			     || (g_strcasecmp (filename, "paper-sizes.xml") == 0) ) {

				full_filename =
				    g_build_filename (dirname, filename, NULL);
				papers =
				    gl_xml_paper_read_papers_from_file (papers,
									full_filename);
				g_free (full_filename);

			}

		}

	}

	g_dir_close (dp);

	gl_debug (DEBUG_PAPER, "END");
	return papers;
}

