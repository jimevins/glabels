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
#include "gnome-recent-view-bonobo.h"
#include "gnome-recent-util.h"
#include "gnome-recent-marshal.h"

#define GNOME_RECENT_VERB_NAME "-uri-"

struct _GnomeRecentViewBonobo {
	GObject parent_instance;	/* We emit signals */

	BonoboUIComponent *uic;
	gchar *path;			/* The menu path where our stuff
					 *  will go
					 */

	gulong changed_cb_id;

	GnomeRecentModel *model;
};

struct _GnomeRecentViewBonoboClass {
	GObjectClass parent_class;
	
	void (*activate) (GnomeRecentViewBonobo *view, const gchar *uri);
};

struct _GnomeRecentViewBonoboMenuData {
	GnomeRecentViewBonobo *view;
	gchar *uri;
};

typedef struct _GnomeRecentViewBonoboMenuData GnomeRecentViewBonoboMenuData;

enum {
	ACTIVATE,
	LAST_SIGNAL
};

/* GObject properties */
enum {
	PROP_BOGUS,
	PROP_UI_COMPONENT,
	PROP_MENU_PATH
};

static guint gnome_recent_view_bonobo_signals[LAST_SIGNAL] = { 0 };

static void
gnome_recent_view_bonobo_clear (GnomeRecentView *view_parent)
{
	gint i=1;
	gboolean done=FALSE;
	gchar *appname;
	GnomeRecentModel *model;
	GnomeRecentViewBonobo *view;

	g_return_if_fail (view_parent);
	view = GNOME_RECENT_VIEW_BONOBO (view_parent);
	g_return_if_fail (view->uic);

	model = gnome_recent_view_get_model (GNOME_RECENT_VIEW (view));
	appname = gnome_recent_model_get_appname (model);
	
	while (!done)
	{
		gchar *verb_name = g_strdup_printf ("%s%s%d", appname,GNOME_RECENT_VERB_NAME, i);
		gchar *item_path = g_strconcat (view->path, "/", verb_name, NULL);
		if (bonobo_ui_component_path_exists (view->uic, item_path, NULL))
			bonobo_ui_component_rm (view->uic, item_path, NULL);
		else
			done=TRUE;

		g_free (item_path);
		g_free (verb_name);

		i++;
	}

	g_free (appname);
}

static void
gnome_recent_view_bonobo_menu_cb (BonoboUIComponent *uic, gpointer data, const char *cname)
{
	gboolean ret;
	GnomeRecentViewBonoboMenuData *md = (GnomeRecentViewBonoboMenuData *) data;
	GnomeRecentModel *model;

	g_return_if_fail (md);
	g_return_if_fail (md->uri);
	g_return_if_fail (md->view);
	g_return_if_fail (GNOME_IS_RECENT_VIEW_BONOBO (md->view));

	ret = FALSE;
	g_signal_emit (G_OBJECT(md->view),
		       gnome_recent_view_bonobo_signals[ACTIVATE], 0,
		       md->uri, &ret);

	if (!ret) {
		model = gnome_recent_view_get_model (GNOME_RECENT_VIEW (md->view));
		gnome_recent_model_delete (model, md->uri);
	}
}

static void
gnome_recent_view_bonobo_menu_data_destroy_cb (gpointer data, GClosure *closure)
{
	GnomeRecentViewBonoboMenuData *md = data;

	g_free (md->uri);
	g_free (md);
}


static void
gnome_recent_view_bonobo_set_list (GnomeRecentViewBonobo *view, GSList *list)
{
	BonoboUIComponent* ui_component;
	unsigned int i;
	gchar *label = NULL;
	gchar *verb_name = NULL;
	gchar *tip = NULL;
	gchar *escaped_name = NULL;
	gchar *item_path = NULL;
	gchar *uri;
	gchar *cmd;
	gchar *appname;
	GnomeRecentViewBonoboMenuData *md;
	GnomeRecentModel *model;
	GClosure *closure;

	g_return_if_fail (view);

	ui_component = view->uic;
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (ui_component));

	model = gnome_recent_view_get_model (GNOME_RECENT_VIEW (view));
	appname = gnome_recent_model_get_appname (model);

	gnome_recent_view_bonobo_clear (GNOME_RECENT_VIEW (view));
	
	bonobo_ui_component_freeze (ui_component, NULL);

	for (i = 1; i <= g_slist_length (list); ++i)
	{
		
		/* this is what gets passed to our private "activate" callback */
		md = (GnomeRecentViewBonoboMenuData *)g_malloc (sizeof (GnomeRecentViewBonoboMenuData));
		md->view = view;
		md->uri = g_strdup (g_slist_nth_data (list, i-1));

		/* Maybe we should use a gnome-vfs call here?? */
		uri = g_path_get_basename (g_slist_nth_data (list, i - 1));
	
		escaped_name = gnome_recent_util_escape_underlines (uri);

		tip =  g_strdup_printf (_("Open %s"), uri);

		verb_name = g_strdup_printf ("%s%s%d", appname,GNOME_RECENT_VERB_NAME, i);
		cmd = g_strdup_printf ("<cmd name = \"%s\" /> ", verb_name);
		bonobo_ui_component_set_translate (ui_component, "/commands/", cmd, NULL);

		closure = g_cclosure_new (G_CALLBACK (gnome_recent_view_bonobo_menu_cb),
					  md, gnome_recent_view_bonobo_menu_data_destroy_cb);
					  
		bonobo_ui_component_add_verb_full (ui_component, verb_name,
						   closure); 
	        
		if (i < 10)
			label = g_strdup_printf ("_%d. %s", i, escaped_name);
		else
			label = g_strdup_printf ("%d. %s", i, escaped_name);
			
		
		
		item_path = g_strconcat (view->path, "/", verb_name, NULL);

		if (bonobo_ui_component_path_exists (ui_component, item_path, NULL))
		{
			bonobo_ui_component_set_prop (ui_component, item_path, 
					              "label", label, NULL);

			bonobo_ui_component_set_prop (ui_component, item_path, 
					              "tip", tip, NULL);
		}
		else
		{
			gchar *xml;

			xml = g_strdup_printf ("<menuitem name=\"%s\" "
						"verb=\"%s\""
						" _label=\"%s\"  _tip=\"%s\" "
						"hidden=\"0\" />", 
						verb_name, verb_name, label,
						tip);

			bonobo_ui_component_set_translate (ui_component, view->path, xml, NULL);

			g_free (xml); 
		}
		
		g_free (label);
		g_free (verb_name);
		g_free (tip);
		g_free (escaped_name);
		g_free (item_path);
		g_free (uri);
		g_free (cmd);
	}


	bonobo_ui_component_thaw (ui_component, NULL);
}

static void
model_changed_cb (GnomeRecentModel *model, GSList *list, GnomeRecentViewBonobo *view)
{
	gnome_recent_view_bonobo_set_list (view, list);
}



static void
gnome_recent_view_bonobo_populate (GnomeRecentViewBonobo *view)
{
	GnomeRecentModel *model;
	GSList *list;

	model = gnome_recent_view_get_model (GNOME_RECENT_VIEW (view));
	list = gnome_recent_model_get_list (model);

	gnome_recent_view_bonobo_set_list (view, list);
}

static GnomeRecentModel *
gnome_recent_view_bonobo_get_model (GnomeRecentView *view_parent)
{
	GnomeRecentViewBonobo *view;
	
	g_return_val_if_fail (view_parent, NULL);
	view = GNOME_RECENT_VIEW_BONOBO (view_parent);
	
	return view->model;
}

static void
gnome_recent_view_bonobo_set_model (GnomeRecentView *view_parent, GnomeRecentModel *model)
{
	GnomeRecentViewBonobo *view;
	
	g_return_if_fail (view_parent);
	view = GNOME_RECENT_VIEW_BONOBO (view_parent);
	
	if (view->model)
		g_signal_handler_disconnect (G_OBJECT (view->model),
					     view->changed_cb_id);
	
	view->model = model;
	view->changed_cb_id = g_signal_connect (G_OBJECT (model), "changed",
					G_CALLBACK (model_changed_cb), view);

	gnome_recent_view_bonobo_populate (view);
}

static void
gnome_recent_view_bonobo_set_property (GObject *object,
			   guint prop_id,
			   const GValue *value,
			   GParamSpec *pspec)
{
	GnomeRecentViewBonobo *view = GNOME_RECENT_VIEW_BONOBO (object);

	switch (prop_id)
	{
		case PROP_UI_COMPONENT:
			gnome_recent_view_bonobo_set_ui_component (GNOME_RECENT_VIEW_BONOBO (view),
						       BONOBO_UI_COMPONENT (g_value_get_object (value)));
		break;
		case PROP_MENU_PATH:
			view->path = g_strdup (g_value_get_string (value));
		break;
		default:
		break;
	}
}

static void
gnome_recent_view_bonobo_get_property (GObject *object,
			   guint prop_id,
			   GValue *value,
			   GParamSpec *pspec)
{
	GnomeRecentViewBonobo *view = GNOME_RECENT_VIEW_BONOBO (object);

	switch (prop_id)
	{
		case PROP_UI_COMPONENT:
			g_value_set_pointer (value, view->uic);
		break;
		case PROP_MENU_PATH:
			g_value_set_string (value, g_strdup (view->path));
		break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
gnome_recent_view_bonobo_class_init (GnomeRecentViewBonoboClass * klass)
{
	GObjectClass *object_class;

	
	object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = gnome_recent_view_bonobo_set_property;
	object_class->get_property = gnome_recent_view_bonobo_get_property;

	gnome_recent_view_bonobo_signals[ACTIVATE] = g_signal_new ("activate",
			G_OBJECT_CLASS_TYPE (object_class),
			G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET (GnomeRecentViewBonoboClass, activate),
			NULL, NULL,
			gnome_recent_BOOLEAN__STRING,
			G_TYPE_BOOLEAN, 1,
			G_TYPE_STRING);

	g_object_class_install_property (object_class,
					 PROP_UI_COMPONENT,
					 g_param_spec_object ("ui-component",
					   "UI Component",
					   "BonoboUIComponent for menus.",
					   bonobo_ui_component_get_type(),
					   G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
					 PROP_MENU_PATH,
					 g_param_spec_string ("ui-path",
					   "Path",
					   "The path to put the menu items.",
					   "/menus/File/GnomeRecentDocuments",
					   G_PARAM_READWRITE));


	klass->activate = NULL;
}

static void
gnome_recent_view_init (GnomeRecentViewClass *iface)
{
	iface->do_clear = gnome_recent_view_bonobo_clear;
	iface->do_get_model = gnome_recent_view_bonobo_get_model;
	iface->do_set_model = gnome_recent_view_bonobo_set_model;
}


static void
gnome_recent_view_bonobo_init (GnomeRecentViewBonobo * recent)
{
	/* maybe should remove this */
}

void
gnome_recent_view_bonobo_set_ui_component (GnomeRecentViewBonobo *view, BonoboUIComponent *uic)
{
	g_return_if_fail (view);
	g_return_if_fail (uic);

	view->uic = uic;
}

void
gnome_recent_view_bonobo_set_ui_path (GnomeRecentViewBonobo *view, const gchar *path)
{
	g_return_if_fail (view);
	g_return_if_fail (path);

	view->path = g_strdup (path);
}

BonoboUIComponent *
gnome_recent_view_bonobo_get_ui_component (GnomeRecentViewBonobo *view)
{
	g_return_val_if_fail (view, NULL);

	return view->uic;
}

gchar *
gnome_recent_view_bonobo_get_ui_path (GnomeRecentViewBonobo *view)
{
	g_return_val_if_fail (view, NULL);

	return g_strdup (view->path);
}

/**
 * gnome_recent_view_bonobo_new:
 * @appname: The name of your application.
 * @limit:  The maximum number of items allowed.
 *
 * This creates a new GnomeRecentViewBonobo object.
 *
 * Returns: a GnomeRecentViewBonobo object
 */
GnomeRecentViewBonobo *
gnome_recent_view_bonobo_new (BonoboUIComponent *uic, const gchar *path)
{
	GnomeRecentViewBonobo *view;

	g_return_val_if_fail (uic, NULL);
	g_return_val_if_fail (path, NULL);

	view = GNOME_RECENT_VIEW_BONOBO (g_object_new (gnome_recent_view_bonobo_get_type (),
					   "ui-path",
					   path,
					   "ui-component",
					   uic, NULL));

	g_return_val_if_fail (view, NULL);
	
	return view;
}

/**
 * gnome_recent_view_bonobo_get_type:
 * @:
 *
 * This returns a GType representing a GnomeRecentViewBonobo object.
 *
 * Returns: a GType
 */
GType
gnome_recent_view_bonobo_get_type (void)
{
	static GType gnome_recent_view_bonobo_type = 0;

	if(!gnome_recent_view_bonobo_type) {
		static const GTypeInfo gnome_recent_view_bonobo_info = {
			sizeof (GnomeRecentViewBonoboClass),
			NULL, /* base init */
			NULL, /* base finalize */
			(GClassInitFunc)gnome_recent_view_bonobo_class_init, /* class init */
			NULL, /* class finalize */
			NULL, /* class data */
			sizeof (GnomeRecentViewBonobo),
			0,
			(GInstanceInitFunc) gnome_recent_view_bonobo_init
		};

		static const GInterfaceInfo view_info =
		{
			(GInterfaceInitFunc) gnome_recent_view_init,
			NULL,
			NULL
		};

		gnome_recent_view_bonobo_type = g_type_register_static (G_TYPE_OBJECT,
							"GnomeRecentViewBonobo",
							&gnome_recent_view_bonobo_info, 0);
		g_type_add_interface_static (gnome_recent_view_bonobo_type,
					     GNOME_TYPE_RECENT_VIEW,
					     &view_info);
	}

	return gnome_recent_view_bonobo_type;
}

