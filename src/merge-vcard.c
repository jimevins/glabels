/*
 *  merge-vcard.c
 *  Copyright (C) 2001-2009  Jim Evins <evins@snaught.com>.
 *  and
 *  Copyright (C) 2005  Austin Henry <ahenry@users.sourceforge.net>
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

#ifdef HAVE_LIBEBOOK


#include "merge-vcard.h"

#include <libebook/e-contact.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "debug.h"


/*===========================================*/
/* Private types                             */
/*===========================================*/

struct _glMergeVCardPrivate {
        FILE        *fp;
};

enum {
        LAST_SIGNAL
};

enum {
        ARG_0,
};


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void           gl_merge_vcard_finalize        (GObject          *object);

static void           gl_merge_vcard_set_property    (GObject          *object,
                                                      guint             param_id,
                                                      const GValue     *value,
                                                      GParamSpec       *pspec);

static void           gl_merge_vcard_get_property    (GObject          *object,
                                                      guint             param_id,
                                                      GValue           *value,
                                                      GParamSpec       *pspec);

static GList         *gl_merge_vcard_get_key_list    (const glMerge    *merge);
static gchar         *gl_merge_vcard_get_primary_key (const glMerge    *merge);
static void           gl_merge_vcard_open            (glMerge          *merge);
static void           gl_merge_vcard_close           (glMerge          *merge);
static glMergeRecord *gl_merge_vcard_get_record      (glMerge          *merge);
static void           gl_merge_vcard_copy            (glMerge          *dst_merge,
                                                      const glMerge    *src_merge);
static char *         parse_next_vcard               (FILE             *fp);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glMergeVCard, gl_merge_vcard, GL_TYPE_MERGE);


static void
gl_merge_vcard_class_init (glMergeVCardClass *class)
{
        GObjectClass *object_class = G_OBJECT_CLASS (class);
        glMergeClass *merge_class  = GL_MERGE_CLASS (class);

        gl_debug (DEBUG_MERGE, "START");

        gl_merge_vcard_parent_class = g_type_class_peek_parent (class);

        object_class->set_property = gl_merge_vcard_set_property;
        object_class->get_property = gl_merge_vcard_get_property;

        object_class->finalize = gl_merge_vcard_finalize;

        merge_class->get_key_list    = gl_merge_vcard_get_key_list;
        merge_class->get_primary_key = gl_merge_vcard_get_primary_key;
        merge_class->open            = gl_merge_vcard_open;
        merge_class->close           = gl_merge_vcard_close;
        merge_class->get_record      = gl_merge_vcard_get_record;
        merge_class->copy            = gl_merge_vcard_copy;

        gl_debug (DEBUG_MERGE, "END");
}


static void
gl_merge_vcard_init (glMergeVCard *merge_vcard)
{
        gl_debug (DEBUG_MERGE, "START");

        merge_vcard->priv = g_new0 (glMergeVCardPrivate, 1);

        gl_debug (DEBUG_MERGE, "END");
}


static void
gl_merge_vcard_finalize (GObject *object)
{
        glMergeVCard *merge_vcard = GL_MERGE_VCARD (object);

        gl_debug (DEBUG_MERGE, "START");

        g_return_if_fail (object && GL_IS_MERGE_VCARD (object));

        g_free (merge_vcard->priv);

        G_OBJECT_CLASS (gl_merge_vcard_parent_class)->finalize (object);

        gl_debug (DEBUG_MERGE, "END");
}


/*--------------------------------------------------------------------------*/
/* Set argument.                                                            */
/*--------------------------------------------------------------------------*/
static void
gl_merge_vcard_set_property (GObject      *object,
                             guint         param_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
        glMergeVCard *merge_vcard;

        merge_vcard = GL_MERGE_VCARD (object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
                break;
        }
}


/*--------------------------------------------------------------------------*/
/* Get argument.                                                            */
/*--------------------------------------------------------------------------*/
static void
gl_merge_vcard_get_property (GObject     *object,
                             guint        param_id,
                             GValue      *value,
                             GParamSpec  *pspec)
{
        glMergeVCard *merge_vcard;

        merge_vcard = GL_MERGE_VCARD (object);

        switch (param_id) {
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
                break;
        }

}


/*--------------------------------------------------------------------------*/
/* Get key list.                                                            */
/*--------------------------------------------------------------------------*/
static GList *
gl_merge_vcard_get_key_list (const glMerge *merge)
{
        glMergeVCard   *merge_vcard;
        GList          *key_list = NULL;
        EContactField   field_id;
        
        gl_debug (DEBUG_MERGE, "BEGIN");

        merge_vcard = GL_MERGE_VCARD (merge);

        for ( field_id = E_CONTACT_FIELD_FIRST; field_id <= E_CONTACT_LAST_SIMPLE_STRING; field_id++ )
        {
                key_list = g_list_append (key_list, g_strdup (e_contact_pretty_name (field_id)));
        }

        gl_debug (DEBUG_MERGE, "END");

        return key_list;
}


/*--------------------------------------------------------------------------*/
/* Get "primary" key.                                                       */
/*--------------------------------------------------------------------------*/
static gchar *
gl_merge_vcard_get_primary_key (const glMerge *merge)
{
        return g_strdup (e_contact_pretty_name(E_CONTACT_FILE_AS));
}


/*--------------------------------------------------------------------------*/
/* Open merge source.                                                       */
/*--------------------------------------------------------------------------*/
static void
gl_merge_vcard_open (glMerge *merge)
{
        glMergeVCard *merge_vcard;
        gchar        *src;

        merge_vcard = GL_MERGE_VCARD (merge);

        src = gl_merge_get_src (merge);

        if (src != NULL) {
                merge_vcard->priv->fp = fopen (src, "r");
        }

        g_free (src);

        return;
}


/*--------------------------------------------------------------------------*/
/* Close merge source.                                                      */
/*--------------------------------------------------------------------------*/
static void
gl_merge_vcard_close (glMerge *merge)
{
        glMergeVCard *merge_vcard;

        merge_vcard = GL_MERGE_VCARD (merge);

        if (merge_vcard->priv->fp != NULL) {
                fclose (merge_vcard->priv->fp);
                merge_vcard->priv->fp = NULL;
        }
}


/*--------------------------------------------------------------------------*/
/* Get next record from merge source, NULL if no records left (i.e EOF)     */
/*--------------------------------------------------------------------------*/
static glMergeRecord *
gl_merge_vcard_get_record (glMerge *merge)
{
        glMergeVCard  *merge_vcard;
        glMergeRecord *record;
        EContactField  field_id;
        glMergeField  *field;

        char *vcard;
        EContact *contact;

        merge_vcard = GL_MERGE_VCARD (merge);

        vcard = parse_next_vcard(merge_vcard->priv->fp);
        if (vcard == NULL || vcard[0] == '\0') {
                return NULL; /* EOF */
        }
        contact = e_contact_new_from_vcard(vcard);
        if (contact == NULL) {
                return NULL; /* invalid vcard */
        }

        record = g_new0 (glMergeRecord, 1);
        record->select_flag = TRUE;

        /* Take the interesting fields one by one from the contact, and put them
         * into the glMergeRecord structure. When done, free up the resources for
         * that contact */

        for ( field_id = E_CONTACT_FIELD_FIRST; field_id <= E_CONTACT_LAST_SIMPLE_STRING; field_id++ )
        {
                gchar *value;
                value = g_strdup (e_contact_get_const (contact, field_id));

                if (value) {
                        field = g_new0 (glMergeField, 1);
                        field->key = g_strdup (e_contact_pretty_name (field_id));
                        field->value = value;
                        record->field_list = g_list_prepend (record->field_list, field);
                }
        }

        record->field_list = g_list_reverse (record->field_list);


        /* free the contact */
        g_object_unref (contact);
        g_free(vcard);

        return record;
}


/*---------------------------------------------------------------------------*/
/* Copy merge_vcard specific fields.                                         */
/*---------------------------------------------------------------------------*/
static void
gl_merge_vcard_copy (glMerge       *dst_merge,
                     const glMerge *src_merge)
{
        glMergeVCard *dst_merge_vcard;
        glMergeVCard *src_merge_vcard;

        dst_merge_vcard = GL_MERGE_VCARD (dst_merge);
        src_merge_vcard = GL_MERGE_VCARD (src_merge);
}


/*---------------------------------------------------------------------------*/
/* PRIVATE: pull out a full VCard from the open file                         */
/* Arguments:                                                                */
/*  fp - an open stream to parse in put from                                 */
/* Returns:                                                                  */
/*  a pointer to the buffer containing the vcard, the empty string on        */
/*  end-of-file or error, this buffer needs to be free by the caller         */
/*---------------------------------------------------------------------------*/
static char * 
parse_next_vcard (FILE *fp)
{
        gboolean  found_begin = FALSE;
        gboolean  found_end = FALSE;
        char     *vcard;
        char      line[512];
        int       size = 2048;
        int       cursize = 0;

        /* if no source has been set up, don't try to read from the file */
        if (!fp) {
                return NULL;
        }

        vcard = g_malloc0(size);

        while (fgets(line, sizeof(line), fp) && found_end == FALSE)
        {
                if (found_begin == TRUE)
                {
                        if (g_ascii_strncasecmp(line, "END:VCARD", strlen("END:VCARD")) == 0)
                        {
                                found_end = TRUE;
                        }
                }
                else
                {
                        if (g_ascii_strncasecmp(line, "BEGIN:VCARD", strlen("BEGIN:VCARD")) == 0)
                        {
                                found_begin = TRUE;
                        } 
                        else
                        {
                                continue; /* skip lines not in a vcard */
                        }
                }

                /* if the buffer passed us isn't big enough, reallocate it */
                cursize += strlen(line);
                if (cursize >= size)
                {
                        size *= 2;
                        vcard = (char *)g_realloc(vcard, size); /* aborts program on error */
                }

                /* add the line (or portion thereof) to the vcard */
                strncat(vcard, line, size);
        }

        return vcard;
}



#endif /* HAVE_LIBEBOOK */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
