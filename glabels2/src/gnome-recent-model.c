/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 * Authors:
 *   James Willcox <jwillcox@cs.indiana.edu>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include <libbonoboui.h>
#include <libgnomevfs/gnome-vfs.h>
#include "gnome-recent-model.h"
#include "gnome-recent-view.h"

#define GNOME_RECENT_MODEL_BASE_KEY "/desktop/gnome/recent_files"
#define GNOME_RECENT_MODEL_GLOBAL_LIMIT_KEY "global_limit"
#define GNOME_RECENT_MODEL_GLOBAL_LIMIT_ENV "GNOME_RECENT_MODEL_GLOBAL_LIMIT"
#define GNOME_RECENT_MODEL_GLOBAL_LIST "gnome-recent-global"

static void gnome_recent_model_class_init      (GnomeRecentModelClass * klass);
static void gnome_recent_model_init            (GnomeRecentModel * recent);
static gchar *gnome_recent_model_gconf_key     (GnomeRecentModel * recent);
static void gnome_recent_model_notify_cb       (GConfClient *client,
						guint cnxn_id,
						GConfEntry *entry,
						gpointer user_data);
static GSList * gnome_recent_model_delete_from_list (GnomeRecentModel *recent,
					       GSList *list,
					       const gchar *uri);
static GSList * gnome_recent_model_gconf_to_list (GConfValue* value);
static void gnome_recent_model_g_slist_deep_free (GSList *list);
static void gnome_recent_model_set_appname (GnomeRecentModel *recent, gchar *appname);

struct _GnomeRecentModel {
	GObject parent_instance;	/* We emit signals */

	gchar *appname; 		/* the app that owns this object */
	GConfClient *gconf_client;	/* we use GConf to store stuff */
	unsigned int limit;		/* maximum number of items to store */

	GnomeRecentModel *global;	/* Another GnomeRecentModel object,
					 * representing the global
					 * recent uri list
					 */

	GHashTable *monitors;		/* A hash table holding
					 * GnomeVfsMonitorHandle objects.
					 */
};

struct _GnomeRecentModelClass {
	GObjectClass parent_class;
	
	void (*changed) (GnomeRecentModel *recent, const GSList *list);
};

struct _GnomeRecentModelMenuData {
	GnomeRecentModel *recent;
	gchar *uri;
};

typedef struct _GnomeRecentModelMenuData GnomeRecentModelMenuData;

enum {
	CHANGED,
	LAST_SIGNAL
};

/* GObject properties */
enum {
	PROP_BOGUS,
	PROP_APPNAME,
	PROP_LIMIT,
};

static GType model_signals[LAST_SIGNAL] = { 0 };
static GObjectClass *parent_class = NULL;

/**
 * gnome_recent_model_get_type:
 * @:
 *
 * This returns a GType representing a GnomeRecentModel object.
 *
 * Returns: a GType
 */
GType
gnome_recent_model_get_type (void)
{
	static GType gnome_recent_model_type = 0;

	if(!gnome_recent_model_type) {
		static const GTypeInfo gnome_recent_model_info = {
			sizeof (GnomeRecentModelClass),
			NULL, /* base init */
			NULL, /* base finalize */
			(GClassInitFunc)gnome_recent_model_class_init, /* class init */
			NULL, /* class finalize */
			NULL, /* class data */
			sizeof (GnomeRecentModel),
			0,
			(GInstanceInitFunc) gnome_recent_model_init
		};

		gnome_recent_model_type = g_type_register_static (G_TYPE_OBJECT,
							"GnomeRecentModel",
							&gnome_recent_model_info, 0);
	}

	return gnome_recent_model_type;
}

static void
gnome_recent_model_set_property (GObject *object,
			   guint prop_id,
			   const GValue *value,
			   GParamSpec *pspec)
{
	GnomeRecentModel *recent = GNOME_RECENT_MODEL (object);
	gchar *appname;

	switch (prop_id)
	{
		case PROP_APPNAME:
			appname = g_strdup (g_value_get_string (value));
			gnome_recent_model_set_appname (recent, appname);
		break;
		case PROP_LIMIT:
			gnome_recent_model_set_limit (GNOME_RECENT_MODEL (recent),
						g_value_get_int (value));
		break;
		default:
		break;
	}
}

static void
gnome_recent_model_get_property (GObject *object,
			   guint prop_id,
			   GValue *value,
			   GParamSpec *pspec)
{
	GnomeRecentModel *recent = GNOME_RECENT_MODEL (object);

	switch (prop_id)
	{
		case PROP_APPNAME:
			g_value_set_string (value, recent->appname);
		break;
		case PROP_LIMIT:
			g_value_set_int (value, recent->limit);
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
gnome_recent_model_class_init (GnomeRecentModelClass * klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	object_class->set_property = gnome_recent_model_set_property;
	object_class->get_property = gnome_recent_model_get_property;

	model_signals[CHANGED] = g_signal_new ("changed",
			G_OBJECT_CLASS_TYPE (object_class),
			G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET (GnomeRecentModelClass, changed),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE, 1,
			G_TYPE_POINTER);

	g_object_class_install_property (object_class,
					 PROP_APPNAME,
					 g_param_spec_string ("appname",
						 	      "Application Name",
							      "The name of the application using this object.",
							      "gnome-app",
							      G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
					 PROP_LIMIT,
					 g_param_spec_int    ("limit",
						 	      "Limit",
							      "The maximum number of items to be allowed in the list.",
							      1,
							      1000,
							      10,
							      G_PARAM_READWRITE));

	klass->changed = NULL;
}


static void
gnome_recent_model_init (GnomeRecentModel * recent)
{
	int argc=0;
	char **argv=NULL;

	if (!gconf_init (argc, argv, NULL))
	{
		g_warning ("GConf Initialization failed.");
		return;
	}
	
	if (!gnome_vfs_init ()) {
		g_warning ("gnome-vfs initialization failed.");
		return;
	}

	recent->gconf_client = gconf_client_get_default ();
	recent->monitors = g_hash_table_new (g_str_hash, g_str_equal);
}

static gint
gnome_recent_model_get_global_limit (GnomeRecentModel *model)
{
	char *key;
	gint limit;

	key = g_strdup_printf ("%s/%s", GNOME_RECENT_MODEL_BASE_KEY,
			       GNOME_RECENT_MODEL_GLOBAL_LIMIT_KEY);


	limit = gconf_client_get_int (model->gconf_client,
				      key, NULL);

	if (limit <= 0) {
		/* ok, gconf schemas are not functioning, so assign a sane value */
		limit = 10;
	}

	g_free (key);
	
	return limit;
}


/**
 * gnome_recent_model_new:
 * @appname: The name of your application.
 * @limit:  The maximum number of items allowed.
 *
 * This creates a new GnomeRecentModel object.
 *
 * Returns: a GnomeRecentModel object
 */
GnomeRecentModel *
gnome_recent_model_new (const gchar *appname, gint limit)
{
	GnomeRecentModel *model;

	g_return_val_if_fail (appname, NULL);
	g_return_val_if_fail (limit > 0, NULL);

	model = GNOME_RECENT_MODEL (g_object_new (gnome_recent_model_get_type (),
					   "appname",
					   appname,
					   "limit",
					   limit, NULL));

	g_return_val_if_fail (model, NULL);
	
	return model;
}

/**
 * gnome_recent_model_new_global:
 * @
 *
 * This creates a new GnomeRecentModel object, with the global history list.
 *
 * Returns: a GnomeRecentModel object
 */
GnomeRecentModel *
gnome_recent_model_new_global (void)
{
	GnomeRecentModel *model;
	gint limit;

	model = GNOME_RECENT_MODEL (g_object_new(gnome_recent_model_get_type (),
					   "appname",
					   GNOME_RECENT_MODEL_GLOBAL_LIST,
					   NULL));

	g_return_val_if_fail (model, NULL);

	limit = gnome_recent_model_get_global_limit (model);
	gnome_recent_model_set_limit (model, limit);
	
	return model;
}



static void
gnome_recent_model_monitor_cb (GnomeVFSMonitorHandle *handle,
			 const gchar *monitor_uri,
			 const gchar *info_uri,
			 GnomeVFSMonitorEventType event_type,
			 gpointer data)
{
	GnomeRecentModel *recent= GNOME_RECENT_MODEL (data);

	g_return_if_fail (recent);

	/* if a file was deleted, we just remove it from our list */
	switch (event_type) {
		case GNOME_VFS_MONITOR_EVENT_DELETED:
			gnome_recent_model_delete (recent, monitor_uri);
			g_hash_table_remove (recent->monitors, monitor_uri);
			break;
		default:
		break;
	}

}

static void
gnome_recent_model_monitor_uri (GnomeRecentModel *recent, const gchar *uri)
{
	GnomeVFSMonitorHandle *handle=NULL;
	GnomeVFSResult result;

	g_return_if_fail (recent);
	g_return_if_fail (GNOME_IS_RECENT_MODEL (recent));
	g_return_if_fail (uri);

	handle = g_hash_table_lookup (recent->monitors, uri);
	if (handle == NULL) {

		/* this is a new uri, so we need to monitor it */
		result = gnome_vfs_monitor_add (&handle,
				       uri,
				       GNOME_VFS_MONITOR_FILE,
				       gnome_recent_model_monitor_cb,
				       recent);
		if (result == GNOME_VFS_OK) {
			g_hash_table_insert (recent->monitors,
					     g_strdup (uri),
					     handle);
		}
	}
}

static void
gnome_recent_model_monitor_uri_list (GnomeRecentModel *recent,
				     GSList *list)
{
	GSList *p;
	const gchar *uri;

	p = list;
	while (p != NULL) {
		uri = (const gchar *)p->data;

		gnome_recent_model_monitor_uri (recent, uri);

		p = p->next;
	}
}

#if 0
static void
gnome_recent_model_monitor_cancel (GnomeRecentModel *recent, const gchar *uri)
{
	g_return_if_fail (recent);
	g_return_if_fail (GNOME_IS_RECENT_MODEL (recent));
	g_return_if_fail (uri);

	g_hash_table_remove (recent->monitors, uri);
}
#endif

/**
 * gnome_recent_model_add:
 * @recent:  A GnomeRecentModel object.
 * @uri: The URI you want to add to the list.
 *
 * This function adds a URI to the list of recently used URIs.
 *
 * Returns: a gboolean
 */
gboolean
gnome_recent_model_add (GnomeRecentModel * recent, const gchar * uri)
{
	GSList *uri_lst;
	gchar *gconf_key;

	g_return_val_if_fail (recent, FALSE);
	g_return_val_if_fail (GNOME_IS_RECENT_MODEL (recent), FALSE);
	g_return_val_if_fail (recent->gconf_client, FALSE);
	g_return_val_if_fail (uri, FALSE);

	gconf_key = gnome_recent_model_gconf_key (recent);


	uri_lst = gconf_client_get_list (recent->gconf_client,
				       gconf_key,
				       GCONF_VALUE_STRING, NULL);

	/* if this is already in our list, remove it */
	uri_lst = gnome_recent_model_delete_from_list (recent, uri_lst, uri);

	/* prepend the new one */
	uri_lst = g_slist_prepend (uri_lst, g_strdup (uri));

	/* if we're over the limit, delete from the end */
	while (g_slist_length (uri_lst) > recent->limit)
	{
		gchar *tmp_uri;
		tmp_uri = g_slist_nth_data (uri_lst, g_slist_length (uri_lst)-1);
		uri_lst = g_slist_remove (uri_lst, tmp_uri);
		g_free (tmp_uri);
	}
	
	gconf_client_set_list (recent->gconf_client,
			      gconf_key,
			      GCONF_VALUE_STRING,
			      uri_lst, NULL);

	gconf_client_suggest_sync (recent->gconf_client, NULL);

	/* add to the global list */
	if (recent->global)
		gnome_recent_model_add (GNOME_RECENT_MODEL (recent->global), uri);

	g_free (gconf_key);
	gnome_recent_model_g_slist_deep_free (uri_lst);

	return TRUE;
}


/**
 * gnome_recent_model_delete:
 * @recent:  A GnomeRecentModel object.
 * @uri: The URI you want to delete from the list.
 *
 * This function deletes a URI from the list of recently used URIs.
 *
 * Returns: a gboolean
 */
gboolean
gnome_recent_model_delete (GnomeRecentModel * recent, const gchar * uri)
{
	GSList *uri_lst;
	GSList *new_uri_lst;
	gboolean ret = FALSE;
	gchar *gconf_key;

	g_return_val_if_fail (recent, FALSE);
	g_return_val_if_fail (GNOME_IS_RECENT_MODEL (recent), FALSE);
	g_return_val_if_fail (recent->gconf_client, FALSE);
	g_return_val_if_fail (uri, FALSE);

	gconf_key = gnome_recent_model_gconf_key (recent);
	uri_lst = gconf_client_get_list (recent->gconf_client,
				       gconf_key,
				       GCONF_VALUE_STRING, NULL);

	new_uri_lst = gnome_recent_model_delete_from_list (recent, uri_lst, uri);

	/* if it wasn't deleted, no need to cause unneeded updates */
	/*
	if (new_uri_lst == uri_lst) {
		return FALSE;
	}
	else
		uri_lst = new_uri_lst;
	*/

	/* delete it from gconf */
	gconf_client_set_list (recent->gconf_client,
			       gconf_key,
			       GCONF_VALUE_STRING,
			       new_uri_lst,
			       NULL);
	gconf_client_suggest_sync (recent->gconf_client, NULL);

	/* delete from the global list */
	if (recent->global)
		gnome_recent_model_delete (GNOME_RECENT_MODEL (recent->global), uri);


	g_free (gconf_key);
	gnome_recent_model_g_slist_deep_free (new_uri_lst);

	return ret;
}

/**
 * gnome_recent_model_get_list:
 * @recent: A GnomeRecentModel object.
 *
 * This returns a linked list of strings (URIs) currently held
 * by this object.
 *
 * Returns: A GSList *
 */
GSList *
gnome_recent_model_get_list (GnomeRecentModel * recent)
{
	GSList *uri_lst;
	gchar *gconf_key = gnome_recent_model_gconf_key (recent);

	g_return_val_if_fail (recent, NULL);
	g_return_val_if_fail (recent->gconf_client, NULL);
	g_return_val_if_fail (GNOME_IS_RECENT_MODEL (recent), NULL);

	uri_lst = gconf_client_get_list (recent->gconf_client,
				       gconf_key,
				       GCONF_VALUE_STRING, NULL);

	g_free (gconf_key);

	/* FIXME:  This sucks. */
	gnome_recent_model_monitor_uri_list (recent, uri_lst);

	return uri_lst;
}



/**
 * gnome_recent_model_set_limit:
 * @recent: A GnomeRecentModel object.
 * @limit: The maximum number of items allowed in the list.
 *
 * Use this function to constrain the number of items allowed in the list.
 * The default is %GNOME_RECENT_MODEL_DEFAULT_LIMIT.
 *
 */
void
gnome_recent_model_set_limit (GnomeRecentModel *recent, gint limit)
{
	GSList *list;
	int len;
	unsigned int i;

	g_return_if_fail (recent);
	g_return_if_fail (GNOME_IS_RECENT_MODEL (recent));
	g_return_if_fail (limit > 0);
	recent->limit = limit;

	list = gnome_recent_model_get_list (recent);
	len = g_slist_length (list);

	if (len <= limit) return;

	/* if we're over the limit, delete from the end */
	i=g_slist_length (list);
	while (i > recent->limit)
	{
		gchar *uri = g_slist_nth_data (list, i-1);
		gnome_recent_model_delete (recent, uri);

		i--;
	}

	gnome_recent_model_g_slist_deep_free (list);
}


/**
 * gnome_recent_model_get_limit:
 * @recent: A GnomeRecentModel object.
 *
 */
gint
gnome_recent_model_get_limit (GnomeRecentModel *recent)
{
	g_return_val_if_fail (recent, -1);
	g_return_val_if_fail (GNOME_IS_RECENT_MODEL (recent), -1);

	return recent->limit;
}


/**
 * gnome_recent_model_clear:
 * @recent: A GnomeRecentModel object.
 *
 * This function clears the list of recently used URIs.
 *
 */
void
gnome_recent_model_clear (GnomeRecentModel *recent)
{
	gchar *key;

	g_return_if_fail (recent);
	g_return_if_fail (recent->gconf_client);
	g_return_if_fail (GNOME_IS_RECENT_MODEL (recent));

	key = gnome_recent_model_gconf_key (recent);

	gconf_client_unset (recent->gconf_client, key, NULL);
}

static void
gnome_recent_model_set_appname (GnomeRecentModel *recent, gchar *appname)
{
	gchar *key;
	gint notify_id;

	g_return_if_fail (recent);
	g_return_if_fail (appname);

	recent->appname = appname;

	/* if this isn't the global list embed a global one */
	if (strcmp (appname, GNOME_RECENT_MODEL_GLOBAL_LIST)) {
		recent->global = gnome_recent_model_new_global ();
	}

	/* Set up the gconf notification stuff */
	key = gnome_recent_model_gconf_key (recent);
	gconf_client_add_dir (recent->gconf_client,
			GNOME_RECENT_MODEL_BASE_KEY, GCONF_CLIENT_PRELOAD_NONE, NULL);
	notify_id = gconf_client_notify_add (recent->gconf_client,
					    key,
					    gnome_recent_model_notify_cb,
					    recent, NULL, NULL);



	g_free (key);
}

static GSList *
gnome_recent_model_delete_from_list (GnomeRecentModel *recent, GSList *list,
			       const gchar *uri)
{
	unsigned int i;
	gchar *text;

	for (i = 0; i < g_slist_length (list); i++) {
		text = g_slist_nth_data (list, i);
		
		if (!strcmp (text, uri)) {
			list = g_slist_remove (list, text);
			g_free (text);
		}
	}

	return list;
}

/* this takes a list of GConfValues, and returns a list of strings */
static GSList *
gnome_recent_model_gconf_to_list (GConfValue* value)
{    
	GSList* iter;
	GSList *list = NULL;

	g_return_val_if_fail (value, NULL);

	iter = gconf_value_get_list(value);

	while (iter != NULL)
	{
		GConfValue* element = iter->data;
		gchar *text = g_strdup (gconf_value_get_string (element));

		list = g_slist_prepend (list, text);

		iter = g_slist_next(iter);
	}

	list = g_slist_reverse (list);

	return list;
}

static void
gnome_recent_model_g_slist_deep_free (GSList *list)
{
	GSList *lst;

	if (list == NULL)
		return;

	lst = list;
	while (lst) {
		g_free (lst->data);
		lst->data = NULL;
		lst = lst->next;
	}

	g_slist_free (list);
}

static gchar *
gnome_recent_model_gconf_key (GnomeRecentModel * model)
{
	gchar *key;

	g_return_val_if_fail (model, NULL);

	key = g_strdup_printf ("%s/%s", GNOME_RECENT_MODEL_BASE_KEY, model->appname);
	return key;
}

/*
static void
print_list (GSList *list)
{
	while (list) {
		g_print ("%s, ", (char *)list->data);

		list = list->next;
	}
	g_print ("\n\n");
}
*/

/* this is the gconf notification callback. */
static void
gnome_recent_model_notify_cb (GConfClient *client, guint cnxn_id,
			GConfEntry *entry, gpointer user_data)
{
	GSList *list=NULL;
	GnomeRecentModel *recent = user_data;

	if (entry->value == NULL) {
		g_signal_emit (G_OBJECT(recent), model_signals[CHANGED], 0, NULL);
		return;
	}

	list = gnome_recent_model_gconf_to_list (entry->value);

	gnome_recent_model_monitor_uri_list (recent, list);

	g_signal_emit (G_OBJECT(recent), model_signals[CHANGED], 0, list);

	gnome_recent_model_g_slist_deep_free (list);
}


gchar *
gnome_recent_model_get_appname (GnomeRecentModel *model)
{
	return g_strdup (model->appname);
}
