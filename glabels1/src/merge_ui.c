/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  merge_ui.c:  document merge user interface module
 *
 *  Copyright (C) 2002  Jim Evins <evins@snaught.com>.
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

#include "merge_ui.h"

/* Backends */
#include "merge_ui_text.h"

#include "debug.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};

typedef struct {

	GtkWidget * (*src_new) (glMergeType);
	void (*src_set_value) ();
	gchar * (*src_get_value) ();

	GtkWidget * (*field_ws_new) (glMergeType, gchar *);
	void (*field_ws_set_field_defs) ();
	GList * (*field_ws_get_field_defs) ();

} BackendFunctions;

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkContainerClass *parent_class;

static gint merge_ui_src_signals[LAST_SIGNAL] = { 0 };
static gint merge_ui_field_ws_signals[LAST_SIGNAL] = { 0 };

static BackendFunctions func[GL_MERGE_N_TYPES];

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_merge_ui_src_class_init (glMergeUISrcClass * class);
static void gl_merge_ui_src_init (glMergeUISrc * src);
static void gl_merge_ui_src_destroy (GtkObject * object);

static void src_changed_cb (glMergeUISrc * src);

static void gl_merge_ui_field_ws_class_init (glMergeUIFieldWSClass * class);
static void gl_merge_ui_field_ws_init (glMergeUIFieldWS * field_ws);
static void gl_merge_ui_field_ws_destroy (GtkObject * object);

static void field_ws_changed_cb (glMergeUIFieldWS * field_ws);

/*****************************************************************************/
/* Initialize module.                                                        */
/*****************************************************************************/
void
gl_merge_ui_init (void)
{
	gint i;

	/* Register backend functions. */

	i = GL_MERGE_NONE;
	func[i].src_new = NULL;
	func[i].src_set_value = NULL;
	func[i].src_get_value = NULL;
	func[i].field_ws_new = NULL;
	func[i].field_ws_set_field_defs = NULL;
	func[i].field_ws_get_field_defs = NULL;

	i = GL_MERGE_TEXT_TAB;
	func[i].src_new = gl_merge_ui_text_src_new;
	func[i].src_set_value = gl_merge_ui_text_src_set_value;
	func[i].src_get_value = gl_merge_ui_text_src_get_value;
	func[i].field_ws_new = gl_merge_ui_text_field_ws_new;
	func[i].field_ws_set_field_defs =
		gl_merge_ui_text_field_ws_set_field_defs;
	func[i].field_ws_get_field_defs =
		gl_merge_ui_text_field_ws_get_field_defs;

	i = GL_MERGE_TEXT_COMMA;
	func[i].src_new = gl_merge_ui_text_src_new;
	func[i].src_set_value = gl_merge_ui_text_src_set_value;
	func[i].src_get_value = gl_merge_ui_text_src_get_value;
	func[i].field_ws_new = gl_merge_ui_text_field_ws_new;
	func[i].field_ws_set_field_defs =
		gl_merge_ui_text_field_ws_set_field_defs;
	func[i].field_ws_get_field_defs =
		gl_merge_ui_text_field_ws_get_field_defs;

	i = GL_MERGE_TEXT_COLON;
	func[i].src_new = gl_merge_ui_text_src_new;
	func[i].src_set_value = gl_merge_ui_text_src_set_value;
	func[i].src_get_value = gl_merge_ui_text_src_get_value;
	func[i].field_ws_new = gl_merge_ui_text_field_ws_new;
	func[i].field_ws_set_field_defs =
		gl_merge_ui_text_field_ws_set_field_defs;
	func[i].field_ws_get_field_defs =
		gl_merge_ui_text_field_ws_get_field_defs;

}

/*****************************************************************************/
/* Boilerplate Object stuff.                                                 */
/*****************************************************************************/
guint
gl_merge_ui_src_get_type (void)
{
	static guint merge_ui_src_type = 0;

	if (!merge_ui_src_type) {
		GtkTypeInfo merge_ui_src_info = {
			"glMergeUISrc",
			sizeof (glMergeUISrc),
			sizeof (glMergeUISrcClass),
			(GtkClassInitFunc) gl_merge_ui_src_class_init,
			(GtkObjectInitFunc) gl_merge_ui_src_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
		};

		merge_ui_src_type = gtk_type_unique (gtk_vbox_get_type (),
						     &merge_ui_src_info);
	}

	return merge_ui_src_type;
}

static void
gl_merge_ui_src_class_init (glMergeUISrcClass * class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	object_class->destroy = gl_merge_ui_src_destroy;

	merge_ui_src_signals[CHANGED] =
	    gtk_signal_new ("changed", GTK_RUN_LAST, object_class->type,
			    GTK_SIGNAL_OFFSET (glMergeUISrcClass, changed),
			    gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals (object_class,
				      merge_ui_src_signals, LAST_SIGNAL);

	class->changed = NULL;
}

static void
gl_merge_ui_src_init (glMergeUISrc * src)
{
	src->backend_widget = NULL;
}

static void
gl_merge_ui_src_destroy (GtkObject * object)
{
	glMergeUISrc *src;
	glMergeUISrcClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_MERGE_UI_SRC (object));

	src = GL_MERGE_UI_SRC (object);
	class = GL_MERGE_UI_SRC_CLASS (GTK_OBJECT (src)->klass);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

GtkWidget *
gl_merge_ui_src_new (void)
{
	glMergeUISrc *src;

	src = gtk_type_new (gl_merge_ui_src_get_type ());

	src->type = GL_MERGE_NONE;
	src->backend_widget = gtk_label_new (_("N/A"));
	gtk_misc_set_alignment (GTK_MISC (src->backend_widget), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (src), src->backend_widget, FALSE, FALSE,
			    0);

	return GTK_WIDGET (src);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when our backend widget has changed.              */
/*--------------------------------------------------------------------------*/
static void
src_changed_cb (glMergeUISrc * src)
{
	/* Emit our "changed" signal */
	gtk_signal_emit (GTK_OBJECT (src), merge_ui_src_signals[CHANGED]);
}

/*****************************************************************************/
/* Set type.                                                                 */
/*****************************************************************************/
void
gl_merge_ui_src_set_type (glMergeUISrc * src,
			  glMergeType type)
{

	if (type != src->type) {

		gtk_widget_destroy (src->backend_widget);

		src->type = type;

		if (type != GL_MERGE_NONE) {
			src->backend_widget = (func[type].src_new) (type);
		} else {
			src->backend_widget = gtk_label_new (_("N/A"));
			gtk_misc_set_alignment (GTK_MISC (src->backend_widget),
						0, 0.5);
		}

		gtk_box_pack_start (GTK_BOX (src), src->backend_widget,
				    FALSE, FALSE, 0);

		if (GTK_WIDGET_VISIBLE (src)) {
			gtk_widget_show_all (src->backend_widget);
		}

		if (type != GL_MERGE_NONE) {
			gtk_signal_connect_object (GTK_OBJECT
						   (src->backend_widget),
						   "changed",
						   GTK_SIGNAL_FUNC
						   (src_changed_cb),
						   GTK_OBJECT (src));
		}

		/* Emit our "changed" signal */
		gtk_signal_emit (GTK_OBJECT (src),
				 merge_ui_src_signals[CHANGED]);
	}

}

/*****************************************************************************/
/* Set src.                                                                  */
/*****************************************************************************/
void
gl_merge_ui_src_set_value (glMergeUISrc * src,
			   gchar * text)
{
	glMergeType type;

	type = src->type;

	if (type != GL_MERGE_NONE) {
		func[type].src_set_value (src->backend_widget, text);
	} else {
		return;
	}

}

/*****************************************************************************/
/* Get src name.                                                             */
/*****************************************************************************/
gchar *
gl_merge_ui_src_get_value (glMergeUISrc * src)
{
	glMergeType type;

	type = src->type;

	if (type != GL_MERGE_NONE) {
		return func[type].src_get_value (src->backend_widget);
	} else {
		return NULL;
	}

}

/*****************************************************************************/
/* Boilerplate Object stuff.                                                 */
/*****************************************************************************/
guint
gl_merge_ui_field_ws_get_type (void)
{
	static guint merge_ui_field_ws_type = 0;

	if (!merge_ui_field_ws_type) {
		GtkTypeInfo merge_ui_field_ws_info = {
			"glMergeUIFieldWS",
			sizeof (glMergeUIFieldWS),
			sizeof (glMergeUIFieldWSClass),
			(GtkClassInitFunc) gl_merge_ui_field_ws_class_init,
			(GtkObjectInitFunc) gl_merge_ui_field_ws_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
		};

		merge_ui_field_ws_type = gtk_type_unique (gtk_vbox_get_type (),
							  &merge_ui_field_ws_info);
	}

	return merge_ui_field_ws_type;
}

static void
gl_merge_ui_field_ws_class_init (glMergeUIFieldWSClass * class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	object_class->destroy = gl_merge_ui_field_ws_destroy;

	merge_ui_field_ws_signals[CHANGED] =
	    gtk_signal_new ("changed", GTK_RUN_LAST, object_class->type,
			    GTK_SIGNAL_OFFSET (glMergeUIFieldWSClass, changed),
			    gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals (object_class,
				      merge_ui_field_ws_signals, LAST_SIGNAL);

	class->changed = NULL;
}

static void
gl_merge_ui_field_ws_init (glMergeUIFieldWS * field_ws)
{
	field_ws->type = GL_MERGE_NONE;
	field_ws->src = NULL;
	field_ws->backend_widget = NULL;
}

static void
gl_merge_ui_field_ws_destroy (GtkObject * object)
{
	glMergeUIFieldWS *field_ws;
	glMergeUIFieldWSClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_MERGE_UI_FIELD_WS (object));

	field_ws = GL_MERGE_UI_FIELD_WS (object);
	class = GL_MERGE_UI_FIELD_WS_CLASS (GTK_OBJECT (field_ws)->klass);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

GtkWidget *
gl_merge_ui_field_ws_new (void)
{
	glMergeUIFieldWS *field_ws;

	field_ws = gtk_type_new (gl_merge_ui_field_ws_get_type ());

	field_ws->type = GL_MERGE_NONE;
	field_ws->backend_widget = gtk_label_new (_("N/A"));
	gtk_misc_set_alignment (GTK_MISC (field_ws->backend_widget), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (field_ws), field_ws->backend_widget,
			    FALSE, FALSE, 0);

	return GTK_WIDGET (field_ws);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when our backend widget has changed.              */
/*--------------------------------------------------------------------------*/
static void
field_ws_changed_cb (glMergeUIFieldWS * field_ws)
{
	/* Emit our "changed" signal */
	gtk_signal_emit (GTK_OBJECT (field_ws),
			 merge_ui_field_ws_signals[CHANGED]);
}

/*****************************************************************************/
/* set type and src data, create appropriate backend widget.                 */
/*****************************************************************************/
void
gl_merge_ui_field_ws_set_type_src (glMergeUIFieldWS * field_ws,
				   glMergeType type,
				   gchar * src)
{

	if (field_ws->src != NULL)
		g_free (field_ws->src);
	if (field_ws->backend_widget != NULL) {
		gtk_widget_destroy (field_ws->backend_widget);
	}

	field_ws->type = type;
	field_ws->src = g_strdup (src);

	if (type != GL_MERGE_NONE) {
		field_ws->backend_widget =
		    (func[type].field_ws_new) (type, src);
	} else {
		field_ws->backend_widget = gtk_label_new (_("N/A"));
		gtk_misc_set_alignment (GTK_MISC (field_ws->backend_widget), 0,
					0.5);
	}

	gtk_box_pack_start (GTK_BOX (field_ws), field_ws->backend_widget,
			    FALSE, FALSE, 0);

	if (type != GL_MERGE_NONE) {
		gtk_signal_connect_object (GTK_OBJECT
					   (field_ws->backend_widget),
					   "changed",
					   GTK_SIGNAL_FUNC
					   (field_ws_changed_cb),
					   GTK_OBJECT (field_ws));
	}

	if (GTK_WIDGET_VISIBLE (field_ws)) {
		gtk_widget_show_all (field_ws->backend_widget);
	}
	/* Emit our "changed" signal */
	gtk_signal_emit (GTK_OBJECT (field_ws),
			 merge_ui_field_ws_signals[CHANGED]);

}

/*****************************************************************************/
/* Set field definitions (associate ids with raw fields).                    */
/*****************************************************************************/
void
gl_merge_ui_field_ws_set_field_defs (glMergeUIFieldWS * field_ws,
				     GList * field_defs)
{
	glMergeType type;

	type = field_ws->type;

	if (type != GL_MERGE_NONE) {

		(func[type].field_ws_set_field_defs) (field_ws->backend_widget,
						      field_defs);

		/* Emit our "changed" signal */
		gtk_signal_emit (GTK_OBJECT (field_ws),
				 merge_ui_field_ws_signals[CHANGED]);

	}
}

/*****************************************************************************/
/* Get field definitions (associate ids with raw fields).                    */
/*****************************************************************************/
GList *
gl_merge_ui_field_ws_get_field_defs (glMergeUIFieldWS * field_ws)
{
	glMergeType type;

	type = field_ws->type;

	if (type != GL_MERGE_NONE) {

		return (func[type].field_ws_get_field_defs) (field_ws->
							     backend_widget);

	} else {

		return NULL;

	}

}
