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

#include <gnome.h>

#include "merge_text.h"

#include "debug.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/
static GList * parse_line ( FILE *handle, gchar delim );
static void free_fields( GList **fields );


/****************************************************************************/
/* Open merge source.                                                       */
/****************************************************************************/
glMergeInput *
gl_merge_text_open (glMergeType type,
		    GList * field_defs,
		    gchar * src)
{
	FILE *fp;
	glMergeInput *input;

	fp = fopen (src, "r");
	if (fp == NULL) {
		return NULL;
	}

	input = g_new0 (glMergeInput, 1);
	input->type = type;
	input->field_defs = field_defs;
	input->handle = fp;

	return input;
}

/****************************************************************************/
/* Close merge source.                                                      */
/****************************************************************************/
void
gl_merge_text_close (glMergeInput * input)
{
	if (input != NULL) {

		fclose ((FILE *) input->handle);
		g_free (input);

	}
}

/****************************************************************************/
/* Get next record from merge source, NULL if no records left (i.e EOF)     */
/****************************************************************************/
glMergeRecord *
gl_merge_text_get_record (glMergeInput * input)
{
	gchar *loc;
	GList *fields, *p;
	gint  i_field;
	glMergeRecord *record = NULL;
	glMergeField *field;

	if (input != NULL) {

		switch (input->type) {
		case GL_MERGE_TEXT_TAB:
			fields = parse_line( input->handle, '\t' );
			break;
		case GL_MERGE_TEXT_COLON:
			fields = parse_line( input->handle, ':' );
			break;
		case GL_MERGE_TEXT_COMMA:
			fields = parse_line( input->handle, ',' );
			break;
		default:
			WARN ("Unexpected merge type");
			return NULL;
		}

		if ( fields != NULL ) {
			record = g_new0 (glMergeRecord, 1);
			record->select_flag = TRUE;
			i_field = 1;
			for (p=fields; p != NULL; p=p->next) {
				loc = g_strdup_printf ("%d", i_field++);
				field = g_new0 (glMergeField, 1);
				field->value = g_strdup (p->data);
				field->key = gl_merge_find_key (input->field_defs, loc);
				record->field_list = g_list_append (record->field_list, field);
				g_free (loc);
			}
			free_fields (&fields);
			return record;
		}

	}
	return NULL;
}

/****************************************************************************/
/* Retrieve a list of raw fields (columns in this case)                     */
/****************************************************************************/
GList *
gl_merge_text_get_raw_record (glMergeInput * input)
{
	GList *list = NULL;
	GList *fields, *p;
	gint i_field;
	glMergeRawField *raw_field;

	if (input != NULL) {

		switch (input->type) {
		case GL_MERGE_TEXT_TAB:
			fields = parse_line( input->handle, '\t' );
			break;
		case GL_MERGE_TEXT_COLON:
			fields = parse_line( input->handle, ':' );
			break;
		case GL_MERGE_TEXT_COMMA:
			fields = parse_line( input->handle, ',' );
			break;
		default:
			WARN ("Unexpected merge type");
			return NULL;
		}

		if ( fields != NULL ) {
			i_field = 1;
			for (p=fields; p != NULL; p=p->next) {
				raw_field = g_new0 (glMergeRawField, 1);
				raw_field->loc = g_strdup_printf ("%d",
								  i_field++);
				raw_field->value = g_strdup (p->data);
				list = g_list_append (list, raw_field);
			}
			free_fields (&fields);
		}

	}
	return list;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Parse line (quoted values may span multiple lines).             */
/*---------------------------------------------------------------------------*/
static GList * parse_line ( FILE *handle,
			    gchar delim )
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
		c=getc (handle);

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

