/* Modified version of gnome-propertybox from gnome-libs-1.4 */
/* Primarily removed the "help" button and changed the names.*/
/*  -Jim Evins 11/25/2001 */

/* gnome-propertybox.c - Property dialog box.

   Copyright (C) 1998 Tom Tromey

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/* Note that the property box is constructed so that we could later
   change how the buttons work.  For instance, we could put an Apply
   button inside each page; this kind of Apply button would only
   affect the current page.  Please do not change the API in a way
   that would violate this goal.  */

#include <config.h>

#include "propertybox.h"
#include <libgnome/gnome-util.h>
#include <libgnomeui/gnome-stock.h>
#include <libgnome/gnome-config.h>
#include <gtk/gtk.h>
#include <libgnomeui/gnome-preferences.h>

enum {
	APPLY,
	LAST_SIGNAL
};

typedef void (*glPropertyBoxSignal) (GtkObject * object,
				     gint arg,
				     gpointer data);

static void gl_property_box_class_init (glPropertyBoxClass * klass);
static void gl_property_box_init (glPropertyBox * property_box);
static void gl_property_box_marshal_signal (GtkObject * object,
					    GtkSignalFunc func,
					    gpointer func_data,
					    GtkArg * args);
static void gl_property_box_destroy (GtkObject * object);

/*
 * These four are called from dialog_clicked_cb(), depending
 * on which button was clicked.
 */
static void global_apply (glPropertyBox * property_box);
static void apply_and_close (glPropertyBox * property_box);
static void just_close (glPropertyBox * property_box);

static void dialog_clicked_cb (GnomeDialog * dialog,
			       gint button,
			       gpointer data);

static GnomeDialogClass *parent_class = NULL;

static gint property_box_signals[LAST_SIGNAL] = { 0 };

/**
 * gl_property_box_get_type:
 *
 * Internal routine that returns the GtkType of the
 * glPropertyBox widget
 */
guint
gl_property_box_get_type (void)
{
	static guint property_box_type = 0;

	if (!property_box_type) {
		GtkTypeInfo property_box_info = {
			"glPropertyBox",
			sizeof (glPropertyBox),
			sizeof (glPropertyBoxClass),
			(GtkClassInitFunc) gl_property_box_class_init,
			(GtkObjectInitFunc) gl_property_box_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		property_box_type = gtk_type_unique (gnome_dialog_get_type (),
						     &property_box_info);
	}

	return property_box_type;
}

static void
gl_property_box_class_init (glPropertyBoxClass * klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GtkWindowClass *window_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;
	window_class = (GtkWindowClass *) klass;

	object_class->destroy = gl_property_box_destroy;

	parent_class = gtk_type_class (gnome_dialog_get_type ());

	property_box_signals[APPLY] =
	    gtk_signal_new ("apply",
			    GTK_RUN_LAST,
			    object_class->type,
			    GTK_SIGNAL_OFFSET (glPropertyBoxClass,
					       apply),
			    gl_property_box_marshal_signal,
			    GTK_TYPE_NONE, 1, GTK_TYPE_INT);

	gtk_object_class_add_signals (object_class, property_box_signals,
				      LAST_SIGNAL);

	klass->apply = NULL;
}

static void
gl_property_box_marshal_signal (GtkObject * object,
				GtkSignalFunc func,
				gpointer func_data,
				GtkArg * args)
{
	glPropertyBoxSignal rfunc;

	rfunc = (glPropertyBoxSignal) func;
	(*rfunc) (object, GTK_VALUE_INT (args[0]), func_data);
}

static void
gl_property_box_init (glPropertyBox * property_box)
{
	GList *button_list;

	property_box->notebook = gtk_notebook_new ();

	if (gnome_preferences_get_property_box_apply ()) {
		gnome_dialog_append_buttons (GNOME_DIALOG (property_box),
					     GNOME_STOCK_BUTTON_OK,
					     GNOME_STOCK_BUTTON_APPLY,
					     GNOME_STOCK_BUTTON_CLOSE, NULL);
	} else {
		gnome_dialog_append_buttons (GNOME_DIALOG (property_box),
					     GNOME_STOCK_BUTTON_OK,
					     GNOME_STOCK_BUTTON_CANCEL, NULL);
	}

	gnome_dialog_set_default (GNOME_DIALOG (property_box), 0);

	/* This is sort of unattractive */

	button_list = GNOME_DIALOG (property_box)->buttons;

	property_box->ok_button = GTK_WIDGET (button_list->data);
	button_list = button_list->next;

	if (gnome_preferences_get_property_box_apply ()) {
		property_box->apply_button = GTK_WIDGET (button_list->data);
		button_list = button_list->next;
		gtk_widget_set_sensitive (property_box->apply_button, FALSE);
	} else
		property_box->apply_button = 0;

	property_box->cancel_button = GTK_WIDGET (button_list->data);
	button_list = button_list->next;

	gtk_signal_connect (GTK_OBJECT (property_box), "clicked",
			    GTK_SIGNAL_FUNC (dialog_clicked_cb), NULL);

	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (property_box)->vbox),
			    property_box->notebook, TRUE, TRUE, 0);

	gtk_widget_show (property_box->notebook);
}

static void
gl_property_box_destroy (GtkObject * object)
{
	glPropertyBox *property_box;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_PROPERTY_BOX (object));

	property_box = GL_PROPERTY_BOX (object);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

/**
 * gl_property_box_new: [constructor]
 *
 * Creates a new glPropertyBox widget.  The PropertyBox widget
 * is useful for making consistent configuration dialog boxes.
 *
 * When a setting has been made to a property in the PropertyBox
 * your program needs to invoke the gl_property_box_changed to signal
 * a change (this will enable the Ok/Apply buttons).
 *
 * Returns a newly created glPropertyBox widget.
 */
GtkWidget *
gl_property_box_new (void)
{
	return gtk_type_new (gl_property_box_get_type ());
}

static void
dialog_clicked_cb (GnomeDialog * dialog,
		   gint button,
		   gpointer data)
{
	glPropertyBox *pbox;
	GtkWidget *page;
	GList *list;
	gboolean dirty = FALSE;

	g_return_if_fail (dialog != NULL);
	g_return_if_fail (GL_IS_PROPERTY_BOX (dialog));

	pbox = GL_PROPERTY_BOX (dialog);

	if (GTK_NOTEBOOK (pbox->notebook)->cur_page != NULL) {

		for (list = GTK_NOTEBOOK (pbox->notebook)->children;
		     list != NULL; list = list->next) {
			GtkNotebookPage *page = list->data;
			g_assert (page != NULL);

			dirty =
			    GPOINTER_TO_INT (gtk_object_get_data
					     (GTK_OBJECT (page->child),
					      GL_PROPERTY_BOX_DIRTY));

			if (dirty)
				break;
		}
	} else {
		page = NULL;
		dirty = FALSE;
	}

	/* Choose which style we did */
	if (pbox->apply_button) {
		switch (button) {
		case 0:
			if (dirty)
				apply_and_close (GL_PROPERTY_BOX (dialog));
			else
				just_close (GL_PROPERTY_BOX (dialog));
			break;
		case 1:
			global_apply (GL_PROPERTY_BOX (dialog));
			break;
		case 2:
			just_close (GL_PROPERTY_BOX (dialog));
			break;
		default:
			g_assert_not_reached ();
		}
	} else {
		switch (button) {
		case 0:
			if (dirty)
				apply_and_close (GL_PROPERTY_BOX (dialog));
			else
				just_close (GL_PROPERTY_BOX (dialog));
			break;
		case 1:
			just_close (GL_PROPERTY_BOX (dialog));
			break;
		default:
			g_assert_not_reached ();
		}
	}
}

static void
set_sensitive (glPropertyBox * property_box,
	       gint dirty)
{
	if (property_box->apply_button)
		gtk_widget_set_sensitive (property_box->apply_button, dirty);
}

/**
 * gl_property_box_changed:
 * @property_box: The glPropertyBox that contains the changed data
 *
 * When a setting has changed, the code needs to invoke this routine
 * to make the Ok/Apply buttons sensitive.
 */
void
gl_property_box_changed (glPropertyBox * property_box)
{
	GtkWidget *page;

	g_return_if_fail (property_box != NULL);
	g_return_if_fail (GL_IS_PROPERTY_BOX (property_box));
	g_return_if_fail (property_box->notebook);
	g_return_if_fail (GTK_NOTEBOOK (property_box->notebook)->cur_page);

	page = GTK_NOTEBOOK (property_box->notebook)->cur_page->child;
	g_assert (page != NULL);

	gtk_object_set_data (GTK_OBJECT (page),
			     GL_PROPERTY_BOX_DIRTY, GINT_TO_POINTER (1));

	set_sensitive (property_box, 1);
}

/**
 * gl_property_box_set_modified:
 * @property_box: The glPropertyBox that contains the changed data
 * @state:        The state.  TRUE means modified, FALSE means unmodified.
 *
 * This sets the modified flag of the glPropertyBox to the value in @state.
 * Affects whether the OK/Apply buttons are sensitive.
 */
void
gl_property_box_set_modified (glPropertyBox * property_box,
			      gboolean state)
{
	GtkWidget *page;

	g_return_if_fail (property_box != NULL);
	g_return_if_fail (GL_IS_PROPERTY_BOX (property_box));
	g_return_if_fail (property_box->notebook);
	g_return_if_fail (GTK_NOTEBOOK (property_box->notebook)->cur_page);

	page = GTK_NOTEBOOK (property_box->notebook)->cur_page->child;
	g_assert (page != NULL);

	gtk_object_set_data (GTK_OBJECT (page),
			     GL_PROPERTY_BOX_DIRTY,
			     GINT_TO_POINTER (state ? 1 : 0));

	set_sensitive (property_box, state);
}

static void
global_apply (glPropertyBox * property_box)
{
	GList *list;
	gint n;

	g_return_if_fail (GTK_NOTEBOOK (property_box->notebook)->children !=
			  NULL);

	for (list = GTK_NOTEBOOK (property_box->notebook)->children, n = 0;
	     list != NULL; list = g_list_next (list), n++) {
		/* FIXME: there should be a way to report an error
		   during Apply.  That way we could prevent closing
		   the window if there were a problem.  */
		GtkNotebookPage *page = list->data;
		if (gtk_object_get_data (GTK_OBJECT (page->child),
					 GL_PROPERTY_BOX_DIRTY)) {
			gtk_signal_emit (GTK_OBJECT (property_box),
					 property_box_signals[APPLY], n);
			gtk_object_set_data (GTK_OBJECT (page->child),
					     GL_PROPERTY_BOX_DIRTY,
					     GINT_TO_POINTER (0));
		}
	}

	/* Emit an apply signal with a button of -1.  This means we
	   just finished a global apply.  Is this a hack?  */
	gtk_signal_emit (GTK_OBJECT (property_box),
			 property_box_signals[APPLY], (gint) - 1);

	/* Doesn't matter which item we use. */
	set_sensitive (property_box, 0);
}

static void
just_close (glPropertyBox * property_box)
{
	gnome_dialog_close (GNOME_DIALOG (property_box));
}

static void
apply_and_close (glPropertyBox * property_box)
{
	global_apply (property_box);
	just_close (property_box);
}

/**
 * gl_property_box_append_page:
 * @property_box: The property box where we are inserting a new page
 * @child:        The widget that is being inserted
 * @tab_label:    The widget used as the label for this confiugration page
 *
 * Appends a new page to the glPropertyBox.
 *
 * Returns the assigned index of the page inside the glPropertyBox or
 * -1 if one of the arguments is invalid.
 */
gint
gl_property_box_append_page (glPropertyBox * property_box,
			     GtkWidget * child,
			     GtkWidget * tab_label)
{
	g_return_val_if_fail (property_box != NULL, -1);
	g_return_val_if_fail (GL_IS_PROPERTY_BOX (property_box), -1);
	g_return_val_if_fail (child != NULL, -1);
	g_return_val_if_fail (GTK_IS_WIDGET (child), -1);
	g_return_val_if_fail (tab_label != NULL, -1);
	g_return_val_if_fail (GTK_IS_WIDGET (tab_label), -1);

	gtk_notebook_append_page (GTK_NOTEBOOK (property_box->notebook),
				  child, tab_label);

	return g_list_length (GTK_NOTEBOOK (property_box->notebook)->children) -
	    1;
}
