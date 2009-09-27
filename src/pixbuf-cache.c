/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  pixbuf-cache.c:  GLabels pixbuf cache module
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

#include "pixbuf-cache.h"

#include "debug.h"

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

typedef struct {
	gchar     *key;
	guint      references;
	GdkPixbuf *pixbuf;
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
	g_object_unref (record->pixbuf);
	g_free (record);
}

/*****************************************************************************/
/* Create a new hash table to keep track of cached pixbufs.                  */
/*****************************************************************************/
GHashTable *
gl_pixbuf_cache_new (void)
{
	GHashTable *pixbuf_cache;

	gl_debug (DEBUG_PIXBUF_CACHE, "START");

	pixbuf_cache = g_hash_table_new_full (g_str_hash,
					      g_str_equal,
					      NULL,
					      record_destroy);

	gl_debug (DEBUG_PIXBUF_CACHE, "END pixbuf_cache=%p", pixbuf_cache);

	return pixbuf_cache;
}

/*****************************************************************************/
/* Free up previously allocated hash table and its contents.                 */
/*****************************************************************************/
void
gl_pixbuf_cache_free (GHashTable *pixbuf_cache)
{
	gl_debug (DEBUG_PIXBUF_CACHE, "START");

	g_hash_table_destroy (pixbuf_cache);

	gl_debug (DEBUG_PIXBUF_CACHE, "END");
}

/*****************************************************************************/
/* Add pixbuf to cache explicitly (not a reference).                         */
/*****************************************************************************/
void
gl_pixbuf_cache_add_pixbuf (GHashTable *pixbuf_cache,
			    gchar      *name,
			    GdkPixbuf  *pixbuf)
{
	CacheRecord *test_record, *record;

	gl_debug (DEBUG_PIXBUF_CACHE, "START");

	test_record = g_hash_table_lookup (pixbuf_cache, name);
	if (test_record != NULL) {
		/* pixbuf is already in the cache. */
		gl_debug (DEBUG_PIXBUF_CACHE, "END already in cache");
		return;
	}

	record = g_new0 (CacheRecord, 1);
	record->key        = g_strdup (name);
	record->references = 0; /* Nobody has referenced it yet. */
	record->pixbuf     = g_object_ref (G_OBJECT (pixbuf));

	g_hash_table_insert (pixbuf_cache, record->key, record);

	gl_debug (DEBUG_PIXBUF_CACHE, "END");
}

/*****************************************************************************/
/* Get pixbuf.  If not in cache, read it and add to cache.                   */
/*****************************************************************************/
GdkPixbuf *
gl_pixbuf_cache_get_pixbuf (GHashTable *pixbuf_cache,
			    gchar      *name)
{
	CacheRecord *record;
	GdkPixbuf   *pixbuf;

	gl_debug (DEBUG_PIXBUF_CACHE, "START pixbuf_cache=%p", pixbuf_cache);

	record = g_hash_table_lookup (pixbuf_cache, name);

	if (record != NULL) {
		record->references++;
		gl_debug (DEBUG_PIXBUF_CACHE, "references=%d", record->references);
		gl_debug (DEBUG_PIXBUF_CACHE, "END cached");
		return record->pixbuf;
	}


	pixbuf = gdk_pixbuf_new_from_file (name, NULL);
	if ( pixbuf != NULL) {
		record = g_new0 (CacheRecord, 1);
		record->key        = g_strdup (name);
		record->references = 1;
		record->pixbuf     = pixbuf;

		g_hash_table_insert (pixbuf_cache, record->key, record);
	}

	gl_debug (DEBUG_PIXBUF_CACHE, "END");

	return pixbuf;
}

/*****************************************************************************/
/* Remove pixbuf, but only if no references left.                            */
/*****************************************************************************/
void
gl_pixbuf_cache_remove_pixbuf (GHashTable *pixbuf_cache,
			       gchar      *name)
{
	CacheRecord *record;

	if (name == NULL) return;

	gl_debug (DEBUG_PIXBUF_CACHE, "START");

	record = g_hash_table_lookup (pixbuf_cache, name);
	if (record == NULL) {
		gl_debug (DEBUG_PIXBUF_CACHE, "END not in cache");
		return;
	}

	record->references--;

	if ( record->references == 0 ) {
		g_hash_table_remove (pixbuf_cache, name);
	}

	gl_debug (DEBUG_PIXBUF_CACHE, "END");
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

	gl_debug (DEBUG_PIXBUF_CACHE, "START");

	gl_debug (DEBUG_PIXBUF_CACHE, "adding name=%s", name);

	*name_list = g_list_append (*name_list, g_strdup(name));

	gl_debug (DEBUG_PIXBUF_CACHE, "END");
}

/*****************************************************************************/
/* Return a list of names for all pixbufs in the cache.                      */
/*****************************************************************************/
GList *
gl_pixbuf_cache_get_name_list (GHashTable *pixbuf_cache)
{
	GList *name_list = NULL;

	gl_debug (DEBUG_PIXBUF_CACHE, "START");

	g_hash_table_foreach (pixbuf_cache, add_name_to_list, &name_list);

	gl_debug (DEBUG_PIXBUF_CACHE, "END");

	return name_list;
}

/*****************************************************************************/
/* Free up a list of pixbuf names.                                           */
/*****************************************************************************/
void
gl_pixbuf_cache_free_name_list (GList *name_list)
{
	GList *p_name;

	gl_debug (DEBUG_PIXBUF_CACHE, "START");

	for (p_name = name_list; p_name != NULL; p_name = p_name->next) {
		g_free (p_name->data);
		p_name->data = NULL;
	}

	g_list_free (name_list);

	gl_debug (DEBUG_PIXBUF_CACHE, "END");
}


