/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  merge_evolution.c:  evolution merge backend module
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


#include "merge-evolution.h"
#include <libebook/e-book.h>

#include <stdio.h>
#include <string.h>

#include "debug.h"

#define DEFAULT_QUERY "(exists \"full_name\")"

/*===========================================*/
/* Private types                             */
/*===========================================*/

struct _glMergeEvolutionPrivate {
	gchar            *query;
	EBook            *book;
	GList            *contacts;
	GList            *fields; /* the fields supported by the addressbook */
	GList            *error_list; /* list of error strings */
};

enum {
	LAST_SIGNAL
};

enum {
	ARG_0,
	ARG_QUERY,
};

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static glMergeClass *parent_class = NULL;


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void           gl_merge_evolution_class_init      (glMergeEvolutionClass *klass);
static void           gl_merge_evolution_instance_init   (glMergeEvolution      *object);
static void           gl_merge_evolution_finalize        (GObject          *object);

static void           gl_merge_evolution_set_property    (GObject          *object,
						     guint             param_id,
						     const GValue     *value,
						     GParamSpec       *pspec);

static void           gl_merge_evolution_get_property    (GObject          *object,
						     guint             param_id,
						     GValue           *value,
						     GParamSpec       *pspec);

static GList         *gl_merge_evolution_get_key_list    (glMerge          *merge);
static gchar         *gl_merge_evolution_get_primary_key (glMerge          *merge);
static void           gl_merge_evolution_open            (glMerge          *merge);
static void           gl_merge_evolution_close           (glMerge          *merge);
static glMergeRecord *gl_merge_evolution_get_record      (glMerge          *merge);
static void           gl_merge_evolution_copy            (glMerge          *dst_merge,
						     glMerge          *src_merge);

/* utility function prototypes go here */
static void           free_field_list                    (GList *fields);


/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_merge_evolution_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (glMergeEvolutionClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_merge_evolution_class_init,
			NULL,
			NULL,
			sizeof (glMergeEvolution),
			0,
			(GInstanceInitFunc) gl_merge_evolution_instance_init,
			NULL
		};

		type = g_type_register_static (GL_TYPE_MERGE,
					       "glMergeEvolution", &info, 0);
	}

	return type;
}

static void
gl_merge_evolution_class_init (glMergeEvolutionClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	glMergeClass *merge_class  = (glMergeClass *) klass;

	gl_debug (DEBUG_MERGE, "START");

	parent_class = g_type_class_peek_parent (klass);

	object_class->set_property = gl_merge_evolution_set_property;
	object_class->get_property = gl_merge_evolution_get_property;

	g_object_class_install_property
                (object_class,
                 ARG_QUERY,
                 g_param_spec_string ("query", NULL, 
					"Query used to select records from the addressbook",
				    "(exists \"full_name\")",
				    (G_PARAM_READABLE | G_PARAM_WRITABLE)));

	object_class->finalize = gl_merge_evolution_finalize;

	merge_class->get_key_list    = gl_merge_evolution_get_key_list;
	merge_class->get_primary_key = gl_merge_evolution_get_primary_key;
	merge_class->open            = gl_merge_evolution_open;
	merge_class->close           = gl_merge_evolution_close;
	merge_class->get_record      = gl_merge_evolution_get_record;
	merge_class->copy            = gl_merge_evolution_copy;

	gl_debug (DEBUG_MERGE, "END");
}

static void
gl_merge_evolution_instance_init (glMergeEvolution *merge_evolution)
{
	gl_debug (DEBUG_MERGE, "START");

	merge_evolution->private = g_new0 (glMergeEvolutionPrivate, 1);
	merge_evolution->private->query = g_strdup(DEFAULT_QUERY);

	gl_debug (DEBUG_MERGE, "END");
}

static void
gl_merge_evolution_finalize (GObject *object)
{
	glMergeEvolution *merge_evolution;

	gl_debug (DEBUG_MERGE, "START");

	g_return_if_fail (object && GL_IS_MERGE_EVOLUTION (object));

	G_OBJECT_CLASS (parent_class)->finalize (object);

	merge_evolution = GL_MERGE_EVOLUTION (object);
	free_field_list(merge_evolution->private->fields);
	g_free (merge_evolution->private->query);
	g_free (merge_evolution->private);

	gl_debug (DEBUG_MERGE, "END");
}

/*--------------------------------------------------------------------------*/
/* Set argument.                                                            */
/*--------------------------------------------------------------------------*/
static void
gl_merge_evolution_set_property (GObject      *object,
			    guint         param_id,
			    const GValue *value,
			    GParamSpec   *pspec)
{
	glMergeEvolution *merge_evolution;

	merge_evolution = GL_MERGE_EVOLUTION (object);

	switch (param_id) {

	case ARG_QUERY:
		g_free (merge_evolution->private->query);
		merge_evolution->private->query = g_value_dup_string (value);
		gl_debug (DEBUG_MERGE, "ARG \"query\" = \"%s\"",
			  merge_evolution->private->query);
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
gl_merge_evolution_get_property (GObject     *object,
			    guint        param_id,
			    GValue      *value,
			    GParamSpec  *pspec)
{
	glMergeEvolution *merge_evolution;

	merge_evolution = GL_MERGE_EVOLUTION (object);

	switch (param_id) {

	case ARG_QUERY:
		g_value_set_string (value, merge_evolution->private->query);
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
gl_merge_evolution_get_key_list (glMerge *merge)
{
	glMergeEvolution   *merge_evolution;
	GList              *key_list, *iter;
	
	gl_debug (DEBUG_MERGE, "BEGIN");

	merge_evolution = GL_MERGE_EVOLUTION (merge);

	/* for the previously retrieved supported fileds, go through them and find
	 * their pretty names */
	for (iter = merge_evolution->private->fields; 
		 iter != NULL; 
		 iter = g_list_next(iter)) 
	{
		key_list = g_list_prepend (key_list, 
			g_strdup (e_contact_pretty_name (*(EContactField *)iter->data)));
	}

	key_list = g_list_reverse (key_list);

	gl_debug (DEBUG_MERGE, "END");

	return key_list;
}

/*--------------------------------------------------------------------------*/
/* Get "primary" key.                                                       */
/*--------------------------------------------------------------------------*/
static gchar *
gl_merge_evolution_get_primary_key (glMerge *merge)
{
	return g_strdup (e_contact_pretty_name(E_CONTACT_FILE_AS));
}

/*--------------------------------------------------------------------------*/
/* Open merge source.                                                       */
/*--------------------------------------------------------------------------*/
static void
gl_merge_evolution_open (glMerge *merge)
{
	glMergeEvolution *merge_evolution;
	EBookQuery *query;
	gboolean status;
	GList *fields, *iter;
	EContactField *field_id;
	GError *error;
	GList **error_list;

	gl_debug (DEBUG_MERGE, "BEGIN");

	merge_evolution = GL_MERGE_EVOLUTION (merge);
	error_list = &merge_evolution->private->error_list; 

	query = e_book_query_from_string(merge_evolution->private->query);
	if (!query) {
		merge_evolution->private->error_list = 
			g_list_append(merge_evolution->private->error_list,
						  g_strdup("Couldn't construct query"));
		g_error_free (error);
		return;
	}

	merge_evolution->private->book = e_book_new_system_addressbook(&error);
	if (!merge_evolution->private->book) {
		*error_list = g_list_append(*error_list,
						g_strdup_printf("Couldn't open addressbook: %s", 
							error->message));
		e_book_query_unref(query);
		g_error_free (error);
		return;
	}

	if (!e_book_open(merge_evolution->private->book, TRUE, &error)) {
		*error_list = g_list_append(*error_list,
						g_strdup_printf("Couldn't open addressbook: %s", 
							error->message));
		g_error_free (error);
		e_book_query_unref(query);
		g_object_unref(merge_evolution->private->book);
		merge_evolution->private->book = NULL;

		return;
	}

	/* fetch the list of fields supported by this address book */
	status = e_book_get_supported_fields(merge_evolution->private->book,
										 &fields, &error);
	if (status == FALSE) {
		*error_list = g_list_append(*error_list,
						g_strdup_printf("Couldn't list available fields: %s", 
							error->message));
		g_error_free (error);
		e_book_query_unref(query);
		g_object_unref(merge_evolution->private->book);
		merge_evolution->private->book = NULL;

		return;
	}

	/* generate a list of field_ids, and put that into private->fields */
	for (iter = fields; iter != NULL; iter = g_list_next(iter)) {
		field_id = g_new(EContactField, 1);
		*field_id = e_contact_field_id(iter->data);

		/* above this value, the data aren't strings anymore */
		if (*field_id >= E_CONTACT_LAST_SIMPLE_STRING) {
			g_free (field_id);
			continue;
		}

		merge_evolution->private->fields = 
			g_list_prepend(merge_evolution->private->fields, field_id);
	}
	free_field_list(fields); /* don't need the list of names anymore */

	gl_debug(DEBUG_MERGE, "Field list length: %d", g_list_length(merge_evolution->private->fields));

	/* finally retrieve the contacts */
	status = e_book_get_contacts (merge_evolution->private->book,
								  query,
								  &merge_evolution->private->contacts,
								  &error);
	if (status == FALSE) {
		*error_list = g_list_append(*error_list,
						g_strdup_printf("Couldn't get contacts: %s", 
							error->message));
		g_error_free (error);
		e_book_query_unref(query);
		free_field_list(merge_evolution->private->fields);
		g_object_unref(merge_evolution->private->book);
		merge_evolution->private->book = NULL;

		return;
	}

	e_book_query_unref(query);

	gl_debug (DEBUG_MERGE, "END");

	return;
	/* XXX I should probably sort the list by name (or the file-as element)*/
}

/*--------------------------------------------------------------------------*/
/* Close merge source.                                                      */
/*--------------------------------------------------------------------------*/
static void
gl_merge_evolution_close (glMerge *merge)
{
	glMergeEvolution *merge_evolution;
	GList *iter;

	merge_evolution = GL_MERGE_EVOLUTION (merge);

	/* unref all of the objects created in _open */
	g_object_unref(merge_evolution->private->book);
	merge_evolution->private->book = NULL;

	for (iter = merge_evolution->private->contacts; 
		 iter != NULL; 
		 iter = g_list_next(iter))
	{
		EContact *contact = E_CONTACT (iter->data);

		g_object_unref(contact);
	}
	g_list_free(merge_evolution->private->contacts);
	merge_evolution->private->contacts = NULL;

	for (iter = merge_evolution->private->error_list; 
		 iter != NULL; 
		 iter = g_list_next(iter))
	{
		g_free(iter->data);
	}
	merge_evolution->private->error_list = NULL;
}

/*--------------------------------------------------------------------------*/
/* Get next record from merge source, NULL if no records left (i.e EOF)     */
/*--------------------------------------------------------------------------*/
static glMergeRecord *
gl_merge_evolution_get_record (glMerge *merge)
{
	glMergeEvolution   *merge_evolution;
	glMergeRecord *record;
	glMergeField  *field;
	EContactField field_id;

	GList *head, *iter; 
	EContact *contact;

	merge_evolution = GL_MERGE_EVOLUTION (merge);

	/* we're in an error state */
	if (merge_evolution->private->error_list) {
		head = merge_evolution->private->error_list;
		char *error_str = head->data;

		/* a yucky hack to make the errors show up in the record list */
		record = g_new0 (glMergeRecord, 1);
		record->select_flag = TRUE;

		field = g_new0 (glMergeField, 1);
		field->key = g_strdup ("full_name");
		field->value = g_strdup (error_str);

		record->field_list = g_list_append (record->field_list, field);

		/* do a destructive read */
		g_free (error_str);
		merge_evolution->private->error_list = 
			g_list_remove_link (merge_evolution->private->error_list, head);
		g_list_free_1 (head);

		return record;
	}

	head = merge_evolution->private->contacts;
	if (head == NULL) {
		return NULL; /* past the last record */
	}
	contact = E_CONTACT(head->data);

	record = g_new0 (glMergeRecord, 1);
	record->select_flag = TRUE;

	/* Take the interesting fields one by one from the contact, and put them
	 * into the glMergeRecord structure. When done, free up the resources for
	 * that contact */

	/* iterate through the supported fields, and add them to the list */
	for (iter = merge_evolution->private->fields;
		 iter != NULL;
		 iter = g_list_next(iter))
	{
		gchar *value;
		field_id = *(EContactField *)iter->data;
		value = g_strdup (e_contact_get_const (contact, field_id));

		if (value) {
			field = g_new0 (glMergeField, 1);
			field->key = g_strdup (e_contact_pretty_name (field_id));
			field->value = value;
			record->field_list = g_list_prepend (record->field_list, field);
		}
	}

	record->field_list = g_list_reverse (record->field_list);

	/* do a destructive read */
	g_object_unref (contact);
	merge_evolution->private->contacts = 
		g_list_remove_link (merge_evolution->private->contacts, head);
	g_list_free_1 (head);

	return record;
}

/*---------------------------------------------------------------------------*/
/* Copy merge_evolution specific fields.                                     */
/*---------------------------------------------------------------------------*/
static void
gl_merge_evolution_copy (glMerge *dst_merge,
		    glMerge *src_merge)
{
	GList *src_iter, *dst_iter;

	gl_debug (DEBUG_MERGE, "BEGIN");

	glMergeEvolution *dst_merge_evolution;
	glMergeEvolution *src_merge_evolution;

	dst_merge_evolution = GL_MERGE_EVOLUTION (dst_merge);
	src_merge_evolution = GL_MERGE_EVOLUTION (src_merge);

	dst_merge_evolution->private->query = g_strdup(src_merge_evolution->private->query);

	dst_merge_evolution->private->fields = g_list_copy(src_merge_evolution->private->fields);
	for (src_iter = src_merge_evolution->private->fields,
		 	dst_iter = dst_merge_evolution->private->fields; 
		 src_iter != NULL && dst_iter != NULL; 
		 src_iter = g_list_next(src_iter), dst_iter = g_list_next(dst_iter))
	{
		dst_iter->data = g_new(EContactField, 1);
		if (src_iter->data) { /* this better not be null, but... */
			memcpy(dst_iter->data, src_iter->data, sizeof(EContactField));
		}
	}

	/* I don't know that there's a good way to do a deep copy of the various
	 * libebook structures/objects, so I'm just going to leave them out.  They
	 * are all regenerated on gl_merge_evolution_open, anyway */

	gl_debug (DEBUG_MERGE, "END");
}

/*---------------------------------------------------------------------------*/
/* Free the list of supported fields                                         */
/*---------------------------------------------------------------------------*/
static void
free_field_list (GList *fields)
{
	GList *iter;

	for (iter = fields; iter != NULL; iter = g_list_next(iter)) 
	{
		if (iter->data) {
			g_free(iter->data);
		}
	}
	g_list_free(fields);
	fields = NULL;
}


#endif /* HAVE_LIBEBOOK */
