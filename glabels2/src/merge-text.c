/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  merge_text.c:  text-file merge backend module
 *
 *  Copyright (C) 2001  Jim Evins <evins@snaught.com>.
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

#include <stdio.h>

#include "merge-text.h"

#include "debug.h"

#define LINE_BUF_LEN 1024

/*===========================================*/
/* Private types                             */
/*===========================================*/

struct _glMergeTextPrivate {
	gchar             delim;
	FILE             *fp;
};

enum {
	LAST_SIGNAL
};

enum {
	ARG_0,
	ARG_DELIM,
};

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static glMergeClass *parent_class = NULL;


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void           gl_merge_text_class_init      (glMergeTextClass *klass);
static void           gl_merge_text_instance_init   (glMergeText      *object);
static void           gl_merge_text_finalize        (GObject          *object);

static void           gl_merge_text_set_property    (GObject          *object,
						     guint             param_id,
						     const GValue     *value,
						     GParamSpec       *pspec);

static void           gl_merge_text_get_property    (GObject          *object,
						     guint             param_id,
						     GValue           *value,
						     GParamSpec       *pspec);

static GList         *gl_merge_text_get_key_list    (glMerge          *merge);
static gchar         *gl_merge_text_get_primary_key (glMerge          *merge);
static void           gl_merge_text_open            (glMerge          *merge);
static void           gl_merge_text_close           (glMerge          *merge);
static glMergeRecord *gl_merge_text_get_record      (glMerge          *merge);
static void           gl_merge_text_copy            (glMerge          *dst_merge,
						     glMerge          *src_merge);

static GList         *parse_line                    (FILE             *fp,
						     gchar             delim);
static void           free_fields                   (GList           **fields);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_merge_text_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (glMergeTextClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_merge_text_class_init,
			NULL,
			NULL,
			sizeof (glMergeText),
			0,
			(GInstanceInitFunc) gl_merge_text_instance_init,
		};

		type = g_type_register_static (GL_TYPE_MERGE,
					       "glMergeText", &info, 0);
	}

	return type;
}

static void
gl_merge_text_class_init (glMergeTextClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	glMergeClass *merge_class  = (glMergeClass *) klass;

	gl_debug (DEBUG_MERGE, "START");

	parent_class = g_type_class_peek_parent (klass);

	object_class->set_property = gl_merge_text_set_property;
	object_class->get_property = gl_merge_text_get_property;

	g_object_class_install_property
                (object_class,
                 ARG_DELIM,
                 g_param_spec_char ("delim", NULL, NULL,
				    0, 0x7F, ',',
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
gl_merge_text_instance_init (glMergeText *merge_text)
{
	gl_debug (DEBUG_MERGE, "START");

	merge_text->private = g_new0 (glMergeTextPrivate, 1);

	gl_debug (DEBUG_MERGE, "END");
}

static void
gl_merge_text_finalize (GObject *object)
{
	gl_debug (DEBUG_MERGE, "START");

	g_return_if_fail (object && GL_IS_MERGE_TEXT (object));

	G_OBJECT_CLASS (parent_class)->finalize (object);

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
		merge_text->private->delim = g_value_get_char (value);
		gl_debug (DEBUG_MERGE, "ARG \"delim\" = \"%c\"",
			  merge_text->private->delim);
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
		g_value_set_char (value, merge_text->private->delim);
		break;

        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
                break;

        }

}

/*--------------------------------------------------------------------------*/
/* Get key list.                                                            */
/*--------------------------------------------------------------------------*/
static GList *
gl_merge_text_get_key_list (glMerge *merge)
{
	glMergeText   *merge_text;
	GList         *record_list, *p_rec;
	glMergeRecord *record;
	GList         *p_field;
	gint           i_field, n_fields, n_fields_max = 0;
	GList         *key_list;
	
	/* Field keys are simply column numbers. */

	gl_debug (DEBUG_MERGE, "BEGIN");

	merge_text = GL_MERGE_TEXT (merge);

	record_list = (GList *)gl_merge_get_record_list (merge);

	for ( p_rec=record_list; p_rec!=NULL; p_rec=p_rec->next ) {
		record = (glMergeRecord *)p_rec->data;

		n_fields = 0;
		for ( p_field=record->field_list; p_field!=NULL; p_field=p_field->next ) {
			n_fields++;
		}
		if ( n_fields > n_fields_max ) n_fields_max = n_fields;
	}

	key_list = NULL;
	for (i_field=1; i_field <= n_fields_max; i_field++) {
		key_list = g_list_append (key_list, g_strdup_printf ("%d", i_field));
	}


	gl_debug (DEBUG_MERGE, "END");

	return key_list;
}

/*--------------------------------------------------------------------------*/
/* Get "primary" key.                                                       */
/*--------------------------------------------------------------------------*/
static gchar *
gl_merge_text_get_primary_key (glMerge *merge)
{
	/* For now, let's always assume the first column is the primary key. */
	return g_strdup ("1");
}

/*--------------------------------------------------------------------------*/
/* Open merge source.                                                       */
/*--------------------------------------------------------------------------*/
static void
gl_merge_text_open (glMerge *merge)
{
	glMergeText *merge_text;
	gchar       *src;

	merge_text = GL_MERGE_TEXT (merge);

	src = gl_merge_get_src (merge);

	if (src != NULL) {
		merge_text->private->fp = fopen (src, "r");
	}

	g_free (src);
}

/*--------------------------------------------------------------------------*/
/* Close merge source.                                                      */
/*--------------------------------------------------------------------------*/
static void
gl_merge_text_close (glMerge *merge)
{
	glMergeText *merge_text;

	merge_text = GL_MERGE_TEXT (merge);

	if (merge_text->private->fp != NULL) {

		fclose (merge_text->private->fp);
		merge_text->private->fp = NULL;

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
	FILE          *fp;
	glMergeRecord *record;
	GList         *fields, *p;
	gint           i_field;
	glMergeField  *field;

	merge_text = GL_MERGE_TEXT (merge);

	delim = merge_text->private->delim;
	fp    = merge_text->private->fp;

	if (fp == NULL) {
		return NULL;
	}
	       
	fields = parse_line (fp, delim);
	if ( fields == NULL ) {
		return NULL;
	}

	record = g_new0 (glMergeRecord, 1);
	record->select_flag = TRUE;
	i_field = 1;
	for (p=fields; p != NULL; p=p->next) {

		field = g_new0 (glMergeField, 1);
		field->key = g_strdup_printf ("%d", i_field++);
		field->value = g_strdup (p->data);

		record->field_list = g_list_append (record->field_list, field);
	}
	free_fields (&fields);

	return record;
}

/*---------------------------------------------------------------------------*/
/* Copy merge_text specific fields.                                          */
/*---------------------------------------------------------------------------*/
static void
gl_merge_text_copy (glMerge *dst_merge,
		    glMerge *src_merge)
{
	glMergeText *dst_merge_text;
	glMergeText *src_merge_text;

	dst_merge_text = GL_MERGE_TEXT (dst_merge);
	src_merge_text = GL_MERGE_TEXT (src_merge);

	dst_merge_text->private->delim = src_merge_text->private->delim;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Parse line (quoted values may span multiple lines).             */
/*---------------------------------------------------------------------------*/
static GList *
parse_line (FILE  *fp,
	    gchar  delim )
{
	GList *list = NULL;
	GString *string;
	gint c;
	enum { BEGIN, NORMAL, NORMAL_ESCAPED,
	       QUOTED, QUOTED_ESCAPED, QUOTED_QUOTE1,
	       DONE } state;

	state = BEGIN;
	string = g_string_new( "" );
	while ( state != DONE ) {
		c=getc (fp);

		switch (state) {

		case BEGIN:
			switch (c) {
			case '\\':
				state = NORMAL_ESCAPED;
				break;
			case '"':
				state = QUOTED;
				break;
			case '\r':
				/* Strip CR. */
				state = NORMAL;
				break;
			case '\n':
			case EOF:
				state = DONE;
				break;
			default:
				if ( c != delim ) {
					string = g_string_append_c (string, c);
				} else {
					list = g_list_append (list,
						      g_strdup (string->str));
					string = g_string_assign( string, "" );
				}
				state = NORMAL;
				break;
			}
			break;

		case NORMAL:
			switch (c) {
			case '\\':
				state = NORMAL_ESCAPED;
				break;
			case '"':
				state = QUOTED;
				break;
			case '\r':
				/* Strip CR. */
				break;
			case '\n':
			case EOF:
				list = g_list_append (list,
						      g_strdup (string->str));
				state = DONE;
				break;
			default:
				if ( c != delim ) {
					string = g_string_append_c (string, c);
				} else {
					list = g_list_append (list,
						      g_strdup (string->str));
					string = g_string_assign( string, "" );
				}
				break;
			}
			break;

		case NORMAL_ESCAPED:
			switch (c) {
			case 'n':
				string = g_string_append_c (string, '\n');
				state = NORMAL;
				break;
			case 't':
				string = g_string_append_c (string, '\t');
				state = NORMAL;
				break;
			case '\r':
				/* Strip CR, stay ESCAPED. */
				break;
			case EOF:
				state = DONE;
				break;
			default:
				string = g_string_append_c (string, c);
				state = NORMAL;
				break;
			}
			break;

		case QUOTED:
			switch (c) {
			case '\\':
				state = QUOTED_ESCAPED;
				break;
			case '"':
				state = QUOTED_QUOTE1;
				break;
			case '\r':
				/* Strip CR. */
				break;
			case EOF:
				/* File ended mid way through quoted item */
				list = g_list_append (list,
						      g_strdup (string->str));
				state = DONE;
				break;
			default:
				string = g_string_append_c (string, c);
				break;
			}
			break;

		case QUOTED_ESCAPED:
			switch (c) {
			case 'n':
				string = g_string_append_c (string, '\n');
				state = QUOTED;
				break;
			case 't':
				string = g_string_append_c (string, '\t');
				state = QUOTED;
				break;
			case '\r':
				/* Strip CR, stay ESCAPED. */
				break;
			case EOF:
				/* File ended mid way through quoted item */
				list = g_list_append (list,
						      g_strdup (string->str));
				state = DONE;
				break;
			default:
				string = g_string_append_c (string, c);
				state = QUOTED;
				break;
			}
			break;

		case QUOTED_QUOTE1:
			switch (c) {
			case '"':
				/* insert quotes in string, stay quoted. */
				string = g_string_append_c (string, c);
				state = QUOTED;
				break;
			case '\r':
				/* Strip CR, return to NORMAL. */
				state = NORMAL;
				break;
			case '\n':
			case EOF:
				/* line or file ended after quoted item */
				list = g_list_append (list,
						      g_strdup (string->str));
				state = DONE;
				break;
			default:
				if ( c != delim ) {
					string = g_string_append_c (string, c);
				} else {
					list = g_list_append (list,
						      g_strdup (string->str));
					string = g_string_assign( string, "" );
				}
				state = NORMAL;
				break;
			}
			break;

		default:
			g_assert_not_reached();
			break;
		}

	}
	g_string_free( string, TRUE );

	return list;
}

/*---------------------------------------------------------------------------*/
/* Free list of fields.                                                      */
/*---------------------------------------------------------------------------*/
void
free_fields (GList ** list)
{
	GList *p;

	for (p = *list; p != NULL; p = p->next) {
		g_free (p->data);
		p->data = NULL;
	}

	g_list_free (*list);
	*list = NULL;
}

