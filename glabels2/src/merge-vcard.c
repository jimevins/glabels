/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  merge_vcard.c:  vcard merge backend module
 *
 *  Copyright (C) 2001  Jim Evins <evins@snaught.com>.
 *  and
 *  Copyright (C) 2005  Austin Henry <ahenry@users.sourceforge.net>
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

static GList         *gl_merge_vcard_get_key_list    (glMerge          *merge);
static gchar         *gl_merge_vcard_get_primary_key (glMerge          *merge);
static void           gl_merge_vcard_open            (glMerge          *merge);
static void           gl_merge_vcard_close           (glMerge          *merge);
static glMergeRecord *gl_merge_vcard_get_record      (glMerge          *merge);
static void           gl_merge_vcard_copy            (glMerge          *dst_merge,
                                                      glMerge          *src_merge);
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

/* TODO */
/*--------------------------------------------------------------------------*/
/* Get key list.                                                            */
/*--------------------------------------------------------------------------*/
static GList *
gl_merge_vcard_get_key_list (glMerge *merge)
{
        glMergeVCard   *merge_vcard;
        GList          *key_list;
        
        gl_debug (DEBUG_MERGE, "BEGIN");

        merge_vcard = GL_MERGE_VCARD (merge);

        /* extremely simple approach until I can list the available keys from the
         * server, and return them. */
        key_list = NULL;
        key_list = g_list_append (key_list, g_strdup ("full_name"));
        key_list = g_list_append (key_list, g_strdup ("home_address"));
        key_list = g_list_append (key_list, g_strdup ("work_address"));

        gl_debug (DEBUG_MERGE, "END");

        return key_list;
}

/* TODO? */
/*--------------------------------------------------------------------------*/
/* Get "primary" key.                                                       */
/*--------------------------------------------------------------------------*/
static gchar *
gl_merge_vcard_get_primary_key (glMerge *merge)
{
        /* For now, let's always assume the full name is the primary key. */
        return g_strdup ("full_name");
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

        /* get the full name */
        field = g_new0 (glMergeField, 1);
        field->key = g_strdup ("full_name");
        field->value = g_strdup (e_contact_get_const(contact, E_CONTACT_FULL_NAME));

        record->field_list = g_list_append (record->field_list, field);

        /* get the home address */
        field = g_new0 (glMergeField, 1);
        field->key = g_strdup ("home_address");
        field->value = g_strdup (e_contact_get_const(contact, E_CONTACT_ADDRESS_LABEL_HOME));

        record->field_list = g_list_append (record->field_list, field);

        /* get the work address */
        field = g_new0 (glMergeField, 1);
        field->key = g_strdup ("work_address");
        field->value = g_strdup (e_contact_get_const(contact, E_CONTACT_ADDRESS_LABEL_WORK));

        record->field_list = g_list_append (record->field_list, field);

        /* free the contact */
        g_object_unref (contact);
        g_free(vcard);

        return record;
}

/*---------------------------------------------------------------------------*/
/* Copy merge_vcard specific fields.                                         */
/*---------------------------------------------------------------------------*/
static void
gl_merge_vcard_copy (glMerge *dst_merge,
                     glMerge *src_merge)
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

        while (fgets(line, sizeof(line), fp) && found_end == FALSE) {
                if (found_begin == TRUE) {
                        if (g_str_has_prefix(line, "END:VCARD")) { found_end = TRUE; }
                } else {
                        if (g_str_has_prefix(line, "BEGIN:VCARD")) { found_begin = TRUE; } 
                        else { continue; }/* skip lines not in a vcard */
                }

                /* if the buffer passed us isn't big enough, reallocate it */
                cursize += strlen(line);
                if (cursize >= size) {
                        size *= 2;
                        vcard = (char *)g_realloc(vcard, size); /* aborts program on error */
                }

                /* add the line (or portion thereof) to the vcard */
                strncat(vcard, line, size);
        }

        return vcard;
}



#endif /* HAVE_LIBEBOOK */
