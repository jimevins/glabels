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

static guint signals[LAST_SIGNAL] = {0};


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void           gl_merge_text_class_init     (glMergeTextClass *klass);
static void           gl_merge_text_instance_init  (glMergeText      *object);
static void           gl_merge_text_finalize       (GObject          *object);

static void           gl_merge_text_set_property   (GObject          *object,
						    guint             param_id,
						    const GValue     *value,
						    GParamSpec       *pspec);

static void           gl_merge_text_get_property   (GObject          *object,
						    guint             param_id,
						    GValue           *value,
						    GParamSpec       *pspec);

static GList         *gl_merge_text_get_key_list   (glMerge          *merge);
static void           gl_merge_text_open           (glMerge          *merge);
static void           gl_merge_text_close          (glMerge          *merge);
static glMergeRecord *gl_merge_text_get_record     (glMerge          *merge);
static void           gl_merge_text_copy           (glMerge          *dst_merge,
						    glMerge          *src_merge);

static GList         *split_fields                 (gchar            *line,
						    gchar             delim);
static void           free_fields                  (GList           **fields);


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

	merge_class->get_key_list = gl_merge_text_get_key_list;
	merge_class->open         = gl_merge_text_open;
	merge_class->close        = gl_merge_text_close;
	merge_class->get_record   = gl_merge_text_get_record;
	merge_class->copy         = gl_merge_text_copy;

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
	glMergeText *merge_text;
	gchar       *src;
	FILE        *fp;
	gchar        delim;
	gchar        line[LINE_BUF_LEN];
	GList       *fields, *p;
	gint         i_field, n_fields, n_fields_max = 0;
	GList       *key_list;
	
	/* Fields are simply column numbers. */
	/* FIXME:  the key_list should probably be cached, and only re-evaluated */
	/*         if the source has changed. */

	merge_text = GL_MERGE_TEXT (merge);

	src = gl_merge_get_src (merge);
	delim = merge_text->private->delim;

	fp = fopen (src, "r");
	if ( fp == NULL ) {
		return NULL;
	}

	while ( fgets (line, LINE_BUF_LEN, fp) != NULL ) {

		if (TRUE /* TODO: skip blank lines or comments */ ) {
			g_strchomp (line);
			fields = split_fields (line, delim);
			n_fields = 0;
			for (p=fields; p != NULL; p=p->next) {
				n_fields++;
			}
			free_fields (&fields);
			if ( n_fields > n_fields_max ) n_fields = n_fields_max;
		}

	}

	key_list = NULL;
	for (i_field=1; i_field <= n_fields; i_field++) {
		key_list = g_list_append (key_list, g_strdup_printf ("%d", i_field));
	}

	return key_list;
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
	gchar          line[LINE_BUF_LEN];
	glMergeRecord *record = NULL;
	GList         *fields, *p;
	gint           i_field;
	glMergeField  *field;

	merge_text = GL_MERGE_TEXT (merge);

	delim = merge_text->private->delim;
	fp    = merge_text->private->fp;

	if (fp == NULL) {
		return NULL;
	}
	       
	while ( fgets (line, LINE_BUF_LEN, fp) != NULL ) {

		if (TRUE /* TODO: skip blank lines or comments */ ) {
			g_strchomp (line);
			record = g_new0 (glMergeRecord, 1);
			record->select_flag = TRUE;
			fields = split_fields (line, delim);
			i_field = 1;
			for (p=fields; p != NULL; p=p->next) {
				
				field = g_new0 (glMergeField, 1);
				field->key = g_strdup_printf ("%d", i_field++);
				field->value = g_strdup (p->data);

				record->field_list =
					g_list_append (record->field_list, field);
			}
			free_fields (&fields);
			return record;
		}

	}

	return NULL;
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
/* PRIVATE.  Split out fields by delimiter while decoding things like "\n".  */
/*---------------------------------------------------------------------------*/
static GList * split_fields ( gchar *line,
			      gchar delim )
{
	GList *list = NULL;
	GString *string;
	gchar *c;
	enum { NORMAL, ESCAPED } state;

	g_return_val_if_fail (line != NULL, NULL);

	state = NORMAL;
	string = g_string_new( "" );
	for ( c=line; *c!=0; c++ ) {

		switch (state) {

		case NORMAL:
			if ( *c == '\\' ) {
				state = ESCAPED;
			} else if ( *c != delim ) {
				string = g_string_append_c (string, *c);
			} else {
				list = g_list_append (list,
						      g_strdup (string->str));
				string = g_string_assign( string, "" );
			}
			break;

		case ESCAPED:
			switch (*c) {
			case 'n':
				string = g_string_append_c (string, '\n');
				break;
			case 't':
				string = g_string_append_c (string, '\t');
				break;
			default:
				string = g_string_append_c (string, *c);
				break;
			}
			state = NORMAL;
			break;

		default:
			g_assert_not_reached();
			break;
		}

	}
	list = g_list_append( list, g_strdup(string->str) );
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

