/*
 *  svg-cache.c
 *  Copyright (C) 2003-2009  Jim Evins <evins@snaught.com>.
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

#include <string.h>

#include "svg-cache.h"

#include "debug.h"


/*========================================================*/
/* Private types.                                         */
/*========================================================*/

typedef struct {
        gchar      *key;
        guint       references;
        RsvgHandle *svg_handle;
        gchar      *contents;
} CacheRecord;


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void  record_destroy   (gpointer val);

static void  add_name_to_list (gpointer key,
                               gpointer val,
                               gpointer user_data);


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Destroy cache record.                                           */
/*---------------------------------------------------------------------------*/
static void
record_destroy (gpointer val)
{
        CacheRecord *record = (CacheRecord *)val;

        g_return_if_fail (record);

        g_free (record->key);
        g_object_unref (record->svg_handle);
        g_free (record->contents);
        g_free (record);
}


/*****************************************************************************/
/* Create a new hash table to keep track of cached svgs.                     */
/*****************************************************************************/
GHashTable *
gl_svg_cache_new (void)
{
        GHashTable *svg_cache;

        gl_debug (DEBUG_SVG_CACHE, "START");

        svg_cache = g_hash_table_new_full (g_str_hash,
                                              g_str_equal,
                                              NULL,
                                              record_destroy);

        gl_debug (DEBUG_SVG_CACHE, "END svg_cache=%p", svg_cache);

        return svg_cache;
}


/*****************************************************************************/
/* Free up previously allocated hash table and its contents.                 */
/*****************************************************************************/
void
gl_svg_cache_free (GHashTable *svg_cache)
{
        gl_debug (DEBUG_SVG_CACHE, "START");

        g_hash_table_destroy (svg_cache);

        gl_debug (DEBUG_SVG_CACHE, "END");
}


/*****************************************************************************/
/* Add svg to cache explicitly.                                              */
/*****************************************************************************/
void
gl_svg_cache_add_svg (GHashTable  *svg_cache,
                      gchar       *name,
                      const gchar *contents)
{
        CacheRecord *test_record, *record;
        RsvgHandle  *svg_handle;

        gl_debug (DEBUG_SVG_CACHE, "START");

        test_record = g_hash_table_lookup (svg_cache, name);
        if (test_record != NULL) {
                /* svg is already in the cache. */
                gl_debug (DEBUG_SVG_CACHE, "END already in cache");
                return;
        }

        record = g_new0 (CacheRecord, 1);
        record->key        = g_strdup (name);
        record->references = 0; /* No references yet. */
        record->svg_handle = rsvg_handle_new_from_data (contents, strlen(contents), NULL);
        record->contents   = g_strdup (contents);

        g_hash_table_insert (svg_cache, record->key, record);

        gl_debug (DEBUG_SVG_CACHE, "END");
}


/*****************************************************************************/
/* Get svg handle adding a reference.  If not in cache, read it and add.     */
/*****************************************************************************/
RsvgHandle *
gl_svg_cache_get_handle (GHashTable *svg_cache,
                         gchar      *name)
{
        CacheRecord *record;
        RsvgHandle  *svg_handle = NULL;
        gchar       *buffer;
        gsize        length;
        GFile       *file;

        gl_debug (DEBUG_SVG_CACHE, "START svg_cache=%p", svg_cache);

        record = g_hash_table_lookup (svg_cache, name);

        if (record != NULL)
        {
                record->references++;
                gl_debug (DEBUG_SVG_CACHE, "references=%d", record->references);
                gl_debug (DEBUG_SVG_CACHE, "END cached");
                return record->svg_handle;
        }


        file = g_file_new_for_path (name);
        if ( g_file_load_contents (file, NULL, &buffer, &length, NULL, NULL) )
        {
                svg_handle = rsvg_handle_new_from_data (buffer, length, NULL);
                if ( svg_handle != NULL) {
                        record = g_new0 (CacheRecord, 1);
                        record->key        = g_strdup (name);
                        record->references = 1;
                        record->svg_handle = svg_handle;
                        record->contents   = buffer;

                        g_hash_table_insert (svg_cache, record->key, record);
                }
        }
        g_object_unref (file);

        gl_debug (DEBUG_SVG_CACHE, "END");

        return svg_handle;
}


/*****************************************************************************/
/* Get contents.   Do not add reference.                                     */
/*****************************************************************************/
gchar *
gl_svg_cache_get_contents (GHashTable *svg_cache,
                           gchar      *name)
{
        CacheRecord *record;
        RsvgHandle  *svg_handle = NULL;
        GFile       *file;

        gl_debug (DEBUG_SVG_CACHE, "START svg_cache=%p", svg_cache);

        record = g_hash_table_lookup (svg_cache, name);

        if (record != NULL)
        {
                gl_debug (DEBUG_SVG_CACHE, "references=%d", record->references);
                gl_debug (DEBUG_SVG_CACHE, "END cached");
                return g_strdup (record->contents);
        }

        gl_debug (DEBUG_SVG_CACHE, "END");

        return NULL;
}


/*****************************************************************************/
/* Remove svg, but only if no references left.                               */
/*****************************************************************************/
void
gl_svg_cache_remove_svg (GHashTable *svg_cache,
                         gchar      *name)
{
        CacheRecord *record;

        if (name == NULL) return;

        gl_debug (DEBUG_SVG_CACHE, "START");

        record = g_hash_table_lookup (svg_cache, name);
        if (record == NULL) {
                gl_debug (DEBUG_SVG_CACHE, "END not in cache");
                return;
        }

        record->references--;

        if ( record->references == 0 ) {
                g_hash_table_remove (svg_cache, name);
        }

        gl_debug (DEBUG_SVG_CACHE, "END");
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Add a name to a GList while iterating over cache.               */
/*---------------------------------------------------------------------------*/
static void
add_name_to_list (gpointer key,
                  gpointer val,
                  gpointer user_data)
{
        gchar     *name       = (gchar *)key;
        GList     **name_list = (GList **)user_data;

        gl_debug (DEBUG_SVG_CACHE, "START");

        gl_debug (DEBUG_SVG_CACHE, "adding name=%s", name);

        *name_list = g_list_append (*name_list, g_strdup(name));

        gl_debug (DEBUG_SVG_CACHE, "END");
}


/*****************************************************************************/
/* Return a list of names for all svgs in the cache.                         */
/*****************************************************************************/
GList *
gl_svg_cache_get_name_list (GHashTable *svg_cache)
{
        GList *name_list = NULL;

        gl_debug (DEBUG_SVG_CACHE, "START");

        g_hash_table_foreach (svg_cache, add_name_to_list, &name_list);

        gl_debug (DEBUG_SVG_CACHE, "END");

        return name_list;
}


/*****************************************************************************/
/* Free up a list of svg names.                                              */
/*****************************************************************************/
void
gl_svg_cache_free_name_list (GList *name_list)
{
        GList *p_name;

        gl_debug (DEBUG_SVG_CACHE, "START");

        for (p_name = name_list; p_name != NULL; p_name = p_name->next) {
                g_free (p_name->data);
                p_name->data = NULL;
        }

        g_list_free (name_list);

        gl_debug (DEBUG_SVG_CACHE, "END");
}




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
