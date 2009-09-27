/*
 *  font-util.h
 *  Copyright (C) 2009  Jim Evins <evins@snaught.com>.
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

#include "font-util.h"

#include <glib.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>

#include <libglabels/libglabels.h>


/****************************************************************************/
/* Get list of all available font families.                                 */
/****************************************************************************/
const GList  *
gl_font_util_get_all_families (void)
{
	static GList         *list = NULL;
	PangoFontMap         *fontmap;
	PangoContext         *context;
	PangoFontFamily     **families;
	gint                  n;
	gint                  i;
	gchar                *name;

        if ( !list )
        {
                fontmap = pango_cairo_font_map_new ();
                context = pango_cairo_font_map_create_context (PANGO_CAIRO_FONT_MAP (fontmap));

                pango_context_list_families (context, &families, &n);

                for ( i=0; i<n; i++ )
                {
                        name = g_strdup (pango_font_family_get_name (families[i]));
                        list = g_list_insert_sorted (list, name,
                                                     (GCompareFunc)lgl_str_utf8_casecmp);
                }

                g_free (families);

                g_object_unref (context);
                g_object_unref (fontmap);
        }

	return list;
}


/****************************************************************************/
/* Get list of all available proportional font families.                    */
/****************************************************************************/
const GList  *
gl_font_util_get_proportional_families (void)
{
	static GList         *list = NULL;
	PangoFontMap         *fontmap;
	PangoContext         *context;
	PangoFontFamily     **families;
	gint                  n;
	gint                  i;
	gchar                *name;

        if ( !list )
        {
                fontmap = pango_cairo_font_map_new ();
                context = pango_cairo_font_map_create_context (PANGO_CAIRO_FONT_MAP (fontmap));

                pango_context_list_families (context, &families, &n);

                for ( i=0; i<n; i++ )
                {
                        if ( !pango_font_family_is_monospace (families[i]) )
                        {
                                name = g_strdup (pango_font_family_get_name (families[i]));
                                list = g_list_insert_sorted (list, name,
                                                             (GCompareFunc)lgl_str_utf8_casecmp);
                        }
                }

                g_free (families);

                g_object_unref (context);
                g_object_unref (fontmap);
        }

	return list;
}


/****************************************************************************/
/* Get list of all available fixed-width font families.                     */
/****************************************************************************/
const GList  *
gl_font_util_get_fixed_width_families (void)
{
	static GList         *list = NULL;
	PangoFontMap         *fontmap;
	PangoContext         *context;
	PangoFontFamily     **families;
	gint                  n;
	gint                  i;
	gchar                *name;

        if ( !list )
        {
                fontmap = pango_cairo_font_map_new ();
                context = pango_cairo_font_map_create_context (PANGO_CAIRO_FONT_MAP (fontmap));

                pango_context_list_families (context, &families, &n);

                for ( i=0; i<n; i++ )
                {
                        if ( pango_font_family_is_monospace (families[i]) )
                        {
                                name = g_strdup (pango_font_family_get_name (families[i]));
                                list = g_list_insert_sorted (list, name,
                                                             (GCompareFunc)lgl_str_utf8_casecmp);
                        }
                }

                g_free (families);

                g_object_unref (context);
                g_object_unref (fontmap);
        }

	return list;
}


/****************************************************************************/
/* Make sure we have a valid font.  If not provide a good default.          */
/****************************************************************************/
gchar *
gl_font_util_validate_family (const gchar *family)
{
        const GList *installed_families;
        gchar       *good_family;

        installed_families = gl_font_util_get_all_families ();

        if (g_list_find_custom ((GList *)installed_families,
                                family,
                                (GCompareFunc)g_utf8_collate))
        {
                good_family = g_strdup (family);
        }
        else if (g_list_find_custom ((GList *)installed_families,
                                     "Sans",
                                     (GCompareFunc)g_utf8_collate))
        {
                good_family = g_strdup ("Sans");
        }
        else if (installed_families != NULL)
        {
                good_family = g_strdup (installed_families->data); /* 1st entry */
        }
        else
        {
                good_family = NULL;
        }

        return good_family;
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
