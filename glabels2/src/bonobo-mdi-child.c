/*
 * bonobo-mdi-child.c - implementation of a BonoboMDI object
 *
 * Copyright (C) 2001-2002 Free Software Foundation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 *
 * Author: Paolo Maggi 
 */

#include "bonobo-mdi-child.h"

struct _BonoboMDIChildPrivate
{
	GObject		*parent;

	gchar 		*name;
	GList 		*views;
};

enum {
	NAME_CHANGED,
	LAST_SIGNAL
};

static void       bonobo_mdi_child_class_init       (BonoboMDIChildClass *klass);
static void       bonobo_mdi_child_instance_init    (BonoboMDIChild *);
static void       bonobo_mdi_child_finalize         (GObject *);

static GtkWidget *bonobo_mdi_child_set_label        (BonoboMDIChild *, GtkWidget *, gpointer);
static GtkWidget *bonobo_mdi_child_create_view      (BonoboMDIChild *);

static void bonobo_mdi_child_real_name_changed (BonoboMDIChild *child, gchar* old_name);

static GObjectClass *parent_class = NULL;
static guint mdi_child_signals [LAST_SIGNAL] = { 0 };

GType
bonobo_mdi_child_get_type (void)
{
	static GType bonobo_mdi_child_type = 0;

  	if (bonobo_mdi_child_type == 0)
    	{
      		static const GTypeInfo our_info =
      		{
        		sizeof (BonoboMDIChildClass),
        		NULL,		/* base_init */
        		NULL,		/* base_finalize */
        		(GClassInitFunc) bonobo_mdi_child_class_init,
        		NULL,           /* class_finalize */
        		NULL,           /* class_data */
        		sizeof (BonoboMDIChild),
        		0,              /* n_preallocs */
        		(GInstanceInitFunc) bonobo_mdi_child_instance_init
      		};

      		bonobo_mdi_child_type = g_type_register_static (G_TYPE_OBJECT,
                				    "BonoboMDIChild",
                                       	 	    &our_info,
                                       		    0);
    	}

	return bonobo_mdi_child_type;
}

static void 
bonobo_mdi_child_class_init (BonoboMDIChildClass *klass)
{
	GObjectClass *gobject_class;

	parent_class = g_type_class_peek_parent (klass);

	gobject_class = (GObjectClass*)klass;
  
	gobject_class->finalize = bonobo_mdi_child_finalize;
  
	klass->create_view = NULL;
	klass->get_config_string = NULL;
	klass->set_label = bonobo_mdi_child_set_label;

	klass->name_changed = bonobo_mdi_child_real_name_changed;

  	mdi_child_signals[NAME_CHANGED] =
		g_signal_new ("name_changed",
			      G_OBJECT_CLASS_TYPE (gobject_class),
                    	      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                    	      G_STRUCT_OFFSET (BonoboMDIChildClass, name_changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__STRING,
			      G_TYPE_NONE, 
			      1, 
			      G_TYPE_STRING);
}

static void
bonobo_mdi_child_instance_init (BonoboMDIChild *mdi_child)
{
	g_return_if_fail (BONOBO_IS_MDI_CHILD (mdi_child));

	mdi_child->priv = g_new0 (BonoboMDIChildPrivate, 1);
	
	mdi_child->priv->name = NULL;
	mdi_child->priv->parent = NULL;
	mdi_child->priv->views = NULL;
}


/* the default set_label function: returns a GtkLabel with child->priv->name
 * if you provide your own, it should return a new widget if its old_label
 * parameter is NULL and modify and return the old widget otherwise. it
 * should (obviously) NOT call the parent class handler!
 */
static GtkWidget *
bonobo_mdi_child_set_label (BonoboMDIChild *child, GtkWidget *old_label, gpointer data)
{
	g_return_val_if_fail (BONOBO_IS_MDI_CHILD (child), NULL);
	g_return_val_if_fail (child->priv != NULL, NULL);

	if (old_label != NULL) 
	{
		gtk_label_set_text (GTK_LABEL (old_label), child->priv->name);
		return old_label;
	}
	else 
	{
		GtkWidget *label;

		label = gtk_label_new (child->priv->name);
		gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
		
		return label;
	}
}

static void 
bonobo_mdi_child_finalize (GObject *obj)
{
	BonoboMDIChild *mdi_child;

	g_return_if_fail (BONOBO_IS_MDI_CHILD (obj));

	mdi_child = BONOBO_MDI_CHILD (obj);
	g_return_if_fail (mdi_child->priv != NULL);

	while (mdi_child->priv->views)
		bonobo_mdi_child_remove_view (mdi_child, 
				GTK_WIDGET (mdi_child->priv->views->data));

	if (mdi_child->priv->name != NULL)
		g_free (mdi_child->priv->name);

	if (mdi_child->priv != NULL)
		g_free (mdi_child->priv);
	
	if (G_OBJECT_CLASS (parent_class)->finalize)
		(* G_OBJECT_CLASS (parent_class)->finalize)(obj);
}

/**
 * bonobo_mdi_child_add_view:
 * @mdi_child: A pointer to a BonoboMDIChild object.
 * 
 * Description:
 * Creates a new view of a child (a GtkWidget) adds it to the list
 * of the views and returns a pointer to it. Virtual function
 * that has to be specified for classes derived from BonoboMDIChild
 * is used to create the new view.
 * 
 * Return value:
 * A pointer to the new view.
 **/
GtkWidget *
bonobo_mdi_child_add_view (BonoboMDIChild *mdi_child)
{
	GtkWidget *view = NULL;

	g_return_val_if_fail (BONOBO_IS_MDI_CHILD (mdi_child), NULL);
	g_return_val_if_fail (mdi_child->priv != NULL, NULL);

	view = bonobo_mdi_child_create_view (mdi_child);

	if (view) {
		mdi_child->priv->views = g_list_append (mdi_child->priv->views, view);

		g_object_set_data (G_OBJECT (view), "BonoboMDIChild", mdi_child);
	}

	return view;
}

/**
 * bonobo_mdi_child_remove_view:
 * @mdi_child: A pointer to a BonoboMDIChild object.
 * @view: View to be removed.
 * 
 * Description:
 * Removes view @view from the list of @mdi_child's views and
 * unrefs it.
 **/
void 
bonobo_mdi_child_remove_view (BonoboMDIChild *mdi_child, GtkWidget *view)
{
	g_return_if_fail (BONOBO_IS_MDI_CHILD (mdi_child));
	g_return_if_fail (mdi_child->priv != NULL);
	g_return_if_fail (GTK_IS_WIDGET (view));

	mdi_child->priv->views = g_list_remove (mdi_child->priv->views, view);

	g_object_unref (G_OBJECT (view));	
}

/**
 * bonobo_mdi_child_set_name:
 * @mdi_child: A pointer to a BonoboMDIChild object.
 * @name: String containing the new name for the child.
 * 
 * Description:
 * Changes name of @mdi_child to @name. @name is duplicated and stored
 * in @mdi_child. If @mdi_child has already been added to BonoboMDI,
 * it also takes care of updating it.
 **/
void 
bonobo_mdi_child_set_name (BonoboMDIChild *mdi_child, const gchar *name)
{
	gchar *old_name;
       	
	g_return_if_fail (BONOBO_IS_MDI_CHILD (mdi_child));
	g_return_if_fail (mdi_child->priv != NULL);

	old_name = mdi_child->priv->name;

	if (old_name == name)
		return;
	
	mdi_child->priv->name = (gchar *)g_strdup (name);

	g_signal_emit (G_OBJECT (mdi_child),
		       mdi_child_signals [NAME_CHANGED], 0,
		       old_name);

	if (old_name)
		g_free (old_name);
}

static GtkWidget *
bonobo_mdi_child_create_view (BonoboMDIChild *child)
{
	g_return_val_if_fail (BONOBO_IS_MDI_CHILD (child), NULL);

	if (BONOBO_MDI_CHILD_GET_CLASS (child)->create_view)
		return BONOBO_MDI_CHILD_GET_CLASS (child)->create_view (child, NULL);

	return NULL;
}

static void 
bonobo_mdi_child_real_name_changed (BonoboMDIChild *child, gchar* old_name)
{
	g_return_if_fail (BONOBO_IS_MDI_CHILD (child));

	return;
}

gchar * 
bonobo_mdi_child_get_name (const BonoboMDIChild *mdi_child)
{
	g_return_val_if_fail (BONOBO_IS_MDI_CHILD (mdi_child), NULL);
	g_return_val_if_fail (mdi_child->priv != NULL, NULL);

	if (mdi_child->priv->name)
		return g_strdup (mdi_child->priv->name);
	else
		return NULL;
}

GList *	      
bonobo_mdi_child_get_views (const BonoboMDIChild *mdi_child)
{
	g_return_val_if_fail (BONOBO_IS_MDI_CHILD (mdi_child), NULL);
	g_return_val_if_fail (mdi_child->priv != NULL, NULL);

	return mdi_child->priv->views;
}

GObject *
bonobo_mdi_child_get_parent (const BonoboMDIChild *mdi_child)
{
	g_return_val_if_fail (BONOBO_IS_MDI_CHILD (mdi_child), NULL);
	g_return_val_if_fail (mdi_child->priv != NULL, NULL);

	return G_OBJECT (mdi_child->priv->parent);
	
}

void
bonobo_mdi_child_set_parent (BonoboMDIChild *mdi_child, GObject *parent)
{
	g_return_if_fail (BONOBO_IS_MDI_CHILD (mdi_child));
	g_return_if_fail (mdi_child->priv != NULL);

	mdi_child->priv->parent = parent;
}
