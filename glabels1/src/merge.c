/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  merge.c:  document merge module
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

#include "merge.h"
#include "merge_text.h"

#include "debug.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/
typedef struct {
	gchar *short_text;
	gchar *long_text;
} TypeTexts;

typedef struct {

	glMergeInput * (*open) (glMergeType, GList *, gchar *);
	void (*close) (glMergeInput *);
	glMergeRecord * (*get_record) (glMergeInput *);
	GList * (*get_raw_record) (glMergeInput *);

} BackendFunctions;

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static TypeTexts type_text[GL_MERGE_N_TYPES];

static BackendFunctions func[GL_MERGE_N_TYPES];

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/


/*****************************************************************************/
/* Initialize module.                                                        */
/*****************************************************************************/
void
gl_merge_init (void)
{
	gint i;

	/* Register backend functions and data. */

	i = GL_MERGE_NONE;
	func[i].open = NULL;
	func[i].close = NULL;
	func[i].get_record = NULL;
	func[i].get_raw_record = NULL;
	type_text[i].short_text = "None";
	type_text[i].long_text = _("None");

	i = GL_MERGE_TEXT_TAB;
	func[i].open = gl_merge_text_open;
	func[i].close = gl_merge_text_close;
	func[i].get_record = gl_merge_text_get_record;
	func[i].get_raw_record = gl_merge_text_get_raw_record;
	type_text[i].short_text = "Text/Tab";
	type_text[i].long_text = _("Text with tab separators");

	i = GL_MERGE_TEXT_COMMA;
	func[i].open = gl_merge_text_open;
	func[i].close = gl_merge_text_close;
	func[i].get_record = gl_merge_text_get_record;
	func[i].get_raw_record = gl_merge_text_get_raw_record;
	type_text[i].short_text = "Text/Comma";
	type_text[i].long_text = _("Text with comma separators");

	i = GL_MERGE_TEXT_COLON;
	func[i].open = gl_merge_text_open;
	func[i].close = gl_merge_text_close;
	func[i].get_record = gl_merge_text_get_record;
	func[i].get_raw_record = gl_merge_text_get_raw_record;
	type_text[i].short_text = "Text/Colon";
	type_text[i].long_text = _("Text with colon separators");

}

/*****************************************************************************/
/* Lookup type from short text.                                              */
/*****************************************************************************/
glMergeType
gl_merge_text_to_type (gchar * text)
{
	glMergeType type;

	for (type = 0; type < GL_MERGE_N_TYPES; type++) {
		if (g_strcasecmp (text, type_text[type].short_text) == 0) {
			return type;
		}
	}

	return GL_MERGE_NONE;

}

/*****************************************************************************/
/* Lookup short text for given type.                                         */
/*****************************************************************************/
gchar *
gl_merge_type_to_text (glMergeType type)
{
	return g_strdup (type_text[type].short_text);
}

/*****************************************************************************/
/* Lookup type from long descriptive text.                                   */
/*****************************************************************************/
glMergeType
gl_merge_long_text_to_type (gchar * text)
{
	glMergeType type;

	for (type = 0; type < GL_MERGE_N_TYPES; type++) {
		if (g_strcasecmp (text, type_text[type].long_text) == 0) {
			return type;
		}
	}

	return GL_MERGE_NONE;
}

/*****************************************************************************/
/* Lookup longer, more descriptive text for given type.                      */
/*****************************************************************************/
gchar *
gl_merge_type_to_long_text (glMergeType type)
{
	return g_strdup (type_text[type].long_text);
}

/*****************************************************************************/
/* Retrieve a list of descriptive texts for all available types.             */
/*****************************************************************************/
GList *
gl_merge_get_long_texts_list (void)
{
	glMergeType type;
	GList *list = NULL;

	for (type = 0; type < GL_MERGE_N_TYPES; type++) {

		list = g_list_append (list, gl_merge_type_to_long_text (type));

	}

	return list;
}

/*****************************************************************************/
/* Free list of descriptive texts.                                           */
/*****************************************************************************/
void
gl_merge_free_long_texts_list (GList ** list)
{
	GList *p;

	for (p = *list; p != NULL; p = p->next) {
		g_free (p->data);
		p->data = NULL;
	}

	g_list_free (*list);
	*list = NULL;
}

/*****************************************************************************/
/* Free list of field definitions.                                           */
/*****************************************************************************/
void
gl_merge_free_field_def_list (GList ** list)
{
	GList *p;
	glMergeFieldDefinition *field_def;

	for (p = *list; p != NULL; p = p->next) {
		field_def = (glMergeFieldDefinition *) p->data;

		g_free (field_def->key);
		field_def->key = NULL;
		g_free (field_def->loc);
		field_def->loc = NULL;

		g_free (p->data);
		p->data = NULL;
	}

	g_list_free (*list);
	*list = NULL;

}

/*****************************************************************************/
/* Extract a list of valid keys from field definitions list                  */
/*****************************************************************************/
GList *
gl_merge_get_key_list (GList * field_defs)
{
	GList *p, *keys;
	glMergeFieldDefinition *field_def;

	keys = NULL;
	for (p = field_defs; p != NULL; p = p->next) {
		field_def = (glMergeFieldDefinition *) p->data;

		keys = g_list_append (keys, g_strdup (field_def->key));
	}

	return keys;
}

/*****************************************************************************/
/* Free a list of keys.                                                      */
/*****************************************************************************/
void
gl_merge_free_key_list (GList ** keys)
{
	GList *p;

	for (p = *keys; p != NULL; p = p->next) {
		g_free (p->data);
		p->data = NULL;
	}

	g_list_free (*keys);
	*keys = NULL;
}

/*****************************************************************************/
/* Lookup key for given locator.                                             */
/*****************************************************************************/
gchar *
gl_merge_find_key (GList * field_defs,
		   gchar * loc)
{
	GList *p;
	glMergeFieldDefinition *field_def;

	for (p = field_defs; p != NULL; p = p->next) {
		field_def = (glMergeFieldDefinition *) p->data;

		if (strcmp (loc, field_def->loc) == 0) {
			return g_strdup (field_def->key);
		}

	}

	return NULL;
}

/*****************************************************************************/
/* Open merge source front-end.                                              */
/*****************************************************************************/
glMergeInput *
gl_merge_open (glMergeType type,
	       GList * field_defs,
	       gchar * src)
{
	return func[type].open (type, field_defs, src);
}

/*****************************************************************************/
/* Close merge source front-end.                                             */
/*****************************************************************************/
void
gl_merge_close (glMergeInput * input)
{
	if ( input != NULL ) {
		func[input->type].close (input);
	}
}

/*****************************************************************************/
/* Get next record from merge source, NULL if exhausted (front-end).         */
/*****************************************************************************/
glMergeRecord *
gl_merge_get_record (glMergeInput * input)
{
	if ( input == NULL ) {
		return NULL;
	}
	return func[input->type].get_record (input);
}

/*****************************************************************************/
/* Get next record (raw) from merge source, NULL if exhausted (front-end).   */
/*****************************************************************************/
GList *
gl_merge_get_raw_record (glMergeInput * input)
{
	if ( input == NULL ) {
		return NULL;
	}
	return func[input->type].get_raw_record (input);
}

/*****************************************************************************/
/* Free a merge record (list of fields)                                      */
/*****************************************************************************/
void
gl_merge_free_record (glMergeRecord ** record)
{
	GList *p;
	glMergeField *field;

	for (p = (*record)->field_list; p != NULL; p = p->next) {
		field = (glMergeField *) p->data;

		g_free (field->key);
		field->key = NULL;
		g_free (field->value);
		field->value = NULL;

		g_free (p->data);
		p->data = NULL;

	}
	g_list_free ((*record)->field_list);
	(*record)->field_list = NULL;

	g_free (*record);
	*record = NULL;
}

/*****************************************************************************/
/* Free a merge record (list of fields)                                      */
/*****************************************************************************/
void
gl_merge_free_raw_record (GList ** record)
{
	GList *p;
	glMergeRawField *field;

	for (p = *record; p != NULL; p = p->next) {
		field = (glMergeRawField *) p->data;

		g_free (field->loc);
		field->loc = NULL;
		g_free (field->value);
		field->value = NULL;

		g_free (p->data);
		p->data = NULL;

	}

	g_list_free (*record);
	*record = NULL;
}

/*****************************************************************************/
/* Find key in given record and evaluate.                                    */
/*****************************************************************************/
extern gchar *
gl_merge_eval_key (gchar * key,
		   glMergeRecord * record)
{
	GList *p;
	glMergeField *field;

	if ( record != NULL ) {
		for (p = record->field_list; p != NULL; p = p->next) {
			field = (glMergeField *) p->data;

			if (strcmp (key, field->key) == 0) {
				return g_strdup (field->value);
			}

		}
	}
	return NULL;
}

/*****************************************************************************/
/* Read all records from merge source.                                       */
/*****************************************************************************/
GList *
gl_merge_read_data(glMergeType type,
		   GList *field_defs,
		   gchar *src)
{
	glMergeInput *mp;
	glMergeRecord *record;
	GList *record_list = NULL;

	mp = gl_merge_open (type, field_defs, src);
	while ( (record = gl_merge_get_record (mp)) != NULL ) {
		record_list = g_list_append( record_list, record );
	}
	gl_merge_close(mp);
	      
	return record_list;
}

/*****************************************************************************/
/* Free a list of records.                                                   */
/*****************************************************************************/
void
gl_merge_free_data (GList ** record_list)
{
	GList *p;
	glMergeRecord *record;

	for (p = *record_list; p != NULL; p = p->next) {
		record = (glMergeRecord *) p->data;

		gl_merge_free_record( &record );

	}

	g_list_free (*record_list);
	*record_list = NULL;
}

/*****************************************************************************/
/* Count selected records.                                                   */
/*****************************************************************************/
gint
gl_merge_count_records (GList *record_list)
{
	GList *p;
	glMergeRecord *record;
	gint count;

	count = 0;
	for ( p=record_list; p!=NULL; p=p->next ) {
		record = (glMergeRecord *)p->data;

		if ( record->select_flag ) count ++;
	}
	return count;
}

