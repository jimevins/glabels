/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  mini-preview-pixbuf-cache.c:  GLabels mini-preview pixbuf cache module
 *
 *  Copyright (C) 2007  Jim Evins <evins@snaught.com>.
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

#include "mini-preview-pixbuf-cache.h"
#include "mini-preview-pixbuf.h"

#include "libglabels/db.h"

#include <glib/gmem.h>
#include <glib/ghash.h>

#include "debug.h"

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static GHashTable *mini_preview_pixbuf_cache = NULL;

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/


/*****************************************************************************/
/* Create a new hash table to keep track of cached mini preview pixbufs.     */
/*****************************************************************************/
void
gl_mini_preview_pixbuf_cache_init (void)
{
        GList       *names = NULL;
        GList       *p;
        lglTemplate *template;

	gl_debug (DEBUG_PIXBUF_CACHE, "START");

	mini_preview_pixbuf_cache = g_hash_table_new (g_str_hash, g_str_equal);

        names = lgl_db_get_template_name_list_unique (NULL, NULL, NULL);
        for ( p=names; p != NULL; p=p->next )
        {
                gl_debug (DEBUG_PIXBUF_CACHE, "name = \"%s\"", p->data);

                template = lgl_db_lookup_template_from_name (p->data);
                gl_mini_preview_pixbuf_cache_add_by_template (template);
                lgl_template_free (template);
        }
        lgl_db_free_template_name_list (names);

	gl_debug (DEBUG_PIXBUF_CACHE, "END pixbuf_cache=%p", mini_preview_pixbuf_cache);
}

/*****************************************************************************/
/* Add pixbuf to cache by template.                                          */
/*****************************************************************************/
void
gl_mini_preview_pixbuf_cache_add_by_template (lglTemplate *template)
{
        GdkPixbuf        *pixbuf;
        GList            *p;
        lglTemplateAlias *alias;
        gchar            *name;

	gl_debug (DEBUG_PIXBUF_CACHE, "START");

        pixbuf = gl_mini_preview_pixbuf_new (template, 72, 72);

        for ( p=template->aliases; p != NULL; p=p->next )
        {
                alias = (lglTemplateAlias *)p->data;

                name = g_strdup_printf ("%s %s", alias->brand, alias->part);
                g_hash_table_insert (mini_preview_pixbuf_cache, name, g_object_ref (pixbuf));
        }

        g_object_unref (pixbuf);

	gl_debug (DEBUG_PIXBUF_CACHE, "END");
}

/*****************************************************************************/
/* Add pixbuf to cache by name.                                              */
/*****************************************************************************/
void
gl_mini_preview_pixbuf_cache_add_by_name (gchar      *name)
{
        lglTemplate *template;
        GdkPixbuf   *pixbuf;

	gl_debug (DEBUG_PIXBUF_CACHE, "START");

        template = lgl_db_lookup_template_from_name (name);
        pixbuf = gl_mini_preview_pixbuf_new (template, 72, 72);
        lgl_template_free (template);

        g_hash_table_insert (mini_preview_pixbuf_cache, g_strdup (name), pixbuf);

	gl_debug (DEBUG_PIXBUF_CACHE, "END");
}

/*****************************************************************************/
/* Get pixbuf.                                                               */
/*****************************************************************************/
GdkPixbuf *
gl_mini_preview_pixbuf_cache_get_pixbuf (gchar      *name)
{
	GdkPixbuf   *pixbuf;

	gl_debug (DEBUG_PIXBUF_CACHE, "START pixbuf_cache=%p", mini_preview_pixbuf_cache);

	pixbuf = g_hash_table_lookup (mini_preview_pixbuf_cache, name);

        if (!pixbuf)
        {
                gl_mini_preview_pixbuf_cache_add_by_name (name);
                pixbuf = g_hash_table_lookup (mini_preview_pixbuf_cache, name);
        }

	gl_debug (DEBUG_PIXBUF_CACHE, "END");

	return g_object_ref (pixbuf);
}

