/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_text_entry.c:  text entry widget module
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

#include "wdgt-text-entry.h"
#include "merge.h"
#include "text-node.h"
#include "marshal.h"

#include "debug.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};

typedef void (*glWdgtTextEntrySignal) (GObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static glHigVBoxClass *parent_class;

static gint wdgt_text_entry_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_wdgt_text_entry_class_init    (glWdgtTextEntryClass *class);
static void gl_wdgt_text_entry_instance_init (glWdgtTextEntry      *text_entry);
static void gl_wdgt_text_entry_finalize      (GObject              *object);
static void gl_wdgt_text_entry_construct     (glWdgtTextEntry      *text_entry,
					      GList                *field_defs);

static void changed_cb (glWdgtTextEntry *text_entry);
static void insert_cb  (glWdgtTextEntry *text_entry);

/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
guint
gl_wdgt_text_entry_get_type (void)
{
	static guint wdgt_text_entry_type = 0;

	if (!wdgt_text_entry_type) {
		GTypeInfo wdgt_text_entry_info = {
			sizeof (glWdgtTextEntryClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_wdgt_text_entry_class_init,
			NULL,
			NULL,
			sizeof (glWdgtTextEntry),
			0,
			(GInstanceInitFunc) gl_wdgt_text_entry_instance_init,
		};

		wdgt_text_entry_type =
			g_type_register_static (gl_hig_vbox_get_type (),
						"glWdgtTextEntry",
						&wdgt_text_entry_info, 0);
	}

	return wdgt_text_entry_type;
}

static void
gl_wdgt_text_entry_class_init (glWdgtTextEntryClass * class)
{
	GObjectClass *object_class;

	gl_debug (DEBUG_WDGT, "START");

	object_class = (GObjectClass *) class;

	parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_wdgt_text_entry_finalize;

	wdgt_text_entry_signals[CHANGED] =
	    g_signal_new ("changed",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glWdgtTextEntryClass, changed),
			  NULL, NULL,
			  gl_marshal_VOID__VOID, G_TYPE_NONE, 0);

	gl_debug (DEBUG_WDGT, "END");
}

static void
gl_wdgt_text_entry_instance_init (glWdgtTextEntry *text_entry)
{
	gl_debug (DEBUG_WDGT, "START");

	text_entry->text_entry    = NULL;

	text_entry->key_combo     = NULL;
	text_entry->key_entry     = NULL;
	text_entry->insert_button = NULL;

	gl_debug (DEBUG_WDGT, "END");
}

static void
gl_wdgt_text_entry_finalize (GObject *object)
{
	glWdgtTextEntry      *text_entry;
	glWdgtTextEntryClass *class;

	gl_debug (DEBUG_WDGT, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WDGT_TEXT_ENTRY (object));

	text_entry = GL_WDGT_TEXT_ENTRY (object);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_WDGT, "END");
}

/****************************************************************************/
/* New widget.                                                              */
/****************************************************************************/
GtkWidget *
gl_wdgt_text_entry_new (GList *field_defs)
{
	glWdgtTextEntry *text_entry;

	gl_debug (DEBUG_WDGT, "START");

	text_entry = g_object_new (gl_wdgt_text_entry_get_type (), NULL);

	gl_wdgt_text_entry_construct (text_entry, field_defs);

	gl_debug (DEBUG_WDGT, "END");

	return GTK_WIDGET (text_entry);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                    */
/*--------------------------------------------------------------------------*/
static void
gl_wdgt_text_entry_construct (glWdgtTextEntry *text_entry,
			      GList           *field_defs)
{
	GtkWidget *wvbox, *whbox, *wscroll;
	GList *keys;

	gl_debug (DEBUG_WDGT, "START");

	wvbox = GTK_WIDGET (text_entry);

	/* ---- Entry line ---- */
	whbox = gl_hig_hbox_new ();
	gl_hig_vbox_add_widget (GL_HIG_VBOX(wvbox), whbox);

	/* Text Label */
	text_entry->edit_label = gtk_label_new (_("Edit text:"));
	gtk_misc_set_alignment (GTK_MISC (text_entry->edit_label), 0, 0);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), text_entry->edit_label);

	/* Actual text entry widget */
	wscroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_size_request (wscroll, -1, 70);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (wscroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gl_hig_hbox_add_widget_justify (GL_HIG_HBOX(whbox), wscroll);
	text_entry->text_entry = gtk_text_view_new ();
	text_entry->text_buffer =
		gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_entry->text_entry));
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (wscroll),
					       text_entry->text_entry);
	g_signal_connect_swapped (G_OBJECT (text_entry->text_buffer),
				  "changed", G_CALLBACK (changed_cb),
				  G_OBJECT (text_entry));

	/* ---- Merge field line ---- */
	whbox = gl_hig_hbox_new ();
	gl_hig_vbox_add_widget (GL_HIG_VBOX(wvbox), whbox);

	/* Insert merge field label */
	text_entry->key_label = gtk_label_new (_("Key:"));
	gtk_misc_set_alignment (GTK_MISC (text_entry->key_label), 0, 0.5);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), text_entry->key_label);

	/* Key entry widget */
	text_entry->key_combo = gtk_combo_new ();
	keys = gl_merge_get_key_list (field_defs);
	if (keys != NULL)
		gtk_combo_set_popdown_strings (GTK_COMBO (text_entry->key_combo),
					       keys);
	gl_merge_free_key_list (&keys);
	text_entry->key_entry = GTK_COMBO (text_entry->key_combo)->entry;
	gtk_entry_set_editable (GTK_ENTRY (text_entry->key_entry), FALSE);
	gtk_widget_set_size_request (text_entry->key_combo, 200, -1);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), text_entry->key_combo);

	/* Insert button */
	text_entry->insert_button =
	    gtk_button_new_with_label (_("Insert merge field"));
	g_signal_connect_swapped (G_OBJECT (text_entry->insert_button),
				  "clicked", G_CALLBACK (insert_cb),
				  G_OBJECT (text_entry));
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), text_entry->insert_button);

	gl_debug (DEBUG_WDGT, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when text has changed.                            */
/*--------------------------------------------------------------------------*/
static void
changed_cb (glWdgtTextEntry *text_entry)
{
	gl_debug (DEBUG_WDGT, "START");

	/* Emit our "changed" signal */
	g_signal_emit (G_OBJECT (text_entry),
		       wdgt_text_entry_signals[CHANGED], 0);

	gl_debug (DEBUG_WDGT, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback to insert field into text buffer.                     */
/*--------------------------------------------------------------------------*/
static void
insert_cb (glWdgtTextEntry *text_entry)
{
	GtkTextBuffer *buffer;
	gchar *key, *field;

	gl_debug (DEBUG_WDGT, "START");

	key =
	    gtk_editable_get_chars (GTK_EDITABLE (text_entry->key_entry), 0,
				    -1);
	field = g_strdup_printf ("FIELD{%s}", key);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_entry->text_entry));
	gtk_text_buffer_insert_at_cursor (buffer, field, -1);

	g_free (field);
	g_free (key);


	gl_debug (DEBUG_WDGT, "END");
}

/****************************************************************************/
/* Set new field definitions.                                               */
/****************************************************************************/
void
gl_wdgt_text_entry_set_field_defs (glWdgtTextEntry *text_entry,
				   GList           *field_defs)
{
	GList *keys;

	keys = gl_merge_get_key_list (field_defs);
	if ( keys != NULL ) {
		gtk_combo_set_popdown_strings (GTK_COMBO (text_entry->key_combo),
					       keys);
		gl_merge_free_key_list (&keys);
	} else {
		keys = g_list_append (keys, "");
		gtk_combo_set_popdown_strings (GTK_COMBO (text_entry->key_combo),
					       keys);
		g_list_free (keys);
	}
}

/****************************************************************************/
/* Get widget data.                                                         */
/****************************************************************************/
GList *
gl_wdgt_text_entry_get_text (glWdgtTextEntry *text_entry)
{
	GtkTextBuffer *buffer;
	gchar *text;
	GList *lines;
	GtkTextIter start, end;

	gl_debug (DEBUG_WDGT, "START");

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_entry->text_entry));

	gtk_text_buffer_get_bounds (buffer, &start, &end);

	text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

	lines = gl_text_node_lines_new_from_text (text);

	g_free (text);

	gl_debug (DEBUG_WDGT, "END");

	return lines;
}

/****************************************************************************/
/* Set widget data.                                                         */
/****************************************************************************/
void
gl_wdgt_text_entry_set_text (glWdgtTextEntry *text_entry,
			     gboolean         merge_flag,
			     GList           *lines)
{
	GtkTextBuffer *buffer;
	gchar *text;

	gl_debug (DEBUG_WDGT, "START");

	gtk_widget_set_sensitive (text_entry->key_combo, merge_flag);
	gtk_widget_set_sensitive (text_entry->insert_button, merge_flag);

	text = gl_text_node_lines_expand (lines, NULL);

	g_signal_handlers_block_by_func (G_OBJECT(text_entry->text_buffer),
					 changed_cb, text_entry);
	gtk_text_buffer_set_text (text_entry->text_buffer, text, -1);
	g_signal_handlers_unblock_by_func (G_OBJECT(text_entry->text_buffer),
					   changed_cb, text_entry);

	gl_debug (DEBUG_WDGT, "END");
}

/*****************************************************************************/
/* Set size group for internal labels                                        */
/*****************************************************************************/
void
gl_wdgt_text_entry_set_label_size_group (glWdgtTextEntry *text_entry,
					 GtkSizeGroup    *label_size_group)
{
	gtk_size_group_add_widget (label_size_group, text_entry->edit_label);
	gtk_size_group_add_widget (label_size_group, text_entry->key_label);
}

