/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  mdi-child.c:  gLabels MDI child module
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

/*
 * This file is based on gedit-mdi-child.c from gedit2:
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi 
 * Copyright (C) 2002  Paolo Maggi 
 *
 */
#include <config.h>

#include "mdi-child.h"
#include "xml-label.h"
#include "label.h"
#include "view.h"
#include "marshal.h"
#include "debug.h"

struct _glMDIChildPrivate
{
	GtkWidget *tab_label;
};

enum {
	STATE_CHANGED,
	UNDO_REDO_STATE_CHANGED,
	LAST_SIGNAL
};

static void gl_mdi_child_class_init (glMDIChildClass *klass);
static void gl_mdi_child_init       (glMDIChild      *mdi);
static void gl_mdi_child_finalize   (GObject         *obj);
static void gl_mdi_child_destroy    (GtkObject       *obj);

static void       gl_mdi_child_real_state_changed (glMDIChild* child);

static GtkWidget *gl_mdi_child_create_view (BonoboMDIChild *child);

static void       gl_mdi_child_document_state_changed_handler (glLabel *label, 
							       glMDIChild* child);

static void       gl_mdi_child_document_can_undo_redo_handler (glLabel *label, 
							       gboolean can,
							       glMDIChild* child);

static gchar*     gl_mdi_child_get_config_string (BonoboMDIChild *child,
						  gpointer data);

static BonoboMDIChildClass *parent_class = NULL;
static guint mdi_child_signals[LAST_SIGNAL] = { 0 };


GType
gl_mdi_child_get_type (void)
{
	static GType mdi_child_type = 0;

	gl_debug (DEBUG_MDI, "START");

  	if (mdi_child_type == 0)
    	{
      		static const GTypeInfo our_info =
      		{
        		sizeof (glMDIChildClass),
        		NULL,		/* base_init */
        		NULL,		/* base_finalize */
        		(GClassInitFunc) gl_mdi_child_class_init,
        		NULL,           /* class_finalize */
        		NULL,           /* class_data */
        		sizeof (glMDIChild),
        		0,              /* n_preallocs */
        		(GInstanceInitFunc) gl_mdi_child_init
      		};

      		mdi_child_type = g_type_register_static (BONOBO_TYPE_MDI_CHILD,
                				    	  "glMDIChild",
                                       	 	    	  &our_info,
                                       		    	  0);
    	}

	gl_debug (DEBUG_MDI, "END");

	return mdi_child_type;
}

static void
gl_mdi_child_class_init (glMDIChildClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gl_debug (DEBUG_MDI, "START");

	parent_class = g_type_class_peek_parent (klass);

  	gobject_class->finalize = gl_mdi_child_finalize;

	klass->state_changed 		= gl_mdi_child_real_state_changed;
	klass->undo_redo_state_changed  = NULL;
  		
	mdi_child_signals [STATE_CHANGED] =
		g_signal_new ("state_changed",
			      G_OBJECT_CLASS_TYPE (gobject_class),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (glMDIChildClass, state_changed),
			      NULL, NULL,
			      gl_marshal_VOID__VOID,
			      G_TYPE_NONE, 
			      0);

	mdi_child_signals [UNDO_REDO_STATE_CHANGED] =
		g_signal_new ("undo_redo_state_changed",
			      G_OBJECT_CLASS_TYPE (gobject_class),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (glMDIChildClass, undo_redo_state_changed),
			      NULL, NULL,
			      gl_marshal_VOID__VOID,
			      G_TYPE_NONE, 
			      0);
                    
	BONOBO_MDI_CHILD_CLASS (klass)->create_view = 
		(BonoboMDIChildViewCreator)(gl_mdi_child_create_view);

	BONOBO_MDI_CHILD_CLASS (klass)->get_config_string = 
		(BonoboMDIChildConfigFunc)(gl_mdi_child_get_config_string);

	gl_debug (DEBUG_MDI, "END");
}

static void 
gl_mdi_child_init (glMDIChild  *child)
{
	gl_debug (DEBUG_MDI, "START");

	child->priv = g_new0 (glMDIChildPrivate, 1);

	child->priv->tab_label = NULL;

	gl_debug (DEBUG_MDI, "END");
}

static void 
gl_mdi_child_finalize (GObject *obj)
{
	glMDIChild *child;

	gl_debug (DEBUG_MDI, "START");

	g_return_if_fail (obj != NULL);
	
   	child = GL_MDI_CHILD (obj);

	g_return_if_fail (GL_IS_MDI_CHILD (child));
	g_return_if_fail (child->priv != NULL);

	if (child->label != NULL)
		g_object_unref (G_OBJECT (child->label));

	g_free (child->priv);

	G_OBJECT_CLASS (parent_class)->finalize (obj);

	gl_debug (DEBUG_MDI, "END");
}

static void gl_mdi_child_real_state_changed (glMDIChild* child)
{
	gchar* docname = NULL;
	gchar* tab_name = NULL;

	gl_debug (DEBUG_MDI, "START");

	g_return_if_fail (child != NULL);
	g_return_if_fail (child->label != NULL);

	docname = gl_label_get_short_name (child->label);
	g_return_if_fail (docname != NULL);
	
	if (gl_label_is_modified (child->label))
	{
		tab_name = g_strdup_printf ("%s*", docname);
	} 
	else 
	{
		tab_name = g_strdup_printf ("%s", docname);
	}
	
	g_free (docname);

	g_return_if_fail (tab_name != NULL);

	bonobo_mdi_child_set_name (BONOBO_MDI_CHILD (child), tab_name);

	g_free (tab_name);	

	gl_debug (DEBUG_MDI, "END");
}

static void 
gl_mdi_child_document_state_changed_handler (glLabel *label, glMDIChild* child)
{
	gl_debug (DEBUG_MDI, "");
	g_return_if_fail (child->label == label);

	g_signal_emit (G_OBJECT (child), mdi_child_signals [STATE_CHANGED], 0);
}

static void 
gl_mdi_child_document_can_undo_redo_handler (glLabel *label, gboolean can, 
		               			glMDIChild* child)
{
	gl_debug (DEBUG_MDI, "");
	g_return_if_fail (child->label == label);

	g_signal_emit (G_OBJECT (child), mdi_child_signals [UNDO_REDO_STATE_CHANGED], 0);
}


static void
gl_mdi_child_connect_signals (glMDIChild *child)
{
	gl_debug (DEBUG_MDI, "START");

	g_signal_connect (G_OBJECT (child->label), "name_changed",
			  G_CALLBACK (gl_mdi_child_document_state_changed_handler), 
			  child);
	g_signal_connect (G_OBJECT (child->label), "modified_changed",
			  G_CALLBACK (gl_mdi_child_document_state_changed_handler), 
			  child);
#if 0
	g_signal_connect (G_OBJECT (child->label), "can_undo",
			  G_CALLBACK (gl_mdi_child_document_can_undo_redo_handler), 
			  child);
	g_signal_connect (G_OBJECT (child->label), "can_redo",
			  G_CALLBACK (gl_mdi_child_document_can_undo_redo_handler), 
			  child);
#endif

	gl_debug (DEBUG_MDI, "END");
}

/**
 * gl_mdi_child_new:
 * 
 * Creates a new #glMDIChild object.
 *
 * Return value: a new #glMDIChild
 **/
glMDIChild*
gl_mdi_child_new (const gchar *tmplt_name,
		  gboolean rotate_flag)
{
	glMDIChild *child;
	gchar *doc_name;
	glTemplate *template;
	
	gl_debug (DEBUG_MDI, "START");

	child = GL_MDI_CHILD (g_object_new (GL_TYPE_MDI_CHILD, NULL));
  	g_return_val_if_fail (child != NULL, NULL);
	
	child->label = GL_LABEL (gl_label_new ());
	g_return_val_if_fail (child->label != NULL, NULL);
	
	template = gl_template_from_name (tmplt_name);
	gl_label_set_template (child->label, template);
	gl_template_free (&template);
	gl_label_set_rotate_flag (child->label, rotate_flag);
	gl_label_clear_modified (child->label);

	doc_name = gl_label_get_short_name (child->label);
	bonobo_mdi_child_set_name (BONOBO_MDI_CHILD (child), doc_name);
	g_free(doc_name);

	gl_mdi_child_connect_signals (child);
	
	gl_debug (DEBUG_MDI, "END");
	
	return child;
}

glMDIChild*
gl_mdi_child_new_with_uri (const gchar *uri, GError **error)
{
	glMDIChild *child;
	glLabel    *doc;
	glXMLLabelStatus status;
	
	gl_debug (DEBUG_MDI, "START");

	doc = gl_xml_label_open (uri, &status);

	if (doc == NULL)
	{
		return NULL;
	}

	child = GL_MDI_CHILD (g_object_new (GL_TYPE_MDI_CHILD, NULL));
  	g_return_val_if_fail (child != NULL, NULL);
	
	child->label = doc;
	g_return_val_if_fail (child->label != NULL, NULL);
	
	gl_mdi_child_real_state_changed (child);
	
	gl_mdi_child_connect_signals (child);

	gl_debug (DEBUG_MDI, "END");
	
	return child;
}
		
static GtkWidget *
gl_mdi_child_create_view (BonoboMDIChild *child)
{
	glView  *new_view;

	gl_debug (DEBUG_MDI, "START");

	g_return_val_if_fail (child != NULL, NULL);
	g_return_val_if_fail (GL_IS_MDI_CHILD (child), NULL);

	new_view = GL_VIEW(gl_view_new (GL_MDI_CHILD (child)->label));

	gtk_widget_show_all (GTK_WIDGET (new_view));

	gl_debug (DEBUG_MDI, "END");

	return GTK_WIDGET (new_view);
}

static gchar* 
gl_mdi_child_get_config_string (BonoboMDIChild *child, gpointer data)
{
	glMDIChild *c;
	
	gl_debug (DEBUG_MDI, "");

	g_return_val_if_fail (child != NULL, NULL);
	g_return_val_if_fail (GL_IS_MDI_CHILD (child), NULL);

	c = GL_MDI_CHILD (child);
	
	return gl_label_get_filename (c->label);
}
