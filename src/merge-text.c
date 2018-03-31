/*
 *  merge-text.c
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

#include "merge-text.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "debug.h"

#define LINE_BUF_LEN 1024

/*
 * Unicode handling.
 *  The default encoding assumption is that files are in the system encoding.
 * However, files are checked for a Unicode BOM (Byte Order Mark), which if found
 * alters the manner in which files are handled.
 */
enum UnicodeEncoding {
        SYSTEM_ENCODING,
        UTF8,
        UTF16_LE,
        UTF16_BE,
        UTF32_LE,
        UTF32_BE
};
        
/*===========================================*/
/* Private types                             */
/*===========================================*/

struct _glMergeTextPrivate {

        gchar             delim;
        gboolean          line1_has_keys;

        enum UnicodeEncoding   encoding;
        GIConv             g_iconverter;
        gchar              char_buf[MB_LEN_MAX];
        gsize              buf_pos;
        gsize              buf_len;

        FILE             *fp;

        GPtrArray        *keys;
        gint              n_fields_max;
};

enum {
        LAST_SIGNAL
};

enum {
        ARG_0,
        ARG_DELIM,
        ARG_LINE1_HAS_KEYS
};


/*===========================================*/
/* Private globals                           */
/*===========================================*/


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void           gl_merge_text_finalize        (GObject          *object);

static void           gl_merge_text_set_property    (GObject          *object,
                                                     guint             param_id,
                                                     const GValue     *value,
                                                     GParamSpec       *pspec);

static void           gl_merge_text_get_property    (GObject          *object,
                                                     guint             param_id,
                                                     GValue           *value,
                                                     GParamSpec       *pspec);

static gchar         *key_from_index                (glMergeText      *merge_text,
                                                     gint              i_field);
static void           clear_keys                    (glMergeText      *merge_text);

static GList         *gl_merge_text_get_key_list    (const glMerge    *merge);
static gchar         *gl_merge_text_get_primary_key (const glMerge    *merge);
static void           gl_merge_text_open            (glMerge          *merge);
static void           gl_merge_text_close           (glMerge          *merge);
static glMergeRecord *gl_merge_text_get_record      (glMerge          *merge);
static void           gl_merge_text_copy            (glMerge          *dst_merge,
                                                     const glMerge    *src_merge);

static GList         *parse_line                    (glMergeText       *merge_text,
                                                     gchar             delim);
static void           free_fields                   (GList           **fields);



/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glMergeText, gl_merge_text, GL_TYPE_MERGE)


static void
gl_merge_text_class_init (glMergeTextClass *class)
{
        GObjectClass *object_class = G_OBJECT_CLASS (class);
        glMergeClass *merge_class  = GL_MERGE_CLASS (class);

        gl_debug (DEBUG_MERGE, "START");

        gl_merge_text_parent_class = g_type_class_peek_parent (class);

        object_class->set_property = gl_merge_text_set_property;
        object_class->get_property = gl_merge_text_get_property;

        g_object_class_install_property
                (object_class,
                 ARG_DELIM,
                 g_param_spec_char ("delim", NULL, NULL,
                                    0, 0x7F, ',',
                                    (G_PARAM_READABLE | G_PARAM_WRITABLE)));

        g_object_class_install_property
                (object_class,
                 ARG_LINE1_HAS_KEYS,
                 g_param_spec_boolean ("line1_has_keys", NULL, NULL,
                                       FALSE,
                                       (G_PARAM_READABLE | G_PARAM_WRITABLE)));

        object_class->finalize = gl_merge_text_finalize;

        merge_class->get_key_list    = gl_merge_text_get_key_list;
        merge_class->get_primary_key = gl_merge_text_get_primary_key;
        merge_class->open            = gl_merge_text_open;
        merge_class->close           = gl_merge_text_close;
        merge_class->get_record      = gl_merge_text_get_record;
        merge_class->copy            = gl_merge_text_copy;

        gl_debug (DEBUG_MERGE, "END");
}


static void
gl_merge_text_init (glMergeText *merge_text)
{
        gl_debug (DEBUG_MERGE, "START");

        merge_text->priv = g_new0 (glMergeTextPrivate, 1);

        merge_text->priv->keys = g_ptr_array_new ();

        gl_debug (DEBUG_MERGE, "END");
}


static void
gl_merge_text_finalize (GObject *object)
{
        glMergeText *merge_text = GL_MERGE_TEXT (object);

        gl_debug (DEBUG_MERGE, "START");

        g_return_if_fail (object && GL_IS_MERGE_TEXT (object));

        clear_keys (merge_text);
        g_ptr_array_free (merge_text->priv->keys, TRUE);
        g_free (merge_text->priv);

        G_OBJECT_CLASS (gl_merge_text_parent_class)->finalize (object);

        gl_debug (DEBUG_MERGE, "END");
}


/*--------------------------------------------------------------------------*/
/* Set argument.                                                            */
/*--------------------------------------------------------------------------*/
static void
gl_merge_text_set_property (GObject      *object,
                            guint         param_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
        glMergeText *merge_text;

        merge_text = GL_MERGE_TEXT (object);

        switch (param_id) {

        case ARG_DELIM:
                merge_text->priv->delim = g_value_get_schar (value);
                gl_debug (DEBUG_MERGE, "ARG \"delim\" = \"%c\"",
                          merge_text->priv->delim);
                break;

        case ARG_LINE1_HAS_KEYS:
                merge_text->priv->line1_has_keys = g_value_get_boolean (value);
                gl_debug (DEBUG_MERGE, "ARG \"line1_has_keys\" = \"%d\"",
                          merge_text->priv->line1_has_keys);
                break;

        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
                break;

        }

}


/*--------------------------------------------------------------------------*/
/* Get argument.                                                            */
/*--------------------------------------------------------------------------*/
static void
gl_merge_text_get_property (GObject     *object,
                            guint        param_id,
                            GValue      *value,
                            GParamSpec  *pspec)
{
        glMergeText *merge_text;

        merge_text = GL_MERGE_TEXT (object);

        switch (param_id) {

        case ARG_DELIM:
                g_value_set_schar (value, merge_text->priv->delim);
                break;

        case ARG_LINE1_HAS_KEYS:
                g_value_set_boolean (value, merge_text->priv->line1_has_keys);
                break;

        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
                break;

        }

}


/*---------------------------------------------------------------------------*/
/* Lookup key name from zero based index.                                    */
/*---------------------------------------------------------------------------*/
static gchar *
key_from_index (glMergeText  *merge_text,
                gint          i_field)
{
        if ( merge_text->priv->line1_has_keys &&
             (i_field < merge_text->priv->keys->len) )
        {
                return g_strdup (g_ptr_array_index (merge_text->priv->keys, i_field));
        }
        else
        {
                return g_strdup_printf ("%d", i_field+1);
        }
}


/*---------------------------------------------------------------------------*/
/* Clear stored keys.                                                        */
/*---------------------------------------------------------------------------*/
static void
clear_keys (glMergeText      *merge_text)
{
        gint i;

        for ( i = 0; i < merge_text->priv->keys->len; i++ )
        {
                g_free (g_ptr_array_index (merge_text->priv->keys, i));
        }
        merge_text->priv->keys->len = 0;
}


/*--------------------------------------------------------------------------*/
/* Get key list.                                                            */
/*--------------------------------------------------------------------------*/
static GList *
gl_merge_text_get_key_list (const glMerge *merge)
{
        glMergeText   *merge_text;
        gint           i_field, n_fields;
        GList         *key_list;
        
        gl_debug (DEBUG_MERGE, "BEGIN");

        merge_text = GL_MERGE_TEXT (merge);

        if ( merge_text->priv->line1_has_keys )
        {
                n_fields = merge_text->priv->keys->len;
        }
        else
        {
                n_fields = merge_text->priv->n_fields_max;
        }

        key_list = NULL;
        for ( i_field=0; i_field < n_fields; i_field++ )
        {
                key_list = g_list_append (key_list, key_from_index(merge_text, i_field));
        }

        gl_debug (DEBUG_MERGE, "END");

        return key_list;
}


/*--------------------------------------------------------------------------*/
/* Get "primary" key.                                                       */
/*--------------------------------------------------------------------------*/
static gchar *
gl_merge_text_get_primary_key (const glMerge *merge)
{
        /* For now, let's always assume the first column is the primary key. */
        return key_from_index (GL_MERGE_TEXT (merge), 0);
}

/*--------------------------------------------------------------------------*/
/* Read the byte order marks to determine unicode encoding, if any.         */
/* See https://en.wikipedia.org/wiki/Byte_order_mark                        */
/*--------------------------------------------------------------------------*/
static enum UnicodeEncoding
gl_read_encoding(FILE* fp) {
        enum UnicodeEncoding encoding;
        gchar ch, ch2, ch3, ch4;
        ch = getc(fp);

        if (ch == '\xff') {
                ch2 = getc(fp);

                if (ch2 == '\xfe') {
                        ch3 = getc(fp);
                        ch4 = getc(fp);
                        if (ch3 == '\0' && ch4 == '\0') {
                                encoding = UTF32_LE;
                        } else {
                                ungetc(ch4, fp);
                                ungetc(ch3, fp);
                                encoding = UTF16_LE;
                        }
                } else {
                        ungetc(ch2, fp);
                        ungetc(ch, fp);
                        encoding = SYSTEM_ENCODING;
                }
        } else if (ch == '\xfe') {
                ch2 = getc(fp);
                if (ch2 == '\xff') {
                        encoding = UTF16_BE;
                } else {
                        ungetc(ch2, fp);
                        ungetc(ch, fp);
                        encoding = SYSTEM_ENCODING;
                }
        } else if (ch == '\0') {
                ch2 = getc(fp);
                ch3 = getc(fp);
                ch4 = getc(fp);
                if (ch2 == '\0' && ch3 == '\xfe' && ch4 == '\xff') {
                        encoding = UTF32_BE;
                } else {
                        ungetc(ch4, fp);
                        ungetc(ch3, fp);
                        ungetc(ch2, fp);
                        encoding = SYSTEM_ENCODING;
                }
        } else if (ch == '\xef') {
                ch2 = getc(fp);
                if (ch2 == '\xbb') {
                        ch3 = getc(fp);
                        if (ch3 == '\xbf') {
                                encoding = UTF8;
                        } else {
                                ungetc(ch3, fp);
                                ungetc(ch2, fp);
                                ungetc(ch, fp);
                                encoding = SYSTEM_ENCODING;
                        }
                } else {
                        ungetc(ch2, fp);
                        ungetc(ch, fp);
                        encoding = SYSTEM_ENCODING;
                }
        } else {
                ungetc(ch, fp);
                encoding = SYSTEM_ENCODING;
        }
        return encoding;
}

/*
 * gLabels get-character routine for possibly Unicode text files.
 * If the source has a byte order mark (BOM) indicating a Unicode file, 
 * g_iconv is used to convert input characters to GDK-standard UTF8 format.
 */

static gint
gl_getc(glMergeText *merge_text) {
        if (merge_text->priv->buf_pos < merge_text->priv->buf_len) {
                return merge_text->priv->char_buf[merge_text->priv->buf_pos++];
        } else if (merge_text->priv->encoding == SYSTEM_ENCODING || 
                   merge_text->priv->encoding == UTF8) {
                return getc(merge_text->priv->fp);
        } else {
                /*
                 * a UTF-16 stream might include surrogates, which encode
                 * characters in successive 16-bit units. If we read a
                 * leading surrogate, read in the trailing one as well for 
                 * processing.
                 */
                gchar wcbuf[4];
                size_t result;
                gchar* outbufp;
                int hob_offset;
                int unit_len;
                switch (merge_text->priv->encoding) {
                case UTF16_BE:
                        hob_offset = 0;
                        unit_len = 2;
                        break;
                case UTF16_LE:
                        hob_offset = 1;
                        unit_len = 2;
                        break;
                case UTF32_BE:
                case UTF32_LE:
                        hob_offset = -1;
                        unit_len = 4;
                        break;
                }
                gsize nBytes = fread(wcbuf, 1, unit_len, merge_text->priv->fp);
                if (nBytes == 0)
                        return EOF;
                if (hob_offset >= 0 && (wcbuf[hob_offset] & 0xfd) == 0xd8) {
                        nBytes += fread(wcbuf+unit_len, 1, unit_len, merge_text->priv->fp);
                }
                gchar* wcbufp = wcbuf;
                outbufp = merge_text->priv->char_buf;
                gsize buflen = sizeof(merge_text->priv->char_buf);
                result = g_iconv(merge_text->priv->g_iconverter, &wcbufp, &nBytes,
                                 &outbufp, &buflen);
                if (result == EOF) {
                        g_warning("g_iconv: %s", strerror(errno));
                }
                merge_text->priv->buf_len = outbufp - merge_text->priv->char_buf;
                merge_text->priv->buf_pos = 0;
                return merge_text->priv->char_buf[merge_text->priv->buf_pos++];
        }
}


/*--------------------------------------------------------------------------*/
/* Open merge source.                                                       */
/*--------------------------------------------------------------------------*/
static void
gl_merge_text_open (glMerge *merge)
{
        glMergeText *merge_text;
        gchar       *src;

        GList       *line1_fields;
        GList       *p;

        merge_text = GL_MERGE_TEXT (merge);

        src = gl_merge_get_src (merge);

        if (src != NULL)
        {
                if (g_utf8_strlen(src, -1) == 1 && src[0] == '-') {
                        merge_text->priv->fp = stdin;
                        merge_text->priv->encoding = SYSTEM_ENCODING;
                } else {
                        if ((merge_text->priv->fp = fopen (src, "r")) != NULL) {
                                merge_text->priv->encoding = gl_read_encoding(merge_text->priv->fp);
                        } else {
                                g_warning("gl_merge_text_open: %s (%s)",
                                        strerror(errno), src);
                        }
                }
                g_free (src);

                gchar* in_codeset = NULL;
                switch (merge_text->priv->encoding) {
                case UTF8:
                case SYSTEM_ENCODING:
                        break;
                case UTF16_BE:
                        in_codeset = "UTF-16BE";
                        break;
                case UTF16_LE:
                        in_codeset = "UTF-16LE";
                        break;
                case UTF32_BE:
                        in_codeset = "UTF-32BE";
                        break;
                case UTF32_LE:
                        in_codeset = "UTF-32LE";
                        break;
                }
                if (in_codeset != NULL) {
                        merge_text->priv->g_iconverter = g_iconv_open("UTF8", in_codeset);
                        /* Since we define both codesets, we should always be able to open the converter */
                        g_assert(merge_text->priv->g_iconverter != (GIConv)-1);
                }
                clear_keys (merge_text);
                merge_text->priv->n_fields_max = 0;

                if ( merge_text->priv->line1_has_keys )
                {
                        /*
                         * Extract keys from first line and discard line
                         */

                        line1_fields = parse_line (merge_text, merge_text->priv->delim);
                        for ( p = line1_fields; p != NULL; p = p->next )
                        {
                                g_ptr_array_add (merge_text->priv->keys, g_strdup (p->data));
                        }
                        free_fields (&line1_fields);
                }

        }


}


/*--------------------------------------------------------------------------*/
/* Close merge source.                                                      */
/*--------------------------------------------------------------------------*/
static void
gl_merge_text_close (glMerge *merge)
{
        glMergeText *merge_text;

        merge_text = GL_MERGE_TEXT (merge);

        if (merge_text->priv->fp != NULL) {

                fclose (merge_text->priv->fp);
                merge_text->priv->fp = NULL;

        }
        if (merge_text->priv->g_iconverter != 0) {
                g_iconv_close(merge_text->priv->g_iconverter);
                merge_text->priv->g_iconverter = 0;
        }
}


/*--------------------------------------------------------------------------*/
/* Get next record from merge source, NULL if no records left (i.e EOF)     */
/*--------------------------------------------------------------------------*/
static glMergeRecord *
gl_merge_text_get_record (glMerge *merge)
{
        glMergeText   *merge_text;
        gchar          delim;
        glMergeRecord *record;
        GList         *fields, *p;
        gint           i_field;
        glMergeField  *field;

        merge_text = GL_MERGE_TEXT (merge);

        delim = merge_text->priv->delim;

        fields = parse_line (merge_text, delim);
        if ( fields == NULL ) {
                return NULL;
        }

        record = g_new0 (glMergeRecord, 1);
        record->select_flag = TRUE;
        for (p=fields, i_field=0; p != NULL; p=p->next, i_field++) {

                field = g_new0 (glMergeField, 1);
                field->key = key_from_index (merge_text, i_field);
#ifndef CSV_ALWAYS_UTF8
                if (merge_text->priv->encoding == SYSTEM_ENCODING) {
                        field->value = g_locale_to_utf8 (p->data, -1, NULL, NULL, NULL);
                } else {
                        field->value = g_strdup (p->data);
                }
#else
                field->value = g_strdup (p->data);
#endif

                record->field_list = g_list_append (record->field_list, field);
        }
        free_fields (&fields);

        if ( i_field > merge_text->priv->n_fields_max )
        {
                merge_text->priv->n_fields_max = i_field;
        }

        return record;
}


/*---------------------------------------------------------------------------*/
/* Copy merge_text specific fields.                                          */
/*---------------------------------------------------------------------------*/
static void
gl_merge_text_copy (glMerge       *dst_merge,
                    const glMerge *src_merge)
{
        glMergeText *dst_merge_text;
        glMergeText *src_merge_text;
        gint         i;

        dst_merge_text = GL_MERGE_TEXT (dst_merge);
        src_merge_text = GL_MERGE_TEXT (src_merge);

        dst_merge_text->priv->delim          = src_merge_text->priv->delim;
        dst_merge_text->priv->line1_has_keys = src_merge_text->priv->line1_has_keys;

        for ( i=0; i < src_merge_text->priv->keys->len; i++ )
        {
                g_ptr_array_add (dst_merge_text->priv->keys,
                                 g_strdup ((gchar *)g_ptr_array_index (src_merge_text->priv->keys, i)));
        }

        dst_merge_text->priv->n_fields_max   = src_merge_text->priv->n_fields_max;
}


/*---------------------------------------------------------------------------*/
/* PRIVATE.  Parse line.                                                     */
/*                                                                           */
/* Attempt to be a robust parser of various CSV (and similar) formats.       */
/*                                                                           */
/* Based on CSV format described in RFC 4180 section 2.                      */
/*                                                                           */
/* Additions to RFC 4180 rules:                                              */
/*   - delimeters and other special characters may be "escaped" by a leading */
/*     backslash (\)                                                         */
/*   - C escape sequences for newline (\n) and tab (\t) are also translated. */
/*   - if quoted text is not followed by a delimeter, any additional text is */
/*     concatenated with quoted portion.                                     */
/*                                                                           */
/* Returns a list of fields.  A blank line is considered a line with one     */
/* empty field.  Returns empty (NULL) when done.                             */
/*---------------------------------------------------------------------------*/
static GList *
parse_line (glMergeText* merge_text,
            gchar  delim )
{
        GList   *list;
        GString *field;
        gint     c;
        enum { DELIM,
               QUOTED, QUOTED_QUOTE1, QUOTED_ESCAPED,
               SIMPLE, SIMPLE_ESCAPED,
               DONE } state;

        if (merge_text->priv->fp == NULL) {
                return NULL;
        }
               
        state = DELIM;
        list  = NULL;
        field = g_string_new( "" );
        while ( state != DONE ) {
                c=gl_getc (merge_text);

                switch (state) {

                case DELIM:
                        switch (c) {
                        case '\n':
                                /* last field is empty. */
                                list = g_list_append (list, g_strdup (""));
                                state = DONE;
                                break;
                        case '\r':
                                /* ignore */
                                state = DELIM;
                                break;
                        case EOF:
                                /* end of file, no more lines. */
                                state = DONE;
                                break;
                        case '"':
                                /* start a quoted field. */
                                state = QUOTED;
                                break;
                        case '\\':
                                /* simple field, but 1st character is an escape. */
                                state = SIMPLE_ESCAPED;
                                break;
                        default:
                                if ( c == delim )
                                {
                                        /* field is empty. */
                                        list = g_list_append (list, g_strdup (""));
                                        state = DELIM;
                                }
                                else
                                {
                                        /* begining of a simple field. */
                                        field = g_string_append_c (field, c);
                                        state = SIMPLE;
                                }
                                break;
                        }
                        break;

                case QUOTED:
                        switch (c) {
                        case EOF:
                                /* File ended mid way through quoted item, truncate field. */
                                list = g_list_append (list, g_strdup (field->str));
                                state = DONE;
                                break;
                        case '"':
                                /* Possible end of field, but could be 1st of a pair. */
                                state = QUOTED_QUOTE1;
                                break;
                        case '\\':
                                /* Escape next character, or special escape, e.g. \n. */
                                state = QUOTED_ESCAPED;
                                break;
                        default:
                                /* Use character literally. */
                                field = g_string_append_c (field, c);
                                break;
                        }
                        break;

                case QUOTED_QUOTE1:
                        switch (c) {
                        case '\n':
                        case EOF:
                                /* line or file ended after quoted item */
                                list = g_list_append (list, g_strdup (field->str));
                                state = DONE;
                                break;
                        case '"':
                                /* second quote, insert and stay quoted. */
                                field = g_string_append_c (field, c);
                                state = QUOTED;
                                break;
                        case '\r':
                                /* ignore and go to fallback */
                                state = SIMPLE;
                                break;
                        default:
                                if ( c == delim )
                                {
                                        /* end of field. */
                                        list = g_list_append (list, g_strdup (field->str));
                                        field = g_string_assign( field, "" );
                                        state = DELIM;
                                }
                                else
                                {
                                        /* fallback if not a delim or another quote. */
                                        field = g_string_append_c (field, c);
                                        state = SIMPLE;
                                }
                                break;
                        }
                        break;

                case QUOTED_ESCAPED:
                        switch (c) {
                        case EOF:
                                /* File ended mid way through quoted item */
                                list = g_list_append (list, g_strdup (field->str));
                                state = DONE;
                                break;
                        case 'n':
                                /* Decode "\n" as newline. */
                                field = g_string_append_c (field, '\n');
                                state = QUOTED;
                                break;
                        case 't':
                                /* Decode "\t" as tab. */
                                field = g_string_append_c (field, '\t');
                                state = QUOTED;
                                break;
                        default:
                                /* Use character literally. */
                                field = g_string_append_c (field, c);
                                state = QUOTED;
                                break;
                        }
                        break;

                case SIMPLE:
                        switch (c) {
                        case '\n':
                        case EOF:
                                /* line or file ended */
                                list = g_list_append (list, g_strdup (field->str));
                                state = DONE;
                                break;
                        case '\r':
                                /* ignore */
                                state = SIMPLE;
                                break;
                        case '\\':
                                /* Escape next character, or special escape, e.g. \n. */
                                state = SIMPLE_ESCAPED;
                                break;
                        default:
                                if ( c == delim )
                                {
                                        /* end of field. */
                                        list = g_list_append (list, g_strdup (field->str));
                                        field = g_string_assign( field, "" );
                                        state = DELIM;
                                }
                                else
                                {
                                        /* Use character literally. */
                                        field = g_string_append_c (field, c);
                                        state = SIMPLE;
                                }
                                break;
                        }
                        break;

                case SIMPLE_ESCAPED:
                        switch (c) {
                        case EOF:
                                /* File ended mid way through quoted item */
                                list = g_list_append (list, g_strdup (field->str));
                                state = DONE;
                                break;
                        case 'n':
                                /* Decode "\n" as newline. */
                                field = g_string_append_c (field, '\n');
                                state = SIMPLE;
                                break;
                        case 't':
                                /* Decode "\t" as tab. */
                                field = g_string_append_c (field, '\t');
                                state = SIMPLE;
                                break;
                        default:
                                /* Use character literally. */
                                field = g_string_append_c (field, c);
                                state = SIMPLE;
                                break;
                        }
                        break;

                default:
                        g_assert_not_reached();
                        break;
                }

        }
        g_string_free( field, TRUE );

        return list;
}


/*---------------------------------------------------------------------------*/
/* Free list of fields.                                                      */
/*---------------------------------------------------------------------------*/
void
free_fields (GList ** list)
{
        GList *p;

        for ( p = *list; p != NULL; p = p->next )
        {
                g_free (p->data);
                p->data = NULL;
        }

        g_list_free (*list);
        *list = NULL;
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
