/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  wdgt_image_select.c:  image selection widget module
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

#include "wdgt-image-select.h"
#include "merge.h"
#include "marshal.h"

#include "debug.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};

typedef void (*glWdgtImageSelectSignal) (GObject * object, gpointer data);

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static glHigVBoxClass *parent_class;

static gint wdgt_image_select_signals[LAST_SIGNAL] = { 0 };

/* Save state of image file entry */
static gchar *image_path = NULL;

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_wdgt_image_select_class_init    (glWdgtImageSelectClass *class);
static void gl_wdgt_image_select_instance_init (glWdgtImageSelect      *image_select);
static void gl_wdgt_image_select_finalize      (GObject                *object);
static void gl_wdgt_image_select_construct     (glWdgtImageSelect      *image_select,
						glMerge                *merge,
						glTextNode             *text_node);


static void changed_cb                         (glWdgtImageSelect      *image_select);

static void radio_toggled_cb                   (GtkToggleButton   *togglebutton,
						glWdgtImageSelect  *image_select);


/****************************************************************************/
/* Boilerplate Object stuff.                                                */
/****************************************************************************/
guint
gl_wdgt_image_select_get_type (void)
{
	static guint wdgt_image_select_type = 0;

	if (!wdgt_image_select_type) {
		GTypeInfo wdgt_image_select_info = {
			sizeof (glWdgtImageSelectClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_wdgt_image_select_class_init,
			NULL,
			NULL,
			sizeof (glWdgtImageSelect),
			0,
			(GInstanceInitFunc) gl_wdgt_image_select_instance_init,
		};

		wdgt_image_select_type =
			g_type_register_static (gl_hig_vbox_get_type (),
						"glWdgtImageSelect",
						&wdgt_image_select_info, 0);
	}

	return wdgt_image_select_type;
}

static void
gl_wdgt_image_select_class_init (glWdgtImageSelectClass *class)
{
	GObjectClass *object_class;

	gl_debug (DEBUG_WDGT, "START");

	object_class = (GObjectClass *) class;

	parent_class = g_type_class_peek_parent (class);

	object_class->finalize = gl_wdgt_image_select_finalize;

	wdgt_image_select_signals[CHANGED] =
	    g_signal_new ("changed",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glWdgtImageSelectClass, changed),
			  NULL, NULL,
			  gl_marshal_VOID__VOID,
			  G_TYPE_NONE, 0);

	gl_debug (DEBUG_WDGT, "END");
}

static void
gl_wdgt_image_select_instance_init (glWdgtImageSelect *image_select)
{
	gl_debug (DEBUG_WDGT, "START");

	image_select->file_radio = NULL;
	image_select->file_entry = NULL;

	image_select->key_radio = NULL;
	image_select->key_combo = NULL;
	image_select->key_entry = NULL;

	gl_debug (DEBUG_WDGT, "END");
}

static void
gl_wdgt_image_select_finalize (GObject *object)
{
	glWdgtImageSelect *image_select;
	glWdgtImageSelectClass *class;

	gl_debug (DEBUG_WDGT, "START");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_WDGT_IMAGE_SELECT (object));

	image_select = GL_WDGT_IMAGE_SELECT (object);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_WDGT, "END");
}

/****************************************************************************/
/* New widget.                                                              */
/****************************************************************************/
GtkWidget *
gl_wdgt_image_select_new (glMerge *merge, glTextNode *text_node)
{
	glWdgtImageSelect *image_select;

	gl_debug (DEBUG_WDGT, "START");

	image_select = g_object_new (gl_wdgt_image_select_get_type (), NULL);

	gl_wdgt_image_select_construct (image_select, merge, text_node);

	gl_debug (DEBUG_WDGT, "END");

	return GTK_WIDGET (image_select);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                    */
/*--------------------------------------------------------------------------*/
static void
gl_wdgt_image_select_construct (glWdgtImageSelect *image_select,
				glMerge           *merge,
				glTextNode        *text_node)
{
	GtkWidget *wvbox, *whbox;
	GSList    *radio_group = NULL;
	GList     *keys;
	gchar     *filename;

	gl_debug (DEBUG_WDGT, "START");

	wvbox = GTK_WIDGET (image_select);

	/* ---- File line ---- */
	whbox = gl_hig_hbox_new ();
	gl_hig_vbox_add_widget (GL_HIG_VBOX(wvbox), whbox);

	/* File radio */
	image_select->file_radio = gtk_radio_button_new_with_label (radio_group,
								  _
								  ("File:"));
	radio_group =
	    gtk_radio_button_get_group (GTK_RADIO_BUTTON (image_select->file_radio));
	g_signal_connect (G_OBJECT (image_select->file_radio), "toggled",
			  G_CALLBACK (radio_toggled_cb),
			  G_OBJECT (image_select));
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), image_select->file_radio);

	/* File entry widget */
	image_select->file_entry =
		gnome_pixmap_entry_new ("image", "Load image", TRUE);
	gnome_pixmap_entry_set_preview_size (GNOME_PIXMAP_ENTRY
					     (image_select->file_entry),
					     128, 128);

	/* Set default path for image entry */
	filename = text_node->field_flag? NULL : text_node->data;
	if (filename != NULL) {
		/* Set default path to the directory containing image. */
		if (image_path != NULL)
			g_free (image_path);
		image_path = g_path_get_dirname (filename);
		if (image_path != NULL) {
			image_path = g_strconcat (image_path, "/", NULL);
		}
	} else if (image_path == NULL) {
		/* First time, set it to our CWD. */
		image_path = g_get_current_dir ();
	}
        /* Otherwise, leave it in the last directory that we got an image. */
        gnome_pixmap_entry_set_pixmap_subdir (GNOME_PIXMAP_ENTRY(image_select->file_entry),
					      image_path);
	g_signal_connect_swapped ( G_OBJECT(gnome_file_entry_gtk_entry
					    (GNOME_FILE_ENTRY (image_select->file_entry))),
				   "changed", G_CALLBACK (changed_cb), image_select);
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), image_select->file_entry);


	/* ---- Key line ---- */
	whbox = gl_hig_hbox_new ();
	gl_hig_vbox_add_widget (GL_HIG_VBOX(wvbox), whbox);

	/* Key radio */
	image_select->key_radio = gtk_radio_button_new_with_label (radio_group,
							      _("Key:"));
	g_signal_connect (G_OBJECT (image_select->key_radio), "toggled",
			  G_CALLBACK (radio_toggled_cb),
			  G_OBJECT (image_select));
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), image_select->key_radio);

	/* Key entry widget */
	image_select->key_combo = gtk_combo_new ();
	keys = gl_merge_get_key_list (merge);
	if (keys != NULL)
		gtk_combo_set_popdown_strings (GTK_COMBO (image_select->key_combo),
					       keys);
	gl_merge_free_key_list (&keys);
	image_select->key_entry = GTK_COMBO (image_select->key_combo)->entry;
	gtk_entry_set_editable (GTK_ENTRY (image_select->key_entry), FALSE);
	gtk_widget_set_size_request (image_select->key_combo, 200, -1);
	g_signal_connect_swapped (G_OBJECT (image_select->key_entry), "changed",
				  G_CALLBACK (changed_cb),
				  G_OBJECT (image_select));
	gl_hig_hbox_add_widget (GL_HIG_HBOX(whbox), image_select->key_combo);


	gl_wdgt_image_select_set_data (image_select,
				       (merge != NULL),
				       text_node);

	gl_debug (DEBUG_WDGT, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when any control in the widget has changed.       */
/*--------------------------------------------------------------------------*/
static void
changed_cb (glWdgtImageSelect *image_select)
{
	gl_debug (DEBUG_WDGT, "START");

	/* Emit our "changed" signal */
	g_signal_emit (G_OBJECT (image_select), wdgt_image_select_signals[CHANGED], 0);

	gl_debug (DEBUG_WDGT, "END");
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback to handle toggling of radio buttons                   */
/*--------------------------------------------------------------------------*/
static void
radio_toggled_cb (GtkToggleButton   *togglebutton,
		  glWdgtImageSelect *image_select)
{
	gl_debug (DEBUG_WDGT, "START");

	if (gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON (image_select->file_radio))) {
		gtk_widget_set_sensitive (image_select->file_entry, TRUE);
		gtk_widget_set_sensitive (image_select->key_combo, FALSE);
	} else {
		gtk_widget_set_sensitive (image_select->file_entry, FALSE);
		gtk_widget_set_sensitive (image_select->key_combo, TRUE);
	}

	/* Emit our "changed" signal */
	g_signal_emit (G_OBJECT (image_select), wdgt_image_select_signals[CHANGED], 0);

	gl_debug (DEBUG_WDGT, "END");
}

/****************************************************************************/
/* Set new field definitions.                                               */
/****************************************************************************/
void
gl_wdgt_image_select_set_field_defs  (glWdgtImageSelect *image_select,
				      glMerge            *merge)
{
	GList *keys;

	gl_debug (DEBUG_WDGT, "START");

	keys = gl_merge_get_key_list (merge);
	if ( keys != NULL ) {
		gtk_combo_set_popdown_strings (GTK_COMBO (image_select->key_combo),
					       keys);
		gl_merge_free_key_list (&keys);
	} else {
		keys = g_list_append (keys, "");
		gtk_combo_set_popdown_strings (GTK_COMBO (image_select->key_combo),
					       keys);
		g_list_free (keys);
	}

	gl_debug (DEBUG_WDGT, "END");
}


/****************************************************************************/
/* Get widget data.                                                         */
/****************************************************************************/
glTextNode *
gl_wdgt_image_select_get_data (glWdgtImageSelect *image_select)
{
	glTextNode *text_node;

	gl_debug (DEBUG_WDGT, "START");

	text_node = g_new0(glTextNode,1);

	if (gtk_toggle_button_get_active
	    (GTK_TOGGLE_BUTTON (image_select->file_radio))) {
		text_node->field_flag = FALSE;
		text_node->data =
			gnome_pixmap_entry_get_filename (GNOME_PIXMAP_ENTRY(image_select->file_entry));
	} else {
		text_node->field_flag = TRUE;
		text_node->data = 
		    gtk_editable_get_chars (GTK_EDITABLE (image_select->key_entry),
					    0, -1);
		gl_debug (DEBUG_WDGT, "text_node: field_flag=%d, data=%s",
			  text_node->field_flag, text_node->data);
	}

	gl_debug (DEBUG_WDGT, "END");

	return text_node;
}

/****************************************************************************/
/* Set widget data.                                                         */
/****************************************************************************/
void
gl_wdgt_image_select_set_data (glWdgtImageSelect *image_select,
			       gboolean           merge_flag,
			       glTextNode        *text_node)
{
	gint pos;

	gl_debug (DEBUG_WDGT, "START");

	gtk_widget_set_sensitive (image_select->key_radio, merge_flag);

	if (!text_node->field_flag) {

		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (image_select->file_radio), TRUE);

		gtk_widget_set_sensitive (image_select->file_entry, TRUE);
		gtk_widget_set_sensitive (image_select->key_combo, FALSE);

		if (text_node->data != NULL ) {
			gnome_file_entry_set_filename (GNOME_FILE_ENTRY(image_select->file_entry),
						       text_node->data);
		} else {
			gnome_file_entry_set_filename (GNOME_FILE_ENTRY(image_select->file_entry),
						       "");
		}
	} else {

		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (image_select->key_radio), TRUE);

		gtk_widget_set_sensitive (image_select->file_entry, FALSE);
		gtk_widget_set_sensitive (image_select->key_combo, TRUE);

		g_signal_handlers_block_by_func (G_OBJECT
						(image_select->key_entry),
						G_CALLBACK (changed_cb),
						image_select);
		gtk_editable_delete_text (GTK_EDITABLE (image_select->key_entry), 0,
					  -1);
		g_signal_handlers_unblock_by_func (G_OBJECT
						  (image_select->key_entry),
						  G_CALLBACK
						  (changed_cb), image_select);

		pos = 0;
		gtk_editable_insert_text (GTK_EDITABLE (image_select->key_entry),
					  text_node->data,
					  strlen (text_node->data),
					  &pos);
	}

	gl_debug (DEBUG_WDGT, "END");
}

/****************************************************************************/
/* Set size group for internal labels                                       */
/****************************************************************************/
void
gl_wdgt_image_select_set_label_size_group (glWdgtImageSelect   *image_select,
					   GtkSizeGroup        *label_size_group)
{
	gl_debug (DEBUG_WDGT, "START");

	gtk_size_group_add_widget (label_size_group, image_select->file_radio);
	gtk_size_group_add_widget (label_size_group, image_select->key_radio);

	gl_debug (DEBUG_WDGT, "END");
}
