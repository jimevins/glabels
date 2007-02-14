/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  merge.c:  document merge module
 *
 *  Copyright (C) 2001-2002  Jim Evins <evins@snaught.com>.
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

#include "merge.h"

#include <glib/gi18n.h>
#include <gobject/gvaluecollector.h>
#include <string.h>

#include "debug.h"

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glMergePrivate {
	gchar             *name;
	gchar             *description;
	gchar             *src;
	glMergeSrcType     src_type;

	GList             *record_list;
};

enum {
	LAST_SIGNAL
};

typedef struct {

	GType              type;
	gchar             *name;
	gchar             *description;
	glMergeSrcType     src_type;

	guint              n_params;
	GParameter        *params;

} Backend;

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static GList *backends = NULL;

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void           gl_merge_finalize      (GObject        *object);

static void           merge_open             (glMerge        *merge);

static void           merge_close            (glMerge        *merge);

static glMergeRecord *merge_get_record       (glMerge        *merge);

static void           merge_free_record      (glMergeRecord **record);

static glMergeRecord *merge_dup_record       (glMergeRecord  *record);

static void           merge_free_record_list (GList         **record_list);

static GList         *merge_dup_record_list  (GList          *record_list);




/*****************************************************************************/
/* Register a new merge backend.                                             */
/*****************************************************************************/
void
gl_merge_register_backend (GType              type,
			   gchar             *name,
			   gchar             *description,
			   glMergeSrcType     src_type,
			   const gchar       *first_arg_name,
			   ...)
{
	Backend      *backend;
	va_list       args;
	const gchar  *pname;
	GObjectClass *class;
	GParamSpec   *pspec;
	GParameter   *params;
	guint         n_params = 0, n_alloced_params = 16;

	backend = g_new0 (Backend, 1);

	backend->type        = type;
	backend->name        = g_strdup (name);
	backend->description = g_strdup (description);
	backend->src_type    = src_type;

	params = g_new (GParameter, n_alloced_params);
	va_start (args, first_arg_name);
	for ( pname=first_arg_name; pname != NULL; pname=va_arg (args,gchar *) ) {
		gchar *error = NULL;

		class = g_type_class_ref (type);
		if (class == NULL) {
			g_message ("%s: unknown object type %d",
				   G_STRLOC, type);
			break;
		}
		pspec = g_object_class_find_property (class, pname);
		if (pspec == NULL) {
			g_message ("%s: object class `%s' has no property named `%s'",
				   G_STRLOC, g_type_name (type), pname);
			break;
		}
		if (n_params >= n_alloced_params) {
			n_alloced_params += 16;
			params = g_renew (GParameter, params, n_alloced_params);
		}
		params[n_params].name = pname;
		params[n_params].value.g_type = 0;
		g_value_init (&params[n_params].value, pspec->value_type);
		G_VALUE_COLLECT (&params[n_params].value, args, 0, &error);
		if (error) {
			g_message ("%s: %s", G_STRLOC, error);
			g_free (error);
			break;
		}

		n_params++;
	}
	va_end (args);

	backend->n_params = n_params;
	backend->params   = params;

	backends = g_list_append (backends, backend);

}

/*****************************************************************************/
/* Get list of registered backend descriptions.                              */
/*****************************************************************************/
GList *
gl_merge_get_descriptions (void)
{
	GList   *descriptions = NULL;
	GList   *p;
	Backend *backend;

	descriptions = g_list_append (descriptions, g_strdup(_("None")));

	for ( p=backends; p!=NULL; p=p->next) {
		backend = (Backend *)p->data;
		descriptions = g_list_append (descriptions,
					      g_strdup(backend->description));
	}

	return descriptions;
}

/*****************************************************************************/
/* Free list of descriptions.                                                */
/*****************************************************************************/
void
gl_merge_free_descriptions (GList **descriptions)
{
	GList *p;

	gl_debug (DEBUG_MERGE, "START");

	for (p = *descriptions; p != NULL; p = p->next) {
		g_free (p->data);
		p->data = NULL;
	}

	g_list_free (*descriptions);
	*descriptions = NULL;

	gl_debug (DEBUG_MERGE, "END");
}

/*****************************************************************************/
/* Lookup name of backend from description.                                  */
/*****************************************************************************/
gchar *
gl_merge_description_to_name (gchar *description)
{
	GList   *p;
	Backend *backend;

	if (g_strcasecmp(description, _("None")) == 0) {
		return g_strdup("None");
	}

	for ( p=backends; p!=NULL; p=p->next) {
		backend = (Backend *)p->data;
		if (g_strcasecmp(description, backend->description) == 0) {
			return g_strdup(backend->name);
		}
	}

	return g_strdup("None");
}

/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
G_DEFINE_TYPE (glMerge, gl_merge, G_TYPE_OBJECT);

static void
gl_merge_class_init (glMergeClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	gl_debug (DEBUG_MERGE, "START");

	gl_merge_parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_merge_finalize;

	gl_debug (DEBUG_MERGE, "END");
}

static void
gl_merge_init (glMerge *merge)
{
	gl_debug (DEBUG_MERGE, "START");

	merge->priv = g_new0 (glMergePrivate, 1);

	gl_debug (DEBUG_MERGE, "END");
}

static void
gl_merge_finalize (GObject *object)
{
	glMerge *merge = GL_MERGE (object);

	gl_debug (DEBUG_MERGE, "START");

	g_return_if_fail (object && GL_IS_MERGE (object));

	merge_free_record_list (&merge->priv->record_list);
	g_free (merge->priv->name);
	g_free (merge->priv->description);
	g_free (merge->priv->src);
	g_free (merge->priv);

	G_OBJECT_CLASS (gl_merge_parent_class)->finalize (object);

	gl_debug (DEBUG_MERGE, "END");
}

/*****************************************************************************/
/* New merge object.                                                         */
/*****************************************************************************/
glMerge *
gl_merge_new (gchar *name)
{
	glMerge *merge = NULL;
	GList   *p;
	Backend *backend;

	gl_debug (DEBUG_MERGE, "START");

	for (p=backends; p!=NULL; p=p->next) {
		backend = (Backend *)p->data;

		if (g_strcasecmp(name, backend->name) == 0) {

			merge = GL_MERGE (g_object_newv (backend->type,
							 backend->n_params,
							 backend->params));

			merge->priv->name        = g_strdup (name);
			merge->priv->description = g_strdup (backend->description);
			merge->priv->src_type    = backend->src_type;

			break;
		}
	}

	if ( (merge == NULL) && (g_strcasecmp (name, "None") != 0)) {
		g_message ("Unknown merge backend \"%s\"", name);
	}

	gl_debug (DEBUG_MERGE, "END");

	return merge;
}

/*****************************************************************************/
/* Duplicate merge.                                                         */
/*****************************************************************************/
glMerge *
gl_merge_dup (glMerge *src_merge)
{
	glMerge    *dst_merge;

	gl_debug (DEBUG_MERGE, "START");

	if (src_merge == NULL) {
		gl_debug (DEBUG_MERGE, "END (NULL)");
		return NULL;
	}

	g_return_val_if_fail (GL_IS_MERGE (src_merge), NULL);

	dst_merge = g_object_new (G_OBJECT_TYPE(src_merge), NULL);
	dst_merge->priv->name        = g_strdup (src_merge->priv->name);
	dst_merge->priv->description = g_strdup (src_merge->priv->description);
	dst_merge->priv->src         = g_strdup (src_merge->priv->src);
	dst_merge->priv->src_type    = src_merge->priv->src_type;
	dst_merge->priv->record_list 
		= merge_dup_record_list (src_merge->priv->record_list);

	if ( GL_MERGE_GET_CLASS(src_merge)->copy != NULL ) {

		/* We have an object specific method, use it */
		GL_MERGE_GET_CLASS(src_merge)->copy (dst_merge, src_merge);

	}

	gl_debug (DEBUG_MERGE, "END");

	return dst_merge;
}

/*****************************************************************************/
/* Get name of merge.                                                        */
/*****************************************************************************/
gchar *
gl_merge_get_name (glMerge *merge)
{
	gl_debug (DEBUG_MERGE, "");

	if (merge == NULL) {
		return g_strdup("None");
	}

	g_return_val_if_fail (GL_IS_MERGE (merge), g_strdup("None"));

	return g_strdup(merge->priv->name);
}

/*****************************************************************************/
/* Get description of merge.                                                 */
/*****************************************************************************/
gchar *
gl_merge_get_description (glMerge *merge)
{
	gl_debug (DEBUG_MERGE, "");

	if (merge == NULL) {
		return g_strdup(_("None"));
	}

	g_return_val_if_fail (GL_IS_MERGE (merge), g_strdup(_("None")));

	return g_strdup(merge->priv->description);
}

/*****************************************************************************/
/* Get source type of merge.                                                 */
/*****************************************************************************/
glMergeSrcType
gl_merge_get_src_type (glMerge *merge)
{
	gl_debug (DEBUG_MERGE, "");

	if (merge == NULL) {
		return GL_MERGE_SRC_IS_FIXED;
	}

	g_return_val_if_fail (GL_IS_MERGE (merge), GL_MERGE_SRC_IS_FIXED);

	return merge->priv->src_type;
}

/*****************************************************************************/
/* Set src of merge.                                                         */
/*****************************************************************************/
void
gl_merge_set_src (glMerge *merge,
		  gchar   *src)
{
	GList         *record_list = NULL;
	glMergeRecord *record;

	gl_debug (DEBUG_MERGE, "START");

	if (merge == NULL)
	{
		gl_debug (DEBUG_MERGE, "END (NULL)");
		return;
	}

	g_return_if_fail (GL_IS_MERGE (merge));

	if ( src == NULL)
	{

		if ( merge->priv->src != NULL )
		{
			g_free (merge->priv->src);
		}
		merge->priv->src = NULL;
		merge_free_record_list (&merge->priv->record_list);

	}
	else
	{

		if ( merge->priv->src != NULL )
		{
			g_free(merge->priv->src);
		}
		merge->priv->src = g_strdup (src);

		merge_free_record_list (&merge->priv->record_list);
			
		merge_open (merge);
		while ( (record = merge_get_record (merge)) != NULL )
		{
			record_list = g_list_append( record_list, record );
		}
		merge_close (merge);
		merge->priv->record_list = record_list;

	}
		     

	gl_debug (DEBUG_MERGE, "END");
}

/*****************************************************************************/
/* Get src of merge.                                                         */
/*****************************************************************************/
gchar *
gl_merge_get_src (glMerge *merge)
{
	gl_debug (DEBUG_MERGE, "");

	if (merge == NULL) {
		return NULL;
	}

	g_return_val_if_fail (GL_IS_MERGE (merge), NULL);

	return g_strdup(merge->priv->src);
}

/*****************************************************************************/
/* Get Key List.                                                             */
/*****************************************************************************/
GList *
gl_merge_get_key_list (glMerge *merge)
{
	GList *key_list = NULL;

	gl_debug (DEBUG_MERGE, "START");

	if (merge == NULL) {
		return NULL;
	}

	g_return_val_if_fail (GL_IS_MERGE (merge), NULL);

	if ( GL_MERGE_GET_CLASS(merge)->get_key_list != NULL ) {

		key_list = GL_MERGE_GET_CLASS(merge)->get_key_list (merge);

	}

	gl_debug (DEBUG_MERGE, "END");

	return key_list;
}

/*****************************************************************************/
/* Free a list of keys.                                                      */
/*****************************************************************************/
void
gl_merge_free_key_list (GList **key_list)
{
	GList *p;

	gl_debug (DEBUG_MERGE, "START");

	for (p = *key_list; p != NULL; p = p->next) {
		g_free (p->data);
		p->data = NULL;
	}

	g_list_free (*key_list);
	*key_list = NULL;

	gl_debug (DEBUG_MERGE, "END");
}

/*****************************************************************************/
/* Get Key List.                                                             */
/*****************************************************************************/
gchar *
gl_merge_get_primary_key (glMerge *merge)
{
	gchar *key = NULL;

	gl_debug (DEBUG_MERGE, "START");

	if (merge == NULL) {
		return NULL;
	}

	g_return_val_if_fail (GL_IS_MERGE (merge), NULL);

	if ( GL_MERGE_GET_CLASS(merge)->get_primary_key != NULL ) {

		key = GL_MERGE_GET_CLASS(merge)->get_primary_key (merge);

	}

	gl_debug (DEBUG_MERGE, "END");

	return key;
}

/*---------------------------------------------------------------------------*/
/* Open merge source.                                                        */
/*---------------------------------------------------------------------------*/
static void
merge_open (glMerge *merge)
{
	gl_debug (DEBUG_MERGE, "START");

	g_return_if_fail (merge && GL_IS_MERGE (merge));

	if ( GL_MERGE_GET_CLASS(merge)->open != NULL ) {

		GL_MERGE_GET_CLASS(merge)->open (merge);

	}

	gl_debug (DEBUG_MERGE, "END");
}

/*---------------------------------------------------------------------------*/
/* Close merge source.                                                       */
/*---------------------------------------------------------------------------*/
static void
merge_close (glMerge *merge)
{
	gl_debug (DEBUG_MERGE, "START");

	g_return_if_fail (merge && GL_IS_MERGE (merge));

	if ( GL_MERGE_GET_CLASS(merge)->close != NULL ) {

		GL_MERGE_GET_CLASS(merge)->close (merge);

	}

	gl_debug (DEBUG_MERGE, "END");
}

/*---------------------------------------------------------------------------*/
/* Get next record (list of fields) from opened merge source.                */
/*---------------------------------------------------------------------------*/
static glMergeRecord *
merge_get_record (glMerge *merge)
{
	glMergeRecord *record = NULL;

	gl_debug (DEBUG_MERGE, "START");

	g_return_val_if_fail (merge && GL_IS_MERGE (merge), NULL);

	if ( GL_MERGE_GET_CLASS(merge)->get_record != NULL ) {

		record = GL_MERGE_GET_CLASS(merge)->get_record (merge);

	}

	gl_debug (DEBUG_MERGE, "END");

	return record;
}

/*---------------------------------------------------------------------------*/
/* Free a merge record (list of fields)                                      */
/*---------------------------------------------------------------------------*/
static void
merge_free_record (glMergeRecord **record)
{
	GList *p;
	glMergeField *field;

	gl_debug (DEBUG_MERGE, "START");

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

	gl_debug (DEBUG_MERGE, "END");
}

/*---------------------------------------------------------------------------*/
/* Duplicate a merge record (list of fields)                                 */
/*---------------------------------------------------------------------------*/
static glMergeRecord *
merge_dup_record (glMergeRecord *record)
{
	glMergeRecord *dest_record;
	GList         *p;
	glMergeField  *dest_field, *field;

	gl_debug (DEBUG_MERGE, "START");

	dest_record = g_new0 (glMergeRecord, 1);
	dest_record->select_flag = record->select_flag;

	for (p = record->field_list; p != NULL; p = p->next) {
		field = (glMergeField *) p->data;

		dest_field = g_new0 (glMergeField, 1);

		dest_field->key   = g_strdup (field->key);
		dest_field->value = g_strdup (field->value);

		dest_record->field_list =
			g_list_append (dest_record->field_list, dest_field);

	}

	gl_debug (DEBUG_MERGE, "END");

	return dest_record;
}

/*****************************************************************************/
/* Find key in given record and evaluate.                                    */
/*****************************************************************************/
gchar *
gl_merge_eval_key (glMergeRecord *record,
		   gchar         *key)
		   
{
	GList        *p;
	glMergeField *field;
	gchar        *val = NULL;

	gl_debug (DEBUG_MERGE, "START");

	if ( (record != NULL) ) {
		for (p = record->field_list; p != NULL; p = p->next) {
			field = (glMergeField *) p->data;

			if (strcmp (key, field->key) == 0) {
				val = g_strdup (field->value);
			}

		}
	}

	gl_debug (DEBUG_MERGE, "END");

	return val;
}

/*****************************************************************************/
/* Read all records from merge source.                                       */
/*****************************************************************************/
const GList *
gl_merge_get_record_list (glMerge *merge)
{
	gl_debug (DEBUG_MERGE, "");
	      
	if ( merge != NULL ) {
		return merge->priv->record_list;
	} else {
		return NULL;
	}
}

/*---------------------------------------------------------------------------*/
/* Free a list of records.                                                   */
/*---------------------------------------------------------------------------*/
static void
merge_free_record_list (GList **record_list)
{
	GList *p;
	glMergeRecord *record;

	gl_debug (DEBUG_MERGE, "START");

	for (p = *record_list; p != NULL; p = p->next) {
		record = (glMergeRecord *) p->data;

		merge_free_record( &record );

	}

	g_list_free (*record_list);
	*record_list = NULL;

	gl_debug (DEBUG_MERGE, "END");
}

/*---------------------------------------------------------------------------*/
/* Duplicate a list of records.                                              */
/*---------------------------------------------------------------------------*/
static GList *
merge_dup_record_list (GList *record_list)
{
	GList         *dest_list = NULL, *p;
	glMergeRecord *dest_record, *record;

	gl_debug (DEBUG_MERGE, "START");

	for (p = record_list; p != NULL; p = p->next) {
		record = (glMergeRecord *) p->data;

		dest_record = merge_dup_record( record );
		dest_list = g_list_append (dest_list, dest_record);
	}


	gl_debug (DEBUG_MERGE, "END");

	return dest_list;
}

/*****************************************************************************/
/* Count selected records.                                                   */
/*****************************************************************************/
gint
gl_merge_get_record_count (glMerge *merge)
{
	GList *p;
	glMergeRecord *record;
	gint count;

	gl_debug (DEBUG_MERGE, "START");

	count = 0;
	for ( p=merge->priv->record_list; p!=NULL; p=p->next ) {
		record = (glMergeRecord *)p->data;

		if ( record->select_flag ) count ++;
	}

	gl_debug (DEBUG_MERGE, "END");

	return count;
}


