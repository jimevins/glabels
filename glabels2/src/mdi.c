/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  mdi.c:  gLabels MDI module
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
 * This file is based on gedit-mdi.c from gedit2:
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi 
 * Copyright (C) 2002  Paolo Maggi 
 *
 */
#include <config.h>

#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnomevfs/gnome-vfs.h>

#include <string.h>

#include "mdi.h"
#include "mdi-child.h"
#include "glabels.h"
#include "ui.h"
#include "prefs.h"
#include "file.h"
#include "view.h"
#include "debug.h"
#include "alert.h"

#include <bonobo/bonobo-ui-util.h>
#include <bonobo/bonobo-control.h>

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

#define DEFAULT_WINDOW_WIDTH  500
#define DEFAULT_WINDOW_HEIGHT 375

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glMDIPrivate
{
	gint untitled_number;
};


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static BonoboMDIClass *parent_class = NULL;


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gl_mdi_class_init  (glMDIClass *klass);
static void gl_mdi_init        (glMDI      *mdi);
static void gl_mdi_finalize    (GObject    *object);

static void gl_mdi_app_created_cb   (BonoboMDI *mdi, BonoboWindow   *win);

static gint gl_mdi_add_child_cb     (BonoboMDI *mdi, BonoboMDIChild *child);
static gint gl_mdi_add_view_cb      (BonoboMDI *mdi, GtkWidget      *view);
static gint gl_mdi_remove_child_cb  (BonoboMDI *mdi, BonoboMDIChild *child);
static gint gl_mdi_remove_view_cb   (BonoboMDI *mdi, GtkWidget      *view);

static void gl_mdi_view_changed_cb  (BonoboMDI *mdi, GtkWidget      *old_view);
static void gl_mdi_child_changed_cb (BonoboMDI *mdi, BonoboMDIChild *old_child);
static void gl_mdi_child_state_changed_cb (glMDIChild *child);

static void gl_mdi_set_active_window_undo_redo_verbs_sensitivity (BonoboMDI *mdi);
static void gl_mdi_set_active_window_selection_verbs_sensitivity (BonoboMDI *mdi);



/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_mdi_get_type (void)
{
	static GType mdi_type = 0;

  	if (mdi_type == 0)
    	{
      		static const GTypeInfo our_info =
      		{
        		sizeof (glMDIClass),
        		NULL,		/* base_init */
        		NULL,		/* base_finalize */
        		(GClassInitFunc) gl_mdi_class_init,
        		NULL,           /* class_finalize */
        		NULL,           /* class_data */
        		sizeof (glMDI),
        		0,              /* n_preallocs */
        		(GInstanceInitFunc) gl_mdi_init
      		};

      		mdi_type = g_type_register_static (BONOBO_TYPE_MDI,
                				    "glMDI",
                                       	 	    &our_info,
                                       		    0);
    	}

	return mdi_type;
}

static void
gl_mdi_class_init (glMDIClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	gl_debug (DEBUG_MDI, "START");

  	parent_class = g_type_class_peek_parent (klass);

  	object_class->finalize = gl_mdi_finalize;

	gl_debug (DEBUG_MDI, "END");
}

static void 
gl_mdi_init (glMDI  *mdi)
{
	gl_debug (DEBUG_MDI, "START");

	bonobo_mdi_construct (BONOBO_MDI (mdi), "glabels", "gLabels",
			      DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
	
	mdi->priv = g_new0 (glMDIPrivate, 1);

	mdi->priv->untitled_number = 0;	

	bonobo_mdi_set_ui_template_file (BONOBO_MDI (mdi),
					 GLABELS_UI_DIR "glabels-ui.xml",
					 gl_ui_verbs);
	
	bonobo_mdi_set_child_list_path (BONOBO_MDI (mdi), "/menu/Documents/");

	/* Connect signals */
	g_signal_connect (G_OBJECT (mdi), "top_window_created",
			  G_CALLBACK (gl_mdi_app_created_cb), NULL);
	
	g_signal_connect (G_OBJECT (mdi), "add_child",
			  G_CALLBACK (gl_mdi_add_child_cb), NULL);
	g_signal_connect (G_OBJECT (mdi), "add_view",
			  G_CALLBACK (gl_mdi_add_view_cb), NULL);
	
	g_signal_connect (G_OBJECT (mdi), "remove_child",
			  G_CALLBACK (gl_mdi_remove_child_cb), NULL);
	g_signal_connect (G_OBJECT (mdi), "remove_view",
			  G_CALLBACK (gl_mdi_remove_view_cb), NULL);

	g_signal_connect (G_OBJECT (mdi), "child_changed",
			  G_CALLBACK (gl_mdi_child_changed_cb), NULL);
	g_signal_connect (G_OBJECT (mdi), "view_changed",
			  G_CALLBACK (gl_mdi_view_changed_cb), NULL);

	g_signal_connect (G_OBJECT (mdi), "all_windows_destroyed",
			  G_CALLBACK (gl_file_exit), NULL);

	gl_debug (DEBUG_MDI, "END");
}

static void
gl_mdi_finalize (GObject *object)
{
	glMDI *mdi;

	gl_debug (DEBUG_MDI, "START");

	g_return_if_fail (object != NULL);
	
   	mdi = GL_MDI (object);

	g_return_if_fail (GL_IS_MDI (mdi));
	g_return_if_fail (mdi->priv != NULL);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	g_free (mdi->priv);

	gl_debug (DEBUG_MDI, "END");
}


/*****************************************************************************/
/* NEW mdi object.                                                           */
/*****************************************************************************/
glMDI*
gl_mdi_new (void)
{
	glMDI *mdi;

	gl_debug (DEBUG_MDI, "START");

	mdi = GL_MDI (g_object_new (GL_TYPE_MDI, NULL));
  	g_return_val_if_fail (mdi != NULL, NULL);
	
	gl_debug (DEBUG_MDI, "END");
	return mdi;
}

/*---------------------------------------------------------------------------*/
/* App created callback.                                                     */
/*---------------------------------------------------------------------------*/
static void
gl_mdi_app_created_cb (BonoboMDI    *mdi,
		       BonoboWindow *win)
{
	GtkWidget         *widget;
	BonoboControl     *control;
	BonoboUIComponent *ui_component;

	gl_debug (DEBUG_MDI, "START");
	
	ui_component = bonobo_mdi_get_ui_component_from_window (win);
	g_return_if_fail (ui_component != NULL);

	gl_ui_init (ui_component, win);

	gl_debug (DEBUG_MDI, "END");
}

/*---------------------------------------------------------------------------*/
/* Child state changed callback.                                             */
/*---------------------------------------------------------------------------*/
static void 
gl_mdi_child_state_changed_cb (glMDIChild *child)
{
	gl_debug (DEBUG_MDI, "START");

	if (bonobo_mdi_get_active_child (BONOBO_MDI (glabels_mdi)) != BONOBO_MDI_CHILD (child))
		return;
	
	gl_mdi_set_active_window_title (BONOBO_MDI (glabels_mdi));
	gl_mdi_set_active_window_verbs_sensitivity (BONOBO_MDI (glabels_mdi));

	gl_debug (DEBUG_MDI, "END");
}

/*---------------------------------------------------------------------------*/
/* Child undo/redo state changed callback.                                   */
/*---------------------------------------------------------------------------*/
static void 
gl_mdi_child_undo_redo_state_changed_cb (glMDIChild *child)
{
	gl_debug (DEBUG_MDI, "START");

	if (bonobo_mdi_get_active_child (BONOBO_MDI (glabels_mdi)) != BONOBO_MDI_CHILD (child))
		return;
	
	gl_mdi_set_active_window_undo_redo_verbs_sensitivity (BONOBO_MDI (glabels_mdi));
	gl_debug (DEBUG_MDI, "END");
}

/*---------------------------------------------------------------------------*/
/* Add child callback.                                                       */
/*---------------------------------------------------------------------------*/
static gint 
gl_mdi_add_child_cb (BonoboMDI      *mdi,
		     BonoboMDIChild *child)
{
	gl_debug (DEBUG_MDI, "START");

	g_signal_connect (G_OBJECT (child), "state_changed",
			  G_CALLBACK (gl_mdi_child_state_changed_cb), 
			  NULL);
	g_signal_connect (G_OBJECT (child), "undo_redo_state_changed",
			  G_CALLBACK (gl_mdi_child_undo_redo_state_changed_cb), 
			  NULL);

	gl_debug (DEBUG_MDI, "END");
	return TRUE;
}

/*---------------------------------------------------------------------------*/
/* View selection state changed callback.                                    */
/*---------------------------------------------------------------------------*/
static void 
gl_mdi_view_selection_state_changed_cb (glView *view)
{
	gl_debug (DEBUG_MDI, "START");

	if (bonobo_mdi_get_active_view (BONOBO_MDI (glabels_mdi)) != GTK_WIDGET (view))
		return;
	
	gl_mdi_set_active_window_selection_verbs_sensitivity (BONOBO_MDI (glabels_mdi));
	gl_debug (DEBUG_MDI, "END");
}

/*---------------------------------------------------------------------------*/
/* Add view callback.                                                        */
/*---------------------------------------------------------------------------*/
static gint 
gl_mdi_add_view_cb (BonoboMDI *mdi,
		    GtkWidget *view)
{
	gl_debug (DEBUG_MDI, "START");

	g_signal_connect (G_OBJECT (view), "selection_changed",
			  G_CALLBACK (gl_mdi_view_selection_state_changed_cb), 
			  NULL);

	gl_debug (DEBUG_MDI, "END");
	return TRUE;
}

/*---------------------------------------------------------------------------*/
/* Remove child callback.                                                    */
/*---------------------------------------------------------------------------*/
static gint 
gl_mdi_remove_child_cb (BonoboMDI      *mdi,
			BonoboMDIChild *child)
{
	glLabel *doc;
	gboolean close = TRUE;
	
	gl_debug (DEBUG_MDI, "START");

	g_return_val_if_fail (child != NULL, FALSE);
	g_return_val_if_fail (GL_MDI_CHILD (child)->label != NULL, FALSE);

	doc = GL_MDI_CHILD (child)->label;

	if (gl_label_is_modified (doc))
	{
		GtkWidget *msgbox, *w;
		gchar *fname = NULL, *msg = NULL;
		gint ret;
		gboolean exiting;

		w = GTK_WIDGET (g_list_nth_data (bonobo_mdi_child_get_views (child), 0));
			
		if(w != NULL)
			bonobo_mdi_set_active_view (mdi, w);

		fname = gl_label_get_short_name (doc);

		msg = g_strdup_printf (_("Save changes to document \"%s\" before closing?"),
					fname);

		msgbox = gl_alert_dialog_new (GTK_WINDOW (bonobo_mdi_get_active_window (mdi)),
				GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_WARNING,
				GTK_BUTTONS_NONE,
				msg,
				_("Your changes will be lost if you don't save them."));

		gtk_dialog_add_button (GTK_DIALOG (msgbox),
				       _("Close without saving"),
				       GTK_RESPONSE_NO);

		if (glabels_close_x_button_pressed)
			exiting = FALSE;
		else if (glabels_exit_button_pressed)
			exiting = TRUE;
		else
		{
			/* Delete event generated */
			if (g_list_length (bonobo_mdi_get_windows (BONOBO_MDI (glabels_mdi))) == 1)
				exiting = TRUE;
			else
				exiting = FALSE;
		}

		
		gtk_dialog_add_button (GTK_DIALOG (msgbox),
				       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);

		gtk_dialog_add_button (GTK_DIALOG (msgbox),
				       GTK_STOCK_SAVE, GTK_RESPONSE_YES);

		gtk_dialog_set_default_response	(GTK_DIALOG (msgbox), GTK_RESPONSE_YES);

		gtk_window_set_resizable (GTK_WINDOW (msgbox), FALSE);

		ret = gtk_dialog_run (GTK_DIALOG (msgbox));
		
		gtk_widget_destroy (msgbox);

		g_free (fname);
		g_free (msg);
		
		switch (ret)
		{
			case GTK_RESPONSE_YES:
				close = gl_file_save (GL_MDI_CHILD (child));
				break;
			case GTK_RESPONSE_NO:
				close = TRUE;
				break;
			default:
				close = FALSE;
		}

		gl_debug (DEBUG_MDI, "CLOSE: %s", close ? "TRUE" : "FALSE");
	}
	
	gl_debug (DEBUG_MDI, "END");
	return close;
}

/*---------------------------------------------------------------------------*/
/* Remove view callback.                                                     */
/*---------------------------------------------------------------------------*/
static gint 
gl_mdi_remove_view_cb (BonoboMDI *mdi,
		       GtkWidget *view)
{
	gl_debug (DEBUG_MDI, "");
	gl_debug (DEBUG_MDI, "END");

	return TRUE;
}

/*****************************************************************************/
/* Set title of active window.                                               */
/*****************************************************************************/
void 
gl_mdi_set_active_window_title (BonoboMDI *mdi)
{
	BonoboMDIChild *active_child = NULL;
	glLabel        *doc          = NULL;
	gchar          *docname      = NULL;
	gchar          *title        = NULL;
	
	gl_debug (DEBUG_MDI, "START");

	
	active_child = bonobo_mdi_get_active_child (mdi);
	if (active_child == NULL)
		return;

	doc = GL_MDI_CHILD (active_child)->label;
	g_return_if_fail (doc != NULL);
	
	/* Set active window title */
	docname = gl_label_get_short_name (doc);
	g_return_if_fail (docname != NULL);

	if (gl_label_is_modified (doc))
	{
		title = g_strdup_printf ("%s %s - glabels",
					 docname, _("(modified)"));
	} 
	else 
	{
		title = g_strdup_printf ("%s - glabels", docname);

	}

	gtk_window_set_title (GTK_WINDOW (bonobo_mdi_get_active_window (mdi)), title);
	
	g_free (docname);
	g_free (title);

	gl_debug (DEBUG_MDI, "END");
}

/*---------------------------------------------------------------------------*/
/* Child changed callback.                                                   */
/*---------------------------------------------------------------------------*/
static 
void gl_mdi_child_changed_cb (BonoboMDI      *mdi,
			      BonoboMDIChild *old_child)
{
	gl_debug (DEBUG_MDI, "START");

	gl_mdi_set_active_window_title (mdi);	

	gl_debug (DEBUG_MDI, "END");
}

/*---------------------------------------------------------------------------*/
/* View changed callback.                                                    */
/*---------------------------------------------------------------------------*/
static 
void gl_mdi_view_changed_cb (BonoboMDI *mdi,
			     GtkWidget *old_view)
{
	BonoboWindow *win;
	GtkWidget    *status;
	GtkWidget    *active_view;
	
	gl_debug (DEBUG_MDI, "START");

	gl_mdi_set_active_window_verbs_sensitivity (mdi);

	active_view = bonobo_mdi_get_active_view (mdi);
		
	win = bonobo_mdi_get_active_window (mdi);
	g_return_if_fail (win != NULL);

	gl_debug (DEBUG_MDI, "END");
}

/*****************************************************************************/
/* Set sensitivity of verbs in active window.                                */
/*****************************************************************************/
void 
gl_mdi_set_active_window_verbs_sensitivity (BonoboMDI *mdi)
{
	BonoboWindow      *active_window = NULL;
	BonoboMDIChild    *active_child  = NULL;
	glView            *view          = NULL;
	BonoboUIComponent *ui_component;
	
	gl_debug (DEBUG_MDI, "START");
	
	active_window = bonobo_mdi_get_active_window (mdi);

	if (active_window == NULL)
		return;
	
	ui_component = bonobo_mdi_get_ui_component_from_window (active_window);
	g_return_if_fail (ui_component != NULL);
	
	active_child = bonobo_mdi_get_active_child (mdi);

	if (active_child == NULL)
	{
		gl_ui_update_nodoc (ui_component);
		return;
	}

	view = GL_VIEW (bonobo_mdi_get_active_view (mdi));

	gl_ui_update_all (ui_component, view);

	gl_debug (DEBUG_MDI, "END");
}


/*****************************************************************************/
/* Set sensitivity of undo/redo verbs in active window.                      */
/*****************************************************************************/
static void 
gl_mdi_set_active_window_undo_redo_verbs_sensitivity (BonoboMDI *mdi)
{
	BonoboWindow      *active_window = NULL;
	BonoboMDIChild    *active_child  = NULL;
	glLabel           *doc           = NULL;
	BonoboUIComponent *ui_component;
	
	gl_debug (DEBUG_MDI, "START");
	
	active_window = bonobo_mdi_get_active_window (mdi);
	g_return_if_fail (active_window != NULL);
	
	ui_component = bonobo_mdi_get_ui_component_from_window (active_window);
	g_return_if_fail (ui_component != NULL);
	
	active_child = bonobo_mdi_get_active_child (mdi);
	doc = GL_MDI_CHILD (active_child)->label;
	g_return_if_fail (doc != NULL);

	gl_ui_update_undo_redo_verbs (ui_component, doc);

	gl_debug (DEBUG_MDI, "END");
}

/*****************************************************************************/
/* Set sensitivity of selection verbs in active window.                      */
/*****************************************************************************/
static void 
gl_mdi_set_active_window_selection_verbs_sensitivity (BonoboMDI *mdi)
{
	BonoboWindow      *active_window = NULL;
	glView            *view          = NULL;
	BonoboUIComponent *ui_component;
	
	gl_debug (DEBUG_MDI, "START");
	
	active_window = bonobo_mdi_get_active_window (mdi);
	g_return_if_fail (active_window != NULL);
	
	ui_component = bonobo_mdi_get_ui_component_from_window (active_window);
	g_return_if_fail (ui_component != NULL);
	
	view = GL_VIEW (bonobo_mdi_get_active_view (mdi));
	g_return_if_fail (view != NULL);

	gl_ui_update_selection_verbs (ui_component, view);

	gl_debug (DEBUG_MDI, "END");
}

