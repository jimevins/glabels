/*
 *  paper.c
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

#include "paper.h"

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



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
