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
 * lgl_paper_new:
 * @id:       Id of paper definition. (E.g. US-Letter, A4, etc.)  Should be
 *            unique.
 * @name:     Localized name of paper.
 * @width:    Width of paper in points.
 * @height:   Height of paper in points.
 * @pwg_size: PWG 5101.1-2002 size name.
 *
 * Allocates and constructs a new #lglPaper structure.
 *
 * Returns: a pointer to a newly allocated #lglPaper structure.
 *
 */
lglPaper *
lgl_paper_new (gchar             *id,
	       gchar             *name,
	       gdouble            width,
	       gdouble            height,
	       gchar             *pwg_size)
{
	lglPaper *paper;

	paper           = g_new0 (lglPaper,1);

	paper->id       = g_strdup (id);
	paper->name     = g_strdup (name);
	paper->width    = width;
	paper->height   = height;
	paper->pwg_size = g_strdup (pwg_size);

	return paper;
}


/**
 * lgl_paper_dup:
 * @orig:  #lglPaper structure to be duplicated.
 *
 * Duplicates an existing #lglPaper structure.
 *
 * Returns: a pointer to a newly allocated #lglPaper structure.
 *
 */
lglPaper *lgl_paper_dup (const lglPaper *orig)
{
	lglPaper       *paper;

	g_return_val_if_fail (orig, NULL);

	paper = g_new0 (lglPaper,1);

	paper->id       = g_strdup (orig->id);
	paper->name     = g_strdup (orig->name);
	paper->width    = orig->width;
	paper->height   = orig->height;
	paper->pwg_size = g_strdup (orig->pwg_size);

	return paper;
}


/**
 * lgl_paper_free:
 * @paper:  pointer to #lglPaper structure to be freed.
 *
 * Free all memory associated with an existing #lglPaper structure.
 *
 */
void lgl_paper_free (lglPaper *paper)
{

	if ( paper != NULL ) {

		g_free (paper->id);
		paper->id = NULL;

		g_free (paper->name);
		paper->name = NULL;

		g_free (paper->pwg_size);
		paper->pwg_size = NULL;

		g_free (paper);
	}

}


