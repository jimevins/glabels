/*
 *  prefs.c
 *  Copyright (C) 2001-2009  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of gLabels.
 *
 *  gLabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gLabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gLabels.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "prefs.h"

#include <glib/gi18n.h>
#include <libglabels/libglabels.h>

#include "debug.h"

glPrefsModel      *gl_prefs     = NULL;

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/


/*========================================================*/
/* Private types.                                         */
/*========================================================*/


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/



/*****************************************************************************/
/* Initialize preferences module.                                            */
/*****************************************************************************/
void 
gl_prefs_init (void)
{
	gl_debug (DEBUG_PREFS, "START");

	gl_prefs = gl_prefs_model_new ();

	gl_prefs_model_load_settings (gl_prefs);

	gl_debug (DEBUG_PREFS, "END");
}


/****************************************************************************/
/* Get string representing desired units.                                   */
/****************************************************************************/
const gchar *
gl_prefs_get_page_size (void)
{
	return (gl_prefs->default_page_size);
}

/****************************************************************************/
/* Get desired units.                                                       */
/****************************************************************************/
lglUnitsType
gl_prefs_get_units (void)
{
	return gl_prefs->units;
}

/****************************************************************************/
/* Get desired units per point.                                             */
/****************************************************************************/
gdouble
gl_prefs_get_units_per_point (void)
{

	switch (gl_prefs->units) {
	case LGL_UNITS_POINT:
		return 1.0;	/* points */
	case LGL_UNITS_INCH:
		return 1.0 / 72.0;	/* inches */
	case LGL_UNITS_MM:
		return 0.35277778;	/* mm */
	default:
		g_message ("Illegal units");	/* Should not happen */
		return 1.0;
	}
}

/****************************************************************************/
/* Get precision for desired units.                                         */
/****************************************************************************/
gint
gl_prefs_get_units_precision (void)
{

	switch (gl_prefs->units) {
	case LGL_UNITS_POINT:
		return 1;	/* points */
	case LGL_UNITS_INCH:
		return 3;	/* inches */
	case LGL_UNITS_MM:
		return 1;	/* mm */
	default:
		g_message ("Illegal units");	/* Should not happen */
		return 1.0;
	}
}

/****************************************************************************/
/* Get step size for desired units.                                         */
/****************************************************************************/
gdouble
gl_prefs_get_units_step_size (void)
{

	switch (gl_prefs->units) {
	case LGL_UNITS_POINT:
		return 0.1;	/* points */
	case LGL_UNITS_INCH:
		return 0.001;	/* inches */
	case LGL_UNITS_MM:
		return 0.1;	/* mm */
	default:
		g_message ("Illegal units");	/* Should not happen */
		return 1.0;
	}
}

/****************************************************************************/
/* Get string representing desired units.                                   */
/****************************************************************************/
const gchar *
gl_prefs_get_units_string (void)
{

	switch (gl_prefs->units) {
	case LGL_UNITS_POINT:
		return _("points");
	case LGL_UNITS_INCH:
		return _("inches");
	case LGL_UNITS_MM:
		return _("mm");
	default:
		g_message ("Illegal units");	/* Should not happen */
		return _("points");
	}
}

/****************************************************************************/
/* Add template to recent template list.                                    */
/****************************************************************************/
void
gl_prefs_add_recent_template (const gchar *name)
{
        GSList *p;

        /*
         * If already in list, remove that entry.
         */
        p = g_slist_find_custom (gl_prefs->recent_templates,
                                 name,
                                 (GCompareFunc)lgl_str_utf8_casecmp);
        if (p)
        {
                gl_prefs->recent_templates =
                        g_slist_remove_link (gl_prefs->recent_templates, p);
                g_free (p->data);
                g_slist_free_1 (p);
        }

        /*
         * Now prepend to list.
         */
        gl_prefs->recent_templates =
                g_slist_prepend (gl_prefs->recent_templates, g_strdup (name));

        /*
         * Truncate list to maximum size
         */
        while (g_slist_length (gl_prefs->recent_templates) > gl_prefs->max_recent_templates)
        {
                p = g_slist_last (gl_prefs->recent_templates);
                gl_prefs->recent_templates =
                        g_slist_remove_link (gl_prefs->recent_templates, p);
                g_free (p->data);
                g_slist_free_1 (p);
        }

        /*
         * Sync to disk.
         */
        gl_prefs_model_save_settings (gl_prefs);
}




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
