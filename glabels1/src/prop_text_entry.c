/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prop_text_entry.c:  text entry widget module
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

#include "prop_text_entry.h"
#include "merge.h"
#include "text_node.h"

#include "debug.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};

typedef void (*glPropTextEntrySignal) (GtkObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkContainerClass *parent_class;

static gint prop_text_entry_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_prop_text_entry_class_init (glPropTextEntryClass * class);
static void gl_prop_text_entry_init (glPropTextEntry * text_entry);
static void gl_prop_text_entry_destroy (GtkObject * object);
static void gl_prop_text_entry_construct (glPropTextEntry * text_entry,
					  gchar * label, GList * field_defs);

static void changed_cb (glPropTextEntry * text_entry);
static void insert_cb (glPropTextEntry * text_entry);

/*================================================================*/
/* Boilerplate Object stuff.                                      */
/*================================================================*/
guint
gl_prop_text_entry_get_type (void)
{
	static guint prop_text_entry_type = 0;

	if (!prop_text_entry_type) {
		GtkTypeInfo prop_text_entry_info = {
			"glPropTextEntry",
			sizeof (glPropTextEntry),
			sizeof (glPropTextEntryClass),
			(GtkClassInitFunc) gl_prop_text_entry_class_init,
			(GtkObjectInitFunc) gl_prop_text_entry_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
		};

		prop_text_entry_type = gtk_type_unique (gtk_vbox_get_type (),
							&prop_text_entry_info);
	}

	return prop_text_entry_type;
}

static void
gl_prop_text_entry_class_init (glPropTextEntryClass * class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	object_class->destroy = gl_prop_text_entry_destroy;

	prop_text_entry_signals[CHANGED] =
	    gtk_signal_new ("changed", GTK_RUN_LAST, object_class->type,
			    GTK_SIGNAL_OFFSET (glPropTextEntryClass, changed),
			    gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals (object_class,
				      prop_text_entry_signals, LAST_SIGNAL);

	class->changed = NULL;
}

static void
gl_prop_text_entry_init (glPropTextEntry * text_entry)
{
	text_entry->text_entry = NULL;
	text_entry->key_entry = NULL;
	text_entry->insert_button = NULL;
}

static void
gl_prop_text_entry_destroy (GtkObject * object)
{
	glPropTextEntry *text_entry;
	glPropTextEntryClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_PROP_TEXT_ENTRY (object));

	text_entry = GL_PROP_TEXT_ENTRY (object);
	class = GL_PROP_TEXT_ENTRY_CLASS (GTK_OBJECT (text_entry)->klass);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

GtkWidget *
gl_prop_text_entry_new (gchar * label,
			GList * field_defs)
{
	glPropTextEntry *text_entry;

	text_entry = gtk_type_new (gl_prop_text_entry_get_type ());

	gl_prop_text_entry_construct (text_entry, label, field_defs);

	return GTK_WIDGET (text_entry);
}

/*============================================================*/
/* Construct composite widget.                                */
/*============================================================*/
static void
gl_prop_text_entry_construct (glPropTextEntry * text_entry,
			      gchar * label,
			      GList * field_defs)
{
	GtkWidget *wvbox, *wframe, *wtable, *wlabel, *wcombo;
	GList *keys;

	wvbox = GTK_WIDGET (text_entry);

	wframe = gtk_frame_new (label);
	gtk_box_pack_start (GTK_BOX (wvbox), wframe, FALSE, FALSE, 0);

	wtable = gtk_table_new (2, 3, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (wtable), 10);
	gtk_table_set_row_spacings (GTK_TABLE (wtable), 5);
	gtk_table_set_col_spacings (GTK_TABLE (wtable), 5);
	gtk_container_add (GTK_CONTAINER (wframe), wtable);

	/* Actual text entry widget */
	text_entry->text_entry = gtk_text_new (NULL, NULL);
	gtk_text_set_editable (GTK_TEXT (text_entry->text_entry), TRUE);
	gtk_signal_connect_object (GTK_OBJECT (text_entry->text_entry),
				   "changed", GTK_SIGNAL_FUNC (changed_cb),
				   GTK_OBJECT (text_entry));
	gtk_table_attach_defaults (GTK_TABLE (wtable), text_entry->text_entry,
				   0, 3, 0, 1);

	/* Insert merge field label */
	wlabel = gtk_label_new (_("Key:"));
	gtk_table_attach_defaults (GTK_TABLE (wtable), wlabel, 0, 1, 1, 2);

	/* Key entry widget */
	wcombo = gtk_combo_new ();
	keys = gl_merge_get_key_list (field_defs);
	if (keys != NULL)
		gtk_combo_set_popdown_strings (GTK_COMBO (wcombo), keys);
	gl_merge_free_key_list (&keys);
	text_entry->key_entry = GTK_COMBO (wcombo)->entry;
	gtk_entry_set_editable (GTK_ENTRY (text_entry->key_entry), FALSE);
	gtk_widget_set_usize (wcombo, 200, 0);
	gtk_table_attach_defaults (GTK_TABLE (wtable), wcombo, 1, 2, 1, 2);

	/* Insert button */
	text_entry->insert_button =
	    gtk_button_new_with_label (_("Insert merge field"));
	gtk_signal_connect_object (GTK_OBJECT (text_entry->insert_button),
				   "clicked", GTK_SIGNAL_FUNC (insert_cb),
				   GTK_OBJECT (text_entry));
	gtk_table_attach_defaults (GTK_TABLE (wtable),
				   text_entry->insert_button, 2, 3, 1, 2);

}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the widget has changed.       */
/*--------------------------------------------------------------------------*/
static void
changed_cb (glPropTextEntry * text_entry)
{
	/* Emit our "changed" signal */
	gtk_signal_emit (GTK_OBJECT (text_entry),
			 prop_text_entry_signals[CHANGED]);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the widget has changed.       */
/*--------------------------------------------------------------------------*/
static void
insert_cb (glPropTextEntry * text_entry)
{
	gchar *key, *field;
	gint pos;

	key =
	    gtk_editable_get_chars (GTK_EDITABLE (text_entry->key_entry), 0,
				    -1);
	field = g_strdup_printf ("FIELD{%s}", key);

	pos = gtk_editable_get_position (GTK_EDITABLE (text_entry->text_entry));
	gtk_editable_insert_text (GTK_EDITABLE (text_entry->text_entry),
				  field, strlen (field), &pos);

	g_free (field);
	g_free (key);

}

/*--------------------------------------------------------------------------*/
/* Get widget data.                                                         */
/*--------------------------------------------------------------------------*/
GList *
gl_prop_text_entry_get_text (glPropTextEntry * text_entry)
{
	gchar *text;
	GList *lines;

	text =
	    gtk_editable_get_chars (GTK_EDITABLE (text_entry->text_entry), 0,
				    -1);

	lines = gl_text_node_lines_new_from_text (text);

	g_free (text);
	return lines;
}

/*--------------------------------------------------------------------------*/
/* Set widget data.                                                         */
/*--------------------------------------------------------------------------*/
void
gl_prop_text_entry_set_text (glPropTextEntry * text_entry,
			     gboolean merge_flag,
			     GList * lines)
{
	gint pos;
	gchar *text;

	gtk_widget_set_sensitive (text_entry->key_entry, merge_flag);
	gtk_widget_set_sensitive (text_entry->insert_button, merge_flag);

	gtk_signal_handler_block_by_func (GTK_OBJECT (text_entry->text_entry),
					  GTK_SIGNAL_FUNC (changed_cb),
					  text_entry);
	gtk_editable_delete_text (GTK_EDITABLE (text_entry->text_entry), 0, -1);
	gtk_signal_handler_unblock_by_func (GTK_OBJECT (text_entry->text_entry),
					    GTK_SIGNAL_FUNC (changed_cb),
					    text_entry);

	text = gl_text_node_lines_expand (lines, NULL);

	pos = 0;
	gtk_editable_insert_text (GTK_EDITABLE (text_entry->text_entry),
				  text, strlen (text), &pos);

}
