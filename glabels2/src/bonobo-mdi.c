/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-mdi.c - implementation of a BonoboMDI object
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

/*
 * Modified by the gedit Team, 2001-2002. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "bonobo-mdi.h"

#include <bonobo/bonobo-i18n.h>
#include <marshal.h>
#include <bonobo/bonobo-dock-layout.h>
#include <bonobo/bonobo-ui-util.h>

#include "debug.h"

#include <string.h>
#include <time.h>
#include <unistd.h>

#define BONOBO_MDI_KEY 		"BonoboMDI"
#define BONOBO_MDI_CHILD_KEY 	"BonoboMDIChild"
#define UI_COMPONENT_KEY 	"UIComponent"
#define BOOK_KEY		"Book"
#define WINDOW_INFO_KEY		"BonoboMDIWindowInfo"

static void            bonobo_mdi_class_init     (BonoboMDIClass  *);
static void            bonobo_mdi_instance_init  (BonoboMDI *);
static void            bonobo_mdi_finalize       (GObject *);
static void            child_list_menu_create    (BonoboMDI *, BonoboWindow *);

static void            child_list_activated_cb   (BonoboUIComponent *uic, gpointer user_data, 
						  const gchar* verbname);

void                   child_list_menu_remove_item(BonoboMDI *, BonoboMDIChild *);
void                   child_list_menu_add_item   (BonoboMDI *, BonoboMDIChild *);

static void            app_create               (BonoboMDI *, gchar *, const char *);
static void            app_clone                (BonoboMDI *, BonoboWindow *, const char *);
static void            app_destroy              (BonoboWindow *, BonoboMDI *);
static void            app_set_view             (BonoboMDI *, BonoboWindow *, GtkWidget *);

static gboolean        app_close_book           (BonoboWindow *, GdkEventAny *, BonoboMDI *);

static GtkWidget      *book_create             	(BonoboMDI *);
static void            book_switch_page         (GtkNotebook *, GtkNotebookPage *,
						 gint, BonoboMDI *);
static gboolean        book_motion              (GtkWidget *widget, GdkEventMotion *e,
						 gpointer data);
static gboolean        book_button_press        (GtkWidget *widget, GdkEventButton *e,
						 gpointer data);
static gboolean        book_button_release      (GtkWidget *widget, GdkEventButton *e,
						 gpointer data);
static void            book_add_view            (GtkNotebook *, GtkWidget *);
static void            set_page_by_widget       (GtkNotebook *, GtkWidget *);

static gboolean        toplevel_focus           (BonoboWindow *, GdkEventFocus *, BonoboMDI *);

static void            set_active_view          (BonoboMDI *, GtkWidget *);

/* convenience functions that call child's "virtual" functions */
static GtkWidget      *child_set_label         (BonoboMDIChild *, GtkWidget *);

static void 	       child_name_changed 	(BonoboMDIChild *mdi_child, 
						 gchar* old_name, 
						 BonoboMDI *mdi);
static void            bonobo_mdi_update_child 	(BonoboMDI *mdi, BonoboMDIChild *child);

static gchar* 	       escape_underscores 	(const gchar* text);

static GtkWidget*      get_book_from_window 	(BonoboWindow *window);

static GdkCursor *drag_cursor = NULL;

enum {
	ADD_CHILD,
	REMOVE_CHILD,
	ADD_VIEW,
	REMOVE_VIEW,
	CHILD_CHANGED,
	VIEW_CHANGED,
	TOP_WINDOW_CREATED,
	TOP_WINDOW_DESTROY,
	ALL_WINDOWS_DESTROYED,
	LAST_SIGNAL
};


struct _BonoboMDIPrivate
{
	GtkPositionType tab_pos;

	guint 		 signal_id;
	gint 		 in_drag : 1;

	gchar 		*mdi_name; 
	gchar		*title;

	gchar 		*ui_xml;
	gchar	        *ui_file_name;
	BonoboUIVerb    *verbs;

	/* Probably only one of these would do, but... redundancy rules ;) */
	BonoboMDIChild 	*active_child;
	GtkWidget 	*active_view;  
	BonoboWindow 	*active_window;

	GList 		*windows;     	/* toplevel windows -  BonoboWindow widgets */
	GList 		*children;    	/* children - BonoboMDIChild objects*/

	GSList 		*registered; 	/* see comment for bonobo_mdi_(un)register() functions 
					 * below for an explanation. */
	
    	/* Paths for insertion of mdi-child list menu via */
	gchar 		*child_list_path;

	gint 		 default_window_height;
	gint		 default_window_width;
};

typedef gboolean   (*BonoboMDISignal1) (GObject *, gpointer, gpointer);
typedef void       (*BonoboMDISignal2) (GObject *, gpointer, gpointer);

static GObjectClass *parent_class = NULL;
static guint mdi_signals [LAST_SIGNAL] = { 0 };

GType
bonobo_mdi_get_type (void)
{
	static GType bonobo_mdi_type = 0;

  	if (bonobo_mdi_type == 0)
    	{
      		static const GTypeInfo our_info =
      		{
        		sizeof (BonoboMDIClass),
        		NULL,		/* base_init */
        		NULL,		/* base_finalize */
        		(GClassInitFunc) bonobo_mdi_class_init,
        		NULL,           /* class_finalize */
        		NULL,           /* class_data */
        		sizeof (BonoboMDI),
        		0,              /* n_preallocs */
        		(GInstanceInitFunc) bonobo_mdi_instance_init
      		};

      		bonobo_mdi_type = g_type_register_static (G_TYPE_OBJECT,
                				    "BonoboMDI",
                                       	 	    &our_info,
                                       		    0);
    	}

	return bonobo_mdi_type;
}

static void 
bonobo_mdi_class_init (BonoboMDIClass *class)
{
	GObjectClass *object_class;
	
	object_class = (GObjectClass*) class;
	
	object_class->finalize = bonobo_mdi_finalize;

	mdi_signals[ADD_CHILD] = 
		g_signal_new ("add_child",
 			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (BonoboMDIClass, add_child),
			      NULL, NULL,
			      gl_marshal_BOOLEAN__OBJECT,
			      G_TYPE_BOOLEAN, 
			      1, 
			      BONOBO_TYPE_MDI_CHILD);
	
	mdi_signals[REMOVE_CHILD] = 
		g_signal_new ("remove_child",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (BonoboMDIClass, remove_child),
			      NULL, NULL,
			      gl_marshal_BOOLEAN__OBJECT,
			      G_TYPE_BOOLEAN, 
			      1, 
			      BONOBO_TYPE_MDI_CHILD);
	
	mdi_signals[ADD_VIEW] = 
		g_signal_new ("add_view",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (BonoboMDIClass, add_view),
			      NULL, NULL,			     
			      gl_marshal_BOOLEAN__OBJECT,
			      G_TYPE_BOOLEAN, 
			      1, 
			      GTK_TYPE_WIDGET);
	
	mdi_signals[REMOVE_VIEW] = 
		g_signal_new ("remove_view",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (BonoboMDIClass, remove_view),
			      NULL, NULL,
			      gl_marshal_BOOLEAN__OBJECT,
			      G_TYPE_BOOLEAN, 
			      1, 
			      GTK_TYPE_WIDGET);

	mdi_signals[CHILD_CHANGED] = 
		g_signal_new ("child_changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (BonoboMDIClass, child_changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE, 
			      1, 
			      BONOBO_TYPE_MDI_CHILD);

	mdi_signals[VIEW_CHANGED] = 
		g_signal_new ("view_changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET(BonoboMDIClass, view_changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE, 
			      1, 
			      GTK_TYPE_WIDGET);

	mdi_signals[TOP_WINDOW_CREATED] = 
		g_signal_new ("top_window_created",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (BonoboMDIClass, top_window_created),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE, 
			      1, 
			      BONOBO_TYPE_WINDOW);

	mdi_signals[TOP_WINDOW_DESTROY] = 
		g_signal_new ("top_window_destroy",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (BonoboMDIClass, top_window_destroy),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE, 
			      1, 
			      BONOBO_TYPE_WINDOW);

	mdi_signals[ALL_WINDOWS_DESTROYED] = 
		g_signal_new ("all_windows_destroyed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (BonoboMDIClass, all_windows_destroyed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 
			      0);

	
	class->add_child 		= NULL;
	class->remove_child 		= NULL;
	class->add_view 		= NULL;
	class->remove_view 		= NULL;
	class->child_changed 		= NULL;
	class->view_changed 		= NULL;
	class->top_window_created 	= NULL;

	parent_class = gtk_type_class (G_TYPE_OBJECT);
}

static void 
bonobo_mdi_finalize (GObject *object)
{
    	BonoboMDI *mdi;

	gl_debug (DEBUG_MDI, "");

	g_return_if_fail (BONOBO_IS_MDI (object));
	
	mdi = BONOBO_MDI (object);
	g_return_if_fail (mdi->priv != NULL);
	
	bonobo_mdi_remove_all (mdi, TRUE);

	if (mdi->priv->child_list_path != NULL)
		g_free (mdi->priv->child_list_path);
	
	if (mdi->priv->mdi_name != NULL)
		g_free (mdi->priv->mdi_name);
	
	if (mdi->priv->title != NULL)
		g_free (mdi->priv->title);

	if (mdi->priv->ui_xml != NULL)
		g_free (mdi->priv->ui_xml);
	
	if (mdi->priv->ui_file_name != NULL)
		g_free (mdi->priv->ui_file_name);
	
	g_free (mdi->priv);
	mdi->priv = NULL;

	if (G_OBJECT_CLASS (parent_class)->finalize)
		(* G_OBJECT_CLASS (parent_class)->finalize)(object);

	gl_debug (DEBUG_MDI, "END");
}


static void 
bonobo_mdi_instance_init (BonoboMDI *mdi)
{
	gl_debug (DEBUG_MDI, "");

	g_return_if_fail (BONOBO_IS_MDI (mdi));

	mdi->priv = g_new0 (BonoboMDIPrivate, 1);
	g_return_if_fail (mdi->priv != NULL);
		
	mdi->priv->tab_pos 		= GTK_POS_TOP;
		
	mdi->priv->signal_id 		= 0;
	mdi->priv->in_drag 		= FALSE;

	mdi->priv->children 		= NULL;
	mdi->priv->windows 		= NULL;
	mdi->priv->registered 		= NULL;
	
	mdi->priv->active_child 	= NULL;
	mdi->priv->active_window 	= NULL;
	mdi->priv->active_view 		= NULL;

	mdi->priv->ui_xml		= NULL;
	mdi->priv->ui_file_name		= NULL;
	mdi->priv->verbs		= NULL;	
	
	mdi->priv->child_list_path 	= NULL;

	gl_debug (DEBUG_MDI, "END");
}


/**
 * bonobo_mdi_new:
 * @mdi_name: Application name as used in filenames and paths.
 * @title: Title of the application windows.
 * 
 * Description:
 * Creates a new MDI object. @mdi_name and @title are used for
 * MDI's calling bonobo_window_new (). 
 * 
 * Return value:
 * A pointer to a new BonoboMDI object.
 **/
GObject*
bonobo_mdi_new (const gchar *mdi_name, const gchar *title,
		gint default_window_width, gint default_window_height) 
{
	BonoboMDI *mdi;

	gl_debug (DEBUG_MDI, "");
	
	mdi = g_object_new (BONOBO_TYPE_MDI, NULL);
  
	mdi->priv->mdi_name = g_strdup (mdi_name);
	mdi->priv->title    = g_strdup (title);

	mdi->priv->default_window_width = default_window_width;
	mdi->priv->default_window_height = default_window_height;

	gl_debug (DEBUG_MDI, "END");

	return G_OBJECT (mdi);
}

static GtkWidget *
child_set_label (BonoboMDIChild *child, GtkWidget *label)
{
	GtkWidget *w;
	w = BONOBO_MDI_CHILD_GET_CLASS (child)->set_label (child, label, NULL);
	
	return w;
}

static void 
set_page_by_widget (GtkNotebook *book, GtkWidget *view)
{
	gint i;
	
	i = gtk_notebook_page_num (book, view);
	
	if (gtk_notebook_get_current_page (book) != i)
		gtk_notebook_set_current_page (book, i);
}

static void 
child_list_activated_cb (BonoboUIComponent *uic, gpointer user_data, const gchar* verbname)
{
	BonoboMDI* mdi;
	BonoboMDIChild *child = BONOBO_MDI_CHILD (user_data);
	g_return_if_fail (child != NULL);

	gl_debug (DEBUG_MDI, "");

	mdi = BONOBO_MDI (g_object_get_data (G_OBJECT (child), BONOBO_MDI_KEY));
	g_return_if_fail (mdi != NULL);
		
	if (child && (child != mdi->priv->active_child)) 
	{
		GList *views = bonobo_mdi_child_get_views (child);
		
		if (views)
			bonobo_mdi_set_active_view (mdi, views->data);
		else
			bonobo_mdi_add_view (mdi, child);
	}

	gl_debug (DEBUG_MDI, "END");
}

static gchar* 
escape_underscores (const gchar* text)
{
	GString *str;
	gint length;
	const gchar *p;
 	const gchar *end;

  	g_return_val_if_fail (text != NULL, NULL);

    	length = strlen (text);

	str = g_string_new ("");

  	p = text;
  	end = text + length;

  	while (p != end)
    	{
      		const gchar *next;
      		next = g_utf8_next_char (p);

		switch (*p)
        	{
       			case '_':
          			g_string_append (str, "__");
          			break;
        		default:
          			g_string_append_len (str, p, next - p);
          			break;
        	}

      		p = next;
    	}

	return g_string_free (str, FALSE);
}

static void 
child_list_menu_create (BonoboMDI *mdi, BonoboWindow *win)
{
	GList *child;
	BonoboUIComponent *ui_component;

	if (mdi->priv->child_list_path == NULL)
		return;
	
	ui_component = BONOBO_UI_COMPONENT (
			g_object_get_data (G_OBJECT (win), UI_COMPONENT_KEY));
			
	child = mdi->priv->children;
	
	bonobo_ui_component_freeze (ui_component, NULL);
	
	while (child) 
	{
		gchar *xml = NULL;
		gchar *cmd = NULL;
		gchar *verb_name = NULL;
		gchar *tip;
		gchar *escaped_name;
		gchar *safe_name;
		gchar *child_name = bonobo_mdi_child_get_name (BONOBO_MDI_CHILD (child->data));

		safe_name = g_markup_escape_text (child_name, strlen (child_name));
		g_return_if_fail (safe_name != NULL);

		escaped_name = escape_underscores (safe_name);
		g_return_if_fail (escaped_name != NULL);

		tip =  g_strdup_printf (_("Activate %s"), safe_name);
		verb_name = g_strdup_printf ("Child_%p", child->data);
		xml = g_strdup_printf ("<menuitem name=\"%s\" verb=\"%s\""
				" _label=\"%s\"/>", verb_name, verb_name, escaped_name);
		cmd =  g_strdup_printf ("<cmd name = \"%s\" _label=\"%s\""
				" _tip=\"%s\"/>", verb_name, escaped_name, tip);

		g_free (tip);
		g_free (child_name);
		g_free (safe_name);
		g_free (escaped_name);

		bonobo_ui_component_set_translate (ui_component, mdi->priv->child_list_path, xml, NULL);
		bonobo_ui_component_set_translate (ui_component, "/commands/", cmd, NULL);
		bonobo_ui_component_add_verb (ui_component, verb_name, child_list_activated_cb, child->data); 
		
		g_free (xml); 
		g_free (cmd);
		g_free (verb_name);

		child = g_list_next (child);
	}

	bonobo_ui_component_thaw (ui_component, NULL);
}


void 
child_list_menu_remove_item (BonoboMDI *mdi, BonoboMDIChild *child)
{
	GList *win_node;
	gchar *path, *cmd, *verb_name;
	
	gl_debug (DEBUG_MDI, "");

	if(mdi->priv->child_list_path == NULL)
		return;
	
	win_node = mdi->priv->windows;

	verb_name = g_strdup_printf ("Child_%p", child);
	path = g_strdup_printf ("%s%s", mdi->priv->child_list_path, verb_name);
	cmd = g_strdup_printf ("/commands/%s", verb_name);

	while (win_node) 
	{
		BonoboUIComponent *ui_component;
		
		ui_component = BONOBO_UI_COMPONENT (
			g_object_get_data (G_OBJECT (win_node->data), UI_COMPONENT_KEY));
		
		bonobo_ui_component_remove_verb (ui_component, verb_name);
		bonobo_ui_component_rm (ui_component, path, NULL);
		bonobo_ui_component_rm (ui_component, cmd, NULL);
		win_node = g_list_next (win_node);
	}

	g_free (path);
	g_free (cmd);
	g_free (verb_name);

	gl_debug (DEBUG_MDI, "END");
}


void 
child_list_menu_add_item (BonoboMDI *mdi, BonoboMDIChild *child)
{
	GList *win_node;
	gchar *child_name, *escaped_name, *safe_name;
	gchar* xml, *cmd, *verb_name, *tip;
	int accel_num;
	
	if(mdi->priv->child_list_path == NULL)
		return;

	accel_num = g_list_length (mdi->priv->children);
	
	win_node = mdi->priv->windows;

	child_name = bonobo_mdi_child_get_name (child);

	safe_name = g_markup_escape_text (child_name, strlen (child_name));
	g_return_if_fail (safe_name != NULL);

	escaped_name = escape_underscores (safe_name);
	g_return_if_fail (escaped_name != NULL);

	verb_name = g_strdup_printf ("Child_%p", child);
	
	tip = g_strdup_printf (_("Activate %s"), safe_name);
	xml = g_strdup_printf ("<menuitem name=\"%s\" verb=\"%s\""
				" _label=\"%s\"/>", verb_name, verb_name, escaped_name);

	if (accel_num > 9)
		cmd =  g_strdup_printf ("<cmd name = \"%s\" _label=\"%s\""
				" _tip=\"%s\"/>", verb_name, escaped_name, tip);
	else
		cmd =  g_strdup_printf ("<cmd name = \"%s\" _label=\"%s\""
			" _tip=\"%s\" accel=\"*Alt*%d\"/>", verb_name,
			escaped_name, tip, accel_num);

	
	while (win_node) 
	{
		BonoboUIComponent *ui_component;
		
		ui_component = BONOBO_UI_COMPONENT (
			g_object_get_data (G_OBJECT (win_node->data), UI_COMPONENT_KEY));
		

		bonobo_ui_component_set_translate (ui_component, mdi->priv->child_list_path, xml, NULL);
		bonobo_ui_component_set_translate (ui_component, "/commands/", cmd, NULL);
		bonobo_ui_component_add_verb (ui_component, verb_name, child_list_activated_cb, child); 

		win_node = g_list_next (win_node);
	}
	
	g_free (tip);
	g_free (escaped_name);
	g_free (child_name);
	g_free (xml);
	g_free (cmd);
	g_free (verb_name);
	g_free (safe_name);
}

static gboolean 
book_motion (GtkWidget *widget, GdkEventMotion *e, gpointer data)
{
	BonoboMDI *mdi;

	mdi = BONOBO_MDI (data);

	if (!drag_cursor)
		drag_cursor = gdk_cursor_new (GDK_HAND2);

	if (e->window == GTK_NOTEBOOK (widget)->event_window) 
	{
		mdi->priv->in_drag = TRUE;
		gtk_grab_add (widget);
		gdk_pointer_grab (widget->window, FALSE,
						 GDK_POINTER_MOTION_MASK |
						 GDK_BUTTON_RELEASE_MASK, NULL,
						 drag_cursor, GDK_CURRENT_TIME);
		if (mdi->priv->signal_id) 
		{
			g_signal_handler_disconnect (G_OBJECT (widget), 
						     mdi->priv->signal_id);
			mdi->priv->signal_id = 0;
		}
	}

	return FALSE;
}

static gboolean 
book_button_press (GtkWidget *widget, GdkEventButton *e, gpointer data)
{
	BonoboMDI *mdi;

	mdi = BONOBO_MDI (data);

	if ((e->button == 1) && (e->window == GTK_NOTEBOOK (widget)->event_window))
		mdi->priv->signal_id = g_signal_connect (
				G_OBJECT (widget), 
				"motion_notify_event",
				G_CALLBACK (book_motion), 
				mdi);

	return FALSE;
}

static gboolean
book_button_release (GtkWidget *widget, GdkEventButton *e, gpointer data)
{
	gint x = e->x_root, y = e->y_root;
	BonoboMDI *mdi;

	mdi = BONOBO_MDI(data);
	
	if (mdi->priv->signal_id) 
	{
		g_signal_handler_disconnect (G_OBJECT (widget), mdi->priv->signal_id);
		mdi->priv->signal_id = 0;
	}

	if ((e->button == 1) && mdi->priv->in_drag) 
	{	
		GdkWindow *window;
		GList *child;
		BonoboWindow *win;
		GtkWidget *view, *new_book;
		GtkNotebook *old_book = GTK_NOTEBOOK (widget);

		mdi->priv->in_drag = FALSE;
		gdk_pointer_ungrab (GDK_CURRENT_TIME);
		gtk_grab_remove (widget);

		window = gdk_window_at_pointer (&x, &y);
		if (window)
			window = gdk_window_get_toplevel (window);

		child = mdi->priv->windows;
		
		while (child) 
		{
			if (window == GTK_WIDGET (child->data)->window) 
			{
				int cur_page;

				/* page was dragged to another notebook */

				old_book = GTK_NOTEBOOK(widget);
				new_book = get_book_from_window (BONOBO_WINDOW (child->data));

				if (old_book == (GtkNotebook *) new_book) 
					/* page has been dropped on the source notebook */
					return FALSE;

				cur_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (old_book));
	
				if (cur_page >= 0) 
				{
					view = gtk_notebook_get_nth_page (GTK_NOTEBOOK (old_book), cur_page);
					gtk_container_remove (GTK_CONTAINER(old_book), view);

					book_add_view (GTK_NOTEBOOK (new_book), view);

					win = bonobo_mdi_get_window_from_view (view);
					gdk_window_raise (GTK_WIDGET(win)->window);

					mdi->priv->active_window = win;

					cur_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (old_book));

					if (cur_page < 0) 
					{
						mdi->priv->active_window = win;
						win = BONOBO_WINDOW (gtk_widget_get_toplevel (
									GTK_WIDGET (old_book)));
						mdi->priv->windows = g_list_remove(mdi->priv->windows, win);
						gtk_widget_destroy (GTK_WIDGET (win));
					}					
										
					g_signal_emit (G_OBJECT (mdi), 
						       mdi_signals [CHILD_CHANGED], 
						       0,
						       NULL);

					g_signal_emit (G_OBJECT (mdi), 
						       mdi_signals [VIEW_CHANGED], 
						       0,
						       view);
				}

				return FALSE;
			}
				
			child = child->next;
		}

		if (g_list_length (old_book->children) == 1)
			return FALSE;

		/* create a new toplevel */
		if (old_book->cur_page) 
		{
			gint width, height;
			int cur_page = gtk_notebook_get_current_page (old_book);

			view = gtk_notebook_get_nth_page (old_book, cur_page);
				
			win = bonobo_mdi_get_window_from_view (view);

			gtk_window_get_size (GTK_WINDOW (win), &width, &height);
		
			gtk_container_remove (GTK_CONTAINER (old_book), view);
			
			app_clone (mdi, win, NULL);
				
			new_book = book_create (mdi);
	
			book_add_view (GTK_NOTEBOOK (new_book), view);

			gtk_window_set_position (GTK_WINDOW (mdi->priv->active_window), GTK_WIN_POS_MOUSE);
	
			gtk_window_set_default_size (GTK_WINDOW (mdi->priv->active_window), width, height);

			if (!GTK_WIDGET_VISIBLE (mdi->priv->active_window))
				gtk_widget_show (GTK_WIDGET (mdi->priv->active_window));
		}
	}

	return FALSE;
}

static GtkWidget *
book_create (BonoboMDI *mdi)
{
	GtkWidget *us;
	GtkWidget *vbox;

	gl_debug (DEBUG_MDI, "");

	g_return_val_if_fail (mdi->priv->active_window != NULL, NULL);
	
	vbox = gtk_vbox_new (FALSE, 0);
	us = gtk_notebook_new ();

	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (us), mdi->priv->tab_pos);

	gtk_box_pack_start (GTK_BOX (vbox), us, TRUE, TRUE, 0);	

	gtk_widget_show_all (vbox);

	bonobo_window_set_contents (mdi->priv->active_window, vbox);
	g_object_set_data (G_OBJECT (mdi->priv->active_window), BOOK_KEY, us);
	
	gtk_widget_add_events (us, GDK_BUTTON1_MOTION_MASK);

	g_signal_connect (G_OBJECT (us), "switch_page",
			  G_CALLBACK (book_switch_page), mdi);

	g_signal_connect (G_OBJECT (us), "button_press_event",
			  G_CALLBACK (book_button_press), mdi);
	g_signal_connect (G_OBJECT (us), "button_release_event",
			  G_CALLBACK (book_button_release), mdi);

	gtk_notebook_set_scrollable (GTK_NOTEBOOK (us), TRUE);
	
	gl_debug (DEBUG_MDI, "END");

	return us;
}

static void 
book_add_view (GtkNotebook *book, GtkWidget *view)
{
	BonoboMDIChild *child;
	GtkWidget *title;

	gl_debug (DEBUG_MDI, "");

	child = bonobo_mdi_get_child_from_view (view);

	title = child_set_label (child, NULL);

	gtk_notebook_append_page (book, view, title);

	if (g_list_length (book->children) > 1)
		set_page_by_widget (book, view);

	gl_debug (DEBUG_MDI, "END");
}

static void 
book_switch_page (GtkNotebook *book, GtkNotebookPage *pg, gint page_num, BonoboMDI *mdi)
{
	BonoboWindow *win;
	GtkWidget *page;
	
	gl_debug (DEBUG_MDI, "");

	win = BONOBO_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (book)));

	page = gtk_notebook_get_nth_page (book, page_num);

	if (page != NULL) 
	{
		if (page != mdi->priv->active_view)
			app_set_view (mdi, win, page);
	}
	else 
		app_set_view (mdi, win, NULL);  

	gl_debug (DEBUG_MDI, "END");

}

static GtkWidget*
get_book_from_window (BonoboWindow *window)
{
	gpointer *book;

	g_return_val_if_fail (window != NULL, NULL);

	book = g_object_get_data (G_OBJECT (window), BOOK_KEY);

	return (book != NULL) ? GTK_WIDGET (book) : NULL;
}

static gboolean
toplevel_focus (BonoboWindow *win, GdkEventFocus *event, BonoboMDI *mdi)
{
	GtkWidget *contents;
	
	gl_debug (DEBUG_MDI, "");

	/* updates active_view and active_child when a new toplevel receives focus */
	g_return_val_if_fail (BONOBO_IS_WINDOW (win), FALSE);
	
	mdi->priv->active_window = win;
	
	contents = get_book_from_window (win);
	
	if (GTK_NOTEBOOK (contents)->cur_page) 
	{
		int cur_page;
		GtkWidget *child;
			
	       	cur_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (contents));
		child = gtk_notebook_get_nth_page (GTK_NOTEBOOK (contents), cur_page);
		set_active_view (mdi, child);
	} 
	else 
		set_active_view (mdi, NULL);
	
	gl_debug (DEBUG_MDI, "END");

	return FALSE;
}

static gboolean 
app_configure_event_handler (GtkWidget *widget, GdkEventConfigure *event)
{
	BonoboMDIWindowInfo *window_info;

	g_return_val_if_fail (BONOBO_IS_WINDOW (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);
	
	window_info = (BonoboMDIWindowInfo*) 
			g_object_get_data (G_OBJECT (widget), WINDOW_INFO_KEY);
	g_return_val_if_fail (window_info != NULL, FALSE);

	window_info->width = event->width;
	window_info->height = event->height;

	return FALSE;
}

static gboolean 
app_window_state_event_handler (GtkWidget *widget, GdkEventWindowState *event)
{
	BonoboMDIWindowInfo *window_info;

	g_return_val_if_fail (BONOBO_IS_WINDOW (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);
	
	window_info = (BonoboMDIWindowInfo*) 
			g_object_get_data (G_OBJECT (widget), WINDOW_INFO_KEY);
	g_return_val_if_fail (window_info != NULL, FALSE);
	
	window_info->state = event->new_window_state;
	
	return FALSE;
}


static void 
app_clone (BonoboMDI *mdi, BonoboWindow *win, const char *window_role)
{
	gl_debug (DEBUG_MDI, "");
	
	app_create (mdi, NULL, window_role);

	if (win != NULL)
	{
		const BonoboMDIWindowInfo *window_info = bonobo_mdi_get_window_info (win);
		g_return_if_fail (window_info != NULL);

		if ((window_info->state & GDK_WINDOW_STATE_MAXIMIZED) != 0)
			gtk_window_maximize (GTK_WINDOW (mdi->priv->active_window));
		else
		{
			gtk_window_set_default_size (GTK_WINDOW (mdi->priv->active_window), 
					     window_info->width,
					     window_info->height);
			
			gtk_window_unmaximize (GTK_WINDOW (mdi->priv->active_window));
		}

		if ((window_info->state & GDK_WINDOW_STATE_STICKY ) != 0)
			gtk_window_stick (GTK_WINDOW (mdi->priv->active_window));
		else
			gtk_window_unstick (GTK_WINDOW (mdi->priv->active_window));
	}

	gl_debug (DEBUG_MDI, "END");
}


static gboolean
app_close_book (BonoboWindow *win, GdkEventAny *event, BonoboMDI *mdi)
{
	BonoboMDIChild *child;
	GtkWidget *view;
	gint handler_ret = TRUE;
	
	gl_debug (DEBUG_MDI, "");

	if (g_list_length (mdi->priv->windows) == 1) 
	{		
		if (!bonobo_mdi_remove_all (mdi, FALSE))
		{
			gl_debug (DEBUG_MDI, "END1");

			return TRUE;
		}

		mdi->priv->windows = g_list_remove (mdi->priv->windows, win);
		gtk_widget_destroy (GTK_WIDGET (win));
		
		/* only emit al_windows_destroyed signal if there are no non-MDI windows registered
		   with it. */
		if (mdi->priv->registered == NULL)
			g_signal_emit (G_OBJECT (mdi), mdi_signals [ALL_WINDOWS_DESTROYED], 0);
	}
	else 
	{
		GList *children = gtk_container_get_children (
					GTK_CONTAINER (get_book_from_window (win)));
		GList *li;

		if (children == NULL) 
		{
			mdi->priv->windows = g_list_remove (mdi->priv->windows, win);
			gtk_widget_destroy (GTK_WIDGET (win));

			gl_debug (DEBUG_MDI, "END2");

			return FALSE;
		}

		/* first check if all the children in this notebook can be removed */
		for (li = children; li != NULL; li = li->next) 
		{
			GList *node;
			view = li->data;

			child = bonobo_mdi_get_child_from_view (view);
			
			node = bonobo_mdi_child_get_views (child);
			
			while (node) 
			{
				if (bonobo_mdi_get_window_from_view (node->data) != win)
					break;
				
				node = node->next;
			}
			
			if (node == NULL) 
			{   
				/* all the views reside in this BonoboWindow */
				g_signal_emit (G_OBJECT(mdi), 
					      mdi_signals [REMOVE_CHILD],
					      0,
					      child, 
					      &handler_ret);

				if (handler_ret == FALSE) 
				{
					g_list_free (children);

					gl_debug (DEBUG_MDI, "END3");

					return TRUE;
				}
			}
		}
		
		/* now actually remove all children/views! */
		for (li = children; li != NULL; li = li->next) 
		{
			view = li->data;

			child = bonobo_mdi_get_child_from_view (view);
			
			/* if this is the last view, remove the child */
			if (g_list_length (bonobo_mdi_child_get_views (child)) == 1)
				bonobo_mdi_remove_child (mdi, child, TRUE);
			else
				bonobo_mdi_remove_view (mdi, view, TRUE);
		}

		g_list_free (children);
	}

	gl_debug (DEBUG_MDI, "END");
	
	return FALSE;
}

static void 
app_set_view (BonoboMDI *mdi, BonoboWindow *win, GtkWidget *view)
{
	gl_debug (DEBUG_MDI, "");

	gtk_window_set_title (GTK_WINDOW (win), mdi->priv->title);
	
	set_active_view (mdi, view);

	gl_debug (DEBUG_MDI, "END");
}

static void 
app_destroy (BonoboWindow *win, BonoboMDI *mdi)
{
	gl_debug (DEBUG_MDI, "");
	
	if (mdi->priv->active_window == win)
		mdi->priv->active_window = 
			(mdi->priv->windows != NULL) ? BONOBO_WINDOW (mdi->priv->windows->data) : NULL;

	g_signal_emit (G_OBJECT (mdi), mdi_signals [TOP_WINDOW_DESTROY], 0, win);

	gl_debug (DEBUG_MDI, "END");
}

/* Generates a unique string for a window role.
 *
 * Taken from EOG.
 */
static char *
gen_role (void)
{
        char *ret;
	static char *hostname;
	time_t t;
	static int serial;

	t = time (NULL);

	if (!hostname) {
		static char buffer [512];

		if ((gethostname (buffer, sizeof (buffer) - 1) == 0) &&
		    (buffer [0] != 0))
			hostname = buffer;
		else
			hostname = "localhost";
	}

	ret = g_strdup_printf ("bonobo-mdi-window-%d-%d-%d-%ld-%d@%s",
			       getpid (),
			       getgid (),
			       getppid (),
			       (long) t,
			       serial++,
			       hostname);

	return ret;
}

static void 
app_create (BonoboMDI *mdi, gchar *layout_string, const char *window_role)
{
	GtkWidget *window;
	BonoboWindow *bw;
	gchar* config_path;
	BonoboUIContainer *ui_container = NULL;
  	BonoboUIComponent *ui_component = NULL;
	BonoboMDIWindowInfo *window_info = NULL;

	gl_debug (DEBUG_MDI, "");

	window = bonobo_window_new (mdi->priv->mdi_name, mdi->priv->title);
	g_return_if_fail (window != NULL);

	gtk_window_set_default_size (GTK_WINDOW (window),
				     mdi->priv->default_window_width,
				     mdi->priv->default_window_height);

	if (window_role)
		gtk_window_set_role (GTK_WINDOW (window), window_role);
	else {
		char *role;

		role = gen_role ();
		gtk_window_set_role (GTK_WINDOW (window), role);
		g_free (role);
	}
	
	bw = BONOBO_WINDOW (window);

	mdi->priv->windows = g_list_append (mdi->priv->windows, window);

	g_signal_connect (G_OBJECT (window), "delete_event", 
			  G_CALLBACK (app_close_book), mdi);
	g_signal_connect (G_OBJECT (window), "focus_in_event",
			  G_CALLBACK (toplevel_focus), mdi);
	g_signal_connect (G_OBJECT (window), "destroy",
			  G_CALLBACK (app_destroy), mdi);
	g_signal_connect (G_OBJECT (window), "configure_event",
	                  G_CALLBACK (app_configure_event_handler), NULL);
	g_signal_connect (G_OBJECT (window), "window_state_event",
	                  G_CALLBACK (app_window_state_event_handler), NULL);

	/* Create Container: */
 	ui_container = bonobo_window_get_ui_container (bw);

	config_path = g_strdup_printf ("/%s/UIConfig/kvps", mdi->priv->mdi_name);

  	bonobo_ui_engine_config_set_path (bonobo_window_get_ui_engine (bw),
                                     config_path);
	g_free (config_path);

	/* Create a UI component with which to communicate with the window */
	ui_component = bonobo_ui_component_new_default ();

	/* Associate the BonoboUIComponent with the container */
  	bonobo_ui_component_set_container (
        	ui_component, BONOBO_OBJREF (ui_container), NULL);

	/* set up UI */
	if (mdi->priv->ui_xml != NULL)
	{
		bonobo_ui_component_set_translate (ui_component, 
				"/", mdi->priv->ui_xml, NULL);
	}
	else
		if (mdi->priv->ui_file_name)
		{
			bonobo_ui_util_set_ui (ui_component, "", mdi->priv->ui_file_name,
				       mdi->priv->mdi_name, NULL);
		}

	if (mdi->priv->verbs)
		bonobo_ui_component_add_verb_list_with_data (ui_component, 
				mdi->priv->verbs, mdi);
	
	mdi->priv->active_window = bw;
	mdi->priv->active_child = NULL;
	mdi->priv->active_view = NULL;

	window_info = g_new0 (BonoboMDIWindowInfo, 1);
			
	g_object_set_data (G_OBJECT (bw), UI_COMPONENT_KEY, ui_component);
	g_object_set_data_full (G_OBJECT (bw), WINDOW_INFO_KEY, window_info, g_free);

	g_signal_emit (G_OBJECT (mdi), mdi_signals [TOP_WINDOW_CREATED], 0, window);

	child_list_menu_create (mdi, bw);

	gl_debug (DEBUG_MDI, "END");
}

static void 
set_active_view (BonoboMDI *mdi, GtkWidget *view)
{
	BonoboMDIChild *old_child;
	GtkWidget *old_view;
		
	gl_debug (DEBUG_MDI, "");

	old_child = mdi->priv->active_child;
	old_view = mdi->priv->active_view;

	mdi->priv->active_view = view;

	if (!view) 
		mdi->priv->active_child = NULL;
	else
	{
		
		mdi->priv->active_child = bonobo_mdi_get_child_from_view (view);
		mdi->priv->active_window = bonobo_mdi_get_window_from_view (view);
	
		gtk_widget_grab_focus (GTK_WIDGET (view));
	}
	
	if (view == old_view)
	{
		gl_debug (DEBUG_MDI, "END1");

		return;
	}
	
	if (mdi->priv->active_child != old_child)
	{
		gl_debug (DEBUG_MDI, "Emit child_changed");

		g_signal_emit (G_OBJECT (mdi), mdi_signals [CHILD_CHANGED], 0, old_child);
	}

	gl_debug (DEBUG_MDI, "Emit view_changed");
	
	g_signal_emit (G_OBJECT (mdi), mdi_signals [VIEW_CHANGED], 0, old_view);

	gl_debug (DEBUG_MDI, "END2");
}

/**
 * bonobo_mdi_set_active_view:
 * @mdi: A pointer to an MDI object.
 * @view: A pointer to the view that is to become the active one.
 * 
 * Description:
 * Sets the active view to @view. It also raises the window containing it
 * and gives it focus.
 **/
void 
bonobo_mdi_set_active_view (BonoboMDI *mdi, GtkWidget *view)
{
	GtkWindow *window;
	
	gl_debug (DEBUG_MDI, "");

	g_return_if_fail (mdi != NULL);
	g_return_if_fail (BONOBO_IS_MDI (mdi));
	g_return_if_fail (view != NULL);
	g_return_if_fail (GTK_IS_WIDGET (view));
	
	set_page_by_widget (GTK_NOTEBOOK (view->parent), view);
	
	window = GTK_WINDOW (bonobo_mdi_get_window_from_view (view));
	
	gtk_window_present (window);

	set_active_view (mdi, view);

	gl_debug (DEBUG_MDI, "END");
}

/**
 * bonobo_mdi_add_view:
 * @mdi: A pointer to a BonoboMDI object.
 * @child: A pointer to a child.
 * 
 * Description:
 * Creates a new view of the child and adds it to the MDI. BonoboMDIChild
 * @child has to be added to the MDI with a call to bonobo_mdi_add_child
 * before its views are added to the MDI. 
 * An "add_view" signal is emitted to the MDI after the view has been
 * created, but before it is shown and added to the MDI, with a pointer to
 * the created view as its parameter. The view is added to the MDI only if
 * the signal handler (if it exists) returns %TRUE. If the handler returns
 * %FALSE, the created view is destroyed and not added to the MDI. 
 * 
 * Return value:
 * %TRUE if adding the view succeeded and %FALSE otherwise.
 **/
gboolean 
bonobo_mdi_add_view (BonoboMDI *mdi, BonoboMDIChild *child)
{
	GtkWidget *view;
	GtkWidget *book;
	gint ret = TRUE;
	
	gl_debug (DEBUG_MDI, "");

	g_return_val_if_fail (mdi != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_MDI (mdi), FALSE);
	g_return_val_if_fail (child != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_MDI_CHILD (child), FALSE);
	
	view = bonobo_mdi_child_add_view (child);

	g_return_val_if_fail (view != NULL, FALSE);

	g_signal_emit (G_OBJECT (mdi), mdi_signals [ADD_VIEW], 0, view, &ret);

	if (!ret) 
	{
		bonobo_mdi_child_remove_view (child, view);
		
		gl_debug (DEBUG_MDI, "END1");

		return FALSE;
	}

	if (mdi->priv->active_window == NULL) 
	{
		app_create (mdi, NULL, NULL);
		gtk_widget_show (GTK_WIDGET (mdi->priv->active_window));
	}

	
	if (!GTK_WIDGET_VISIBLE (view))
		gtk_widget_show (view);

	book = get_book_from_window (mdi->priv->active_window);
	
	if (book == NULL)
		book = book_create (mdi);
	
	book_add_view (GTK_NOTEBOOK (book), view);
	
	/* this reference will compensate the view's unrefing
	   when removed from its parent later, as we want it to
	   stay valid until removed from the child with a call
	   to bonobo_mdi_child_remove_view() */
	g_object_ref (G_OBJECT (view));
	gtk_object_sink (GTK_OBJECT (view));
	
	g_object_set_data (G_OBJECT (view), BONOBO_MDI_CHILD_KEY, child);
	
	gl_debug (DEBUG_MDI, "END2");

	return TRUE;
}

/**
 * bonobo_mdi_add_toplevel_view:
 * @mdi: A pointer to a BonoboMDI object.
 * @child: A pointer to a BonoboMDIChild object to be added to the MDI.
 * @window_role: X window role to use for the window, for session-management
 * purposes.  If this is %NULL, a unique role string will be automatically
 * generated.
 * 
 * Description:
 * Creates a new view of the child and adds it to the MDI; it behaves the
 * same way as bonobo_mdi_add_view in %BONOBO_MDI_MODAL and %BONOBO_MDI_TOPLEVEL
 * modes, but in %BONOBO_MDI_NOTEBOOK mode, the view is added in a new
 * toplevel window unless the active one has no views in it. 
 * 
 * Return value: 
 * %TRUE if adding the view succeeded and %FALSE otherwise.
 **/
gboolean
bonobo_mdi_add_toplevel_view (BonoboMDI *mdi, BonoboMDIChild *child, const char *window_role)
{
	GtkWidget *view;
	gint ret = TRUE;
	
	gl_debug (DEBUG_MDI, "");

	g_return_val_if_fail (mdi != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_MDI (mdi), FALSE);
	g_return_val_if_fail (child != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_MDI_CHILD (child), FALSE);
	
	view = bonobo_mdi_child_add_view (child);

	g_return_val_if_fail (view != NULL, FALSE);

	g_signal_emit (G_OBJECT (mdi), mdi_signals [ADD_VIEW], 0, view, &ret);

	if (ret == FALSE) 
	{
		bonobo_mdi_child_remove_view (child, view);

		gl_debug (DEBUG_MDI, "END1");

		return FALSE;
	}

	bonobo_mdi_open_toplevel (mdi, window_role);
	
	if (!GTK_WIDGET_VISIBLE (view))
		gtk_widget_show (view);

	book_add_view (GTK_NOTEBOOK (get_book_from_window (mdi->priv->active_window)), view);
	
	/* this reference will compensate the view's unrefing
	   when removed from its parent later, as we want it to
	   stay valid until removed from the child with a call
	   to bonobo_mdi_child_remove_view() */
	g_object_ref (G_OBJECT (view));	
	gtk_object_sink (GTK_OBJECT (view));
	
	g_object_set_data (G_OBJECT (view), BONOBO_MDI_CHILD_KEY, child);
	
	gl_debug (DEBUG_MDI, "END2");

	return TRUE;
}

/**
 * bonobo_mdi_remove_view:
 * @mdi: A pointer to a BonoboMDI object.
 * @view: View to remove.
 * @force: If TRUE, the "remove_view" signal is not emitted.
 * 
 * Description:
 * Removes a view from an MDI. 
 * A "remove_view" signal is emitted to the MDI before actually removing
 * view. The view is removed only if the signal handler (if it exists and
 * the @force is set to %FALSE) returns %TRUE. 
 * 
 * Return value: 
 * %TRUE if the view was removed and %FALSE otherwise.
 **/
gboolean
bonobo_mdi_remove_view (BonoboMDI *mdi, GtkWidget *view, gint force)
{
	GtkWidget *book;
	BonoboWindow *window;
	BonoboMDIChild *child;
	gint ret = TRUE;
	gint pn;
	gboolean was_active_window = FALSE;
		
	gl_debug (DEBUG_MDI, "");
	
	g_return_val_if_fail (mdi != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_MDI (mdi), FALSE);
	g_return_val_if_fail (view != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_WIDGET (view), FALSE);

	if (!force)
		g_signal_emit (G_OBJECT (mdi), mdi_signals [REMOVE_VIEW], 0, view, &ret);

	if (ret == FALSE)
	{
		gl_debug (DEBUG_MDI, "END1");

		return FALSE;
	}

	was_active_window = (view == mdi->priv->active_view);
	
	child = bonobo_mdi_get_child_from_view (view);
	window = bonobo_mdi_get_window_from_view (view);
	
	book = get_book_from_window (window);
	g_return_val_if_fail (book != NULL, TRUE);

	if (g_list_length (GTK_NOTEBOOK (book)->children) == 1)
		app_set_view (mdi, window, NULL);
		
	bonobo_mdi_child_remove_view (child, view);

	pn = gtk_notebook_page_num (GTK_NOTEBOOK (book), view);
	gtk_notebook_remove_page (GTK_NOTEBOOK (book), pn);

	if (GTK_NOTEBOOK (book)->cur_page == NULL) 
	{
		if ((g_list_length (mdi->priv->windows) > 1) || 
		    (mdi->priv->registered != NULL)) 
		{
			gl_debug (DEBUG_VIEW, "Destroy window");

			/* if this is NOT the last toplevel or a registered object
		   	exists, destroy the toplevel */
			mdi->priv->windows = g_list_remove (mdi->priv->windows, window);
			gtk_widget_destroy (GTK_WIDGET (window));
	
			if (mdi->priv->active_window && was_active_window)
				mdi->priv->active_view = bonobo_mdi_get_view_from_window (
									mdi, 
									mdi->priv->active_window);
		}
	}
	else
	{
		pn = gtk_notebook_get_current_page (GTK_NOTEBOOK (book));
		app_set_view (mdi, window, gtk_notebook_get_nth_page (GTK_NOTEBOOK (book), pn));
	}

		
	gl_debug (DEBUG_MDI, "END2");

	return TRUE;
}

static void 
child_name_changed (BonoboMDIChild *mdi_child, gchar* old_name, BonoboMDI *mdi)
{
	bonobo_mdi_update_child (mdi, mdi_child);
}

/**
 * bonobo_mdi_add_child:
 * @mdi: A pointer to a BonoboMDI object.
 * @child: A pointer to a BonoboMDIChild to add to the MDI.
 * 
 * Description:
 * Adds a new child to the MDI. No views are added: this has to be done with
 * a call to bonobo_mdi_add_view. 
 * First an "add_child" signal is emitted to the MDI with a pointer to the
 * child as its parameter. The child is added to the MDI only if the signal
 * handler (if it exists) returns %TRUE. If the handler returns %FALSE, the
 * child is not added to the MDI. 
 * 
 * Return value: 
 * %TRUE if the child was added successfully and %FALSE otherwise.
 **/
gint 
bonobo_mdi_add_child (BonoboMDI *mdi, BonoboMDIChild *child)
{
	gint ret = TRUE;

	gl_debug (DEBUG_MDI, "");

	g_return_val_if_fail (mdi != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_MDI (mdi), FALSE);
	g_return_val_if_fail (child != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_MDI_CHILD (child), FALSE);

	g_signal_emit (G_OBJECT (mdi), mdi_signals [ADD_CHILD], 0, child, &ret);

	if (ret == FALSE)
	{
		gl_debug (DEBUG_MDI, "END1");

		return FALSE;
	}

	bonobo_mdi_child_set_parent (child, G_OBJECT (mdi));

	mdi->priv->children = g_list_append (mdi->priv->children, child);

	g_signal_connect (G_OBJECT (child), "name_changed",
			  G_CALLBACK (child_name_changed), mdi);

	child_list_menu_add_item (mdi, child);

	g_object_set_data (G_OBJECT (child), BONOBO_MDI_KEY, mdi);

	gl_debug (DEBUG_MDI, "END2");

	return TRUE;
}

/**
 * bonobo_mdi_remove_child:
 * @mdi: A pointer to a BonoboMDI object.
 * @child: Child to remove.
 * @force: If TRUE, the "remove_child" signal is not emitted
 * 
 * Description:
 * Removes a child and all of its views from the MDI. 
 * A "remove_child" signal is emitted to the MDI with @child as its parameter
 * before actually removing the child. The child is removed only if the signal
 * handler (if it exists and the @force is set to %FALSE) returns %TRUE. 
 * 
 * Return value: 
 * %TRUE if the removal was successful and %FALSE otherwise.
 **/
gint 
bonobo_mdi_remove_child (BonoboMDI *mdi, BonoboMDIChild *child, gint force)
{
	gint ret = TRUE;
	GList *view_node;
	GtkWidget *view;

	gl_debug (DEBUG_MDI, "");
	
	g_return_val_if_fail (mdi != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_MDI (mdi), FALSE);
	g_return_val_if_fail (child != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_MDI_CHILD (child), FALSE);

	/* if force is set to TRUE, don't call the remove_child handler (ie there is no way for the
	   user to stop removal of the child) */

	if (!force)
		g_signal_emit (G_OBJECT (mdi), mdi_signals [REMOVE_CHILD], 0, child, &ret);

	if (ret == FALSE)
	{
		gl_debug (DEBUG_MDI, "END1");

		return FALSE;
	}

	view_node = bonobo_mdi_child_get_views (child);
	
	while (view_node) 
	{
		view = GTK_WIDGET (view_node->data);
		view_node = view_node->next;
		bonobo_mdi_remove_view (mdi, GTK_WIDGET (view), TRUE);
	}

	mdi->priv->children = g_list_remove (mdi->priv->children, child);

	child_list_menu_remove_item (mdi, child);

	if (child == mdi->priv->active_child)
		mdi->priv->active_child = NULL;

	bonobo_mdi_child_set_parent (child, NULL);

	g_signal_handlers_disconnect_by_func (G_OBJECT (child), G_CALLBACK (child_name_changed), mdi);

	g_object_unref (G_OBJECT (child));

	gl_debug (DEBUG_MDI, "END2");

	return TRUE;
}

/**
 * bonobo_mdi_remove_all:
 * @mdi: A pointer to a BonoboMDI object.
 * @force: If TRUE, the "remove_child" signal is not emitted
 * 
 * Description:
 * Removes all children and all views from the MDI. 
 * A "remove_child" signal is emitted to the MDI for each child before
 * actually trying to remove any. If signal handlers for all children (if
 * they exist and the @force is set to %FALSE) return %TRUE, all children
 * and their views are removed and none otherwise. 
 * 
 * Return value:
 * %TRUE if the removal was successful and %FALSE otherwise.
 **/
gint 
bonobo_mdi_remove_all (BonoboMDI *mdi, gint force)
{
	GList *child_node;
	gint handler_ret = TRUE;

	gl_debug (DEBUG_MDI, "");

	g_return_val_if_fail (mdi != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_MDI (mdi), FALSE);

	/* first check if removal of any child will be prevented by the
	   remove_child signal handler */
	if (!force) 
	{
		child_node = mdi->priv->children;
		while (child_node) 
		{
			g_signal_emit (G_OBJECT (mdi), 
				       mdi_signals [REMOVE_CHILD],
				       0,
				       child_node->data, 
				       &handler_ret);

			/* if any of the children may not be removed, none will be */
			if (handler_ret == FALSE)
			{
				gl_debug (DEBUG_MDI, "END1");

				return FALSE;
			}

			child_node = child_node->next;
		}
	}

	/* remove all the children with force arg set to true so that remove_child
	   handlers are not called again */
	while (mdi->priv->children)
		bonobo_mdi_remove_child (mdi, BONOBO_MDI_CHILD (mdi->priv->children->data), TRUE);

	gl_debug (DEBUG_MDI, "END2");

	return TRUE;
}

/**
 * bonobo_mdi_open_toplevel:
 * @mdi: A pointer to a BonoboMDI object.
 * @window_role: X window role to use for the window, for session-management
 * purposes.  If this is %NULL, a unique role string will be automatically
 * generated.
 * 
 * Description:
 * Opens a new toplevel window. This is usually used only for opening
 * the initial window on startup (just before calling gtkmain()) if no
 * windows were open because a session was restored or children were added
 * because of command line args).
 **/
void 
bonobo_mdi_open_toplevel (BonoboMDI *mdi, const char *window_role)
{
	gl_debug (DEBUG_MDI, "");

	g_return_if_fail (mdi != NULL);
	g_return_if_fail (BONOBO_IS_MDI (mdi));

	app_clone (mdi, mdi->priv->active_window, window_role);

	book_create (mdi);
		
	gtk_widget_show (GTK_WIDGET (mdi->priv->active_window));

	gl_debug (DEBUG_MDI, "END1");
}

/**
 * bonobo_mdi_update_child:
 * @mdi: A pointer to a BonoboMDI object.
 * @child: Child to update.
 * 
 * Description:
 * Updates all notebook labels of @child's views and their window titles
 * after its name changes. It is not required if bonobo_mdi_child_set_name()
 * is used for setting the child's name.
 **/
static void 
bonobo_mdi_update_child (BonoboMDI *mdi, BonoboMDIChild *child)
{
	GtkWidget *view, *title;
	GList *view_node;
	GList *win_node;
	gchar* child_name, *path, *path_cmd, *tip, *escaped_name;

	gl_debug (DEBUG_MDI, "");

	g_return_if_fail (mdi != NULL);
	g_return_if_fail (BONOBO_IS_MDI (mdi));
	g_return_if_fail (child != NULL);
	g_return_if_fail (BONOBO_IS_MDI_CHILD (child));

	view_node = bonobo_mdi_child_get_views (child);
	
	while (view_node) 
	{
		view = GTK_WIDGET (view_node->data);

		title = child_set_label (child, NULL);
		
		gtk_notebook_set_tab_label (GTK_NOTEBOOK (view->parent), view, title);
		
		view_node = g_list_next (view_node);
	}

	/* Update child list menus */	
	if(mdi->priv->child_list_path == NULL)
	{
		gl_debug (DEBUG_MDI, "END1");

		return;
	}
	
	win_node = mdi->priv->windows;

	child_name = bonobo_mdi_child_get_name (child);
	escaped_name = escape_underscores (child_name);
	path = g_strdup_printf ("%sChild_%p", mdi->priv->child_list_path, child);
	path_cmd =  g_strdup_printf ("/commands/Child_%p", child);
	tip = g_strdup_printf (_("Activate %s"), child_name);
			
	while (win_node) 
	{
		BonoboUIComponent *ui_component;
		
		ui_component = BONOBO_UI_COMPONENT (
			g_object_get_data (G_OBJECT (win_node->data), UI_COMPONENT_KEY));

		bonobo_ui_component_set_prop (ui_component, path, "label", escaped_name, NULL);
		bonobo_ui_component_set_prop (ui_component, path, "tip", tip, NULL);

		win_node = g_list_next (win_node);
	}

	g_free (escaped_name);
	g_free (path);
	g_free (path_cmd);
	g_free (tip);
	g_free (child_name);

	gl_debug (DEBUG_MDI, "END2");
}

/**
 * bonobo_mdi_find_child:
 * @mdi: A pointer to a BonoboMDI object.
 * @name: A string with a name of the child to find.
 * 
 * Description:
 * Finds a child named @name.
 * 
 * Return value: 
 * A pointer to the BonoboMDIChild object if the child was found and NULL
 * otherwise.
 **/
BonoboMDIChild *
bonobo_mdi_find_child (BonoboMDI *mdi, const gchar *name)
{
	GList *child_node;

	gl_debug (DEBUG_MDI, "");

	g_return_val_if_fail (mdi != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_MDI (mdi), NULL);

	child_node = mdi->priv->children;
	while (child_node) 
	{
		gchar* child_name = bonobo_mdi_child_get_name (BONOBO_MDI_CHILD (child_node->data));
		 
		if (strcmp (child_name, name) == 0)
		{
			g_free (child_name);	

			gl_debug (DEBUG_MDI, "END1");

			return BONOBO_MDI_CHILD (child_node->data);
		}
		
		g_free (child_name);
		
		child_node = g_list_next (child_node);
	}

	gl_debug (DEBUG_MDI, "END2");

	return NULL;
}

/**
 * bonobo_mdi_get_active_child:
 * @mdi: A pointer to a BonoboMDI object.
 * 
 * Description:
 * Returns a pointer to the active BonoboMDIChild object.
 * 
 * Return value: 
 * A pointer to the active BonoboMDIChild object. %NULL, if there is none.
 **/
BonoboMDIChild *
bonobo_mdi_get_active_child (BonoboMDI *mdi)
{
	g_return_val_if_fail (mdi != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_MDI (mdi), NULL);

	if (mdi->priv->active_view)
		return (bonobo_mdi_get_child_from_view (mdi->priv->active_view));

	return NULL;
}

/**
 * bonobo_mdi_get_active_view:
 * @mdi: A pointer to a BonoboMDI object.
 * 
 * Description:
 * Returns a pointer to the active view (the one with the focus).
 * 
 * Return value: 
 * A pointer to a GtkWidget *.
 **/
GtkWidget *
bonobo_mdi_get_active_view (BonoboMDI *mdi)
{
	g_return_val_if_fail (mdi != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_MDI (mdi), NULL);

	return mdi->priv->active_view;
}

/**
 * bonobo_mdi_get_active_window:
 * @mdi: A pointer to a BonoboMDI object.
 * 
 * Description:
 * Returns a pointer to the toplevel window containing the active view.
 * 
 * Return value:
 * A pointer to a BonoboWindow that has the focus.
 **/
BonoboWindow *
bonobo_mdi_get_active_window (BonoboMDI *mdi)
{
	g_return_val_if_fail (mdi != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_MDI (mdi), NULL);

	return mdi->priv->active_window;
}

void 
bonobo_mdi_set_ui_template (BonoboMDI *mdi, const gchar *xml, BonoboUIVerb verbs[])
{
	g_return_if_fail (mdi != NULL);
	g_return_if_fail (BONOBO_IS_MDI (mdi));
	g_return_if_fail (xml != NULL);

	if (mdi->priv->ui_xml != NULL)
	       g_free (mdi->priv->ui_xml);
	
	mdi->priv->ui_xml = g_strdup (xml);

	/* FIXME */
	mdi->priv->verbs = verbs;
}

void          
bonobo_mdi_set_ui_template_file (BonoboMDI *mdi, const gchar *file_name, BonoboUIVerb verbs[])
{
	g_return_if_fail (mdi != NULL);
	g_return_if_fail (BONOBO_IS_MDI (mdi));
	g_return_if_fail (file_name != NULL);

	if (mdi->priv->ui_file_name != NULL)
		g_free (mdi->priv->ui_file_name);
	
	mdi->priv->ui_file_name = g_strdup (file_name);

	/* FIXME */
	mdi->priv->verbs = verbs;
}

/**
 * bonobo_mdi_set_child_list_path:
 * @mdi: A pointer to a BonoboMDI object.
 * @path: A menu path where the child list menu should be inserted
 * 
 * Description:
 * Sets the position for insertion of menu items used to activate the MDI
 * children that were added to the MDI. See gnome_app_find_menu_pos for
 * details on menu paths. If the path is not set or set to %NULL, these menu
 * items aren't going to be inserted in the MDI menu structure. Note that if
 * you want all menu items to be inserted in their own submenu, you have to
 * create that submenu (and leave it empty, of course).
 **/
void 
bonobo_mdi_set_child_list_path (BonoboMDI *mdi, const gchar *path)
{
	g_return_if_fail (mdi != NULL);
	g_return_if_fail (BONOBO_IS_MDI (mdi));

	if (mdi->priv->child_list_path)
		g_free (mdi->priv->child_list_path);

	mdi->priv->child_list_path = g_strdup (path);
}

/**
 * bonobo_mdi_register:
 * @mdi: A pointer to a BonoboMDI object.
 * @object: Object to register.
 * 
 * Description:
 * Registers GObject @object with MDI. 
 * This is mostly intended for applications that open other windows besides
 * those opened by the MDI and want to continue to run even when no MDI
 * windows exist (an example of this would be GIMP's window with tools, if
 * the pictures were MDI children). As long as there is an object registered
 * with the MDI, the MDI will not destroy itself when the last of its windows
 * is closed. If no objects are registered, closing the last MDI window
 * results in MDI being destroyed. 
 **/
void 
bonobo_mdi_register (BonoboMDI *mdi, GObject *object)
{
	if (!g_slist_find (mdi->priv->registered, object))
		mdi->priv->registered = g_slist_append (mdi->priv->registered, object);
}

/**
 * bonobo_mdi_unregister:
 * @mdi: A pointer to a BonoboMDI object.
 * @object: Object to unregister.
 * 
 * Description:
 * Removes GObject @object from the list of registered objects. 
 **/
void 
bonobo_mdi_unregister (BonoboMDI *mdi, GObject *object)
{
	mdi->priv->registered = g_slist_remove (mdi->priv->registered, object);
}

/**
 * bonobo_mdi_get_child_from_view:
 * @view: A pointer to a GtkWidget.
 * 
 * Description:
 * Returns a child that @view is a view of.
 * 
 * Return value:
 * A pointer to the BonoboMDIChild the view belongs to.
 **/
BonoboMDIChild *
bonobo_mdi_get_child_from_view (GtkWidget *view)
{
	return BONOBO_MDI_CHILD (g_object_get_data (G_OBJECT(view), BONOBO_MDI_CHILD_KEY));
}

/**
 * bonobo_mdi_get_window_from_view:
 * @view: A pointer to a GtkWidget.
 * 
 * Description:
 * Returns the toplevel window for this view.
 * 
 * Return value:
 * A pointer to the BonoboWindow containg the specified view.
 **/
BonoboWindow *
bonobo_mdi_get_window_from_view (GtkWidget *view)
{
	return BONOBO_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (view)));
}

/**
 * bonobo_mdi_get_view_from_window:
 * @mdi: A pointer to a BonoboMDI object.
 * @win: A pointer to a BonoboWindow widget.
 * 
 * Description:
 * Returns the pointer to the view in the MDI toplevel window @win.
 * If the mode is set to %GNOME_MDI_NOTEBOOK, the view in the current
 * page is returned.
 * 
 * Return value: 
 * A pointer to a view.
 **/
GtkWidget *
bonobo_mdi_get_view_from_window (BonoboMDI *mdi, BonoboWindow *win)
{
	GtkWidget *book;
	
	gl_debug (DEBUG_MDI, "");

	g_return_val_if_fail (mdi != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_MDI (mdi), NULL);
	g_return_val_if_fail (win != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_WINDOW (win), NULL);

	book = get_book_from_window (win);
	g_return_val_if_fail (book != NULL, NULL);
	
	if (GTK_NOTEBOOK (book)->cur_page) 
	{
		int cur_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (book));

		gl_debug (DEBUG_MDI, "END1");

		return gtk_notebook_get_nth_page (GTK_NOTEBOOK (book), cur_page);
	} 
	else 
	{
		gl_debug (DEBUG_MDI, "END2");

		return NULL;
	}
}

void 
bonobo_mdi_construct (BonoboMDI *mdi, const gchar* name, const gchar* title,
		gint default_window_width, gint default_window_height)
{
	g_return_if_fail (mdi->priv->mdi_name == NULL);
	g_return_if_fail (mdi->priv->title == NULL);

	g_return_if_fail (name != NULL);
	g_return_if_fail (title != NULL);

	mdi->priv->mdi_name = g_strdup (name);
	mdi->priv->title = g_strdup (title);

	mdi->priv->default_window_width = default_window_width;
	mdi->priv->default_window_height = default_window_height;

}

GList *
bonobo_mdi_get_children (BonoboMDI *mdi)
{
	g_return_val_if_fail (BONOBO_IS_MDI (mdi), FALSE);

	return mdi->priv->children;
}

GList *
bonobo_mdi_get_windows (BonoboMDI *mdi)
{
	g_return_val_if_fail (BONOBO_IS_MDI (mdi), FALSE);

	return mdi->priv->windows;
}

	

BonoboUIComponent*
bonobo_mdi_get_ui_component_from_window (BonoboWindow* win)
{
	return BONOBO_UI_COMPONENT (
			g_object_get_data (G_OBJECT (win), UI_COMPONENT_KEY));
}

const BonoboMDIWindowInfo *
bonobo_mdi_get_window_info (BonoboWindow *win)
{
	return (const BonoboMDIWindowInfo *) 
			g_object_get_data (G_OBJECT (win), WINDOW_INFO_KEY);
}
